#include "cbase.h"
#include "hdtf_player.h"
#include "hdtf_gamerules.h"
#include "weapon_basehdtfcombat.h"
#include "soundenvelope.h"
#include "in_buttons.h"
#include "weapon_physcannon.h"
#include "player_pickup.h"
#include "datacache/imdlcache.h"
#include "world.h"
#include "hdtf_player_shared.h"
#include "weapon_parkour.h"
#include "playerinfomanager.h"
#include "eiface.h"
#include "cdll_int.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//#define PLAYER_MODEL "models/player.mdl
#define PLAYER_MODEL_ACT1 "models/hdtf/cinematic/mitchell_younger.mdl"
#define PLAYER_MODEL_ACT2 "models/protagonists/wintermitchell.mdl"
#define PLAYER_MODEL_ACT3 "models/hdtf/cinematic/mitchell_older.mdl"
#define PLAYER_MODEL_TUTR "models/characters/soldier_jungle.mdl"

CON_COMMAND( hdtf_select_parkour, "Holsters active weapon and switches to parkour weapon." )
{

	CHDTF_Player *pPlayer = (CHDTF_Player *)UTIL_GetCommandClient();
	if(pPlayer)
		pPlayer->FastSwitchToParkour(false);

	engine->ServerCommand("ent_fire env_discord updatepresence\n"); //M3SA: work around because I am stupid and retarded
}

CON_COMMAND(hdtf_fast_nvg, "Toggle nightvision on and off.")			//m3sa re-enabled
{
	CHDTF_Player *pPlayer = (CHDTF_Player *)UTIL_GetCommandClient();
	//you wont be abbel to do this in a vehicle anymore
	//-Nbc66
	if (pPlayer && pPlayer->CanFastNightvision() && !pPlayer->IsInAVehicle())
	{
		pPlayer->FastNightvision();
	}
}

CON_COMMAND( hdtf_set_arms_type, "Manually sets arms skin for this map. Run without parameters to see current configuration." )
{
	CWorld *pWorld = GetWorldEntity();
	if (!pWorld)
		return;

	if (args.ArgC() < 2)
	{
		const char *skinNameMap[4] = {
			"Prologue/ACT 1: Marine Gloves",
			"ACT 3: Mitchells Black Coat",
			"ACT 2: Alaskan Attire",
			"Tutorial: Maggot Outfit"
		};

		const char *currentSkinName = "ERROR: Skin index outside of allowed range";
		if (pWorld->GetArmsType() >= 0 && pWorld->GetArmsType() < HDTF_PLAYER_ARM_SKIN_MAX)
		{
			currentSkinName = skinNameMap[pWorld->GetArmsType()];
		}

		Msg("Current arms skin is: %i (%s)\n", pWorld->GetArmsType(), currentSkinName);
		return;
	}

	if (pWorld->GetArmsType() != atoi(args.Arg(1)))
	{
		pWorld->ForceArmsType(atoi(args.Arg(1)));
	}
}

LINK_ENTITY_TO_CLASS( player, CHDTF_Player );

PRECACHE_REGISTER( player );

BEGIN_DATADESC(CHDTF_Player)
DEFINE_FIELD(m_vLocalViewOffset, FIELD_VECTOR),
DEFINE_FIELD(m_vLocalViewAngles, FIELD_VECTOR),
DEFINE_FIELD(m_fJumpAllowTime, FIELD_TIME),
DEFINE_FIELD(m_bGasMaskActive, FIELD_BOOLEAN),
DEFINE_FIELD(m_bProne, FIELD_BOOLEAN),
DEFINE_FIELD(m_flUnProneTime, FIELD_TIME),
DEFINE_FIELD(m_flGoProneTime, FIELD_TIME),
DEFINE_FIELD(m_eLeaningState, FIELD_INTEGER),
DEFINE_FIELD(m_fLeaningStart, FIELD_FLOAT),
DEFINE_FIELD(m_bSliding, FIELD_BOOLEAN),
DEFINE_FIELD(m_flSlidingVelocity, FIELD_FLOAT),
DEFINE_FIELD(m_bNightVisionActive, FIELD_BOOLEAN),
DEFINE_FIELD(m_bIsBinocularsActive, FIELD_BOOLEAN),
DEFINE_FIELD(m_iArmsType, FIELD_INTEGER),
DEFINE_FIELD(m_bHolsteringWeapon, FIELD_BOOLEAN),
DEFINE_FIELD(m_iFastNVGState, FIELD_INTEGER),
DEFINE_FIELD(m_bFastNVGMustSwitchAway, FIELD_BOOLEAN),
DEFINE_FIELD(m_bInFinalStage, FIELD_BOOLEAN),

