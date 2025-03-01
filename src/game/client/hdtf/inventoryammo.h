#ifndef INVENTORYAMMO_H
#define INVENTORYAMMO_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Label.h>
#include <vgui_controls/Panel.h>
#include <vgui/ISurface.h>

namespace vgui
{
	class Label;
}

class AmmoEntry : public vgui::Panel 
{
	DECLARE_CLASS_SIMPLE(AmmoEntry, vgui::Panel);

public:
	AmmoEntry() {};
	AmmoEntry(vgui::Panel *parent,
		const char *name,
		const int amount,
		C_BaseCombatWeapon *pWeapon,
		bool secondary = false)
		: Panel(parent, name)
	{
		SetScheme(vgui::scheme()->LoadSchemeFromFile("resource/inventoryscheme.res", "InventoryPanelScheme"));
		SetParent(parent);

		ammoName = new vgui::Label(this, name, VarArgs("#HDTF_Ammo_%s", name));
		ammoCount = new vgui::Label(this, name, VarArgs("%i", amount));

		ammoName->SetContentAlignment(vgui::Label::Alignment::a_west);
		ammoName->SetWrap(true);
		ammoCount->SetContentAlignment(vgui::Label::Alignment::a_center);

		assignedWeapon = pWeapon;
		isSecondary = secondary;
	}
	~AmmoEntry()
	{
		ammoName->MarkForDeletion();
		ammoCount->MarkForDeletion();
		assignedWeapon = NULL;
	}

	void SetDisplayInfo(const char *name, const int amount, C_BaseCombatWeapon *pWeapon, bool secondary = false)
	{
		ammoName->SetText(VarArgs("#HDTF_Ammo_%s", name));
		ammoCount->SetText(VarArgs("%i", amount));

		assignedWeapon = pWeapon;
		isSecondary = secondary;
		isEmpty = (amount == 0);

		ammoName->SetFgColor(isEmpty ? clrEmpty : clrTitle);
		ammoCount->SetFgColor(isEmpty ? clrEmpty : clrNormal);
	}

	void ApplySchemeSettings(vgui::IScheme *pScheme)
	{
		BaseClass::ApplySchemeSettings(pScheme);
		clrTitle = GetSchemeColor("Inventory.AmmoTitle", pScheme);
		clrNormal = GetSchemeColor("Inventory.AmmoNumber", pScheme);
		clrEmpty = GetSchemeColor("Inventory.AmmoEmpty", pScheme);

		iconWidth = vgui::scheme()->GetProportionalScaledValue(20);
		counterWidth = vgui::scheme()->GetProportionalScaledValue(24);
	}

	void PerformLayout()
	{
		BaseClass::PerformLayout();

		ammoName->SetPos(iconWidth, 0);
		ammoName->SetSize(GetWide() - counterWidth - iconWidth, GetTall());

		ammoCount->SetPos(GetWide() - counterWidth, 0);
		ammoCount->SetSize(counterWidth, GetTall());
	}

	void Paint()
	{
		if (assignedWeapon)
		{
			CHudTexture *ammoTexture = 
				(isSecondary ? assignedWeapon->GetWpnData().iconAmmo2 : assignedWeapon->GetWpnData().iconAmmo);

			if (!ammoTexture)
				return;

			int x = 0;
			if (ammoTexture->Width() < iconWidth)
			{
				x = iconWidth / 2 - ammoTexture->Width() / 2;
			}

			const Color activeColor = (isEmpty ? clrEmpty : clrNormal);

			const int y = (GetTall() / 2) - (ammoTexture->Height() / 2);
			ammoTexture->DrawSelf(x, y, iconWidth, GetTall(), activeColor);

			const int hPadding = GetTall() / 12;
			vgui::surface()->DrawSetColor(activeColor);
			vgui::surface()->DrawFilledRect(
				GetWide() - counterWidth, hPadding,
				GetWide() - counterWidth + 1, GetTall() - hPadding * 2);
		}
	}

private:
	vgui::Label *ammoName;
	vgui::Label *ammoCount;
	C_BaseCombatWeapon *assignedWeapon;

	Color clrTitle;
	Color clrNormal;
	Color clrEmpty;

	bool isSecondary;
	bool isEmpty;
	int counterWidth;
	int iconWidth;
};

//-----------------------------------------------------------------------------
// Purpose: Draws the inventory slots
//-----------------------------------------------------------------------------
class CInventoryAmmo : public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CInventoryAmmo, vgui::Panel );

public:
	CInventoryAmmo( vgui::Panel *parent, const char *name );

	void UpdateItems( );

	void PerformLayout( );

private:
	void AddItem( const char *name, int amount, C_BaseCombatWeapon *weapon, bool secondary = false );
	void RemoveItem( int idx );
	void Paint();

	CUtlVector<AmmoEntry *> list;
};

#endif // INVENTORYSLOT_H
