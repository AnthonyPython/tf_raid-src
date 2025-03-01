// TODO: Fix this broken crap

#include "cbase.h"
#include "npcevent.h"
#include "in_buttons.h"
#include "effect_dispatch_data.h"
#include "weapon_basehdtfcombat.h"
#include "hdtf_player_shared.h"

#ifdef CLIENT_DLL

#include "c_sprite.h"
#include "tier2/beamsegdraw.h"
#include "util.h"

#include "view.h"

#else

#include "grenade_molotov.h"
#include "Sprite.h"
#include "IEffects.h"

#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define RETHROW_DELAY 0.5f

#define GRENADE_RADIUS 4.0f // inches

#define BG_VISIBLE 1
#define BG_INVISIBLE 0

#define MOLOTOV_LIGHTER_SPRITE "sprites/candle_flame.vmt"

#ifdef CLIENT_DLL

#define CWeaponMolotov C_WeaponMolotov

#endif

class CWeaponMolotov : public CBaseHDTFCombatWeapon
{
public:
	DECLARE_CLASS( CWeaponMolotov, CBaseHDTFCombatWeapon );
	DECLARE_NETWORKCLASS( );
	DECLARE_PREDICTABLE( );

#ifndef CLIENT_DLL

	DECLARE_ACTTABLE( );

#endif

	CWeaponMolotov( );

	~CWeaponMolotov( );

	void Precache( );
	void PrimaryAttack( );
	void SecondaryAttack( );
	void DecrementAmmo( CBaseCombatCharacter *pOwner );
	void ItemPostFrame( );
	void Spawn( );

	bool Deploy( );
	bool Holster( CBaseCombatWeapon *pSwitchingTo = NULL );

	bool Reload( );


#ifndef CLIENT_DLL

	void Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator );
	void RemoveFlames( );
	void DoLighterSparks();

#else

	void ViewModelDrawn( C_BaseViewModel *pBaseViewModel );

	void DrawLighterFlameSprite( const Vector &vecOrigin, const Vector &vecUp );

#endif

private:
	CWeaponMolotov( const CWeaponMolotov & );

	void CheckThrowPosition( CBasePlayer *pPlayer, const Vector &vecEye, Vector &vecSrc );
	void ThrowMolotov( CBasePlayer *pPlayer );

	CNetworkVar( bool, m_bRedraw );

	CNetworkVar( bool, m_bPullback );

	CNetworkVar( bool, m_bFlameVisible);


#ifndef CLIENT_DLL

	CSprite *pLighterFlame;

#else

	Vector				m_vLastFlameDir;
	float				m_flFlameSize;
	float				m_flCurrentFlameSize;
	float				m_flLastSizeChange;

	CMaterialReference	m_hFlameMaterial;
#endif

	int m_iExplosionStep;
	bool m_bIgnited;
};

#ifndef CLIENT_DLL

acttable_t CWeaponMolotov::m_acttable[] =
{
	{ ACT_HL2MP_IDLE, ACT_HL2MP_IDLE_GRENADE, false },
	{ ACT_HL2MP_RUN, ACT_HL2MP_RUN_GRENADE, false },
	{ ACT_HL2MP_IDLE_CROUCH, ACT_HL2MP_IDLE_CROUCH_GRENADE, false },
	{ ACT_HL2MP_WALK_CROUCH, ACT_HL2MP_WALK_CROUCH_GRENADE, false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK, ACT_HL2MP_GESTURE_RANGE_ATTACK_GRENADE, false },
	{ ACT_HL2MP_GESTURE_RELOAD, ACT_HL2MP_GESTURE_RELOAD_GRENADE, false },
	{ ACT_HL2MP_JUMP, ACT_HL2MP_JUMP_GRENADE, false },
};

IMPLEMENT_ACTTABLE( CWeaponMolotov );

#endif

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponMolotov, DT_WeaponMolotov )

BEGIN_NETWORK_TABLE( CWeaponMolotov, DT_WeaponMolotov )

#ifdef CLIENT_DLL

RecvPropBool( RECVINFO( m_bRedraw ) ),
RecvPropBool( RECVINFO( m_bPullback ) ),
RecvPropBool( RECVINFO( m_bFlameVisible ) ),

#else

SendPropBool( SENDINFO( m_bRedraw ) ),
SendPropBool( SENDINFO( m_bPullback ) ),
SendPropBool( SENDINFO( m_bFlameVisible ) ),

