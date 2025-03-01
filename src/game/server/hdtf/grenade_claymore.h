#pragma once

class CGrenadeClaymore;

CGrenadeClaymore *ClaymoreGrenadeCreate(const Vector &pos,
	const QAngle &ang,
	CBasePlayer *pOwner,
	CBaseEntity *pParent);
