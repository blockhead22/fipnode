#include "DirectOutputImpl.h"
#include <iostream>
#include <ostream>
#include <windows.h>

CDirectOutput::CDirectOutput() : m_module(nullptr), m_initialize(nullptr), m_deinitialize(nullptr), 
                                 m_registerdevicecallback(nullptr), m_enumerate(nullptr), 
                                 m_registerpagecallback(nullptr), m_registersoftbuttoncallback(nullptr),
                                 m_getdevicetype(nullptr), m_getdeviceinstance(nullptr),
                                 m_setprofile(nullptr), m_addpage(nullptr), m_removepage(nullptr),
                                 m_setled(nullptr), m_setstring(nullptr), m_setimage(nullptr), m_setimagefromfile(nullptr),
                                 m_startserver(nullptr), m_closeserver(nullptr), m_sendservermsg(nullptr), m_sendserverfile(nullptr),
                                 m_savefile(nullptr), m_displayfile(nullptr), m_deletefile(nullptr) {

    wchar_t filename[MAX_PATH] = {0};
    if (GetDirectOutputFilename(filename, MAX_PATH)) {
        m_module = LoadLibraryW(filename);
        if (!m_module) {
            std::cerr << "Failed to load DirectOutput.dll! Error Code: " << GetLastError() << std::endl;
        } else {
            std::wcout << L"Loaded DirectOutput.dll from: " << filename << std::endl;
            
            m_initialize = (Pfn_DirectOutput_Initialize)GetProcAddress(m_module, "DirectOutput_Initialize");
            m_deinitialize = (Pfn_DirectOutput_Deinitialize)GetProcAddress(m_module, "DirectOutput_Deinitialize");
            m_registerdevicecallback = (Pfn_DirectOutput_RegisterDeviceCallback)GetProcAddress(m_module, "DirectOutput_RegisterDeviceCallback");
            m_enumerate = (Pfn_DirectOutput_Enumerate)GetProcAddress(m_module, "DirectOutput_Enumerate");
            m_registerpagecallback = (Pfn_DirectOutput_RegisterPageCallback)GetProcAddress(m_module, "DirectOutput_RegisterPageCallback");
            m_registersoftbuttoncallback = (Pfn_DirectOutput_RegisterSoftButtonCallback)GetProcAddress(m_module, "DirectOutput_RegisterSoftButtonCallback");
            m_getdevicetype = (Pfn_DirectOutput_GetDeviceType)GetProcAddress(m_module, "DirectOutput_GetDeviceType");
            m_getdeviceinstance = (Pfn_DirectOutput_GetDeviceInstance)GetProcAddress(m_module, "DirectOutput_GetDeviceInstance");
            m_setprofile = (Pfn_DirectOutput_SetProfile)GetProcAddress(m_module, "DirectOutput_SetProfile");
            m_addpage = (Pfn_DirectOutput_AddPage)GetProcAddress(m_module, "DirectOutput_AddPage");
            m_removepage = (Pfn_DirectOutput_RemovePage)GetProcAddress(m_module, "DirectOutput_RemovePage");
            m_setled = (Pfn_DirectOutput_SetLed)GetProcAddress(m_module, "DirectOutput_SetLed");
            m_setstring = (Pfn_DirectOutput_SetString)GetProcAddress(m_module, "DirectOutput_SetString");
            m_setimage = (Pfn_DirectOutput_SetImage)GetProcAddress(m_module, "DirectOutput_SetImage");
            m_setimagefromfile = (Pfn_DirectOutput_SetImageFromFile)GetProcAddress(m_module, "DirectOutput_SetImageFromFile");
            m_startserver = (Pfn_DirectOutput_StartServer)GetProcAddress(m_module, "DirectOutput_StartServer");
            m_closeserver = (Pfn_DirectOutput_CloseServer)GetProcAddress(m_module, "DirectOutput_CloseServer");
            m_sendservermsg = (Pfn_DirectOutput_SendServerMsg)GetProcAddress(m_module, "DirectOutput_SendServerMsg");
            m_sendserverfile = (Pfn_DirectOutput_SendServerFile)GetProcAddress(m_module, "DirectOutput_SendServerFile");
            m_savefile = (Pfn_DirectOutput_SaveFile)GetProcAddress(m_module, "DirectOutput_SaveFile");
            m_displayfile = (Pfn_DirectOutput_DisplayFile)GetProcAddress(m_module, "DirectOutput_DisplayFile");
            m_deletefile = (Pfn_DirectOutput_DeleteFile)GetProcAddress(m_module, "DirectOutput_DeleteFile");

            if (!m_initialize || !m_deinitialize || !m_registerdevicecallback || !m_enumerate ||
                !m_registerpagecallback || !m_registersoftbuttoncallback || !m_getdevicetype || !m_getdeviceinstance ||
                !m_setprofile || !m_addpage || !m_removepage || !m_setled || !m_setstring || !m_setimage ||
                !m_setimagefromfile || !m_startserver || !m_closeserver || !m_sendservermsg || !m_sendserverfile ||
                !m_savefile || !m_displayfile || !m_deletefile) {
                std::cerr << "One or more DirectOutput functions failed to load!" << std::endl;
            }
        }
    } else {
        std::cerr << "Could not determine DirectOutput.dll path from registry." << std::endl;
    }
}


