#include "cbase.h"
#include "grenade_molotov.h"
#include "fire.h"
#include "props.h"
#include "gib.h"
#include "te_effect_dispatch.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CFlamingGib : public CGib
{
public:
	DECLARE_CLASS( CFlamingGib, CGib );
};

LINK_ENTITY_TO_CLASS( flaming_gib, CFlamingGib );

extern short g_sModelIndexFireball;

extern ConVar sk_plr_dmg_molotov;
extern ConVar sk_npc_dmg_molotov;
ConVar sk_molotov_radius( "sk_molotov_radius", "128" );
ConVar sk_molotov_fire_height( "sk_molotov_fire_height", "1.45" );
ConVar sk_molotov_fire_attack_duration( "sk_molotov_fire_attack_duration", "17.5" );

#define NUM_FIREBALLS 13

#ifndef CLIENT_DLL

BEGIN_DATADESC( CGrenade_Molotov )
	DEFINE_FIELD( m_pFireTrail, FIELD_CLASSPTR ),
	DEFINE_ENTITYFUNC ( MolotovTouch ),
	DEFINE_THINKFUNC( MolotovThink ),
END_DATADESC( )

#endif

LINK_ENTITY_TO_CLASS( grenade_molotov, CGrenade_Molotov );

CGrenade_Molotov::~CGrenade_Molotov( )
{
	UTIL_Remove( m_pFireTrail );
}

void CGrenade_Molotov::Spawn( )
{
	Precache( );

	SetMoveType( MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_BOUNCE );
	SetSolid( SOLID_BBOX ); 
	SetCollisionGroup( COLLISION_GROUP_PROJECTILE );
	RemoveEffects( EF_NOINTERP );

	SetModel( "models/weapons/w_molotov.mdl" );

	SetTouch( &CGrenade_Molotov::MolotovTouch );
	SetThink( &CGrenade_Molotov::MolotovThink );

	SetNextThink( gpGlobals->curtime + 0.1f );

	m_flDamage = sk_plr_dmg_molotov.GetFloat();
	m_DmgRadius = sk_molotov_radius.GetFloat();

	m_takedamage = DAMAGE_YES;
	m_iHealth = 1;

	SetGravity( 1.0f );
	SetFriction( 0.8f );
	SetSequence( 1 );

	CFireTrail *fire = CFireTrail::CreateFireTrail( );
	if( fire != NULL )
	{
		fire->FollowEntity( this, "flame" );
		fire->SetLocalOrigin( vec3_origin );
		fire->SetMoveType( MOVETYPE_NONE );
		fire->SetLifetime( 20.0f ); 
	}
}

void CGrenade_Molotov::MolotovTouch( CBaseEntity *pOther )
{
	if ( pOther->IsSolidFlagSet( FSOLID_TRIGGER | FSOLID_VOLUME_CONTENTS ) && pOther->GetCollisionGroup( ) != COLLISION_GROUP_WEAPON )
		return;

	Detonate( );
}

