use axum::{
    body::Body,
    extract::{
        ws::{Message, WebSocket, WebSocketUpgrade},
        Path, State,
    },
    http::{header, HeaderMap, HeaderValue, Method, Request, StatusCode, Uri},
    middleware::{self, Next},
    response::{IntoResponse, Response},
    routing::{get, post},
    Json, Router,
};
use futures_util::{SinkExt, StreamExt};
use rand::RngCore;
use serde::{Deserialize, Serialize};
use std::{
    collections::{HashMap, HashSet},
    env, io,
    net::{SocketAddr, ToSocketAddrs, UdpSocket},
    path::{Component, Path as FsPath, PathBuf},
    sync::{
        atomic::{AtomicBool, AtomicUsize, Ordering},
        Arc, Mutex, RwLock,
    },
    thread,
    time::{Duration, Instant},
};
use tokio::net::TcpListener;
use tokio_util::io::ReaderStream;

#[cfg(target_os = "windows")]
mod gui_windows;
#[cfg(target_os = "linux")]
mod gui_x11;

const MAX_DATAGRAM_BYTES: usize = 1400;
const MAX_PACKETS_PER_SECOND: u32 = 120;
const MAX_CONCURRENT_SERVER_QUERIES: usize = 128;
const SESSION_LIFETIME: Duration = Duration::from_secs(12 * 60 * 60);

#[derive(Clone, Debug)]
struct Endpoint {
    host: String,
    port: u16,
    id: String,
}

#[derive(Clone, Debug, Serialize)]
struct ServerInfo {
    id: String,
    name: String,
    map: String,
    players: u32,
    #[serde(rename = "maxPlayers")]
    max_players: u32,
    ping: u128,
}

#[derive(Serialize)]
struct ServerList {
    servers: Vec<ServerInfo>,
}

#[derive(Deserialize)]
struct PairRequest {
    code: String,
}

#[derive(Serialize)]
struct PairResponse {
    token: String,
    #[serde(rename = "expiresIn")]
    expires_in: u64,
}

#[derive(Serialize)]
struct ErrorResponse {
    error: String,
}

pub(crate) struct AppState {
    pub(crate) pairing_code: Mutex<String>,
    pub(crate) pair_used: Mutex<bool>,
    pub(crate) sessions: Mutex<HashMap<String, Instant>>,
    pub(crate) servers: RwLock<HashMap<String, (Endpoint, ServerInfo)>>,
    last_refresh: Mutex<Option<Instant>>,
    fixed_endpoints: Vec<Endpoint>,
    allowed_origins: HashSet<String>,
    protocols: Vec<String>,
    master: String,
    master_port: u16,
    web_root: PathBuf,
    local_origin: String,
    pub(crate) stop_requested: AtomicBool,
}

fn env_port(name: &str, fallback: u16) -> Result<u16, String> {
    match env::var(name) {
        Ok(value) => value
            .parse::<u16>()
            .ok()
            .filter(|n| *n != 0)
            .ok_or_else(|| format!("{name} must be a port from 1 to 65535")),
        Err(_) => Ok(fallback),
    }
}

pub(crate) fn random_hex(bytes: usize) -> String {
    let mut value = vec![0u8; bytes];
    rand::thread_rng().fill_bytes(&mut value);
    value.iter().map(|byte| format!("{byte:02x}")).collect()
}

fn endpoint(value: &str) -> Result<Endpoint, String> {
    let (host, port) = value
        .rsplit_once(':')
        .ok_or_else(|| format!("Invalid server endpoint: {value}"))?;
    if host.is_empty()
        || !host
            .chars()
            .all(|c| c.is_ascii_alphanumeric() || c == '.' || c == '-')
    {
        return Err(format!("Invalid server endpoint: {value}"));
    }
    let port = port
        .parse::<u16>()
        .ok()
        .filter(|n| *n != 0)
        .ok_or_else(|| format!("Invalid server port: {value}"))?;
    let id = format!("{}-{port}", host.replace('.', "-"));
    Ok(Endpoint {
        host: host.to_owned(),
        port,
        id,
    })
}

