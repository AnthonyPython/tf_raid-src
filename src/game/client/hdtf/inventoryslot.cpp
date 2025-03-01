#include "cbase.h"
#include "inventoryslot.h"
#include "vgui_controls/Label.h"
#include "vgui_controls/ImagePanel.h"
#include "vgui_controls/RadioButton.h"
#include "vgui_controls/Controls.h"
#include "vgui/ISurface.h"

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

#define MAX_ITEMS_PER_SLOT 6

DECLARE_BUILD_FACTORY(CInventorySlot);

CInventorySlot::CInventorySlot(vgui::Panel *parent, const char *name) :
	BaseClass(parent, name), slot(-1), label(NULL), icon(NULL), current(-1)
{
	m_flAnimPerc = 0.f;

	label = new vgui::Label(this, "Label", "Slot 0");
	label->SetContentAlignment(vgui::Label::a_west);

	//icon = new CInventoryIcon(this, name);
}

CInventorySlot::~CInventorySlot()
{
	if (current != -1)
	{
		list[current].button->MarkForDeletion();
		current = -1;
	}
}

void CInventorySlot::RecomputePositions(bool applyChanges)
{
	int xpos = 0;
	int unused = 0;

	if (current != -1)
	{
		int temp = 0;
		list[current].button->GetPos(temp, unused);
		list[current].lastPos = temp;

		if (applyChanges)
		{
			list[current].button->SetWide(GetWide() / MAX_ITEMS_PER_SLOT);
			list[current].button->SetPos(xpos, 0);
			list[current].lastPos = xpos;
		}

		list[current].pos = xpos;
		xpos += list[current].button->GetWide();
	}

	for (int k = 0; k < list.Count(); ++k)
	{
		if (!list[k].button)
			continue;

		if (current == k)
			continue;

		list[k].button->GetPos(list[k].lastPos, unused);

		if (applyChanges)
		{
			list[k].button->SetWide(GetWide() / MAX_ITEMS_PER_SLOT);
			list[k].button->SetPos(xpos, 0);
			list[k].lastPos = xpos;
		}

		list[k].pos = xpos;
		xpos += list[k].button->GetWide();
	}
}

void CInventorySlot::AnimateCandidats()
{
	for (int k = 0; k < list.Count(); ++k)
	{
		int pos = Lerp(SmoothCurve(m_flAnimPerc * 0.5f), list[k].lastPos, list[k].pos);
		list[k].button->SetPos(pos, 0);
	}
}

void CInventorySlot::OnThink()
{
	BaseClass::OnThink();

	if (m_bMoving)
	{
		if (m_flAnimPerc == 1.f)
		{
			m_bMoving = false;
			return;
		}

		m_flAnimPerc = Approach(1.f, m_flAnimPerc, gpGlobals->frametime * 4);
		AnimateCandidats();
	}
}

void CInventorySlot::Paint()
{
	BaseClass::Paint();

	vgui::surface()->DrawSetColor(100, 100, 100, 100);
	for (int i = 0; i < list.Count(); i++)
	{
		if (i == 0)
			continue;

		int x = (GetWide() / MAX_ITEMS_PER_SLOT) * i;
		vgui::surface()->DrawLine(x, 0, x, GetTall());
	}
}

void CInventorySlot::UpdateItems()
{
	//icon->SetWeapon(NULL);

	current = -1;

	int idx = 0;
	C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();
	for (int k = 0; k < player->WeaponCount(); ++k)
	{
		C_BaseCombatWeapon *weapon = player->GetWeapon(k);
		if (weapon == NULL || weapon->GetSlot() != slot)
			continue;

		if (idx < list.Count())
		{
			list[idx].classname = weapon->GetName();
			list[idx].button->SetWeapon(weapon);

			if (weapon->VisibleInWeaponSelection())
			{
				list[idx].button->SetSelected(true);
				current = idx;
			}
			else
				list[idx].button->SetSelected(false);
		}
		else
		{
			AddItem(weapon);
			if (weapon->VisibleInWeaponSelection())
			{
				list[idx].button->SetSelected(true);
				current = idx;
			}
		}

		++idx;
	}

	if (idx < list.Count())
	{
		for (int k = idx; k < list.Count(); ++k)
			list[k].button->MarkForDeletion();

		list.SetCount(idx);
	}

	InvalidateLayout();
}

void CInventorySlot::ApplySettings(KeyValues *rsrc)
{
	BaseClass::ApplySettings(rsrc);

	slot = rsrc->GetInt("slot");
	Assert(slot >= 0 && slot <= 5);

	char slotlabel[] = "#HDTF_Inventory_Slot1";
	slotlabel[20] = '1' + slot;
	label->SetText(slotlabel);
}

void CInventorySlot::PerformLayout()
{
	BaseClass::PerformLayout();

	int wide = GetWide(), cwide = 0, ctall = 0;
	label->SetWide(wide);
	label->GetContentSize(cwide, ctall);
	label->SetTall(ctall);
	label->SetPos(0, 0);

	//icon->SetWide(wide);
	//icon->SetTall(wide);
	//icon->SetPos(0, ctall);

	RecomputePositions();
	/*int ypos = ctall + wide + 5;
	for (int k = 0; k < list.Count(); ++k)
	{
		if (!list[k].button)
			continue;

		list[k].button->SetWide(wide);
		list[k].button->SetPos(0, ypos);
		list[k].posY = ypos;
		ypos += list[k].button->GetTall() + 5;
	}*/
}

void CInventorySlot::OnWeaponClick(Panel *panel)
{
	if (current != -1 && list[current].button == panel)
		return;

	for (int k = 0; k < list.Count(); ++k)
		if (panel == list[k].button)
		{
			sprintf(m_pRequestedWeapon, "%s", list[k].classname);
		}

	engine->ServerCmd(VarArgs("inventory_select %s", m_pRequestedWeapon));
}

Candidate *CInventorySlot::AddItem(C_BaseCombatWeapon *weapon)
{
	Candidate candidate;
	candidate.pos = 0;
	candidate.classname = weapon->GetName();
	candidate.button = new CInventoryButton(this, candidate.classname);
	candidate.button->SetWeapon(weapon);
	candidate.button->AddActionSignalTarget(this);
	candidate.button->SetTall(GetTall());
	list.AddToTail(candidate);
	return &list[list.Count() - 1];
}

void CInventorySlot::RemoveItem(int idx)
{
	if (idx < list.Count())
	{
		list[idx].button->MarkForDeletion();
		list.Remove(idx);
	}
}

void CInventorySlot::ProcessWeaponChange(const char *weaponClass)
{
	if (!strcmp(m_pRequestedWeapon, weaponClass))
	{
		//C_BaseCombatWeapon *pWeapon = C_BasePlayer::GetLocalPlayer()->Weapon_OwnsThisType(m_pRequestedWeapon);

		//Assert(pWeapon != NULL);

		//icon->SetWeapon(pWeapon);

		for (int i = 0; i < list.Count(); i++)
		{
			if (!strcmp(list[i].classname, weaponClass))
			{
				for (int x = 0; x < list.Count(); x++)
				{
					if(list[x].button != list[i].button)
						list[x].button->SetSelected(false);
				}

				current = i;
				break;
			}
		}

		RecomputePositions(false);

		m_flAnimPerc = 0.f;
		m_bMoving = true;
	}
}
