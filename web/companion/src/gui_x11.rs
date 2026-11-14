use crate::AppState;
use std::{error::Error, sync::Arc, thread, time::Duration};
use x11rb::{
    connection::Connection,
    protocol::{
        xproto::{
            AtomEnum, ClientMessageEvent, ConnectionExt, CreateGCAux, CreateWindowAux,
            EventMask, PropMode, SelectionNotifyEvent, WindowClass, SELECTION_NOTIFY_EVENT,
        },
        Event,
    },
    wrapper::ConnectionExt as WrapperConnectionExt,
    NONE,
};

const WIDTH: u16 = 420;
const HEIGHT: u16 = 220;
const CLIPBOARD: &[u8] = b"CLIPBOARD";
const UTF8_STRING: &[u8] = b"UTF8_STRING";
const WM_PROTOCOLS: &[u8] = b"WM_PROTOCOLS";
const WM_DELETE_WINDOW: &[u8] = b"WM_DELETE_WINDOW";

pub(super) fn run(state: Arc<AppState>) -> Result<(), Box<dyn Error + Send + Sync>> {
    let (conn, screen_num) = x11rb::connect(None)?;
    let screen = &conn.setup().roots[screen_num];
    let win = conn.generate_id()?;
    let gc = conn.generate_id()?;
    let font = conn.generate_id()?;
    conn.open_font(font, b"fixed")?;
    let white = screen.white_pixel;
    let black = screen.black_pixel;
    conn.create_window(
        screen.root_depth,
        win,
        screen.root,
        0,
        0,
        WIDTH,
        HEIGHT,
        0,
        WindowClass::INPUT_OUTPUT,
        screen.root_visual,
        &CreateWindowAux::new()
            .background_pixel(black)
            .border_pixel(white)
            .event_mask(EventMask::EXPOSURE | EventMask::BUTTON_PRESS | EventMask::STRUCTURE_NOTIFY),
    )?;
    let title = b"Smokin' Guns Companion";
    conn.change_property8(PropMode::REPLACE, win, AtomEnum::WM_NAME, AtomEnum::STRING, title)?;
    conn.create_gc(gc, win, &CreateGCAux::new().foreground(white).background(black).font(font))?;
    let protocols = conn.intern_atom(false, WM_PROTOCOLS)?.reply()?.atom;
    let delete = conn.intern_atom(false, WM_DELETE_WINDOW)?.reply()?.atom;
    conn.change_property32(PropMode::REPLACE, win, protocols, AtomEnum::ATOM, &[delete])?;
    let clipboard = conn.intern_atom(false, CLIPBOARD)?.reply()?.atom;
    let utf8 = conn.intern_atom(false, UTF8_STRING)?.reply()?.atom;
    let targets = conn.intern_atom(false, b"TARGETS")?.reply()?.atom;

    conn.map_window(win)?;
    conn.flush()?;
    let mut should_exit = false;
    let mut redraw = true;
    let mut next_refresh = std::time::Instant::now();
    while !should_exit {
        while let Some(event) = conn.poll_for_event()? {
            match event {
                Event::Expose(_) | Event::ConfigureNotify(_) => redraw = true,
                Event::ClientMessage(event) => {
                    let event: ClientMessageEvent = event;
                    if event.type_ == protocols && event.data.as_data32()[0] == delete {
                        should_exit = true;
                    }
                }
                Event::ButtonPress(event) => match (event.event_x, event.event_y) {
                    (x, y) if (20..190).contains(&x) && (139..167).contains(&y) => {
                        conn.set_selection_owner(win, clipboard, event.time)?;
                        conn.flush()?;
                    }
                    (x, y) if (205..390).contains(&x) && (139..167).contains(&y) => {
                        regenerate_code(&state);
                        redraw = true;
                    }
                    (x, y) if (20..390).contains(&x) && (184..212).contains(&y) => {
                        should_exit = true;
                    }
                    _ => {}
                },
                Event::SelectionRequest(request) => {
                    let target = request.target;
                    let response_property = if request.property == NONE { target } else { request.property };
                    if target == targets {
                        conn.change_property32(PropMode::REPLACE, request.requestor, response_property, AtomEnum::ATOM, &[targets, utf8, AtomEnum::STRING.into()])?;
                    } else if target == utf8 || target == u32::from(AtomEnum::STRING) {
                        let code = state.pairing_code.lock().unwrap().clone();
                        conn.change_property8(PropMode::REPLACE, request.requestor, response_property, target, code.as_bytes())?;
                    }
                    let notify = SelectionNotifyEvent { response_type: SELECTION_NOTIFY_EVENT, sequence: 0, time: request.time, requestor: request.requestor, selection: request.selection, target, property: response_property };
                    conn.send_event(false, request.requestor, EventMask::NO_EVENT, &notify)?;
                    conn.flush()?;
                }
                _ => {}
            }
        }
        if redraw {
            draw(&conn, win, gc, &state)?;
            redraw = false;
        }
        thread::sleep(Duration::from_millis(120));
        if next_refresh.elapsed() >= Duration::from_secs(1) {
            redraw = true;
            next_refresh = std::time::Instant::now();
        }
        if state.stop_requested.load(std::sync::atomic::Ordering::Acquire) {
            should_exit = true;
        }
    }
    state.stop_requested.store(true, std::sync::atomic::Ordering::Release);
    conn.destroy_window(win)?;
    conn.close_font(font)?;
    conn.flush()?;
    Ok(())
}

fn regenerate_code(state: &AppState) {
    let mut used = state.pair_used.lock().unwrap();
    *state.pairing_code.lock().unwrap() = crate::random_hex(16);
    *used = false;
    state.sessions.lock().unwrap().clear();
}

fn draw<C: Connection>(conn: &C, win: u32, gc: u32, state: &AppState) -> Result<(), Box<dyn Error + Send + Sync>> {
    conn.clear_area(false, win, 0, 0, WIDTH, HEIGHT)?;
    let code = state.pairing_code.lock().unwrap().clone();
    let paired = *state.pair_used.lock().unwrap();
    let servers = state.servers.read().unwrap().len();
    let status = if paired { "Paired. New code resets pairing." } else { "Enter this code in the browser multiplayer panel:" };
    let rows: [(i16, &str); 6] = [
        (26, "Smokin' Guns companion"),
        (57, status),
        (88, ""),
        (111, ""),
        (136, ""),
        (150, ""),
    ];
    for (y, text) in rows { if !text.is_empty() { conn.image_text8(win, gc, 20, y, text.as_bytes())?; } }
    let code_text = if paired { "(used)".to_owned() } else { code };
    conn.image_text8(win, gc, 20, 88, format!("Pairing code: {code_text}").as_bytes())?;
    conn.image_text8(win, gc, 20, 112, format!("{} server(s) responding", servers).as_bytes())?;
    conn.image_text8(win, gc, 20, 135, b"Keep this window open while using multiplayer.")?;
    for (x, label) in [(20, "Copy code"), (205, "New code"), (20, "Close companion")] {
        let idx = if label == "Close companion" { 2 } else if label == "New code" { 1 } else { 0 };
        let y = if idx == 2 { 200 } else { 155 };
        let w = if idx == 2 { 370 } else if idx == 0 { 170 } else { 185 };
        conn.poly_rectangle(win, gc, &[x11rb::protocol::xproto::Rectangle { x, y: y - 16, width: w, height: 28 }])?;
        conn.image_text8(win, gc, x + 12, y + 2, label.as_bytes())?;
    }
    conn.flush()?;
    Ok(())
}