CDirectOutput::~CDirectOutput() {
    if (m_module) {
        FreeLibrary(m_module);
    }
}

HRESULT CDirectOutput::Initialize(const wchar_t* wszPluginName) {
    if (m_module && m_initialize) {
        return m_initialize(wszPluginName);
    }
    std::cerr << "DirectOutput_Initialize is NULL! DLL not loaded properly." << std::endl;
    return E_NOTIMPL;
}

HRESULT CDirectOutput::Deinitialize() {
    if (m_module && m_deinitialize) {
        return m_deinitialize();
    }
    return E_NOTIMPL;
}

HRESULT CDirectOutput::AddPage(void* hDevice, DWORD dwPage, DWORD dwFlags) {
    if (m_module && m_addpage) {
        return m_addpage(hDevice, dwPage, dwFlags);
    }
    std::cerr << "AddPage is NULL! DirectOutput function not loaded properly." << std::endl;
    return E_NOTIMPL;
}


HRESULT CDirectOutput::SetString(void* hDevice, DWORD dwPage, DWORD dwLine, DWORD cchLength, const wchar_t* wszString) {
    if (!m_setstring) {
        std::cerr << "ERROR: SetString() function is NULL!" << std::endl;
        return E_FAIL;
    }

    std::cout << "Calling SetString() on FIP device..." << std::endl;
    HRESULT result = m_setstring(hDevice, dwPage, dwLine, cchLength, wszString);

    if (FAILED(result)) {
        std::cerr << "ERROR: SetString() failed! Error Code: 0x" << std::hex << result << std::endl;
    } else {
        std::cout << "SetString() call succeeded!" << std::endl;
    }

    return result;
}

HRESULT CDirectOutput::RegisterDeviceCallback(Pfn_DirectOutput_DeviceChange pfnCb, void* pCtxt) {
    if (m_module && m_registerdevicecallback) {
        return m_registerdevicecallback(pfnCb, pCtxt);
    }
    std::cerr << "RegisterDeviceCallback is NULL! DirectOutput function not loaded properly." << std::endl;
    return E_NOTIMPL;
}

HRESULT CDirectOutput::Enumerate(Pfn_DirectOutput_EnumerateCallback pfnCb, void* pCtxt) {
    if (m_module && m_enumerate) {
        return m_enumerate(pfnCb, pCtxt);
    }
    std::cerr << "Enumerate is NULL! DirectOutput function not loaded properly." << std::endl;
    return E_NOTIMPL;
}

HRESULT CDirectOutput::SetImage(void* hDevice, DWORD dwPage, DWORD dwIndex, DWORD cbValue, const void* pvValue) {
    if (!m_setimage) {
        std::cerr << "SetImage function is not available!" << std::endl;
        return E_FAIL;
    }
    return m_setimage(hDevice, dwPage, dwIndex, cbValue, pvValue);
}

HRESULT CDirectOutput::RegisterSoftButtonCallback(void* hDevice, Pfn_DirectOutput_SoftButtonChange pfnCb, void* pCtxt) {
    if (m_registersoftbuttoncallback) {
        return m_registersoftbuttoncallback(hDevice, pfnCb, pCtxt);
    }
    return E_FAIL;
}



bool CDirectOutput::GetDirectOutputFilename(wchar_t* filename, DWORD length) {
    bool retval(false);
    HKEY hk;
    
    // Try both DirectOutput and DirectOutput_Saitek
    const wchar_t* registryPaths[] = {
        L"SOFTWARE\\Saitek\\DirectOutput",
        L"SOFTWARE\\Saitek\\DirectOutput_Saitek"
    };

    for (const auto& path : registryPaths) {
        long lRet = RegOpenKeyExW(HKEY_LOCAL_MACHINE, path, 0, KEY_READ, &hk);
        if (ERROR_SUCCESS == lRet) {
            DWORD size = length * sizeof(filename[0]);
            lRet = RegQueryValueExW(hk, L"DirectOutput", 0, 0, (LPBYTE)filename, &size);
            if (ERROR_SUCCESS == lRet) {
                std::wcout << L"DirectOutput DLL Path: " << filename << std::endl;
                retval = true;
            } else {
                std::cerr << "Failed to read registry value for DirectOutput." << std::endl;
            }
            RegCloseKey(hk);
            break; // Stop if we find the registry key
        }
    }

    if (!retval) {
        std::cerr << "Failed to open registry key for DirectOutput." << std::endl;
    }
    return retval;
}
