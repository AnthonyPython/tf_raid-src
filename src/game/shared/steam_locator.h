#ifndef STEAM_LOCATOR_H
#define STEAM_LOCATOR_H

#ifdef _WIN32
#pragma once
#endif

class ISteamLocator {
public:
    virtual bool LocateSteamDumpsDir(char *buffer, const unsigned int length) = 0;
};

ISteamLocator *GetSteamLocator();

#endif
