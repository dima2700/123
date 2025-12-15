// grup.cpp : Определяет точку входа для приложения.

#include "framework.h"
#include "grup.h"
#include <vector>
#include <string>
#include <algorithm> // для std::shuffle
#include <random>    // для генератора случайных чисел
#include <chrono>    // для зерна генератора

#define MAX_LOADSTRING 100

// ==========================================
// СТРУКТУРЫ И ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ
// ==========================================

enum FlagType {
    FLAG_FRANCE, FLAG_GERMANY, FLAG_JAPAN, FLAG_ITALY, FLAG_UKRAINE,
    FLAG_POLAND, FLAG_AUSTRIA, FLAG_BELGIUM, FLAG_SWEDEN, FLAG_FINLAND
};

struct CountryLevel {
    std::wstring name;
    std::wstring description;
    FlagType flagType;
};

std::vector<CountryLevel> levels;
int currentLevelIndex = 0;
int score = 0;
bool gameFinished = false;
bool gameLost = false;

// Элементы управления
HWND hEdit;
HWND hButton;
HWND hRestartButton; // Новая кнопка

HINSTANCE hInst;
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];

// Прототипы
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
void InitGameData();
void ResetGame(); // Функция сброса игры
void DrawFlag(HDC hdc, FlagType type, int x, int y, int width, int height);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    InitGameData(); // Заполняем список стран (один раз)

    // Перемешивание происходит внутри InitInstance -> ResetGame, 
    // но нам нужно инициализировать окна сначала.

    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_GRUP, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    if (!InitInstance(hInstance, nCmdShow)) return FALSE;

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_GRUP));
    MSG msg;

    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    return (int)msg.wParam;
}

// Заполняем базу данных стран
void InitGameData() {
    levels.push_back({ L"Франция", L"Страна Эйфелевой башни, Лувра и высокой моды.", FLAG_FRANCE });
    levels.push_back({ L"Япония", L"Страна восходящего солнца и передовых технологий.", FLAG_JAPAN });
    levels.push_back({ L"Германия", L"Экономический двигатель Европы, родина Баха и Бетховена.", FLAG_GERMANY });
    levels.push_back({ L"Италия", L"Страна в форме сапога, родина Римской империи.", FLAG_ITALY });
    levels.push_back({ L"Украина", L"Крупнейшая страна, полностью находящаяся в Европе.", FLAG_UKRAINE });
    levels.push_back({ L"Польша", L"Страна в центре Европы, столица — Варшава.", FLAG_POLAND });
    levels.push_back({ L"Австрия", L"Родина Моцарта и вальса, страна в Альпах.", FLAG_AUSTRIA });
    levels.push_back({ L"Бельгия", L"Страна шоколада, вафель и штаб-квартиры Евросоюза.", FLAG_BELGIUM });
    levels.push_back({ L"Швеция", L"Скандинавская страна, родина группы ABBA и магазина IKEA.", FLAG_SWEDEN });
    levels.push_back({ L"Финляндия", L"Страна тысячи озер и родина Санта-Клауса.", FLAG_FINLAND });
}

// Функция сброса игры в начало
void ResetGame() {
    score = 0;
    currentLevelIndex = 0;
    gameFinished = false;
    gameLost = false;

    // Перемешиваем уровни
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::shuffle(levels.begin(), levels.end(), std::default_random_engine(seed));

    // Сбрасываем интерфейс
    SetWindowText(hEdit, L"");
    EnableWindow(hEdit, TRUE);
    EnableWindow(hButton, TRUE);
    ShowWindow(hRestartButton, SW_HIDE); // Прячем кнопку рестарта
    SetFocus(hEdit); // Ставим курсор в поле ввода
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GRUP));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_GRUP);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance;
    HWND hWnd = CreateWindowW(szWindowClass, L"Напиши страну по описанию и флагу", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, 600, 500, nullptr, nullptr, hInstance, nullptr);
    if (!hWnd) return FALSE;

    // Сразу запускаем подготовку игры
    ResetGame();

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    return TRUE;
}

// Утилита для рисования
void FillRectRGB(HDC hdc, int l, int t, int r, int b, COLORREF color) {
    HBRUSH brush = CreateSolidBrush(color);
    RECT rect = { l, t, r, b };
    FillRect(hdc, &rect, brush);
    DeleteObject(brush);
}

