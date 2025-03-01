#pragma once
#include "cbase.h"

class CClimbablePole : public CBaseEntity
{
public:
	DECLARE_CLASS(CClimbablePole, CBaseEntity);
	DECLARE_DATADESC();

	void Spawn();
	void GetLimits(Vector &out_top, Vector &out_bottom);
	const char *GetIgnoreEntity() { return STRING( sIgnoreEntity ); }

private:
	Vector vTopLimit;
	Vector vBottomLimit;
	string_t sIgnoreEntity;
};