#endif

END_NETWORK_TABLE( )

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( CWeaponMolotov )
DEFINE_PRED_FIELD( m_bRedraw, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
DEFINE_PRED_FIELD( m_bPullback, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
DEFINE_PRED_FIELD( m_bFlameVisible, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
END_PREDICTION_DATA( )
#endif

LINK_ENTITY_TO_CLASS( weapon_molotov, CWeaponMolotov );
PRECACHE_WEAPON_REGISTER( weapon_molotov );

CWeaponMolotov::CWeaponMolotov( )
{

#ifndef CLIENT_DLL

	pLighterFlame = NULL;

#else
	m_flFlameSize = 0.f;
	m_flCurrentFlameSize = 0.f;
	m_flLastSizeChange = 0.f;
	m_vLastFlameDir = vec3_origin;
#endif

	m_bFlameVisible = false;
	m_bRedraw = false;
	m_bPullback = false;
}

CWeaponMolotov::~CWeaponMolotov( )
{

#ifndef CLIENT_DLL

	if( pLighterFlame != NULL )
		UTIL_Remove( pLighterFlame );

#endif

}

void CWeaponMolotov::Precache( )
{
	BaseClass::Precache( );

#ifndef CLIENT_DLL
	UTIL_PrecacheOther( "grenade_molotov" );
	UTIL_PrecacheOther( "env_fire" );
#endif

	PrecacheModel("sprites/glow01.vmt");
	PrecacheModel( MOLOTOV_LIGHTER_SPRITE );

	PrecacheScriptSound( "WeaponMolotov.Throw" );

	PrecacheModel("models/weapons/w_molotov.mdl");
	PrecacheScriptSound("WeaponMolotov.Explode");
	PrecacheParticleSystem("hdtf_molotov");
}

void CWeaponMolotov::Spawn( )
{
	SetPrimaryAmmoCount( GetDefaultClip1( ) );

	m_iClip1 = WEAPON_NOCLIP;

	BaseClass::Spawn( );
}

#ifndef CLIENT_DLL
void CWeaponMolotov::DoLighterSparks()
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (!pOwner)
		return;

	QAngle angSpark;
	Vector vecSpark, vecDir;
	int attachment = pOwner->GetViewModel()->LookupAttachment("lighter_sparks");

	pOwner->GetViewModel()->GetAttachment(attachment, vecSpark, angSpark);
	AngleVectors(angSpark, &vecDir);

	// TODO: replace with particle sparks
	g_pEffects->Sparks(vecSpark, 1, 1, &vecDir);
}

void CWeaponMolotov::Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator )
{
	switch( pEvent->event )
	{
	
	case EVENT_WEAPON_MELEE_HIT:
		DoLighterSparks();

		m_bFlameVisible = true;
		break;

	case EVENT_WEAPON_MELEE_SWISH:
		m_bFlameVisible = false;
		break;

	default:
		BaseClass::Operator_HandleAnimEvent( pEvent, pOperator );
		break;
	}
}

#endif

bool CWeaponMolotov::Deploy( )
{
	m_bRedraw = false;
	m_bPullback = false;
	m_bFlameVisible = false;

	return BaseClass::Deploy( );
}

bool CWeaponMolotov::Holster( CBaseCombatWeapon *pSwitchingTo )
{
	m_bRedraw = false;
	m_bPullback = false;
	m_bFlameVisible = false;

	return BaseClass::Holster( pSwitchingTo );
}

bool CWeaponMolotov::Reload( )
{
	if( !HasPrimaryAmmo( ) )
		return false;

	if( m_bRedraw )
	{
		SendWeaponAnim( ACT_VM_DRAW );

		m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration( );
		m_flNextSecondaryAttack = gpGlobals->curtime + SequenceDuration( );
		m_flTimeWeaponIdle = gpGlobals->curtime + SequenceDuration( );

		m_bRedraw = false;
	}

	return true;
}

void CWeaponMolotov::SecondaryAttack( )
{ }

void CWeaponMolotov::PrimaryAttack( )
{
	if( m_bRedraw )
		return;

	if (!HasPrimaryAmmo())
		return;

	CBasePlayer *pPlayer = ToBasePlayer( GetOwner( ) );
	if( pPlayer == NULL )
		return;

	if( pPlayer->GetMoveType( ) == MOVETYPE_LADDER || pPlayer->GetWaterLevel( ) == 3 )
		return;

	SendWeaponAnim( ACT_VM_PULLBACK_HIGH );
	m_bPullback = true;

	m_flTimeWeaponIdle = FLT_MAX;
	m_flNextPrimaryAttack = FLT_MAX;
	m_flNextSecondaryAttack = FLT_MAX;
}

#ifndef CLIENT_DLL

void CWeaponMolotov::RemoveFlames( )
{
	if( pLighterFlame != NULL )
		pLighterFlame->TurnOff( );
}

#endif

void CWeaponMolotov::DecrementAmmo( CBaseCombatCharacter *pOwner )
{
	pOwner->RemoveAmmo( 1, m_iPrimaryAmmoType );
}

void CWeaponMolotov::ItemPostFrame( )
{
	CHDTF_Player *pOwner = ToHDTFPlayer(GetOwner());
	if (pOwner == NULL)
		return;

	bool primaryButton = (pOwner->m_nButtons & IN_ATTACK) != 0,
		isSprinting = pOwner->IsSprinting();

	if (m_bPullback)
	{
		if (!primaryButton && IsViewModelSequenceFinished())
		{
			ThrowMolotov(pOwner);
			m_bPullback = false;
		}
	}

	if (primaryButton && m_flNextPrimaryAttack < gpGlobals->curtime)
		PrimaryAttack();

	if (m_bRedraw && IsViewModelSequenceFinished())
		Reload();

	if (m_bLowered || isSprinting ||
		(!primaryButton &&
		(!m_bRedraw) && !ReloadOrSwitchWeapons() && !m_bInReload))
		WeaponIdle();
}

void CWeaponMolotov::CheckThrowPosition( CBasePlayer *pPlayer, const Vector &vecEye, Vector &vecSrc )
{
	trace_t tr;
	UTIL_TraceHull( vecEye, vecSrc,
		-Vector( GRENADE_RADIUS + 2, GRENADE_RADIUS + 2, GRENADE_RADIUS + 2 ),
		Vector( GRENADE_RADIUS + 2, GRENADE_RADIUS + 2, GRENADE_RADIUS + 2 ),
		pPlayer->PhysicsSolidMaskForEntity( ), pPlayer, pPlayer->GetCollisionGroup( ), &tr );
	if( tr.DidHit( ) )
		vecSrc = tr.endpos;
}

void CWeaponMolotov::ThrowMolotov( CBasePlayer *pPlayer )
{
	DecrementAmmo(pPlayer);
#ifndef CLIENT_DLL

	if( pPlayer == NULL )
		return;

	Vector vecEye = pPlayer->EyePosition( );
	Vector vForward, vRight;

	pPlayer->EyeVectors( &vForward, &vRight, NULL );
	Vector vecSrc = vecEye + vForward * 18.0f + vRight * 10.0f;
	CheckThrowPosition( pPlayer, vecEye, vecSrc );
	vForward[2] += 0.1f;

	Vector vecThrow;
	pPlayer->GetVelocity( &vecThrow, NULL );
	vecThrow += vForward * 1200.0f;

	CGrenade_Molotov *pGrenade = static_cast<CGrenade_Molotov *>( Create( "grenade_molotov", vecSrc, vec3_angle, pPlayer ) );
	if( pGrenade != NULL )
	{
		pGrenade->SetVelocity( vecThrow, AngularImpulse( 600.0f, random->RandomInt( -1200.0f, 1200.0f ), 0.0f ) );
		pGrenade->SetThrower( ToBaseCombatCharacter( pPlayer ) );
		pGrenade->m_takedamage = DAMAGE_EVENTS_ONLY;
		pGrenade->ApplyAbsVelocityImpulse( vecThrow );
		pGrenade->SetLocalAngularVelocity( QAngle( 0.0f, 0.0f, random->RandomFloat( -100.0f, -500.0f ) ) );
		pGrenade->m_pDamageParent = pPlayer;
	}

#endif

	m_bRedraw = true;

	WeaponSound( SINGLE );

	pPlayer->SetAnimation(PLAYER_ATTACK1);
	SendWeaponAnim(ACT_VM_THROW);

	m_flTimeWeaponIdle = gpGlobals->curtime + SequenceDuration();
	m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
	m_flNextSecondaryAttack = gpGlobals->curtime + SequenceDuration();

	if (!HasPrimaryAmmo())
		pPlayer->SwitchToNextBestWeapon(this);
}

#ifdef CLIENT_DLL

void CWeaponMolotov::ViewModelDrawn( C_BaseViewModel *pBaseViewModel )
{
	if(IsWeaponVisible() && m_bFlameVisible && IsCarriedByLocalPlayer())
	{
		if(m_hFlameMaterial == NULL)
			m_hFlameMaterial.Init(MOLOTOV_LIGHTER_SPRITE, TEXTURE_GROUP_CLIENT_EFFECTS);

		QAngle angles;
		Vector vecAttach;
		int attachment = LookupAttachment("lighter_fire");

		UTIL_GetWeaponAttachment(this, attachment, vecAttach, angles);

		materials->GetRenderContext()->Bind(m_hFlameMaterial, this);

		Vector fireUp = vecAttach + Vector(0, 0, 5), dirUp = Vector(0, 0, 1);

		if (m_vLastFlameDir == vec3_origin || m_vLastFlameDir.DistTo(fireUp) > 24.f)
			m_vLastFlameDir = fireUp;

		VectorSubtract(m_vLastFlameDir, vecAttach, dirUp);
		VectorNormalize(dirUp);

		DrawLighterFlameSprite(vecAttach, dirUp);

		InterpolateVector(0.32f, m_vLastFlameDir, fireUp, m_vLastFlameDir);
	}
	else
	{
		m_flFlameSize = 0.f;
		m_flCurrentFlameSize = 0.f;
	}

	BaseClass::ViewModelDrawn(pBaseViewModel);
}

void CWeaponMolotov::DrawLighterFlameSprite( const Vector &vecOrigin, const Vector &vecUp )
{
	if (m_flFlameSize == 0.f || m_flLastSizeChange <= gpGlobals->curtime)
	{
		m_flFlameSize = RandomFloat(0.7f, 1.2f);

		m_flLastSizeChange = gpGlobals->curtime + RandomFloat(0.02f, 0.04f);
	}

	m_flCurrentFlameSize = Approach(m_flFlameSize, 
		m_flCurrentFlameSize, 
		gpGlobals->frametime * 3.2f);

	unsigned char pColor[4] = { 255, 255, 255, 255 };

	float flWidth = 0.06;
	flWidth *= 0.5f;
	float flHeight = 1.6 * m_flCurrentFlameSize;

	// Compute direction vectors for the sprite
	Vector fwd, right(1, 0, 0);
	VectorSubtract(CurrentViewOrigin(), vecOrigin, fwd);
	CrossProduct(CurrentViewUp(), fwd, right);

	CMeshBuilder meshBuilder;
	Vector point;
	CMatRenderContextPtr pRenderContext(materials);
	IMesh* pMesh = pRenderContext->GetDynamicMesh();

	meshBuilder.Begin(pMesh, MATERIAL_QUADS, 1);

	meshBuilder.Color4ubv(pColor);
	meshBuilder.TexCoord2f(0, 0, 1);
	point = vecOrigin;
	VectorMA(point, -flWidth, right, point);
	meshBuilder.Position3fv(point.Base());
	meshBuilder.AdvanceVertex();

	meshBuilder.Color4ubv(pColor);
	meshBuilder.TexCoord2f(0, 0, 0);
	point = vecOrigin;
	VectorMA(vecOrigin, flHeight, vecUp, point);
	VectorMA(point, -flWidth, right, point);
	meshBuilder.Position3fv(point.Base());
	meshBuilder.AdvanceVertex();

	meshBuilder.Color4ubv(pColor);
	meshBuilder.TexCoord2f(0, 1, 0);
	point = vecOrigin;
	VectorMA(vecOrigin, flHeight, vecUp, point);
	VectorMA(point, flWidth, right, point);
	meshBuilder.Position3fv(point.Base());
	meshBuilder.AdvanceVertex();

	meshBuilder.Color4ubv(pColor);
	meshBuilder.TexCoord2f(0, 1, 1);
	point = vecOrigin;
	VectorMA(point, flWidth, right, point);
	meshBuilder.Position3fv(point.Base());
	meshBuilder.AdvanceVertex();

	meshBuilder.End();
	pMesh->Draw();
}

#endif
