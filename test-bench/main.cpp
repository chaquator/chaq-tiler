#define NOMINMAX
#include <Windows.h>
#include <initializer_list>
#include <iostream>

#define NAME TEXT("chaq_tiler_dmmy")

void Proc(HWINEVENTHOOK hook, DWORD event, HWND hwnd, LONG idObj, LONG idChild, DWORD idEventThread,
          DWORD dwmsEventTime) {
    bool process_window = idChild == CHILDID_SELF && idObj == OBJID_WINDOW && hwnd != NULL;
    if (!process_window) return;
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

struct EventHookSpec {
    using Func = void (*)(HWINEVENTHOOK, DWORD, HWND, LONG, LONG, DWORD, DWORD);
    DWORD start;
    DWORD end;
    Func function;
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
    if (AttachConsole(ATTACH_PARENT_PROCESS) || AllocConsole()) {
#pragma warning(disable : 4996)
        freopen("CONOUT$", "w", stdout);
        freopen("CONOUT$", "w", stderr);
    }
    putchar('\n');

    std::initializer_list<EventHookSpec> specs = {
        {EVENT_OBJECT_CREATE, EVENT_OBJECT_DESTROY, Proc},
        {EVENT_OBJECT_CLOAKED, EVENT_OBJECT_UNCLOAKED, Proc},
        {EVENT_SYSTEM_MINIMIZESTART, EVENT_SYSTEM_MINIMIZEEND, Proc},
        {EVENT_SYSTEM_MOVESIZESTART, EVENT_SYSTEM_MOVESIZEEND, Proc},
        {EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND, Proc},
        {EVENT_OBJECT_LOCATIONCHANGE, EVENT_OBJECT_LOCATIONCHANGE, Proc},
    };
    for (const auto& spec : specs) {
        SetWinEventHook(spec.start, spec.end, NULL, spec.function, NULL, NULL, WINEVENT_OUTOFCONTEXT);
    }

    WNDCLASSEX wc = {};
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