fn oob(message: &str) -> Vec<u8> {
    let mut packet = vec![0xff; 4];
    packet.extend_from_slice(message.as_bytes());
    packet.push(0);
    packet
}

fn info_map(text: &str) -> HashMap<String, String> {
    let text = text.trim_matches(|c: char| c == '\0' || c.is_whitespace());
    let mut parts = text.trim_start_matches('\\').split('\\');
    let mut result = HashMap::new();
    while let (Some(key), Some(value)) = (parts.next(), parts.next()) {
        result.insert(key.to_owned(), value.to_owned());
    }
    result
}

fn socket_address(host: &str, port: u16) -> io::Result<SocketAddr> {
    (host, port)
        .to_socket_addrs()?
        .find(SocketAddr::is_ipv4)
        .ok_or_else(|| io::Error::new(io::ErrorKind::AddrNotAvailable, "no IPv4 address found"))
}

fn query_server(server: &Endpoint, protocols: &[String]) -> Result<(Endpoint, ServerInfo), String> {
    let address = socket_address(&server.host, server.port).map_err(|e| e.to_string())?;
    let socket = UdpSocket::bind("0.0.0.0:0").map_err(|e| e.to_string())?;
    socket
        .set_read_timeout(Some(Duration::from_millis(900)))
        .map_err(|e| e.to_string())?;
    let started = Instant::now();
    socket
        .send_to(&oob("getinfo xxx"), address)
        .map_err(|e| e.to_string())?;
    let mut packet = [0u8; 4096];
    let (size, _) = socket.recv_from(&mut packet).map_err(|e| e.to_string())?;
    let packet = &packet[..size];
    let marker = b"infoResponse\n";
    let offset = packet
        .windows(marker.len())
        .position(|window| window == marker)
        .ok_or_else(|| {
            let reply = String::from_utf8_lossy(packet.get(4..).unwrap_or(packet));
            format!(
                "Unexpected UDP reply from {}:{}: {}",
                server.host,
                server.port,
                reply.chars().take(100).collect::<String>()
            )
        })?;
    let info = info_map(&String::from_utf8_lossy(&packet[offset + marker.len()..]));
    let game = info.get("gamename").map(String::as_str).unwrap_or("");
    let reported_protocol = info.get("protocol").map(String::as_str).unwrap_or("?");
    let protocol_matches = protocols
        .iter()
        .any(|protocol| protocol == reported_protocol);
    // Legacy protocol 68 servers may omit gamename entirely. The original
    // client accepts that response when it is using the legacy protocol.
    let legacy_without_gamename = reported_protocol == "68"
        && protocols.iter().any(|protocol| protocol == "68")
        && game.is_empty();
    if (!legacy_without_gamename && game != "smokinguns") || !protocol_matches {
        return Err(format!("Server {}:{} reports game={game}, protocol={reported_protocol}; companion accepts smokinguns/{}.", server.host, server.port, protocols.join(" or ")));
    }
    let num = |key: &str| {
        info.get(key)
            .and_then(|value| value.parse().ok())
            .unwrap_or(0)
    };
    let details = ServerInfo {
        id: server.id.clone(),
        name: info
            .get("hostname")
            .cloned()
            .unwrap_or_else(|| format!("{}:{}", server.host, server.port)),
        map: info.get("mapname").cloned().unwrap_or_default(),
        players: num("clients"),
        max_players: num("sv_maxclients"),
        ping: started.elapsed().as_millis(),
    };
    Ok((server.clone(), details))
}

