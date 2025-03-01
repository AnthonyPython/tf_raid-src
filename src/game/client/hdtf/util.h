#ifndef UTIL_CLIENT_H
#define UTIL_CLIENT_H

#ifdef _WIN32
#pragma once
#endif

class C_BaseCombatWeapon;
class Vector;
class QAngle;

bool UTIL_GetWeaponAttachment( C_BaseCombatWeapon *pWeapon, int attachmentID, Vector &absOrigin, QAngle &absAngles );

#endif // UTIL_CLIENT_H
