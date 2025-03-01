#include "cbase.h"
#include "materialsystem/imaterialvar.h"
#include "materialsystem/imaterialproxy.h"
#include "baseviewmodel_shared.h"
#ifdef DMCS_DLL
#include "c_dmc_player.h"
#endif

class C_CloakProxy : public IMaterialProxy
{
public:
    C_CloakProxy();
    virtual ~C_CloakProxy();
    virtual bool Init(IMaterial* pMaterial, KeyValues* pKeyValues);
    C_BaseEntity* BindArgToEntity(void* pArg);
    virtual void OnBind(void* C_BaseEntity);
    virtual void Release(void) { delete this; }
    IMaterial* GetMaterial(void);

private:
    IMaterialVar* cloakFactorVar;
};

C_CloakProxy::C_CloakProxy()
{
    cloakFactorVar = NULL;
}

C_CloakProxy::~C_CloakProxy()
{
}

bool C_CloakProxy::Init(IMaterial* pMaterial, KeyValues* pKeyValues)
{
    bool found;

    pMaterial->FindVar("$cloakpassenabled", &found, false);
    if (!found)
        return false;

    cloakFactorVar = pMaterial->FindVar("$cloakfactor", &found, false);
    if (!found)
        return false;

    return true;
}

C_BaseEntity* C_CloakProxy::BindArgToEntity(void* pArg)
{
    IClientRenderable* pRend = (IClientRenderable*)pArg;
    return pRend ? pRend->GetIClientUnknown()->GetBaseEntity() : NULL;
}

void C_CloakProxy::OnBind(void* pC_BaseEntity)
{
	C_BaseEntity* pEntity = BindArgToEntity(pC_BaseEntity);
    if ( !pEntity )
    {
        cloakFactorVar->SetFloatValue( 0.0f );
        return;
    }

#ifdef DMCS_DLL
	C_DMCRagdoll* pRagdoll = dynamic_cast<C_DMCRagdoll *>(pEntity);
#endif

	//If this is a player's viewmodel...
	if (C_BaseViewModel* pViewModel = dynamic_cast<C_BaseViewModel*>(pEntity))
	{
		C_BasePlayer* pPlayer = ToBasePlayer(pViewModel->GetOwner());
		cloakFactorVar->SetFloatValue(pPlayer->GetCloakFactor());
	}

	//If this is a non-player character...
	else if (C_BaseCombatCharacter* pNPC = dynamic_cast<C_BaseCombatCharacter*>(pEntity))
	{
		cloakFactorVar->SetFloatValue(pNPC->GetCloakFactor());
	}
#ifdef DMCS_DLL
	else if (pRagdoll)
	{
		cloakFactorVar->SetFloatValue(0.0); //Another dumb fix - AnthonyH
		return;
	}
#endif
    //If this is a weapon's worldmodel (under the assumption it's in something's possesion)...
    else if (C_BaseCombatWeapon* pWeapon = dynamic_cast<C_BaseCombatWeapon*>(pEntity))
    {
        C_BaseCombatCharacter* pOwner = ToBaseCombatCharacter(pWeapon->GetOwner());
        if (!pOwner)
        {
            cloakFactorVar->SetFloatValue(0.0); //Dumb fix to prevent Item spawned weapons turning invis but it works. -AnthonyH
            return;
        }
            

        cloakFactorVar->SetFloatValue(pOwner->GetCloakFactor());
    }
    else
        return;
}
IMaterial* C_CloakProxy::GetMaterial()
{
    return cloakFactorVar->GetOwningMaterial();
}
EXPOSE_INTERFACE(C_CloakProxy, IMaterialProxy, "Invisibility" IMATERIAL_PROXY_INTERFACE_VERSION);