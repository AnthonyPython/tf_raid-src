#include "cbase.h"
#include "fx.h"
#include "c_te_effect_dispatch.h"
#include "tier0/vprof.h"
#include "fx_line.h"
#include "fx_quad.h"
#include "view.h"
#include "particles_localspace.h"
#include "dlight.h"
#include "iefx.h"
#include "clienteffectprecachesystem.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern Vector GetTracerOrigin(const CEffectData &data);
extern void FX_TracerSound(const Vector &start, const Vector &end, int iTracerType);

extern ConVar muzzleflash_light;

CLIENTEFFECT_REGISTER_BEGIN(PrecacheHDTFTracers)
CLIENTEFFECT_MATERIAL("effects/tracer_middle2")
CLIENTEFFECT_REGISTER_END()

//-----------------------------------------------------------------------------
// Purpose: Apache helicopter tracer
//-----------------------------------------------------------------------------
void ApacheHelicopterTracerCallback(const CEffectData &data)
{
	// Grab the data
	Vector vecStart = GetTracerOrigin(data);
	float flVelocity = data.m_flScale;

	// Use default velocity if none specified
	if (!flVelocity)
	{
		flVelocity = 8000;
	}

	//Get out shot direction and length
	Vector vecShotDir;
	VectorSubtract(data.m_vOrigin, vecStart, vecShotDir);
	float flTotalDist = VectorNormalize(vecShotDir);

	// Don't make small tracers
	if (flTotalDist <= 256)
		return;

	float flLength = random->RandomFloat(256.0f, 384.0f);
	float flLife = (flTotalDist + flLength) / flVelocity;

	// TODO: replace material with HDTF one
	FX_AddDiscreetLine(vecStart, vecShotDir, flVelocity, flLength, flTotalDist, 1.0f, flLife, "effects/tracer_middle2");

	if (data.m_fFlags & TRACER_FLAG_WHIZ)
	{
		FX_TracerSound(vecStart, data.m_vOrigin, TRACER_TYPE_GUNSHIP);
	}
}

DECLARE_CLIENT_EFFECT("ApacheHelicopterTracer", ApacheHelicopterTracerCallback);

//-----------------------------------------------------------------------------
// Purpose: Apache helicopter muzzleflash
//-----------------------------------------------------------------------------
void ApacheMuzzleFlashCallback(const CEffectData &data)
{
	Vector origin;
	QAngle angles;

	if (!FX_GetAttachmentTransform(data.m_hEntity, data.m_nAttachmentIndex, &origin, &angles))
		return;

	// request new particle for this?
	DispatchParticleEffect("hd_7.62mm_muzzleflash", origin, angles, NULL);
}

DECLARE_CLIENT_EFFECT("ApacheMuzzleFlash", ApacheMuzzleFlashCallback);

void DynamicLightCallback(const CEffectData& data)
{
	Vector origin;
	origin = data.m_vOrigin;

	dlight_t* dl = effects->CL_AllocDlight( LIGHT_INDEX_TE_DYNAMIC + data.entindex());
	dl->origin = origin;
	dl->color.r = 255;
	dl->color.g = 0;
	dl->color.b = 0;
	dl->radius = random->RandomFloat(245.0f, 256.0f);
	dl->decay = 512.0f;
	dl->die = gpGlobals->curtime + 0.1f;
}

DECLARE_CLIENT_EFFECT("GrenadeDynamicLight0.3s", DynamicLightCallback);

void DynamicLightCallback1(const CEffectData& data)
{
	Vector origin;
	origin = data.m_vOrigin;

	dlight_t* dl = effects->CL_AllocDlight(LIGHT_INDEX_TE_DYNAMIC + data.entindex());
	dl->origin = origin;
	dl->color.r = 255;
	dl->color.g = 0;
	dl->color.b = 0;
	dl->radius = 200;
	dl->die = gpGlobals->curtime + 1.0f;
}

DECLARE_CLIENT_EFFECT("GrenadeDynamicLight1s", DynamicLightCallback1);