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
    STATE_GAME3,
    STATE_GAME4
};

AppState currentState = STATE_MENU;

// Кнопки ГЛАВНОГО МЕНЮ
HWND hBtnMenuGame1, hBtnMenuGame2, hBtnMenuGame3, hBtnMenuGame4;

#define ID_BTN_MENU_1   1
#define ID_BTN_MENU_2   2
#define ID_BTN_MENU_3   3
#define ID_BTN_MENU_4   4
#define ID_BTN_BACK     5 

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
// 3. КЛАСС ИГРЫ "ГЕОГРАФИЧЕСКАЯ ВИКТОРИНА"
// =========================================================
class GameThree {
public:
    struct Question {
        wstring question;
        vector<wstring> answers;
        int correctAnswer;
    };

    static void Init(HWND hWnd) {
        hBackButton = CreateWindowW(L"BUTTON", L"Меню", WS_CHILD | BS_PUSHBUTTON,
            10, 10, 80, 30, hWnd, (HMENU)ID_BTN_BACK, hInst, nullptr);
        
        HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
        SendMessage(hBackButton, WM_SETFONT, (WPARAM)hFont, TRUE);

        int cx = WIN_W / 2;
        for (int i = 0; i < 4; i++) {
            hAnswerButtons[i] = CreateWindowW(L"BUTTON", L"...", WS_CHILD | BS_PUSHBUTTON,
                cx - 200, 250 + i * 60, 400, 50, hWnd, (HMENU)(2001 + i), hInst, nullptr);
            SendMessage(hAnswerButtons[i], WM_SETFONT, (WPARAM)hFont, TRUE);
        }

        hRestartButton = CreateWindowW(L"BUTTON", L"ИГРАТЬ ЗАНОВО", WS_CHILD | BS_PUSHBUTTON,
            cx - 100, 500, 200, 40, hWnd, (HMENU)2005, hInst, nullptr);
        SendMessage(hRestartButton, WM_SETFONT, (WPARAM)hFont, TRUE);

        InitQuestions();
    }

    static void SetVisible(bool visible) {
        int cmd = visible ? SW_SHOW : SW_HIDE;
        ShowWindow(hBackButton, cmd);
        for (int i = 0; i < 4; i++) ShowWindow(hAnswerButtons[i], cmd);
        if (visible) Reset();
        if (!visible) ShowWindow(hRestartButton, SW_HIDE);
    }

    static void OnPaint(HDC hdc) {
        int cx = WIN_W / 2;
        if (gameOver) {
            wstring msg = L"ИГРА ОКОНЧЕНА!";
            SetTextColor(hdc, RGB(0, 150, 0));
            TextOutW(hdc, cx - 70, 200, msg.c_str(), msg.length());
            SetTextColor(hdc, RGB(0, 0, 0));
            
            wstring scoreMsg = L"Ваш счет: " + to_wstring(score) + L" / " + to_wstring(questions.size() * 10);
            TextOutW(hdc, cx - 80, 240, scoreMsg.c_str(), scoreMsg.length());
        }
        else {
            if (currentQuestionIndex < questions.size()) {
                wstring stat = L"Вопрос " + to_wstring(currentQuestionIndex + 1) + L" из " + 
                              to_wstring(questions.size()) + L" | Очки: " + to_wstring(score);
                TextOutW(hdc, cx - 100, 30, stat.c_str(), stat.length());

                RECT r = { 50, 80, WIN_W - 50, 200 };
                DrawTextW(hdc, questions[currentQuestionIndex].question.c_str(), -1, &r, 
                         DT_CENTER | DT_WORDBREAK | DT_TOP);

                for (int i = 0; i < 4; i++) {
                    SetWindowTextW(hAnswerButtons[i], questions[currentQuestionIndex].answers[i].c_str());
                }
            }
        }
    }