void CGrenade_Molotov::CreateFlyingChunk( const Vector &vecChunkPos, const QAngle &vecChunkAngles, const char *pszChunkName, bool bSmall )
{
	CFlamingGib *pChunk = CREATE_ENTITY( CFlamingGib, "flaming_gib" );
	pChunk->Spawn( pszChunkName );
	pChunk->SetBloodColor( DONT_BLEED );

	pChunk->SetAbsOrigin( vecChunkPos );
	pChunk->SetAbsAngles( vecChunkAngles );
	pChunk->SetOwnerEntity( this );

	pChunk->m_lifeTime = 15.0f;

	pChunk->SetNextThink( gpGlobals->curtime + pChunk->m_lifeTime );
	pChunk->SetThink ( &CGib::DieThink );

	pChunk->m_takedamage = DAMAGE_NO;
	
	pChunk->SetCollisionGroup( COLLISION_GROUP_DEBRIS );
	pChunk->AddEffects( EF_NODRAW );
	
	QAngle angles( random->RandomFloat( -70.0f, 20.0f ), random->RandomFloat( 0.0f, 360.0f ), 0.0f );

	Vector vecVelocity;
	AngleVectors( angles, &vecVelocity );

	vecVelocity *= random->RandomFloat( 10.0f, 20.0f );
	vecVelocity.z *= random->RandomFloat( 10.0f, 20.0f );
	vecVelocity.y *= random->RandomFloat( 5.0f, 10.0f );
	vecVelocity.x *= random->RandomFloat( 5.0f, 10.0f );

	AngularImpulse angImpulse = RandomAngularImpulse( -90.0f, 90.0f );

	pChunk->SetAbsVelocity( vecVelocity );

	IPhysicsObject *pPhysicsObject = pChunk->VPhysicsInitNormal( SOLID_NONE	, pChunk->GetSolidFlags( ), false );
	if( pPhysicsObject != NULL )
	{
		pPhysicsObject->EnableMotion( true );
		pPhysicsObject->SetVelocity(&vecVelocity, &angImpulse );
	}

	CFireTrail *pFireTrail = CFireTrail::CreateFireTrail( );
	if( pFireTrail == NULL )
		return;

	pFireTrail->FollowEntity( pChunk, "" );
	pFireTrail->SetParent( pChunk, 0 );
	pFireTrail->SetLocalOrigin( vec3_origin );
	pFireTrail->SetMoveType( MOVETYPE_NONE );
	pFireTrail->SetLifetime( pChunk->m_lifeTime - 1.0f );

}

void CGrenade_Molotov::Detonate( ) 
{
	SetModelName( NULL_STRING );
	AddSolidFlags( FSOLID_NOT_SOLID );

	m_takedamage = DAMAGE_NO;
	
	trace_t trace;
	trace = GetTouchTrace( );

	if( trace.fraction != 1.0f )
		SetLocalOrigin( trace.endpos + trace.plane.normal * (sk_plr_dmg_molotov.GetFloat() - 24.0f) * 0.6f);

	int contents = UTIL_PointContents( GetAbsOrigin( ) );
	
	if ( ( contents & MASK_WATER ) != 0 )
	{
		UTIL_Remove( this );
		return;
	}

	EmitSound( "WeaponMolotov.Explode" );

	/*for( int i = 0 ; i < 13 ; ++i )
	{
		QAngle vecTraceAngles( random->RandomFloat( 45.0f, 135.0f ), random->RandomFloat( 0.0f, 360.0f ), 0.0f );

		Vector vecTraceDir;
		AngleVectors( vecTraceAngles, &vecTraceDir );

		Vector vecStart = GetAbsOrigin() + trace.plane.normal * 48.0f;
		vecStart.z = vecStart.z + 32.0f;

		Vector vecEnd = vecStart + vecTraceDir * 420.0f;

		trace_t firetrace;
		UTIL_TraceLine( vecStart, vecEnd, MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &firetrace );

		Vector ofsDir = firetrace.endpos - GetAbsOrigin();
		float offset = VectorNormalize( ofsDir );
		if( offset > 128.0f )
			offset = 128.0f;

		float growth = 0.8f + 0.75f * ( offset / 128.0f );
		float scale = 165.f;

		if( firetrace.fraction != 1.0f )
			FireSystem_StartFire( firetrace.endpos, scale, growth, 17.5f, SF_FIRE_START_ON | SF_FIRE_SMOKELESS | SF_FIRE_CREATED_BY_MOLOTOV, m_pDamageParent , FIRE_NATURAL );
		else
			FireSystem_StartFire( trace.endpos + trace.plane.normal * 12.0f, scale, growth, 10.0f, SF_FIRE_START_ON | SF_FIRE_SMOKELESS | SF_FIRE_CREATED_BY_MOLOTOV, m_pDamageParent, FIRE_NATURAL );
	}*/

	CBaseEntity *pOwner = GetOwnerEntity( );
	SetOwnerEntity( NULL );

	// BUG(wheatley): if we attempt to apply this decal to a character then engine will run out indices!
	CBaseCombatCharacter *pHitCharacter = dynamic_cast<CBaseCombatCharacter *>(trace.m_pEnt);
	if(!pHitCharacter)
		UTIL_DecalTrace( &trace, "Scorch" );

	//UTIL_ScreenShake( GetAbsOrigin(), 10.0, 60.0, 1.0, 100, SHAKE_START );

	trace_t firetrace;
	UTIL_TraceLine(GetAbsOrigin(), GetAbsOrigin() - Vector(0, 0, 1024), MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &firetrace);

	float scale = 270.f;
	FireSystem_StartFire(firetrace.endpos, scale, sk_molotov_fire_height.GetFloat(), sk_molotov_fire_attack_duration.GetFloat(), SF_FIRE_START_ON | SF_FIRE_START_FULL | SF_FIRE_SMOKELESS | SF_FIRE_DONT_DROP | SF_FIRE_CREATED_BY_MOLOTOV, m_pDamageParent, FIRE_NATURAL);

	DispatchParticleEffect("hdtf_molotov", firetrace.endpos, QAngle(0, 0, 0), NULL);

	if( m_pDamageParent != NULL && 
		!m_pDamageParent->IsDisconnecting( ) &&
		m_pDamageParent->IsConnected( ) )
	{
		RadiusDamage( CTakeDamageInfo( this, m_pDamageParent, sk_plr_dmg_molotov.GetFloat(), DMG_BURN), GetAbsOrigin(), sk_molotov_radius.GetFloat(), CLASS_NONE, NULL);
		pOwner = NULL;
	}

	CBaseEntity *pObject = NULL;
	const Vector vecSource = GetAbsOrigin();
	while( ( pObject = gEntList.FindEntityInSphere( pObject, this->GetAbsOrigin( ), sk_molotov_radius.GetFloat()) ) != NULL )
	{
		Vector vecSpot = pObject->BodyTarget( vecSource, false );
		UTIL_TraceLine( vecSource, vecSpot, MASK_SOLID, pObject, COLLISION_GROUP_NONE, &trace );

		if( trace.fraction == 1.0f )
		{
			if( ( pObject == pOwner || pObject->IsNPC( ) ) && pObject != this )
			{
				CBaseCombatCharacter *pOther = dynamic_cast<CBaseCombatCharacter *>( pObject );
				if( pOther != NULL && pOther->GetWaterLevel( ) < 3 )
					pOther->Ignite( 100.0f );
			}
			else
			{
				CBreakableProp *pProp = dynamic_cast<CBreakableProp *>( pObject );
				if( pProp != NULL )
					pProp->Ignite( 100.0f, false );
			}
		}
	}

	SetAbsVelocity( vec3_origin );
	SetNextThink( TICK_NEVER_THINK );

	if( m_pFireTrail != NULL )
		UTIL_Remove( m_pFireTrail );

	UTIL_Remove( this );
}

