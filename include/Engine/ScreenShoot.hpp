#pragma once

#ifdef __linux__
#elif _WIN32
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
    HDC     hScreen = nullptr;
    HDC     hDC     = nullptr;
    HBITMAP hBitmap = nullptr;
    HGDIOBJ old_obj = nullptr;
    HGLOBAL  hDIB    = nullptr;

    Data data;

public:
    ScreenShoot(int x, int y, int w, int h, bool saveIntoClipboard = false)
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

        if (saveIntoClipboard)
        {
            // save bitmap to clipboard
            OpenClipboard(NULL);
            EmptyClipboard();
            SetClipboardData(CF_BITMAP, hBitmap);
            CloseClipboard();
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

#include "../../deps/screen_capture_lite/include/ScreenCapture.h" // Sorry for that shame
#define TJE_IMPLEMENTATION
#include "tiny_jpeg.h" // TODO: Remove me
#include <thread>

class ScreenCaptureLite
{
    bool canScreenCapture = false;
    std::atomic<int> realcounter;
    std::shared_ptr<SL::Screen_Capture::IScreenCaptureManager> framgrabber;

public: 
    ScreenCaptureLite()
    {
        InitScreenCaptureLite();
        Capture();
    }

    
    void InitScreenCaptureLite()
    {
        // Checking for Permission to capture the screen
        if (SL::Screen_Capture::IsScreenCaptureEnabled())
        {
            // Application Allowed to Capture the screen!
            canScreenCapture = true;
        }
        else if (SL::Screen_Capture::CanRequestScreenCapture())
        {
            // Application Not Allowed to Capture the screen. Waiting for permission
            while (!SL::Screen_Capture::IsScreenCaptureEnabled())
            {
                SL::Screen_Capture::RequestScreenCapture();
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
            canScreenCapture = true;
        }
        else
        {
            canScreenCapture = false;
        }
    }

    void Capture()
    {
        framgrabber =
            SL::Screen_Capture::CreateCaptureConfiguration([]() {
                auto mons    = SL::Screen_Capture::GetMonitors();
                auto newmons = std::vector<SL::Screen_Capture::Monitor>();
                // Request the list of monitors to capture!
                for (auto& m : mons)
                {
                    if (SL::Screen_Capture::Height(m) >= 512 * 2 && SL::Screen_Capture::Width(m) >= 512 * 2)
                    {
                        SL::Screen_Capture::Height(m, 512);
                        SL::Screen_Capture::Width(m, 512);
                        newmons.push_back(m);
                    }
                }
                return newmons;
            })
                ->onNewFrame([&](const SL::Screen_Capture::Image& img, const SL::Screen_Capture::Monitor& monitor) {
                    // Uncomment the below code to write the image to disk for debugging

                    auto r    = realcounter.fetch_add(1);
                    auto s    = std::string("MONITORNEW_") + std::to_string(r) + std::string(".jpg");
                    auto size = Width(img) * Height(img) * sizeof(SL::Screen_Capture::ImageBGRA);
                    auto imgbuffer(std::make_unique<unsigned char[]>(size));
                    ExtractAndConvertToRGBA(img, imgbuffer.get(), size);
                    //tje_encode_to_file(s.c_str(), Width(img), Height(img), 4, (const unsigned char*)imgbuffer.get());
                })
                ->start_capturing();
        framgrabber->pause();

        framgrabber->setFrameChangeInterval(std::chrono::milliseconds(100));
        framgrabber->setMouseChangeInterval(std::chrono::milliseconds(100));
    }

    void StartCatpure()
    {
        framgrabber->resume();
    }

    void StopCapture()
    {
        framgrabber->pause();
    }

    void ExtractAndConvertToRGBA(const SL::Screen_Capture::Image& img, unsigned char* dst, size_t dst_size)
    {
        assert(dst_size >= static_cast<size_t>(SL::Screen_Capture::Width(img) * SL::Screen_Capture::Height(img) *
                                               sizeof(SL::Screen_Capture::ImageBGRA)));
        auto imgsrc  = StartSrc(img);
        auto imgdist = dst;
        for (auto h = 0; h < Height(img); h++)
        {
            auto startimgsrc = imgsrc;
            for (auto w = 0; w < Width(img); w++)
            {
                *imgdist++ = imgsrc->R;
                *imgdist++ = imgsrc->G;
                *imgdist++ = imgsrc->B;
                *imgdist++ = 0; // alpha should be zero
                imgsrc++;
            }
            imgsrc = SL::Screen_Capture::GotoNextRow(img, startimgsrc);
        }
    }
};
#else
#endif