#include <windows.h>
#include <windowsx.h>
#include <vector>
#include <string>
#include <random>
#include <algorithm>
#include <ctime>
#include <iostream>
#include <clocale>

using namespace std;

// ================== СТРУКТУРЫ ==================
struct Country {
    wstring name;
    int x = 0; // 0–100
    int y = 0; // 0–100
};

struct Option {
    wstring text;
    bool isCorrect;
};

// ================== ГЛОБАЛЬНЫЕ ==================
HINSTANCE g_hInst;
vector<Country> g_countries;
Country g_currentCountry;
vector<Option> g_options;
mt19937 g_rng;

HWND g_buttons[4];
const int BTN_IDS[4] = { 1001,1002,1003,1004 };

HBITMAP g_hMapBmp = nullptr;
int g_mapBmpWidth = 0;
int g_mapBmpHeight = 0;

int g_score = 0;
bool g_debugMode = true;

// очередь стран, которые ещё НЕ угаданы правильно
vector<int> g_remaining;
bool g_gameOver = false;

// Сообщение в окне (зелёное/красное/финальное)
wstring g_feedbackText = L"";
COLORREF g_feedbackColor = RGB(0, 0, 0);
const UINT_PTR FEEDBACK_TIMER_ID = 1;
const UINT FEEDBACK_MS = 900;

// ================== ПРОТОТИПЫ ==================
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void InitCountries();
void InitRemainingQueue();
void StartNewRound(HWND hwnd);
void EndGame(HWND hwnd);

void DrawAll(HWND hwnd, HDC hdc);

// ================== MAIN ==================
int main() {
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
    setlocale(LC_ALL, "Russian");

    g_hInst = GetModuleHandle(nullptr);
    g_rng = mt19937((unsigned)time(nullptr));

    InitCountries();
    InitRemainingQueue();

    const wchar_t CLASS_NAME[] = L"GeoTrainer";

    WNDCLASSEX wc{};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = g_hInst;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = CLASS_NAME;
    RegisterClassEx(&wc);

    HWND hwnd = CreateWindowExW(
        0, CLASS_NAME,
        L"Геотренажёр — найди страну по точке",
        WS_OVERLAPPEDWINDOW ^ WS_MAXIMIZEBOX ^ WS_THICKFRAME,
        100, 100, 900, 600,
        nullptr, nullptr, g_hInst, nullptr
    );

    ShowWindow(hwnd, SW_SHOW);

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}

// ================== СТРАНЫ ==================
void InitCountries() {
    g_countries = {
        // Европа
        {L"Франция",43,24},{L"Германия",45,20},{L"Италия",46,27},
        {L"Испания",41,28},{L"Великобритания",42,20},
        {L"Польша",48,20},{L"Украина",51,22},

        // Россия и СНГ
        {L"Россия",65,14},{L"Казахстан",61,24},

        // Африка
        {L"Египет",51,39},{L"ЮАР",50,79},{L"Нигерия",45,51},

        // Азия
        {L"Китай",72,33},{L"Япония",84,32},{L"Индия",66,42},
        {L"Турция",52,29},{L"Южная Корея",80,31},

        // Америка
        {L"США",12,30},{L"Канада",13,16},{L"Мексика",10,40},
        {L"Бразилия",25,62},{L"Аргентина",23,81},

        // Австралия
        {L"Австралия",83,76}
    };
}

void InitRemainingQueue() {
    g_remaining.clear();
    g_remaining.reserve(g_countries.size());
    for (int i = 0; i < (int)g_countries.size(); ++i) g_remaining.push_back(i);
    shuffle(g_remaining.begin(), g_remaining.end(), g_rng);
    g_gameOver = false;
}

