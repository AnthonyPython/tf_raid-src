#include "cbase.h"
#include "hud_inventory.h"
#include "hud.h"
#include "hud_macros.h"
#include "inventoryammo.h"
#include "inventoryslot.h"
#include "iclientmode.h"
#include "vgui/ISurface.h"
#include "vgui_controls/AnimationController.h"
#include "usermessages.h"
#include "weapon_parkour.h"
#include "c_hdtf_player.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

CHudInventory *CHudInventory::current = NULL;

CHudInventory *CHudInventory::GetCurrent( )
{
	return current;
}

DECLARE_HUDELEMENT( CHudInventory );
DECLARE_HUD_MESSAGE(CHudInventory, WeaponChanged);

void IN_InventoryDown( const CCommand &args )
{
	// we should not open the inventory when we have
	// HIDEHUD_WEAPONSELECTION flag set.
	C_HDTF_Player *localPlayer = ToHDTFPlayer(C_BasePlayer::GetLocalPlayer());
	if (!localPlayer)
		return;

	if (localPlayer->m_Local.m_iHideHUD & HIDEHUD_WEAPONSELECTION)
		return;

	if (!localPlayer->IsInventoryEnabled())
		return;

	if (localPlayer->IsInAVehicle())
		return;

	if (engine->IsPaused())
		return;

	// NOTE(wheatley): when parkour prevents movement modifications we're either hanging from ledge
	// or hanging on a pipe and, unless Mitchell has a third arm, reaching out to his
	// backpack should be impossible
	CWeaponParkour *pParkour = dynamic_cast<CWeaponParkour*>(localPlayer->GetActiveWeapon());
	if (pParkour && !pParkour->AllowCustomizedMovement())
		return;

	localPlayer->ShowInventory();
}

static ConCommand startinventory( "+inventory", IN_InventoryDown );

void IN_InventoryUp( const CCommand &args )
{
	C_HDTF_Player* localPlayer = ToHDTFPlayer(C_BasePlayer::GetLocalPlayer());
	if (!localPlayer)
		return;

	localPlayer->HideInventory();
}

static ConCommand endinventory( "-inventory", IN_InventoryUp );

CHudInventory::CHudInventory( const char *name ) :
	CHudElement( name ), BaseClass(g_pClientMode->GetViewport(), "HudInventory" )
{
	SetScheme(vgui::scheme()->LoadSchemeFromFile("resource/inventoryscheme.res", "InventoryPanelScheme"));
	LoadControlSettings( "Resource/UI/Inventory.res" );
	SetMaximizeButtonVisible( false );
	SetMinimizeButtonVisible( false );
	SetCloseButtonVisible( false );
	SetMenuButtonVisible( false );
	SetMoveable( false );
	SetSizeable( false );
	SetFadeEffectDisableOverride( true );
	SetHiddenBits( HIDEHUD_WEAPONSELECTION | HIDEHUD_PLAYERDEAD );

	current = this;
	m_bIsActive = false;

	HOOK_HUD_MESSAGE(CHudInventory, WeaponChanged);
}

void CHudInventory::VidInit()
{
	LoadControlSettings("Resource/UI/Inventory.res");
}

bool CHudInventory::ShouldDraw( )
{
	return IsVisible( ) && CHudElement::ShouldDraw( );
}

void CHudInventory::UpdateItems( )
{
	for( int k = 0; k < SLOTS; ++k )
		if( slots[k] != NULL )
			slots[k]->UpdateItems( );

	if( ammo != NULL )
		ammo->UpdateItems( );

	InvalidateLayout(true);
}

void CHudInventory::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );
	SetCloseButtonVisible( false );

	char slotlabel[] = "slot0";
	for( int k = 0; k < SLOTS; ++k )
	{
		slotlabel[4] = '1' + k;
		slots[k] = FindControl<CInventorySlot>( slotlabel );
	}

	ammo = FindControl<CInventoryAmmo>( "ammo" );
	emptyTextLabel = FindControl<vgui::Label>("InventoryEmptyLabel");

	if (emptyTextLabel != NULL)
		emptyTextLabel->SetMouseInputEnabled(false);
}

void CHudInventory::PerformLayout()
{
	BaseClass::PerformLayout();

	C_HDTF_Player *localPlayer = ToHDTFPlayer(C_BasePlayer::GetLocalPlayer());
	if (!localPlayer)
		return;

	const bool bHasAnyWeapons = localPlayer->GetWeaponCount() > 0;

	for (int k = 0; k < SLOTS; ++k)
		if (slots[k] != NULL)
			slots[k]->SetVisible(bHasAnyWeapons);

	if (ammo != NULL)
		ammo->SetVisible(bHasAnyWeapons);
}

void CHudInventory::MsgFunc_WeaponChanged(bf_read &msg)
{
	char weapon[64];
	msg.ReadString(weapon, 64);

	for (int i = 0; i < SLOTS; i++)
	{
		slots[i]->ProcessWeaponChange(weapon);
	}
}

void CHudInventory::Paint()
{
	BaseClass::Paint();

	vgui::surface()->DrawSetColor(m_flFlashColor);
	vgui::surface()->DrawFilledRect(0, 0, GetWide(), GetTall());
}

void CHudInventory::PlayInventorySound(const char *snd)
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if (!pPlayer)
		return;

	pPlayer->EmitSound(snd);
}

void CHudInventory::StopInventorySound(const char *snd)
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if (!pPlayer)
		return;

	pPlayer->StopSound(snd);
}

void CHudInventory::ShowInventory()
{
	C_HDTF_Player* localPlayer = ToHDTFPlayer(C_BasePlayer::GetLocalPlayer());
	if (!localPlayer)
		return;

	UpdateItems();
	SetCloseButtonVisible(false);
	Activate();
	PlayInventorySound("Inventory.Open");

	g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("RevealHudNoHide");

	if (localPlayer->GetWeaponCount() <= 0)
	{
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("InventoryRevealEmpty");
	}
	else
	{
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("InventoryReveal");
	}

	m_bIsActive = true;
}



void CHudInventory::HideInventory()
{
	ConVarRef r_hud_fade("r_hud_fade");

	if (!IsActive())
	{
		return;
	}

	PlayInventorySound("Inventory.Close");
	Close();

	if (r_hud_fade.GetBool())
	{
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("HideHud");
	}
	g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("InventoryHide");

	m_bIsActive = false;
}