    static void OnCommand(HWND hWnd, int id) {
        if (gameOver) {
            if (id == 2005) {
                Reset();
                InvalidateRect(hWnd, nullptr, TRUE);
            }
            return;
        }

        if (answered) return;

        for (int i = 0; i < 4; i++) {
            if (id == 2001 + i) {
                selectedAnswer = i;
                answered = true;
                if (i == questions[currentQuestionIndex].correctAnswer) {
                    score += 10;
                    MessageBoxW(hWnd, L"Правильно! +10 очков", L"Правильный ответ", MB_OK | MB_ICONINFORMATION);
                }
                else {
                    wstring correct = L"Неправильно! Правильный ответ: " + 
                                    questions[currentQuestionIndex].answers[questions[currentQuestionIndex].correctAnswer];
                    MessageBoxW(hWnd, correct.c_str(), L"Неправильный ответ", MB_OK | MB_ICONEXCLAMATION);
                }
                
                currentQuestionIndex++;
                if (currentQuestionIndex >= questions.size()) {
                    gameOver = true;
                    ShowWindow(hRestartButton, SW_SHOW);
                    for (int j = 0; j < 4; j++) EnableWindow(hAnswerButtons[j], FALSE);
                }
                else {
                    answered = false;
                    selectedAnswer = -1;
                }
                InvalidateRect(hWnd, nullptr, TRUE);
                break;
            }
        }
    }

private:
    static vector<Question> questions;
    static int currentQuestionIndex;
    static int score;
    static bool answered;
    static bool gameOver;
    static int selectedAnswer;
    static HWND hBackButton;
    static HWND hAnswerButtons[4];
    static HWND hRestartButton;

    static void InitQuestions() {
        if (!questions.empty()) return;
        questions = {
            { L"Какая столица Франции?", {L"Лондон", L"Париж", L"Берлин", L"Мадрид"}, 1 },
            { L"Какая самая большая страна в мире?", {L"Китай", L"США", L"Россия", L"Канада"}, 2 },
            { L"Какая столица Японии?", {L"Сеул", L"Пекин", L"Токио", L"Бангкок"}, 2 },
            { L"Какая река самая длинная в мире?", {L"Амазонка", L"Нил", L"Янцзы", L"Миссисипи"}, 1 },
            { L"Какая самая высокая гора в мире?", {L"К2", L"Эверест", L"Килиманджаро", L"Монблан"}, 1 },
            { L"Какая столица Австралии?", {L"Сидней", L"Мельбурн", L"Канберра", L"Брисбен"}, 2 },
            { L"Какая столица Бразилии?", {L"Рио-де-Жанейро", L"Сан-Паулу", L"Бразилиа", L"Сальвадор"}, 2 },
            { L"Какая столица Египта?", {L"Александрия", L"Каир", L"Гиза", L"Луксор"}, 1 },
            { L"Какая столица Канады?", {L"Торонто", L"Ванкувер", L"Оттава", L"Монреаль"}, 2 },
            { L"Какая столица Индии?", {L"Мумбаи", L"Дели", L"Калькутта", L"Бангалор"}, 1 }
        };
    }

    static void Reset() {
        score = 0;
        currentQuestionIndex = 0;
        answered = false;
        gameOver = false;
        selectedAnswer = -1;
        unsigned seed = (unsigned)chrono::system_clock::now().time_since_epoch().count();
        shuffle(questions.begin(), questions.end(), default_random_engine(seed));
        for (int i = 0; i < 4; i++) EnableWindow(hAnswerButtons[i], TRUE);
        ShowWindow(hRestartButton, SW_HIDE);
    }
};

vector<GameThree::Question> GameThree::questions;
int GameThree::currentQuestionIndex = 0;
int GameThree::score = 0;
bool GameThree::answered = false;
bool GameThree::gameOver = false;
int GameThree::selectedAnswer = -1;
HWND GameThree::hBackButton = nullptr;
HWND GameThree::hAnswerButtons[4] = { nullptr, nullptr, nullptr, nullptr };
HWND GameThree::hRestartButton = nullptr;


// =========================================================
// 4. КЛАСС ИГРЫ "ПЕРЕТАСКИВАНИЕ СТОЛИЦ"
// =========================================================
class GameFour {
public:
    struct DragTask {
        wstring country;
        wstring capital;
        int countryX, countryY;
        int capitalX, capitalY;
        bool matched;
    };

    static void Init(HWND hWnd) {
        hWndParent = hWnd;
        hBackButton = CreateWindowW(L"BUTTON", L"Меню", WS_CHILD | BS_PUSHBUTTON,
            10, 10, 80, 30, hWnd, (HMENU)ID_BTN_BACK, hInst, nullptr);
        
        HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
        SendMessage(hBackButton, WM_SETFONT, (WPARAM)hFont, TRUE);

        InitTasks();
        Reset();
    }

    static void SetVisible(bool visible) {
        int cmd = visible ? SW_SHOW : SW_HIDE;
        ShowWindow(hBackButton, cmd);
        if (visible) {
            Reset();
            InvalidateRect(hWndParent, nullptr, TRUE);
        }
    }

