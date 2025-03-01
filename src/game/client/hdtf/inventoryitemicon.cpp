#include "cbase.h"
#include "iclientmode.h"
#include <vgui/ISurface.h>
#include "basecombatweapon_shared.h"
#include "inventoryitemicon.h"

CInventoryIcon::CInventoryIcon(vgui::Panel *parent, const char *name) :
	BaseClass(parent, name)
{
	m_pCurrentWeapon = NULL;
	m_bIsSelected = false;
	m_cInactive = Color(100, 100, 100, 255);
}

void CInventoryIcon::SetWeapon(CBaseCombatWeapon *pWeapon)
{
	m_pCurrentWeapon = pWeapon;
}

void CInventoryIcon::SetSelected(bool state)
{
	m_bIsSelected = state;
}

void CInventoryIcon::Paint()
{
	if (m_pCurrentWeapon == NULL)
		return;

	if (m_pCurrentWeapon->GetSpriteActive())
	{
		int w = m_pCurrentWeapon->GetSpriteActive()->Width();
		int h = m_pCurrentWeapon->GetSpriteActive()->Height();

		int x = (GetWide() / 2) - (w / 2);
		int y = (GetTall() / 2) - (h / 2);

		m_pCurrentWeapon->GetSpriteInactive()->DrawSelf(x, y, (m_bIsSelected ? GetFgColor() : m_cInactive));
	}
}