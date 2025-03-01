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
//	Last Change :	02.03.2023 DMY
//
//	Purpose of this File :	LUX_Lightmappedgeneric Shader for brushes.
//
//===========================================================================//

// File in which you can determine what to use.
// We do not use preprocessor definitions for this because this is honestly faster and easier to set up.
#include "../../ACROHS_Defines.h"

// All Shaders use the same struct and sampler definitions. This way multipass CSM doesn't need to re-setup the struct
#include "../../ACROHS_Shared.h"

// We need all of these
#include "../CommonFunctions.h"
#include "../../cpp_shader_constant_register_map.h"

// this is required for our Function Array's
#include <functional>

// Includes for Shaderfiles...
#include "lux_model_vs30.inc"
#include "lux_brush_vs30.inc"
#include "lux_unlitgeneric_ps30.inc"

extern ConVar mat_oldshaders;
extern ConVar mat_specular;

#define LuxUnlitGeneric_Params() void LuxUnlitGeneric_Link_Params(Unlit_Vars_t &info)\
{																					  \
	Link_EnvMapFresnel()															   \
	Link_MiscParameters()																\
	Link_GlobalParameters()																 \
	Link_SeamlessParameters()															  \
	Link_EnvMapMaskParameters()															   \
	Link_DetailTextureParameters()															\
	Link_EnvironmentMapParameters()															 \
	Link_ParallaxCorrectionParameters()														  \
	info.m_nMultipurpose6	=	RECEIVEFLASHLIGHT;											   \
	info.m_nMultipurpose7	=	MATERIALNAME;													\
	info.m_nColor			=	COLOR;															 \
}//-----------------------------------------------------------------------------------------------\

void LuxUnlitGeneric_Init_Params(CBaseVSShader *pShader, IMaterialVar **params, const char *pMaterialName, Unlit_Vars_t &info)
{
	// I wanted to debug print some values and needed to know which material they belong to. However pMaterialName is not available to the draw!
	// Well, now it is...
	params[info.m_nMultipurpose7]->SetStringValue(pMaterialName);

	// See ACROHS_Defines.h for more information.
	if (GetIntParamValue(info.m_nDistanceAlpha) != 0)
	{
		SET_FLAGS2(MATERIAL_VAR2_DISTANCEALPHA);
	}

	//	I couldn't put this into the Link_Params because it would complain that "Params" is undefined.
	//	It should work here though.
	info.m_nColor = IS_FLAG_SET(MATERIAL_VAR_MODEL) ? COLOR2 : COLOR;

	// The usual...
	Flashlight_BorderColorSupportCheck();

	// Default Value is supposed to be 1.0f
	FloatParameterDefault(info.m_nDetailBlendFactor, 1.0f)

	// Default Value is supposed to be 4.0f
	FloatParameterDefault(info.m_nDetailScale, 4.0f)

	// Default Value is supposed to be 1.0f
	FloatParameterDefault(info.m_nEnvMapSaturation, 1.0f)

	// If in decal mode, no debug override...
	if (IS_FLAG_SET(MATERIAL_VAR_DECAL))
	{
		SET_FLAGS(MATERIAL_VAR_NO_DEBUG_OVERRIDE);
	}

	// No BaseTexture ? None of that.
	if (!params[info.m_nBaseTexture]->IsDefined())
	{
		CLEAR_FLAGS(MATERIAL_VAR_BASEALPHAENVMAPMASK);
	}

	// If mat_specular 0, then get rid of envmap
	if (!g_pConfig->UseSpecular() && params[info.m_nEnvMap]->IsDefined())
	{
		params[info.m_nEnvMap]->SetUndefined();
	}

	// Seamless 
	if (GetFloatParamValue(info.m_nSeamless_Scale) != 0.0f)
	{
		// if we don't have DetailScale we want Seamless_Scale
		if (GetFloatParamValue(info.m_nSeamless_DetailScale) == 0.0f)
		{
			SetFloatParamValue(info.m_nSeamless_DetailScale, GetFloatParamValue(info.m_nSeamless_Scale))
		}
		// Using Seamless_Scale will enable Seamless_Base
		// IMPORTANT: **Not on ULG**. Stock behaviour demands you use $seamless_base!
		// SetIntParamValue(info.m_nSeamless_Base, 1)
	}
}

