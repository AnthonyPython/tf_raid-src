#include "cbase.h"
#include "hdtf_player.h"
#include "weapon_basehdtfcombat.h"

CON_COMMAND( inventory_select, "Selects an item from the inventory to equip (while unequipping the old one)." )
{
	if( args.ArgC( ) < 2 )
		return;

	CHDTF_Player *player = ToHDTFPlayer( UTIL_GetCommandClient( ) );
	const char *item = args.Arg( 1 );

	CBaseHDTFCombatWeapon *active = dynamic_cast<CBaseHDTFCombatWeapon *>( player->GetActiveWeapon() );
	if (active != NULL && !active->CanBeDeselected())
		return;

	Assert( dynamic_cast<CBaseHDTFCombatWeapon *>( player->GetWeaponByName( item ) ) != NULL );
	CBaseHDTFCombatWeapon *weapon = dynamic_cast<CBaseHDTFCombatWeapon *>( player->GetWeaponByName( item ) );
	if( weapon == NULL )
		return;

	int slot = weapon->GetSlot( );
	for( int k = 0; k < player->WeaponCount( ); ++k )
	{
		Assert( dynamic_cast<CBaseHDTFCombatWeapon *>( player->GetWeapon( k ) ) != NULL );
		CBaseHDTFCombatWeapon *wep = dynamic_cast<CBaseHDTFCombatWeapon *>( player->GetWeapon( k ) );
		if( wep == NULL )
			continue;

		if( wep != weapon && wep->GetSlot( ) == slot )
			wep->SetVisibleInWeaponSelection( false );
	}

	weapon->SetVisibleInWeaponSelection( true );

	if (!weapon->HasAnyAmmo())
	{
		CBaseCombatWeapon *pActive = player->GetActiveWeapon();
		if (pActive && pActive->GetSlot() == weapon->GetSlot())
		{
			player->SwitchToNextBestWeapon(pActive);
		}
	}
	else
	{
		player->Weapon_Switch(weapon);
		weapon->InventoryDeploy();
	}

	CRecipientFilter filter;
	filter.AddRecipient(player);
	UserMessageBegin(filter, "WeaponChanged");
		WRITE_STRING(item);
	MessageEnd();
}
