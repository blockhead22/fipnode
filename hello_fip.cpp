#include "C:/fipnode/node_modules/node-addon-api/napi.h"
#include "DirectOutputImpl.h"
#include <windows.h>
#include <iostream>

CDirectOutput g_DirectOutput;
void* g_hDevice = nullptr;
DWORD g_PageId = 0;

// Callback for device enumeration
void __stdcall DeviceEnumerateCallback(void* hDevice, void* pCtxt) {
    g_hDevice = hDevice;
    std::cout << "FIP device found." << std::endl;
}

// Callback for device connection/disconnection
void __stdcall DeviceCallback(void* hDevice, bool bAdded, void* pvContext) {
    if (bAdded) {
        g_hDevice = hDevice;
        std::cout << "FIP connected." << std::endl;
    } else {
        g_hDevice = nullptr;
        std::cout << "FIP disconnected." << std::endl;
    }
}

// Initialize FIP
Napi::Boolean InitializeFIP(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    HRESULT hr = g_DirectOutput.Initialize(L"FIP_NodeAddon");
    if (hr != S_OK) {
        std::cerr << "Failed to initialize DirectOutput! Code: 0x" << std::hex << hr << std::endl;
        return Napi::Boolean::New(env, false);
    }
    
    g_DirectOutput.RegisterDeviceCallback(DeviceCallback, nullptr);
    g_DirectOutput.Enumerate(DeviceEnumerateCallback, nullptr);
    
    return Napi::Boolean::New(env, true);
}

// Send image to FIP
Napi::Boolean SendImageToFIP(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (g_hDevice == nullptr) {
        std::cerr << "No FIP device detected!" << std::endl;
        return Napi::Boolean::New(env, false);
    }

    if (!info[0].IsBuffer()) {
        Napi::TypeError::New(env, "Expected a buffer").ThrowAsJavaScriptException();
        return Napi::Boolean::New(env, false);
    }

    Napi::Buffer<uint8_t> buffer = info[0].As<Napi::Buffer<uint8_t>>();
    
    HRESULT result = g_DirectOutput.SetImage(g_hDevice, g_PageId, 0, buffer.Length(), buffer.Data());
    return Napi::Boolean::New(env, result == S_OK);
}

// Set text on FIP
Napi::Boolean SetStringToFIP(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (g_hDevice == nullptr) {
        std::cerr << "No FIP device detected!" << std::endl;
        return Napi::Boolean::New(env, false);
    }

    if (!info[0].IsString()) {
        Napi::TypeError::New(env, "Expected a string").ThrowAsJavaScriptException();
        return Napi::Boolean::New(env, false);
    }

    std::wstring text = std::wstring(info[0].As<Napi::String>().Utf16Value());

    HRESULT result = g_DirectOutput.SetString(g_hDevice, g_PageId, 0, text.length() + 1, text.c_str());
    return Napi::Boolean::New(env, result == S_OK);
}

// Shutdown FIP
Napi::Boolean ShutdownFIP(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    g_DirectOutput.Deinitialize();
    return Napi::Boolean::New(env, true);
}

// Expose functions to Node.js
Napi::Object Init(Napi::Env env, Napi::Object exports) {
    exports.Set(Napi::String::New(env, "initializeFIP"), Napi::Function::New(env, InitializeFIP));
    exports.Set(Napi::String::New(env, "sendImageToFIP"), Napi::Function::New(env, SendImageToFIP));
    exports.Set(Napi::String::New(env, "setStringToFIP"), Napi::Function::New(env, SetStringToFIP));
    exports.Set(Napi::String::New(env, "shutdownFIP"), Napi::Function::New(env, ShutdownFIP));
    return exports;
}

NODE_API_MODULE(fip_addon, Init)
