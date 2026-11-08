#define UNICODE
#define _UNICODE
#include <windows.h>
#include <commdlg.h>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <sstream>
#include <algorithm>\n#include <cmath>

#pragma comment(lib, "Comdlg32.lib")

static const COLORREF BG = RGB(4, 10, 18);
static const COLORREF PANEL = RGB(8, 20, 31);
static const COLORREF CYAN = RGB(25, 227, 255);
static const COLORREF DIM = RGB(60, 138, 153);
static const COLORREF AMBER = RGB(255, 176, 0);
static const COLORREF RED = RGB(255, 82, 82);
static const COLORREF INK = RGB(214, 248, 255);

enum {
    ID_ADB_DEVICES = 1001, ID_ADB_BOOT = 1002, ID_FASTBOOT_DEVICES = 1003,
    ID_REFRESH = 1004, ID_UNLOCK = 1005, ID_FLASH_RECOVERY = 1006,
    ID_FLASH_MISC = 1007, ID_REBOOT = 1008, ID_SIDeload = 1009,
    ID_PICK_RECOVERY = 1010, ID_PICK_MISC = 1011, ID_PICK_ZIP = 1012,
    ID_GUIDE_ROOT = 1013, ID_GUIDE_SPLASH = 1014
};

HWND gLog = nullptr, gInfo = nullptr, gStatus = nullptr, gRecoveryPath = nullptr, gMiscPath = nullptr, gZipPath = nullptr;
HFONT gFont = nullptr, gSmall = nullptr, gTitle = nullptr;
std::wstring gRecovery, gMisc, gZip;
std::mutex gLogMutex;
HWND gMain = nullptr;

std::wstring widen(const std::string& s) {
    if (s.empty()) return L"";
    int n = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), nullptr, 0);
    std::wstring out(n, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), out.data(), n);
    return out;
}

void appendLog(const std::wstring& line) {
    if (!gLog) return;
    std::wstring* p = new std::wstring(line + L"\r\n");
    PostMessage(gMain, WM_APP + 1, 0, (LPARAM)p);
}

void runCommand(const std::wstring& command) {
    appendLog(L"> " + command);
    std::thread([command]() {
        SECURITY_ATTRIBUTES sa{ sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE };
        HANDLE readPipe = nullptr, writePipe = nullptr;
        if (!CreatePipe(&readPipe, &writePipe, &sa, 0)) {
            appendLog(L"[ERROR] Could not create command pipe.");
            return;
        }
        SetHandleInformation(readPipe, HANDLE_FLAG_INHERIT, 0);
        STARTUPINFOW si{};
        si.cb = sizeof(si);
        si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;
        si.hStdOutput = writePipe;
        si.hStdError = writePipe;
        si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
        PROCESS_INFORMATION pi{};
        std::wstring cmd = L"cmd.exe /c " + command;
        std::vector<wchar_t> buffer(cmd.begin(), cmd.end());
        buffer.push_back(L'\0');
        BOOL ok = CreateProcessW(nullptr, buffer.data(), nullptr, nullptr, TRUE,
                                 CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi);
        CloseHandle(writePipe);
        if (!ok) {
            CloseHandle(readPipe);
            appendLog(L"[ERROR] Could not start command. Install Android SDK Platform Tools and add adb/fastboot to PATH.");
            return;
        }
        char data[2048];
        DWORD read = 0;
        std::string output;
        while (ReadFile(readPipe, data, sizeof(data), &read, nullptr) && read > 0) {
            output.append(data, data + read);
            if (output.size() > 16000) {
                appendLog(widen(output));
                output.clear();
            }
        }
        WaitForSingleObject(pi.hProcess, INFINITE);
        DWORD exitCode = 1;
        GetExitCodeProcess(pi.hProcess, &exitCode);
        if (!output.empty()) appendLog(widen(output));
        appendLog(L"[EXIT] code " + std::to_wstring(exitCode));
        if (command.find(L"=== ADB MODEL ===") != std::wstring::npos) {
            std::wstring* snapshot = new std::wstring(
                L"LAST TELEMETRY SNAPSHOT\r\n\r\n" + widen(output) +
                L"\r\n\r\nNOTE: Fastboot variables are only available while the tablet is in Fastboot mode."
            );
            PostMessage(gMain, WM_APP + 2, 0, (LPARAM)snapshot);
        }
        CloseHandle(readPipe); CloseHandle(pi.hThread); CloseHandle(pi.hProcess);
    }).detach();
}