void DrawFlag(HDC hdc, FlagType type, int x, int y, int w, int h) {
    Rectangle(hdc, x - 1, y - 1, x + w + 1, y + h + 1);
    switch (type) {
    case FLAG_FRANCE:
        FillRectRGB(hdc, x, y, x + w / 3, y + h, RGB(0, 85, 164));
        FillRectRGB(hdc, x + w / 3, y, x + 2 * w / 3, y + h, RGB(255, 255, 255));
        FillRectRGB(hdc, x + 2 * w / 3, y, x + w, y + h, RGB(239, 65, 53));
        break;
    case FLAG_ITALY:
        FillRectRGB(hdc, x, y, x + w / 3, y + h, RGB(0, 146, 70));
        FillRectRGB(hdc, x + w / 3, y, x + 2 * w / 3, y + h, RGB(255, 255, 255));
        FillRectRGB(hdc, x + 2 * w / 3, y, x + w, y + h, RGB(206, 43, 55));
        break;
    case FLAG_BELGIUM:
        FillRectRGB(hdc, x, y, x + w / 3, y + h, RGB(0, 0, 0));
        FillRectRGB(hdc, x + w / 3, y, x + 2 * w / 3, y + h, RGB(255, 233, 54));
        FillRectRGB(hdc, x + 2 * w / 3, y, x + w, y + h, RGB(239, 51, 64));
        break;
    case FLAG_GERMANY:
        FillRectRGB(hdc, x, y, x + w, y + h / 3, RGB(0, 0, 0));
        FillRectRGB(hdc, x, y + h / 3, x + w, y + 2 * h / 3, RGB(221, 0, 0));
        FillRectRGB(hdc, x, y + 2 * h / 3, x + w, y + h, RGB(255, 206, 0));
        break;
    case FLAG_AUSTRIA:
        FillRectRGB(hdc, x, y, x + w, y + h / 3, RGB(237, 41, 57));
        FillRectRGB(hdc, x, y + h / 3, x + w, y + 2 * h / 3, RGB(255, 255, 255));
        FillRectRGB(hdc, x, y + 2 * h / 3, x + w, y + h, RGB(237, 41, 57));
        break;
    case FLAG_UKRAINE:
        FillRectRGB(hdc, x, y, x + w, y + h / 2, RGB(0, 87, 183));
        FillRectRGB(hdc, x, y + h / 2, x + w, y + h, RGB(255, 215, 0));
        break;
    case FLAG_POLAND:
        FillRectRGB(hdc, x, y, x + w, y + h / 2, RGB(255, 255, 255));
        FillRectRGB(hdc, x, y + h / 2, x + w, y + h, RGB(220, 20, 60));
        break;
    case FLAG_JAPAN:
        FillRectRGB(hdc, x, y, x + w, y + h, RGB(255, 255, 255));
        {
            HBRUSH red = CreateSolidBrush(RGB(188, 0, 45));
            HBRUSH old = (HBRUSH)SelectObject(hdc, red);
            HPEN nullPen = CreatePen(PS_NULL, 0, 0);
            HPEN oldPen = (HPEN)SelectObject(hdc, nullPen);
            int r = h / 3;
            Ellipse(hdc, x + w / 2 - r, y + h / 2 - r, x + w / 2 + r, y + h / 2 + r);
            SelectObject(hdc, old); SelectObject(hdc, oldPen);
            DeleteObject(red); DeleteObject(nullPen);
        }
        break;
    case FLAG_SWEDEN:
        FillRectRGB(hdc, x, y, x + w, y + h, RGB(0, 106, 167));
        FillRectRGB(hdc, x + w / 3 - 10, y, x + w / 3 + 10, y + h, RGB(254, 204, 0));
        FillRectRGB(hdc, x, y + h / 2 - 10, x + w, y + h / 2 + 10, RGB(254, 204, 0));
        break;
    case FLAG_FINLAND:
        FillRectRGB(hdc, x, y, x + w, y + h, RGB(255, 255, 255));
        FillRectRGB(hdc, x + w / 3 - 10, y, x + w / 3 + 10, y + h, RGB(0, 53, 128));
        FillRectRGB(hdc, x, y + h / 2 - 10, x + w, y + h / 2 + 10, RGB(0, 53, 128));
        break;
    }
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
    {
        HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

        hEdit = CreateWindowW(L"EDIT", L"",
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
            50, 300, 200, 25, hWnd, (HMENU)101, hInst, nullptr);
        SendMessage(hEdit, WM_SETFONT, (WPARAM)hFont, TRUE);

        hButton = CreateWindowW(L"BUTTON", L"Ответить",
            WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
            260, 300, 100, 25, hWnd, (HMENU)102, hInst, nullptr);
        SendMessage(hButton, WM_SETFONT, (WPARAM)hFont, TRUE);

        // КНОПКА ПЕРЕЗАПУСКА (Скрыта при старте)
        hRestartButton = CreateWindowW(L"BUTTON", L"ИГРАТЬ ЗАНОВО",
            WS_CHILD | BS_PUSHBUTTON, // Нет флага WS_VISIBLE
            200, 250, 150, 40, hWnd, (HMENU)103, hInst, nullptr);
        SendMessage(hRestartButton, WM_SETFONT, (WPARAM)hFont, TRUE);
    }
    break;

    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);

        // Кнопка "Ответить" (ID 102)
        if (wmId == 102 && !gameFinished)
        {
            WCHAR buffer[100];
            GetWindowTextW(hEdit, buffer, 100);
            std::wstring userAnswer = buffer;

            if (lstrcmpiW(userAnswer.c_str(), levels[currentLevelIndex].name.c_str()) == 0)
            {
                score += 10;
                currentLevelIndex++;
                SetWindowText(hEdit, L"");

                if (currentLevelIndex >= levels.size()) {
                    gameFinished = true;
                    // Победа
                    EnableWindow(hEdit, FALSE);
                    EnableWindow(hButton, FALSE);
                    ShowWindow(hRestartButton, SW_SHOW); // Показать рестарт
                }
                else {
                    MessageBox(hWnd, L"Верно! Следующий вопрос.", L"Успех", MB_OK);
                }
            }
            else
            {
                // Проигрыш
                gameFinished = true;
                gameLost = true;

                EnableWindow(hEdit, FALSE);
                EnableWindow(hButton, FALSE);
                ShowWindow(hRestartButton, SW_SHOW); // Показать рестарт
            }
            InvalidateRect(hWnd, nullptr, TRUE);
        }

        // Кнопка "Играть заново" (ID 103)
        else if (wmId == 103) {
            ResetGame();
            InvalidateRect(hWnd, nullptr, TRUE); // Полная перерисовка
        }

        switch (wmId)
        {
        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        }
    }
    break;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        if (gameFinished) {
            std::wstring finalMsg;
            std::wstring answerMsg = L"";

            if (gameLost) {
                SetTextColor(hdc, RGB(200, 0, 0));
                finalMsg = L"ВЫ ПРОИГРАЛИ";
                answerMsg = L"Правильный ответ был: " + levels[currentLevelIndex].name;
            }
            else {
                SetTextColor(hdc, RGB(0, 150, 0));
                finalMsg = L"ПОБЕДА!";
                answerMsg = L"Вы прошли все уровни.";
            }
            std::wstring scoreMsg = L"ВАШ СЧЕТ: " + std::to_wstring(score);

            HFONT hLargeFont = CreateFont(30, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Arial");
            SelectObject(hdc, hLargeFont);

            TextOutW(hdc, 150, 100, finalMsg.c_str(), finalMsg.length());
            TextOutW(hdc, 150, 140, scoreMsg.c_str(), scoreMsg.length());

            DeleteObject(hLargeFont);

            SetTextColor(hdc, RGB(0, 0, 0));
            TextOutW(hdc, 100, 190, answerMsg.c_str(), answerMsg.length());
        }
        else {
            std::wstring scoreText = L"Уровень: " + std::to_wstring(currentLevelIndex + 1) +
                L" / " + std::to_wstring(levels.size()) +
                L" | Очки: " + std::to_wstring(score);
            TextOutW(hdc, 50, 20, scoreText.c_str(), scoreText.length());

            RECT textRect = { 50, 60, 550, 120 };
            DrawTextW(hdc, levels[currentLevelIndex].description.c_str(), -1, &textRect, DT_WORDBREAK | DT_LEFT);

            DrawFlag(hdc, levels[currentLevelIndex].flagType, 50, 130, 200, 120);

            TextOutW(hdc, 50, 280, L"Введите название страны:", 24);
        }

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

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG: return (INT_PTR)TRUE;
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
