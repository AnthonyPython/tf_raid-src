//========= ShiroDkxtro2's --ACROHS Ultimate Shaders Project-- ============//
//
//	Description :	What is this?
//					This is a total rewrite of the SDK Shaders.
//					Under the prefix of lux_ shaders.
//					The goal is to make the most possible combinations work,
//					add new parameters and functionality and...
//					Most importantly.-Decrease Compile times. A lot.
//					For ACROHS specifically, we also add CSM support.
//					This nukes ps20b and below. No Linux support.
//
//	Initial D.	:	20.01.2023 DMY
//	Last Change :	
//
//	Purpose of this File :	
//
//===========================================================================//

// File in which you can determine what to use.
// We do not use preprocessor definitions for this because this is honestly faster and easier to set up.
#include "../ACROHS_Defines.h"

// All Shaders use the same struct and sampler definitions. This way multipass CSM doesn't need to re-setup the struct
// Also this specific file won't have to redeclare the struct if it needs it...
#include "../ACROHS_Shared.h"

// Get the pre-declaration of the Functions we are supposed to write here...
#include "CommonFunctions.h"

// Compute base on
int ComputeEnvMapMode(bool bEnvMap, bool bEnvMapMask, bool bBaseAlphaEnvMapMask, bool bNormalMapAlphaEnvMapMask)
{
	int EnvMapMode = bEnvMap;
	
	if (bEnvMapMask)
		return EnvMapMode = 2;

	if (bBaseAlphaEnvMapMask)
		return EnvMapMode = 3;

	if (bNormalMapAlphaEnvMapMask)
		return EnvMapMode = 4;

	return EnvMapMode; // 0 or 1 case
}

// In case we have a shader without Bumpmaps
int ComputeEnvMapMode(bool bEnvMap, bool bEnvMapMask, bool bBaseAlphaEnvMapMask)
{
	int EnvMapMode = bEnvMap;

	if (bEnvMapMask)
		return EnvMapMode = 2;

	if (bBaseAlphaEnvMapMask)
		return EnvMapMode = 3;

	return EnvMapMode; // 0 or 1 case
}

// Returns 2 if seamless, 1 if not seamless.
int	CheckSeamless(bool bIsSeamless)
{
	return bIsSeamless ? 2 : 1;
}

//===========================================================================//
// Check header for details.
//===========================================================================//
bool bStaticLightVertex(LightState_t LightState)
{
	uintptr_t LightStateAddress = reinterpret_cast<uintptr_t>(&LightState.m_bAmbientLight);
	uintptr_t StaticLightAddress = LightStateAddress + sizeof(bool);

	return *reinterpret_cast<bool*>(StaticLightAddress);
}


//-----------------------------------------------------------------------------
// Sets up hw morphing state for the vertex shader
//-----------------------------------------------------------------------------
void SetHWMorphVertexShaderState(IShaderDynamicAPI *pShaderAPI, int nDimConst, int nSubrectConst, VertexTextureSampler_t morphSampler)
{
	if (!pShaderAPI->IsHWMorphingEnabled())
		return;

	int nMorphWidth, nMorphHeight;
	pShaderAPI->GetStandardTextureDimensions(&nMorphWidth, &nMorphHeight, TEXTURE_MORPH_ACCUMULATOR);

	int nDim = pShaderAPI->GetIntRenderingParameter(INT_RENDERPARM_MORPH_ACCUMULATOR_4TUPLE_COUNT);
	float pMorphAccumSize[4] = { nMorphWidth, nMorphHeight, nDim, 0.0f };
	pShaderAPI->SetVertexShaderConstant(nDimConst, pMorphAccumSize);

	int nXOffset = pShaderAPI->GetIntRenderingParameter(INT_RENDERPARM_MORPH_ACCUMULATOR_X_OFFSET);
	int nYOffset = pShaderAPI->GetIntRenderingParameter(INT_RENDERPARM_MORPH_ACCUMULATOR_Y_OFFSET);
	int nWidth = pShaderAPI->GetIntRenderingParameter(INT_RENDERPARM_MORPH_ACCUMULATOR_SUBRECT_WIDTH);
	int nHeight = pShaderAPI->GetIntRenderingParameter(INT_RENDERPARM_MORPH_ACCUMULATOR_SUBRECT_HEIGHT);
	float pMorphAccumSubrect[4] = { nXOffset, nYOffset, nWidth, nHeight };
	pShaderAPI->SetVertexShaderConstant(nSubrectConst, pMorphAccumSubrect);

	pShaderAPI->BindStandardVertexTexture(morphSampler, TEXTURE_MORPH_ACCUMULATOR);
}