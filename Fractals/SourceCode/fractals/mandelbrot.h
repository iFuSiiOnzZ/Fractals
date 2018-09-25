#pragma once

/*
    NOTE(Andrei):
        https://rosettacode.org/wiki/Mandelbrot_set#C
        https://en.wikipedia.org/wiki/Mandelbrot_set
*/

template <typename T> void DrawMandelbrot(CGraphicsManager *GraphicsManager, offscreen_buffer_t *Buffer, int w, int h, T zoom = (T)1, T cxMin = (T)-2.5, T cxMax = (T)1.0, T cyMin = (T)-1.0f, T cyMax = (T)1.0, int maxIterations = 255)
{
    float pixelWidth  = (cxMax - cxMin) / (float)w;
    float pixelHeight = (cyMax - cyMin) / (float)h;

    #pragma omp parallel for num_threads(8)
    for (int y = 0, itr = 0; y < h; y++)
    {
        float cy = cyMin + y * pixelHeight;

        for (int x = 0; x < w; x++)
        {
            float xt = 0, yt = 0;
            int itr = maxIterations;
            float cx = cxMin + x * pixelWidth;

            while (xt * xt + yt * yt < 2.0f * 2.0f && itr-- > 0)
            {
                float xtemp = xt * xt - yt *yt + cx;
                yt = 2 * xt * yt + cy;
                xt = xtemp;
            }

            if (itr > 0)
            {
                GraphicsManager->PutPixel(Buffer, (float)x, (float)y, 0, 0, 0) ;
            }
            else
            {
                GraphicsManager->PutPixel(Buffer, (float)x, (float)y, 255, 255, 255);
            }
        }
    }
}
