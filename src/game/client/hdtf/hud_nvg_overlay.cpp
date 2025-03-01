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

class CHudNVGOverlay : public Panel, public CHudElement
{
	DECLARE_CLASS_SIMPLE(CHudNVGOverlay, Panel);

public:
	CHudNVGOverlay(const char *pElementName);

	bool	ShouldDraw();

protected:
	virtual void ApplySchemeSettings(vgui::IScheme *scheme);
	virtual void Paint();

private:
	int m_iNVGTextureID;
	int m_iGrainTextureID;
	int m_iScanLinesID;
};

DECLARE_HUDELEMENT(CHudNVGOverlay);

CHudNVGOverlay::CHudNVGOverlay(const char *pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudNVG")
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent(pParent);

	SetHiddenBits(HIDEHUD_PLAYERDEAD);

	m_iNVGTextureID = surface()->CreateNewTextureID();
	surface()->DrawSetTextureFile(m_iNVGTextureID, "overlays/nvg_overlay", true, true);

	m_iGrainTextureID = surface()->CreateNewTextureID();
	surface()->DrawSetTextureFile(m_iGrainTextureID, "overlays/nvg_grain", true, true);

	m_iScanLinesID = surface()->CreateNewTextureID();
	surface()->DrawSetTextureFile(m_iScanLinesID, "overlays/nvg_scanlines", true, true);

	SetZPos(-1); // make this panel behind all others
}

void CHudNVGOverlay::ApplySchemeSettings(vgui::IScheme *scheme)
{
	BaseClass::ApplySchemeSettings(scheme);

	SetPaintBackgroundEnabled(false);
	SetPaintBorderEnabled(false);

	SetSize(ScreenWidth(), ScreenHeight());

}

bool CHudNVGOverlay::ShouldDraw(void)
{
	C_HDTF_Player *pPlayer = dynamic_cast<C_HDTF_Player *>(C_HDTF_Player::GetLocalPlayer());
	if (pPlayer == NULL)
		return false;

	return pPlayer->IsNightVisionActive();
}

void CHudNVGOverlay::Paint(void)
{
	int w = GetWide(), h = GetTall();
	
	int x0, y0, x1, y1;

	x0 = 0;
	y0 = h / 2 - w / 2;
	x1 = w;
	y1 = y0 + w;

	surface()->DrawSetTexture(m_iScanLinesID);
	surface()->DrawSetColor(32, 192, 0, 10);

	for (int x = 0; x < w; x += 512)
		for (int y = 0; y < h; y += 512)
			surface()->DrawTexturedRect(x, y, x + 512, y + 512);

	surface()->DrawSetTexture(m_iNVGTextureID);
	surface()->DrawSetColor(255, 255, 255, 255);
	surface()->DrawTexturedRect(x0, y0, x1, y1);

	surface()->DrawSetTexture(m_iGrainTextureID);
	for (int x = 0; x < w; x += 512)
		for (int y = 0; y < h; y += 512)
			surface()->DrawTexturedRect(x, y, x + 512, y + 512);

}