void setStatus(const std::wstring& s) {
    if (gStatus) SetWindowTextW(gStatus, s.c_str());
}

void createLabel(HWND parent, const wchar_t* text, int x, int y, int w, int height, HFONT font = nullptr, COLORREF color = INK) {
    HWND label = CreateWindowW(L"STATIC", text, WS_CHILD | WS_VISIBLE, x, y, w, height, parent, nullptr, nullptr, nullptr);
    if (font) SendMessage(label, WM_SETFONT, (WPARAM)font, TRUE);
    SetPropW(label, L"DM_COLOR", (HANDLE)(ULONG_PTR)color);
}

HWND createButton(HWND parent, const wchar_t* text, int id, int x, int y, int w, int h) {
    HWND b = CreateWindowW(L"BUTTON", text, WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, x, y, w, h, parent, (HMENU)(INT_PTR)id, nullptr, nullptr);
    SendMessage(b, WM_SETFONT, (WPARAM)gFont, TRUE);
    return b;
}

void chooseFile(std::wstring& target, const wchar_t* filter, const wchar_t* title) {
    wchar_t path[MAX_PATH] = L"";
    OPENFILENAMEW ofn{};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = gMain;
    ofn.lpstrFilter = filter;
    ofn.lpstrFile = path;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrTitle = title;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    if (GetOpenFileNameW(&ofn)) target = path;
}

