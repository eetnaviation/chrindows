#include <windows.h>
#include <string>
#include <filesystem>
#include <thread> // For sleep_for in C++

#define MOVE_RIGHT -1
#define MOVE_LEFT  1

using namespace std::chrono;

void SetScreenColor(HWND hwnd, COLORREF color);
void ExecuteHiddenCommand(const char* command);

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_SYSKEYDOWN: // Handle Alt + F4
            if (wParam == VK_F4 && GetAsyncKeyState(VK_MENU)) {
                return 0; // Prevent Alt + F4
            }
        break;
        case WM_DESTROY:
            PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void SetScreenColor(HWND hwnd, COLORREF color) {
    HDC hdc = GetDC(hwnd);
    RECT rect;
    GetClientRect(hwnd, &rect);
    HBRUSH brush = CreateSolidBrush(color);
    FillRect(hdc, &rect, brush);
    DeleteObject(brush);
    ReleaseDC(hwnd, hdc);
}

void ExecuteHiddenCommand(const char* command) {
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    // No need to set dwFlags and wShowWindow because we will use CREATE_NO_WINDOW

    ZeroMemory(&pi, sizeof(pi));

    // Start the process
    if (!CreateProcess(
            NULL,               // No module name (use command line)
            (LPSTR)command,     // Command line
            NULL,               // Process handle not inheritable
            NULL,               // Thread handle not inheritable
            FALSE,              // Set handle inheritance to FALSE
            CREATE_NO_WINDOW,   // No window for this process
            NULL,               // Use parent's environment block
            NULL,               // Use parent's starting directory
            &si,                // Pointer to STARTUPINFO structure
            &pi)                // Pointer to PROCESS_INFORMATION structure
        ) {
        // Handle error if needed
        MessageBox(NULL, "Failed to execute command", "Error", MB_OK | MB_ICONERROR);
        return; // Exit the function if command execution fails
        }

    // Wait until child process exits.
    WaitForSingleObject(pi.hProcess, INFINITE);

    // Close process and thread handles.
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}

void shakeWindow(int moveDirection, HWND hWindow) {
    RECT windowRect;

    // Get the current window position
    GetWindowRect(hWindow, &windowRect);

    // Move the window along the X-axis
    LONG moveAxisX = windowRect.left;

    if (moveDirection == MOVE_RIGHT) {
        moveAxisX += 10; // Move 10 pixels to the right
    }
    if (moveDirection == MOVE_LEFT) {
        moveAxisX -= 10; // Move 10 pixels to the left
    }

    // Set the new window position (only moving on X-axis, keeping Y-axis same)
    SetWindowPos(hWindow, NULL, moveAxisX, windowRect.top, 0, 0, SWP_NOSIZE);
}

BOOL CALLBACK EnumWindowsProc(HWND hWindow, LPARAM lParam) {
    // Ignore invisible or non-visible windows
    if (IsWindowVisible(hWindow)) {
        shakeWindow(*(int*)lParam, hWindow);
    }
    return TRUE;
}

int main(int argc, char** argv) {
    HWND window;
    AllocConsole();
    window = FindWindowA("ConsoleWindowClass", NULL);
    ShowWindow(window, 0);
    HINSTANCE hInstance = GetModuleHandle(NULL);

    int currentHoverDirection = MOVE_LEFT;

    auto start_time = steady_clock::now();
    auto next_function1_time = steady_clock::now();

    while (true) {
        auto current_time = steady_clock::now();
        if (duration_cast<milliseconds>(current_time - next_function1_time).count() >= 35) {
            EnumWindows(EnumWindowsProc, (LPARAM)&currentHoverDirection);
            currentHoverDirection = (currentHoverDirection == MOVE_LEFT) ? MOVE_RIGHT : MOVE_LEFT;
            next_function1_time = current_time;
        }

        if (duration_cast<seconds>(current_time - start_time).count() >= 23) {
            HWND hwnd = GetDesktopWindow();
            COLORREF colors[] = { RGB(255, 0, 0), RGB(0, 255, 0), RGB(0, 0, 255), RGB(123, 45, 67), RGB(255, 123, 0), RGB(34, 178, 255), RGB(200, 100, 150), RGB(75, 255, 25), RGB(0, 0, 0), RGB(255, 255, 0), RGB(100, 0, 200), RGB(150, 50, 100), RGB(240, 30, 90), RGB(15, 45, 255), RGB(178, 255, 102), RGB(80, 80, 80), RGB(255, 200, 0), RGB(0, 150, 255), RGB(180, 0, 80), RGB(255, 128, 64), RGB(100, 100, 0), RGB(25, 25, 255), RGB(0, 200, 0), RGB(128, 128, 128), RGB(255, 64, 128), RGB(0, 50, 100), RGB(102, 204, 255), RGB(0, 0, 255), RGB(255, 0, 64), RGB(0, 128, 128), RGB(220, 40, 90), RGB(128, 255, 128), RGB(150, 150, 0), RGB(0, 0, 0), RGB(255, 255, 255), RGB(50, 255, 50), RGB(60, 90, 120), RGB(200, 0, 200), RGB(75, 25, 100), RGB(255, 102, 153), RGB(250, 250, 0), RGB(0, 60, 120), RGB(123, 123, 123), RGB(245, 245, 245), RGB(128, 64, 0), RGB(0, 0, 0), RGB(180, 255, 180), RGB(255, 204, 204), RGB(153, 51, 255), RGB(255, 140, 0), RGB(0, 128, 255), RGB(140, 70, 180) };
            int colorCount = sizeof(colors) / sizeof(colors[0]);

            for (int i = 0; i < 1; i++) {
                for (int j = 0; j < colorCount; j++) {
                    SetScreenColor(hwnd, colors[j]);
                }
            }
            SetScreenColor(hwnd, RGB(255, 255, 255));
            break;
        }
        std::this_thread::sleep_for(milliseconds(1));
    }
    return 0;
}