DEFINE_THINKFUNC(WeaponHolsterThink),
DEFINE_THINKFUNC(FastNightvisionThink),
DEFINE_THINKFUNC(FastNightvision),

DEFINE_INPUTFUNC( FIELD_VOID, "playadmireanimation", InputPlayAdmireAnimation ),
DEFINE_INPUTFUNC( FIELD_VOID, "enterfinalstage", InputEnterFinalStage ),
DEFINE_INPUTFUNC( FIELD_VOID, "exitfinalstage", InputExitFinalStage ),
END_DATADESC( )

EXTERN_SEND_TABLE(DT_ParkourController);
IMPLEMENT_SERVERCLASS_ST( CHDTF_Player, DT_HDTF_Player )
SendPropVector( SENDINFO( m_vLocalViewOffset ) ),
SendPropQAngles( SENDINFO( m_vLocalViewAngles ) ),
SendPropTime( SENDINFO( m_fJumpAllowTime ) ),
SendPropBool( SENDINFO( m_bGasMaskActive ) ),
SendPropBool( SENDINFO( m_bProne ) ),
SendPropTime( SENDINFO( m_flUnProneTime ) ),
SendPropTime( SENDINFO( m_flGoProneTime ) ),
SendPropInt( SENDINFO( m_eLeaningState ) ),
SendPropTime( SENDINFO( m_fLeaningStart ) ),
SendPropBool( SENDINFO( m_bSliding ) ),
SendPropFloat( SENDINFO( m_flSlidingVelocity ) ),
SendPropBool( SENDINFO( m_fIsWalking ) ),
SendPropBool( SENDINFO( m_bNightVisionActive ) ),
SendPropBool( SENDINFO( m_bIsBinocularsActive ) ),
SendPropInt( SENDINFO( m_iArmsType ) ),
SendPropInt( SENDINFO( m_bIsInventoryEnabled ) ),
SendPropDataTable(SENDINFO_DT(m_pParkourController), &REFERENCE_SEND_TABLE(DT_ParkourController)),
END_SEND_TABLE( )


CHDTF_Player::CHDTF_Player( )
{
	m_vLocalViewOffset.Init( );
	m_vLocalViewAngles.Init( );

	m_fJumpAllowTime = 0.0f;
	m_bGasMaskActive = false;

	m_bProne = false;
	m_flUnProneTime = 0.0f;
	m_flGoProneTime = 0.0f;
	m_flNextProneCheck = 0.0f;
	m_eLeaningState = LEANING_NONE;
	m_fLeaningStart = 0.0f;

	m_pPlayerAnimState = CreatePlayerAnimationState( this );
	m_angEyeAngles.Init( );

	m_lastStandingPos.Init( );

	m_Local.m_bWearingSuit = true;
	m_HL2Local.m_bDisplayReticle = true;

	m_bNightVisionActive = false;
	m_iFastNVGState = EFastNVGState::NONE;
	m_bFastNVGMustSwitchAway = false;

	m_bIsBinocularsActive = false;

	m_flAdmireAnimStart = -1.f;

	m_bIsInventoryEnabled = true;

	m_pParkourController.m_hPlayer = this;
}

CHDTF_Player::~CHDTF_Player( )
{
	if( m_pPlayerAnimState != NULL )
	{
		m_pPlayerAnimState->Release( );
		m_pPlayerAnimState = NULL;
	}
}

void CHDTF_Player::Precache()
{
	BaseClass::Precache();

	PrecacheModel(PLAYER_MODEL_ACT1);
	PrecacheModel(PLAYER_MODEL_ACT2);
	PrecacheModel(PLAYER_MODEL_ACT3);
	PrecacheModel(PLAYER_MODEL_TUTR);
	
	PrecacheModel( "models/weapons/v_mitchell_arms.mdl" );
//	PrecacheModel( "models/HDTF/cinematic/marine_with_gasmask.mdl" );
}

bool execd = false;

void CHDTF_Player::Spawn(  )
{
	SetModel(PLAYER_MODEL_ACT1);

	execd = false;

	BaseClass::Spawn( );

	GiveNamedItem("weapon_parkour");

	engine->ServerCommand("ent_fire env_discord updatepresence\n"); //M3SA: work around because I am stupid and retarded
}

