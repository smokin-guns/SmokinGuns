use crate::AppState;
use std::{error::Error, sync::{atomic::{AtomicBool, Ordering}, Arc, OnceLock}};
use windows_sys::Win32::{
    Foundation::{HWND, LPARAM, LRESULT, WPARAM},
    System::{Console::{FreeConsole, GetConsoleProcessList, GetConsoleWindow}, DataExchange::{CloseClipboard, EmptyClipboard, OpenClipboard, SetClipboardData}, LibraryLoader::GetModuleHandleW, Memory::{GlobalAlloc, GlobalLock, GlobalUnlock, GMEM_MOVEABLE}},
    UI::WindowsAndMessaging::{
        CreateWindowExW, DefWindowProcW, DispatchMessageW, GetMessageW, PostQuitMessage,
        RegisterClassW, SetTimer, SetWindowTextW, ShowWindow, TranslateMessage, CS_HREDRAW,
        CS_VREDRAW, CW_USEDEFAULT, HMENU, MSG, SW_HIDE, SW_SHOW, WM_CLOSE, WM_COMMAND,
        WM_CREATE, WM_DESTROY, WM_NCCREATE, WM_TIMER, WNDCLASSW, WS_CAPTION, WS_CHILD,
        WS_OVERLAPPED, WS_SYSMENU, WS_VISIBLE, ES_AUTOHSCROLL, ES_READONLY, WS_TABSTOP,
    },
};

const ID_COPY: usize = 101;
const ID_NEW: usize = 102;
const ID_CLOSE: usize = 103;
static STATE: OnceLock<Arc<AppState>> = OnceLock::new();
static CODE_EDIT: OnceLock<isize> = OnceLock::new();
static STATUS: OnceLock<isize> = OnceLock::new();
static HIDDEN_OWN_CONSOLE: AtomicBool = AtomicBool::new(false);

fn wide(text: &str) -> Vec<u16> { text.encode_utf16().chain(Some(0)).collect() }

pub(super) fn prepare_console_for_gui() {
    unsafe {
        let console = GetConsoleWindow();
        if console.is_null() { return; }
        let mut pids = [0u32; 2];
        let count = GetConsoleProcessList(pids.as_mut_ptr(), pids.len() as u32);
        // A console shared with a terminal has at least its shell and this process.
        if count == 1 {
            ShowWindow(console, SW_HIDE);
            HIDDEN_OWN_CONSOLE.store(true, Ordering::Release);
        }
    }
}

pub(super) fn prepare_console_for_terminal() {
    unsafe {
        let console = GetConsoleWindow();
        if console.is_null() { return; }
        if HIDDEN_OWN_CONSOLE.swap(false, Ordering::AcqRel) {
            ShowWindow(console, SW_SHOW);
        }
    }
}

fn release_hidden_console() {
    if HIDDEN_OWN_CONSOLE.swap(false, Ordering::AcqRel) {
        unsafe { FreeConsole(); }
    }
}

pub(super) fn run(state: Arc<AppState>) -> Result<(), Box<dyn Error + Send + Sync>> {
    STATE.set(state).map_err(|_| "window already initialized")?;
    unsafe {
        let instance = GetModuleHandleW(std::ptr::null());
        let class_name = wide("SmokinGunsCompanionWindow");
        let class = WNDCLASSW { style: CS_HREDRAW | CS_VREDRAW, lpfnWndProc: Some(wnd_proc), hInstance: instance, lpszClassName: class_name.as_ptr(), ..std::mem::zeroed() };
        if RegisterClassW(&class) == 0 { return Err(std::io::Error::last_os_error().into()); }
        let title = wide("Smokin' Guns Companion");
        let window = CreateWindowExW(0, class_name.as_ptr(), title.as_ptr(), WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
            CW_USEDEFAULT, CW_USEDEFAULT, 450, 245, std::ptr::null_mut(), std::ptr::null_mut(), instance, std::ptr::null());
        if window.is_null() { return Err(std::io::Error::last_os_error().into()); }
        ShowWindow(window, SW_SHOW);
        let mut message: MSG = std::mem::zeroed();
        while GetMessageW(&mut message, std::ptr::null_mut(), 0, 0) > 0 {
            TranslateMessage(&message);
            DispatchMessageW(&message);
        }
    }
    release_hidden_console();
    Ok(())
}

