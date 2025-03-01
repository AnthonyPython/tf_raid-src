#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "c_hdtf_player.h"
#include "iclientmode.h"
#include "vgui_controls/Panel.h"
#include "vgui/ISurface.h"
#include "vgui/ILocalize.h"
#include "hdtf_boss.h"
#include "c_ai_basenpc.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: Shows the boss health bar
//-----------------------------------------------------------------------------
class CHudBossHealth : public CHudElement, public vgui::Panel
{
public:
	DECLARE_CLASS_SIMPLE( CHudBossHealth, vgui::Panel );

	CHudBossHealth( const char *pElementName );
	void Init( );
	void Reset( );
	bool ShouldDraw( );
	void ApplySchemeSettings( vgui::IScheme *scheme );

protected:
	void OnThink( );
	void Paint( );

private:
	CPanelAnimationVar( Color, m_BossHealthColor, "BossHealthColor", "255 0 0 220" );
	CPanelAnimationVar( Color, m_BossHealthDamagedColor, "BossHealthDamagedColor", "255 0 0 70" );

	CPanelAnimationVarAliasType( float, m_flBarInsetX, "BarInsetX", "8", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flBarInsetY, "BarInsetY", "15", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flBarWidth, "BarWidth", "592", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flBarHeight, "BarHeight", "4", "proportional_float" );

	CPanelAnimationVar( vgui::HFont, m_hTextFont, "TextFont", "Default" );
	CPanelAnimationVarAliasType( float, text_xpos, "text_xpos", "8", "proportional_float" );
	CPanelAnimationVarAliasType( float, text_ypos, "text_ypos", "4", "proportional_float" );

	int m_nBossInitialHealth;
	int m_nBossHealth;
};

DECLARE_HUDELEMENT( CHudBossHealth );

CHudBossHealth::CHudBossHealth( const char *pElementName ) :
	CHudElement( pElementName ), BaseClass( g_pClientMode->GetViewport( ), "HudBossHealth" )
{
	SetHiddenBits( HIDEHUD_HEALTH | HIDEHUD_PLAYERDEAD );
}

void CHudBossHealth::Init( )
{
	m_nBossInitialHealth = m_nBossHealth = -1;
}

void CHudBossHealth::Reset( )
{
	Init( );
}

//-----------------------------------------------------------------------------
// Purpose: Save CPU cycles by letting the HUD system early cull
// costly traversal.  Called per frame, return true if thinking and 
// painting need to occur.
//-----------------------------------------------------------------------------
bool CHudBossHealth::ShouldDraw( )
{
	return ( m_nBossHealth != -1 || CHDTF_BossBase::m_hBoss != NULL ) && CHudElement::ShouldDraw( );
}

void CHudBossHealth::OnThink( )
{
	C_AI_BaseNPC *pBoss = CHDTF_BossBase::m_hBoss;
	if( pBoss != NULL )
	{
		if( m_nBossInitialHealth == -1 )
			m_nBossInitialHealth = pBoss->GetMaxHealth( );

		m_nBossHealth = pBoss->IsAlive( ) ? pBoss->GetHealth( ) : 0;
	}
	else if( m_nBossInitialHealth != -1 )
	{
		m_nBossInitialHealth = m_nBossHealth = -1;
	}
}

//-----------------------------------------------------------------------------
// Purpose: draws the health bar
//-----------------------------------------------------------------------------
void CHudBossHealth::Paint( )
{
	// draw the boss health bar
	int xpos = m_flBarInsetX,
		ypos = m_flBarInsetY,
		cut = (int)( m_flBarWidth * (float)m_nBossHealth / (float)m_nBossInitialHealth );
	vgui::surface( )->DrawSetColor( m_BossHealthColor );
	vgui::surface( )->DrawFilledRect( xpos, ypos, xpos + cut, ypos + m_flBarHeight );
	xpos += cut;

	// draw the exhausted portion of the bar.
	vgui::surface( )->DrawSetColor( m_BossHealthDamagedColor );
	vgui::surface( )->DrawFilledRect( xpos, ypos, xpos + m_flBarWidth - cut, ypos + m_flBarHeight );

	// draw our name
	vgui::surface( )->DrawSetTextFont( m_hTextFont );
	vgui::surface( )->DrawSetTextColor( m_BossHealthColor );
	vgui::surface( )->DrawSetTextPos( text_xpos, text_ypos );

	wchar_t *tempString = g_pVGuiLocalize->Find( "#HDTF_Hud_BOSS" );
	if( tempString != NULL )
		vgui::surface( )->DrawPrintText( tempString, wcslen( tempString ) );
	else
		vgui::surface( )->DrawPrintText( L"BOSS", wcslen( L"BOSS" ) );
}

void CHudBossHealth::ApplySchemeSettings( vgui::IScheme *scheme )
{
	BaseClass::ApplySchemeSettings( scheme );

	int insetx, insety;
	GetPos( insetx, insety );

	SetWide( ScreenWidth( ) - insetx - insetx );

	m_flBarWidth = GetWide( ) - m_flBarInsetX - m_flBarInsetX;
}