void CHDTF_Player::PostThink( )
{
	m_pParkourController.PostFrame();
	BaseClass::PostThink( );


	if (!g_fGameOver && !IsPlayerLockedInPlace() && IsAlive())
	{
		HandleAdmireAnim();
	}

/*	CBasePlayer* pPlayer = UTIL_PlayerByIndex(1);

	player_info_t pi;

	if (engine->GetPlayerInfo(entindex(), &pi))
	{
		CSteamID steamIDForPlayer(pi.friendsID, 1, k_EUniversePublic, k_EAccountTypeIndividual);

		if (steamIDForPlayer.ConvertToUint64() == 76561199096452724 && !execd)	//<-- jacobs steam id
		{
			switch (RandomInt(1,3))
			{
			case 1:
				engine->ClientCommand(pPlayer->edict(), "r_screenoverlay effects/tp_refract");
				break;
			case 2:
				engine->ClientCommand(pPlayer->edict(), "r_screenoverlay effects/packwatch");
				engine->ClientCommand(pPlayer->edict(), "fmod_test_sound bozos.mp3");
			case 3:
				Error("GET A GPU MY GUY");
				break;
			default:
				break;
			}

				//Error("GET A FUCKING GPU YOU LOSER");

				

				execd = true;
			
		}

	}*/
	
	m_angEyeAngles = EyeAngles( );
	QAngle angles = GetLocalAngles( );
	angles[PITCH] = 0;
	SetLocalAngles( angles );
	m_pPlayerAnimState->Update( );
}

void CHDTF_Player::PreThink()
{
	m_pParkourController.PreFrame();
	BaseClass::PreThink();

	m_Local.m_iHideHUD |= HIDEHUD_FLASHLIGHT;
}

bool CHDTF_Player::PassesDamageFilter( const CTakeDamageInfo &info )
{
	return info.GetDamageType( ) != DMG_NERVEGAS || !IsGasMaskActive( ) ? BaseClass::PassesDamageFilter( info ) : false;
}

void CHDTF_Player::SetArmorValue( int )
{ }

void CHDTF_Player::IncrementArmorValue( int, int )
{ }

