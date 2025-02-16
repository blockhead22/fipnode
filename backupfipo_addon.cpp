#include "C:/fipnode/node_modules/node-addon-api/napi.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "DirectOutputImpl.h"
#include <thread>
#include <windows.h>
#include <iostream>
#include <fstream>
#include <vector>

void __stdcall DeviceCallback(void* hDevice, bool bAdded, void* pvContext);
void __stdcall DeviceEnumerateCallback(void* hDevice, void* pCtxt);
void __stdcall ButtonCallback(void* hDevice, DWORD dwButtons, void* pCtxt);


CDirectOutput g_DirectOutput;
void* g_hDevice = nullptr;
Napi::ThreadSafeFunction tsfn;

// Flipping the image vertically
void FlipImageVertically(unsigned char* imageData, int width, int height, int channels) {
    int rowSize = width * channels;
    for (int y = 0; y < height / 2; ++y) {
        unsigned char* topRow = imageData + y * rowSize;
        unsigned char* bottomRow = imageData + (height - y - 1) * rowSize;
        for (int x = 0; x < rowSize; ++x) {
            std::swap(topRow[x], bottomRow[x]);
        }
    }
}

// Convert RGB to BGR (for correct color)
void ConvertRGBToBGR(unsigned char* imageData, int width, int height, int channels) {
    if (channels == 3) {  // RGB format
        for (int i = 0; i < width * height; ++i) {
            std::swap(imageData[i * 3], imageData[i * 3 + 2]);  // Swap R and B
        }
    }
}

// Device enumeration callback
void __stdcall DeviceEnumerateCallback(void* hDevice, void* pCtxt) {
    if (hDevice) {
        g_hDevice = hDevice;
        printf("✅ FIP device detected! Handle: %p\n", hDevice);
    } else {
        printf("❌ DeviceEnumerateCallback triggered, but no valid device handle.\n");
    }
}

// Device connection callback
void __stdcall DeviceCallback(void* hDevice, bool bAdded, void* pvContext) {
    if (bAdded) {
        g_hDevice = hDevice;
        printf("✅ FIP connected! Handle: %p\n", g_hDevice);
    } else {
        g_hDevice = nullptr;
        printf("❌ FIP disconnected!\n");
    }
}


// Send image to the FIP device
void SendImageToFIP(const unsigned char* imageData, int width, int height, int channels) {
    std::cout << "Sending image to FIP display..." << std::endl;
    HRESULT hr = g_DirectOutput.SetImage(g_hDevice, 1, 0, width * height * channels, imageData);

    if (hr != S_OK) {
        std::cout << "❌ Failed to update FIP display. Error Code: 0x" << std::hex << hr << std::dec << std::endl;
    } else {
        std::cout << "✅ Image successfully sent to FIP display." << std::endl;
    }
}

// Send image from file to FIP
Napi::String SendImageFromFile(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() < 1 || !info[0].IsString()) {
        Napi::TypeError::New(env, "Path to image file is required").ThrowAsJavaScriptException();
        return Napi::String::New(env, "");
    }

    std::string filePath = info[0].As<Napi::String>().Utf8Value();

    int width, height, channels;
    unsigned char* imageData = stbi_load(filePath.c_str(), &width, &height, &channels, 3);

    if (imageData == nullptr) {
        Napi::Error::New(env, "Failed to load image").ThrowAsJavaScriptException();
        return Napi::String::New(env, "");
    }

    // Flip and convert the image
    FlipImageVertically(imageData, width, height, channels);
    ConvertRGBToBGR(imageData, width, height, channels);

    // Send image to FIP
    SendImageToFIP(imageData, width, height, channels);

    // Free the image data
    stbi_image_free(imageData);

    return Napi::String::New(env, "Image sent to FIP successfully.");
}

// Initialize DirectOutput
Napi::Number InitDirectOutput(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    HRESULT hr = g_DirectOutput.Initialize(L"FIP_HelloWorld");
    if (hr != S_OK) {
        printf("❌ Failed to initialize DirectOutput! Error Code: 0x%08X\n", hr);
        return Napi::Number::New(env, hr);
    }

    HRESULT hrCallback = g_DirectOutput.RegisterDeviceCallback(DeviceCallback, nullptr);
    if (hrCallback != S_OK) {
        printf("❌ Failed to register device callback! Error Code: 0x%08X\n", hrCallback);
        return Napi::Number::New(env, hrCallback);
    }

    HRESULT hrEnum = g_DirectOutput.Enumerate(DeviceEnumerateCallback, nullptr);
    if (hrEnum != S_OK) {
        printf("❌ Failed to enumerate devices! Error Code: 0x%08X\n", hrEnum);
    }

    HRESULT hrPage = g_DirectOutput.AddPage(g_hDevice, 1, FLAG_SET_AS_ACTIVE);
    if (hrPage != S_OK) {
        printf("❌ Failed to add page! Error Code: 0x%08X\n", hrPage);
    }

    return Napi::Number::New(env, S_OK);
}

// Register functions
Napi::Object Init(Napi::Env env, Napi::Object exports) {
    exports.Set(Napi::String::New(env, "initDirectOutput"), Napi::Function::New(env, InitDirectOutput));
    exports.Set("sendImageFromFile", Napi::Function::New(env, SendImageFromFile));
    return exports;
}

NODE_API_MODULE(fip_addon, Init);