void LuxUnlitGeneric_Shader_Init(CBaseVSShader *pShader, IMaterialVar **params, Unlit_Vars_t &info)
{
	// Only when $receiveflashlight
	if (GetIntParamValue(info.m_nMultipurpose6) != 0)
	{
		pShader->LoadTexture(info.m_nFlashlightTexture, TEXTUREFLAGS_SRGB);
		SET_FLAGS2(MATERIAL_VAR2_SUPPORTS_FLASHLIGHT);				// Yes, Yes we support
		SET_FLAGS2(MATERIAL_VAR2_USE_FLASHLIGHT);					// Yes, Yes we use it...?
	}

	LoadTextureWithCheck(info.m_nBaseTexture, TEXTUREFLAGS_SRGB)

	// I can't make sense of whatever they tried to do on the Stock Shaders
	// They load Detail as sRGB but then don't do sRGB Read... and vice versa.
	// We will just check if this  Texture has the flag and do sRGB read based on that
	LoadTextureWithCheck(info.m_nDetailTexture, 0)

#ifdef CUBEMAPS
	// ShiroDkxtro2 : Valve loads the cubemap as sRGB based on whether or not you are on HDR_TYPE_NONE
	// This makes no sense! You can use custom sRGB Cubemaps or have a fallback to LDR.
	// I don't support the continious usage of LDR, however it does not hurt to check at all
	LoadEnvMap(info.m_nEnvMap, 0)

#ifdef MAPBASE_FEATURED
	if (mat_specular_disable_on_missing.GetBool())
	{
		// Revert to defaultcubemap when the envmap texture is missing
		// (should be equivalent to toolsblack in Mapbase)
		if (params[info.m_nEnvMap]->GetTextureValue()->IsError())
		{
			params[info.m_nEnvMap]->SetStringValue("engine/defaultcubemap");
			pShader->LoadCubeMap(info.m_nEnvMap, 0);
		}
	}
#endif
	// This big block of if-statements is to determine if we even have any envmapmasking.
	// We don't want EnvMapMasking if we don't even have an envmap
	if (!params[info.m_nEnvMap]->GetTextureValue()->IsError())
	{
		if (params[info.m_nEnvMapMask]->IsDefined())
		{
			pShader->LoadTexture(info.m_nEnvMapMask, 0);

			// We already have an envmapmask now, so discard the others!
			CLEAR_FLAGS(MATERIAL_VAR_BASEALPHAENVMAPMASK);
		}
		else
		{
				if (params[info.m_nBaseTexture]->GetTextureValue()->IsError())
					CLEAR_FLAGS(MATERIAL_VAR_BASEALPHAENVMAPMASK); // If we have no Basetexture, can't use its alpha.
		}
	}
	else
#endif // #ifdef CUBEMAPS #endif
	{ // No EnvMap == No Masking.
		CLEAR_FLAGS(MATERIAL_VAR_BASEALPHAENVMAPMASK);
	}

	// Shouldn't we check for $Alphatest too and then disable $BaseAlphaEnvMapMask and $BlendTintByBaseAlpha
	// Also check for $BlendTintByBaseAlpha and then disable $Alphatest and $BaseAlphaEnvMapMask...?
	if (IS_FLAG_SET(MATERIAL_VAR_BASEALPHAENVMAPMASK))
	{
		CLEAR_FLAGS(MATERIAL_VAR_ALPHATEST);
		SetIntParamValue(info.m_nBlendTintByBaseAlpha, 0)
	}
}

