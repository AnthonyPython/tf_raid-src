#include "cbase.h"
#include "c_hdtf_player.h"
#include "iviewrender.h"
#include "hdtf_gamerules.h"
#include "view.h"
#include "util.h"
#include "weapon_parkour.h"
#include "hud_inventory.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static ConVar cl_legs_origin_shift( "cl_legs_origin_shift", "-17", FCVAR_ARCHIVE, "Amount in game units to shift the player model relative to the direction the player is facing" );
static ConVar cl_legs_clip_height( "cl_legs_clip_height", "0", FCVAR_ARCHIVE, "Amount in game units of the player model to render up to [0 = disable]", true, 0, false, 0 );

LINK_ENTITY_TO_CLASS( player, C_HDTF_Player );

EXTERN_RECV_TABLE(DT_ParkourController);

IMPLEMENT_CLIENTCLASS_DT( C_HDTF_Player, DT_HDTF_Player, CHDTF_Player )
RecvPropVector( RECVINFO( m_vLocalViewOffset ) ),
RecvPropQAngles( RECVINFO( m_vLocalViewAngles ) ),
RecvPropTime( RECVINFO( m_fJumpAllowTime ) ),
RecvPropBool( RECVINFO( m_bGasMaskActive ) ),
RecvPropBool( RECVINFO( m_bProne ) ),
RecvPropTime( RECVINFO( m_flUnProneTime ) ),
RecvPropTime( RECVINFO( m_flGoProneTime ) ),
RecvPropInt( RECVINFO( m_eLeaningState ) ),
RecvPropTime( RECVINFO( m_fLeaningStart ) ),
RecvPropBool( RECVINFO( m_bSliding ) ),
RecvPropFloat( RECVINFO( m_flSlidingVelocity ) ),
RecvPropBool( RECVINFO( m_fIsWalking ) ),
RecvPropInt( RECVINFO( m_eGrabbyHandsState ) ),
RecvPropBool( RECVINFO( m_bNightVisionActive ) ),
RecvPropBool( RECVINFO( m_bIsBinocularsActive ) ),
RecvPropInt( RECVINFO( m_iArmsType ) ),
RecvPropInt( RECVINFO( m_bIsInventoryEnabled ) ),
RecvPropDataTable(RECVINFO_DT(m_pParkourController), 0, &REFERENCE_RECV_TABLE(DT_ParkourController)),
END_RECV_TABLE( )

