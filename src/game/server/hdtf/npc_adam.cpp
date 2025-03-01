#include "cbase.h"
#include "npc_barney.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//	>> CNPC_Adam
//=========================================================
class CNPC_Adam : public CNPC_Barney
{
	DECLARE_CLASS( CNPC_Adam, CNPC_Barney );

	Class_T m_Class;

public:
	virtual void PostConstructor( const char *szClassname );

	virtual void SelectModel( );

	virtual Vector GetActualShootTrajectory(const Vector &shootOrigin);

	void OnUpdateShotRegulator();

	virtual Class_T Classify( );
};

LINK_ENTITY_TO_CLASS( npc_adam_young, CNPC_Adam );
LINK_ENTITY_TO_CLASS( npc_adam_old, CNPC_Adam );

void CNPC_Adam::PostConstructor( const char *szClassname )
{
	BaseClass::PostConstructor( szClassname );

	m_Class =
		FClassnameIs( this, "npc_adam_young" ) ? CLASS_PLAYER_ALLY_VITAL : CLASS_PLAYER_COMBINE_ALLY_VITAL;
}

void CNPC_Adam::SelectModel( )
{
	// Allow multiple models, but default to young_adam.mdl or old_adam.mdl
	char* szModel = (char*)STRING(GetModelName());
	
	if (FClassnameIs(this, "npc_adam_young"))
	{
		if (!szModel || !*szModel)
		{
			szModel = "models/Protagonists/young_adam.mdl";
			SetModelName(AllocPooledString(szModel));
		}
	}
	else
	{
		if (!szModel || !*szModel)
		{
			szModel = "models/Protagonists/old_adam.mdl";
			SetModelName(AllocPooledString(szModel));
		}
	}
}

Class_T CNPC_Adam::Classify( )
{
	return m_Class;
}

// if enemies come too close Adam won't need much of a time to aim at them
#define ADAM_FAST_AIM_TIME_RANGE (12.0f * 8.0f)
void CNPC_Adam::OnUpdateShotRegulator()
{
	if (GetEnemy() && HasCondition(COND_CAN_RANGE_ATTACK1))
	{
		CBaseCombatWeapon *pWeapon = GetActiveWeapon();
		if (pWeapon && pWeapon->ClassMatches("weapon_awp"))
		{
			if (GetAbsOrigin().DistTo(GetEnemy()->GetAbsOrigin()) <= ADAM_FAST_AIM_TIME_RANGE)
			{
				GetShotRegulator()->SetBurstShotsRemaining(0);
				GetShotRegulator()->SetRestInterval(pWeapon->GetMinRestTime(), pWeapon->GetMaxRestTime() / 2.25f);
				GetShotRegulator()->SetBurstInterval(pWeapon->GetMinRestTime(), pWeapon->GetMaxRestTime() / 2.25f);
			}
			else
			{
				GetShotRegulator()->SetBurstShotsRemaining(0);
				GetShotRegulator()->SetRestInterval(pWeapon->GetMinRestTime(), pWeapon->GetMaxRestTime());
				GetShotRegulator()->SetBurstInterval(pWeapon->GetMinRestTime(), pWeapon->GetMaxRestTime());
			}

			return;
		}
	}

	BaseClass::OnUpdateShotRegulator();
}

// if Adam uses sniper rifle he should aim for the head instead of body
Vector CNPC_Adam::GetActualShootTrajectory(const Vector &shootOrigin)
{
	CBaseCombatWeapon *pWeapon = GetActiveWeapon();
	if (pWeapon && pWeapon->ClassMatches("weapon_awp"))
	{
		if (!GetEnemy())
			return GetShootEnemyDir(shootOrigin);

		CBaseAnimating *pEnemy = dynamic_cast<CBaseAnimating *>(GetEnemy());
		if (pEnemy)
		{
			// look for the eyes
			const int eyes = pEnemy->LookupAttachment("eyes");
			if (eyes)
			{
				Vector eyePos;
				pEnemy->GetAttachment(eyes, eyePos);

				Vector result = eyePos - shootOrigin;
				VectorNormalize(result);

				return result;
			}
		}
	}

	return BaseClass::GetActualShootTrajectory(shootOrigin);
}
