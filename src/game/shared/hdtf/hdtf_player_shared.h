#ifndef HDTF_PLAYER_SHARED_H
#define HDTF_PLAYER_SHARED_H

#ifdef _WIN32
#pragma once
#endif

#ifdef CLIENT_DLL

#define CHDTF_Player C_HDTF_Player
#include "c_hdtf_player.h"

#else

#include "hdtf_player.h"

#endif

#define TIME_TO_PRONE 0.6f

#define TIME_TO_LEAN 0.2f
#define LEANING_DEGREES 25.0f
#define LEANING_RADIANS ( LEANING_DEGREES * M_PI_F / 180.0f )

#define HDTF_PLAYER_ARM_SKIN_MAX 4

#endif // HDTF_PLAYER_SHARED_H
