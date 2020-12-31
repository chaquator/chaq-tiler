#include <windows.h>

#include <cstdio>
#include <cwchar>
#include <string_view>
#include <unordered_map>

#ifdef NDEBUG
#define debug(s) ((void)0)
#else
#define debug(s) OutputDebugStringA(s "\n\n")
#endif // NDEBUG

constexpr std::size_t buflen = 512;

void StyleDecompose(HWND window) {
    using namespace std::literals;

    // Yes I copied these manually
    static std::unordered_map<LONG, std::wstring_view> styles = {
        {0x00800000L, L"WS_BORDER"sv},
        {0x00C00000L, L"WS_CAPTION"sv},
        {0x40000000L, L"WS_CHILD"sv},
        {0x40000000L, L"WS_CHILDWINDOW"sv},
        {0x02000000L, L"WS_CLIPCHILDREN"sv},
        {0x04000000L, L"WS_CLIPSIBLINGS"sv},
        {0x08000000L, L"WS_DISABLED"sv},
        {0x00400000L, L"WS_DLGFRAME"sv},
        {0x00020000L, L"WS_GROUP"sv},
        {0x00100000L, L"WS_HSCROLL"sv},
        {0x20000000L, L"WS_ICONIC"sv},
        {0x01000000L, L"WS_MAXIMIZE"sv},
        {0x00010000L, L"WS_MAXIMIZEBOX"sv},
        {0x20000000L, L"WS_MINIMIZE"sv},
        {0x00020000L, L"WS_MINIMIZEBOX"sv},
        {0x00000000L, L"WS_OVERLAPPED"sv},
        {(WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX),
         L"WS_OVERLAPPEDWINDOW"sv},
        {0x80000000L, L"WS_POPUP"sv},
        {(WS_POPUP | WS_BORDER | WS_SYSMENU), L"WS_POPUPWINDOW"sv},
        {0x00040000L, L"WS_SIZEBOX"sv},
        {0x00080000L, L"WS_SYSMENU"sv},
        {0x00010000L, L"WS_TABSTOP"sv},
        {0x00040000L, L"WS_THICKFRAME"sv},
        {0x00000000L, L"WS_TILED"sv},
        {(WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX),
         L"WS_TILEDWINDOW"sv},
        {0x10000000L, L"WS_VISIBLE"sv},
        {0x00200000L, L"WS_VSCROLL"sv},
    };
    static std::unordered_map<LONG, std::wstring_view> ex_styles = {
        {0x00000010L, L"WS_EX_ACCEPTFILES"sv},
        {0x00040000L, L"WS_EX_APPWINDOW"sv},
        {0x00000200L, L"WS_EX_CLIENTEDGE"sv},
        {0x02000000L, L"WS_EX_COMPOSITED"sv},
        {0x00000400L, L"WS_EX_CONTEXTHELP"sv},
        {0x00010000L, L"WS_EX_CONTROLPARENT"sv},
        {0x00000001L, L"WS_EX_DLGMODALFRAME"sv},
        {0x00080000L, L"WS_EX_LAYERED"sv},
        {0x00400000L, L"WS_EX_LAYOUTRTL"sv},
        {0x00000000L, L"WS_EX_LEFT"sv},
        {0x00004000L, L"WS_EX_LEFTSCROLLBAR"sv},
        {0x00000000L, L"WS_EX_LTRREADING"sv},
        {0x00000040L, L"WS_EX_MDICHILD"sv},
        {0x08000000L, L"WS_EX_NOACTIVATE"sv},
        {0x00100000L, L"WS_EX_NOINHERITLAYOUT"sv},
        {0x00000004L, L"WS_EX_NOPARENTNOTIFY"sv},
        {0x00200000L, L"WS_EX_NOREDIRECTIONBITMAP"sv},
        {(WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE), L"WS_EX_OVERLAPPEDWINDOW"sv},
        {(WS_EX_WINDOWEDGE | WS_EX_TOOLWINDOW | WS_EX_TOPMOST), L"WS_EX_PALETTEWINDOW"sv},
        {0x00001000L, L"WS_EX_RIGHT"sv},
        {0x00000000L, L"WS_EX_RIGHTSCROLLBAR"sv},
        {0x00002000L, L"WS_EX_RTLREADING"sv},
        {0x00020000L, L"WS_EX_STATICEDGE"sv},
        {0x00000080L, L"WS_EX_TOOLWINDOW"sv},
        {0x00000008L, L"WS_EX_TOPMOST"sv},
        {0x00000020L, L"WS_EX_TRANSPARENT"sv},
        {0x00000100L, L"WS_EX_WINDOWEDGE"sv},
    };

    auto match_styles = [](const auto& map, LONG style) {
        for (auto&& [key, val] : map) {
            if ((key & style) == key) std::wprintf(L"    %ls\n", val.data());
        }
    };

    std::printf("Styles:\n");
    match_styles(styles, GetWindowLong(window, GWL_STYLE));

    std::printf("Extended Styles:\n");
    match_styles(ex_styles, GetWindowLong(window, GWL_EXSTYLE));
}

