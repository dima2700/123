#include <windows.h>
#include <windowsx.h>
#include <vector>
#include <string>
#include <algorithm>
#include <random>
#include <chrono>
#include <ctime>

using namespace std;
#pragma warning(disable : 4996)

// =========================================================
// ГЛОБАЛЬНЫЕ НАСТРОЙКИ
// =========================================================
HINSTANCE hInst;
WCHAR szTitle[] = L"Сборник игр";
WCHAR szWindowClass[] = L"MyGameApp";

// ФИКСИРОВАННЫЕ РАЗМЕРЫ (Как в оригинале у друга: 900x600)
const int WIN_W = 900;
const int WIN_H = 600;

// Состояния приложения
enum AppState {
    STATE_MENU,
    STATE_GAME1,
    STATE_GAME2,
    STATE_GAME3
};

AppState currentState = STATE_MENU;

// Кнопки ГЛАВНОГО МЕНЮ
HWND hBtnMenuGame1, hBtnMenuGame2, hBtnMenuGame3;

#define ID_BTN_MENU_1   1
#define ID_BTN_MENU_2   2
#define ID_BTN_MENU_3   3
#define ID_BTN_BACK     4 

// =========================================================
// 1. КЛАСС ИГРЫ "ВИКТОРИНА (ФЛАГИ)"
// =========================================================
class GeoGame {
public:
    enum FlagType {
        FLAG_FRANCE, FLAG_GERMANY, FLAG_JAPAN, FLAG_ITALY, FLAG_UKRAINE,
        FLAG_POLAND, FLAG_AUSTRIA, FLAG_BELGIUM, FLAG_SWEDEN, FLAG_FINLAND
    };

    struct CountryLevel {
        wstring name;
        wstring description;
        FlagType flagType;
    };

    static void Init(HWND hWnd) {
        HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
        int cx = WIN_W / 2; // Центр

        // Координаты подогнаны под высоту 600
        hEdit = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
            cx - 105, 430, 200, 25, hWnd, (HMENU)101, hInst, nullptr);

        hButton = CreateWindowW(L"BUTTON", L"Ответить", WS_CHILD | BS_DEFPUSHBUTTON,
            cx + 105, 430, 100, 25, hWnd, (HMENU)102, hInst, nullptr);

        hRestartButton = CreateWindowW(L"BUTTON", L"ИГРАТЬ ЗАНОВО", WS_CHILD | BS_PUSHBUTTON,
            cx - 75, 380, 150, 40, hWnd, (HMENU)103, hInst, nullptr);

        hBackButton = CreateWindowW(L"BUTTON", L"Меню", WS_CHILD | BS_PUSHBUTTON,
            10, 10, 80, 30, hWnd, (HMENU)ID_BTN_BACK, hInst, nullptr);

        SendMessage(hEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hButton, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hRestartButton, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hBackButton, WM_SETFONT, (WPARAM)hFont, TRUE);