// ================== РАУНД / КОНЕЦ ИГРЫ ==================
void StartNewRound(HWND hwnd) {
    if (g_gameOver) return;

    if (g_remaining.empty()) {
        EndGame(hwnd);
        return;
    }

    // Берём следующую страну из очереди
    int idx = g_remaining.front();
    g_remaining.erase(g_remaining.begin());
    g_currentCountry = g_countries[idx];

    // Составляем варианты: 1 правильный + 3 случайных других
    vector<int> pool;
    pool.reserve(g_countries.size() - 1);
    for (int i = 0; i < (int)g_countries.size(); ++i)
        if (i != idx) pool.push_back(i);
    shuffle(pool.begin(), pool.end(), g_rng);

    g_options.clear();
    g_options.reserve(4);
    g_options.push_back({ g_currentCountry.name, true });
    for (int i = 0; i < 3; ++i)
        g_options.push_back({ g_countries[pool[i]].name, false });

    shuffle(g_options.begin(), g_options.end(), g_rng);

    for (int i = 0; i < 4; i++) {
        SetWindowTextW(g_buttons[i], g_options[i].text.c_str());
        EnableWindow(g_buttons[i], TRUE);
    }

    InvalidateRect(hwnd, nullptr, TRUE);
}

void EndGame(HWND hwnd) {
    g_gameOver = true;

    // отключаем кнопки
    for (int i = 0; i < 4; i++) {
        EnableWindow(g_buttons[i], FALSE);
    }

    g_feedbackText = L"ВСЕ СТРАНЫ УГАДАНЫ! ИГРА ОКОНЧЕНА";
    g_feedbackColor = RGB(0, 140, 0);

    InvalidateRect(hwnd, nullptr, TRUE);
}

// ================== РИСОВАНИЕ ==================
void DrawAll(HWND hwnd, HDC hdc) {
    RECT rc;
    GetClientRect(hwnd, &rc);

    int mapL = 20, mapT = 20;
    int mapR = rc.right - 220;
    int mapB = rc.bottom - 20;

    // Карта
    if (g_hMapBmp) {
        HDC mem = CreateCompatibleDC(hdc);
        HBITMAP old = (HBITMAP)SelectObject(mem, g_hMapBmp);
        StretchBlt(hdc, mapL, mapT, mapR - mapL, mapB - mapT,
            mem, 0, 0, g_mapBmpWidth, g_mapBmpHeight, SRCCOPY);
        SelectObject(mem, old);
        DeleteDC(mem);
    }
    else {
        Rectangle(hdc, mapL, mapT, mapR, mapB);
    }

    // Точка (если игра не закончена — по текущей стране; если закончена — остаётся последняя)
    int x = mapL + g_currentCountry.x * (mapR - mapL) / 100;
    int y = mapT + g_currentCountry.y * (mapB - mapT) / 100;

    HBRUSH b = CreateSolidBrush(RGB(255, 0, 0));
    HBRUSH oldB = (HBRUSH)SelectObject(hdc, b);
    Ellipse(hdc, x - 6, y - 6, x + 6, y + 6);
    SelectObject(hdc, oldB);
    DeleteObject(b);

    // Очки + осталось стран
    wstring scoreText = L"Очки: " + to_wstring(g_score);
    TextOutW(hdc, mapR + 10, 30, scoreText.c_str(), (int)scoreText.size());

    wstring leftText = L"Осталось: " + to_wstring((int)g_remaining.size());
    TextOutW(hdc, mapR + 10, 55, leftText.c_str(), (int)leftText.size());

    // Надпись результата (зелёная/красная/финальная)
    if (!g_feedbackText.empty()) {
        SetTextColor(hdc, g_feedbackColor);
        SetBkMode(hdc, TRANSPARENT);

        LOGFONTW lf{};
        lf.lfHeight = 26;
        lf.lfWeight = FW_BOLD;
        wcscpy_s(lf.lfFaceName, L"Segoe UI");
        HFONT hFont = CreateFontIndirectW(&lf);
        HFONT oldFont = (HFONT)SelectObject(hdc, hFont);

        TextOutW(hdc, mapR + 10, 110, g_feedbackText.c_str(), (int)g_feedbackText.size());

        SelectObject(hdc, oldFont);
        DeleteObject(hFont);

        SetTextColor(hdc, RGB(0, 0, 0));
        SetBkMode(hdc, OPAQUE);
    }
}

