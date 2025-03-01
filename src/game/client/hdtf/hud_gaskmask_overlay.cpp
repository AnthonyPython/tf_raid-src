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

class CHudGaskmaskOverlay : public Panel, public CHudElement
{
	DECLARE_CLASS_SIMPLE(CHudGaskmaskOverlay, Panel);

public:
	CHudGaskmaskOverlay(const char *pElementName);

	bool	ShouldDraw();

protected:
	virtual void ApplySchemeSettings(vgui::IScheme *scheme);
	virtual void Paint();

private:
	int m_iTextureID;
};

DECLARE_HUDELEMENT(CHudGaskmaskOverlay);

CHudGaskmaskOverlay::CHudGaskmaskOverlay(const char *pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudGaskmask")
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent(pParent);

	SetHiddenBits(HIDEHUD_PLAYERDEAD);

	m_iTextureID = surface()->CreateNewTextureID();
	surface()->DrawSetTextureFile(m_iTextureID, "overlays/gaskmask_overlay", true, true);

	SetZPos(-1); // make this panel behind all others
}

void CHudGaskmaskOverlay::ApplySchemeSettings(vgui::IScheme *scheme)
{
	BaseClass::ApplySchemeSettings(scheme);

	SetPaintBackgroundEnabled(false);
	SetPaintBorderEnabled(false);

	SetSize(ScreenWidth(), ScreenHeight());
}

bool CHudGaskmaskOverlay::ShouldDraw(void)
{
	C_HDTF_Player *pPlayer = dynamic_cast<C_HDTF_Player *>(C_HDTF_Player::GetLocalPlayer());
	if (pPlayer == NULL)
		return false;

	return pPlayer->IsGasMaskActive();
}

void CHudGaskmaskOverlay::Paint(void)
{
	int w = GetWide(), h = GetTall();
	
	int x0, y0, x1, y1;

	x0 = 0;
	y0 = h / 2 - w / 2;
	x1 = w;
	y1 = y0 + w;

	surface()->DrawSetTexture(m_iTextureID);
	surface()->DrawSetColor(255, 255, 255, 255);
	surface()->DrawTexturedRect(x0, y0, x1, y1);
}