BEGIN_PREDICTION_DATA( C_HDTF_Player )
DEFINE_PRED_FIELD( m_vLocalViewOffset, FIELD_VECTOR, FTYPEDESC_INSENDTABLE ),
DEFINE_PRED_FIELD( m_vLocalViewAngles, FIELD_VECTOR, FTYPEDESC_INSENDTABLE ),
DEFINE_PRED_FIELD( m_fJumpAllowTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
DEFINE_PRED_FIELD( m_bGasMaskActive, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
DEFINE_PRED_FIELD( m_bProne, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
DEFINE_PRED_FIELD( m_flUnProneTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
DEFINE_PRED_FIELD( m_flGoProneTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
DEFINE_PRED_FIELD( m_eLeaningState, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
DEFINE_PRED_FIELD( m_fLeaningStart, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
DEFINE_PRED_FIELD( m_bSliding, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
DEFINE_PRED_FIELD( m_flSlidingVelocity, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
DEFINE_PRED_FIELD( m_fIsWalking, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
DEFINE_PRED_FIELD( m_eGrabbyHandsState, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
DEFINE_PRED_FIELD( m_bNightVisionActive, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
DEFINE_PRED_FIELD( m_bIsBinocularsActive, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
DEFINE_PRED_FIELD( m_iArmsType, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
DEFINE_PRED_FIELD( m_bIsInventoryEnabled, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
END_PREDICTION_DATA( )

bool IsInEye( )
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer( );
	return pPlayer == NULL || pPlayer->GetObserverMode( ) != OBS_MODE_IN_EYE;
}

C_HDTF_Player::C_HDTF_Player( )
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

	m_eGrabbyHandsState = GRABSTATE_NONE;

	m_Local.m_bWearingSuit = true;
	m_HL2Local.m_bDisplayReticle = true;

	m_bNightVisionActive = false;
	m_hNightVisionLight = CLIENTSHADOW_INVALID_HANDLE;
	m_hWeaponLight = CLIENTSHADOW_INVALID_HANDLE;

	m_hNightVisionCC = INVALID_CLIENT_CCHANDLE;

	m_iIDEntIndex = 0;

	m_bIsInventoryEnabled = true;
	m_bDesiredInventoryState = false;
	m_flIronsightDistanceFraction = 0.0f;

	m_iOldArmsType = 0;
	m_pParkourController.m_hPlayer = this;
}

C_HDTF_Player::~C_HDTF_Player()
{
	g_pColorCorrectionMgr->RemoveColorCorrection(m_hNightVisionCC);

	RemoveNightVisionLight();
	DestroyWeaponLight();
}

const Vector &C_HDTF_Player::GetRenderOrigin( )
{
	// If we're not observing this player, or if we're not drawing it at the
	// moment then use the normal absolute origin.
	// NOTE: the GetCurrentlyDrawingEntity check is here to make sure the
	// shadow is rendered from the correct origin
	if ( !IsInEye( ) || view->GetCurrentlyDrawingEntity( ) != this )
		return BaseClass::GetRenderOrigin( );

	// Get the forward vector
	static Vector forward; // static because this method returns a reference
	AngleVectors( GetRenderAngles( ), &forward );

	// Shift the render origin by a fixed amount
	forward *= cl_legs_origin_shift.GetFloat( );
	forward += GetAbsOrigin( );

	return forward;
}

int C_HDTF_Player::DrawModel( int flags )
{
	CMatRenderContextPtr context( materials );
	if( cl_legs_clip_height.GetInt( ) > 0 )
	{
		context->SetHeightClipMode( MATERIAL_HEIGHTCLIPMODE_RENDER_BELOW_HEIGHT );
		context->SetHeightClipZ( GetAbsOrigin( ).z + cl_legs_clip_height.GetFloat( ) );
	}

	return BaseClass::DrawModel( flags );
}

void C_HDTF_Player::AddEntity( )
{
	BaseClass::AddEntity( );

	SetLocalAnglesDim( X_INDEX, 0 );
}

bool C_HDTF_Player::IsWalking( ) const
{
	return m_fIsWalking;
}

void C_HDTF_Player::AdjustMouseSensitivity( float &sensitivity )
{
	// adjust sensitivity for weapons with scopes like remigton 700
	C_BaseHDTFCombatWeapon *pWeapon = dynamic_cast<C_BaseHDTFCombatWeapon *>(GetActiveWeapon());
	if (pWeapon && pWeapon->IsIronsightsEnabled() && pWeapon->GetIronSensitivity() != 0.f)
		sensitivity = pWeapon->GetIronSensitivity();

	if( IsSliding( ) )
		sensitivity = 0.01f;
}

// -----------------------------------------------------------------
// Purpose: Creates and updates new projected texture handle for
// use with night vision weapon
// -----------------------------------------------------------------
void C_HDTF_Player::CreateNightVisionLight(const Vector *overridePos, const Quaternion *overrideQuat)
{
	// setup projected texture
	FlashlightState_t state;

	// having values under 100 seems to cause
	// graphics issues at certain angles
	state.m_fHorizontalFOVDegrees = 110.f;
	state.m_fVerticalFOVDegrees = 110.f;

	if (overridePos != NULL)
		state.m_vecLightOrigin = *overridePos;
	else
		state.m_vecLightOrigin = EyePosition();

	if (overrideQuat != NULL)
	{
		state.m_quatOrientation = *overrideQuat;
	}
	else
	{
		Vector vForward, vRight, vUp;
		EyeVectors(&vForward, &vRight, &vUp);
		BasisToQuaternion(vForward, vRight, vUp, state.m_quatOrientation);
	}

	state.m_fQuadraticAtten = 0.0;
	state.m_fLinearAtten = 100;
	state.m_fConstantAtten = 0.0f;
	state.m_Color[0] = 0.f;
	state.m_Color[1] = 0.74f;
	state.m_Color[2] = 0.f;
	state.m_Color[3] = 0.0f;
	state.m_NearZ = 4.f;
	state.m_FarZ = 460.f;
	state.m_bEnableShadows = false;
	state.m_pSpotlightTexture = materials->FindTexture("effects/nightvision", TEXTURE_GROUP_OTHER, false);
	state.m_nSpotlightTextureFrame = 0;

	if (m_hNightVisionLight == CLIENTSHADOW_INVALID_HANDLE)
	{
		m_hNightVisionLight = g_pClientShadowMgr->CreateFlashlight(state);
	}
	else
	{
		if (m_hNightVisionLight != NULL)
		{
			g_pClientShadowMgr->UpdateFlashlightState(m_hNightVisionLight, state);
			g_pClientShadowMgr->UpdateProjectedTexture(m_hNightVisionLight, true);
		}
	}
}

// -----------------------------------------------------------------
// Purpose: Removes projected texture of the night vision if any
// -----------------------------------------------------------------
void C_HDTF_Player::RemoveNightVisionLight()
{
	if (m_hNightVisionLight != CLIENTSHADOW_INVALID_HANDLE)
	{
		g_pClientShadowMgr->DestroyFlashlight(m_hNightVisionLight);
		m_hNightVisionLight = CLIENTSHADOW_INVALID_HANDLE;
	}
}

// -----------------------------------------------------------------
// Purpose: Creates and updates new projected texture handle for
// use with weapon-attached flashlight
// -----------------------------------------------------------------
void C_HDTF_Player::UpdateWeaponLight(C_BaseHDTFCombatWeapon *pWeapon)
{
	Vector vForward, vRight, vUp, vPos;
	QAngle aDir;

	UTIL_GetWeaponAttachment(pWeapon, pWeapon->LookupAttachment("flashlight"), vPos, aDir);

	AngleVectors(aDir, &vForward, &vRight, &vUp);

	VectorNormalize(vForward);
	VectorNormalize(vRight);
	VectorNormalize(vUp);

	vUp -= DotProduct(vForward, vUp) * vForward;
	VectorNormalize(vUp);
	vRight -= DotProduct(vForward, vRight) * vForward;
	VectorNormalize(vRight);
	vRight -= DotProduct(vUp, vRight) * vUp;
	VectorNormalize(vRight);

	AssertFloatEquals(DotProduct(vForward, vRight), 0.0f, 1e-3);
	AssertFloatEquals(DotProduct(vForward, vUp), 0.0f, 1e-3);
	AssertFloatEquals(DotProduct(vRight, vUp), 0.0f, 1e-3);

	FlashlightState_t state;

	state.m_fHorizontalFOVDegrees = pWeapon->FlashlightFov();
	state.m_fVerticalFOVDegrees = pWeapon->FlashlightFov();

	state.m_vecLightOrigin = vPos;
	BasisToQuaternion(vForward, vRight, vUp, state.m_quatOrientation);

	state.m_fQuadraticAtten = 0.0;
	state.m_fLinearAtten = 100;
	state.m_fConstantAtten = 0.0f;
	state.m_Color[0] = 1.f;
	state.m_Color[1] = 1.f;
	state.m_Color[2] = 1.f;
	state.m_Color[3] = 0.0f;
	state.m_NearZ = pWeapon->FlashlightNearZ();
	state.m_FarZ = pWeapon->FlashlightFarZ();
	state.m_bEnableShadows = true;
	state.m_pSpotlightTexture = materials->FindTexture("effects/flashlight", TEXTURE_GROUP_OTHER, false);
	state.m_nSpotlightTextureFrame = 0;

	state.m_flShadowAtten = 0.35f;
	state.m_flShadowSlopeScaleDepthBias = 16.f;
	state.m_flShadowDepthBias = 0.00001f;

	if (m_hWeaponLight == CLIENTSHADOW_INVALID_HANDLE)
	{
		m_hWeaponLight = g_pClientShadowMgr->CreateFlashlight(state);
	}
	else
	{
		if (m_hWeaponLight != NULL)
		{
			g_pClientShadowMgr->UpdateFlashlightState(m_hWeaponLight, state);
			g_pClientShadowMgr->UpdateProjectedTexture(m_hWeaponLight, true);
		}
	}
}

// -----------------------------------------------------------------
// Purpose: Removes projected texture of the weapon light
// -----------------------------------------------------------------
void C_HDTF_Player::DestroyWeaponLight()
{
	if (m_hWeaponLight != CLIENTSHADOW_INVALID_HANDLE)
	{
		g_pClientShadowMgr->DestroyFlashlight(m_hWeaponLight);
		m_hWeaponLight = CLIENTSHADOW_INVALID_HANDLE;
	}
}

// -----------------------------------------------------------------
// Purpose: Updates night vision's color correction
// -----------------------------------------------------------------
void C_HDTF_Player::ClientThink()
{
	if (m_hNightVisionCC != INVALID_CLIENT_CCHANDLE)
	{
		g_pColorCorrectionMgr->SetColorCorrectionWeight(
			m_hNightVisionCC, 
			IsNightVisionActive() ? 1.f : 0.f);
	}

	UpdateIDTarget( );
	UpdateInventoryState();
	UpdateIronsightDistanceFraction();

	BaseClass::ClientThink();
}

void C_HDTF_Player::UpdateFlashlight()
{
	if (IsNightVisionActive())
	{
		if(!m_pParkourController.IsControllingView())
			CreateNightVisionLight();
	}
	else
		RemoveNightVisionLight();

	CBaseHDTFCombatWeapon *pWeapon = dynamic_cast<CBaseHDTFCombatWeapon *>(GetActiveWeapon());
	if (pWeapon && pWeapon->HasFlashlight() && IsAlive())
		UpdateWeaponLight(pWeapon);
	else
		DestroyWeaponLight();
}

void C_HDTF_Player::Flashlight()
{
}

void C_HDTF_Player::OnDataChanged(DataUpdateType_t updateType)
{
	BaseClass::OnDataChanged(updateType);

	if (updateType == DATA_UPDATE_CREATED)
	{
		if (m_hNightVisionCC == INVALID_CLIENT_CCHANDLE)
		{
			m_hNightVisionCC = g_pColorCorrectionMgr->AddColorCorrection("cc_nightvision.raw");
			SetNextClientThink(CLIENT_THINK_ALWAYS);
		}
	}
}

int C_HDTF_Player::GetIDTarget( ) const
{
	return m_iIDEntIndex;
}

//-----------------------------------------------------------------------------
// Purpose: Update this client's target entity
//-----------------------------------------------------------------------------
void C_HDTF_Player::UpdateIDTarget( )
{
	if( !IsLocalPlayer( ) )
		return;

	// Clear old target and find a new one
	m_iIDEntIndex = 0;

	// don't show IDs in chase spec mode
	if( GetObserverMode( ) == OBS_MODE_CHASE || GetObserverMode( ) == OBS_MODE_DEATHCAM )
		return;

	Vector vecStart, vecEnd;
	VectorMA( MainViewOrigin( ), 1500, MainViewForward( ), vecEnd );
	VectorMA( MainViewOrigin( ), 10,   MainViewForward( ), vecStart );

	trace_t tr;
	UTIL_TraceLine( vecStart, vecEnd, MASK_SOLID, this, COLLISION_GROUP_NONE, &tr );

	if( !tr.startsolid && tr.DidHitNonWorldEntity( ) )
	{
		C_BaseEntity *pEntity = tr.m_pEnt;
		if( pEntity != NULL && pEntity != this )
			m_iIDEntIndex = pEntity->entindex( );
	}
}

void C_HDTF_Player::UpdateIronsightDistanceFraction()
{
	C_BaseHDTFCombatWeapon* pWeapon = dynamic_cast<C_BaseHDTFCombatWeapon*>(GetActiveWeapon());
	if (pWeapon && pWeapon->IsIronsightsEnabled())
	{
		Vector origin, forward;
		EyePositionAndVectors(&origin, &forward, NULL, NULL);

		trace_t tr;
		UTIL_TraceLine(origin, origin + forward * 500.f, MASK_SOLID, this, COLLISION_GROUP_NONE, &tr);
		m_flIronsightDistanceFraction = Approach(tr.fraction, m_flIronsightDistanceFraction, gpGlobals->frametime * 4.f);
	}
	else
	{
		m_flIronsightDistanceFraction = Approach(0.0f, m_flIronsightDistanceFraction, gpGlobals->frametime * 4.f);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Update this client's inventory visibility
//-----------------------------------------------------------------------------
void C_HDTF_Player::UpdateInventoryState()
{
	CHudInventory* inventory = CHudInventory::GetCurrent();
	if (!inventory)
	{
		return;
	}

	if (inventory->IsActive() != m_bDesiredInventoryState)
	{
		if (m_bDesiredInventoryState)
		{
			inventory->ShowInventory();
		}
		else
		{
			inventory->HideInventory();
		}
	}
}

C_HDTF_Player *C_HDTF_Player::GetLocalHDTFPlayer( )
{
	return static_cast<C_HDTF_Player *>( C_BasePlayer::GetLocalPlayer( ) );
}

void C_HDTF_Player::CalcPlayerView(Vector& eyeOrigin, QAngle& eyeAngles, float& fov)
{
	BaseClass::CalcPlayerView(eyeOrigin, eyeAngles, fov);
	m_pParkourController.CalcView(eyeOrigin, eyeAngles, fov);
}
