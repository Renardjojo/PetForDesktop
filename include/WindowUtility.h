#define NOMINMAX
#include <Windows.h>

class ScreenShoot
{
public:
    struct Data
    {
        unsigned int width       = 0;
        unsigned int height      = 0;
        unsigned int bitPerPixel = 0;
        void*        bits        = nullptr;
    };

protected:
    HDC     hScreen;
    HDC     hDC;
    HBITMAP hBitmap;
    HGDIOBJ old_obj;
    HANDLE  hDIB;

    Data data;

public:
    ScreenShoot(int x, int y, int w, int h)
    {
        if (w * h == 0)
            return;

        // https://docs.microsoft.com/fr-fr/windows/win32/gdi/capturing-an-image
        // https://stackoverflow.com/a/28248531
        // copy screen to bitmap
        hScreen = GetDC(NULL);
        hDC     = CreateCompatibleDC(hScreen);
        hBitmap = CreateCompatibleBitmap(hScreen, w, h);
        old_obj = SelectObject(hDC, hBitmap);
        if (!BitBlt(hDC, 0, 0, w, h, hScreen, x, y, SRCCOPY))
        {
            puts("BitBlt has failed");
        }

        BITMAP bitmap;
        GetObject(hBitmap, sizeof(BITMAP), &bitmap);

        BITMAPINFOHEADER bi;

        bi.biSize          = sizeof(BITMAPINFOHEADER);
        bi.biWidth         = bitmap.bmWidth;
        bi.biHeight        = bitmap.bmHeight;
        bi.biPlanes        = bitmap.bmPlanes;
        bi.biBitCount      = bitmap.bmBitsPixel;
        bi.biCompression   = BI_RGB;
        bi.biSizeImage     = 0;
        bi.biXPelsPerMeter = 0;
        bi.biYPelsPerMeter = 0;
        bi.biClrUsed       = 0;
        bi.biClrImportant  = 0;

        DWORD dwBmpSize = ((bitmap.bmWidth * bi.biBitCount + bi.biBitCount - 1) / bi.biBitCount) * 4 * bitmap.bmHeight;

        // Starting with 32-bit Windows, GlobalAlloc and LocalAlloc are implemented as wrapper functions that
        // call HeapAlloc using a handle to the process's default heap. Therefore, GlobalAlloc and LocalAlloc
        // have greater overhead than HeapAlloc.
        hDIB             = GlobalAlloc(GHND, dwBmpSize);
        data.bits        = (char*)GlobalLock(hDIB);
        data.bitPerPixel = bitmap.bmBitsPixel;
        data.height      = bitmap.bmHeight;
        data.width       = bitmap.bmWidth;

        // Gets the "bits" from the bitmap, and copies them into a buffer
        // that's pointed to by lpbitmap.
        GetDIBits(hScreen, hBitmap, 0, (UINT)bitmap.bmHeight, data.bits, (BITMAPINFO*)&bi, DIB_RGB_COLORS);
    }

    ~ScreenShoot()
    {
        // Unlock and Free the DIB from the heap.
        GlobalUnlock(hDIB);
        GlobalFree(hDIB);

        // clean up
        SelectObject(hDC, old_obj);
        DeleteDC(hDC);
        ReleaseDC(NULL, hScreen);
        DeleteObject(hBitmap);
    }

    const Data& get() // void* for static polymorphisme
    {
        return data;
    }
};