void appendToEdit(const std::wstring& s) {
    int len = GetWindowTextLengthW(gLog);
    SendMessageW(gLog, EM_SETSEL, len, len);
    SendMessageW(gLog, EM_REPLACESEL, FALSE, (LPARAM)s.c_str());
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_CREATE: {
        gMain = hwnd;
        gFont = CreateFontW(18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, FIXED_PITCH, L"Consolas");
        gSmall = CreateFontW(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, FIXED_PITCH, L"Consolas");
        gTitle = CreateFontW(28, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, FIXED_PITCH, L"Consolas");

        createLabel(hwnd, L"◈ DEVICEMOD  /  ANDROID DEVICE OPERATIONS", 22, 16, 760, 38, gTitle, CYAN);
        createLabel(hwnd, L"LENOVO TB-X505X TARGET PROFILE   •   ADB / FASTBOOT HOST CONSOLE", 25, 55, 850, 22, gSmall, DIM);
        gStatus = CreateWindowW(L"STATIC", L"● HOST READY  |  WAITING FOR DEVICE", WS_CHILD | WS_VISIBLE, 1000, 24, 450, 25, hwnd, nullptr, nullptr, nullptr);
        SendMessage(gStatus, WM_SETFONT, (WPARAM)gFont, TRUE);

        // Left: command/log console
        createLabel(hwnd, L"01 // LIVE OPERATION LOG", 22, 95, 500, 24, gFont, CYAN);
        gLog = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"[DeviceMod] Initialized.\r\n[DeviceMod] Connect tablet using a data-capable USB cable.\r\n",
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
            22, 125, 520, 590, hwnd, nullptr, nullptr, nullptr);
        SendMessage(gLog, WM_SETFONT, (WPARAM)gSmall, TRUE);

        // Center: operation buttons
        createLabel(hwnd, L"02 // COMMAND DECK", 565, 95, 450, 24, gFont, CYAN);
        createButton(hwnd, L"ADB DEVICES", ID_ADB_DEVICES, 565, 130, 205, 42);
        createButton(hwnd, L"REBOOT TO FASTBOOT", ID_ADB_BOOT, 780, 130, 225, 42);
        createButton(hwnd, L"FASTBOOT DEVICES", ID_FASTBOOT_DEVICES, 565, 180, 205, 42);
        createButton(hwnd, L"REFRESH DEVICE INFO", ID_REFRESH, 780, 180, 225, 42);
        createButton(hwnd, L"UNLOCK BOOTLOADER  [ERASES DATA]", ID_UNLOCK, 565, 235, 440, 44);

        createLabel(hwnd, L"RECOVERY IMAGE", 565, 300, 200, 20, gSmall, DIM);
        gRecoveryPath = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 565, 325, 340, 28, hwnd, nullptr, nullptr, nullptr);
        SendMessage(gRecoveryPath, WM_SETFONT, (WPARAM)gSmall, TRUE);
        createButton(hwnd, L"BROWSE", ID_PICK_RECOVERY, 915, 325, 90, 28);
        createButton(hwnd, L"FLASH RECOVERY", ID_FLASH_RECOVERY, 565, 360, 440, 38);

        createLabel(hwnd, L"MISC REBOOT IMAGE", 565, 410, 200, 20, gSmall, DIM);
        gMiscPath = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 565, 435, 340, 28, hwnd, nullptr, nullptr, nullptr);
        SendMessage(gMiscPath, WM_SETFONT, (WPARAM)gSmall, TRUE);
        createButton(hwnd, L"BROWSE", ID_PICK_MISC, 915, 435, 90, 28);
        createButton(hwnd, L"FLASH MISC", ID_FLASH_MISC, 565, 470, 440, 38);

        createLabel(hwnd, L"RECOVERY ZIP", 565, 520, 200, 20, gSmall, DIM);
        gZipPath = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 565, 545, 340, 28, hwnd, nullptr, nullptr, nullptr);
        SendMessage(gZipPath, WM_SETFONT, (WPARAM)gSmall, TRUE);
        createButton(hwnd, L"BROWSE", ID_PICK_ZIP, 915, 545, 90, 28);
        createButton(hwnd, L"ADB SIDELOAD ZIP", ID_SIDeload, 565, 580, 440, 38);
        createButton(hwnd, L"REBOOT", ID_REBOOT, 565, 640, 210, 42);
        createButton(hwnd, L"ROOT WORKFLOW GUIDE", ID_GUIDE_ROOT, 785, 640, 220, 42);
        createButton(hwnd, L"SPLASH LAB / SAFETY", ID_GUIDE_SPLASH, 565, 690, 440, 34);

        // Right: device info and animation
        createLabel(hwnd, L"03 // DEVICE TELEMETRY", 1030, 95, 420, 24, gFont, CYAN);
        gInfo = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT",
            L"DEVICE       : NOT DETECTED\r\nMODEL        : --\r\nPRODUCT      : --\r\nBATTERY      : --\r\nBOOTLOADER   : UNKNOWN\r\nADB STATE    : UNKNOWN\r\nFASTBOOT     : UNKNOWN\r\n\r\nUSB LINK     : WAITING\r\nTARGET       : TB-X505X",
            WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_READONLY,
            1030, 125, 420, 230, hwnd, nullptr, nullptr, nullptr);
        SendMessage(gInfo, WM_SETFONT, (WPARAM)gSmall, TRUE);
        createLabel(hwnd, L"04 // DEVICE ORIENTATION", 1030, 380, 420, 24, gFont, CYAN);
        createLabel(hwnd, L"[ FRONT VIEW ]  ANIMATED CONNECTION GUIDE", 1030, 415, 420, 20, gSmall, DIM);
        createLabel(hwnd, L"FASTBOOT: POWER OFF → VOL DOWN + USB", 1030, 650, 420, 22, gSmall, AMBER);
        createLabel(hwnd, L"Illustration is generic; verify your tablet's port location.", 1030, 678, 420, 20, gSmall, DIM);
        SetTimer(hwnd, 1, 70, nullptr);
        return 0;
    }
    case WM_TIMER:
        if (wp == 1) InvalidateRect(hwnd, nullptr, FALSE);
        return 0;
    case WM_PAINT: {
        PAINTSTRUCT ps{};
        HDC dc = BeginPaint(hwnd, &ps);
        // Animated device silhouette, radar rings and USB lead-in.
        const int cx = 1240, cy = 535;
        HPEN cyanPen = CreatePen(PS_SOLID, 2, CYAN);
        HPEN dimPen = CreatePen(PS_SOLID, 1, DIM);
        HPEN amberPen = CreatePen(PS_SOLID, 2, AMBER);
        HBRUSH panelBrush = CreateSolidBrush(PANEL);
        HBRUSH oldBrush = (HBRUSH)SelectObject(dc, panelBrush);
        HPEN oldPen = (HPEN)SelectObject(dc, cyanPen);
        RoundRect(dc, cx - 78, cy - 105, cx + 78, cy + 105, 18, 18);
        SelectObject(dc, GetStockObject(NULL_BRUSH));
        Ellipse(dc, cx - 5, cy - 91, cx + 5, cy - 81);
        Ellipse(dc, cx - 38, cy - 38, cx + 38, cy + 38);
        SelectObject(dc, dimPen);
        Ellipse(dc, cx - 53, cy - 53, cx + 53, cy + 53);
        Ellipse(dc, cx - 67, cy - 67, cx + 67, cy + 67);
        // Moving radar sweep line.
        double phase = (GetTickCount64() % 4000) / 4000.0 * 6.283185307;
        int sx = cx + (int)(62.0 * std::cos(phase));
        int sy = cy + (int)(62.0 * std::sin(phase));
        MoveToEx(dc, cx, cy, nullptr); LineTo(dc, sx, sy);
        SelectObject(dc, amberPen);
        MoveToEx(dc, cx, cy + 105, nullptr);
        LineTo(dc, cx, cy + 133);
        LineTo(dc, cx + 24, cy + 133);
        Rectangle(dc, cx + 18, cy + 127, cx + 34, cy + 139);
        SetBkMode(dc, TRANSPARENT);
        SetTextColor(dc, CYAN);
        TextOutW(dc, cx - 50, cy - 9, L"X505X", 5);
        SetTextColor(dc, AMBER);
        TextOutW(dc, cx - 43, cy + 145, L"USB LINK", 8);
        SelectObject(dc, oldPen);
        SelectObject(dc, oldBrush);
        DeleteObject(cyanPen); DeleteObject(dimPen); DeleteObject(amberPen); DeleteObject(panelBrush);
        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_APP + 1: {
        std::wstring* p = (std::wstring*)lp;
        if (p) { appendToEdit(*p); delete p; }
        return 0;
    }
    case WM_APP + 2: {
        std::wstring* p = (std::wstring*)lp;
        if (p) { SetWindowTextW(gInfo, p->c_str()); delete p; }
        setStatus(L"● TELEMETRY RECEIVED  |  REVIEW VALUES IN RIGHT PANEL");
        return 0;
    }
    case WM_COMMAND:
        switch (LOWORD(wp)) {
        case ID_ADB_DEVICES: runCommand(L"adb devices -l"); break;
        case ID_ADB_BOOT:
            if (MessageBoxW(hwnd, L"Reboot the connected Android device to bootloader mode?", L"Confirm operation", MB_YESNO | MB_ICONQUESTION) == IDYES)
                runCommand(L"adb reboot bootloader");
            break;
        case ID_FASTBOOT_DEVICES: runCommand(L"fastboot devices -l"); break;
        case ID_REFRESH:
            runCommand(L"echo === ADB MODEL === & adb shell getprop ro.product.model & echo === ADB PRODUCT === & adb shell getprop ro.product.device & echo === BATTERY === & adb shell dumpsys battery & echo === FASTBOOT UNLOCK STATE (if in fastboot) === & fastboot getvar unlocked");
            setStatus(L"● QUERYING DEVICE  |  SEE LIVE LOG");
            break;
        case ID_UNLOCK:
            if (MessageBoxW(hwnd,
                L"WARNING: Bootloader unlocking normally erases ALL user data and reduces device security.\n\nThis command is community-reported for some X505X builds, not Lenovo-confirmed. Verify your exact firmware and backup first.\n\nRun: fastboot oem unlock-go",
                L"DESTRUCTIVE OPERATION", MB_YESNO | MB_ICONWARNING) == IDYES)
                runCommand(L"fastboot oem unlock-go");
            break;
        case ID_PICK_RECOVERY:
            chooseFile(gRecovery, L"Recovery image (*.img)\0*.img\0All files (*.*)\0*.*\0\0", L"Select TB-X505X recovery.img");
            SetWindowTextW(gRecoveryPath, gRecovery.c_str()); break;
        case ID_FLASH_RECOVERY:
            if (gRecovery.empty()) { MessageBoxW(hwnd, L"Select recovery.img first.", L"Missing file", MB_OK | MB_ICONWARNING); break; }
            if (MessageBoxW(hwnd, L"Flash the selected image to the recovery partition? Confirm it is specifically for TB-X505X and your firmware layout.", L"Confirm flash", MB_YESNO | MB_ICONWARNING) == IDYES)
                runCommand(L"fastboot flash recovery \"" + gRecovery + L"\"");
            break;
        case ID_PICK_MISC:
            chooseFile(gMisc, L"Image (*.img)\0*.img\0All files (*.*)\0*.*\0\0", L"Select misc_rebootrecovery.img");
            SetWindowTextW(gMiscPath, gMisc.c_str()); break;
        case ID_FLASH_MISC:
            if (gMisc.empty()) { MessageBoxW(hwnd, L"Select misc_rebootrecovery.img first.", L"Missing file", MB_OK | MB_ICONWARNING); break; }
            if (MessageBoxW(hwnd, L"Flash the selected misc image? Use only the matching image supplied for your exact OrangeFox build.", L"Confirm flash", MB_YESNO | MB_ICONWARNING) == IDYES)
                runCommand(L"fastboot flash misc \"" + gMisc + L"\"");
            break;
        case ID_PICK_ZIP:
            chooseFile(gZip, L"ZIP package (*.zip)\0*.zip\0All files (*.*)\0*.*\0\0", L"Select sideload ZIP");
            SetWindowTextW(gZipPath, gZip.c_str()); break;
        case ID_SIDeload:
            if (gZip.empty()) { MessageBoxW(hwnd, L"Select a ZIP package first.", L"Missing file", MB_OK | MB_ICONWARNING); break; }
            if (MessageBoxW(hwnd, L"Start ADB sideload? First boot the device into a recovery that has enabled ADB sideload.", L"Confirm sideload", MB_YESNO | MB_ICONQUESTION) == IDYES)
                runCommand(L"adb sideload \"" + gZip + L"\"");
            break;
        case ID_REBOOT: runCommand(L"fastboot reboot"); break;
        case ID_GUIDE_ROOT:
            MessageBoxW(hwnd,
                L"ROOT WORKFLOW (manual safety-first guide)\n\n1. Download the exact stock firmware for the installed build.\n2. Extract its boot.img.\n3. Install official Magisk app on the tablet.\n4. Patch boot.img in Magisk and copy the patched image to this PC.\n5. Verify the TB-X505X partition layout and boot slot before flashing.\n6. DeviceMod does not guess a boot partition name; use the device-specific verified instructions.",
                L"Magisk Root Workflow", MB_OK | MB_ICONINFORMATION);
            break;
        case ID_GUIDE_SPLASH:
            MessageBoxW(hwnd,
                L"SPLASH LAB LOCKED\n\nThe X505X splash/logo partition, image format, dimensions and offsets have not been verified. Redmi Note 6 Pro splash instructions must not be reused. No raw splash flashing command is provided.",
                L"Device-specific verification required", MB_OK | MB_ICONWARNING);
            break;
        }
        return 0;
    case WM_CTLCOLORSTATIC: {
        HDC dc = (HDC)wp;
        HWND ctl = (HWND)lp;
        SetBkColor(dc, BG);
        HANDLE color = GetPropW(ctl, L"DM_COLOR");
        SetTextColor(dc, color ? (COLORREF)(ULONG_PTR)color : INK);
        return (LRESULT)GetStockObject(NULL_BRUSH);
    }
    case WM_CTLCOLOREDIT: {
        HDC dc = (HDC)wp;
        SetTextColor(dc, INK);
        SetBkColor(dc, PANEL);
        return (LRESULT)CreateSolidBrush(PANEL);
    }
    case WM_ERASEBKGND: {
        RECT r; GetClientRect(hwnd, &r);
        HBRUSH b = CreateSolidBrush(BG); FillRect((HDC)wp, &r, b); DeleteObject(b);
        // Subtle HUD grid
        HPEN pen = CreatePen(PS_SOLID, 1, RGB(10, 30, 42));
        auto old = SelectObject((HDC)wp, pen);
        for (int x = 0; x < r.right; x += 32) { MoveToEx((HDC)wp, x, 0, nullptr); LineTo((HDC)wp, x, r.bottom); }
        for (int y = 0; y < r.bottom; y += 32) { MoveToEx((HDC)wp, 0, y, nullptr); LineTo((HDC)wp, r.right, y); }
        SelectObject((HDC)wp, old); DeleteObject(pen);
        return 1;
    }
    case WM_DESTROY:
        if (gFont) DeleteObject(gFont);
        if (gSmall) DeleteObject(gSmall);
        if (gTitle) DeleteObject(gTitle);
        PostQuitMessage(0); return 0;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int show) {
    const wchar_t CLASS_NAME[] = L"DeviceModWindow";
    WNDCLASSW wc{};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = instance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(BG);
    RegisterClassW(&wc);
    HWND hwnd = CreateWindowExW(0, CLASS_NAME, L"DeviceMod (Android) - Device Operations",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 1490, 810,
        nullptr, nullptr, instance, nullptr);
    if (!hwnd) return 0;
    ShowWindow(hwnd, show);
    UpdateWindow(hwnd);
    MSG msg{};
    while (GetMessageW(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return (int)msg.wParam;
}
