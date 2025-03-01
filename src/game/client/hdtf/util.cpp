#include "cbase.h"
#include "util.h"
#include "basecombatweapon_shared.h"
#include "c_baseplayer.h"
#include "c_baseviewmodel.h"

bool UTIL_GetWeaponAttachment( C_BaseCombatWeapon *pWeapon, int attachmentID, Vector &absOrigin, QAngle &absAngles )
{
	// This is already correct in third-person
	if( pWeapon != NULL && pWeapon->IsCarriedByLocalPlayer( ) == false )
		return pWeapon->GetAttachment( attachmentID, absOrigin, absAngles );

	// Otherwise we need to translate the attachment to the viewmodel's version and reformat it
	CBasePlayer *pOwner = ToBasePlayer( pWeapon->GetOwner( ) );
	if( pOwner != NULL )
	{
		int ret = pOwner->GetViewModel( )->GetAttachment( attachmentID, absOrigin, absAngles );
		FormatViewModelAttachment( absOrigin, true );
		return ret;
	}

	// Wasn't found
	return false;
}
