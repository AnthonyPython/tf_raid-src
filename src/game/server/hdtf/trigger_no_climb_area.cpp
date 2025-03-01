#include "cbase.h"
#include "triggers.h"
#include "hdtf_player.h"
#include "weapon_parkour.h"

class CTriggerNoClimbArea : public CBaseTrigger
{
	DECLARE_CLASS(CTriggerNoClimbArea, CBaseTrigger);

public:
	void Spawn(void);
	void Touch(CBaseEntity *pOther);
	void EndTouch(CBaseEntity *pOther);
};

LINK_ENTITY_TO_CLASS(trigger_no_climb_area, CTriggerNoClimbArea);

void CTriggerNoClimbArea::Spawn()
{
	BaseClass::Spawn();
	InitTrigger();
}

void CTriggerNoClimbArea::Touch(CBaseEntity *pOther)
{
	CHDTF_Player *pPlayer = dynamic_cast<CHDTF_Player*>(pOther);

	if (!pPlayer)
		return;

	pPlayer->m_pParkourController.DisallowClimb();
}

void CTriggerNoClimbArea::EndTouch(CBaseEntity *pOther)
{
	CHDTF_Player* pPlayer = dynamic_cast<CHDTF_Player*>(pOther);

	if (!pPlayer)
		return;

	pPlayer->m_pParkourController.AllowClimb();

	BaseClass::EndTouch(pOther);
}
