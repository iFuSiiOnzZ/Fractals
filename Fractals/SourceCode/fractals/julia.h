#pragma once

/*
    NOTE(Andrei):
        http://lodev.org/cgtutor/juliamandelbrot.html
        http://www.karlsims.com/julia.html
*/

// NOTE(Andrei): Colors
int r[256], g[256], b[256];

template <typename T> void DrawJulia(CGraphicsManager *GraphicsManager, offscreen_buffer_t *Buffer, int w, int h, T zoom = (T) 1, T cRe = (T) -0.7, T cIm = (T)0.27015, T moveX = (T)0, T moveY = (T)0, int maxIterations = 255)
{
    #pragma omp parallel for num_threads(8)
    for (int y = 0, itr = 0; y < h; y++)
    {
        for (int x = 0; x < w; x++)
        {
            T newRe = (T)(1.5 * (x - w / 2) / (0.5 * zoom * w) + moveX);
            T newIm = (T)((y - h / 2) / (0.5 * zoom * h) + moveY);

            for (itr = maxIterations; itr > 0 ; --itr)
            {
                T oldRe = newRe;
                T oldIm = newIm;

                newRe = oldRe * oldRe - oldIm * oldIm + cRe;
                newIm = 2 * oldRe * oldIm + cIm;

                if ((newRe * newRe + newIm * newIm) > 4) break;
            }

            GraphicsManager->PutPixel(Buffer, (float)x, (float)y, r[itr], g[itr], b[itr]);
        }
    }
}