fn parse_master_packet(packet: &[u8]) -> Vec<Endpoint> {
    // Classic getserversResponse entries are `\\` + four IPv4 bytes + two
    // big-endian port bytes. The six address bytes are binary and may contain
    // `\\`, so only inspect delimiters at the fixed seven-byte record stride.
    let marker = b"getserversResponse";
    let Some(offset) = packet
        .windows(marker.len())
        .position(|window| window == marker)
    else {
        return Vec::new();
    };
    let mut cursor = offset + marker.len();
    let mut servers = Vec::new();
    while cursor + 7 <= packet.len() {
        if packet.get(cursor..cursor + 4) == Some(b"\\EOT") {
            break;
        }
        if packet[cursor] != b'\\' {
            break;
        }
        cursor += 1;
        let ip = format!(
            "{}.{}.{}.{}",
            packet[cursor],
            packet[cursor + 1],
            packet[cursor + 2],
            packet[cursor + 3]
        );
        let port = u16::from_be_bytes([packet[cursor + 4], packet[cursor + 5]]);
        if port != 0 {
            if let Ok(server) = endpoint(&format!("{ip}:{port}")) {
                servers.push(server);
            }
        }
        cursor += 6;
    }
    servers
}

fn query_master(master: &str, port: u16, protocol: &str) -> Result<Vec<Endpoint>, String> {
    let address = socket_address(master, port).map_err(|e| e.to_string())?;
    let socket = UdpSocket::bind("0.0.0.0:0").map_err(|e| e.to_string())?;
    socket
        .send_to(&oob(&format!("getservers {protocol} full empty")), address)
        .map_err(|e| e.to_string())?;
    let mut found = HashMap::new();
    let deadline = Instant::now() + Duration::from_secs(5);
    let mut last_packet = None;
    let mut response_packets = 0usize;
    let mut first_reply = None;
    let mut packet = [0u8; 4096];
    while Instant::now() < deadline {
        let quiet_remaining = last_packet
            .map(|at: Instant| Duration::from_millis(350).saturating_sub(at.elapsed()))
            .unwrap_or(Duration::from_secs(5));
        if quiet_remaining.is_zero() {
            break;
        }
        let remaining = deadline
            .saturating_duration_since(Instant::now())
            .min(quiet_remaining);
        socket
            .set_read_timeout(Some(remaining))
            .map_err(|e| e.to_string())?;
        match socket.recv_from(&mut packet) {
            Ok((size, _)) => {
                let reply = &packet[..size];
                last_packet = Some(Instant::now());
                response_packets += 1;
                if first_reply.is_none() {
                    first_reply = Some(reply.iter().take(96).copied().collect::<Vec<_>>());
                }
                for server in parse_master_packet(reply) {
                    found.insert(server.id.clone(), server);
                }
            }
            Err(error)
                if error.kind() == io::ErrorKind::WouldBlock
                    || error.kind() == io::ErrorKind::TimedOut =>
            {
                if last_packet.is_some() || Instant::now() >= deadline {
                    break;
                }
            }
            Err(error) => return Err(error.to_string()),
        }
    }
    if response_packets == 0 {
        return Err(format!(
            "No UDP response from master server {master}:{port} within 5 seconds"
        ));
    }
    if found.is_empty() {
        let sample = first_reply
            .as_deref()
            .unwrap_or_default()
            .iter()
            .map(|byte| format!("{byte:02x}"))
            .collect::<Vec<_>>()
            .join(" ");
        eprintln!(
            "Master {master}:{port} replied with {response_packets} packet(s) for protocol {protocol}, but no server addresses were listed. First reply bytes (hex, up to 96): {sample}"
        );
    }
    Ok(found.into_values().collect())
}

