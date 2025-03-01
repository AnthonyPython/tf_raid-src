#ifndef WEAPON_BASEHDTFCOMBAT_H
#define WEAPON_BASEHDTFCOMBAT_H

#ifdef _WIN32
#pragma once
#endif

#ifdef CLIENT_DLL

class Beam_t;
class C_LaserDot;
#define CLaserDot C_LaserDot

#else

class CLaserDot;

#endif

#include "basehlcombatweapon_shared.h"

#if defined( CLIENT_DLL )

#define CBaseHDTFCombatWeapon C_BaseHDTFCombatWeapon

enum class EScopeType : uint8
{
	SCOPE_NONE,
	SCOPE_SNIPER,
	SCOPE_COLLIMATOR,
};

#endif

#define HDTF_HUD_REVEAL_TIMER 0.85f

class CBaseHDTFCombatWeapon : public CBaseHLCombatWeapon
{
public:
	DECLARE_CLASS( CBaseHDTFCombatWeapon, CBaseHLCombatWeapon );
	DECLARE_NETWORKCLASS( );
	DECLARE_PREDICTABLE( );

#ifndef CLIENT_DLL

	DECLARE_DATADESC( );

#endif

	CBaseHDTFCombatWeapon( );
	virtual ~CBaseHDTFCombatWeapon( );

	const Vector &GetBulletSpread( );
	virtual Vector GetBulletSpread(WeaponProficiency_t proficiency);

	virtual void Activate( );
	virtual void Drop( const Vector &vecVelocity );
	virtual bool Deploy();
	virtual bool Holster( CBaseCombatWeapon *pSwitchingTo );
	virtual void Equip( CBaseCombatCharacter *pOwner );
	virtual void WeaponIdle( );
	virtual void InventoryDeploy();

	virtual void Precache( );
	virtual void ItemBusyFrame();
	virtual void ItemPostFrame( );

	virtual Activity GetPrimaryAttackActivity( );
	virtual bool CanPrimaryAttack( );
	virtual void PrimaryAttack( );
	virtual void SecondaryAttack( );
	virtual void SpecialAttack( );

	virtual bool WeaponShouldBeLowered( );

	virtual bool ShouldDisplayAltFireHUDHint() { return false; }
	virtual bool ShouldDisplayReloadHUDHint() { return false; }

#ifndef CLIENT_DLL
	virtual bool BusyFrameForcesUnLean() const { return true; }
	virtual void ProcessCornerLean();
	void ForceUnLean();
	virtual void ModifyDamageViewPunch(float& flOutPunch) { /*do nothing by default*/ }
#endif

	// Check if the weapon should be lowered. Moved from ItemPostFrame
	// in case we override it so we don't need to write this again.
	virtual void CheckLoweredCondition();

#ifdef CLIENT_DLL

	// We need to render opaque and translucent pieces
	virtual RenderGroup_t GetRenderGroup( );

	void GetWeaponAttachment( int attachmentId, Vector &outVector, Vector *dir = NULL );

	virtual void OnRestore();

	virtual bool ShouldDrawAmmo() { return true; }

	virtual float GetIronSpeed() { return 3.5f; }

	virtual EScopeType GetScopeType() { return EScopeType::SCOPE_NONE; }
	virtual char *GetScopeReticleTextureName() { return NULL; }
	virtual int GetScopeReticleTextureId();

#endif //CLIENT_DLL

	virtual bool ObjectInWay( ) const;

	// return true if this weapon 
	// can be deselected in the inventory
	virtual bool CanBeDeselected( ) const;

	virtual bool CanBeSelected( );
	virtual bool VisibleInWeaponSelection( );
	virtual void SetVisibleInWeaponSelection( bool visible );

	virtual bool HasIronsights( ) const;
	bool IsIronsightsEnabled( ) const;
	void ToggleIronsights( );
	virtual void EnableIronsights( );
	virtual void DisableIronsights( bool ignoreAnimation = false );
	virtual float GetIronSensitivity() { return 0.f; } // mouse sensitivity whilst ironsighted

	bool IsLeaning() { return m_bIsCornerLeaning; }
	virtual bool ShouldAutoLean() { return true; }
	//M3SA: disabled auto lean because it was annoying
	//M3SA: renabled because I forgot it was its own thing

	void SetSpecialEnabled( bool enabled );
	virtual bool IsSpecialEnabled( ) const;

	virtual bool Reload( );
	virtual bool DefaultReload( int iClipSize1, int iClipSize2, int iActivity );
	virtual bool CanReload();

	virtual void	AddViewmodelBob(CBaseViewModel *viewmodel, Vector &origin, QAngle &angles);
	virtual float	CalcViewmodelBob();

	// override in child classes if needed
	virtual bool HasFlashlight() { return false; }
	virtual float FlashlightFov() { return 45.f; }
	virtual float FlashlightNearZ() { return 4.f; }
	virtual float FlashlightFarZ() { return 740.f; }

	// use to compute FOV value accounting for player's FOV setting;
	// for uses like weapon ironsights zooming the view in to get consistent results
	// with different FOV settings.
	virtual int CalcViewCorrectedFov(const int desired) const;

protected:
	CNetworkVar( float, m_flNextSpecialAttack );

	CNetworkVector( m_vecNPCLaserDot );

	CNetworkVar( bool, m_bLaserActive );
	CNetworkHandle( CLaserDot, m_hLaserDot );

#ifdef CLIENT_DLL

	float m_flViewBobMultiplier;

	int m_iScopeReticleTextureId;

#endif

	CNetworkVar( bool, m_bVisibleInWeaponSelection );

	CNetworkVar( bool, m_bIsIronsighted );
	CNetworkVar( float, m_flIronsightedTime );

	CNetworkVar( bool, m_bIsSpecialEnabled );

	CNetworkVar( bool, m_bIsCornerLeaning );

	CNetworkVar( bool, m_bIsReloadKeyDown );

	CNetworkVar( bool, m_bDelayedReload );

	float m_flHudRevealTimer;

private:
	CBaseHDTFCombatWeapon( const CBaseHDTFCombatWeapon & );

	friend class CBaseViewModel;
};

#endif // WEAPON_BASEHDTFCOMBAT_H