        InitData();
    }

    static void SetVisible(bool visible) {
        int cmd = visible ? SW_SHOW : SW_HIDE;
        if (visible) Reset();

        ShowWindow(hEdit, cmd);
        ShowWindow(hButton, cmd);
        ShowWindow(hBackButton, cmd);
        if (!visible) ShowWindow(hRestartButton, SW_HIDE);
    }

    static void OnPaint(HDC hdc) {
        int cx = WIN_W / 2;
        if (gameFinished) {
            wstring msg = gameLost ? L"ВЫ ПРОИГРАЛИ" : L"ПОБЕДА!";
            SetTextColor(hdc, gameLost ? RGB(200, 0, 0) : RGB(0, 150, 0));
            TextOutW(hdc, cx - 40, 150, msg.c_str(), msg.length());
            SetTextColor(hdc, RGB(0, 0, 0));
            if (gameLost) {
                wstring ans = L"Ответ: " + levels[currentLevelIndex].name;
                TextOutW(hdc, cx - 60, 180, ans.c_str(), ans.length());
            }
        }
        else {
            // Статистика
            wstring s = L"Уровень: " + to_wstring(currentLevelIndex + 1) + L" | Очки: " + to_wstring(score);
            TextOutW(hdc, cx - 60, 30, s.c_str(), s.length());

            // Описание
            RECT r = { cx - 300, 60, cx + 300, 130 };
            DrawTextW(hdc, levels[currentLevelIndex].description.c_str(), -1, &r, DT_CENTER | DT_WORDBREAK);

            // Флаг (300x200, чуть выше, y=140)
            DrawFlag(hdc, levels[currentLevelIndex].flagType, cx - 150, 140, 300, 200);

            TextOutW(hdc, cx - 105, 400, L"Введите страну:", 15);
        }
    }

    static void OnCommand(HWND hWnd, int id) {
        if (id == 102 && !gameFinished) {
            WCHAR buf[100]; GetWindowTextW(hEdit, buf, 100);
            if (lstrcmpiW(buf, levels[currentLevelIndex].name.c_str()) == 0) {
                score += 10; currentLevelIndex++; SetWindowText(hEdit, L"");
                if (currentLevelIndex >= levels.size()) Finish(true);
                else MessageBox(hWnd, L"Верно!", L"Успех", MB_OK);
            }
            else Finish(false);
            InvalidateRect(hWnd, nullptr, TRUE);
        }
        else if (id == 103) {
            Reset(); InvalidateRect(hWnd, nullptr, TRUE);
        }
    }