fn discover(state: &AppState) -> Result<Vec<(Endpoint, ServerInfo)>, String> {
    let endpoints = if !state.fixed_endpoints.is_empty() {
        state.fixed_endpoints.clone()
    } else {
        let queries = thread::scope(|scope| {
            state
                .protocols
                .iter()
                .map(|protocol| {
                    let master = state.master.clone();
                    let protocol = protocol.clone();
                    scope.spawn(move || {
                        let found = query_master(&master, state.master_port, &protocol);
                        (protocol, found)
                    })
                })
                .map(|job| job.join())
                .collect::<Vec<_>>()
        });
        let mut addresses = HashMap::new();
        let mut errors = Vec::new();
        for query in queries {
            match query {
                Ok((protocol, Ok(found))) => {
                    println!(
                        "Master query for protocol {protocol}: {} addresses.",
                        found.len()
                    );
                    for endpoint in found {
                        addresses.insert(endpoint.id.clone(), endpoint);
                    }
                }
                Ok((protocol, Err(error))) => {
                    eprintln!("Master query for protocol {protocol} failed: {error}");
                    errors.push(error);
                }
                Err(_) => errors.push("Master query worker failed".to_owned()),
            }
        }
        if addresses.is_empty() && !errors.is_empty() {
            return Err(errors.join("; "));
        }
        addresses.into_values().collect()
    };
    let count = endpoints.len();
    let next_endpoint = AtomicUsize::new(0);
    let failed_queries = AtomicUsize::new(0);
    let failure_examples = Mutex::new(Vec::new());
    let results = Mutex::new(Vec::new());
    let worker_count = count.min(MAX_CONCURRENT_SERVER_QUERIES);
    thread::scope(|scope| {
        for _ in 0..worker_count {
            let endpoints = &endpoints;
            let next_endpoint = &next_endpoint;
            let failed_queries = &failed_queries;
            let failure_examples = &failure_examples;
            let results = &results;
            let protocols = &state.protocols;
            scope.spawn(move || loop {
                let index = next_endpoint.fetch_add(1, Ordering::Relaxed);
                let Some(server) = endpoints.get(index) else {
                    break;
                };
                match query_server(server, protocols) {
                    Ok(found) => results.lock().unwrap().push(found),
                    Err(error) => {
                        failed_queries.fetch_add(1, Ordering::Relaxed);
                        let mut examples = failure_examples.lock().unwrap();
                        if examples.len() < 5 {
                            examples.push(error);
                        }
                    }
                }
            });
        }
    });
    let results = results.into_inner().unwrap();
    let failed = failed_queries.load(Ordering::Relaxed);
    if failed > 0 {
        eprintln!(
            "{failed} of {count} discovered server addresses did not return usable info (up to five examples): {}",
            failure_examples.into_inner().unwrap().join("; ")
        );
    }
    if state.fixed_endpoints.is_empty() {
        println!(
            "Discovery updated: {} responding Smokin' Guns servers from {count} addresses (protocols {}).",
            results.len(),
            state.protocols.join("/")
        );
    } else {
        println!(
            "Discovery updated: {} responding configured server(s).",
            results.len()
        );
    }
    Ok(results)
}

async fn cors(State(state): State<Arc<AppState>>, request: Request<Body>, next: Next) -> Response {
    if !request.uri().path().starts_with("/api/") {
        return next.run(request).await;
    }
    let origin = request
        .headers()
        .get(header::ORIGIN)
        .and_then(|h| h.to_str().ok())
        .unwrap_or("")
        .to_owned();
    let local_app_request = origin.is_empty()
        && request
            .headers()
            .get(header::HOST)
            .and_then(|value| value.to_str().ok())
            .is_some_and(|host| host == state.local_origin.trim_start_matches("http://"))
        && request
            .headers()
            .get(header::REFERER)
            .and_then(|value| value.to_str().ok())
            .is_some_and(|referer| referer.starts_with(&format!("{}/", state.local_origin)));
    if local_app_request {
        return next.run(request).await;
    }
    if origin.is_empty()
        || origin == "null"
        || (!state.allowed_origins.is_empty() && !state.allowed_origins.contains(&origin))
    {
        return (
            StatusCode::FORBIDDEN,
            Json(ErrorResponse {
                error: "This browser origin is not allowed by the companion".into(),
            }),
        )
            .into_response();
    }
    if request.method() == Method::OPTIONS {
        let mut response = StatusCode::NO_CONTENT.into_response();
        set_cors_headers(
            response.headers_mut(),
            &origin,
            request
                .headers()
                .get("access-control-request-private-network")
                .is_some_and(|h| h == "true"),
        );
        response.headers_mut().insert(
            header::ACCESS_CONTROL_ALLOW_METHODS,
            HeaderValue::from_static("GET, POST, OPTIONS"),
        );
        response.headers_mut().insert(
            header::ACCESS_CONTROL_ALLOW_HEADERS,
            HeaderValue::from_static("authorization, content-type"),
        );
        return response;
    }
    let private_network = request
        .headers()
        .get("access-control-request-private-network")
        .is_some_and(|h| h == "true");
    let mut response = next.run(request).await;
    set_cors_headers(response.headers_mut(), &origin, private_network);
    response
}

