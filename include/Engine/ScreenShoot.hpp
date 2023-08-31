#pragma once

#include "../../deps/screen_capture_lite/include/ScreenCapture.h" // Sorry for that shame
#include <map>
#include <thread>

class ScreenCaptureLite
{
public:
    struct ImageData
    {
        unsigned int                     width       = 0;
        unsigned int                     height      = 0;
        unsigned int                     bitPerPixel = 0;
        std::unique_ptr<unsigned char[]> bits        = nullptr;
    };

private:
    bool                                                       canScreenCapture = false;
    std::shared_ptr<SL::Screen_Capture::IScreenCaptureManager> framgrabber;
    std::map<const SL::Screen_Capture::Monitor*, std::unique_ptr<ImageData>> monitorFrameBuffer;

public:
    ScreenCaptureLite()
    {
        initScreenCaptureLite();
        initCaptureProcess();
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

    void initCaptureProcess()
    {
        framgrabber =
            SL::Screen_Capture::CreateCaptureConfiguration([]() {
                auto mons = SL::Screen_Capture::GetMonitors();
                return SL::Screen_Capture::GetMonitors();
            })
                ->onNewFrame([&](const SL::Screen_Capture::Image& img, const SL::Screen_Capture::Monitor& monitor) {
                    auto monitorIt = monitorFrameBuffer.find(&monitor);
                    if (monitorIt == monitorFrameBuffer.end())
                    {
                        monitorIt = monitorFrameBuffer.emplace(&monitor, nullptr).first;
                    }
                    size_t size = Width(img) * Height(img) * sizeof(SL::Screen_Capture::ImageBGRA);
                    std::unique_ptr<unsigned char[]> imgbuffer(std::make_unique<unsigned char[]>(size));
                    extractAndConvertToRGBA(img, imgbuffer.get(), size);

                    monitorIt->second = std::make_unique<ImageData>(
                        ImageData{(unsigned int)Width(img), (unsigned int)Height(img),
                                  sizeof(SL::Screen_Capture::ImageBGRA), std::move(imgbuffer)});
                })
                ->start_capturing();

        framgrabber->setFrameChangeInterval(std::chrono::milliseconds(100));
        framgrabber->setMouseChangeInterval(std::chrono::milliseconds(100));
    }

    ImageData getMonitorRegion(int x, int y, int w, int h)
    {
        if (monitorFrameBuffer.empty() || monitorFrameBuffer.begin()->second == nullptr)
            return ImageData{};

        const ImageData& sourceImage = *monitorFrameBuffer.begin()->second;

        // Make sure the requested region is within bounds
        if (x < 0 || y < 0 || x + w > sourceImage.width || y + h > sourceImage.height)
        {
            // Return an empty image data structure or handle the error as appropriate
            return ImageData{};
        }

        // Calculate the size of the region in bytes
        size_t regionSize = static_cast<size_t>(w) * h * sourceImage.bitPerPixel;

        // Allocate memory for the new image
        auto newBits(std::make_unique<unsigned char[]>(regionSize));

        // Calculate the offset to the starting pixel in the source image
        size_t offset = static_cast<size_t>(y) * sourceImage.width + x;

        // Calculate the number of bytes in a row of pixels in the source image
        size_t sourceRowSize = sourceImage.width * sourceImage.bitPerPixel;

        // Iterate over each row in the region and copy pixels to the new image
        for (int row = 0; row < h; ++row)
        {
            void* sourceRow = static_cast<unsigned char*>(sourceImage.bits.get()) + offset * sourceImage.bitPerPixel;
            void* destRow   = static_cast<unsigned char*>(newBits.get()) +
                            (h - row - 1) * w * sourceImage.bitPerPixel; // Vertical flip
            memcpy(destRow, sourceRow, static_cast<size_t>(w) * sourceImage.bitPerPixel);
            offset += sourceImage.width;
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

    void extractAndConvertToRGBA(const SL::Screen_Capture::Image& img, unsigned char* dst, size_t dst_size)
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