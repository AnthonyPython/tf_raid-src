#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "iclientmode.h"
#include <vgui_controls/Panel.h>
#include <vgui/ISurface.h>

#include "c_hdtf_player.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

class CHudBinocularsOverlay : public Panel, public CHudElement
{
	DECLARE_CLASS_SIMPLE(CHudBinocularsOverlay, Panel);

public:
	CHudBinocularsOverlay(const char *pElementName);

	bool	ShouldDraw();

protected:
	virtual void ApplySchemeSettings(vgui::IScheme *scheme);
	virtual void Paint();

private:
	int m_iOverlayTextureID;
};

DECLARE_HUDELEMENT(CHudBinocularsOverlay);

CHudBinocularsOverlay::CHudBinocularsOverlay(const char *pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudBinoculars")
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent(pParent);

	SetHiddenBits(HIDEHUD_PLAYERDEAD);

	m_iOverlayTextureID = surface()->CreateNewTextureID();
	surface()->DrawSetTextureFile(m_iOverlayTextureID, "overlays/binoculars", true, true);

	SetZPos(-1); // make this panel behind all others
}

void CHudBinocularsOverlay::ApplySchemeSettings(vgui::IScheme *scheme)
{
	BaseClass::ApplySchemeSettings(scheme);

	SetPaintBackgroundEnabled(false);
	SetPaintBorderEnabled(false);

	SetSize(ScreenWidth(), ScreenHeight());

}

bool CHudBinocularsOverlay::ShouldDraw(void)
{
	C_HDTF_Player *pPlayer = dynamic_cast<C_HDTF_Player *>(C_HDTF_Player::GetLocalPlayer());
	if (pPlayer == NULL)
		return false;

	return pPlayer->IsBinocularsActive();
}

void CHudBinocularsOverlay::Paint(void)
{
	int w = GetWide(), h = GetTall();

	int x0, y0, x1, y1;

	x0 = 0;
	y0 = 0;
	x1 = w;
	y1 = h;

	surface()->DrawSetTexture(m_iOverlayTextureID);
	surface()->DrawSetColor(255, 255, 255, 255);
	surface()->DrawTexturedRect(x0, y0, x1, y1);
}