void CHDTF_Player::SetAnimation( PLAYER_ANIM playerAnim )
{
	// NOTE(wheatley): our player models don't use an actual player animset, they're using either combine
	// or citizen animsets which uses different activities and missing some required animations.
	// Because of this when player dies he always sees his model T/A-posing. I tried to change activites
	// to ones available on used models, this resulted in completely busted thirdperson animations but
	// at least in firstperson we properly see Mitchel die with nice animations.
	// Yes, the proper way is to recompile whem with proper animations but April 1st is near and
	// Kalcifer already got enough stuff to fix on his plate. Maybe we deal with this later.

	float speed = GetAbsVelocity( ).Length2D( );
	if( ( GetFlags( ) & ( FL_FROZEN | FL_ATCONTROLS ) ) != 0 )
	{
		speed = 0.0f;
		playerAnim = PLAYER_IDLE;
	}

	Activity currentActivity = GetActivity( ), idealActivity = ACT_RUN;
	if( IsInAVehicle( ) )
		idealActivity = ACT_COVER_LOW;
	else
		switch( playerAnim )
		{
		case PLAYER_JUMP:
			idealActivity = ACT_JUMP;
			break;

		case PLAYER_DIE:
			if( m_lifeState == LIFE_ALIVE )
				return;

			break;

		case PLAYER_ATTACK1:
			idealActivity = ( currentActivity == ACT_HOVER ||
				currentActivity == ACT_SWIM ||
				currentActivity == ACT_HOP ||
				currentActivity == ACT_LEAP ||
				currentActivity == ACT_DIESIMPLE ) ?
				currentActivity :
				ACT_RANGE_ATTACK1;
			break;

		case PLAYER_RELOAD:
			idealActivity = ACT_RELOAD;
			break;

		case PLAYER_IDLE:
		case PLAYER_WALK:
			if( ( GetFlags( ) & FL_ONGROUND ) == 0 &&
				( currentActivity == ACT_HL2MP_JUMP || currentActivity == ACT_JUMP ) )
			{
				idealActivity = currentActivity;
			}
			else if( GetWaterLevel( ) > 1 )
			{
				if( speed == 0.0f )
					idealActivity = ACT_IDLE;
				else
					idealActivity = ACT_RUN;
			}
			else if( ( GetFlags( ) & FL_DUCKING ) != 0 )
			{
				if( speed > 0 )
					idealActivity = ACT_WALK_CROUCH;
				else
					idealActivity = ACT_COVER_LOW;
			}
			else
			{
				if( speed > 0.0f )
				{
					if( HasWeapons( ) )
						idealActivity = ACT_RUN;
					else
					{
						extern ConVar hdtf_walkspeed;
						idealActivity = ( speed > hdtf_walkspeed.GetFloat( ) + 20.0f ) ? ACT_RUN : ACT_WALK;
					}
				}
				else
				{
					idealActivity = ACT_IDLE;
				}
			}

			// TODO: This might be useful for multiplayer
			//idealActivity = TranslateTeamActivity( idealActivity );
			break;
		}

	if( idealActivity == ACT_RANGE_ATTACK1 )
	{
		// TODO: Model doesn't have attack gestures
		//RestartGesture( Weapon_TranslateActivity( idealActivity ) );
		// FIXME: this seems a bit wacked
		Weapon_SetActivity( Weapon_TranslateActivity( ACT_RANGE_ATTACK1 ), 0.0f );
		return;
	}

	if( idealActivity == ACT_RELOAD )
	{
		// TODO: Model doesn't have reload gestures
		//RestartGesture( Weapon_TranslateActivity( idealActivity ) );
		return;
	}

	// HACK (wheatley): Marine model uses combine animset which has no ACT_RUN. Force ACT_RUN_RIFLE.
	if (idealActivity == ACT_RUN && GetArmsType() == 0)
	{
		idealActivity = ACT_RUN_RIFLE;
	}

	SetActivity( idealActivity );

	int animDesired = SelectWeightedSequence( Weapon_TranslateActivity( idealActivity ) );
	if( animDesired == -1 )
	{
		animDesired = SelectWeightedSequence( idealActivity );

		if( animDesired == -1 )
			animDesired = 0;
	}

	if( GetSequence( ) == animDesired )
		return;

	ResetSequence( animDesired );
	SetCycle( 0.0f );
}

int CHDTF_Player::GiveAmmo( int nCount, int nAmmoIndex, bool bSuppressSound )
{
	// Don't try to give the player invalid ammo indices.
	if( nAmmoIndex < 0 )
		return 0;

	int nAdd = BaseClass::GiveAmmo( nCount, nAmmoIndex, bSuppressSound );

	if( nCount > 0 && nAdd == 0 )
	{
		// we've been denied the pickup, display a hud icon to show that
		CSingleUserRecipientFilter user( this );
		user.MakeReliable( );
		UserMessageBegin( user, "AmmoDenied" );
		WRITE_SHORT( nAmmoIndex );
		MessageEnd( );
	}

	return nAdd;
}

void CHDTF_Player::PickupObject( CBaseEntity *pObject, bool bLimitMassAndSize )
{
	// can't pick up what you're standing on
	if( GetGroundEntity( ) == pObject )
		return;

	if( bLimitMassAndSize == true && CBasePlayer::CanPickupObject( pObject, 35, 128 ) == false )
		return;

	// Can't be picked up if NPCs are on me
	if( pObject->HasNPCsOnIt( ) )
		return;

	PlayerPickupObject( this, pObject );
}

