// SP MOD Launcher – Wyatt
// Compile with: cl main.cpp user32.lib gdi32.lib comdlg32.lib /link /SUBSYSTEM:WINDOWS
#include <windows.h>
#include <commctrl.h>
#include <tchar.h>
#include <string>

#pragma comment(lib, "comctl32.lib")

// ---------------------------------------------------------------------------
// Configuration – edit these paths to point to your legitimate game installs
// ---------------------------------------------------------------------------
struct GameInfo {
    const TCHAR* name;      // shown in the column header
    const TCHAR* exePath;   // full path to the .exe (or empty to disable)
    const TCHAR* args;      // optional command-line arguments
};

GameInfo games[] = {
    { TEXT("IW4 SP"),      TEXT(""), TEXT("") },   // Modern Warfare 2 SP
    { TEXT("T5 SP"),       TEXT(""), TEXT("") },   // Black Ops SP
    { TEXT("T5 Zombies"),  TEXT(""), TEXT("") },   // Black Ops Zombies
    { TEXT("IW5 SP"),      TEXT(""), TEXT("") },   // Modern Warfare 3 SP
    { TEXT("IW3 SP"),      TEXT(""), TEXT("") },   // Modern Warfare (2007) SP
    { TEXT("IW5 SP Mod"),  TEXT(""), TEXT("") }    // extra slot
};
const int NUM_GAMES = _countof(games);

// ---------------------------------------------------------------------------
// Global handles
// ---------------------------------------------------------------------------
HWND g_hWndMain = nullptr;
HWND g_hBtn[NUM_GAMES] = { 0 };

// ---------------------------------------------------------------------------
// Helper: launch a game (if exePath is not empty)
// ---------------------------------------------------------------------------
void LaunchGame(int idx)
{
    if (!games[idx].exePath || games[idx].exePath[0] == 0) {
        MessageBox(g_hWndMain,
            TEXT("Game path not configured – edit the source and recompile."),
            TEXT("Missing path"), MB_ICONWARNING);
        return;
    }

    STARTUPINFO si = { sizeof(si) };
    PROCESS_INFORMATION pi = { 0 };

    // Build command line: "exePath" args
    std::basic_string<TCHAR> cmd = TEXT("\"");
    cmd += games[idx].exePath;
    cmd += TEXT("\"");
    if (games[idx].args && games[idx].args[0])
    {
        cmd += TEXT(" ");
        cmd += games[idx].args;
    }

    BOOL ok = CreateProcess(
        nullptr,
        const_cast<LPTSTR>(cmd.c_str()),
        nullptr, nullptr,
        FALSE, 0, nullptr, nullptr,
        &si, &pi);

    if (ok) {
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
    }
    else {
        MessageBox(g_hWndMain,
            TEXT("Failed to start the game executable."),
            TEXT("Error"), MB_ICONERROR);
    }
}

// ---------------------------------------------------------------------------
// Button callback
// ---------------------------------------------------------------------------
void OnLaunchButton(HWND hBtn)
{
    for (int i = 0; i < NUM_GAMES; ++i) {
        if (hBtn == g_hBtn[i]) {
            LaunchGame(i);
            break;
        }
    }
}

// ---------------------------------------------------------------------------
// Window procedure
// ---------------------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CREATE:
    {
        // ----- create column headers (static text) -----
        int colWidth = 140;
        int x = 20;
        int yHeader = 30;
        int yButton = 70;
        int btnHeight = 40;

        for (int i = 0; i < NUM_GAMES; ++i)
        {
            // Header
            CreateWindow(TEXT("STATIC"), games[i].name,
                WS_VISIBLE | WS_CHILD | SS_CENTER,
                x, yHeader, colWidth, 25,
                hWnd, nullptr, nullptr, nullptr);

            // Launch button
            g_hBtn[i] = CreateWindow(TEXT("BUTTON"), TEXT("Launch SP MOD"),
                WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                x, yButton, colWidth, btnHeight,
                hWnd, (HMENU)(1000 + i), nullptr, nullptr);

            x += colWidth + 20;
        }

        // ----- watermark at the bottom -----
        CreateWindow(TEXT("STATIC"),
            TEXT("SP MOD Launcher – Wyatt © 2025"),
            WS_VISIBLE | WS_CHILD | SS_CENTER,
            0, 0, 0, 0,
            hWnd, nullptr, nullptr, nullptr);

        return 0;
    }

    case WM_COMMAND:
    {
        WORD id = LOWORD(wParam);
        if (id >= 1000 && id < 1000 + NUM_GAMES) {
            OnLaunchButton((HWND)lParam);
        }
        break;
    }

    case WM_CTLCOLORSTATIC:
    {
        // Make the watermark light-gray
        HDC hdc = (HDC)wParam;
        SetTextColor(hdc, RGB(120, 120, 120));
        SetBkMode(hdc, TRANSPARENT);
        return (LRESULT)GetStockObject(NULL_BRUSH);
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

// ---------------------------------------------------------------------------
// WinMain
// ---------------------------------------------------------------------------
int APIENTRY wWinMain(HINSTANCE hInst, HINSTANCE, LPTSTR, int nCmdShow)
{
    INITCOMMONCONTROLSEX icc = { sizeof(icc), ICC_WIN95_CLASSES };
    InitCommonControlsEx(&icc);

    const TCHAR* CLASS_NAME = TEXT("SPModLauncherClass");

    WNDCLASSEX wc = { sizeof(WNDCLASSEX) };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.style = CS_HREDRAW | CS_VREDRAW;

    RegisterClassEx(&wc);

    // Calculate window size to fit all columns + margins
    int colWidth = 140;
    int totalWidth = 40 + NUM_GAMES * (colWidth + 20);
    int totalHeight = 180;

    g_hWndMain = CreateWindowEx(
        0,
        CLASS_NAME,
        TEXT("SP MOD Launcher - Wyatt"),
        WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT,
        totalWidth, totalHeight,
        nullptr, nullptr, hInst, nullptr);

    // Center the watermark
    HWND hWater = GetDlgItem(g_hWndMain, 0); // first static control
    if (hWater) {
        RECT rcClient;
        GetClientRect(g_hWndMain, &rcClient);
        SetWindowPos(hWater, nullptr,
            0, rcClient.bottom - 30,
            rcClient.right, 25,
            SWP_NOZORDER);
    }

    ShowWindow(g_hWndMain, nCmdShow);
    UpdateWindow(g_hWndMain);

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}