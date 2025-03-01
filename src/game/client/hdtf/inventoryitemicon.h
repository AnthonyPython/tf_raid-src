#pragma once

#include "vgui_controls/Panel.h"

class CInventoryIcon : public vgui::Panel
{
	DECLARE_CLASS_SIMPLE(CInventoryIcon, vgui::Panel);

public:
	CInventoryIcon(vgui::Panel *parent, const char *name);

	void SetSelected(bool state);
	void SetWeapon(CBaseCombatWeapon *pWeapon);
	void Paint();

private:
	CBaseCombatWeapon *m_pCurrentWeapon;
	bool m_bIsSelected;
	Color m_cInactive;
};