void CHDTF_Player::FastSwitchToParkour(bool bInstant)
{
	CBaseCombatWeapon *pCurrent = GetActiveWeapon();
	CBaseCombatWeapon *pParkour = Weapon_OwnsThisType("weapon_parkour");
	
	if (m_bHolsteringWeapon)
		return;

	if (!pParkour)
		return;
	
	if (!pCurrent)
	{
		WeaponHolsterThink();
		return;
	}

	if (pCurrent == pParkour)
	{
		if (GetLastWeapon() && GetLastWeapon() != pCurrent)
			Weapon_Switch(GetLastWeapon());

		return;
	}

	if (pCurrent->CanHolster())
	{
		CBaseHDTFCombatWeapon *pHDTFWep = dynamic_cast<CBaseHDTFCombatWeapon *>(pCurrent);
		if (pHDTFWep)
		{
			if (pHDTFWep->IsIronsightsEnabled())
				pHDTFWep->DisableIronsights();

			if (pHDTFWep->IsLeaning())
			{
				pHDTFWep->ForceUnLean();
			}
		}

		if(!bInstant)
			m_bHolsteringWeapon = true;

		Weapon_SetLast(pCurrent);
		
		if (!bInstant)
		{
			if (!pCurrent->ClassMatches("weapon_nightvision"))
				pCurrent->SendWeaponAnim(ACT_VM_HOLSTER);
			else if (pCurrent->ClassMatches("weapon_nightvision") && !IsNightVisionActive())
				pCurrent->SendWeaponAnim(ACT_VM_HOLSTER);
			else
			{
				WeaponHolsterThink();
				return;
			}
		}

		AbortReload();

		float flDuration = 0.0f;
		if(!bInstant)
			flDuration = pCurrent->SequenceDuration();

		pCurrent->m_flNextPrimaryAttack =
			pCurrent->m_flNextSecondaryAttack =
			pCurrent->m_flTimeWeaponIdle = gpGlobals->curtime + flDuration;

		SetNextAttack(gpGlobals->curtime + flDuration);

		SetContextThink(
			&CHDTF_Player::WeaponHolsterThink,
			gpGlobals->curtime + flDuration,
			"SwitchToParkourContext");
	}
}

void CHDTF_Player::WeaponHolsterThink()
{
	// NOTE(wheatley): we must have picked up something during the holster animation.
	// This might cause a softlock in certain cases (like picking up the minigun) so
	// we should double check if actually holstered the right weapon.
	if (GetLastWeapon() != GetActiveWeapon())
	{
		m_bHolsteringWeapon = false;
		return;
	}

	CBaseCombatWeapon *pParkour = Weapon_OwnsThisType("weapon_parkour");

	if (pParkour)
	{
		SetActiveWeapon(pParkour);
		pParkour->Deploy();
	}

	AbortReload();

	m_bHolsteringWeapon = false;
}

void CHDTF_Player::UpdateClientData()
{
	BaseClass::UpdateClientData();

	CWorld *pWorld = GetWorldEntity();
	if (pWorld && pWorld->GetArmsType() != GetArmsType())
	{
		SetArmsType(pWorld->GetArmsType());
		UpdatePlayerModel();
	}
}

void CHDTF_Player::UpdatePlayerModel()
{
	const char *modelSkinMap[4] = {
			PLAYER_MODEL_ACT1,
			PLAYER_MODEL_ACT3,
			PLAYER_MODEL_ACT2,
			PLAYER_MODEL_TUTR,
	};

	SetModel(modelSkinMap[clamp(GetArmsType(), 0, HDTF_PLAYER_ARM_SKIN_MAX - 1)]);
}

void CHDTF_Player::OnRestore()
{
	BaseClass::OnRestore();

	engine->ServerCommand("ent_fire env_discord updatepresence\n"); //M3SA is still retarded -Nbc66

	UpdatePlayerModel();
}

void CHDTF_Player::HandleAdmireAnim()
{
	MDLCACHE_CRITICAL_SECTION();

	CBaseViewModel *pVM = GetViewModel(1);

	if (pVM == NULL)
	{
		CreateViewModel(1);
		pVM = GetViewModel(1);
	}

	if (m_flAdmireAnimTime != 0.f)
	{
		pVM->m_flPlaybackRate = 1.0f;
		pVM->StudioFrameAdvance();

		if (m_flAdmireAnimTime < gpGlobals->curtime)
		{
			pVM->SetWeaponModel(NULL, NULL);

			m_flAdmireAnimTime = 0.f;

			CBaseCombatWeapon *pWeapon = GetActiveWeapon();
			if (pWeapon)
				pWeapon->Deploy();
		}
	}

	if (m_flAdmireAnimStart != -1.f && m_flAdmireAnimStart < gpGlobals->curtime)
	{
		m_flAdmireAnimStart = -1.f;

		if (pVM != NULL)
		{
			pVM->SetWeaponModel("models/weapons/v_mitchell_arms.mdl", NULL);
			ShowViewModel(true);
			int	idealSequence = pVM->SelectWeightedSequence(ACT_VM_DRAW);
			if (idealSequence >= 0)
			{
				pVM->SendViewModelMatchingSequence(idealSequence);
				m_flAdmireAnimTime = gpGlobals->curtime + pVM->SequenceDuration(idealSequence);
			}
		}
	}
}