unsafe extern "system" fn wnd_proc(hwnd: HWND, message: u32, wparam: WPARAM, lparam: LPARAM) -> LRESULT {
    match message {
        WM_NCCREATE => 1,
        WM_CREATE => {
            create_control(hwnd, "STATIC", "Paste this one-use code into the browser multiplayer panel:", 0, 18, 16, 405, 24, 0);
            let edit = create_control(hwnd, "EDIT", "", (ES_AUTOHSCROLL | ES_READONLY) as u32 | WS_TABSTOP, 18, 43, 405, 27, 0);
            let _ = CODE_EDIT.set(edit as isize);
            let status = create_control(hwnd, "STATIC", "", 0, 18, 79, 405, 50, 0);
            let _ = STATUS.set(status as isize);
            create_control(hwnd, "BUTTON", "Copy code", WS_TABSTOP, 18, 137, 120, 32, ID_COPY);
            create_control(hwnd, "BUTTON", "New code", WS_TABSTOP, 148, 137, 120, 32, ID_NEW);
            create_control(hwnd, "BUTTON", "Close companion", WS_TABSTOP, 278, 137, 145, 32, ID_CLOSE);
            SetTimer(hwnd, 1, 1000, None);
            update(hwnd);
            0
        }
        WM_TIMER => { update(hwnd); 0 }
        WM_COMMAND => {
            match (wparam & 0xffff) as usize {
                ID_COPY => copy_code(hwnd),
                ID_NEW => if let Some(state) = STATE.get() {
                    let mut used = state.pair_used.lock().unwrap();
                    *state.pairing_code.lock().unwrap() = crate::random_hex(16);
                    *used = false;
                    state.sessions.lock().unwrap().clear();
                    update(hwnd);
                },
                ID_CLOSE => { windows_sys::Win32::UI::WindowsAndMessaging::DestroyWindow(hwnd); }
                _ => {}
            }
            0
        }
        WM_CLOSE => { windows_sys::Win32::UI::WindowsAndMessaging::DestroyWindow(hwnd); 0 }
        WM_DESTROY => {
            if let Some(state) = STATE.get() { state.stop_requested.store(true, std::sync::atomic::Ordering::Release); }
            PostQuitMessage(0);
            0
        }
        _ => DefWindowProcW(hwnd, message, wparam, lparam),
    }
}

unsafe fn update(hwnd: HWND) {
    let Some(state) = STATE.get() else { return; };
    let code = state.pairing_code.lock().unwrap().clone();
    let paired = *state.pair_used.lock().unwrap();
    let servers = state.servers.read().unwrap().len();
    let code_line = if paired { "Pairing code used".to_owned() } else { format!("Pairing code: {code}") };
    if let Some(handle) = CODE_EDIT.get() { SetWindowTextW(*handle as HWND, wide(&code_line).as_ptr()); }
    let text = if paired { format!("Paired. New code resets pairing. {servers} server(s) responding.") }
        else { format!("Paste the code in the browser multiplayer panel. {servers} server(s) responding.") };
    if let Some(handle) = STATUS.get() { SetWindowTextW(*handle as HWND, wide(&text).as_ptr()); }
    let _ = hwnd;
}

unsafe fn create_control(parent: HWND, class: &str, text: &str, style: u32, x: i32, y: i32, width: i32, height: i32, id: usize) -> HWND {
    CreateWindowExW(0, wide(class).as_ptr(), wide(text).as_ptr(), WS_CHILD | WS_VISIBLE | style,
        x, y, width, height, parent, id as HMENU, GetModuleHandleW(std::ptr::null()), std::ptr::null())
}

unsafe fn copy_code(hwnd: HWND) {
    let Some(state) = STATE.get() else { return; };
    let code = state.pairing_code.lock().unwrap().clone();
    if OpenClipboard(hwnd) == 0 { return; }
    EmptyClipboard();
    let bytes = (code.encode_utf16().count() + 1) * std::mem::size_of::<u16>();
    let memory = GlobalAlloc(GMEM_MOVEABLE, bytes);
    if memory.is_null() { CloseClipboard(); return; }
    let destination = GlobalLock(memory) as *mut u16;
    if destination.is_null() { CloseClipboard(); return; }
    std::ptr::copy_nonoverlapping(code.encode_utf16().chain(Some(0)).collect::<Vec<_>>().as_ptr(), destination, bytes / 2);
    GlobalUnlock(memory);
    SetClipboardData(13, memory);
    CloseClipboard();
}
