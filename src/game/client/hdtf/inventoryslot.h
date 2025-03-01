#ifndef INVENTORYSLOT_H
#define INVENTORYSLOT_H

#ifdef _WIN32
#pragma once
#endif

#include "vgui_controls/Panel.h"
#include "inventoryitemicon.h"
#include "inventorybutton.h"
#include "utlvector.h"

namespace vgui
{
	class Label;
	class ImagePanel;
}

class C_BaseCombatWeapon;
class KeyValues;

struct Candidate
{
	int pos;
	int lastPos;
	const char *classname;
	CInventoryButton *button;
};

//-----------------------------------------------------------------------------
// Purpose: Draws the inventory slots
//-----------------------------------------------------------------------------
class CInventorySlot : public vgui::Panel
{
	DECLARE_CLASS_SIMPLE(CInventorySlot, vgui::Panel);

public:
	CInventorySlot(vgui::Panel *parent, const char *name);
	~CInventorySlot();

	void UpdateItems();

	void ApplySettings(KeyValues *rsrc);

	void OnThink();
	void Paint();

	void PerformLayout();
	void RecomputePositions(bool applyChanges = true);
	void AnimateCandidats();

	void ProcessWeaponChange(const char *weaponClass);

	MESSAGE_FUNC_PTR(OnWeaponClick, "OnWeaponClick", panel);
	//MESSAGE_FUNC_PTR_INT(OnWeaponClick, "OnWeaponClick", panel, tabposition);

private:
	Candidate *AddItem(C_BaseCombatWeapon *weapon);
	void RemoveItem(int idx);

	bool m_bMoving;
	float m_flAnimPerc;
	char m_pRequestedWeapon[64];

	int slot;
	vgui::Label *label;
	CInventoryIcon *icon;
	CUtlVector<Candidate> list;
	int current;
};

#endif // INVENTORYSLOT_H