fn set_cors_headers(headers: &mut HeaderMap, origin: &str, private_network: bool) {
    if let Ok(value) = HeaderValue::from_str(origin) {
        headers.insert(header::ACCESS_CONTROL_ALLOW_ORIGIN, value);
    }
    headers.insert(header::VARY, HeaderValue::from_static("Origin"));
    if private_network {
        headers.insert(
            "access-control-allow-private-network",
            HeaderValue::from_static("true"),
        );
    }
}

fn api_error(status: StatusCode, message: impl Into<String>) -> Response {
    (
        status,
        Json(ErrorResponse {
            error: message.into(),
        }),
    )
        .into_response()
}

fn bearer(headers: &HeaderMap, state: &AppState) -> bool {
    let Some(value) = headers
        .get(header::AUTHORIZATION)
        .and_then(|h| h.to_str().ok())
    else {
        return false;
    };
    let Some(token) = value.strip_prefix("Bearer ") else {
        return false;
    };
    state
        .sessions
        .lock()
        .unwrap()
        .get(token)
        .is_some_and(|expires| *expires > Instant::now())
}

async fn pair(State(state): State<Arc<AppState>>, Json(input): Json<PairRequest>) -> Response {
    let mut used = state.pair_used.lock().unwrap();
    if *used || input.code != *state.pairing_code.lock().unwrap() {
        return api_error(
            StatusCode::FORBIDDEN,
            "Pairing key is invalid or already used",
        );
    }
    *used = true;
    let token = random_hex(24);
    state
        .sessions
        .lock()
        .unwrap()
        .insert(token.clone(), Instant::now() + SESSION_LIFETIME);
    Json(PairResponse {
        token,
        expires_in: SESSION_LIFETIME.as_secs(),
    })
    .into_response()
}

async fn reset_pairing(State(state): State<Arc<AppState>>, headers: HeaderMap) -> Response {
    if !bearer(&headers, &state) {
        return api_error(
            StatusCode::UNAUTHORIZED,
            "Pair this browser with the companion first",
        );
    }
    let code = random_hex(16);
    {
        let mut used = state.pair_used.lock().unwrap();
        *state.pairing_code.lock().unwrap() = code.clone();
        *used = false;
    }
    state.sessions.lock().unwrap().clear();
    Json(serde_json::json!({ "code": code })).into_response()
}

async fn servers(State(state): State<Arc<AppState>>, headers: HeaderMap) -> Response {
    if !bearer(&headers, &state) {
        return api_error(
            StatusCode::UNAUTHORIZED,
            "Pair this browser with the companion first",
        );
    }
    let servers = state
        .servers
        .read()
        .unwrap()
        .values()
        .map(|(_, info)| info.clone())
        .collect();
    Json(ServerList { servers }).into_response()
}

async fn refresh(State(state): State<Arc<AppState>>, headers: HeaderMap) -> Response {
    if !bearer(&headers, &state) {
        return api_error(
            StatusCode::UNAUTHORIZED,
            "Pair this browser with the companion first",
        );
    }
    {
        let mut last = state.last_refresh.lock().unwrap();
        if last.is_some_and(|instant| instant.elapsed() < Duration::from_secs(5)) {
            let total = state.servers.read().unwrap().len();
            return Json(serde_json::json!({ "servers": total })).into_response();
        }
        *last = Some(Instant::now());
    }
    let state_for_job = state.clone();
    match tokio::task::spawn_blocking(move || discover(&state_for_job)).await {
        Ok(Ok(found)) => {
            let total = found.len();
            *state.servers.write().unwrap() = found
                .into_iter()
                .map(|(endpoint, info)| (info.id.clone(), (endpoint, info)))
                .collect();
            Json(serde_json::json!({ "servers": total })).into_response()
        }
        Ok(Err(error)) => {
            eprintln!("Discovery failed: {error}");
            *state.servers.write().unwrap() = HashMap::new();
            api_error(StatusCode::BAD_GATEWAY, error)
        }
        Err(error) => api_error(StatusCode::INTERNAL_SERVER_ERROR, error.to_string()),
    }
}