void CGrenade_Molotov::MolotovThink( )
{
	int contents = UTIL_PointContents( GetAbsOrigin( ) );
	if( ( contents & MASK_WATER ) != 0 )
	{
		UTIL_Remove( this );
		return;
	}

	if( GetOwnerEntity( ) != NULL )
	{
		Vector vUpABit = GetAbsOrigin( );
		vUpABit.z += 5.0f;

		CBaseEntity *saveOwner = GetOwnerEntity( );
		SetOwnerEntity( NULL );

		trace_t tr;
		UTIL_TraceEntity( this, GetAbsOrigin( ), vUpABit, MASK_SOLID, &tr );

		if( tr.startsolid || tr.fraction != 1.0f )
			SetOwnerEntity( saveOwner );
	}

	SetNextThink( gpGlobals->curtime + 0.1f );
}

void CGrenade_Molotov::Precache( )
{
	BaseClass::Precache( );

	PrecacheModel( "models/weapons/w_molotov.mdl" );

	UTIL_PrecacheOther( "_firesmoke" );
	UTIL_PrecacheOther("env_fire");

	PrecacheScriptSound( "WeaponMolotov.Explode" );

	PrecacheParticleSystem("hdtf_molotov");
}

void CGrenade_Molotov::SetVelocity( const Vector &velocity, const AngularImpulse &angVelocity )
{
	IPhysicsObject *pPhysicsObject = VPhysicsGetObject( );
	if( pPhysicsObject != NULL )
		pPhysicsObject->AddVelocity( &velocity, &angVelocity );
}
