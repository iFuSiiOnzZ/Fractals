
/*
    Project includes
*/

#include "graphics\graphics.h"
#include "fractals\julia.h"

/*
    Libs include
*/

/*
    STDLib/OS includes
*/
#include <Windows.h>
#include <stdio.h>

///////////////////////////////////////////////////////////////////////////////

#ifndef UNUSED
    #define UNUSED(x) (void)(x)
#endif

#ifndef WasDown
    #define WasDown(x) (GetAsyncKeyState(x) & 0x0001)
#endif
#ifndef IsDown
    #define IsDown(x) (GetAsyncKeyState(x) & 0x8000)
#endif

#define WINDOW_NAME "Fractals"
#define CLASS_NAME  "Fractals"

#define WINDOW_HEIGHT    720
#define WINDOW_WIDTH    1280

///////////////////////////////////////////////////////////////////////////////

typedef struct win32_offscreen_buffer_t
{
    BITMAPINFO Info;
    void *Memory;

    int Width;
    int Height;

    int Pitch;
    int BytesPerPixel;
} win32_offscreen_buffer_t;

typedef struct  window_dimensions_t
{
    int Width;
    int Height;
} window_dimensions_t;

///////////////////////////////////////////////////////////////////////////////

static win32_offscreen_buffer_t GlobalBackBuffer = { 0 };
static bool GlobalExit = false;

extern int r[256] = { 0 };
extern int g[256] = { 0 };
extern int b[256] = { 0 };

void GenerateColors()
{
    for (int i = 0; i < 256; ++i)
    {
        r[i] = (i >> 5) * 36, g[i] = (i >> 3 & 7) * 36, b[i] = (i & 3) * 85;
    }
}

///////////////////////////////////////////////////////////////////////////////

static window_dimensions_t GetWindowDimension(HWND Window)
{
    RECT ClientRect = { 0 };
    GetClientRect(Window, &ClientRect);

    window_dimensions_t r = { 0 };
    r.Width = ClientRect.right - ClientRect.left;
    r.Height = ClientRect.bottom - ClientRect.top;

    return r;
}


static void ResizeBIBSection(win32_offscreen_buffer_t *Buffer, int Width, int Height)
{
    Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
    Buffer->Info.bmiHeader.biHeight = -Height;
    Buffer->Info.bmiHeader.biWidth = Width;

    Buffer->Info.bmiHeader.biCompression = BI_RGB;
    Buffer->Info.bmiHeader.biBitCount = 32;
    Buffer->Info.bmiHeader.biPlanes = 1;

    Buffer->Height = Height;
    Buffer->Width = Width;

    Buffer->Pitch = Width * 4;
    Buffer->BytesPerPixel = 4;

    size_t BitmapMemorySize = (size_t)(4 * Width * Height);
    if (Buffer->Memory != NULL) VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
    Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
}

static void DisplayBuffer(win32_offscreen_buffer_t *Buffer, HDC DeviceContext, int WindowWidth, int WindowHeight)
{
    int  OffsetX = 0, OffsetY = 0;

    PatBlt(DeviceContext, 0, 0, WindowWidth, OffsetY, BLACKNESS);
    PatBlt(DeviceContext, 0, OffsetY + Buffer->Height, WindowWidth, WindowHeight, BLACKNESS);

    PatBlt(DeviceContext, 0, 0, OffsetX, WindowHeight, BLACKNESS);
    PatBlt(DeviceContext, OffsetX + Buffer->Width, 0, WindowWidth, WindowHeight, BLACKNESS);

    StretchDIBits
    (
        DeviceContext,

        OffsetX, OffsetY, Buffer->Width, Buffer->Height,
        0, 0, Buffer->Width, Buffer->Height,

        Buffer->Memory, &Buffer->Info,
        DIB_RGB_COLORS, SRCCOPY
    );
}

POINT mouse_pos(HWND hWnd)
{
    POINT mouse_pos;
    GetCursorPos(&mouse_pos);
    
    ScreenToClient(hWnd, &mouse_pos);
    return mouse_pos;
}

static float lerp(float v0, float v1, float t)
{
    return (1 - t) * v0 + t * v1;
}

static float seconds_now()
{
    static LARGE_INTEGER s_frequency = { 0 };
    static BOOL s_use_qpc = QueryPerformanceFrequency(&s_frequency);

    LARGE_INTEGER now = { 0 };
    QueryPerformanceCounter(&now);
    return (float) ((double)now.QuadPart / (double)s_frequency.QuadPart);
}

