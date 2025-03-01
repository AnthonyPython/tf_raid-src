#include "cbase.h"
#include "func_climbable_pole.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS(func_climbable_pole, CClimbablePole);

BEGIN_DATADESC(CClimbablePole)
	DEFINE_FIELD(vTopLimit, FIELD_VECTOR),
	DEFINE_FIELD(vBottomLimit, FIELD_VECTOR),
	DEFINE_KEYFIELD(sIgnoreEntity, FIELD_STRING, "ignorename"),
END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: Sets up the entity's initial state
//-----------------------------------------------------------------------------
void CClimbablePole::Spawn()
{
	BaseClass::Spawn();

	SetSolid(SOLID_VPHYSICS); // change to SOLID_BBOX?
	SetSolidFlags(FSOLID_FORCE_WORLD_ALIGNED);
	SetCollisionGroup(COLLISION_GROUP_PLAYER);

	SetModel(STRING(GetModelName()));
	SetMoveType(MOVETYPE_NONE);

	AddEffects(EF_NODRAW);

	vBottomLimit = WorldAlignMins();
	vTopLimit = WorldAlignMaxs();
}

//-----------------------------------------------------------------------------
// Purpose: Returns top and bottom limits of the pole entity
//-----------------------------------------------------------------------------
void CClimbablePole::GetLimits(Vector &out_top, Vector &out_bottom)
{
	out_top = vTopLimit;
	out_bottom = vBottomLimit;
}
