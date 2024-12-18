#define UNICODE
#define _UNICODE
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#include <memory>
#include <string>
#include <tchar.h>
#include <strsafe.h>
#include <vector>
#include <dwmapi.h>

#define IDI_ICON1 101  // 添加图标ID定义

// 常量定义
namespace Constants {
    static const UINT WM_TRAYICON = WM_USER + 1;    // 自定义托盘图标消息
    constexpr UINT ID_TRAYICON = 1;              
    constexpr UINT ID_ROTATE = 2001;             
    constexpr UINT ID_HOTKEY = 2002;             // 修改热键菜单项ID
    constexpr UINT ID_NOTIFY = 2003;             // 提示开关菜单项ID
    constexpr UINT ID_TASKBAR = 2004;            // 窗口置顶菜单项ID
    constexpr UINT ID_EXIT = 2005;               // 退出菜单项ID
    constexpr UINT ID_RESET = 2006;               // 重置窗口尺寸菜单项ID
    const TCHAR* APP_NAME = TEXT("旋转吧大喵");          
    const TCHAR* WINDOW_CLASS = TEXT("SpinningMomoClass");  
    const TCHAR* CONFIG_FILE = TEXT("config.ini");     // 配置文件名
    const TCHAR* WINDOW_SECTION = TEXT("Window");      // 窗口配置
    const TCHAR* WINDOW_TITLE = TEXT("Title");        // 窗口标题配置项
    const TCHAR* HOTKEY_SECTION = TEXT("Hotkey");      // 热键配置节名
    const TCHAR* HOTKEY_MODIFIERS = TEXT("Modifiers"); // 修饰键配置项
    const TCHAR* HOTKEY_KEY = TEXT("Key");            // 主键配置项
    const TCHAR* NOTIFY_SECTION = TEXT("Notify");      // 提示配置节名
    const TCHAR* NOTIFY_ENABLED = TEXT("Enabled");     // 提示开关配置项
    const TCHAR* TOPMOST_SECTION = TEXT("Topmost");    // 置顶配置节名
    const TCHAR* TOPMOST_ENABLED = TEXT("Enabled");    // 窗口置顶配置项
    constexpr UINT ID_RATIO_BASE = 4000;          // 比例菜单项的基础ID
    constexpr UINT ID_CONFIG = 2009;              // 打开配置文件菜单项ID
    constexpr UINT ID_QUICK_SELECT = 2007;           // 快捷选择模式菜单项ID
    const TCHAR* QUICK_SELECT_SECTION = TEXT("QuickSelect");  // 快捷选择配置节名
    const TCHAR* QUICK_SELECT_ENABLED = TEXT("Enabled");      // 快捷选择开关配置项
    const TCHAR* CUSTOM_RATIO_SECTION = TEXT("CustomRatio");  // 自定义比例配置节名
    const TCHAR* CUSTOM_RATIO_LIST = TEXT("RatioList");       // 自定义比例列表配置项
    constexpr UINT ID_WINDOW_BASE = 3000;    // 窗口选择菜单项的基础ID
    constexpr UINT ID_WINDOW_MAX = 3999;     // 窗口选择菜单项的最大ID
    constexpr size_t DEFAULT_RATIO_INDEX = 11;  // 9:16 的索引位置
    constexpr UINT ID_SIZE_BASE = 5000;          // 固定尺寸菜单项的基础ID
    constexpr UINT ID_SIZE_CUSTOM = 5999;        // 自定义尺寸菜单项ID
    const TCHAR* CUSTOM_SIZE_SECTION = TEXT("CustomSize");   // 自定义尺寸配置节名
    const TCHAR* CUSTOM_SIZE_LIST = TEXT("SizeList");        // 自定义尺寸列表配置项
}

// 添加比例结构体定义
struct AspectRatio {
    std::wstring name;     // 比例名称
    double ratio;          // 宽高比值
    
    AspectRatio(const std::wstring& n, double r) 
        : name(n), ratio(r) {}
};

// 添加固定尺寸结构体定义
struct FixedSize {
    std::wstring name;     // 显示名称
    int width;            // 宽度
    int height;           // 高度
    
    FixedSize(const std::wstring& n, int w, int h) 
        : name(n), width(w), height(h) {}
};

// 系统托盘图标管理类
class TrayIcon {
public:
    TrayIcon(HWND hwnd) : m_hwnd(hwnd) {
        m_nid.cbSize = sizeof(NOTIFYICONDATA);
        m_nid.hWnd = hwnd;
        m_nid.uID = Constants::ID_TRAYICON;
        m_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
        m_nid.uCallbackMessage = Constants::WM_TRAYICON;
        
        // 使用自定义图标
        HINSTANCE hInstance = GetModuleHandle(NULL);
        m_nid.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
        if (!m_nid.hIcon) {
            // 如果加载失败，使用系统默认图标
            m_nid.hIcon = LoadIcon(NULL, IDI_APPLICATION);
        }
        
        StringCchCopy(m_nid.szTip, _countof(m_nid.szTip), Constants::APP_NAME);
    }

    ~TrayIcon() {
        Shell_NotifyIcon(NIM_DELETE, &m_nid);
        if (m_nid.hIcon) DestroyIcon(m_nid.hIcon);
    }

    bool Create() {
        return Shell_NotifyIcon(NIM_ADD, &m_nid) != FALSE;
    }

    void ShowBalloon(const TCHAR* title, const TCHAR* message) {
        try {
            m_nid.uFlags = NIF_INFO;
            if (FAILED(StringCchCopy(m_nid.szInfoTitle, _countof(m_nid.szInfoTitle), title)) ||
                FAILED(StringCchCopy(m_nid.szInfo, _countof(m_nid.szInfo), message))) {
                // 如果复制失败，使用安全的默认消息
                StringCchCopy(m_nid.szInfoTitle, _countof(m_nid.szInfoTitle), TEXT("提示"));
                StringCchCopy(m_nid.szInfo, _countof(m_nid.szInfo), TEXT("发生错误"));
            }
            m_nid.dwInfoFlags = NIIF_INFO;
            Shell_NotifyIcon(NIM_MODIFY, &m_nid);
            m_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
        } catch (...) {
            // 确保即使出错也能恢复正常状态
            m_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
        }
    }

private:
    HWND m_hwnd;
    NOTIFYICONDATA m_nid = {0};
};