static LRESULT CALLBACK WndProc(HWND hWnd, UINT hMsg, WPARAM wParam, LPARAM lParam)
{
    if (hMsg == WM_CLOSE)
    {
        DestroyWindow(hWnd);
        GlobalExit = true;
        return 0;
    }
    else if (hMsg == WM_DESTROY)
    {
        PostQuitMessage(EXIT_SUCCESS);
        return 0;
    }
    else if (hMsg == WM_PAINT)
    {
        PAINTSTRUCT Paint = { 0 };
        HDC DeviceContext = BeginPaint(hWnd, &Paint);

        window_dimensions_t d = GetWindowDimension(hWnd);
        ResizeBIBSection(&GlobalBackBuffer, d.Width, d.Height);
        DisplayBuffer(&GlobalBackBuffer, DeviceContext, d.Width, d.Height);

        EndPaint(hWnd, &Paint);
    }

    return DefWindowProcA(hWnd, hMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hActualInst, HINSTANCE hPrevInst, LPSTR cmdLine, int cmdShow)
{
    UNUSED(hPrevInst);
    UNUSED(cmdLine);
    UNUSED(cmdShow);

    WNDCLASSEXA	wndCls = { 0 };
    HWND hWnd = nullptr;

    wndCls.hInstance = hActualInst;
    wndCls.cbSize = sizeof(WNDCLASSEXA);

    wndCls.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndCls.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wndCls.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    wndCls.lpfnWndProc = WndProc;
    wndCls.lpszClassName = CLASS_NAME;
    wndCls.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    if (!RegisterClassExA(&wndCls))
    {
        MessageBoxA(NULL, "Window registration failed", "Error", MB_ICONEXCLAMATION | MB_OK);
        return EXIT_FAILURE;
    }

    int StartX = GetSystemMetrics(SM_CXSCREEN) / 2 - WINDOW_WIDTH / 2;
    int StartY = GetSystemMetrics(SM_CYSCREEN) / 2 - WINDOW_HEIGHT / 2;

    hWnd = CreateWindowExA
    (
        WS_EX_CLIENTEDGE, CLASS_NAME, WINDOW_NAME, WS_OVERLAPPEDWINDOW, StartX, StartY,
        WINDOW_WIDTH, WINDOW_HEIGHT, NULL, NULL, hActualInst, NULL
    );

    if (!hWnd)
    {
        MessageBoxA(NULL, "Window creation failed", "Error", MB_ICONEXCLAMATION | MB_OK);
        return EXIT_FAILURE;
    }

    ShowWindow(hWnd, SW_SHOW);
    UpdateWindow(hWnd);

    CGraphicsManager GraphicsManager;
    HDC DeviceContext = GetDC(hWnd);
    MSG hMsg = { 0 };

    float last_frame_time = seconds_now();
    float fix_frame_time = 1.0f / 60.0f;

    float acc_time = 0.0f, pause_acc_time = 0.0f;
    float total_time = 10.0f;

    GenerateColors();
    bool pause = false;

    float x = 0.0f, y = 0.0f, z = 1.0f;
    POINT last_mouse_pos = mouse_pos(hWnd);

    while (!GlobalExit)
    {
        while (PeekMessage(&hMsg, NULL, 0U, 0U, PM_REMOVE) > 0)
        {
            TranslateMessage(&hMsg);
            DispatchMessageA(&hMsg);
        }

        float current_frame_time = seconds_now();
        float dt = current_frame_time - last_frame_time;

        dt = dt > fix_frame_time ? fix_frame_time : dt;
        acc_time += dt;

        acc_time = acc_time > total_time ? total_time : acc_time;
        float cRe = lerp(-0.162f, -0.79f, acc_time / total_time);
        float cIm = lerp(1.04f, 0.15f, acc_time / total_time);

        if (WasDown(VK_LEFT))
        {
            x -= 1.0f * dt;
        }

        if (WasDown(VK_RIGHT))
        {
            x += 1.0f * dt;
        }

        if (WasDown(VK_UP))
        {
            y -= 1.0f * dt;
        }

        if (WasDown(VK_DOWN))
        {
            y += 1.0f * dt;
        }

        if (WasDown(VK_ADD))
        {
            z += 10.0f * dt;
        }

        if (WasDown(VK_SUBTRACT))
        {
            z -= 2.0f * dt;
            if (z < 1.0f) z = 1.0f;
        }

        if (WasDown('P'))
        {
            pause = !pause;
            pause_acc_time = acc_time;
        }

        if (WasDown('R'))
        {
            acc_time = 0.0f;
        }

        if (pause)
        {
            acc_time = pause_acc_time;
        }
        
        window_dimensions_t WndDimensions = GetWindowDimension(hWnd);
        offscreen_buffer_t DrawBuffer = { 0 };

        DrawBuffer.Memory = GlobalBackBuffer.Memory;
        DrawBuffer.BytesPerPixel = GlobalBackBuffer.BytesPerPixel;

        DrawBuffer.Height= GlobalBackBuffer.Height;
        DrawBuffer.Width = GlobalBackBuffer.Width;
        DrawBuffer.Pitch = GlobalBackBuffer.Pitch;

        DrawJulia<float>(&GraphicsManager, &DrawBuffer, WndDimensions.Width, WndDimensions.Height, z, cRe, cIm, x, y);
        DisplayBuffer(&GlobalBackBuffer, DeviceContext, WndDimensions.Width, WndDimensions.Height);
        last_frame_time = current_frame_time;
    }

    UnregisterClassA(CLASS_NAME, wndCls.hInstance);
    return EXIT_SUCCESS;
}