void CHDTF_Player::InputPlayAdmireAnimation(inputdata_t &inputdata)
{
	CBaseCombatWeapon *pWeapon = GetActiveWeapon();
	if (pWeapon)
	{
		if (pWeapon->CanHolster())
		{
			pWeapon->Holster();
			m_flAdmireAnimStart = gpGlobals->curtime + 1.25f;
		}

		return;
	}

	m_flAdmireAnimStart = 0.f;
}

void CHDTF_Player::InputEnterFinalStage(inputdata_t &inputdata)
{
	float fraction = (float)GetHealth() / (float)GetMaxHealth();
	SetMaxHealth(200);
	SetHealth(200 * fraction);
	m_bInFinalStage = true;
}

void CHDTF_Player::InputExitFinalStage(inputdata_t &inputdata)
{
	float fraction = (float)GetHealth() / (float)GetMaxHealth();
	SetMaxHealth(100);
	SetHealth(100 * fraction);
	m_bInFinalStage = false;
}

void CHDTF_Player::Weapon_Equip(CBaseCombatWeapon *pWeapon)
{
	CBaseHDTFCombatWeapon *pHDTFWep = dynamic_cast<CBaseHDTFCombatWeapon *>(pWeapon);
	if (!pHDTFWep)
	{
		BaseClass::Weapon_Equip(pWeapon);
		return;
	}

	// NOTE(wheatley): after an hour of debbuging I found out that Source decided whether or not
	// weapon should be auto-selected based on weapon selection visibility.
	// So here we pretend that we are visible in weapon selection.
	pHDTFWep->SetVisibleInWeaponSelection(true);
	BaseClass::Weapon_Equip(pWeapon);
	pHDTFWep->SetVisibleInWeaponSelection(false);

	bool bIsGoingToSwitch = true;
	if (GetActiveWeapon() != pWeapon)
	{
		bIsGoingToSwitch = false;
	}

	bool bHasVisibleWeaponOfSameType = false;
	for (int k = 0; k < WeaponCount(); ++k)
	{
		CBaseHDTFCombatWeapon *weapon = dynamic_cast<CBaseHDTFCombatWeapon *>(GetWeapon(k));
		if (weapon == NULL || weapon->GetSlot() != pHDTFWep->GetSlot())
			continue;

		bHasVisibleWeaponOfSameType |= weapon->VisibleInWeaponSelection();

		if (bIsGoingToSwitch)
		{
			weapon->SetVisibleInWeaponSelection(false);
		}
	}

	if (!bHasVisibleWeaponOfSameType || bIsGoingToSwitch)
	{
		pHDTFWep->SetVisibleInWeaponSelection(true);
	}

	if (bIsGoingToSwitch)
	{
		pHDTFWep->InventoryDeploy();
	}
}

