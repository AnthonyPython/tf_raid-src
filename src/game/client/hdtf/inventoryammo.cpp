#include <cbase.h>
#include "inventoryammo.h"
#include <ammodef.h>
#include <vgui_controls/Label.h>

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

DECLARE_BUILD_FACTORY( CInventoryAmmo );

CInventoryAmmo::CInventoryAmmo( vgui::Panel *parent, const char *name ) :
	BaseClass( parent, name )
{ }

void CInventoryAmmo::UpdateItems( )
{
	int idx = 0;
	CUtlVector<int> ammoTypes;
	CAmmoDef *ammodef = GetAmmoDef();

	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	for (int i = 0; i < pPlayer->WeaponCount(); i++)
	{
		CBaseCombatWeapon *pWeapon = pPlayer->GetWeapon(i);
		if (pWeapon == NULL)
			continue;

		int iPrimary = pWeapon->GetPrimaryAmmoType();
		int iSecondary = pWeapon->GetSecondaryAmmoType();

		// we may have two or more weapons with the same
		// ammo type at the same time. do not add them twice
		if (iPrimary != -1 && !ammoTypes.HasElement(iPrimary) && pWeapon->UsesPrimaryAmmo())
		{
			Ammo_t *ammo = ammodef->GetAmmoOfIndex(iPrimary);

			int total = pPlayer->GetAmmoCount(iPrimary);
			if (pWeapon->UsesClipsForAmmo1())
				total += pWeapon->Clip1();

			if (idx < list.Count())
			{
				list[idx]->SetDisplayInfo(ammo->pName, total, pWeapon);
			}
			else
				AddItem(ammo->pName, total, pWeapon);

			ammoTypes.AddToTail(iPrimary);
			idx++;
		}

		// same for the secondary ammo type
		if (iSecondary != -1 && !ammoTypes.HasElement(iSecondary) && pWeapon->UsesSecondaryAmmo())
		{
			Ammo_t *ammo = ammodef->GetAmmoOfIndex(iSecondary);

			int total = pPlayer->GetAmmoCount(iSecondary);
			if (pWeapon->UsesClipsForAmmo2())
				total += pWeapon->Clip2();

			if (idx < list.Count())
			{
				list[idx]->SetDisplayInfo(ammo->pName, total, pWeapon, true);
			}
			else
				AddItem(ammo->pName, total, pWeapon, true);

			ammoTypes.AddToTail(iSecondary);
			idx++;
		}

	}

	if (idx < list.Count())
	{
		for (int k = idx; k < list.Count(); ++k)
			list[k]->MarkForDeletion();

		list.SetCount(idx);
	}

	InvalidateLayout( );
}

void CInventoryAmmo::PerformLayout( )
{
	BaseClass::PerformLayout( );

	int wide = GetWide( ), ypos = 0;
	for( int k = 0; k < list.Count( ); ++k )
	{
		list[k]->SetPos(0, ypos);
		list[k]->SetWide(wide);
		list[k]->SetTall(vgui::scheme()->GetProportionalScaledValue(19));
		ypos += list[k]->GetTall();
	}
}

void CInventoryAmmo::AddItem( const char *name, int amount, C_BaseCombatWeapon *weapon, bool secondary )
{
	list.AddToTail(new AmmoEntry(this, name, amount, weapon, secondary));
}

void CInventoryAmmo::RemoveItem( int idx )
{
	if( idx < list.Count( ) )
	{
		list[idx]->MarkForDeletion( );
		list.Remove( idx );
	}
}

void CInventoryAmmo::Paint()
{
	vgui::surface()->DrawSetColor(255, 35, 0, 50);

	const int offset = vgui::scheme()->GetProportionalScaledValue(19);
	const int sides = vgui::scheme()->GetProportionalScaledValue(4);
	for (int i = 0; i < list.Count(); i++)
	{
		if (i == 0)
			continue;

		const int y = i * offset - 1;
		vgui::surface()->DrawLine(sides, y, GetWide() - sides * 2, y);
	}
}