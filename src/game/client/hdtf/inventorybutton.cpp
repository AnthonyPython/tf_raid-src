#include "cbase.h"
#include "inventorybutton.h"
#include <vgui/IInput.h>
#include <vgui/ISurface.h>
#include "ammodef.h"

CInventoryButton::CInventoryButton(Panel *parent, 
	const char *panelName) : Panel(parent, panelName)
{
	m_pWeapon = NULL;
	m_bIsSelected = false;
	m_InactiveColor = Color(100, 100, 100, 255);

	m_pMessage = new KeyValues("OnWeaponClick", "command", "weaponselect");

	m_bButtonDown = false;

	m_pAmmoLabel = new vgui::Label(this, "ammotype", "ammo");

	m_bIsHovered = false;
}

CInventoryButton::~CInventoryButton()
{
	m_pWeapon = NULL;

	if (m_pAmmoLabel)
		m_pAmmoLabel->MarkForDeletion();

	if (m_pMessage)
		m_pMessage->deleteThis();
}

void CInventoryButton::PerformLayout()
{
	BaseClass::PerformLayout();

	m_pAmmoLabel->SizeToContents();
	m_pAmmoLabel->SetWide(GetWide());
	m_pAmmoLabel->SetPos(0, GetTall() - m_pAmmoLabel->GetTall());
	m_pAmmoLabel->SetFont(m_hAmmoFont);
	m_pAmmoLabel->SetContentAlignment(vgui::Label::a_east);
	m_pAmmoLabel->SetFgColor(IsSelected() ? GetFgColor() : m_InactiveColor);

	m_bIsHovered = false;
}

void CInventoryButton::SetSelected(bool state)
{
	m_bIsSelected = state;

	m_pAmmoLabel->SetFgColor(IsSelected() ? GetFgColor() : m_InactiveColor);
}

void CInventoryButton::SetWeapon(C_BaseCombatWeapon *weapon)
{
	m_pWeapon = weapon;

	m_pAmmoLabel->SetVisible(false);

	if (weapon->UsesClipsForAmmo1())
	{
		CAmmoDef *ammodef = GetAmmoDef();
		Ammo_t *ammo = ammodef->GetAmmoOfIndex(weapon->GetPrimaryAmmoType());
		
		if (ammo)
		{
			m_pAmmoLabel->SetText(VarArgs("#HDTF_Ammo_Short_%s", ammo->pName));
			m_pAmmoLabel->SetVisible(true);
		}
	}
}

void CInventoryButton::OnCursorEntered()
{
	m_bIsHovered = true;
}

void CInventoryButton::OnCursorExited()
{
	m_bIsHovered = false;
}

bool CInventoryButton::IsHovered()
{
	return m_bIsHovered;
}

void CInventoryButton::OnMousePressed(vgui::MouseCode code)
{
	if (!IsEnabled())
		return;

	m_bButtonDown = true;
}

void CInventoryButton::OnMouseReleased(vgui::MouseCode code)
{
	if (IsEnabled() && (GetVPanel() == vgui::input()->GetMouseOver() || m_bButtonDown))
	{
		DoClick();
	}

	m_bButtonDown = false;
}

void CInventoryButton::DoClick()
{
	if (m_pWeapon == NULL)
		return;

	SetSelected(true);

	PostActionSignal(m_pMessage->MakeCopy());
}

void CInventoryButton::Paint()
{
	if (m_pWeapon == NULL)
		return;

	if (IsHovered())
	{
		if(IsSelected())
			vgui::surface()->DrawSetColor(255, 50, 0, 10);
		else
			vgui::surface()->DrawSetColor(255, 255, 255, 4);

		vgui::surface()->DrawFilledRect(0, 0, GetWide(), GetTall());
	}

	if (m_pWeapon->GetSpriteActive())
	{
		int w = m_pWeapon->GetSpriteActive()->Width();
		int h = m_pWeapon->GetSpriteActive()->Height();

		int x = (GetWide() / 2) - (w / 2);
		int y = (GetTall() / 2) - (h / 2);

		m_pWeapon->GetSpriteInactive()->DrawSelf(x, y, (IsSelected() ? GetFgColor() : m_InactiveColor));
	}
}