// 窗口调整器类
class WindowResizer {
public:
    // 添加查找指定标题窗口的方法
    static HWND FindGameWindow() {
        return FindWindow(NULL, TEXT("无限暖暖  "));  // 修正为正确的空格数量（两个空格）
    }

    static std::wstring TrimRight(const std::wstring& str) {
        size_t end = str.find_last_not_of(L' ');
        return (end == std::wstring::npos) ? L"" : str.substr(0, end + 1);
    }

    static bool CompareWindowTitle(const std::wstring& title1, const std::wstring& title2) {
        return TrimRight(title1) == TrimRight(title2);
    }

    static bool ResizeWindow(HWND hwnd, int width, int height, bool shouldTopmost) {
        if (!hwnd || !IsWindow(hwnd)) return false;

        int screenWidth = GetSystemMetrics(SM_CXSCREEN);
        int screenHeight = GetSystemMetrics(SM_CYSCREEN);

        // 计算屏幕中心位置
        int newLeft = (screenWidth - width) / 2;
        int newTop = (screenHeight - height) / 2;

        // 设置窗口置顶状态
        HWND insertAfter = shouldTopmost ? HWND_TOPMOST : HWND_NOTOPMOST;
        SetWindowPos(hwnd, insertAfter, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

        // 设置新的窗口大小和位置
        return SetWindowPos(hwnd, NULL, newLeft, newTop, width, height, 
                          SWP_NOZORDER | SWP_NOACTIVATE) != FALSE;
    }

    static bool ResizeWindow(HWND hwnd, const AspectRatio& ratio, bool shouldTopmost) {
        int screenWidth = GetSystemMetrics(SM_CXSCREEN);
        int screenHeight = GetSystemMetrics(SM_CYSCREEN);
        double screenRatio = static_cast<double>(screenWidth) / screenHeight;

        int newWidth, newHeight;
        double targetRatio = ratio.ratio;

        // 计算新尺寸，始终基于屏幕尺寸
        if (targetRatio > screenRatio) {
            newWidth = screenWidth;
            newHeight = static_cast<int>(newWidth / targetRatio);
        } else {
            newHeight = screenHeight;
            newWidth = static_cast<int>(newHeight * targetRatio);
        }

        return ResizeWindow(hwnd, newWidth, newHeight, shouldTopmost);
    }

    static BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
        TCHAR className[256];
        TCHAR windowText[256];
        
        if (!IsWindowVisible(hwnd)) return TRUE;
        if (!GetClassName(hwnd, className, 256)) return TRUE;
        if (!GetWindowText(hwnd, windowText, 256)) return TRUE;

        auto windows = reinterpret_cast<std::vector<std::pair<HWND, std::wstring>>*>(lParam);
        if (windowText[0] != '\0') {  // 只收集有标题的窗口
            windows->push_back({hwnd, windowText});
        }

        return TRUE;
    }

    static std::vector<std::pair<HWND, std::wstring>> GetWindows() {
        std::vector<std::pair<HWND, std::wstring>> windows;
        EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&windows));
        return windows;
    }
};

// 主应用程序类
class WindowResizerApp {
public:
    WindowResizerApp() {
        // 获取程序所在目录
        TCHAR exePath[MAX_PATH];
        GetModuleFileName(NULL, exePath, MAX_PATH);
        
        // 获取程序所在目录
        m_configPath = exePath;
        size_t lastSlash = m_configPath.find_last_of(TEXT("\\"));
        if (lastSlash != std::wstring::npos) {
            m_configPath = m_configPath.substr(0, lastSlash + 1);
        }
        m_configPath += Constants::CONFIG_FILE;

        // 加载配置
        LoadConfig();
        InitializeRatios();
        InitializeFixedSizes();
        LoadCustomRatios();
        LoadCustomSizes();
    }

    ~WindowResizerApp() {
        SaveConfig();
    }

    bool Initialize(HINSTANCE hInstance) {
        if (!RegisterWindowClass(hInstance)) return false;
        if (!CreateAppWindow(hInstance)) return false;
        
        m_trayIcon = std::make_unique<TrayIcon>(m_hwnd);
        if (!m_trayIcon->Create()) return false;

        // 注册热键
        if (!RegisterHotKey(m_hwnd, Constants::ID_TRAYICON, m_hotkeyModifiers, m_hotkeyKey)) {
            ShowNotification(Constants::APP_NAME, 
                TEXT("热键注册失败。程序仍可使用，但快捷键将不可用。"));
        }

        // 显示启动提示
        std::wstring hotkeyText = GetHotkeyText();
        std::wstring message = TEXT("窗口比例调整工具已在后台运行。\n按 ") + hotkeyText + TEXT(" 调整窗口比例。");
        ShowNotification(Constants::APP_NAME, message.c_str());

        return true;
    }

    int Run() {
        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        return (int)msg.wParam;
    }

