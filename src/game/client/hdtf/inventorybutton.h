#pragma once

#include <vgui/MouseCode.h>
#include <vgui/IScheme.h>
#include "vgui_controls/Button.h"
#include "vgui_controls/Label.h"
#include "c_basecombatweapon.h"

class CInventoryButton : public vgui::Panel
{
	DECLARE_CLASS_SIMPLE(CInventoryButton, vgui::Panel);

public:
	CInventoryButton(Panel *parent, const char *panelName);
	~CInventoryButton();

	void PerformLayout();

	void SetSelected(bool state);
	void SetWeapon(C_BaseCombatWeapon *weapon);
	void Paint();

	bool IsSelected() { return m_bIsSelected; }

	virtual void DoClick();
	virtual void OnMousePressed(vgui::MouseCode code);
	virtual void OnMouseReleased(vgui::MouseCode code);
	virtual void OnCursorEntered();
	virtual void OnCursorExited();

	bool IsHovered();

private:
	C_BaseCombatWeapon *m_pWeapon;
	bool m_bIsSelected;
	bool m_bButtonDown;
	bool m_bIsHovered;
	Color m_InactiveColor;

	CPanelAnimationVar(vgui::HFont, m_hAmmoFont, "AmmoFont", "Default");
	vgui::Label *m_pAmmoLabel;

	KeyValues* m_pMessage;
};