BOOL CALLBACK MatchWindowPrintStyle(HWND window, LPARAM param) {
    auto search_title = std::wstring_view(reinterpret_cast<wchar_t*>(param));

    TCHAR title_buf[buflen];
    int len = GetWindowTextW(window, (LPWSTR)title_buf, buflen);
    if (len == 0) {
        debug("Title len 0, skipping...");
        return TRUE;
    }
    std::wstring_view title{(LPWSTR)title_buf, static_cast<std::size_t>(len)};

    if (title.find(search_title) != std::wstring_view::npos) {
        std::wprintf(L"Matched \"%.*ls\":\n", static_cast<unsigned int>(title.length()), title.data());
        StyleDecompose(window);
        std::putwchar(L'\n');
    } else {
        std::wprintf(L"Did not match \"%ls\"\n", title.data());
        StyleDecompose(window);
        std::putwchar(L'\n');
    }

    return TRUE;
}

BOOL CALLBACK ListWindowTraits(HWND window, LPARAM) {
    // Attributes
    LONG style = GetWindowLong(window, GWL_STYLE);
    if (style == 0) {
        debug("Style 0, skipping...");
        return TRUE;
    }

    LONG exStyle = GetWindowLong(window, GWL_EXSTYLE);
    if (exStyle == 0) {
        debug("ExStyle 0, skipping...");
        return TRUE;
    }

    TCHAR title_buf[buflen];
    int len = GetWindowText(window, title_buf, buflen);
    if (len == 0) {
        debug("Title len 0, skipping...");
        return TRUE;
    }
    // std::wstring_view title{ title_buf, static_cast<std::size_t>(len) };

    TCHAR class_buf[buflen];
    len = GetClassName(window, class_buf, buflen);
    if (len == 0) {
        debug("Class len 0, skipping...");
        return TRUE;
    }
    // std::wstring_view class_name{ class_buf, static_cast<std::size_t>(len) };

    std::wprintf(L"Title: %hs\n"
                 L"Class: %hs\n"
                 L"Style: 0x%08X, ExStyle: 0x%08X\n"
                 L"Breakdown:\n",
                 title_buf, class_buf, style, exStyle);
    StyleDecompose(window);
    std::putwchar(L'\n');

    return TRUE;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        EnumWindows(ListWindowTraits, (LPARAM)0);
    } else {
        std::printf("Searching for windows with \"%s\"\n", argv[1]);
        int wargc;
        LPWSTR* wargv = CommandLineToArgvW(GetCommandLineW(), &wargc);
        if (wargv == NULL) {
            exit(1);
        }
        EnumWindows(MatchWindowPrintStyle, reinterpret_cast<LPARAM>(wargv[1]));
        LocalFree(wargv);
    }
    return 0;
}