void LuxUnlitGeneric_Shader_Draw(CBaseVSShader *pShader, IMaterialVar **params, IShaderDynamicAPI *pShaderAPI, IShaderShadow *pShaderShadow, Unlit_Vars_t &info, VertexCompressionType_t vertexCompression, CBasePerMaterialContextData **pContextDataPtr)
{
	bool bIsModel					=						IS_FLAG_SET(MATERIAL_VAR_MODEL);
	bool bHasFlashlight				=						pShader->UsingFlashlight(params);
	// Only use Flashlight with $receiveflashlight
	bool bUseFlashlight				=	bHasFlashlight	&&	GetIntParamValue(info.m_nMultipurpose6) != 0;
	bool bAlphatest					=						IS_FLAG_SET(MATERIAL_VAR_ALPHATEST);
//	bool bIsDecal					=						IS_FLAG_SET(MATERIAL_VAR_DECAL);
	bool bBlendTintByBaseAlpha		=						GetIntParamValue(info.m_nBlendTintByBaseAlpha) != 0;
	bool bBaseAlphaEnvMapMask		=	!bUseFlashlight	&&	IS_FLAG_SET(MATERIAL_VAR_BASEALPHAENVMAPMASK); // No Envmapping under the flashlight

	// Textures
	bool bHasBaseTexture			=						IsTextureLoaded(info.m_nBaseTexture);
	//==================================================================================================
#ifdef DETAILTEXTURING
	bool	bHasDetailTexture		=						IsTextureLoaded(info.m_nDetailTexture);
#else
	bool	bHasDetailTexture		=	false;
#endif
	//==================================================================================================
#ifdef CUBEMAPS
	bool	bHasEnvMap				=	!bUseFlashlight	&&	IsTextureLoaded(info.m_nEnvMap); // No Envmapping under the flashlight
	bool	bHasEnvMapMask			=	!bUseFlashlight	&&	IsTextureLoaded(info.m_nEnvMapMask); // No Envmapping under the flashlight
	bool	bHasEnvMapFresnel		=	bHasEnvMap		&&	GetIntParamValue(info.m_nEnvMapFresnel) != 0;
#ifdef PARALLAXCORRECTEDCUBEMAPS
	bool	bPCC					=	!bIsModel		&&	GetIntParamValue(info.m_nEnvMapParallax) != 0; // Only available on Brushes.
#else
	bool	bPCC					=	false;
#endif
#else
	bool	bHasEnvMap				=	false;
	bool	bHasEnvMapMask			=	false;
	bool	bPCC					=	false;
	bool	bHasEnvMapFresnel		=	false;
#endif

	bool bHasVertexRGBA = IS_FLAG_SET(MATERIAL_VAR_VERTEXCOLOR) || IS_FLAG_SET(MATERIAL_VAR_VERTEXALPHA);

	bool bDetailTextureTransform = false;
	bool bEnvMapMaskTransform = false;
	// Seamless Data
	bool bHasSeamlessBase		= (GetIntParamValue(info.m_nSeamless_Base) != 0		 ) && !bHasVertexRGBA;
	bool bHasSeamlessDetail		= (GetIntParamValue(info.m_nSeamless_Detail) != 0	 ) && !bHasVertexRGBA;
	bool bHasSeamlessBump		= (GetIntParamValue(info.m_nSeamless_Bump) != 0		 ) && !bHasVertexRGBA;
	bool bHasSeamlessEnvMapMask	= (GetIntParamValue(info.m_nSeamless_EnvMapMask) != 0) && !bHasVertexRGBA;
	// we need to tell the VertexShader if we want any Seamless Data at all
	bool bUsesSeamlessData = bHasSeamlessBase || bHasSeamlessDetail || bHasSeamlessBump || bHasSeamlessEnvMapMask;
	int	 iModeDetail = 0;
	int	 iModeEnvMapMask = 0;

	// When we don't have these, we don't compute them.
	if (bHasDetailTexture)
	{
		bDetailTextureTransform = !params[info.m_nDetailTextureTransform]->MatrixIsIdentity();//  IsParamDefined(info.m_nDetailTextureTransform);
		iModeDetail = CheckSeamless(bHasSeamlessDetail);
	}

	if (bHasEnvMapMask)
	{
		bEnvMapMaskTransform = !params[info.m_nEnvMapMaskTransform]->MatrixIsIdentity();//  IsParamDefined(info.m_nEnvMapMaskTransform);
		iModeEnvMapMask = CheckSeamless(bHasSeamlessEnvMapMask);
	}

	int iEnvMapMode			=	ComputeEnvMapMode(bHasEnvMap, bHasEnvMapMask, bBaseAlphaEnvMapMask, false); // false is $NormalMapAlphaEnvMapMask, which well... we don't have bumpmapping on this shader!
	int iDetailBlendMode	=	GetIntParamValue(info.m_nDetailBlendMode);
#ifdef TREESWAYING
	int iTreeSway = 0;
#else
	int iTreeSway = 0;
#endif

	bool IsOpaque(bIsFullyOpaque, info.m_nBaseTexture, bAlphatest);

	//===========================================================================//
	// Snapshotting State
	// This gets setup basically once, or again when using the flashlight
	// Once this is done we use dynamic state.
	//===========================================================================//
	if (pShader->IsSnapshotting())
	{
		//===========================================================================//
		// Alphatesting, VertexShaderFormat, sRGBWrite...
		//===========================================================================//
		if (bAlphatest)
		{
			pShaderShadow->EnableAlphaTest(true);
			pShaderShadow->AlphaFunc(SHADER_ALPHAFUNC_GEQUAL, params[info.m_nAlphaTestReference]->GetFloatValue()); // This will by default be 0
		}
		pShaderShadow->EnableAlphaWrites(bIsFullyOpaque);

		if (bIsModel)
		{
			GetVertexShaderFormat_Model()
		}
		else
		{
			GetVertexShaderFormat_Brush()
		}
		
		pShaderShadow->EnableSRGBWrite(true); // We always do sRGB Writes. We use ps30.

		//===========================================================================//
		// Enable Samplers
		//===========================================================================//
		EnableSampler(SAMPLER_BASETEXTURE, true) // We always have a basetexture, and yes they should always be sRGB

#ifdef DETAILTEXTURING
		// ShiroDkxtro2: Stock Shaders do some cursed Ternary stuff with the detailblendmode
		// Which... If I interpret it correctly makes it read non-sRGB maps as sRGB and vice versa...
		if (bHasDetailTexture)
		{
			ITexture *pDetailTexture = params[info.m_nDetailTexture]->GetTextureValue();
			EnableSampler(SAMPLER_DETAILTEXTURE, (pDetailTexture->GetFlags() & TEXTUREFLAGS_SRGB) ? true : false)
		}
#endif

#ifdef CUBEMAPS
		// ShiroDkxtro2: Stock Shaders will enable sRGB based on HDR_TYPE_NONE
		// Yes. sRGB cubemaps would be read incorrectly on HDR.
		if (bHasEnvMap)
		{
			ITexture *pEnvMap = params[info.m_nEnvMap]->GetTextureValue();
			EnableSampler(SAMPLER_ENVMAPTEXTURE, (pEnvMap->GetFlags() & TEXTUREFLAGS_SRGB) ? true : false)
				EnableSamplerWithCheck(bHasEnvMapMask, SAMPLER_ENVMAPMASK, true) // Yes, Yes we want sRGB Read
		}
#endif

		EnableFlashlightSamplers(bUseFlashlight, SAMPLER_SHADOWDEPTH, SAMPLER_RANDOMROTATION, SAMPLER_FLASHLIGHTCOOKIE, info.m_nBaseTexture)

		//===========================================================================//
		// Declare Static Shaders, manually.
		//===========================================================================//
		if (bIsModel)
		{
			DECLARE_STATIC_VERTEX_SHADER(lux_model_vs30);
			SET_STATIC_VERTEX_SHADER_COMBO(DECAL, IS_FLAG_SET(MATERIAL_VAR_DECAL));
			SET_STATIC_VERTEX_SHADER_COMBO(HALFLAMBERT, 0); // No Lighting.
			SET_STATIC_VERTEX_SHADER_COMBO(LIGHTMAP_UV, 0); // No Lighting.
			SET_STATIC_VERTEX_SHADER_COMBO(TREESWAY, iTreeSway);
			SET_STATIC_VERTEX_SHADER_COMBO(SEAMLESS_BASE, bHasSeamlessBase);
			SET_STATIC_VERTEX_SHADER_COMBO(DETAILTEXTURE_UV, iModeDetail);
			SET_STATIC_VERTEX_SHADER_COMBO(NORMALTEXTURE_UV, 0); // No Lighting.
			SET_STATIC_VERTEX_SHADER_COMBO(ENVMAPMASK_UV, iModeEnvMapMask);
			SET_STATIC_VERTEX_SHADER(lux_model_vs30);
		}
		else // Gotta be a brush...
		{
			// This will most definitely be cheaper to render.
			DECLARE_STATIC_VERTEX_SHADER(lux_brush_vs30);
			SET_STATIC_VERTEX_SHADER_COMBO(VERTEX_RGBA, bHasVertexRGBA); // This does not work with the below.
			SET_STATIC_VERTEX_SHADER_COMBO(SEAMLESS_BASE, bHasSeamlessBase);
			SET_STATIC_VERTEX_SHADER_COMBO(DETAILTEXTURE_UV, iModeDetail);
			SET_STATIC_VERTEX_SHADER_COMBO(NORMALTEXTURE_UV, 0); // No Lighting.
			SET_STATIC_VERTEX_SHADER_COMBO(ENVMAPMASK_UV, iModeEnvMapMask);
			SET_STATIC_VERTEX_SHADER(lux_brush_vs30);
		}

		DECLARE_STATIC_PIXEL_SHADER(lux_unlitgeneric_ps30);
		SET_STATIC_PIXEL_SHADER_COMBO(FLASHLIGHT, bHasFlashlight);
		SET_STATIC_PIXEL_SHADER_COMBO(DETAILTEXTURE, bHasDetailTexture);
		SET_STATIC_PIXEL_SHADER_COMBO(ENVMAPMODE, iEnvMapMode);
		SET_STATIC_PIXEL_SHADER_COMBO(ENVMAPFRESNEL, bHasEnvMapFresnel);
		SET_STATIC_PIXEL_SHADER_COMBO(PCC, bPCC);
		SET_STATIC_PIXEL_SHADER_COMBO(BLENDTINTBYBASEALPHA, bBlendTintByBaseAlpha);
		SET_STATIC_PIXEL_SHADER_COMBO(DESATURATEBYBASEALPHA, 0);
		SET_STATIC_PIXEL_SHADER(lux_unlitgeneric_ps30);

	}
	else // End of Snapshotting ------------------------------------------------------------------------------------------------------------------------------------------------------------------
	{
		//===========================================================================//
		// Bind Textures
		//===========================================================================//
		BindTextureWithCheckAndFallback(bHasBaseTexture, SAMPLER_BASETEXTURE, info.m_nBaseTexture, info.m_nBaseTextureFrame, TEXTURE_WHITE)

#ifdef DETAILTEXTURING
		BindTextureWithCheck(bHasDetailTexture, SAMPLER_DETAILTEXTURE, info.m_nDetailTexture, info.m_nDetailFrame)
#endif

#ifdef CUBEMAPS
		if (bHasEnvMap)
		{
			BindTextureWithCheckAndFallback(mat_specular.GetBool(), SAMPLER_ENVMAPTEXTURE, info.m_nEnvMap, info.m_nEnvMapFrame, TEXTURE_BLACK)
			BindTextureWithCheck(bHasEnvMapMask, SAMPLER_ENVMAPMASK, info.m_nEnvMapMask, info.m_nEnvMapMaskFrame)
		}
#endif
		// This does a lot of things for us!!
		// Just don't touch constants <32 and you should be fine c:
		bool bFlashlightShadows = false;
		SetupStockConstantRegisters(bUseFlashlight, SAMPLER_FLASHLIGHTCOOKIE, SAMPLER_RANDOMROTATION, info.m_nFlashlightTexture, info.m_nFlashlightTextureFrame, bFlashlightShadows)

		int iFogIndex = 0;
		bool bWriteDepthToAlpha = false;
		bool bWriteWaterFogToAlpha = false;
		SetupDynamicComboVariables(iFogIndex, bWriteDepthToAlpha, bWriteWaterFogToAlpha, bIsFullyOpaque)

		//===========================================================================//
		// Prepare floats for Constant Registers
		//===========================================================================//
		// f4Empty is just float4 Name = {1,1,1,1};
		// Yes I have some excessive problems with macro overusage...
		f4Empty(f4BaseTextureTint_Factor)
		f4Empty(f4DetailTint_BlendFactor)
		f4Empty(f4EnvMapTint_LightScale)
		f4Empty(f4EnvMapSaturation_Contrast)
		f4Empty(f4EnvMapFresnelRanges_)
		f4Empty(f4DetailBlendMode_) // yzw empty
		BOOL BBools[16] = { false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false };

		// $color/$color2 and $BlendTintColorOverBase
		GetVec3ParamValue(info.m_nColor, f4BaseTextureTint_Factor);
		f4BaseTextureTint_Factor[3] = GetFloatParamValue(info.m_nBlendTintColorOverBase);


#ifdef DETAILTEXTURING
		if (bHasDetailTexture)
		{
			// $DetailTint and $DetailBlendFactor
			GetVec3ParamValue(info.m_nDetailTint, f4DetailTint_BlendFactor);
			f4DetailTint_BlendFactor[3] = GetFloatParamValue(info.m_nDetailBlendFactor); // Consider writing a macro for this
			f4DetailBlendMode_[0] = iDetailBlendMode; // We trunc it later and do some dynamic branching. Saves a ton of combos but makes rendering slower. but oh well...
		}
#endif

#ifdef CUBEMAPS
		if (bHasEnvMap)
		{
			GetVec3ParamValue(info.m_nEnvMapTint, f4EnvMapTint_LightScale);
			f4EnvMapTint_LightScale[3] = GetFloatParamValue(info.m_nEnvMapLightScale); // We always need the LightScale.

			GetVec3ParamValue(info.m_nEnvMapSaturation, f4EnvMapSaturation_Contrast); // Yes. Yes this is a vec3 parameter.
			f4EnvMapSaturation_Contrast[3] = GetFloatParamValue(info.m_nEnvMapContrast);

			if (bHasEnvMapFresnel)
			{
				GetVec3ParamValue(info.m_nEnvMapFresnelMinMaxExp, f4EnvMapFresnelRanges_);
			}

#ifdef PARALLAXCORRECTEDCUBEMAPS
			SetUpPCC(bPCC, info.m_nEnvMapOrigin, info.m_nEnvMapParallaxOBB1, info.m_nEnvMapParallaxOBB2, info.m_nEnvMapParallaxOBB3, 39, 38)
#endif
		}
#endif

		//==================================================================================================
		// Set Pixelshader Constant Registers (PSREG's)
		//==================================================================================================
		pShaderAPI->SetPixelShaderConstant(	32, f4BaseTextureTint_Factor, 1);
		pShaderAPI->SetPixelShaderConstant(	33, f4DetailTint_BlendFactor, 1);
		//									34, f4SelfIllumTint_Scale
		pShaderAPI->SetPixelShaderConstant(	35, f4EnvMapTint_LightScale, 1);
		pShaderAPI->SetPixelShaderConstant(	36, f4EnvMapFresnelRanges_, 1);
		pShaderAPI->SetPixelShaderConstant(	37, f4DetailBlendMode_, 1);
		//									38, PCC Bounding Box\
		//									39, PCC Bounding Box \/ setup
		//									40, PCC Bounding Box /\ above
		//									41, PCC Bounding Box/
		//									42, f4PhongTint_Boost
		//									43, f4PhongFresnelRanges_Exponent
		//									44, f4RimLightControls		
		//									45, f4SelfIllumFresnelRanges
		pShaderAPI->SetPixelShaderConstant(	46, f4EnvMapSaturation_Contrast, 1);

		//==================================================================================================
		// Set Vertexshader Constant Registers (VSREG's...?) Also some other stuff
		//==================================================================================================
		// Always having this
		pShader->SetVertexShaderTextureTransform(VERTEX_SHADER_SHADER_SPECIFIC_CONST_13, info.m_nBaseTextureTransform);

		if (bDetailTextureTransform)
			pShader->SetVertexShaderTextureScaledTransform(VERTEX_SHADER_SHADER_SPECIFIC_CONST_17, info.m_nDetailTextureTransform, info.m_nDetailScale);
		else
			pShader->SetVertexShaderTextureScaledTransform(VERTEX_SHADER_SHADER_SPECIFIC_CONST_17, info.m_nBaseTextureTransform, info.m_nDetailScale);

		if (bEnvMapMaskTransform)
			pShader->SetVertexShaderTextureTransform(VERTEX_SHADER_SHADER_SPECIFIC_CONST_19, info.m_nEnvMapMaskTransform);
		else
			pShader->SetVertexShaderTextureTransform(VERTEX_SHADER_SHADER_SPECIFIC_CONST_19, info.m_nBaseTextureTransform);

		f4Empty(SeamlessScales)
			if (bUsesSeamlessData) // Skip all of this if not used.
			{
				if (bHasSeamlessBase)
				{
					// All textures must use these coordinates or they will end up scuffed.
					// The way this works is that the Vertex Shader will output the weights and UV of the Basetexture for all textures with no regards to transforms
					// They can't get screwed up but you also can't transform them unless you also enable Seamless Scale for those Textures.
					// This might have undesired consequences on Custom Materials that might have abused the previous behaviour on purpose.
					SeamlessScales[0] = GetFloatParamValue(info.m_nSeamless_Scale);
					BBools[3] = true; // bHasSeamless_Base
					BBools[4] = true; // bHasSeamless_Detail
					BBools[5] = true; // bHasSeamless_Bump
					BBools[6] = true; // bHasSeamless_EnvMapMask
				}

				if (bHasSeamlessDetail)
				{
					BBools[4] = true; // bHasSeamless_Detail
					SeamlessScales[1] = GetFloatParamValue(info.m_nSeamless_DetailScale);
				}

				if (bHasSeamlessBump)
				{
					BBools[5] = true; // bHasSeamless_Bump
					SeamlessScales[2] = GetFloatParamValue(info.m_nSeamless_BumpScale);
				}

				if (bHasSeamlessEnvMapMask)
				{
					BBools[6] = true; // bHasSeamless_EnvMapMask
					SeamlessScales[3] = GetFloatParamValue(info.m_nSeamless_EnvMapMaskScale);
				}
				// I tried shader specific ones. 0-3 and those didn't work. So we are going to use our new registers instead.
				pShaderAPI->SetVertexShaderConstant(VERTEX_SHADER_SHADER_SPECIFIC_CONST_30, SeamlessScales);
			}

		// Always! Required for Lightwarp
		// BBools[1] = bHasLightWarpTexture;
		pShaderAPI->SetBooleanPixelShaderConstant(0, BBools, 16, true);

		if (bIsModel)
		{
			DECLARE_DYNAMIC_VERTEX_SHADER(lux_model_vs30);
			SET_DYNAMIC_VERTEX_SHADER_COMBO(MORPHING, g_pHardwareConfig->HasFastVertexTextures() && pShaderAPI->IsHWMorphingEnabled());
			SET_DYNAMIC_VERTEX_SHADER_COMBO(SKINNING, pShaderAPI->GetCurrentNumBones() > 0 ? true : false);
			SET_DYNAMIC_VERTEX_SHADER_COMBO(STATICPROPLIGHTING, 0);
			SET_DYNAMIC_VERTEX_SHADER_COMBO(DYNAMICPROPLIGHTING, 0);
			SET_DYNAMIC_VERTEX_SHADER(lux_model_vs30);
		}
		else // Gotta be a brush...
		{
			DECLARE_DYNAMIC_VERTEX_SHADER(lux_brush_vs30);
//			SET_DYNAMIC_VERTEX_SHADER_COMBO(DOWATERFOG, pShaderAPI->GetPixelFogCombo());
			SET_DYNAMIC_VERTEX_SHADER(lux_brush_vs30);
		}

		// Setting up DYNAMIC pixel shader
		DECLARE_DYNAMIC_PIXEL_SHADER(lux_unlitgeneric_ps30);
		SET_DYNAMIC_PIXEL_SHADER_COMBO(FLASHLIGHTSHADOWS, bFlashlightShadows);
		SET_DYNAMIC_PIXEL_SHADER_COMBO(PIXELFOGTYPE, pShaderAPI->GetPixelFogCombo());
		SET_DYNAMIC_PIXEL_SHADER_COMBO(WRITE_DEPTH_TO_DESTALPHA, bWriteDepthToAlpha);
		SET_DYNAMIC_PIXEL_SHADER_COMBO(WRITEWATERFOGTODESTALPHA, bWriteWaterFogToAlpha);
		SET_DYNAMIC_PIXEL_SHADER(lux_unlitgeneric_ps30);

	}

	// ShiroDkxtro2:	I had it happen a bunch of times...
	//		pShader->Draw(); MUST BE above the final }
	//		It is very easy to mess this up ( done so several times )
	//		Game will just crash and debug leads you to a bogus function.
	pShader->Draw();
}

