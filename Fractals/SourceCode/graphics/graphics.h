#pragma once

typedef struct offscreen_buffer_t
{
    void *Memory;

    int Width;
    int Height;

    int Pitch;
    int BytesPerPixel;
} offscreen_buffer_t;

struct CGraphicsManager
{
    static void PutPixel(offscreen_buffer_t *Buffer, float x, float y, int r, int g, int b);
    static void ClearBuffer(offscreen_buffer_t *Buffer, float r, float g, float b);

    static unsigned int MakeColor(float r, float g, float b);
    static unsigned int MakeColor(int r, int g, int b);
};