async fn websocket(
    State(state): State<Arc<AppState>>,
    Path(id): Path<String>,
    headers: HeaderMap,
    ws: WebSocketUpgrade,
) -> Response {
    let origin = headers
        .get(header::ORIGIN)
        .and_then(|value| value.to_str().ok())
        .unwrap_or("");
    if origin.is_empty()
        || (!state.allowed_origins.is_empty() && !state.allowed_origins.contains(origin))
    {
        return api_error(
            StatusCode::FORBIDDEN,
            "This browser origin is not allowed by the companion",
        );
    }
    let protocols = headers
        .get(header::SEC_WEBSOCKET_PROTOCOL)
        .and_then(|h| h.to_str().ok())
        .unwrap_or("");
    let parts: Vec<_> = protocols.split(',').map(str::trim).collect();
    if parts.len() != 2 || parts[0] != "smokinguns" {
        return api_error(
            StatusCode::UNAUTHORIZED,
            "WebSocket session token is missing",
        );
    }
    let valid = state
        .sessions
        .lock()
        .unwrap()
        .get(parts[1])
        .is_some_and(|expires| *expires > Instant::now());
    if !valid {
        return api_error(
            StatusCode::UNAUTHORIZED,
            "Pair this browser with the companion first",
        );
    }
    let endpoint = state
        .servers
        .read()
        .unwrap()
        .get(&id)
        .map(|(endpoint, _)| endpoint.clone());
    let Some(endpoint) = endpoint else {
        return api_error(StatusCode::NOT_FOUND, "Selected server is no longer listed");
    };
    ws.protocols(["smokinguns"])
        .on_upgrade(move |socket| relay(socket, endpoint))
}

async fn relay(socket: WebSocket, endpoint: Endpoint) {
    let Ok(address) = socket_address(&endpoint.host, endpoint.port) else {
        return;
    };
    let Ok(udp) = tokio::net::UdpSocket::bind("0.0.0.0:0").await else {
        return;
    };
    let (mut outgoing, mut incoming) = socket.split();
    let mut packet = vec![0u8; MAX_DATAGRAM_BYTES + 1];
    let mut window = Instant::now();
    let mut packets_this_second = 0u32;
    loop {
        tokio::select! {
            message = incoming.next() => match message {
                Some(Ok(Message::Binary(payload))) if !payload.is_empty() && payload.len() <= MAX_DATAGRAM_BYTES => {
                    if window.elapsed() >= Duration::from_secs(1) { window = Instant::now(); packets_this_second = 0; }
                    packets_this_second += 1;
                    if packets_this_second > MAX_PACKETS_PER_SECOND { break; }
                    if udp.send_to(&payload, address).await.is_err() { break; }
                }
                Some(Ok(Message::Ping(payload))) => if outgoing.send(Message::Pong(payload)).await.is_err() { break; },
                Some(Ok(Message::Close(_))) | None | Some(Err(_)) => break,
                Some(Ok(_)) => break,
            },
            result = udp.recv_from(&mut packet) => match result {
                Ok((size, _)) if size <= MAX_DATAGRAM_BYTES => {
                    if outgoing.send(Message::Binary(packet[..size].to_vec().into())).await.is_err() { break; }
                }
                _ => break,
            }
        }
    }
}