    static void OnPaint(HDC hdc) {
        int cx = WIN_W / 2;
        
        if (allMatched) {
            wstring msg = L"ВСЕ СТОЛИЦЫ СОПОСТАВЛЕНЫ!";
            SetTextColor(hdc, RGB(0, 150, 0));
            TextOutW(hdc, cx - 120, 250, msg.c_str(), msg.length());
            SetTextColor(hdc, RGB(0, 0, 0));
            
            wstring scoreMsg = L"Вы набрали: " + to_wstring(score) + L" из " + to_wstring(tasks.size()) + L" очков!";
            TextOutW(hdc, cx - 100, 290, scoreMsg.c_str(), scoreMsg.length());
        }
        else {
            wstring scoreMsg = L"Счет: " + to_wstring(score) + L" / " + to_wstring(tasks.size());
            TextOutW(hdc, cx - 50, 30, scoreMsg.c_str(), scoreMsg.length());

            wstring instruction = L"Перетащите столицы к соответствующим странам";
            TextOutW(hdc, cx - 180, 70, instruction.c_str(), instruction.length());

            for (size_t i = 0; i < tasks.size(); i++) {
                if (!tasks[i].matched) {
                    // Рисуем страны (слева)
                    RECT countryRect = { tasks[i].countryX, tasks[i].countryY, 
                                       tasks[i].countryX + 200, tasks[i].countryY + 40 };
                    Rectangle(hdc, countryRect.left, countryRect.top, countryRect.right, countryRect.bottom);
                    SetBkMode(hdc, TRANSPARENT);
                    DrawTextW(hdc, tasks[i].country.c_str(), -1, &countryRect, 
                            DT_CENTER | DT_VCENTER | DT_SINGLELINE);

                    // Рисуем зоны сброса
                    RECT dropRect = { tasks[i].countryX + 220, tasks[i].countryY,
                                     tasks[i].countryX + 420, tasks[i].countryY + 40 };
                    HBRUSH hBrush = CreateHatchBrush(HS_BDIAGONAL, RGB(200, 200, 200));
                    FrameRect(hdc, &dropRect, hBrush);
                    DeleteObject(hBrush);

                    // Рисуем столицы (справа)
                    int capX = (draggingIndex == (int)i && isDragging) ? dragX : tasks[i].capitalX;
                    int capY = (draggingIndex == (int)i && isDragging) ? dragY : tasks[i].capitalY;
                    
                    RECT capitalRect = { capX, capY, capX + 200, capY + 40 };
                    HBRUSH capBrush = CreateSolidBrush(RGB(255, 165, 0));
                    FillRect(hdc, &capitalRect, capBrush);
                    DeleteObject(capBrush);
                    Rectangle(hdc, capitalRect.left, capitalRect.top, capitalRect.right, capitalRect.bottom);
                    SetBkMode(hdc, TRANSPARENT);
                    DrawTextW(hdc, tasks[i].capital.c_str(), -1, &capitalRect,
                            DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                }
                else {
                    // Рисуем правильно сопоставленные
                    RECT countryRect = { tasks[i].countryX, tasks[i].countryY,
                                       tasks[i].countryX + 200, tasks[i].countryY + 40 };
                    HBRUSH matchedBrush = CreateSolidBrush(RGB(144, 238, 144));
                    FillRect(hdc, &countryRect, matchedBrush);
                    DeleteObject(matchedBrush);
                    Rectangle(hdc, countryRect.left, countryRect.top, countryRect.right, countryRect.bottom);
                    DrawTextW(hdc, tasks[i].country.c_str(), -1, &countryRect,
                            DT_CENTER | DT_VCENTER | DT_SINGLELINE);

                    RECT capitalRect = { tasks[i].countryX + 220, tasks[i].countryY,
                                       tasks[i].countryX + 420, tasks[i].countryY + 40 };
                    FillRect(hdc, &capitalRect, matchedBrush);
                    DeleteObject(matchedBrush);
                    Rectangle(hdc, capitalRect.left, capitalRect.top, capitalRect.right, capitalRect.bottom);
                    DrawTextW(hdc, tasks[i].capital.c_str(), -1, &capitalRect,
                            DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                }
            }
            SetBkMode(hdc, OPAQUE);
        }
    }

    static void OnLButtonDown(int x, int y) {
        if (allMatched) return;
        
        for (size_t i = 0; i < tasks.size(); i++) {
            if (!tasks[i].matched && 
                x >= tasks[i].capitalX && x <= tasks[i].capitalX + 200 &&
                y >= tasks[i].capitalY && y <= tasks[i].capitalY + 40) {
                isDragging = true;
                draggingIndex = (int)i;
                originalCapitalX = tasks[i].capitalX;
                originalCapitalY = tasks[i].capitalY;
                dragX = tasks[i].capitalX;
                dragY = tasks[i].capitalY;
                dragOffsetX = x - tasks[i].capitalX;
                dragOffsetY = y - tasks[i].capitalY;
                InvalidateRect(hWndParent, nullptr, TRUE);
                break;
            }
        }
    }

    static void OnMouseMove(int x, int y) {
        if (isDragging && draggingIndex >= 0) {
            dragX = x - dragOffsetX;
            dragY = y - dragOffsetY;
            InvalidateRect(hWndParent, nullptr, TRUE);
        }
    }

    static void OnLButtonUp(int x, int y) {
        if (!isDragging || draggingIndex < 0) return;

        // Проверяем, попали ли в зону сброса
        for (size_t i = 0; i < tasks.size(); i++) {
            if (!tasks[i].matched &&
                x >= tasks[i].countryX + 220 && x <= tasks[i].countryX + 420 &&
                y >= tasks[i].countryY && y <= tasks[i].countryY + 40) {
                
                // Проверяем правильность
                if (draggingIndex == (int)i) {
                    tasks[i].matched = true;
                    score++;
                    MessageBoxW(hWndParent, L"Правильно!", L"Успех", MB_OK | MB_ICONINFORMATION);
                    
                    // Проверяем, все ли сопоставлено
                    allMatched = true;
                    for (const auto& task : tasks) {
                        if (!task.matched) {
                            allMatched = false;
                            break;
                        }
                    }
                }
                else {
                    MessageBoxW(hWndParent, L"Неправильно! Попробуйте еще раз.", L"Ошибка", MB_OK | MB_ICONEXCLAMATION);
                    tasks[draggingIndex].capitalX = originalCapitalX;
                    tasks[draggingIndex].capitalY = originalCapitalY;
                }
                break;
            }
        }

        if (!tasks[draggingIndex].matched) {
            tasks[draggingIndex].capitalX = originalCapitalX;
            tasks[draggingIndex].capitalY = originalCapitalY;
        }

        isDragging = false;
        draggingIndex = -1;
        InvalidateRect(hWndParent, nullptr, TRUE);
    }

    static void OnCommand(HWND hWnd, int id) {}

private:
    static vector<DragTask> tasks;
    static int score;
    static bool allMatched;
    static bool isDragging;
    static int draggingIndex;
    static int dragX, dragY;
    static int dragOffsetX, dragOffsetY;
    static int originalCapitalX, originalCapitalY;
    static HWND hBackButton;
    static HWND hWndParent;

    static void InitTasks() {
        if (!tasks.empty()) return;
        tasks = {
            { L"Франция", L"Париж", 150, 150, 650, 150 },
            { L"Россия", L"Москва", 150, 220, 650, 220 },
            { L"Япония", L"Токио", 150, 290, 650, 290 },
            { L"Бразилия", L"Бразилиа", 150, 360, 650, 360 },
            { L"Египет", L"Каир", 150, 430, 650, 430 }
        };
    }

    static void Reset() {
        score = 0;
        allMatched = false;
        isDragging = false;
        draggingIndex = -1;
        
        // Перемешиваем позиции столиц
        vector<pair<int, int>> positions;
        for (const auto& task : tasks) {
            positions.push_back({ task.capitalX, task.capitalY });
        }
        
        unsigned seed = (unsigned)chrono::system_clock::now().time_since_epoch().count();
        shuffle(positions.begin(), positions.end(), default_random_engine(seed));
        
        for (size_t i = 0; i < tasks.size(); i++) {
            tasks[i].matched = false;
            tasks[i].capitalX = positions[i].first;
            tasks[i].capitalY = positions[i].second;
        }
    }
};

vector<GameFour::DragTask> GameFour::tasks;
int GameFour::score = 0;
bool GameFour::allMatched = false;
bool GameFour::isDragging = false;
int GameFour::draggingIndex = -1;
int GameFour::dragX = 0, GameFour::dragY = 0;
int GameFour::dragOffsetX = 0, GameFour::dragOffsetY = 0;
int GameFour::originalCapitalX = 0, GameFour::originalCapitalY = 0;
HWND GameFour::hBackButton = nullptr;
HWND GameFour::hWndParent = nullptr;


// =========================================================
// ОСНОВНАЯ ЛОГИКА (MAIN)
// =========================================================

void SetMenuVisible(bool visible) {
    int cmd = visible ? SW_SHOW : SW_HIDE;
    ShowWindow(hBtnMenuGame1, cmd);
    ShowWindow(hBtnMenuGame2, cmd);
    ShowWindow(hBtnMenuGame3, cmd);
    ShowWindow(hBtnMenuGame4, cmd);
}

void ChangeState(AppState newState, HWND hWnd) {
    if (currentState == STATE_MENU) SetMenuVisible(false);
    else if (currentState == STATE_GAME1) GeoGame::SetVisible(false);
    else if (currentState == STATE_GAME2) GameTwo::SetVisible(false);
    else if (currentState == STATE_GAME3) GameThree::SetVisible(false);
    else if (currentState == STATE_GAME4) GameFour::SetVisible(false);

    currentState = newState;

    if (currentState == STATE_MENU) SetMenuVisible(true);
    else if (currentState == STATE_GAME1) GeoGame::SetVisible(true);
    else if (currentState == STATE_GAME2) GameTwo::SetVisible(true);
    else if (currentState == STATE_GAME3) GameThree::SetVisible(true);
    else if (currentState == STATE_GAME4) GameFour::SetVisible(true);

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
        int cx = WIN_W / 2;
        int btnWidth = 250;
        int btnHeight = 45;
        int startY = 180;
        int spacing = 60;

        hBtnMenuGame1 = CreateWindowW(L"BUTTON", L"Назови страну по флагу", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 
            cx - btnWidth / 2, startY, btnWidth, btnHeight, hWnd, (HMENU)ID_BTN_MENU_1, hInst, nullptr);
        hBtnMenuGame2 = CreateWindowW(L"BUTTON", L"Угадай страну по карте", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 
            cx - btnWidth / 2, startY + spacing, btnWidth, btnHeight, hWnd, (HMENU)ID_BTN_MENU_2, hInst, nullptr);
        hBtnMenuGame3 = CreateWindowW(L"BUTTON", L"Географическая викторина", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 
            cx - btnWidth / 2, startY + spacing * 2, btnWidth, btnHeight, hWnd, (HMENU)ID_BTN_MENU_3, hInst, nullptr);
        hBtnMenuGame4 = CreateWindowW(L"BUTTON", L"Перетаскивание столиц", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 
            cx - btnWidth / 2, startY + spacing * 3, btnWidth, btnHeight, hWnd, (HMENU)ID_BTN_MENU_4, hInst, nullptr);

        SendMessage(hBtnMenuGame1, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hBtnMenuGame2, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hBtnMenuGame3, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hBtnMenuGame4, WM_SETFONT, (WPARAM)hFont, TRUE);

        GeoGame::Init(hWnd);
        GameTwo::Init(hWnd);
        GameThree::Init(hWnd);
        GameFour::Init(hWnd);

        GeoGame::SetVisible(false);
        GameTwo::SetVisible(false);
        GameThree::SetVisible(false);
        GameFour::SetVisible(false);
    }
    break;

    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        if (wmId == ID_BTN_MENU_1) ChangeState(STATE_GAME1, hWnd);
        else if (wmId == ID_BTN_MENU_2) ChangeState(STATE_GAME2, hWnd);
        else if (wmId == ID_BTN_MENU_3) ChangeState(STATE_GAME3, hWnd);
        else if (wmId == ID_BTN_MENU_4) ChangeState(STATE_GAME4, hWnd);
        else if (wmId == ID_BTN_BACK) ChangeState(STATE_MENU, hWnd);
        else if (currentState == STATE_GAME1) GeoGame::OnCommand(hWnd, wmId);
        else if (currentState == STATE_GAME2) GameTwo::OnCommand(hWnd, wmId);
        else if (currentState == STATE_GAME3) GameThree::OnCommand(hWnd, wmId);
        else if (currentState == STATE_GAME4) GameFour::OnCommand(hWnd, wmId);
    }
    break;

    case WM_LBUTTONDOWN:
        if (currentState == STATE_GAME2) {
            GameTwo::OnLButtonDown(hWnd, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        }
        else if (currentState == STATE_GAME4) {
            GameFour::OnLButtonDown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        }
        break;

    case WM_MOUSEMOVE:
        if (currentState == STATE_GAME4) {
            GameFour::OnMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        }
        break;

    case WM_LBUTTONUP:
        if (currentState == STATE_GAME4) {
            GameFour::OnLButtonUp(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
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
        else if (currentState == STATE_GAME4) GameFour::OnPaint(hdc);
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