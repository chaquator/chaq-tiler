#define NOMINMAX
#include <Windows.h>
#include <initializer_list>
#include <iostream>

#define NAME "chaq_tiler_dmmy"

void Proc(HWINEVENTHOOK hook, DWORD event, HWND hwnd, LONG isObj, LONG isChild, DWORD idEventThread,
          DWORD dwmsEventTime) {
    std::cout << "Poop\n";
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
        break;
    case WM_CLOSE:
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
    if (AttachConsole(ATTACH_PARENT_PROCESS) || AllocConsole()) {
#pragma warning(disable : 4996)
        freopen("CONOUT$", "w", stdout);
        freopen("CONOUT$", "w", stderr);
    }
    putchar('\n');

    std::initializer_list<DWORD[2]> pairs = {
        {EVENT_OBJECT_CREATE, EVENT_OBJECT_DESTROY},
        {EVENT_OBJECT_CLOAKED, EVENT_OBJECT_UNCLOAKED},
        {EVENT_SYSTEM_MINIMIZESTART, EVENT_SYSTEM_MINIMIZEEND},
        {EVENT_SYSTEM_MOVESIZESTART, EVENT_SYSTEM_MOVESIZEEND},
        {EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND},
        {EVENT_OBJECT_LOCATIONCHANGE, EVENT_OBJECT_LOCATIONCHANGE},
    };
    for (const auto& pair : pairs) {
        SetWinEventHook(pair[0], pair[1], NULL, Proc, NULL, NULL, WINEVENT_OUTOFCONTEXT);
    }

    WNDCLASSEX wc;
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = 0;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = NULL;
    wc.hIconSm = NULL;
    wc.hCursor = NULL;
    wc.hbrBackground = NULL;
    wc.lpszMenuName = NULL;
    wc.lpszClassName = NAME;

    if (!RegisterClassEx(&wc)) return -1;

    CreateWindowEx(0, NAME, NAME, 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, hInstance, NULL);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