async fn static_files(State(state): State<Arc<AppState>>, uri: Uri) -> Response {
    let relative = uri.path().trim_start_matches('/');
    let relative = if relative.is_empty() {
        "index.html"
    } else {
        relative
    };
    let live_web_root = PathBuf::from(env!("CARGO_MANIFEST_DIR")).join("..");
    let live_file = if relative == "index.html" || relative == "transport.js" {
        Some(live_web_root.join(relative))
    } else {
        None
    };
    let requested = FsPath::new(relative);
    if requested
        .components()
        .any(|part| !matches!(part, Component::Normal(_)))
    {
        return (StatusCode::BAD_REQUEST, "Invalid path").into_response();
    }
    let file = live_file
        .filter(|path| path.is_file())
        .unwrap_or_else(|| state.web_root.join(requested));
    if relative == "index.html" && !file.is_file() {
        return (StatusCode::OK, [(header::CONTENT_TYPE, "text/plain; charset=utf-8")],
            "Smokin' Guns local companion is running. Open the Smokin' Guns web app, paste the pairing key printed in this terminal, then search for servers.\n").into_response();
    }
    let metadata = match tokio::fs::metadata(&file).await {
        Ok(value) if value.is_file() => value,
        _ => return (StatusCode::NOT_FOUND, "Not found").into_response(),
    };
    let mime = match file
        .extension()
        .and_then(|value| value.to_str())
        .unwrap_or("")
    {
        "html" => "text/html; charset=utf-8",
        "js" => "text/javascript; charset=utf-8",
        "wasm" => "application/wasm",
        "data" | "pk3" => "application/octet-stream",
        "css" => "text/css; charset=utf-8",
        "json" => "application/json; charset=utf-8",
        "png" => "image/png",
        "ico" => "image/x-icon",
        "svg" => "image/svg+xml",
        "txt" => "text/plain; charset=utf-8",
        _ => "application/octet-stream",
    };
    let file = match tokio::fs::File::open(&file).await {
        Ok(value) => value,
        Err(_) => return (StatusCode::NOT_FOUND, "Not found").into_response(),
    };
    let mut response = Response::new(Body::from_stream(ReaderStream::new(file)));
    response
        .headers_mut()
        .insert(header::CONTENT_TYPE, HeaderValue::from_static(mime));
    response.headers_mut().insert(
        header::CONTENT_LENGTH,
        HeaderValue::from_str(&metadata.len().to_string()).unwrap(),
    );
    response
        .headers_mut()
        .insert(header::CACHE_CONTROL, HeaderValue::from_static("no-cache"));
    response
}

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    let port = env_port("SG_COMPANION_PORT", 8765)?;
    let master_port = env_port("SG_MASTER_PORT", 27950)?;
    let protocols = match env::var("SG_PROTOCOL") {
        Ok(protocol) => vec![protocol],
        Err(_) => vec!["71".into(), "68".into()],
    };
    let master = env::var("SG_MASTER").unwrap_or_else(|_| "master.smokin-guns.org".into());
    let web_root = env::var_os("SG_WEB_ROOT")
        .map(PathBuf::from)
        .unwrap_or_else(|| {
            let root = env::current_dir().unwrap_or_default();
            let site = root.join("build/site");
            if site.join("game-data/manifest.json").is_file() {
                site
            } else {
                root.join("build/release-emscripten-wasm32")
            }
        });
    let allowed_origins = env::var("SG_ALLOWED_ORIGINS")
        .unwrap_or_default()
        .split(',')
        .map(str::trim)
        .filter(|value| !value.is_empty())
        .map(str::to_owned)
        .collect();
    let fixed_endpoints = env::var("SG_SERVERS")
        .unwrap_or_default()
        .split(',')
        .map(str::trim)
        .filter(|value| !value.is_empty())
        .map(endpoint)
        .collect::<Result<Vec<_>, _>>()?;
    let state = Arc::new(AppState {
        pairing_code: Mutex::new(random_hex(16)),
        pair_used: Mutex::new(false),
        sessions: Mutex::new(HashMap::new()),
        servers: RwLock::new(HashMap::new()),
        last_refresh: Mutex::new(None),
        fixed_endpoints,
        allowed_origins,
        protocols,
        master,
        master_port,
        web_root,
        local_origin: format!("http://127.0.0.1:{port}"),
        stop_requested: AtomicBool::new(false),
    });
    let app = Router::new()
        .route(
            "/api/pair",
            post(pair).options(|| async { StatusCode::NO_CONTENT }),
        )
        .route(
            "/api/reset-pairing",
            post(reset_pairing).options(|| async { StatusCode::NO_CONTENT }),
        )
        .route(
            "/api/servers",
            get(servers).options(|| async { StatusCode::NO_CONTENT }),
        )
        .route(
            "/api/refresh",
            post(refresh).options(|| async { StatusCode::NO_CONTENT }),
        )
        .route("/udp/{id}", get(websocket))
        .fallback(static_files)
        .layer(middleware::from_fn_with_state(state.clone(), cors))
        .with_state(state.clone());
    let listener = TcpListener::bind(("127.0.0.1", port)).await?;
    println!("Smokin' Guns companion listening at http://127.0.0.1:{port}");
    println!(
        "Pairing key (one use): {}",
        state.pairing_code.lock().unwrap()
    );
    if state.web_root.join("index.html").is_file() {
        println!("Serving local web build: {}", state.web_root.display());
    } else {
        println!("Proxy-only mode: open the hosted Smokin' Guns web app in your browser.");
    }
    let expiry_state = state.clone();
    tokio::spawn(async move {
        loop {
            tokio::time::sleep(Duration::from_secs(3600)).await;
            expiry_state
                .sessions
                .lock()
                .unwrap()
                .retain(|_, expiry| *expiry > Instant::now());
        }
    });
    let server_state = state.clone();
    let server_task = tokio::spawn(async move {
        let shutdown_state = server_state.clone();
        let shutdown_signal = async move {
            loop {
                tokio::time::sleep(Duration::from_millis(250)).await;
                if shutdown_state.stop_requested.load(Ordering::Acquire) {
                    break;
                }
            }
        };
        axum::serve(listener, app)
            .with_graceful_shutdown(shutdown_signal)
            .await
    });

    #[cfg(target_os = "windows")]
    gui_windows::prepare_console_for_gui();
    #[cfg(target_os = "windows")]
    let gui_result = gui_windows::run(state.clone());
    #[cfg(target_os = "linux")]
    let gui_result = gui_x11::run(state.clone());
    #[cfg(not(any(target_os = "windows", target_os = "linux")))]
    let gui_result: Result<(), Box<dyn std::error::Error + Send + Sync>> =
        Err("The native companion window is available on Windows and Linux.".into());

    if let Err(error) = gui_result {
        eprintln!("Native companion window unavailable: {error}");
        #[cfg(target_os = "windows")]
        gui_windows::prepare_console_for_terminal();
        terminal_control(state.clone())?;
    }

    state.stop_requested.store(true, Ordering::Release);
    server_task.await??;
    Ok(())
}