// ================== ОКНО ==================
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM w, LPARAM l) {
    switch (msg) {
    case WM_CREATE: {
        for (int i = 0; i < 4; i++) {
            g_buttons[i] = CreateWindowW(
                L"BUTTON", L"...",
                WS_CHILD | WS_VISIBLE,
                660, 170 + i * 55, 200, 45,
                hwnd, (HMENU)BTN_IDS[i], g_hInst, nullptr
            );
        }

        g_hMapBmp = (HBITMAP)LoadImageW(nullptr, L"map.bmp",
            IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

        if (g_hMapBmp) {
            BITMAP bm;
            GetObject(g_hMapBmp, sizeof(bm), &bm);
            g_mapBmpWidth = bm.bmWidth;
            g_mapBmpHeight = bm.bmHeight;
        }
        else {
            MessageBoxW(hwnd, L"Не удалось загрузить map.bmp (положи рядом с exe)", L"Ошибка", MB_OK | MB_ICONERROR);
        }

        StartNewRound(hwnd);
    } break;

    case WM_COMMAND: {
        if (g_gameOver) break;

        for (int i = 0; i < 4; i++) {
            if (LOWORD(w) == BTN_IDS[i]) {

                if (g_options[i].isCorrect) {
                    g_score += 10;
                    g_feedbackText = L"ПРАВИЛЬНО! +10";
                    g_feedbackColor = RGB(0, 170, 0);
                    // страна угадана -> НЕ возвращаем в очередь
                }
                else {
                    g_score -= 10;
                    g_feedbackText = L"НЕПРАВИЛЬНО! -10";
                    g_feedbackColor = RGB(200, 0, 0);

                    // ошибка -> возвращаем текущую страну обратно в конец очереди
                    // (чтобы её снова спросило позже)
                    // найдём индекс текущей страны
                    for (int k = 0; k < (int)g_countries.size(); ++k) {
                        if (g_countries[k].name == g_currentCountry.name) {
                            g_remaining.push_back(k);
                            break;
                        }
                    }
                }

                // показываем надпись некоторое время, потом скрываем
                KillTimer(hwnd, FEEDBACK_TIMER_ID);
                SetTimer(hwnd, FEEDBACK_TIMER_ID, FEEDBACK_MS, nullptr);

                // Следующая страна / конец игры
                StartNewRound(hwnd);
                break;
            }
        }
    } break;

    case WM_TIMER:
        if (w == FEEDBACK_TIMER_ID) {
            KillTimer(hwnd, FEEDBACK_TIMER_ID);
            if (!g_gameOver) g_feedbackText = L"";
            InvalidateRect(hwnd, nullptr, TRUE);
        }
        break;

    case WM_LBUTTONDOWN: {
        if (!g_debugMode) break;

        int mx = GET_X_LPARAM(l);
        int my = GET_Y_LPARAM(l);

        RECT rc;
        GetClientRect(hwnd, &rc);
        int mapL = 20, mapT = 20;
        int mapR = rc.right - 220;
        int mapB = rc.bottom - 20;

        if (mx >= mapL && mx <= mapR && my >= mapT && my <= mapB) {
            int px = (mx - mapL) * 100 / max(1, (mapR - mapL));
            int py = (my - mapT) * 100 / max(1, (mapB - mapT));
            wstring msgText = L"X=" + to_wstring(px) + L"  Y=" + to_wstring(py);
            MessageBoxW(hwnd, msgText.c_str(), L"Координаты", MB_OK);
        }
    } break;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        DrawAll(hwnd, hdc);
        EndPaint(hwnd, &ps);
    } break;

    case WM_DESTROY:
        if (g_hMapBmp) DeleteObject(g_hMapBmp);
        PostQuitMessage(0);
        break;
    }
    return DefWindowProc(hwnd, msg, w, l);
}
