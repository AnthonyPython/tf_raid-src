#include "cbase.h"
#include "proxyentity.h"
#include "materialsystem/imaterial.h"
#include "materialsystem/imaterialvar.h"
#include "viewrender.h"

class CHDTFScopeProxy : public CEntityMaterialProxy
{
public:
	CHDTFScopeProxy() { m_ReflectionVar = NULL; }
	virtual ~CHDTFScopeProxy() {}

	virtual bool Init(IMaterial* pMaterial, KeyValues* pKeyValues);
	virtual void OnBind(C_BaseEntity *pEnt);
	virtual IMaterial *GetMaterial();

private:
	IMaterialVar	*m_ReflectionVar;
	float			m_flMinValue;
	float			m_flMaxValue;
};

bool CHDTFScopeProxy::Init(IMaterial* pMaterial, KeyValues* pKeyValues)
{
	bool foundVar;
	m_ReflectionVar = pMaterial->FindVar("$envmaptint", &foundVar, false);

	m_flMinValue = pKeyValues->GetFloat("minReflection");
	m_flMaxValue = pKeyValues->GetFloat("maxReflection");

	return foundVar;
}

void CHDTFScopeProxy::OnBind(C_BaseEntity *pEnt)
{
	if (!m_ReflectionVar)
		return;

	static ConVarRef r_scope_reflection_enabled("r_scope_reflection_enabled");
	if (r_scope_reflection_enabled.GetBool())
	{
		m_ReflectionVar->SetVecValue(0, 0, 0);
		return;
	}

	float value = Lerp(
		view->GetScopeTransition(),
		m_flMaxValue,
		m_flMinValue
	);

	m_ReflectionVar->SetVecValue(value, value, value);
}

IMaterial *CHDTFScopeProxy::GetMaterial()
{
	if (!m_ReflectionVar)
		return NULL;

	return m_ReflectionVar->GetOwningMaterial();
}

EXPOSE_INTERFACE(CHDTFScopeProxy, IMaterialProxy, "RTScope" IMATERIAL_PROXY_INTERFACE_VERSION);