#pragma once
#include "vgui_controls/Frame.h"
#include "hudelement.h"

#define SLOTS 6

//-----------------------------------------------------------------------------
// Purpose: Draws the inventory
//-----------------------------------------------------------------------------
class CHudInventory : public CHudElement, public vgui::Frame
{
	DECLARE_CLASS_SIMPLE(CHudInventory, vgui::Frame);

public:
	CHudInventory(const char* name);

	void VidInit();

	bool ShouldDraw();

	void UpdateItems();

	void PerformLayout();

	void Paint();

	void PlayInventorySound(const char* snd);
	void StopInventorySound(const char* snd);

	static CHudInventory* GetCurrent();

	void MsgFunc_WeaponChanged(bf_read& msg);

	void ShowInventory();
	void HideInventory();
	bool IsActive() const { return m_bIsActive; }

private:
	void ApplySchemeSettings(vgui::IScheme* pScheme);

	class CInventorySlot* slots[SLOTS];
	class CInventoryAmmo* ammo;
	vgui::Label* emptyTextLabel;

	CPanelAnimationVar(Color, m_flFlashColor, "FlashColor", "255 35 0 255");

	static CHudInventory* current;

	bool m_bIsActive;
};