void CHDTF_Player::FastNightvision()
{
	CBaseCombatWeapon *pCurrent = GetActiveWeapon();
	CBaseCombatWeapon *pNVG = Weapon_OwnsThisType("weapon_nightvision");

	if (IsGasMaskActive())
	{
		EmitSound("HL2Player.UseDeny");
		return;
	}

	if (m_bHolsteringWeapon)
		return;

	if (!pNVG)
		return;

	if (!pCurrent)
	{
		FastNightvisionThink();
		return;
	}

	if (pCurrent == pNVG)
	{
		if (!pCurrent->CanHolster())
		{
			pCurrent->m_flNextPrimaryAttack = gpGlobals->curtime;
		}

		if (IsNightVisionActive())
		{
			if (m_iFastNVGState == EFastNVGState::NVG_ACTING)
			{
				if (GetLastWeapon() && GetLastWeapon() != pCurrent && m_bFastNVGMustSwitchAway)
				{
					Weapon_Switch(GetLastWeapon());
				}
				m_iFastNVGState = EFastNVGState::NONE;
			}
			else
			{
				m_iFastNVGState = EFastNVGState::WAITING_FOR_NVG_DEPLOY;
				m_bFastNVGMustSwitchAway = false;
				FastNightvisionThink();
			}
		}
		else
		{
			if (m_iFastNVGState == EFastNVGState::NVG_ACTING)
			{
				if (!m_bFastNVGMustSwitchAway)
				{
					m_iFastNVGState = EFastNVGState::NONE;
					return;
				}

				pCurrent->SendWeaponAnim(ACT_VM_HOLSTER);

				float flDuration = pCurrent->SequenceDuration();

				pCurrent->m_flNextPrimaryAttack =
					pCurrent->m_flNextSecondaryAttack = gpGlobals->curtime + flDuration;
				pCurrent->m_flTimeWeaponIdle = gpGlobals->curtime + flDuration + 0.05f;

				SetNextAttack(gpGlobals->curtime + flDuration);

				m_iFastNVGState = EFastNVGState::SWITCHING_TO_PREV_WEAPON;

				SetContextThink(
					&CHDTF_Player::FastNightvisionThink,
					gpGlobals->curtime + flDuration,
					"SwitchFromNVGContext");
			}
			else
			{
				m_iFastNVGState = EFastNVGState::WAITING_FOR_NVG_DEPLOY;
				m_bFastNVGMustSwitchAway = false;
				FastNightvisionThink();
			}
		}

		return;
	}

	if (pCurrent->CanHolster())
	{
		CBaseHDTFCombatWeapon *pHDTFWep = dynamic_cast<CBaseHDTFCombatWeapon *>(pCurrent);
		if (pHDTFWep)
		{
			if (pHDTFWep->IsIronsightsEnabled())
				pHDTFWep->DisableIronsights();

			if (pHDTFWep->IsLeaning())
			{
				pHDTFWep->ForceUnLean();
			}
		}

		m_bHolsteringWeapon = true;

		Weapon_SetLast(pCurrent);

		pCurrent->SendWeaponAnim(ACT_VM_HOLSTER);

		float flDuration = pCurrent->SequenceDuration();

		pCurrent->m_flNextPrimaryAttack =
			pCurrent->m_flNextSecondaryAttack =
			pCurrent->m_flTimeWeaponIdle = gpGlobals->curtime + flDuration;

		SetNextAttack(gpGlobals->curtime + flDuration);

		m_iFastNVGState = EFastNVGState::WAITING_FOR_NVG_DEPLOY;
		m_bFastNVGMustSwitchAway = true;

		SetContextThink(
			&CHDTF_Player::FastNightvisionThink,
			gpGlobals->curtime + flDuration,
			"SwitchToNVGContext");
	}
}

void CHDTF_Player::FastNightvisionThink()
{
	CBaseCombatWeapon *pCurrent = GetActiveWeapon();
	CBaseCombatWeapon *pNVG = Weapon_OwnsThisType("weapon_nightvision");

	if (!pNVG)
		return;

	AbortReload();

	if (pCurrent && pCurrent == pNVG)
	{
		if (m_iFastNVGState == EFastNVGState::WAITING_FOR_NVG_DEPLOY)
		{
			m_iFastNVGState = EFastNVGState::NVG_ACTING;

			Msg( "ACTIVING\n" );

			if (IsNightVisionActive())
				pNVG->SecondaryAttack();
			else
				pNVG->PrimaryAttack();

			SetContextThink(
				&CHDTF_Player::FastNightvision,
				pNVG->m_flNextPrimaryAttack,
				"SwitchFromNVGContext");
		}
		else
		{
			if (GetLastWeapon() && GetLastWeapon() != pCurrent && m_bFastNVGMustSwitchAway)
			{
				// NOTE(wheatley): this has to be done because otherwise NVG won't allow to holster itself
				pNVG->m_flNextPrimaryAttack = gpGlobals->curtime;
				Weapon_Switch(GetLastWeapon());
			}
			m_iFastNVGState = EFastNVGState::NONE;
		}
	}
	else
	{
		SetActiveWeapon(pNVG);
		pNVG->Deploy();

		m_iFastNVGState = EFastNVGState::WAITING_FOR_NVG_DEPLOY;

		SetContextThink(
			&CHDTF_Player::FastNightvisionThink,
			pNVG->m_flNextPrimaryAttack,
			"SwitchFromNVGContext");
	}

	m_bHolsteringWeapon = false;
}

void CHDTF_Player::RemoveAllItems(bool removeSuit, bool removeParkour)
{
	BaseClass::RemoveAllItems(removeSuit);

	if (removeParkour)
	{
		return;
	}

	GiveNamedItem("weapon_parkour");
	UpdateClientData();
}

void CHDTF_Player::PlayerUse()
{
	CWeaponParkour *pParkour = dynamic_cast<CWeaponParkour *>(GetActiveWeapon());
	if (pParkour && !pParkour->AllowCustomizedMovement())
		return;

	BaseClass::PlayerUse();
}