private:
    static vector<CountryLevel> levels;
    static int currentLevelIndex;
    static int score;
    static bool gameFinished;
    static bool gameLost;
    static HWND hEdit, hButton, hRestartButton, hBackButton;

    static void InitData() {
        if (!levels.empty()) return;
        levels = {
            { L"Франция", L"Страна Эйфелевой башни.", FLAG_FRANCE },
            { L"Япония", L"Страна восходящего солнца.", FLAG_JAPAN },
            { L"Германия", L"Родина Бетховена.", FLAG_GERMANY },
            { L"Италия", L"Родина пиццы.", FLAG_ITALY },
            { L"Украина", L"Крупнейшая страна в Европе.", FLAG_UKRAINE },
            { L"Польша", L"Столица — Варшава.", FLAG_POLAND },
            { L"Австрия", L"Родина Моцарта.", FLAG_AUSTRIA },
            { L"Бельгия", L"Страна шоколада.", FLAG_BELGIUM },
            { L"Швеция", L"Родина IKEA.", FLAG_SWEDEN },
            { L"Финляндия", L"Родина Санта-Клауса.", FLAG_FINLAND }
        };
    }

    static void Reset() {
        score = 0; currentLevelIndex = 0; gameFinished = false; gameLost = false;
        unsigned seed = (unsigned)chrono::system_clock::now().time_since_epoch().count();
        shuffle(levels.begin(), levels.end(), default_random_engine(seed));
        SetWindowText(hEdit, L""); EnableWindow(hEdit, TRUE); EnableWindow(hButton, TRUE);
        ShowWindow(hRestartButton, SW_HIDE); SetFocus(hEdit);
    }

    static void Finish(bool win) {
        gameFinished = true; gameLost = !win;
        EnableWindow(hEdit, FALSE); EnableWindow(hButton, FALSE);
        ShowWindow(hRestartButton, SW_SHOW);
    }

    static void FillRectRGB(HDC hdc, int l, int t, int r, int b, COLORREF color) {
        HBRUSH brush = CreateSolidBrush(color); RECT rect = { l, t, r, b }; FillRect(hdc, &rect, brush); DeleteObject(brush);
    }

    static void DrawFlag(HDC hdc, FlagType type, int x, int y, int w, int h) {
        Rectangle(hdc, x - 1, y - 1, x + w + 1, y + h + 1);
        switch (type) {
        case FLAG_FRANCE: FillRectRGB(hdc, x, y, x + w / 3, y + h, RGB(0, 85, 164)); FillRectRGB(hdc, x + w / 3, y, x + 2 * w / 3, y + h, RGB(255, 255, 255)); FillRectRGB(hdc, x + 2 * w / 3, y, x + w, y + h, RGB(239, 65, 53)); break;
        case FLAG_ITALY: FillRectRGB(hdc, x, y, x + w / 3, y + h, RGB(0, 146, 70)); FillRectRGB(hdc, x + w / 3, y, x + 2 * w / 3, y + h, RGB(255, 255, 255)); FillRectRGB(hdc, x + 2 * w / 3, y, x + w, y + h, RGB(206, 43, 55)); break;
        case FLAG_BELGIUM: FillRectRGB(hdc, x, y, x + w / 3, y + h, RGB(0, 0, 0)); FillRectRGB(hdc, x + w / 3, y, x + 2 * w / 3, y + h, RGB(255, 233, 54)); FillRectRGB(hdc, x + 2 * w / 3, y, x + w, y + h, RGB(239, 51, 64)); break;
        case FLAG_GERMANY: FillRectRGB(hdc, x, y, x + w, y + h / 3, RGB(0, 0, 0)); FillRectRGB(hdc, x, y + h / 3, x + w, y + 2 * h / 3, RGB(221, 0, 0)); FillRectRGB(hdc, x, y + 2 * h / 3, x + w, y + h, RGB(255, 206, 0)); break;
        case FLAG_AUSTRIA: FillRectRGB(hdc, x, y, x + w, y + h / 3, RGB(237, 41, 57)); FillRectRGB(hdc, x, y + h / 3, x + w, y + 2 * h / 3, RGB(255, 255, 255)); FillRectRGB(hdc, x, y + 2 * h / 3, x + w, y + h, RGB(237, 41, 57)); break;
        case FLAG_UKRAINE: FillRectRGB(hdc, x, y, x + w, y + h / 2, RGB(0, 87, 183)); FillRectRGB(hdc, x, y + h / 2, x + w, y + h, RGB(255, 215, 0)); break;
        case FLAG_POLAND: FillRectRGB(hdc, x, y, x + w, y + h / 2, RGB(255, 255, 255)); FillRectRGB(hdc, x, y + h / 2, x + w, y + h, RGB(220, 20, 60)); break;
        case FLAG_SWEDEN: FillRectRGB(hdc, x, y, x + w, y + h, RGB(0, 106, 167)); FillRectRGB(hdc, x + w / 3 - 10, y, x + w / 3 + 10, y + h, RGB(254, 204, 0)); FillRectRGB(hdc, x, y + h / 2 - 10, x + w, y + h / 2 + 10, RGB(254, 204, 0)); break;
        case FLAG_FINLAND: FillRectRGB(hdc, x, y, x + w, y + h, RGB(255, 255, 255)); FillRectRGB(hdc, x + w / 3 - 10, y, x + w / 3 + 10, y + h, RGB(0, 53, 128)); FillRectRGB(hdc, x, y + h / 2 - 10, x + w, y + h / 2 + 10, RGB(0, 53, 128)); break;
        case FLAG_JAPAN: FillRectRGB(hdc, x, y, x + w, y + h, RGB(255, 255, 255)); { HBRUSH b = CreateSolidBrush(RGB(188, 0, 45)); HBRUSH o = (HBRUSH)SelectObject(hdc, b); HPEN p = CreatePen(PS_NULL, 0, 0); HPEN op = (HPEN)SelectObject(hdc, p); int r = h / 3; Ellipse(hdc, x + w / 2 - r, y + h / 2 - r, x + w / 2 + r, y + h / 2 + r); SelectObject(hdc, o); SelectObject(hdc, op); DeleteObject(b); DeleteObject(p); } break;
        }
    }
};
vector<GeoGame::CountryLevel> GeoGame::levels;
int GeoGame::currentLevelIndex = 0, GeoGame::score = 0;
bool GeoGame::gameFinished = false, GeoGame::gameLost = false;
HWND GeoGame::hEdit = NULL, GeoGame::hButton = NULL, GeoGame::hRestartButton = NULL, GeoGame::hBackButton = NULL;