// Lux shaders will replace whatever already exists.
DEFINE_FALLBACK_SHADER(SDK_UnlitGeneric, LUX_UnlitGeneric)
DEFINE_FALLBACK_SHADER(SDK_UnlitGeneric_DX8, LUX_UnlitGeneric)
DEFINE_FALLBACK_SHADER(SDK_UnlitGeneric_DX6, LUX_UnlitGeneric)

DEFINE_FALLBACK_SHADER(SDK_UnlitTwoTexture, LUX_UnlitTwoTexture)
DEFINE_FALLBACK_SHADER(SDK_UnlitTwoTexture_DX9, LUX_UnlitTwoTexture)

// ShiroDkxtro2 : Brainchild of Totterynine.
// Declare param declarations separately then call that from within shader declaration.
// This makes it possible to easily run multiple shaders in one file
BEGIN_VS_SHADER(LUX_UnlitGeneric, "ShiroDkxtro2's ACROHS Shader-Rewrite Shader")

BEGIN_SHADER_PARAMS

Declare_MiscParameters()
Declare_SeamlessParameters()
Declare_DetailTextureParameters()
Declare_EnvironmentMapParameters()
Declare_EnvMapMaskParameters()
Declare_ParallaxCorrectionParameters()
SHADER_PARAM(MATERIALNAME, SHADER_PARAM_TYPE_STRING, "", "") // Can't use pMaterialName on Draw so we put it on this parameter instead...
SHADER_PARAM(DISTANCEALPHA, SHADER_PARAM_TYPE_BOOL, "", "")
SHADER_PARAM(RECEIVEFLASHLIGHT, SHADER_PARAM_TYPE_BOOL, "0", "Forces this material to receive flashlights.")
END_SHADER_PARAMS

