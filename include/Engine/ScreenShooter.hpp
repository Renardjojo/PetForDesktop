#pragma once

#include "../../deps/screen_capture_lite/include/ScreenCapture.h" // Sorry for that shame
#include <map>
#include <memory>
#include "Game/GameData.hpp"

class ScreenShooter
{
public:
    struct ImageData
    {
        unsigned int                     width       = 0;
        unsigned int                     height      = 0;
        unsigned int                     bitPerPixel = 0;
        std::unique_ptr<unsigned char[]> bits        = nullptr;
    };

    struct MonitorsFramebuffer : public ImageData
    {
        int offsetX = 0;
        int offsetY = 0;
    };

private:
    bool                                                                     canScreenCapture = false;
    std::shared_ptr<SL::Screen_Capture::IScreenCaptureManager>               framgrabber;
    MonitorsFramebuffer monitorsFramebuffer;

public:
    ScreenShooter(GameData& data)
    {
        initScreenCaptureLite();
        initCaptureProcess(data);
    }

    void initScreenCaptureLite()
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

    void initCaptureProcess(const GameData& data)
    {
        framgrabber =
            SL::Screen_Capture::CreateCaptureConfiguration([&]() {
                auto monitors = SL::Screen_Capture::GetMonitors();

                int minOffsetX = std::numeric_limits<int>::max();
                int minOffsetY = std::numeric_limits<int>::max();
                int maxOffsetX = std::numeric_limits<int>::min();
                int maxOffsetY = std::numeric_limits<int>::min();

                for (size_t i = 0; i < monitors.size(); i++)
                {
                    minOffsetX = std::min(monitors[i].OffsetX, minOffsetX);
                    minOffsetY = std::min(monitors[i].OffsetY, minOffsetY);
                    maxOffsetX = std::max(monitors[i].OffsetX + monitors[i].Width, maxOffsetX);
                    maxOffsetY = std::max(monitors[i].OffsetY + monitors[i].Height, maxOffsetY);
                }

                // Init global framebuffer
                monitorsFramebuffer.offsetX     = minOffsetX;
                monitorsFramebuffer.offsetY     = minOffsetY;
                monitorsFramebuffer.width       = maxOffsetX - minOffsetX;
                monitorsFramebuffer.height      = maxOffsetY - minOffsetY;
                monitorsFramebuffer.bitPerPixel = sizeof(SL::Screen_Capture::ImageBGRA);
                size_t size = monitorsFramebuffer.width * monitorsFramebuffer.height * monitorsFramebuffer.bitPerPixel;
                monitorsFramebuffer.bits = std::make_unique<unsigned char[]>(size);

                return SL::Screen_Capture::GetMonitors();
            })
                ->onNewFrame([&](const SL::Screen_Capture::Image& img, const SL::Screen_Capture::Monitor& monitor) {
                    extractAndConvertToRGBA(img, monitorsFramebuffer.bits.get(),
                                            monitor.Width * monitor.Height * sizeof(SL::Screen_Capture::ImageBGRA),
                                            monitor.OffsetX, monitor.OffsetY);
                })
                ->start_capturing();

        framgrabber->setFrameChangeInterval(std::chrono::milliseconds(data.screenCaptureInterval));
    }

    ImageData getMonitorRegion(int x, int y, int w, int h)
    {
        if (monitorsFramebuffer.bits == nullptr)
            return ImageData{};

        // Make sure the requested region is within bounds
        if (x < monitorsFramebuffer.offsetX || y < monitorsFramebuffer.offsetY ||
            x + w > monitorsFramebuffer.offsetX + monitorsFramebuffer.width ||
            y + h > monitorsFramebuffer.offsetY + monitorsFramebuffer.height)
        {
            // Return an empty image data structure or handle the error as appropriate
            return ImageData{};
        }
        
        x -= monitorsFramebuffer.offsetX;
        y -= monitorsFramebuffer.offsetY;

        // Calculate the size of the region in bytes
        size_t regionSize = static_cast<size_t>(w) * h * monitorsFramebuffer.bitPerPixel;

        // Allocate memory for the new image
        auto newBits(std::make_unique<unsigned char[]>(regionSize));

        // Calculate the offset to the starting pixel in the source image
        size_t offset = static_cast<size_t>(y) * monitorsFramebuffer.width + x;

        // Calculate the number of bytes in a row of pixels in the source image
        size_t sourceRowSize = monitorsFramebuffer.width * monitorsFramebuffer.bitPerPixel;

        // Iterate over each row in the region and copy pixels to the new image
        for (int row = 0; row < h; ++row)
        {
            void* sourceRow =
                static_cast<unsigned char*>(monitorsFramebuffer.bits.get()) + offset * monitorsFramebuffer.bitPerPixel;
            void* destRow = static_cast<unsigned char*>(newBits.get()) +
                            (h - row - 1) * w * monitorsFramebuffer.bitPerPixel; // Vertical flip
            memcpy(destRow, sourceRow, static_cast<size_t>(w) * monitorsFramebuffer.bitPerPixel);
            offset += monitorsFramebuffer.width;
        }

        // Create a new ImageData structure for the extracted region
        return ImageData{static_cast<unsigned int>(w), static_cast<unsigned int>(h),
                         sizeof(SL::Screen_Capture::ImageBGRA), std::move(newBits)};
    }

    void startCatpure()
    {
        framgrabber->resume();
    }

    void stopCapture()
    {
        framgrabber->pause();
    }

    void extractAndConvertToRGBA(const SL::Screen_Capture::Image& img, unsigned char* dst, size_t dst_size, int x,
                                 int y)
    {
        auto imgsrc    = StartSrc(img);
        auto imgdst    = dst;
        int  srcHeight = Height(img);
        int  srcWidth  = Width(img);
        for (auto h = 0; h < srcHeight; h++)
        {
            auto startimgsrc = imgsrc;
            for (auto w = 0; w < srcWidth; w++)
            {
                int offset               = (x + w + (y + h) * srcWidth) * 4;
                *(imgdst + offset + 0) = imgsrc->R;
                *(imgdst + offset + 1) = imgsrc->G;
                *(imgdst + offset + 2) = imgsrc->B;
                *(imgdst + offset + 3) = 0; // alpha should be zero
                imgsrc++;
            }
            imgsrc = SL::Screen_Capture::GotoNextRow(img, startimgsrc);
        }
    }
};