// =========================================================
// 2. КЛАСС ИГРЫ "ГЕОТРЕНАЖЁР"
// =========================================================
class GameTwo {
public:
    struct Country { wstring name; int x, y; };
    struct Option { wstring text; bool isCorrect; };

    static vector<Country> g_countries;
    static Country g_currentCountry;
    static vector<Option> g_options;
    static mt19937 g_rng;
    static HWND g_buttons[4];
    static const int BTN_IDS[4];
    static HBITMAP g_hMapBmp;
    static int g_mapBmpWidth, g_mapBmpHeight;
    static int g_score;
    static bool g_debugMode;
    static vector<int> g_remaining;
    static bool g_gameOver;
    static wstring g_feedbackText;
    static COLORREF g_feedbackColor;
    static const UINT_PTR FEEDBACK_TIMER_ID = 1;
    static HWND hBackButton;

    static void Init(HWND hWnd) {
        hBackButton = CreateWindowW(L"BUTTON", L"Меню", WS_CHILD | BS_PUSHBUTTON,
            10, 10, 80, 30, hWnd, (HMENU)ID_BTN_BACK, hInst, nullptr);

        HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
        SendMessage(hBackButton, WM_SETFONT, (WPARAM)hFont, TRUE);

        for (int i = 0; i < 4; i++) {
            g_buttons[i] = CreateWindowW(L"BUTTON", L"...", WS_CHILD | WS_VISIBLE,
                660, 170 + i * 55, 200, 45, hWnd, (HMENU)BTN_IDS[i], hInst, nullptr);
            SendMessage(g_buttons[i], WM_SETFONT, (WPARAM)hFont, TRUE);
        }

        g_hMapBmp = (HBITMAP)LoadImageW(nullptr, L"map.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
        if (g_hMapBmp) {
            BITMAP bm; GetObject(g_hMapBmp, sizeof(bm), &bm);
            g_mapBmpWidth = bm.bmWidth; g_mapBmpHeight = bm.bmHeight;
        }

        g_rng.seed((unsigned)time(nullptr));
        InitCountries(); InitRemainingQueue(); StartNewRound(hWnd);
    }

    static void SetVisible(bool visible) {
        int cmd = visible ? SW_SHOW : SW_HIDE;
        ShowWindow(hBackButton, cmd);
        for (int i = 0; i < 4; i++) ShowWindow(g_buttons[i], cmd);
        if (visible) InvalidateRect(GetParent(hBackButton), nullptr, TRUE);
    }

    static void InitCountries() {
        if (!g_countries.empty()) return;
        g_countries = {
            {L"Франция",43,24},{L"Германия",45,20},{L"Италия",46,27},
            {L"Испания",41,28},{L"Великобритания",42,20},{L"Польша",48,20},{L"Украина",51,22},
            {L"Россия",65,14},{L"Казахстан",61,24},{L"Египет",51,39},{L"ЮАР",50,79},{L"Нигерия",45,51},
            {L"Китай",72,33},{L"Япония",84,32},{L"Индия",66,42},{L"Турция",52,29},{L"Южная Корея",80,31},
            {L"США",12,30},{L"Канада",13,16},{L"Мексика",10,40},{L"Бразилия",25,62},{L"Аргентина",23,81},
            {L"Австралия",83,76}
        };
    }

    static void InitRemainingQueue() {
        g_remaining.clear();
        for (int i = 0; i < (int)g_countries.size(); ++i) g_remaining.push_back(i);
        shuffle(g_remaining.begin(), g_remaining.end(), g_rng);
        g_score = 0; g_gameOver = false; g_feedbackText = L"";
    }

    static void StartNewRound(HWND hwnd) {
        if (g_gameOver) return;
        if (g_remaining.empty()) { EndGame(hwnd); return; }

        int idx = g_remaining.front();
        g_remaining.erase(g_remaining.begin());
        g_currentCountry = g_countries[idx];

        vector<int> pool;
        for (int i = 0; i < (int)g_countries.size(); ++i) if (i != idx) pool.push_back(i);
        shuffle(pool.begin(), pool.end(), g_rng);

        g_options.clear();
        g_options.push_back({ g_currentCountry.name, true });
        for (int i = 0; i < 3; ++i) g_options.push_back({ g_countries[pool[i]].name, false });
        shuffle(g_options.begin(), g_options.end(), g_rng);

        for (int i = 0; i < 4; i++) {
            SetWindowTextW(g_buttons[i], g_options[i].text.c_str());
            EnableWindow(g_buttons[i], TRUE);
        }
        InvalidateRect(hwnd, nullptr, TRUE);
    }

    static void EndGame(HWND hwnd) {
        g_gameOver = true;
        for (int i = 0; i < 4; i++) EnableWindow(g_buttons[i], FALSE);
        g_feedbackText = L"ВСЕ СТРАНЫ УГАДАНЫ! ИГРА ОКОНЧЕНА";
        g_feedbackColor = RGB(0, 140, 0);
        InvalidateRect(hwnd, nullptr, TRUE);
    }

    static void OnCommand(HWND hWnd, int id) {
        if (g_gameOver) return;
        for (int i = 0; i < 4; i++) {
            if (id == BTN_IDS[i]) {
                if (g_options[i].isCorrect) {
                    g_score += 10;
                    g_feedbackText = L"ПРАВИЛЬНО! +10";
                    g_feedbackColor = RGB(0, 170, 0);
                }
                else {
                    g_score -= 10;
                    g_feedbackText = L"НЕПРАВИЛЬНО! -10";
                    g_feedbackColor = RGB(200, 0, 0);
                    for (int k = 0; k < (int)g_countries.size(); ++k) {
                        if (g_countries[k].name == g_currentCountry.name) {
                            g_remaining.push_back(k); break;
                        }
                    }
                }
                SetTimer(hWnd, FEEDBACK_TIMER_ID, 900, nullptr);
                StartNewRound(hWnd);
                break;
            }
        }
    }

    static void OnTimer(HWND hWnd) {
        KillTimer(hWnd, FEEDBACK_TIMER_ID);
        if (!g_gameOver) g_feedbackText = L"";
        InvalidateRect(hWnd, nullptr, TRUE);
    }

    static void OnLButtonDown(HWND hwnd, int mx, int my) {
        if (!g_debugMode) return;
        RECT rc; GetClientRect(hwnd, &rc);
        int mapL = 20, mapT = 20;
        int mapR = rc.right - 220;
        int mapB = rc.bottom - 20;
        if (mx >= mapL && mx <= mapR && my >= mapT && my <= mapB) {
            int px = (mx - mapL) * 100 / max(1, (mapR - mapL));
            int py = (my - mapT) * 100 / max(1, (mapB - mapT));
            wstring msg = L"X=" + to_wstring(px) + L" Y=" + to_wstring(py);
            MessageBoxW(hwnd, msg.c_str(), L"Координаты", MB_OK);
        }
    }

    static void DrawAll(HWND hwnd, HDC hdc) {
        RECT rc; GetClientRect(hwnd, &rc);
        int mapL = 20, mapT = 20;
        int mapR = rc.right - 220;
        int mapB = rc.bottom - 20;

        if (g_hMapBmp) {
            HDC mem = CreateCompatibleDC(hdc);
            HBITMAP old = (HBITMAP)SelectObject(mem, g_hMapBmp);
            StretchBlt(hdc, mapL, mapT, mapR - mapL, mapB - mapT,
                mem, 0, 0, g_mapBmpWidth, g_mapBmpHeight, SRCCOPY);
            SelectObject(mem, old); DeleteDC(mem);
        }
        else {
            Rectangle(hdc, mapL, mapT, mapR, mapB);
            TextOutW(hdc, mapL + 100, mapT + 100, L"Нет map.bmp (положите файл рядом с exe)", 39);
        }

        if (!g_gameOver) {
            int x = mapL + g_currentCountry.x * (mapR - mapL) / 100;
            int y = mapT + g_currentCountry.y * (mapB - mapT) / 100;
            HBRUSH b = CreateSolidBrush(RGB(255, 0, 0));
            HBRUSH oldB = (HBRUSH)SelectObject(hdc, b);
            Ellipse(hdc, x - 6, y - 6, x + 6, y + 6);
            SelectObject(hdc, oldB); DeleteObject(b);
        }

        wstring scoreText = L"Очки: " + to_wstring(g_score);
        TextOutW(hdc, mapR + 10, 30, scoreText.c_str(), scoreText.size());
        wstring leftText = L"Осталось: " + to_wstring((int)g_remaining.size());
        TextOutW(hdc, mapR + 10, 55, leftText.c_str(), leftText.size());

        if (!g_feedbackText.empty()) {
            SetTextColor(hdc, g_feedbackColor);
            SetBkMode(hdc, TRANSPARENT);
            TextOutW(hdc, mapR + 10, 110, g_feedbackText.c_str(), g_feedbackText.size());
            SetTextColor(hdc, RGB(0, 0, 0));
            SetBkMode(hdc, OPAQUE);
        }
    }
};

vector<GameTwo::Country> GameTwo::g_countries;
GameTwo::Country GameTwo::g_currentCountry;
vector<GameTwo::Option> GameTwo::g_options;
mt19937 GameTwo::g_rng;
HWND GameTwo::g_buttons[4];
const int GameTwo::BTN_IDS[4] = { 1001, 1002, 1003, 1004 };
HBITMAP GameTwo::g_hMapBmp = nullptr;
int GameTwo::g_mapBmpWidth = 0, GameTwo::g_mapBmpHeight = 0;
int GameTwo::g_score = 0;
bool GameTwo::g_debugMode = true;
vector<int> GameTwo::g_remaining;
bool GameTwo::g_gameOver = false;
wstring GameTwo::g_feedbackText;
COLORREF GameTwo::g_feedbackColor;
HWND GameTwo::hBackButton = nullptr;


// =========================================================
// 3. ПУСТОЙ КЛАСС ИГРЫ №3
// =========================================================
class GameThree {
public:
    static void Init(HWND hWnd) {
        hBackButton = CreateWindowW(L"BUTTON", L"Меню", WS_CHILD | BS_PUSHBUTTON,
            10, 10, 80, 30, hWnd, (HMENU)ID_BTN_BACK, hInst, nullptr);
        HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
        SendMessage(hBackButton, WM_SETFONT, (WPARAM)hFont, TRUE);
    }
    static void SetVisible(bool visible) {
        ShowWindow(hBackButton, visible ? SW_SHOW : SW_HIDE);
    }
    static void OnPaint(HDC hdc) {
        TextOutW(hdc, 100, 100, L"ЭТО ТРЕТЬЯ ИГРА", 15);
    }
    static void OnCommand(HWND hWnd, int id) {}
private:
    static HWND hBackButton;
};
HWND GameThree::hBackButton = nullptr;


// =========================================================
// ОСНОВНАЯ ЛОГИКА (MAIN)
// =========================================================

void SetMenuVisible(bool visible) {
    int cmd = visible ? SW_SHOW : SW_HIDE;
    ShowWindow(hBtnMenuGame1, cmd);
    ShowWindow(hBtnMenuGame2, cmd);
    ShowWindow(hBtnMenuGame3, cmd);
}

void ChangeState(AppState newState, HWND hWnd) {
    if (currentState == STATE_MENU) SetMenuVisible(false);
    else if (currentState == STATE_GAME1) GeoGame::SetVisible(false);
    else if (currentState == STATE_GAME2) GameTwo::SetVisible(false);
    else if (currentState == STATE_GAME3) GameThree::SetVisible(false);

    currentState = newState;

    if (currentState == STATE_MENU) SetMenuVisible(true);
    else if (currentState == STATE_GAME1) GeoGame::SetVisible(true);
    else if (currentState == STATE_GAME2) GameTwo::SetVisible(true);
    else if (currentState == STATE_GAME3) GameThree::SetVisible(true);

    InvalidateRect(hWnd, nullptr, TRUE);
}

ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE, _In_ LPWSTR, _In_ int nCmdShow) {
    MyRegisterClass(hInstance);
    if (!InitInstance(hInstance, nCmdShow)) return FALSE;
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance) {
    WNDCLASSEXW wcex; wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0; wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);
    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow) {
    hInst = hInstance;
    // Создаем СТАТИЧНОЕ окно 900x600 (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX запрещает растягивание)
    HWND hWnd = CreateWindowW(szWindowClass, szTitle,
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE,
        CW_USEDEFAULT, 0, WIN_W, WIN_H, nullptr, nullptr, hInstance, nullptr);
    if (!hWnd) return FALSE;
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE:
    {
        HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
        int cx = WIN_W / 2 - 100;

        hBtnMenuGame1 = CreateWindowW(L"BUTTON", L"Назови страну по флагу", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, cx, 200, 200, 50, hWnd, (HMENU)ID_BTN_MENU_1, hInst, nullptr);
        hBtnMenuGame2 = CreateWindowW(L"BUTTON", L"Угадай страну по карте", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, cx, 270, 200, 50, hWnd, (HMENU)ID_BTN_MENU_2, hInst, nullptr);
        hBtnMenuGame3 = CreateWindowW(L"BUTTON", L"Игра 3 (Пусто)", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, cx, 340, 200, 50, hWnd, (HMENU)ID_BTN_MENU_3, hInst, nullptr);

        SendMessage(hBtnMenuGame1, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hBtnMenuGame2, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hBtnMenuGame3, WM_SETFONT, (WPARAM)hFont, TRUE);

        GeoGame::Init(hWnd);
        GameTwo::Init(hWnd);
        GameThree::Init(hWnd);

        GeoGame::SetVisible(false);
        GameTwo::SetVisible(false);
        GameThree::SetVisible(false);
    }
    break;

    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        if (wmId == ID_BTN_MENU_1) ChangeState(STATE_GAME1, hWnd);
        else if (wmId == ID_BTN_MENU_2) ChangeState(STATE_GAME2, hWnd);
        else if (wmId == ID_BTN_MENU_3) ChangeState(STATE_GAME3, hWnd);
        else if (wmId == ID_BTN_BACK) ChangeState(STATE_MENU, hWnd);
        else if (currentState == STATE_GAME1) GeoGame::OnCommand(hWnd, wmId);
        else if (currentState == STATE_GAME2) GameTwo::OnCommand(hWnd, wmId);
    }
    break;

    case WM_LBUTTONDOWN:
        if (currentState == STATE_GAME2) {
            GameTwo::OnLButtonDown(hWnd, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        }
        break;

    case WM_TIMER:
        if (currentState == STATE_GAME2 && wParam == GameTwo::FEEDBACK_TIMER_ID) {
            GameTwo::OnTimer(hWnd);
        }
        break;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        if (currentState == STATE_MENU) {
            HFONT hTitleFont = CreateFont(40, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Arial");
            SelectObject(hdc, hTitleFont);
            SetTextColor(hdc, RGB(0, 50, 150));
            SetBkMode(hdc, TRANSPARENT);
            TextOutW(hdc, WIN_W / 2 - 120, 100, L"ВЫБОР ИГРЫ", 10);
            DeleteObject(hTitleFont);
        }
        else if (currentState == STATE_GAME1) GeoGame::OnPaint(hdc);
        else if (currentState == STATE_GAME2) GameTwo::DrawAll(hWnd, hdc);
        else if (currentState == STATE_GAME3) GameThree::OnPaint(hdc);
        EndPaint(hWnd, &ps);
    }
    break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}