LuxUnlitGeneric_Params()

SHADER_INIT_PARAMS()
{
	Unlit_Vars_t vars;
	LuxUnlitGeneric_Link_Params(vars);
	LuxUnlitGeneric_Init_Params(this, params, pMaterialName, vars);
}

SHADER_FALLBACK
{
	if (mat_oldshaders.GetBool())
	{
		return "UnlitGeneric";
	}

	// We also set the SelfIllum Flag to tell the shader its unlit.
	if (IS_FLAG2_SET(MATERIAL_VAR2_DISTANCEALPHA))
	{
		SET_FLAGS2(MATERIAL_VAR2_LIGHTING_UNLIT); // TODO: Is this even usable?
		return "LUX_DistanceAlpha";
	}

	if (g_pHardwareConfig->GetDXSupportLevel() < 90)
	{
		Warning("Game run at DXLevel < 90 \n");
		return "Wireframe";
	}
	return 0;
}

SHADER_INIT
{
	Unlit_Vars_t vars;
	LuxUnlitGeneric_Link_Params(vars);
	LuxUnlitGeneric_Shader_Init(this, params, vars);
}

SHADER_DRAW
{
	Unlit_Vars_t vars;
	LuxUnlitGeneric_Link_Params(vars);
	LuxUnlitGeneric_Shader_Draw(this, params, pShaderAPI, pShaderShadow, vars, vertexCompression, pContextDataPtr);

#ifdef ACROHS_CSM
	// Function Here
#endif

}
END_SHADER