    void ShowWindowSelectionMenu() {
        HMENU hMenu = CreatePopupMenu();
        if (!hMenu) return;

        // 窗口选择
        HMENU hWindowMenu = CreatePopupMenu();
        if (hWindowMenu) {
            auto windows = WindowResizer::GetWindows();
            int id = Constants::ID_WINDOW_BASE;
            for (const auto& window : windows) {
                UINT flags = MF_BYPOSITION | MF_STRING;
                if (WindowResizer::CompareWindowTitle(window.second, m_windowTitle)) {
                    flags |= MF_CHECKED;
                }
                InsertMenu(hWindowMenu, -1, flags, id++, window.second.c_str());
            }

            InsertMenu(hMenu, -1, MF_BYPOSITION | MF_STRING | MF_POPUP, 
                      (UINT_PTR)hWindowMenu, TEXT("选择窗口"));

            m_windows = std::move(windows);
        }

        // 比例选择
        HMENU hRatioMenu = CreatePopupMenu();
        if (hRatioMenu) {
            for (size_t i = 0; i < m_ratios.size(); ++i) {
                UINT flags = MF_BYPOSITION | MF_STRING;
                if (i == m_currentRatioIndex) {
                    flags |= MF_CHECKED;
                }
                InsertMenu(hRatioMenu, -1, flags, Constants::ID_RATIO_BASE + i, m_ratios[i].name.c_str());
            }
            
            InsertMenu(hMenu, -1, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT_PTR)hRatioMenu, TEXT("窗口比例"));
        }

        // 重置选项
        InsertMenu(hMenu, -1, MF_BYPOSITION | MF_STRING, Constants::ID_RESET, TEXT("重置窗口"));
        InsertMenu(hMenu, -1, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);

        // 操作模式设置
        InsertMenu(hMenu, -1, MF_BYPOSITION | MF_STRING | (m_quickSelectEnabled ? MF_CHECKED : 0),
                  Constants::ID_QUICK_SELECT, TEXT("快捷选择模式"));
        InsertMenu(hMenu, -1, MF_BYPOSITION | MF_STRING, Constants::ID_HOTKEY, TEXT("修改热键"));
        InsertMenu(hMenu, -1, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);

        // 其他设置
        InsertMenu(hMenu, -1, MF_BYPOSITION | MF_STRING | (m_notifyEnabled ? MF_CHECKED : 0), 
                  Constants::ID_NOTIFY, TEXT("显示操作提示"));
        InsertMenu(hMenu, -1, MF_BYPOSITION | MF_STRING | (m_topmostEnabled ? MF_CHECKED : 0),
                  Constants::ID_TASKBAR, TEXT("窗口置顶"));
        InsertMenu(hMenu, -1, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);

        // 添加打开配置文件选项
        InsertMenu(hMenu, -1, MF_BYPOSITION | MF_STRING, Constants::ID_CONFIG, TEXT("打开配置文件"));
        InsertMenu(hMenu, -1, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);

        // 退出选项
        InsertMenu(hMenu, -1, MF_BYPOSITION | MF_STRING, Constants::ID_EXIT, TEXT("退出"));

        // 显示菜单
        POINT pt;
        GetCursorPos(&pt);
        SetForegroundWindow(m_hwnd);
        TrackPopupMenu(hMenu, TPM_RIGHTALIGN | TPM_BOTTOMALIGN | TPM_RIGHTBUTTON,
                      pt.x, pt.y, 0, m_hwnd, NULL);

