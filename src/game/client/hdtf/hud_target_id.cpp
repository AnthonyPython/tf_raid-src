#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "c_hdtf_player.h"
#include "c_playerresource.h"
#include "vgui_entitypanel.h"
#include "iclientmode.h"
#include "vgui/ILocalize.h"
#include "hdtf_gamerules.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define PLAYER_HINT_DISTANCE 150
#define PLAYER_HINT_DISTANCE_SQ	( PLAYER_HINT_DISTANCE * PLAYER_HINT_DISTANCE )

static ConVar hud_centerid( "hud_centerid", "1" );
static ConVar hud_showtargetid( "hud_showtargetid", "1" );

class CTargetID : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CTargetID, vgui::Panel );

public:
	CTargetID( const char *pElementName );
	void ApplySchemeSettings( vgui::IScheme *scheme );
	void Paint( );
	void VidInit( );

private:
	Color GetColorForTargetTeam( int iTeamNumber );

	vgui::HFont m_hFont;
	int m_iLastEntIndex;
	float m_flLastChangeTime;
};

DECLARE_HUDELEMENT( CTargetID );

CTargetID::CTargetID( const char *pElementName ) :
	CHudElement( pElementName ), BaseClass( NULL, "TargetID" )
{
	vgui::Panel *pParent = g_pClientMode->GetViewport( );
	SetParent( pParent );

	m_hFont = g_hFontTrebuchet24;
	m_flLastChangeTime = 0;
	m_iLastEntIndex = 0;

	SetHiddenBits( HIDEHUD_MISCSTATUS );
}

void CTargetID::ApplySchemeSettings( vgui::IScheme *scheme )
{
	BaseClass::ApplySchemeSettings( scheme );

	m_hFont = scheme->GetFont( "TargetID", IsProportional( ) );

	SetPaintBackgroundEnabled( false );
}

//-----------------------------------------------------------------------------
// Purpose: clear out string etc between levels
//-----------------------------------------------------------------------------
void CTargetID::VidInit( )
{
	CHudElement::VidInit( );

	m_flLastChangeTime = 0;
	m_iLastEntIndex = 0;
}

Color CTargetID::GetColorForTargetTeam( int iTeamNumber )
{
	return GameResources( )->GetTeamColor( iTeamNumber );
}

//-----------------------------------------------------------------------------
// Purpose: Draw function for the element
//-----------------------------------------------------------------------------
void CTargetID::Paint( )
{
	C_HDTF_Player *pPlayer = C_HDTF_Player::GetLocalHDTFPlayer( );
	if( pPlayer == NULL )
		return;

	// Get our target's ent index
	int iEntIndex = pPlayer->GetIDTarget( );
	// Didn't find one?
	if( iEntIndex == 0 )
	{
		// Check to see if we should clear our ID
		if( m_flLastChangeTime != 0.0f && gpGlobals->curtime > m_flLastChangeTime + 0.5f )
		{
			m_flLastChangeTime = 0.0f;
			m_iLastEntIndex = 0;
		}
		else
		{
			// Keep re-using the old one
			iEntIndex = m_iLastEntIndex;
		}
	}
	else
	{
		m_flLastChangeTime = gpGlobals->curtime;
	}

	// Is this an entindex sent by the server?
	if( iEntIndex == 0 )
		return;

	C_BaseEntity *pEntity = cl_entitylist->GetBaseEntity( iEntIndex );
	if( pEntity == NULL || ( !pEntity->IsNPC( ) && !pEntity->IsPlayer( ) ) )
		return;

	const char *displayName = pEntity->GetDisplayName( );
	if( displayName[0] == '\0' )
		return;

	wchar_t wszName[256] = { L'\0' };
	g_pVGuiLocalize->ConvertANSIToUnicode( displayName, wszName, sizeof( wszName ) );

	wchar_t sIDString[256] = { L'\0' };
	g_pVGuiLocalize->ConstructString( sIDString, sizeof( sIDString ), g_pVGuiLocalize->Find( "#Entityid_sameteam" ), 1, wszName );

	int wide = -1, tall = -1;
	vgui::surface( )->GetTextSize( m_hFont, sIDString, wide, tall );

	int ypos = YRES( 260 ), xpos = XRES( 10 );
	if( hud_centerid.GetInt( ) == 0 )
		ypos = YRES( 420 );
	else
		xpos = ( ScreenWidth( ) - wide ) / 2.0f;

	vgui::surface( )->DrawSetTextFont( m_hFont );
	vgui::surface( )->DrawSetTextPos( xpos, ypos );
	vgui::surface( )->DrawSetTextColor( 0, 255, 0, 255 );
	vgui::surface( )->DrawPrintText( sIDString, wcslen( sIDString ) );
}
