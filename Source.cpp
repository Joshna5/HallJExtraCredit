#include <windows.h>
#include <string>

namespace {
    constexpr int WIDTH = 900;
    constexpr int HEIGHT = 600;
    constexpr int PADDLE_W = 14;
    constexpr int PADDLE_H = 100;
    constexpr int BALL_SIZE = 16;

    HWND windowHandle = nullptr;
    int leftY = (HEIGHT - PADDLE_H) / 2;
    int rightY = (HEIGHT - PADDLE_H) / 2;
    float ballX = WIDTH / 2.0f;
    float ballY = HEIGHT / 2.0f;
    float ballVX = 5.0f;
    float ballVY = 3.0f;
    int leftScore = 0;
    int rightScore = 0;
    bool gameOver = false;

    void ResetBall(int direction) {
        ballX = WIDTH / 2.0f - BALL_SIZE / 2.0f;
        ballY = HEIGHT / 2.0f - BALL_SIZE / 2.0f;
        ballVX = 5.0f * direction;
        ballVY = (GetTickCount() % 2 ? 1.0f : -1.0f) * 3.0f;
    }

    void DrawTextCentered(HDC dc, const std::wstring& text, int y, int height, COLORREF color) {
        RECT area{ 0, y, WIDTH, y + height };
        SetTextColor(dc, color);
        DrawTextW(dc, text.c_str(), -1, &area, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }

    void PaintGame(HWND hwnd) {
        PAINTSTRUCT ps{};
        HDC dc = BeginPaint(hwnd, &ps);
        RECT client{};
        GetClientRect(hwnd, &client);
        HBRUSH background = CreateSolidBrush(RGB(12, 18, 30));
        FillRect(dc, &client, background);
        DeleteObject(background);

        SetBkMode(dc, TRANSPARENT);
        HBRUSH white = CreateSolidBrush(RGB(235, 241, 250));
        RECT leftPaddle{ 30, leftY, 30 + PADDLE_W, leftY + PADDLE_H };
        RECT rightPaddle{ WIDTH - 30 - PADDLE_W, rightY, WIDTH - 30, rightY + PADDLE_H };
        RECT ball{ static_cast<int>(ballX), static_cast<int>(ballY), static_cast<int>(ballX) + BALL_SIZE, static_cast<int>(ballY) + BALL_SIZE };
        FillRect(dc, &leftPaddle, white);
        FillRect(dc, &rightPaddle, white);
        FillRect(dc, &ball, white);
        DeleteObject(white);

        HPEN dashPen = CreatePen(PS_DASH, 2, RGB(80, 94, 115));
        HGDIOBJ oldPen = SelectObject(dc, dashPen);
        MoveToEx(dc, WIDTH / 2, 0, nullptr);
        LineTo(dc, WIDTH / 2, HEIGHT);
        SelectObject(dc, oldPen);
        DeleteObject(dashPen);

        HFONT scoreFont = CreateFontW(52, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
        HGDIOBJ oldFont = SelectObject(dc, scoreFont);
        DrawTextCentered(dc, std::to_wstring(leftScore) + L"     " + std::to_wstring(rightScore), 35, 65, RGB(235, 241, 250));
        SelectObject(dc, oldFont);
        DeleteObject(scoreFont);

        HFONT infoFont = CreateFontW(19, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
        oldFont = SelectObject(dc, infoFont);
        DrawTextCentered(dc, L"W / S     LEFT PADDLE                    UP / DOWN     RIGHT PADDLE", HEIGHT - 44, 30, RGB(155, 170, 193));
        if (gameOver) {
            DrawTextCentered(dc, L"GAME OVER", HEIGHT / 2 - 48, 48, RGB(255, 208, 110));
            DrawTextCentered(dc, L"Press R to play again", HEIGHT / 2 + 2, 35, RGB(235, 241, 250));
        }
        SelectObject(dc, oldFont);
        DeleteObject(infoFont);
        EndPaint(hwnd, &ps);
    }

    void Tick() {
        if (gameOver) return;
        if (GetAsyncKeyState('W') & 0x8000) leftY -= 7;
        if (GetAsyncKeyState('S') & 0x8000) leftY += 7;
        if (GetAsyncKeyState(VK_UP) & 0x8000) rightY -= 7;
        if (GetAsyncKeyState(VK_DOWN) & 0x8000) rightY += 7;
        if (leftY < 0) leftY = 0;
        if (leftY > HEIGHT - PADDLE_H) leftY = HEIGHT - PADDLE_H;
        if (rightY < 0) rightY = 0;
        if (rightY > HEIGHT - PADDLE_H) rightY = HEIGHT - PADDLE_H;

        ballX += ballVX;
        ballY += ballVY;
        if (ballY <= 0 || ballY + BALL_SIZE >= HEIGHT) ballVY = -ballVY;

        RECT ball{ static_cast<int>(ballX), static_cast<int>(ballY), static_cast<int>(ballX) + BALL_SIZE, static_cast<int>(ballY) + BALL_SIZE };
        RECT left{ 30, leftY, 30 + PADDLE_W, leftY + PADDLE_H };
        RECT right{ WIDTH - 30 - PADDLE_W, rightY, WIDTH - 30, rightY + PADDLE_H };
        RECT overlap{};
        if (ballVX < 0 && IntersectRect(&overlap, &ball, &left)) {
            ballX = static_cast<float>(left.right);
            ballVX = -ballVX * 1.06f;
            ballVY += (ballY + BALL_SIZE / 2.0f - (leftY + PADDLE_H / 2.0f)) * 0.045f;
        }
        else if (ballVX > 0 && IntersectRect(&overlap, &ball, &right)) {
            ballX = static_cast<float>(right.left - BALL_SIZE);
            ballVX = -ballVX * 1.06f;
            ballVY += (ballY + BALL_SIZE / 2.0f - (rightY + PADDLE_H / 2.0f)) * 0.045f;
        }

        if (ballX + BALL_SIZE < 0) {
            ++rightScore;
            if (rightScore >= 5) gameOver = true;
            else ResetBall(-1);
        }
        else if (ballX > WIDTH) {
            ++leftScore;
            if (leftScore >= 5) gameOver = true;
            else ResetBall(1);
        }
        InvalidateRect(windowHandle, nullptr, FALSE);
    }

    LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
        switch (message) {
        case WM_PAINT:
            PaintGame(hwnd);
            return 0;
        case WM_ERASEBKGND:
            return 1;
        case WM_TIMER:
            Tick();
            return 0;
        case WM_KEYDOWN:
            if (wParam == 'R' && gameOver) {
                leftScore = rightScore = 0;
                leftY = rightY = (HEIGHT - PADDLE_H) / 2;
                gameOver = false;
                ResetBall(1);
                InvalidateRect(hwnd, nullptr, FALSE);
            }
            if (wParam == VK_ESCAPE) DestroyWindow(hwnd);
            return 0;
        case WM_DESTROY:
            KillTimer(hwnd, 1);
            PostQuitMessage(0);
            return 0;
        }
        return DefWindowProcW(hwnd, message, wParam, lParam);
    }
}

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int showCommand) {
    const wchar_t CLASS_NAME[] = L"SimplePongWindow";
    WNDCLASSW wc{};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = instance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
    if (!RegisterClassW(&wc)) return 1;

    RECT windowRect{ 0, 0, WIDTH, HEIGHT };
    AdjustWindowRect(&windowRect, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, FALSE);
    windowHandle = CreateWindowExW(0, CLASS_NAME, L"Pong", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top,
        nullptr, nullptr, instance, nullptr);
    if (!windowHandle) return 1;
    ShowWindow(windowHandle, showCommand);
    ResetBall(1);
    SetTimer(windowHandle, 1, 16, nullptr);

    MSG message{};
    while (GetMessageW(&message, nullptr, 0, 0) > 0) {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }
    return static_cast<int>(message.wParam);
}