fn terminal_control(state: Arc<AppState>) -> io::Result<()> {
    use std::io::{BufRead, Write};

    println!("Smokin' Guns companion is running in terminal mode.");
    println!("Commands: [s] status, [n] new pairing key, [q] quit");
    print!("> ");
    io::stdout().flush()?;
    for line in io::stdin().lock().lines() {
        match line?.trim().to_ascii_lowercase().as_str() {
            "n" | "new" => {
                let mut used = state.pair_used.lock().unwrap();
                *state.pairing_code.lock().unwrap() = random_hex(16);
                *used = false;
                state.sessions.lock().unwrap().clear();
                println!(
                    "New one-use pairing key: {}",
                    state.pairing_code.lock().unwrap()
                );
            }
            "s" | "status" => {
                let paired = *state.pair_used.lock().unwrap();
                let code = state.pairing_code.lock().unwrap().clone();
                let servers = state.servers.read().unwrap().len();
                if paired {
                    println!("Pairing: used; {servers} responding server(s). Use n to reset.");
                } else {
                    println!("Pairing key: {code}; {servers} responding server(s).");
                }
            }
            "q" | "quit" | "exit" => break,
            "" => {}
            _ => println!("Commands: [s] status, [n] new pairing key, [q] quit"),
        }
        print!("> ");
        io::stdout().flush()?;
    }
    Ok(())
}