        DestroyMenu(hMenu);  // 这会自动销毁所有子菜单
    }

    void HandleWindowSelect(int id) {
        int index = id - Constants::ID_WINDOW_BASE;
        if (index >= 0 && index < m_windows.size()) {
            m_windowTitle = m_windows[index].second;
            SaveConfig();

            ShowNotification(Constants::APP_NAME, 
                    TEXT("已选择窗口"));
        }
    }

    void HandleRatioSelect(UINT id) {
        if (id == Constants::ID_CONFIG) {
            // 打开配置文件
            ShellExecute(NULL, TEXT("open"), TEXT("notepad.exe"), 
                        m_configPath.c_str(), NULL, SW_SHOW);
            
            // 显示提示
            ShowNotification(Constants::APP_NAME, 
                TEXT("添加自定义比例步骤：\n"
                     "1. 找到 [CustomRatio] 节\n"
                     "2. 在 RatioList 后添加比例\n"
                     "3. 保存并重启软件"));
            return;
        }

        size_t index = id - Constants::ID_RATIO_BASE;
        if (index < m_ratios.size()) {
            m_currentRatioIndex = index;
            
            HWND gameWindow = NULL;
            if (!m_windowTitle.empty()) {
                auto windows = WindowResizer::GetWindows();
                for (const auto& window : windows) {
                    if (WindowResizer::CompareWindowTitle(window.second, m_windowTitle)) {
                        gameWindow = window.first;
                        break;
                    }
                }
            }
            
            if (!gameWindow) {
                gameWindow = WindowResizer::FindGameWindow();
            }

            if (gameWindow) {
                if (WindowResizer::ResizeWindow(gameWindow, m_ratios[m_currentRatioIndex], 
                                              m_topmostEnabled)) {
                    m_windowModified = true;
                    ShowNotification(Constants::APP_NAME, 
                        TEXT("窗口比例调整成功！"), true);
                } else {
                    ShowNotification(Constants::APP_NAME, 
                        TEXT("窗口比例调整失败。可能需要管理员权限，或窗口不支持调整大小。"));
                }
            } else {
                ShowNotification(Constants::APP_NAME, 
                    TEXT("未找到目标窗口，请确保窗口已启动。"));
            }
        }
    }

    void ResizeGameWindow() {
        HWND gameWindow = NULL;
        
        // 查找目标窗口
        if (!m_windowTitle.empty()) {
            auto windows = WindowResizer::GetWindows();
            for (const auto& window : windows) {
                if (WindowResizer::CompareWindowTitle(window.second, m_windowTitle)) {
                    gameWindow = window.first;
                    break;
                }
            }
        }
        
        if (!gameWindow) {
            gameWindow = WindowResizer::FindGameWindow();
        }

        if (gameWindow) {
            // 如果窗口已修改，先重置窗口
            if (m_windowModified) {
                AspectRatio resetRatio(TEXT("重置"), 
                                     static_cast<double>(GetSystemMetrics(SM_CXSCREEN)) / 
                                     GetSystemMetrics(SM_CYSCREEN));
                if (WindowResizer::ResizeWindow(gameWindow, resetRatio, m_topmostEnabled)) {
                    m_windowModified = false;
                    ShowNotification(Constants::APP_NAME, 
                        TEXT("窗口已重置为屏幕大小。"), true);
                    return;
                }
            }

            // 应用选择的比例
            if (WindowResizer::ResizeWindow(gameWindow, m_ratios[m_currentRatioIndex], 
                                          m_topmostEnabled)) {
                m_windowModified = true;
                ShowNotification(Constants::APP_NAME, 
                    TEXT("窗口比例调整成功！"), true);
            } else {
                ShowNotification(Constants::APP_NAME, 
                    TEXT("窗口比例调整失败。可能需要管理员权限，或窗口不支持调整大小。"));
            }
        } else {
            ShowNotification(Constants::APP_NAME, 
                TEXT("未找到目标窗口，请确保窗口已启动。"));
        }
    }

    void SetHotkey() {
        // 注销现有热键
        UnregisterHotKey(m_hwnd, Constants::ID_TRAYICON);
        
        // 进入热键设置模式
        m_hotkeySettingMode = true;
        
        // 显示提示
        ShowNotification(Constants::APP_NAME, 
            TEXT("请按下新的热键组合...\n支持 Ctrl、Shift、Alt 组合其他按键"));
    }

    void ShowNotification(const TCHAR* title, const TCHAR* message, bool isSuccess = false) {
        // 如果是成功提示，则根据关控制；其他提示始终显示
        if (!isSuccess || m_notifyEnabled) {
            m_trayIcon->ShowBalloon(title, message);
        }
    }

    void ToggleNotification() {
        m_notifyEnabled = !m_notifyEnabled;
        SaveNotifyConfig();
    }

    void ToggleTopmost() {
        m_topmostEnabled = !m_topmostEnabled;
        SaveTopmostConfig();

        // 立即应用置顶状态到当前窗口
        HWND gameWindow = NULL;
        
        // 查找目标窗口
        if (!m_windowTitle.empty()) {
            auto windows = WindowResizer::GetWindows();
            for (const auto& window : windows) {
                if (WindowResizer::CompareWindowTitle(window.second, m_windowTitle)) {
                    gameWindow = window.first;
                    break;
                }
            }
        }
        
        if (!gameWindow) {
            gameWindow = WindowResizer::FindGameWindow();
        }

        if (gameWindow) {
            // 设置窗口置顶状态
            HWND insertAfter = m_topmostEnabled ? HWND_TOPMOST : HWND_NOTOPMOST;
            SetWindowPos(gameWindow, insertAfter, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
        }
    }

    bool IsTopmostEnabled() const {
        return m_topmostEnabled;
    }

    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        WindowResizerApp* app = reinterpret_cast<WindowResizerApp*>(
            GetWindowLongPtr(hwnd, GWLP_USERDATA));

        switch (msg) {
            case WM_CREATE: {
                CREATESTRUCT* cs = reinterpret_cast<CREATESTRUCT*>(lParam);
                SetWindowLongPtr(hwnd, GWLP_USERDATA, 
                               reinterpret_cast<LONG_PTR>(cs->lpCreateParams));
                return 0;
            }

            case WM_KEYDOWN: {
                // 处理热键设置
                if (app && app->m_hotkeySettingMode) {
                    // 获取修饰键状态
                    UINT modifiers = 0;
                    if (GetAsyncKeyState(VK_CONTROL) & 0x8000) modifiers |= MOD_CONTROL;
                    if (GetAsyncKeyState(VK_SHIFT) & 0x8000) modifiers |= MOD_SHIFT;
                    if (GetAsyncKeyState(VK_MENU) & 0x8000) modifiers |= MOD_ALT;

                    // 如果按下了非修饰键
                    if (wParam != VK_CONTROL && wParam != VK_SHIFT && wParam != VK_MENU) {
                        app->m_hotkeyModifiers = modifiers;
                        app->m_hotkeyKey = static_cast<UINT>(wParam);
                        app->m_hotkeySettingMode = false;

                        // 尝试注册新热键
                        if (RegisterHotKey(hwnd, Constants::ID_TRAYICON, modifiers, static_cast<UINT>(wParam))) {
                            std::wstring hotkeyText = app->GetHotkeyText();
                            std::wstring message = TEXT("热键已设置为：") + hotkeyText;
                            app->ShowNotification(Constants::APP_NAME, message.c_str());
                            // 保存新的热键配置
                            app->SaveHotkeyConfig();
                        } else {
                            // 注册失败，恢复默认热键
                            app->m_hotkeyModifiers = MOD_CONTROL | MOD_ALT;
                            app->m_hotkeyKey = 'R';
                            RegisterHotKey(hwnd, Constants::ID_TRAYICON, 
                                        app->m_hotkeyModifiers, app->m_hotkeyKey);
                            app->ShowNotification(Constants::APP_NAME, 
                                TEXT("热键设置失败，已恢复默认热键。"));
                            // 保存默认热键配置
                            app->SaveHotkeyConfig();
                        }
                    }
                }
                return 0;
            }

            case WM_COMMAND: {
                if (!app) return 0;
                WORD cmd = LOWORD(wParam);
                if (cmd >= Constants::ID_RATIO_BASE && cmd < Constants::ID_RATIO_BASE + 1000) {  // 修改：使用范围检查
                    app->HandleRatioSelect(cmd);
                } else if (cmd >= Constants::ID_SIZE_BASE && cmd <= Constants::ID_SIZE_CUSTOM) {
                    app->HandleSizeSelect(cmd);
                } else if (cmd >= Constants::ID_WINDOW_BASE && cmd <= Constants::ID_WINDOW_MAX) {
                    app->HandleWindowSelect(cmd);
                } else {
                    switch (cmd) {
                        case Constants::ID_CONFIG:  // 添加：新的配置文件处理
                            app->OpenConfigFile();
                            break;
                        case Constants::ID_HOTKEY:
                            app->SetHotkey();
                            break;
                        case Constants::ID_NOTIFY:
                            app->ToggleNotification();
                            break;
                        case Constants::ID_TASKBAR:
                            app->ToggleTopmost();
                            break;
                        case Constants::ID_EXIT:
                            DestroyWindow(hwnd);
                            break;
                        case Constants::ID_RESET:
                            app->ResetWindowSize();
                            break;
                        case Constants::ID_QUICK_SELECT:
                            app->ToggleQuickSelect();
                            break;
                    }
                }
                return 0;
            }

            case WM_USER + 1: {  // Constants::WM_TRAYICON
                if (app && (lParam == WM_RBUTTONUP || lParam == WM_LBUTTONUP)) {
                    app->ShowWindowSelectionMenu();
                }
                return 0;
            }

            case WM_HOTKEY: {
                if (app && wParam == Constants::ID_TRAYICON) {
                    if (app->IsQuickSelectEnabled()) {
                        POINT pt;
                        GetCursorPos(&pt);
                        app->ShowQuickMenu(pt);
                    } else {
                        app->ResizeGameWindow();
                    }
                }
                return 0;
            }

            case WM_DESTROY: {
                PostQuitMessage(0);
                return 0;
            }
        }
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }

