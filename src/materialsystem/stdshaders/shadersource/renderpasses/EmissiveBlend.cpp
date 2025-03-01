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
#include "../../ACROHS_Defines.h"

// All Shaders use the same struct and sampler definitions. This way multipass CSM doesn't need to re-setup the struct
#include "../../ACROHS_Shared.h"

// We need all of these
#include "EmissiveBlend.h"
#include "../CommonFunctions.h"
#include "../../cpp_shader_constant_register_map.h"

// Includes for Shaderfiles...

// This is in EmissiveBlend.h
// void LuxEmissiveBlend_Link_Params(EmissiveBlend_Vars_t &info)

void LuxDistanceAlpha_Init_Params(CBaseVSShader *pShader, IMaterialVar **params, const char *pMaterialName, EmissiveBlend_Vars_t &info)
{

}

void LuxDistanceAlpha_Shader_Init(CBaseVSShader *pShader, IMaterialVar **params, EmissiveBlend_Vars_t &info)
{

}

void LuxDistanceAlpha_Shader_Draw(CBaseVSShader *pShader, IMaterialVar **params, IShaderDynamicAPI *pShaderAPI, IShaderShadow *pShaderShadow, EmissiveBlend_Vars_t &info, VertexCompressionType_t vertexCompression, CBasePerMaterialContextData **pContextDataPtr)
{

}