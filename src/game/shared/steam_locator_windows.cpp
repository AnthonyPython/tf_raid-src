#if defined( WIN32 ) && !defined( _X360 )
#define _WIN32_WINNT 0x0502
#include <windows.h>
#endif
#include "cbase.h"
#include "steam_locator.h"

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

#ifdef PLATFORM_WINDOWS
// NOTE(wheatley): prevents protected_things.h to get in a way of winapi defs
#undef RegCloseKey

class SteamLocatorWindows : public ISteamLocator
{
public:
    virtual bool LocateSteamDumpsDir(char *buffer, const unsigned int length)
    {
        HKEY hKey;

        const LONG result = RegOpenKeyExW(HKEY_CURRENT_USER, L"SOFTWARE\\Valve\\Steam", NULL, KEY_READ, &hKey);
        if (result != ERROR_SUCCESS)
        {
            return false;
        }

        char tempBuffer[512];
        DWORD readLength = 512;
        const ULONG readResult = RegQueryValueExA(hKey, "SteamPath", NULL, NULL, (LPBYTE)tempBuffer, &readLength);
        RegCloseKey(hKey);
        if (readResult != ERROR_SUCCESS || readLength > (DWORD)length)
        {
            return false;
        }

        Q_snprintf(buffer, length, "%s\\dumps", tempBuffer);
        Q_FixSlashes(buffer);

        return true;
    }
};

ISteamLocator *GetSteamLocator()
{
    static ISteamLocator *instance = nullptr;
    if (!instance)
    {
        instance = new SteamLocatorWindows();
    }

    return instance;
}

#endif