private:
    bool RegisterWindowClass(HINSTANCE hInstance) {
        WNDCLASSEX wc = {0};
        wc.cbSize = sizeof(WNDCLASSEX);
        wc.lpfnWndProc = WindowProc;
        wc.hInstance = hInstance;
        wc.lpszClassName = Constants::WINDOW_CLASS;
        wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));        // 添加大图标
        wc.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));      // 添加小图标
        return RegisterClassEx(&wc) != 0;
    }

    bool CreateAppWindow(HINSTANCE hInstance) {
        m_hwnd = CreateWindow(Constants::WINDOW_CLASS, Constants::APP_NAME,
                            WS_POPUP, 0, 0, 0, 0, NULL, NULL, hInstance, this);
        return m_hwnd != NULL;
    }

    HWND m_hwnd = NULL;
    std::unique_ptr<TrayIcon> m_trayIcon;
    std::vector<std::pair<HWND, std::wstring>> m_windows;  // 存储窗口列表
    std::wstring m_configPath;                         // 配置文件完整路径
    std::wstring m_windowTitle;                        // 保存的窗口标题
    UINT m_hotkeyModifiers = MOD_CONTROL | MOD_ALT;   // 热键修饰键
    UINT m_hotkeyKey = 'R';                           // 热键主键
    bool m_hotkeySettingMode = false;                  // 是否处于热键设置模式
    bool m_notifyEnabled = false;                      // 是否显示提示，默认关闭
    bool m_topmostEnabled = false;                    // 是否窗口置顶，默认关闭
    std::vector<AspectRatio> m_ratios;
    size_t m_currentRatioIndex = Constants::DEFAULT_RATIO_INDEX;  // 使用常量初始化
    bool m_quickSelectEnabled = true;  // 快捷选择模式是否启用，默认开启
    bool m_useScreenSize = true;    // 是否使用屏幕尺寸计算，默认开启
    bool m_windowModified = false;  // 窗口是否被修改过
    std::vector<FixedSize> m_sizes;  // 固定尺寸列表

    void InitializeRatios() {
        // 横屏比例（从宽到窄）
        m_ratios.emplace_back(TEXT("32:9"), 32.0/9.0);
        m_ratios.emplace_back(TEXT("21:9"), 21.0/9.0);
        m_ratios.emplace_back(TEXT("16:9"), 16.0/9.0);
        m_ratios.emplace_back(TEXT("3:2"), 3.0/2.0);
        m_ratios.emplace_back(TEXT("1:1"), 1.0/1.0);
        
        // 竖屏比例（从窄到宽）
        m_ratios.emplace_back(TEXT("2:3"), 2.0/3.0);
        m_ratios.emplace_back(TEXT("9:16"), 9.0/16.0);
    }

    void InitializeFixedSizes() {
        // 添加预设尺寸
        m_sizes.emplace_back(TEXT("7680×4320 16:9"), 7680, 4320);
        m_sizes.emplace_back(TEXT("4320×7680 9:16"), 4320, 7680);
        m_sizes.emplace_back(TEXT("8192×5464 3:2"), 8192, 5464); 
        m_sizes.emplace_back(TEXT("5464×8192 2:3"), 5464, 8192);
    }

    void LoadConfig() {
        // 检查配置文件是否存在
        if (GetFileAttributes(m_configPath.c_str()) == INVALID_FILE_ATTRIBUTES) {
            // 创建默认配置文件
            WritePrivateProfileString(Constants::WINDOW_SECTION, Constants::WINDOW_TITLE, TEXT(""), m_configPath.c_str());
            WritePrivateProfileString(Constants::HOTKEY_SECTION, Constants::HOTKEY_MODIFIERS, TEXT("3"), m_configPath.c_str());
            WritePrivateProfileString(Constants::HOTKEY_SECTION, Constants::HOTKEY_KEY, TEXT("82"), m_configPath.c_str());
            WritePrivateProfileString(Constants::NOTIFY_SECTION, Constants::NOTIFY_ENABLED, TEXT("0"), m_configPath.c_str());
            WritePrivateProfileString(Constants::TOPMOST_SECTION, Constants::TOPMOST_ENABLED, TEXT("0"), m_configPath.c_str());
            WritePrivateProfileString(Constants::QUICK_SELECT_SECTION, Constants::QUICK_SELECT_ENABLED, TEXT("1"), m_configPath.c_str());
            WritePrivateProfileString(Constants::CUSTOM_RATIO_SECTION, Constants::CUSTOM_RATIO_LIST, TEXT(""), m_configPath.c_str());
            WritePrivateProfileString(Constants::CUSTOM_SIZE_SECTION, Constants::CUSTOM_SIZE_LIST, TEXT(""), m_configPath.c_str());
        }

        // 加载各项配置
        LoadHotkeyConfig();
        LoadWindowConfig();
        LoadNotifyConfig();
        LoadTopmostConfig();
        LoadQuickSelectConfig();
    }

    void SaveConfig() {
        SaveHotkeyConfig();
        SaveWindowConfig();
        SaveNotifyConfig();
        SaveTopmostConfig();
        SaveQuickSelectConfig();
    }

    void LoadHotkeyConfig() {
        TCHAR buffer[32];
        // 读取修饰键
        if (GetPrivateProfileString(Constants::HOTKEY_SECTION, 
                                  Constants::HOTKEY_MODIFIERS,
                                  TEXT(""), buffer, _countof(buffer),
                                  m_configPath.c_str()) > 0) {
            m_hotkeyModifiers = _wtoi(buffer);
        }

        // 读取主键
        if (GetPrivateProfileString(Constants::HOTKEY_SECTION,
                                  Constants::HOTKEY_KEY,
                                  TEXT(""), buffer, _countof(buffer),
                                  m_configPath.c_str()) > 0) {
            m_hotkeyKey = _wtoi(buffer);
        }
    }

    void SaveHotkeyConfig() {
        TCHAR buffer[32];
        // 保存修饰键
        _stprintf_s(buffer, _countof(buffer), TEXT("%u"), m_hotkeyModifiers);
        WritePrivateProfileString(Constants::HOTKEY_SECTION,
                                Constants::HOTKEY_MODIFIERS,
                                buffer, m_configPath.c_str());

        // 保存主键
        _stprintf_s(buffer, _countof(buffer), TEXT("%u"), m_hotkeyKey);
        WritePrivateProfileString(Constants::HOTKEY_SECTION,
                                Constants::HOTKEY_KEY,
                                buffer, m_configPath.c_str());
    }

    void LoadWindowConfig() {
        TCHAR buffer[256];
        if (GetPrivateProfileString(Constants::WINDOW_SECTION,
                                  Constants::WINDOW_TITLE,
                                  TEXT(""), buffer, _countof(buffer),
                                  m_configPath.c_str()) > 0) {
            m_windowTitle = buffer;
        }
    }

    void SaveWindowConfig() {
        if (!m_windowTitle.empty()) {
            WritePrivateProfileString(Constants::WINDOW_SECTION,
                                    Constants::WINDOW_TITLE,
                                    m_windowTitle.c_str(),
                                    m_configPath.c_str());
        }
    }

    void LoadNotifyConfig() {
        TCHAR buffer[32];
        if (GetPrivateProfileString(Constants::NOTIFY_SECTION,
                                  Constants::NOTIFY_ENABLED,
                                  TEXT("0"), buffer, _countof(buffer),
                                  m_configPath.c_str()) > 0) {
            m_notifyEnabled = (_wtoi(buffer) != 0);
        }
    }

    void SaveNotifyConfig() {
        WritePrivateProfileString(Constants::NOTIFY_SECTION,
                                Constants::NOTIFY_ENABLED,
                                m_notifyEnabled ? TEXT("1") : TEXT("0"),
                                m_configPath.c_str());
    }

    void LoadTopmostConfig() {
        TCHAR buffer[32];
        if (GetPrivateProfileString(Constants::TOPMOST_SECTION,
                                  Constants::TOPMOST_ENABLED,
                                  TEXT("0"), buffer, _countof(buffer),
                                  m_configPath.c_str()) > 0) {
            m_topmostEnabled = (_wtoi(buffer) != 0);
        }
    }

    void SaveTopmostConfig() {
        WritePrivateProfileString(Constants::TOPMOST_SECTION,
                                Constants::TOPMOST_ENABLED,
                                m_topmostEnabled ? TEXT("1") : TEXT("0"),
                                m_configPath.c_str());
    }

    void ResetWindowSize() {
        HWND gameWindow = FindTargetWindow();
        if (gameWindow) {
            // 创建一个比例对象用于重置为屏幕大小
            AspectRatio resetRatio(TEXT("重置"), 
                                 static_cast<double>(GetSystemMetrics(SM_CXSCREEN)) / 
                                 GetSystemMetrics(SM_CYSCREEN));
            
            if (WindowResizer::ResizeWindow(gameWindow, resetRatio, m_topmostEnabled)) {
                m_windowModified = true;  // 保持窗口修改状态的跟踪
                ShowNotification(Constants::APP_NAME, 
                    TEXT("窗口已重置为屏幕大小。"), true);
            } else {
                ShowNotification(Constants::APP_NAME, 
                    TEXT("重置窗口尺寸失败。"));
            }
        } else {
            ShowNotification(Constants::APP_NAME, 
                TEXT("未找到目标窗口，请确保窗口已启动。"));
        }
    }

    void ShowQuickMenu(const POINT& pt) {
        HMENU hMenu = CreatePopupMenu();
        if (!hMenu) return;

        // 添加比例选项
        for (size_t i = 0; i < m_ratios.size(); ++i) {
            UINT flags = MF_BYPOSITION | MF_STRING;
            if (i == m_currentRatioIndex) {
                flags |= MF_CHECKED;
            }
            InsertMenu(hMenu, -1, flags,
                      Constants::ID_RATIO_BASE + i, m_ratios[i].name.c_str());
        }

        // 添加分隔线
        InsertMenu(hMenu, -1, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);

        // 添加固定尺寸选项
        for (size_t i = 0; i < m_sizes.size(); ++i) {
            InsertMenu(hMenu, -1, MF_BYPOSITION | MF_STRING,
                      Constants::ID_SIZE_BASE + i, m_sizes[i].name.c_str());
        }

        // 添加分隔线
        InsertMenu(hMenu, -1, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);

        // 添加重置和置顶选项
        InsertMenu(hMenu, -1, MF_BYPOSITION | MF_STRING, 
                  Constants::ID_RESET, TEXT("重置窗口"));
        InsertMenu(hMenu, -1, MF_BYPOSITION | MF_STRING | (m_topmostEnabled ? MF_CHECKED : 0),
                  Constants::ID_TASKBAR, TEXT("窗口置顶"));

        // 显示菜单
        SetForegroundWindow(m_hwnd);
        TrackPopupMenu(hMenu, TPM_RIGHTALIGN | TPM_BOTTOMALIGN | TPM_RIGHTBUTTON,
                      pt.x, pt.y, 0, m_hwnd, NULL);

        DestroyMenu(hMenu);
    }

    void ToggleQuickSelect() {
        m_quickSelectEnabled = !m_quickSelectEnabled;
        SaveQuickSelectConfig();
    }

    void LoadQuickSelectConfig() {
        TCHAR buffer[32];
        if (GetPrivateProfileString(Constants::QUICK_SELECT_SECTION,
                                  Constants::QUICK_SELECT_ENABLED,
                                  TEXT("0"), buffer, _countof(buffer),
                                  m_configPath.c_str()) > 0) {
            m_quickSelectEnabled = (_wtoi(buffer) != 0);
        }
    }

    void SaveQuickSelectConfig() {
        WritePrivateProfileString(Constants::QUICK_SELECT_SECTION,
                                Constants::QUICK_SELECT_ENABLED,
                                m_quickSelectEnabled ? TEXT("1") : TEXT("0"),
                                m_configPath.c_str());
    }

    bool AddCustomRatio(const std::wstring& ratio) {
        size_t colonPos = ratio.find(TEXT(":"));
        if (colonPos == std::wstring::npos) return false;
        
        try {
            double width = std::stod(ratio.substr(0, colonPos));
            double height = std::stod(ratio.substr(colonPos + 1));
            if (height <= 0) return false;
            
            m_ratios.emplace_back(ratio, width/height);
            return true;
        } catch (...) {
            return false;
        }
    }

    void LoadCustomRatios() {
        try {
            TCHAR buffer[1024];
            if (GetPrivateProfileString(Constants::CUSTOM_RATIO_SECTION,
                                      Constants::CUSTOM_RATIO_LIST,
                                      TEXT(""), buffer, _countof(buffer),
                                      m_configPath.c_str()) > 0) {
                std::wstring ratios = buffer;
                if (ratios.empty()) return;

                bool hasError = false;
                std::wstring errorDetails;
                
                // 分割并处理每个比例
                size_t start = 0, end = 0;
                while ((end = ratios.find(TEXT(","), start)) != std::wstring::npos) {
                    std::wstring ratio = ratios.substr(start, end - start);
                    if (!AddCustomRatio(ratio)) {
                        hasError = true;
                        errorDetails += ratio + TEXT("\n");
                    }
                    start = end + 1;
                }
                
                // 处理最后一个比例
                if (start < ratios.length()) {
                    std::wstring ratio = ratios.substr(start);
                    if (!AddCustomRatio(ratio)) {
                        hasError = true;
                        errorDetails += ratio;
                    }
                }

                if (hasError) {
                    // 使用 MessageBox 而不是通知来显示错误
                    std::wstring errorMsg = TEXT("以下自定义比例格式错误：\n\n") + 
                                          errorDetails + TEXT("\n\n") +
                                          TEXT("请确保：\n") +
                                          TEXT("1. 使用英文冒号 \":\" 分隔数字\n") +
                                          TEXT("2. 使用英文逗号 \",\" 分隔多个比例\n") +
                                          TEXT("3. 只输入数字，例如：16:10,17:10");
                    
                    MessageBox(NULL, errorMsg.c_str(), Constants::APP_NAME, 
                              MB_ICONWARNING | MB_OK);
                }
            }
        } catch (...) {
            // 如果发生任何错误，显示一个通用错误消息
            MessageBox(NULL, 
                TEXT("加载自定义比例时发生错误。\n请检查配置文件格式是否正确。"), 
                Constants::APP_NAME, MB_ICONERROR | MB_OK);
        }
    }

    std::wstring GetHotkeyText() {
        std::wstring text;
        
        // 添加修饰键
        if (m_hotkeyModifiers & MOD_CONTROL) text += TEXT("Ctrl+");
        if (m_hotkeyModifiers & MOD_ALT) text += TEXT("Alt+");
        if (m_hotkeyModifiers & MOD_SHIFT) text += TEXT("Shift+");
        
        // 添加主键
        if (m_hotkeyKey >= 'A' && m_hotkeyKey <= 'Z') {
            text += static_cast<TCHAR>(m_hotkeyKey);
        } else {
            // 处理特殊键
            switch (m_hotkeyKey) {
                case VK_F1: text += TEXT("F1"); break;
                case VK_F2: text += TEXT("F2"); break;
                case VK_F3: text += TEXT("F3"); break;
                case VK_F4: text += TEXT("F4"); break;
                case VK_F5: text += TEXT("F5"); break;
                case VK_F6: text += TEXT("F6"); break;
                case VK_F7: text += TEXT("F7"); break;
                case VK_F8: text += TEXT("F8"); break;
                case VK_F9: text += TEXT("F9"); break;
                case VK_F10: text += TEXT("F10"); break;
                case VK_F11: text += TEXT("F11"); break;
                case VK_F12: text += TEXT("F12"); break;
                case VK_NUMPAD0: text += TEXT("Num0"); break;
                case VK_NUMPAD1: text += TEXT("Num1"); break;
                case VK_NUMPAD2: text += TEXT("Num2"); break;
                case VK_NUMPAD3: text += TEXT("Num3"); break;
                case VK_NUMPAD4: text += TEXT("Num4"); break;
                case VK_NUMPAD5: text += TEXT("Num5"); break;
                case VK_NUMPAD6: text += TEXT("Num6"); break;
                case VK_NUMPAD7: text += TEXT("Num7"); break;
                case VK_NUMPAD8: text += TEXT("Num8"); break;
                case VK_NUMPAD9: text += TEXT("Num9"); break;
                case VK_MULTIPLY: text += TEXT("Num*"); break;
                case VK_ADD: text += TEXT("Num+"); break;
                case VK_SUBTRACT: text += TEXT("Num-"); break;
                case VK_DECIMAL: text += TEXT("Num."); break;
                case VK_DIVIDE: text += TEXT("Num/"); break;
                default: text += static_cast<TCHAR>(m_hotkeyKey); break;
            }
        }
        
        return text;
    }

    void HandleSizeSelect(UINT id) {
        size_t index = id - Constants::ID_SIZE_BASE;
        if (index < m_sizes.size()) {
            HWND gameWindow = FindTargetWindow();
            if (gameWindow) {
                if (ResizeWindowToFixedSize(gameWindow, m_sizes[index])) {
                    m_windowModified = true;
                    ShowNotification(Constants::APP_NAME, 
                        TEXT("窗口尺寸调整成功！"), true);
                } else {
                    ShowNotification(Constants::APP_NAME, 
                        TEXT("窗口尺寸调整失败。可能需要管理员权限，或窗口不支持调整大小。"));
                }
            } else {
                ShowNotification(Constants::APP_NAME, 
                    TEXT("未找到目标窗口，请确保窗口已启动。"));
            }
        }
    }

    bool ResizeWindowToFixedSize(HWND hwnd, const FixedSize& size) {
        return WindowResizer::ResizeWindow(hwnd, size.width, size.height, m_topmostEnabled);
    }

    HWND FindTargetWindow() {
        HWND gameWindow = NULL;
        
        // 查找目标窗口
        if (!m_windowTitle.empty()) {
            auto windows = WindowResizer::GetWindows();
            for (const auto& window : windows) {
                if (WindowResizer::CompareWindowTitle(window.second, m_windowTitle)) {
                    gameWindow = window.first;
                    break;
                }
            }
        }
        
        if (!gameWindow) {
            gameWindow = WindowResizer::FindGameWindow();
        }

        return gameWindow;
    }

    bool AddCustomSize(const std::wstring& sizeStr) {
        size_t xPos = sizeStr.find(TEXT("x"));
        if (xPos == std::wstring::npos) return false;
        
        try {
            int width = std::stoi(sizeStr.substr(0, xPos));
            int height = std::stoi(sizeStr.substr(xPos + 1));
            if (width <= 0 || height <= 0) return false;
            
            m_sizes.emplace_back(sizeStr, width, height);
            return true;
        } catch (...) {
            return false;
        }
    }

    void LoadCustomSizes() {
        try {
            TCHAR buffer[1024];
            if (GetPrivateProfileString(Constants::CUSTOM_SIZE_SECTION,
                                      Constants::CUSTOM_SIZE_LIST,
                                      TEXT(""), buffer, _countof(buffer),
                                      m_configPath.c_str()) > 0) {
                std::wstring sizes = buffer;
                if (sizes.empty()) return;

                bool hasError = false;
                std::wstring errorDetails;
                
                // 分割并处理每个尺寸
                size_t start = 0, end = 0;
                while ((end = sizes.find(TEXT(","), start)) != std::wstring::npos) {
                    std::wstring size = sizes.substr(start, end - start);
                    if (!AddCustomSize(size)) {
                        hasError = true;
                        errorDetails += size + TEXT("\n");
                    }
                    start = end + 1;
                }
                
                // 处理最后一个尺寸
                if (start < sizes.length()) {
                    std::wstring size = sizes.substr(start);
                    if (!AddCustomSize(size)) {
                        hasError = true;
                        errorDetails += size;
                    }
                }

                if (hasError) {
                    std::wstring errorMsg = TEXT("以下自定义尺寸格式错误：\n\n") + 
                                          errorDetails + TEXT("\n\n") +
                                          TEXT("请确保：\n") +
                                          TEXT("1. 使用英文字母 \"x\" 分隔宽高\n") +
                                          TEXT("2. 使用英文逗号 \",\" 分隔多个尺寸\n") +
                                          TEXT("3. 只输入数字，例如：1920x1080,2560x1440");
                    
                    MessageBox(NULL, errorMsg.c_str(), Constants::APP_NAME, 
                              MB_ICONWARNING | MB_OK);
                }
            }
        } catch (...) {
            MessageBox(NULL, 
                TEXT("加载自定义尺寸时发生错误。\n请检查配置文件格式是否正确。"), 
                Constants::APP_NAME, MB_ICONERROR | MB_OK);
        }
    }

    void OpenConfigFile() {
        // 打开配置文件
        ShellExecute(NULL, TEXT("open"), TEXT("notepad.exe"), 
                    m_configPath.c_str(), NULL, SW_SHOW);
        
        // 显示提示
        ShowNotification(Constants::APP_NAME, 
            TEXT("配置文件说明：\n"
                 "1. [CustomRatio] 节用于添加自定义比例\n"
                 "2. [CustomSize] 节用于添加自定义尺寸\n"
                 "3. 保存后重启软件生效"));
    }

public:
    bool IsQuickSelectEnabled() const {
        return m_quickSelectEnabled;
    }
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WindowResizerApp app;
    
    if (!app.Initialize(hInstance)) {
        MessageBox(NULL, TEXT("应用程序初始化失败"), 
                  Constants::APP_NAME, MB_ICONERROR);
        return 1;
    }

    return app.Run();
}