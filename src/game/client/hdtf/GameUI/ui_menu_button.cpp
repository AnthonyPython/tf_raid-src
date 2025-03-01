#include "cbase.h"
#include "ui_menu_button.h"

#include "vgui/ISurface.h"

using namespace vgui;

HDTFMenuButton::HDTFMenuButton(Panel *parent, const char *name, const char *text) : Button(parent, name, text)
{
	SetParent(parent);

	m_iGradient = surface()->CreateNewTextureID();
	surface()->DrawSetTextureFile(m_iGradient, "vgui/gradient_h", true, true);

	m_bIsHovered = false;
}

void HDTFMenuButton::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	SetFont(pScheme->GetFont("MainMenuButtons", false));
	SetFgColor(Color(255, 255, 255, 255));
	SetSelectedColor(Color(100, 5, 0, 255), Color(0, 0, 0, 0));
	SetArmedColor(Color(255, 25, 0, 255), Color(0, 0, 0, 0));
	SetBlinkColor(Color(255, 25, 0, 255));
	SetDisabledFgColor1(Color(25, 25, 25, 255));
	SetDisabledFgColor2(Color(25, 25, 25, 255));
	SetTextInset(scheme()->GetProportionalScaledValue(25), 0);

	SetPaintBorderEnabled(false);
}

void HDTFMenuButton::SetEnabled(bool state)
{
	BaseClass::SetEnabled(state);
	
	if (!state)
	{
		OnHoverStateChanged(false);
		m_bIsHovered = false;
	}
}

void HDTFMenuButton::FireActionSignal()
{
	BaseClass::FireActionSignal();

	OnHoverStateChanged(false);
	m_bIsHovered = false;
}

void HDTFMenuButton::Paint()
{
	BaseClass::Paint();

	if (IsHovered())
	{
		// HACK(wheatley): there's a frame-perfect glitch in VGUI where you rapidly move mouse over
		// the button and clicking the LMB, which causes OnCursorEntered to fire after the mouse have
		// already left the panel
		if (!IsCursorOver())
		{
			OnCursorExited();
			return;
		}

		surface()->DrawSetColor(255, 25, 0, 255);
		surface()->DrawFilledRect(0, 0, GetTall() / 2.5f, GetTall());

		surface()->DrawSetColor(255, 25, 0, 15);
		surface()->DrawSetTexture(m_iGradient);
		surface()->DrawTexturedRect(GetTall() / 2.5f, 0, GetWide() / 2, GetTall());
	}
}

void HDTFMenuButton::PaintBackground()
{
}

void HDTFMenuButton::OnHoverStateChanged(bool newState)
{
	if (newState)
	{
		SetFgColor(Color(255, 25, 0, 255));
	}
	else
	{
		SetFgColor(Color(255, 255, 255, 255));
	}
}
