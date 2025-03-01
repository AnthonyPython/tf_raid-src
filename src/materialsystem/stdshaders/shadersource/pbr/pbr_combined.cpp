//========= ShiroDkxtro2's --ACROHS Ultimate Shaders Project-- ============//
//
//	Description :	What is this?
//					PBR. Look it up!
//
//	Initial D.	:	25.09.2022 DMY
//	Last Change :	02.04.2023 DMY
//
//	Purpose of this File :	PBR for Brushes, Models and displacements.
//
//===========================================================================//

// WRD : Definitions were moved to ACROHS_Defines.h

// File in which you can determine what to use.
// We do not use preprocessor definitions for this because this is honestly faster and easier to set up.
#include "../../ACROHS_Defines.h"

#ifdef PHYSICALLY_BASED_RENDERING
// Include for struct
#include "pbr_combined.h"

// We need all of these
#include "../CommonFunctions.h"
#include "../../cpp_shader_constant_register_map.h"

// this is required for our Function Array's
#include <functional>

// Includes for PS30
#include "pbr_mrao_brush_ps30.inc"			// 0 - BRUSH MRAO
//#include "pbr_spec_brush_ps30.inc"		// 1 - BRUSH SPEC
#include "pbr_mrao_model_ps30.inc"			// 2 - MODEL MRAO
//#include "pbr_spec_model_ps30.inc"		// 3 - MODEL SPEC
//#include "pbr_spec_override_ps30.inc"		// 4 - NOT DONE -- BOTH SPEC
#include "pbr_mrao_displacement_ps30.inc"	// 5 - WVT MRAO
											// 6 - no longer WVT SPEC
											// 7 - no longer SSS MRAO
											// 8 - no longer SSS SPEC
#include "pbr_debug_ps30.inc"				// 9 - BOTH MRAO/SPEC

// Includes for VS30
#include "pbr_brush_vs30.inc"
#include "pbr_displacement_vs30.inc"
#include "pbr_model_vs30.inc"



extern ConVar mat_specular;
extern ConVar mat_printvalues;

#ifdef DEBUG_MRAO
// Note :	This is also a convar on the cdll_client_int.cpp and everything there has to be implemented for this to work.
//			Not having it will crash your game when this is changed :)
extern ConVar mat_mrao;
#endif

#ifdef DEBUG_FULLBRIGHT2
extern ConVar mat_fullbright; // Important to be an extern otherwise you get "muh convar redefined :(("
#endif

static ConVar mat_envmapdebug("mat_envmapdebug", "0");

#ifdef SDK2013MP
#ifdef DEBUG_LUXELS
static ConVar mat_luxels("mat_luxels", "0", FCVAR_CHEAT);
#endif
#endif

// Sampler List :
// s00 = BaseTexture
// s01 = WVT BASE2 / Wrinklemapping Base Wrinkles
// s02 = NormalMap
// s03 = NormalMap2 / Wrinklemapping Base Stretch
// s04 = Detail
// s05 = Special Effects ( Bent Normals )
// s06 = MRAO
// s07 = MRAO2 / Wrinklemapping Bump Wrinkles
// s08 = Properties Texture		( Wetness, Porosity, VectorMask, VectorRoughness )
// s09 = Properties Texture2	( Wetness, Porosity, VectorMask, VectorRoughness )
// s10 = VectorLayer2			( sRGB Color, AO )
// s11 = Lightmap Sampler / SSSS ( self shadow )
// s12 = SSS / BlendModulate
// s13 = Emission
// s14 = Emission2 / Wrinklemapping Bump Stretch / CUbemap ( last one, in case of cubemaplerp )
// s15 = Cubemap

// COLOR is for brushes. COLOR2 for models
#define LuxPBR_Params() void LuxPBR_Link_Params(PBR_Vars_t &info)	\
{																	\
	info.m_nMultipurpose7			= MATERIALNAME;					\
	info.m_nBumpMap					= BUMPMAP;						\
	info.m_nBumpFrame				= BUMPFRAME;					\
	info.m_nBumpTransform			= BUMPTRANSFORM;				\
	info.m_nNormalTexture			= NORMALTEXTURE;				\
	info.m_nNormalTexture2			= NORMALTEXTURE2;				\
	info.m_nSelfIllum_EnvMapMask_Alpha = SELFILLUM_ENVMAPMASK_ALPHA;\
	info.m_nMRAOTexture				= MRAOTEXTURE;					\
	info.m_nMRAOTexture2			= MRAOTEXTURE2;					\
	info.m_nSpecularTexture			= SPECULARTEXTURE;				\
	info.m_nSpecularTexture2		= SPECULARTEXTURE2;				\
	info.m_nEmissionTexture			= EMISSIONTEXTURE;				\
	info.m_nBaseTextureTint			= BASETEXTURETINT;				\
	info.m_nBaseTextureTint2		= BASETEXTURETINT2;				\
	info.m_nMRAOBias				= MRAOBIAS;						\
	info.m_nMRAO2Bias				= MRAOBIAS2;					\
	info.m_nEmissionTint			= EMISSIONTINT;					\
	info.m_MRAOTextureFrame			= MRAOFRAME;					\
	info.m_nMRAOTexture2			= MRAOTEXTURE2;					\
	info.m_nEmissionTexture2		= EMISSIONTEXTURE2;				\
	info.m_nSpecularTexture2		= SPECULARTEXTURE2;				\
	info.m_nSSSTexture				= SSSTEXTURE;					\
	info.m_nSSSTint					= SSSTINT;						\
	info.m_nSSSScale				= SSSSCALE;						\
	info.m_nSSSPower				= SSSPOWER;						\
	info.m_nSSSEmission				= SSSEMISSION;					\
	info.m_nSSSIntensity			= SSSINTENSITY;					\
	info.m_bHasParallax				= PARALLAXMAPPING;				\
	info.m_nParallaxHeight			= PARALLAXHEIGHT;				\
	info.m_nParallaxMaxOffset		= PARALLAXMAXOFFSET;			\
	info.m_nParallaxIntensity		= PARALLAXINTENSITY;			\
	info.m_nParallaxInterval		= PARALLAXINTERVAL;				\
	info.m_nDiElectricCoefficient	= DIELECTRICCOEFFICIENT;		\
	info.m_nSheen					= SHEEN;						\
	info.m_nSheenIntensity			= SHEENINTENSITY;				\
	info.m_nHullShell				= HULLSHELL;					\
	info.m_nWrinkleCompress			= COMPRESS;						\
	info.m_nWrinkleStretch			= STRETCH;						\
	info.m_nWrinkleCompressBump		= BUMPCOMPRESS;					\
	info.m_nWrinkleStretchBump		= BUMPSTRETCH;					\
	info.m_nMicroShadows			= MICROSHADOWS;					\
	info.m_nBentNormal				= BENTNORMAL;					\
	info.m_nWetnessPorosity			= WETNESSPOROSITY;				\
	info.m_nWetnessBias				= WETNESSBIAS;					\
	info.m_nPropertiesTexture		= PROPERTIESTEXTURE;			\
	info.m_nVectorLayerTexture		= VECTORLAYERTEXTURE;			\
	info.m_nLayerVector				= LAYERVECTOR;					\
	info.m_nLayerVectorSimilarity	= LAYERVECTORSIMILARITY;		\
	info.m_nLayerVector2			= LAYERVECTOR2;					\
	info.m_nLayerVectorSimilarity2	= LAYERVECTORSIMILARITY2;		\
	info.m_nWetnessBias2			= WETNESSBIAS2;					\
	info.m_nPropertiesTexture2		= PROPERTIESTEXTURE2;			\
	info.m_nVectorLayerTexture2		= VECTORLAYERTEXTURE2;			\
	info.m_bUseWVT					= INTERNALBOOLWVT;				\
	info.m_nSeamless				= SEAMLESS;						\
	info.m_nSeamlessScale			= SEAMLESS_SCALE;				\
	info.m_nSeamlessSecondary		= SEAMLESS_SECONDARY;			\
																	\
	Link_GlobalParameters()											\
	Link_DisplacementParameters()									\
	Link_MiscParameters()											\
	Link_DetailTextureParameters()									\
	Link_LightmappingParameters()									\
	Link_EnvironmentMapParameters()									\
	Link_ParallaxCorrectionParameters()								\
	Link_EnvMapMaskParameters()										\
}//-----------------------------------------------------------------|

#define LuxPBR_ParameterDeclaration()																																																			\
SHADER_PARAM(MATERIALNAME			,	SHADER_PARAM_TYPE_STRING	, "", "Can't access Material Name in the Drawing Function without this. Stores Materialname.")																				\
SHADER_PARAM(BUMPMAP				,	SHADER_PARAM_TYPE_TEXTURE	, "", "Bumpmap required for Lighting on Models. For model lightmaps use both $normaltexture and $bumpmap.")																	\
SHADER_PARAM(BUMPFRAME				,	SHADER_PARAM_TYPE_INTEGER	, "", "Frame Var")																																							\
SHADER_PARAM(BUMPTRANSFORM			,	SHADER_PARAM_TYPE_MATRIX	, "", "Transform Var")																																						\
SHADER_PARAM(NORMALTEXTURE			,	SHADER_PARAM_TYPE_TEXTURE	, "", "Internally used for Bumpmaps. $Bumpmap will get set automatically. Set this to the actual bumpmap preferably.")														\
SHADER_PARAM(NORMALTEXTURE2			,	SHADER_PARAM_TYPE_TEXTURE	, "", "Internally used for Bumpmaps. $Bumpmap will get set automatically. Set this to the actual bumpmap preferably.")														\
SHADER_PARAM(MRAOTEXTURE			,	SHADER_PARAM_TYPE_TEXTURE	, "", "MRAO Texture. Red = Metallic, Green = Roughness, Blue = Ambient Occlusion, Alpha = Nothing yet")																		\
SHADER_PARAM(SPECULARTEXTURE		,	SHADER_PARAM_TYPE_TEXTURE	, "", "Specular Texture. Enables Specular/Glossiness behaviour, instead of MRAO. RGB = Specular ( linear range ), Alpha = Glossiness")										\
SHADER_PARAM(EMISSIONTEXTURE		,	SHADER_PARAM_TYPE_TEXTURE	, "", "Emission Textures get added after lighting to cause overbrightening even in darkness.")																				\
SHADER_PARAM(BASETEXTURETINT		,	SHADER_PARAM_TYPE_COLOR		, "", "Tint the Basetexture. $Color and $Color2 will ( probably ) not work. This is to be used above those two.")															\
SHADER_PARAM(BASETEXTURETINT2		,	SHADER_PARAM_TYPE_COLOR		, "", "Same as $BaseTextureTint, but for $BaseTexture2.")																													\
SHADER_PARAM(MRAOBIAS				,	SHADER_PARAM_TYPE_VEC3		, "", "Bias MRAO [Roughness Metallic Ambient Occlusion]. This will add ontop of MRAO. -1.0f will remove, 1.0f will add. Similar to Levels")									\
SHADER_PARAM(MRAOBIAS2				,	SHADER_PARAM_TYPE_VEC3		, "", "Bias MRAO [Roughness Metallic Ambient Occlusion]. This will add ontop of MRAO. -1.0f will remove, 1.0f will add. Similar to Levels")									\
SHADER_PARAM(EMISSIONTINT			,	SHADER_PARAM_TYPE_COLOR		, "", "Tint Var")																																							\
SHADER_PARAM(MRAOFRAME				,	SHADER_PARAM_TYPE_INTEGER	, "", "Frame Var")																																							\
SHADER_PARAM(MRAOTEXTURE2			,	SHADER_PARAM_TYPE_TEXTURE	, "", "MRAO		Texture for Blended Materials")																																\
SHADER_PARAM(MRAOFRAME2				,	SHADER_PARAM_TYPE_INTEGER	, "", "Frame Var")																																							\
SHADER_PARAM(EMISSIONTEXTURE2		,	SHADER_PARAM_TYPE_TEXTURE	, "", "Emission	Texture for Blended Materials")																																\
SHADER_PARAM(SPECULARTEXTURE2		,	SHADER_PARAM_TYPE_TEXTURE	, "", "Specular	Texture for Blended Materials")																																\
SHADER_PARAM(SSSTEXTURE				,	SHADER_PARAM_TYPE_TEXTURE	, "", "Sub-Surface Scattering Texture. Enables SSS Behaviour, R = Thickness, GBA = RGB Color. This is done so you can use Compression when Blue is unused.")				\
SHADER_PARAM(SSSTINT				,	SHADER_PARAM_TYPE_COLOR		, "", "Tint Var. Blue is set to 0 if the SSSTexture does not carry alpha information. If your textures appear purple, set to 0 manually...")								\
SHADER_PARAM(SSSSCALE				,	SHADER_PARAM_TYPE_FLOAT		, "", "Scalar applied before Calculation. This is useful for $SSSEmission, which does not consider $SSSIntensity.")															\
SHADER_PARAM(SSSPOWER				,	SHADER_PARAM_TYPE_FLOAT		, "", "Power Factor for SSS Lighting. Will wrap the lighting more around the model when set higher than the default ( 1.0f )")												\
SHADER_PARAM(SSSEMISSION			,	SHADER_PARAM_TYPE_FLOAT		, "", "SSS Emission outputs the SSS without $SSSIntensity using the $EmissionTexture instead of the $SSSTexture. 1.0f full effect, 0.0f is usualy emission.")				\
SHADER_PARAM(SSSINTENSITY			,	SHADER_PARAM_TYPE_FLOAT		, "", "Multiplier for the Lighting results for SSS, does not apply to $SSSEmission")																						\
SHADER_PARAM(PARALLAXMAPPING		,	SHADER_PARAM_TYPE_BOOL		, "", "Unfinished Feature")																																					\
SHADER_PARAM(PARALLAXHEIGHT			,	SHADER_PARAM_TYPE_FLOAT		, "", "Unfinished Feature")																																					\
SHADER_PARAM(PARALLAXMAXOFFSET		,	SHADER_PARAM_TYPE_FLOAT		, "", "Unfinished Feature")																																					\
SHADER_PARAM(PARALLAXINTENSITY		,	SHADER_PARAM_TYPE_FLOAT		, "", "Unfinished Feature")																																					\
SHADER_PARAM(PARALLAXINTERVAL		,	SHADER_PARAM_TYPE_INTEGER	, "", "Unfinished Feature")																																					\
SHADER_PARAM(DIELECTRICCOEFFICIENT	,	SHADER_PARAM_TYPE_FLOAT		, "", "I cannot explain this. Look it up. Default Value is 0.025, however it usually ranges between 0.01 and 0.04. Essentially forces minimum reflection values!")			\
SHADER_PARAM(SELFILLUM_ENVMAPMASK_ALPHA, SHADER_PARAM_TYPE_BOOL		, "", "Override Feature.")																																					\
SHADER_PARAM(SHEEN					,	SHADER_PARAM_TYPE_BOOL		, "", "Enables Sheen Lighting Calculation for Cloth materials. Kind of experimental... Yes this works with SSS")															\
SHADER_PARAM(SHEENINTENSITY			,	SHADER_PARAM_TYPE_FLOAT		, "", "Multiplier for Lighting results.")																																	\
SHADER_PARAM(HULLSHELL				,	SHADER_PARAM_TYPE_BOOL		, "", "Unfinished Feature")																																					\
SHADER_PARAM(DITHEREDALPHA			,	SHADER_PARAM_TYPE_BOOL		, "", "Dithered Alpha. Uses alpha test but gives a result that looks similar to translucency... But bad.")																	\
SHADER_PARAM(MICROSHADOWS			,	SHADER_PARAM_TYPE_FLOAT		, "", "Microshadows will occlude additional areas based on the light direction. 0.0f to 1.0f range. High values might be too strong.")										\
SHADER_PARAM(BENTNORMAL				,	SHADER_PARAM_TYPE_TEXTURE   , "", "Specular Occlusion Texture. We only use it for the Occlusion, not for normals...")																					\
SHADER_PARAM(WETNESSPOROSITY		,	SHADER_PARAM_TYPE_BOOL		, "", "Enables Wetness Porosity Behaviour. Requires $PropertiesTexture!!"				)																					\
SHADER_PARAM(WETNESSBIAS			,	SHADER_PARAM_TYPE_FLOAT		, "", "Bias amount of Wetness ( set to -1 then add on top to slowly increase wetness. )")																					\
SHADER_PARAM(WETNESSBIAS2			,	SHADER_PARAM_TYPE_FLOAT		, "", "Bias amount of Wetness, for second texture on Blended Materials...)")																								\
SHADER_PARAM(PROPERTIESTEXTURE		,	SHADER_PARAM_TYPE_TEXTURE	, "", "R = Wetness, G = Porosity, B = VecLayerMask, A = VecLayer Roughness")																								\
SHADER_PARAM(PROPERTIESTEXTURE2		,	SHADER_PARAM_TYPE_TEXTURE	, "", "R = Wetness2, G = Porosity2, B = VecLayerMask2, A = VecLayer Roughness2")																							\
SHADER_PARAM(VECTORLAYERTEXTURE		,	SHADER_PARAM_TYPE_TEXTURE	, "", "The Texture to use for applying on a Vector. Incompatible with $Detail."			)																					\
SHADER_PARAM(VECTORLAYERTEXTURE2	,	SHADER_PARAM_TYPE_TEXTURE	, "", "Same as the first, but blended!")																																	\
SHADER_PARAM(LAYERVECTOR			,	SHADER_PARAM_TYPE_VEC3		, "", "The Normal Vector on which the Texture can appear on. [0 0 1] for upwards faces.")																					\
SHADER_PARAM(LAYERVECTORSIMILARITY	,	SHADER_PARAM_TYPE_FLOAT		, "", "Similarity that $LayerVector and Face Normals need to have. 0.3f is the default.")																					\
SHADER_PARAM(LAYERVECTOR2			,	SHADER_PARAM_TYPE_VEC3		, "", "The Normal Vector on which the Texture can appear on. [0 0 1] for upwards faces.")																					\
SHADER_PARAM(LAYERVECTORSIMILARITY2	,	SHADER_PARAM_TYPE_FLOAT		, "", "Similarity that $LayerVector and Face Normals need to have. 0.3f is the default.")																					\
SHADER_PARAM(SEAMLESS				,	SHADER_PARAM_TYPE_BOOL		, "", "Enable Seamless Texture Mapping behaviour for pbr. Note : BlendModulate is not Seamless.")\
SHADER_PARAM(SEAMLESS_SCALE			,	SHADER_PARAM_TYPE_FLOAT		, "", "Enable Seamless Texture Mapping behaviour for pbr. Note : BlendModulate is not Seamless.")\
SHADER_PARAM(SEAMLESS_SECONDARY		,	SHADER_PARAM_TYPE_BOOL		, "", "Enable Seamless Texture Mapping behaviour for pbr. Note : BlendModulate is not Seamless.")\
\
SHADER_PARAM(COMPRESS				,	SHADER_PARAM_TYPE_TEXTURE	, "", "I'm too lazy to add a real description to this right now, fix it for me, thanks!")																					\
SHADER_PARAM(STRETCH				,	SHADER_PARAM_TYPE_TEXTURE	, "", "I'm too lazy to add a real description to this right now, fix it for me, thanks!")																					\
SHADER_PARAM(BUMPCOMPRESS			,	SHADER_PARAM_TYPE_TEXTURE	, "", "I'm too lazy to add a real description to this right now, fix it for me, thanks!")																					\
SHADER_PARAM(BUMPSTRETCH			,	SHADER_PARAM_TYPE_TEXTURE	, "", "I'm too lazy to add a real description to this right now, fix it for me, thanks!")																					\
SHADER_PARAM(INTERNALBOOLWVT		,	SHADER_PARAM_TYPE_BOOL		, "", "Used internally by the shader. Set automatically if you do Blending.")																								\
Declare_DisplacementParameters()																																																				\
Declare_MiscParameters()																																																						\
Declare_DetailTextureParameters()																																																				\
Declare_LightmappingParameters()																																																				\
Declare_EnvironmentMapParameters()																																																				\
Declare_ParallaxCorrectionParameters()																																																			\
Declare_EnvMapMaskParameters()//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|


//==================================================================================================
// Static Shader Declarations ( this is done to avoid a bazillion if statements for dynamic state.
// Also static state, but it doesn't quite matter really )
//==================================================================================================
void StaticShader_Brush_Mrao(IShaderShadow *pShaderShadow, bool bHasDetailTexture, bool bHasSpecularTexture, bool bHasLightMapUVs, bool bHasBlendModulateTexture,
	bool bHasFlashlight, bool bHasEmissionTexture, bool bHasParallaxMapping, bool bHasParallaxCorrection,
	bool bHasSheen, bool bHasHullShell, bool bHasAnyWrinkleMapping, int iEnvMapMode,
	int iSelfIllumMode, bool bIsDecal, bool bIsModel, bool bHasBentNormal, bool bHasSSS, bool bHasSpecialProperties, bool bHasSeamless)
{
	DECLARE_STATIC_VERTEX_SHADER(pbr_brush_vs30);
	SET_STATIC_VERTEX_SHADER_COMBO(DETAILTEXTURE_UV, bHasDetailTexture);
	SET_STATIC_VERTEX_SHADER_COMBO(ENVMAPMASK_UV, bHasSpecularTexture);
	SET_STATIC_VERTEX_SHADER_COMBO(PARALLAXMAPPING, false);
	SET_STATIC_VERTEX_SHADER(pbr_brush_vs30);

	DECLARE_STATIC_PIXEL_SHADER(pbr_mrao_brush_ps30);
	SET_STATIC_PIXEL_SHADER_COMBO(FLASHLIGHT, bHasFlashlight);
	SET_STATIC_PIXEL_SHADER_COMBO(DETAILTEXTURE, bHasDetailTexture);
	SET_STATIC_PIXEL_SHADER_COMBO(EMISSION, bHasEmissionTexture);
	SET_STATIC_PIXEL_SHADER_COMBO(PCC, bHasParallaxCorrection);
	SET_STATIC_PIXEL_SHADER_COMBO(PARALLAXINTERVAL, bHasParallaxMapping);
	SET_STATIC_PIXEL_SHADER_COMBO(SPECIALPROPERTIES, bHasSpecialProperties);
	SET_STATIC_PIXEL_SHADER(pbr_mrao_brush_ps30);
}

void StaticShader_Brush_Specular(IShaderShadow *pShaderShadow, bool bHasDetailTexture, bool bHasSpecularTexture, bool bHasLightMapUVs, bool bHasBlendModulateTexture,
	bool bHasFlashlight, bool bHasEmissionTexture, bool bHasParallaxMapping, bool bHasParallaxCorrection,
	bool bHasSheen, bool bHasHullShell, bool bHasAnyWrinkleMapping, int iEnvMapMode,
	int iSelfIllumMode, bool bIsDecal, bool bIsModel, bool bHasBentNormal, bool bHasSSS, bool bHasSpecialProperties, bool bHasSeamless)
{
	DECLARE_STATIC_VERTEX_SHADER(pbr_brush_vs30);
	SET_STATIC_VERTEX_SHADER_COMBO(DETAILTEXTURE_UV, bHasDetailTexture);
	SET_STATIC_VERTEX_SHADER_COMBO(ENVMAPMASK_UV, bHasSpecularTexture);
	SET_STATIC_VERTEX_SHADER_COMBO(PARALLAXMAPPING, false);
	SET_STATIC_VERTEX_SHADER(pbr_brush_vs30);

//	DECLARE_STATIC_PIXEL_SHADER(pbr_spec_brush_ps30);
//	SET_STATIC_PIXEL_SHADER_COMBO(FLASHLIGHT, bHasFlashlight);
//	SET_STATIC_PIXEL_SHADER(pbr_spec_brush_ps30);
}

void StaticShader_Model_Mrao(IShaderShadow *pShaderShadow, bool bHasDetailTexture, bool bHasSpecularTexture, bool bHasLightMapUVs, bool bHasBlendModulateTexture,
	bool bHasFlashlight, bool bHasEmissionTexture, bool bHasParallaxMapping, bool bHasParallaxCorrection,
	bool bHasSheen, bool bHasHullShell, bool bHasAnyWrinkleMapping, int iEnvMapMode,
	int iSelfIllumMode, bool bIsDecal, bool bIsModel, bool bHasBentNormal, bool bHasSSS, bool bHasSpecialProperties, bool bHasSeamless)
{
	DECLARE_STATIC_VERTEX_SHADER(pbr_model_vs30);
	SET_STATIC_VERTEX_SHADER_COMBO(DETAILTEXTURE_UV, bHasDetailTexture);
	SET_STATIC_VERTEX_SHADER_COMBO(ENVMAPMASK_UV, bHasSpecularTexture);
	SET_STATIC_VERTEX_SHADER_COMBO(LIGHTMAP_UV, bHasLightMapUVs); // bLightMapUVs
	SET_STATIC_VERTEX_SHADER_COMBO(DECAL, bIsDecal);
	SET_STATIC_VERTEX_SHADER(pbr_model_vs30);

	DECLARE_STATIC_PIXEL_SHADER(pbr_mrao_model_ps30);
	SET_STATIC_PIXEL_SHADER_COMBO(FLASHLIGHT, bHasFlashlight);
	SET_STATIC_PIXEL_SHADER_COMBO(EMISSION, bHasEmissionTexture);
	SET_STATIC_PIXEL_SHADER_COMBO(DETAILTEXTURE, bHasDetailTexture);
	SET_STATIC_PIXEL_SHADER_COMBO(PARALLAXINTERVAL, bHasParallaxMapping);
	SET_STATIC_PIXEL_SHADER_COMBO(SHEEN, bHasSheen);
//	SET_STATIC_PIXEL_SHADER_COMBO(HULLSHELL, bHasHullShell);
	SET_STATIC_PIXEL_SHADER_COMBO(WRINKLEMAPPING, bHasAnyWrinkleMapping);
	SET_STATIC_PIXEL_SHADER_COMBO(BENTNORMALS, bHasBentNormal);
	SET_STATIC_PIXEL_SHADER_COMBO(SSS, bHasSSS);
	SET_STATIC_PIXEL_SHADER_COMBO(SPECIALPROPERTIES, bHasSpecialProperties);
	SET_STATIC_PIXEL_SHADER(pbr_mrao_model_ps30);
}

void StaticShader_Model_Specular(IShaderShadow *pShaderShadow, bool bHasDetailTexture, bool bHasSpecularTexture, bool bHasLightMapUVs, bool bHasBlendModulateTexture,
	bool bHasFlashlight, bool bHasEmissionTexture, bool bHasParallaxMapping, bool bHasParallaxCorrection,
	bool bHasSheen, bool bHasHullShell, bool bHasAnyWrinkleMapping, int iEnvMapMode,
	int iSelfIllumMode, bool bIsDecal, bool bIsModel, bool bHasBentNormal, bool bHasSSS, bool bHasSpecialProperties, bool bHasSeamless)
{
	DECLARE_STATIC_VERTEX_SHADER(pbr_model_vs30);
	SET_STATIC_VERTEX_SHADER_COMBO(DETAILTEXTURE_UV, bHasDetailTexture);
	SET_STATIC_VERTEX_SHADER_COMBO(ENVMAPMASK_UV, bHasSpecularTexture);
	SET_STATIC_VERTEX_SHADER_COMBO(LIGHTMAP_UV, bHasLightMapUVs); // bLightMapUVs
	SET_STATIC_VERTEX_SHADER_COMBO(DECAL, bIsDecal);
	SET_STATIC_VERTEX_SHADER(pbr_model_vs30);

//	DECLARE_STATIC_PIXEL_SHADER(pbr_spec_model_ps30);
//	SET_STATIC_PIXEL_SHADER_COMBO(FLASHLIGHT, bHasFlashlight);
//	SET_STATIC_PIXEL_SHADER(pbr_spec_model_ps30);
}

void StaticShader_Override(IShaderShadow *pShaderShadow, bool bHasDetailTexture, bool bHasSpecularTexture, bool bHasLightMapUVs, bool bHasBlendModulateTexture,
	bool bHasFlashlight, bool bHasEmissionTexture, bool bHasParallaxMapping, bool bHasParallaxCorrection,
	bool bHasSheen, bool bHasHullShell, bool bHasAnyWrinkleMapping, int iEnvMapMode,
	int iSelfIllumMode, bool bIsDecal, bool bIsModel, bool bHasBentNormal, bool bHasSSS, bool bHasSpecialProperties, bool bHasSeamless)
{
	if (bIsModel)
	{
		DECLARE_STATIC_VERTEX_SHADER(pbr_model_vs30);
		SET_STATIC_VERTEX_SHADER_COMBO(DETAILTEXTURE_UV, bHasDetailTexture);
		SET_STATIC_VERTEX_SHADER_COMBO(ENVMAPMASK_UV, bHasSpecularTexture);
		SET_STATIC_VERTEX_SHADER_COMBO(LIGHTMAP_UV, bHasLightMapUVs); // bLightMapUVs
		SET_STATIC_VERTEX_SHADER_COMBO(DECAL, bIsDecal);
		SET_STATIC_VERTEX_SHADER(pbr_model_vs30);
	}
	else
	{
		DECLARE_STATIC_VERTEX_SHADER(pbr_brush_vs30);
		SET_STATIC_VERTEX_SHADER_COMBO(DETAILTEXTURE_UV, bHasDetailTexture);
		SET_STATIC_VERTEX_SHADER_COMBO(ENVMAPMASK_UV, bHasSpecularTexture);
		SET_STATIC_VERTEX_SHADER_COMBO(PARALLAXMAPPING, false);
		SET_STATIC_VERTEX_SHADER(pbr_brush_vs30);
	}

//	DECLARE_STATIC_PIXEL_SHADER(pbr_spec_override_ps30);
//	SET_STATIC_PIXEL_SHADER_COMBO(FLASHLIGHT, bHasFlashlight);
//	SET_STATIC_PIXEL_SHADER(pbr_spec_override_ps30);
}

void StaticShader_WVT_Mrao(IShaderShadow *pShaderShadow, bool bHasDetailTexture, bool bHasSpecularTexture, bool bHasLightMapUVs, bool bHasBlendModulateTexture,
	bool bHasFlashlight, bool bHasEmissionTexture, bool bHasParallaxMapping, bool bHasParallaxCorrection,
	bool bHasSheen, bool bHasHullShell, bool bHasAnyWrinkleMapping, int iEnvMapMode,
	int iSelfIllumMode, bool bIsDecal, bool bIsModel, bool bHasBentNormal, bool bHasSSS, bool bHasSpecialProperties, bool bHasSeamless)
{
	DECLARE_STATIC_VERTEX_SHADER(pbr_displacement_vs30);
	SET_STATIC_VERTEX_SHADER_COMBO(SEAMLESS, bHasSeamless);
	SET_STATIC_VERTEX_SHADER_COMBO(SEAMLESS_SECONDARY, false);
	SET_STATIC_VERTEX_SHADER_COMBO(DETAILTEXTURE_UV, bHasDetailTexture);
	SET_STATIC_VERTEX_SHADER_COMBO(BLENDMODULATE_UV, bHasBlendModulateTexture);
	SET_STATIC_VERTEX_SHADER(pbr_displacement_vs30);

	DECLARE_STATIC_PIXEL_SHADER(pbr_mrao_displacement_ps30);
	SET_STATIC_PIXEL_SHADER_COMBO(FLASHLIGHT, bHasFlashlight);
	SET_STATIC_PIXEL_SHADER_COMBO(EMISSION, false);
	SET_STATIC_PIXEL_SHADER_COMBO(PCC, bHasParallaxCorrection);
	SET_STATIC_PIXEL_SHADER_COMBO(DETAILTEXTURE, bHasDetailTexture);
	SET_STATIC_PIXEL_SHADER_COMBO(PARALLAXINTERVAL, bHasParallaxMapping);
	SET_STATIC_PIXEL_SHADER_COMBO(SPECIALPROPERTIES, bHasSpecialProperties);
	SET_STATIC_PIXEL_SHADER(pbr_mrao_displacement_ps30);
}

void StaticShader_WVT_Specular(IShaderShadow *pShaderShadow, bool bHasDetailTexture, bool bHasSpecularTexture, bool bHasLightMapUVs, bool bHasBlendModulateTexture,
	bool bHasFlashlight, bool bHasEmissionTexture, bool bHasParallaxMapping, bool bHasParallaxCorrection,
	bool bHasSheen, bool bHasHullShell, bool bHasAnyWrinkleMapping, int iEnvMapMode,
	int iSelfIllumMode, bool bIsDecal, bool bIsModel, bool bHasBentNormal, bool bHasSSS, bool bHasSpecialProperties, bool bHasSeamless)
{
	/*
	DECLARE_STATIC_VERTEX_SHADER(pbr_displacement_vs30);
	SET_STATIC_VERTEX_SHADER_COMBO(SEAMLESS, bHasSeamless);
	SET_STATIC_VERTEX_SHADER_COMBO(SEAMLESS_SECONDARY, false);
	SET_STATIC_VERTEX_SHADER_COMBO(DETAILTEXTURE_UV, bHasDetailTexture);
	SET_STATIC_VERTEX_SHADER_COMBO(BLENDMODULATE_UV, bHasBlendModulateTexture);
	SET_STATIC_VERTEX_SHADER(pbr_displacement_vs30);
	*/
}

void StaticShader_Debug(IShaderShadow *pShaderShadow, bool bHasDetailTexture, bool bHasSpecularTexture, bool bHasLightMapUVs, bool bHasBlendModulateTexture,
	bool bHasFlashlight, bool bHasEmissionTexture, bool bHasParallaxMapping, bool bHasParallaxCorrection,
	bool bHasSheen, bool bHasHullShell, bool bHasAnyWrinkleMapping, int iEnvMapMode,
	int iSelfIllumMode, bool bIsDecal, bool bIsModel, bool bHasBentNormal, bool bHasSSS, bool bHasSpecialProperties, bool bHasSeamless)
{
	if (bIsModel)
	{
		DECLARE_STATIC_VERTEX_SHADER(pbr_model_vs30);
		SET_STATIC_VERTEX_SHADER_COMBO(DETAILTEXTURE_UV, false);
		SET_STATIC_VERTEX_SHADER_COMBO(ENVMAPMASK_UV, false);
		SET_STATIC_VERTEX_SHADER_COMBO(LIGHTMAP_UV, false); // bLightMapUVs
		SET_STATIC_VERTEX_SHADER_COMBO(DECAL, bIsDecal); // Is this important for debugging?
		SET_STATIC_VERTEX_SHADER(pbr_model_vs30);
	}
	else
	{
		DECLARE_STATIC_VERTEX_SHADER(pbr_brush_vs30);
		SET_STATIC_VERTEX_SHADER_COMBO(DETAILTEXTURE_UV, false);
		SET_STATIC_VERTEX_SHADER_COMBO(ENVMAPMASK_UV, false);
		SET_STATIC_VERTEX_SHADER_COMBO(PARALLAXMAPPING, false);
		SET_STATIC_VERTEX_SHADER(pbr_brush_vs30);
	}

	DECLARE_STATIC_PIXEL_SHADER(pbr_debug_ps30);
	SET_STATIC_PIXEL_SHADER(pbr_debug_ps30);
}

//==================================================================================================
// Dynamic Shader Declarations ( this is done to avoid a bazillion if statements for dynamic state.)
//
//==================================================================================================
//	bWriteWaterFogToAlpha,			bWriteDepthToAlpha,		iPixelFogCombo,				bFlashLightShadows,
//	bCascadedShadow,				bSkinning,				bHasMorphing,				bHasModelLightMapping,		iNumLights,		bIsModel
void DynamicShader_Brush_Mrao(IShaderDynamicAPI *pShaderAPI,
	bool bWriteWaterFogToAlpha,		bool bWriteDepthToAlpha, int iPixelFogCombo,		bool bFlashlightShadows,
	bool bHasCascadedShadow,		bool bSkinning,			bool bHasMorphing,			bool bHasModelLightMapping, int iNumLights, bool bIsModel)
{
	DECLARE_DYNAMIC_VERTEX_SHADER(pbr_brush_vs30);
	SET_DYNAMIC_VERTEX_SHADER(pbr_brush_vs30);

	// Setting up DYNAMIC pixel shader
	DECLARE_DYNAMIC_PIXEL_SHADER(pbr_mrao_brush_ps30);
	SET_DYNAMIC_PIXEL_SHADER_COMBO(WRITEWATERFOGTODESTALPHA, bWriteWaterFogToAlpha);
	SET_DYNAMIC_PIXEL_SHADER_COMBO(WRITE_DEPTH_TO_DESTALPHA, bWriteDepthToAlpha);
	SET_DYNAMIC_PIXEL_SHADER_COMBO(PIXELFOGTYPE, iPixelFogCombo);
	SET_DYNAMIC_PIXEL_SHADER_COMBO(FLASHLIGHTSHADOWS, bFlashlightShadows);
	SET_DYNAMIC_PIXEL_SHADER_COMBO(CASCADED_SHADOW, bHasCascadedShadow);
	SET_DYNAMIC_PIXEL_SHADER(pbr_mrao_brush_ps30);
}

void DynamicShader_Brush_Specular(IShaderDynamicAPI *pShaderAPI,
	bool bWriteWaterFogToAlpha, bool bWriteDepthToAlpha, int iPixelFogCombo, bool bFlashlightShadows,
	bool bHasCascadedShadow, bool bSkinning, bool bHasMorphing, bool bHasModelLightMapping, int iNumLights, bool bIsModel)
{
	DECLARE_DYNAMIC_VERTEX_SHADER(pbr_brush_vs30);
//	SET_DYNAMIC_VERTEX_SHADER_COMBO(X, Y);
	SET_DYNAMIC_VERTEX_SHADER(pbr_brush_vs30);

	// Setting up DYNAMIC pixel shader
//	DECLARE_DYNAMIC_PIXEL_SHADER(pbr_spec_brush_ps30);
//	SET_DYNAMIC_PIXEL_SHADER_COMBO(WRITEWATERFOGTODESTALPHA, bWriteWaterFogToAlpha);
//	SET_DYNAMIC_PIXEL_SHADER_COMBO(WRITE_DEPTH_TO_DESTALPHA, bWriteDepthToAlpha);
//	SET_DYNAMIC_PIXEL_SHADER_COMBO(PIXELFOGTYPE, nPixelFogCombo);
//	SET_DYNAMIC_PIXEL_SHADER_COMBO(FLASHLIGHTSHADOWS, bFlashlightShadows);
//	SET_DYNAMIC_PIXEL_SHADER(pbr_spec_brush_ps30);
}

void DynamicShader_Model_Mrao(IShaderDynamicAPI *pShaderAPI,
	bool bWriteWaterFogToAlpha, bool bWriteDepthToAlpha, int iPixelFogCombo, bool bFlashlightShadows,
	bool bHasCascadedShadow, bool bSkinning, bool bHasMorphing, bool bHasModelLightMapping, int iNumLights, bool bIsModel)
{
	DECLARE_DYNAMIC_VERTEX_SHADER(pbr_model_vs30);
	SET_DYNAMIC_VERTEX_SHADER_COMBO(SKINNING, bSkinning);
	SET_DYNAMIC_VERTEX_SHADER_COMBO(MORPHING, bHasMorphing);
	SET_DYNAMIC_VERTEX_SHADER(pbr_model_vs30);

	DECLARE_DYNAMIC_PIXEL_SHADER(pbr_mrao_model_ps30);
	SET_DYNAMIC_PIXEL_SHADER_COMBO(NUM_LIGHTS, iNumLights);
	SET_DYNAMIC_PIXEL_SHADER_COMBO(WRITEWATERFOGTODESTALPHA, bWriteWaterFogToAlpha);
	SET_DYNAMIC_PIXEL_SHADER_COMBO(WRITE_DEPTH_TO_DESTALPHA, bWriteDepthToAlpha);
	SET_DYNAMIC_PIXEL_SHADER_COMBO(PIXELFOGTYPE, iPixelFogCombo);
	SET_DYNAMIC_PIXEL_SHADER_COMBO(FLASHLIGHTSHADOWS, bFlashlightShadows);
	SET_DYNAMIC_PIXEL_SHADER_COMBO(LIGHTMAPPED_MODEL, bHasModelLightMapping);
	SET_DYNAMIC_PIXEL_SHADER_COMBO(CASCADED_SHADOW, bHasCascadedShadow);
	SET_DYNAMIC_PIXEL_SHADER(pbr_mrao_model_ps30);
}

void DynamicShader_Model_Specular(IShaderDynamicAPI *pShaderAPI,
	bool bWriteWaterFogToAlpha, bool bWriteDepthToAlpha, int iPixelFogCombo, bool bFlashlightShadows,
	bool bHasCascadedShadow, bool bSkinning, bool bHasMorphing, bool bHasModelLightMapping, int iNumLights, bool bIsModel)
{
	DECLARE_DYNAMIC_VERTEX_SHADER(pbr_model_vs30);
	SET_DYNAMIC_VERTEX_SHADER_COMBO(SKINNING, bSkinning);
	SET_DYNAMIC_VERTEX_SHADER_COMBO(MORPHING, bHasMorphing);
	SET_DYNAMIC_VERTEX_SHADER(pbr_model_vs30);

//	DECLARE_DYNAMIC_PIXEL_SHADER(pbr_spec_model_ps30);
//	SET_DYNAMIC_PIXEL_SHADER_COMBO(NUM_LIGHTS, nNumLights);
//	SET_DYNAMIC_PIXEL_SHADER_COMBO(WRITEWATERFOGTODESTALPHA, bWriteWaterFogToAlpha);
//	SET_DYNAMIC_PIXEL_SHADER_COMBO(WRITE_DEPTH_TO_DESTALPHA, bWriteDepthToAlpha);
//	SET_DYNAMIC_PIXEL_SHADER_COMBO(PIXELFOGTYPE, nPixelFogCombo);
//	SET_DYNAMIC_PIXEL_SHADER_COMBO(FLASHLIGHTSHADOWS, bFlashlightShadows);
//	SET_DYNAMIC_PIXEL_SHADER_COMBO(LIGHTMAPPED_MODEL, bLightMappedModel);
//	SET_DYNAMIC_PIXEL_SHADER(pbr_spec_model_ps30);
}

void DynamicShader_Override(IShaderDynamicAPI *pShaderAPI,
	bool bWriteWaterFogToAlpha, bool bWriteDepthToAlpha, int iPixelFogCombo, bool bFlashlightShadows,
	bool bHasCascadedShadow, bool bSkinning, bool bHasMorphing, bool bHasModelLightMapping, int iNumLights, bool bIsModel)
{
	if (bIsModel)
	{
		DECLARE_DYNAMIC_VERTEX_SHADER(pbr_model_vs30);
		SET_DYNAMIC_VERTEX_SHADER_COMBO(SKINNING, bSkinning);
		SET_DYNAMIC_VERTEX_SHADER_COMBO(MORPHING, bHasMorphing);
		SET_DYNAMIC_VERTEX_SHADER(pbr_model_vs30);
	}
	else
	{
		DECLARE_DYNAMIC_VERTEX_SHADER(pbr_brush_vs30);
		SET_DYNAMIC_VERTEX_SHADER(pbr_brush_vs30);
	}

	// Setting up DYNAMIC pixel shader
//	DECLARE_DYNAMIC_PIXEL_SHADER(pbr_spec_override_ps30);
//	SET_DYNAMIC_PIXEL_SHADER_COMBO(NUM_LIGHTS, nNumLights);
//	SET_DYNAMIC_PIXEL_SHADER_COMBO(WRITEWATERFOGTODESTALPHA, bWriteWaterFogToAlpha);
//	SET_DYNAMIC_PIXEL_SHADER_COMBO(WRITE_DEPTH_TO_DESTALPHA, bWriteDepthToAlpha);
//	SET_DYNAMIC_PIXEL_SHADER_COMBO(PIXELFOGTYPE, nPixelFogCombo);
//	SET_DYNAMIC_PIXEL_SHADER_COMBO(FLASHLIGHTSHADOWS, bFlashlightShadows);
//	SET_DYNAMIC_PIXEL_SHADER_COMBO(LIGHTMAPPED_MODEL, bLightMappedModel);
//	SET_DYNAMIC_PIXEL_SHADER(pbr_spec_override_ps30);
}

void DynamicShader_WVT_Mrao(IShaderDynamicAPI *pShaderAPI,
	bool bWriteWaterFogToAlpha, bool bWriteDepthToAlpha, int iPixelFogCombo, bool bFlashlightShadows,
	bool bHasCascadedShadow, bool bSkinning, bool bHasMorphing, bool bHasModelLightMapping, int iNumLights, bool bIsModel)
{

}

void DynamicShader_WVT_Specular(IShaderDynamicAPI *pShaderAPI,
	bool bWriteWaterFogToAlpha, bool bWriteDepthToAlpha, int iPixelFogCombo, bool bFlashlightShadows,
	bool bHasCascadedShadow, bool bSkinning, bool bHasMorphing, bool bHasModelLightMapping, int iNumLights, bool bIsModel)
{

}

void DynamicShader_Debug(IShaderDynamicAPI *pShaderAPI,
	bool bWriteWaterFogToAlpha, bool bWriteDepthToAlpha, int iPixelFogCombo, bool bFlashlightShadows,
	bool bHasCascadedShadow, bool bSkinning, bool bHasMorphing, bool bHasModelLightMapping, int iNumLights, bool bIsModel)
{
//	DECLARE_DYNAMIC_VERTEX_SHADER(pbr_comb_vs30);
//	SET_DYNAMIC_VERTEX_SHADER_COMBO(SKINNING, bHasBones);
//	SET_DYNAMIC_VERTEX_SHADER_COMBO(COMPRESSED_VERTS, nVertexCompression);
//	SET_DYNAMIC_VERTEX_SHADER(pbr_comb_vs30);
//
//	// Setting up DYNAMIC pixel shader
//	DECLARE_DYNAMIC_PIXEL_SHADER(pbr_mrao_debug_ps30);
//	SET_DYNAMIC_PIXEL_SHADER(pbr_mrao_debug_ps30);
}

//==================================================================================================
// Putting these into arrays. Yes you heard right, function array!
// Could do it with int's but that will execute the code from the array and pointers dumb
//==================================================================================================
/*
bHasDetailTexture,	bHasSpecularTexture,	bHasLightMapUVs,		bHasBlendModulateTexture,
bHasFlashlight,		bHasEmissionTexture,	bHasParallaxMapping,	bHasParallaxCorrection,
bHasSheen,			bHasHullShell,			bHasAnyWrinkleMapping,	iEnvMapMode,
iSelfIllumMode,		bIsDecal,				bIsModel,				bHasBentNormal
bHasSSS,			bhasSpecialProperties,	bHasSeamless)
*/
std::function<void(	IShaderShadow*,
bool,				bool,					bool,					bool,
bool,				bool,					bool,					bool,
bool,				bool,					bool,					int,
int,				bool,					bool,					bool,
bool,				bool,					bool)> DeclareStatics[]
{
	StaticShader_Brush_Mrao,		StaticShader_Brush_Specular,	StaticShader_Model_Mrao,
	StaticShader_Model_Specular,	StaticShader_Override,			StaticShader_WVT_Mrao,
	StaticShader_WVT_Specular,
	StaticShader_Debug
};

/*
bWriteWaterFogToAlpha,			bWriteDepthToAlpha,		(pShaderAPI->GetPixelFogCombo()),	bFlashlightShadows,
false,							(pShaderAPI->GetCurrentNumBones() > 0 ? true : false),		(g_pHardwareConfig->HasFastVertexTextures() && pShaderAPI->IsHWMorphingEnabled()),	false,
LightState.m_nNumLights,		bIsModel
*/
std::function<void(IShaderDynamicAPI*,
bool,							bool,					int,						bool,
bool,							bool,					bool,						bool,
int,							bool)> DeclareDynamics[]
{
	DynamicShader_Brush_Mrao,		DynamicShader_Brush_Specular,	DynamicShader_Model_Mrao,
	DynamicShader_Model_Specular,	DynamicShader_Override,			DynamicShader_WVT_Mrao,
	DynamicShader_WVT_Specular,
	DynamicShader_Debug
};

//==================================================================================================
// Initiate Parameters, mainly making sure something is set for normal maps and cubemaps
//==================================================================================================
void InitParams(CBaseVSShader *pShader, IMaterialVar **params, const char *pMaterialName, PBR_Vars_t &info, bool bIsModel = false)
{
	// I wanted to debug print some values and needed to know which material they belong to. However pMaterialName is not available to the draw!
	// Well, now it is...
	params[info.m_nMultipurpose7]->SetStringValue(pMaterialName);
	
	// The usual...
	Flashlight_BorderColorSupportCheck();

	// PBR relies heavily on envmaps
	if (!params[info.m_nEnvMap]->IsDefined())
		params[info.m_nEnvMap]->SetStringValue("env_cubemap");

	// This is kinda cursed, fix it ? no : yes; Well too bad its cursed so thats a no
	if (IsParamDefined(info.m_nBaseTexture2) || IsParamDefined(info.m_nBumpMap2) || IsParamDefined(info.m_nNormalTexture2) || IsParamDefined(info.m_nMRAOTexture2) || IsParamDefined(info.m_nEmissionTexture2))
	{
		if (IsParamDefined(info.m_nBumpMap2))
		{
			params[info.m_nNormalTexture2]->SetStringValue(params[info.m_nBumpMap2]->GetStringValue()); // Set Normaltexture2 to whatever $whatevermap2 is
		}
		// This will actually enable WVT...
		SetIntParamValue(info.m_bUseWVT, true);
	}

	//////////////////////////////
	//		GIGA IMPORTANT		//
	//////////////////////////////
	//	Models that want to use Model Lightmapping with Bump Maps previously used $NormalTexture
	//	This is because the Engine checks for $BumpMap ( and $Phong ) to determine whether or not a model should be able to use Model Lightmaps.
	//	Originally we just always Set m_nBumpMap to undefined after settings its StringValue to m_nNormalTexture.
	//	Later we figured out that in order to get Lighting Information for Static Lightsources ( Non-Named Lights, aka those without TargetName )
	//	$BumpMap needs to have some kind of StringValue on it. Yeah guess what the engine checks for...
	//	The damage was already done ( On HDTF for example ) I didn't want to tell everyone to change their vmt's for a feature they don't use.
	//	So now we force set $bumpmap. However to retain Model Lightmapping functionality, models need to use both.
	if (IsParamDefined(info.m_nBumpMap))
	{
		if (IsParamDefined(info.m_nNormalTexture))
		{
			params[info.m_nBumpMap]->SetUndefined(); // Jokes on you Valve Programmers. Model Lightmapping with Bumpmaps! but no bumped lighting...
		}
		else
		{
			params[info.m_nNormalTexture]->SetStringValue(params[info.m_nBumpMap]->GetStringValue());
		}		
	}
	else
	{
		if (!IsParamDefined(info.m_nNormalTexture))
		{
			params[info.m_nBumpMap]->SetStringValue("ShoopDaWhoop"); // Jokes on you Valve Programmers. That Bumped Lighting belongs to us now
		}
	}

#ifdef DefaultMRAOTexture
	if (!params[info.m_nSpecularTexture]->IsDefined() && !params[info.m_nMRAOTexture]->IsDefined())
	{
			params[info.m_nMRAOTexture]->SetStringValue("dev/pbr_mraotexture");
	}
#endif

	// Default Value is supposed to be 1.0f
	FloatParameterDefault(info.m_nDetailBlendFactor, 1.0f)

	// Default Value is supposed to be 4.0f
	FloatParameterDefault(info.m_nDetailScale, 4.0f)

	// Default Value is supposed to be 1.0f
	FloatParameterDefault(info.m_nEnvMapSaturation, 1.0f)

	// Default Value is supposed to be 0.025f
	FloatParameterDefault(info.m_nDiElectricCoefficient, 0.025f)

	// Default Value is supposed to be 1.0f
	FloatParameterDefault(info.m_nSheenIntensity, 1.0f)

	// Default Values are supposed to be 1.0f
	FloatParameterDefault(info.m_nSSSScale	  , 1.0f)
	FloatParameterDefault(info.m_nSSSPower	  , 1.0f)
	FloatParameterDefault(info.m_nSSSIntensity, 1.0f)

	FloatParameterDefault(info.m_nLayerVectorSimilarity, 0.3f)

	// If in decal mode, no debug override...
	if (IS_FLAG_SET(MATERIAL_VAR_DECAL)) // TODO: also on models?
	{
		SET_FLAGS(MATERIAL_VAR_NO_DEBUG_OVERRIDE);
	}

	size_t iLength = Q_strlen(pMaterialName);
	bool bHasModelPath = false;
	if (iLength > 6)
	{
		const char *ModelPath = "models";
		if (memicmp(ModelPath, pMaterialName, 6) == 0)
		{
			bHasModelPath = true;
		}
	}

	if (bIsModel || bHasModelPath)
	{
		SET_FLAGS(MATERIAL_VAR_MODEL);
		SET_FLAGS2(MATERIAL_VAR2_SUPPORTS_HW_SKINNING);             // Required for skinning
		SET_FLAGS2(MATERIAL_VAR2_LIGHTING_VERTEX_LIT);              // Required for dynamic lighting
		SET_FLAGS2(MATERIAL_VAR2_NEEDS_TANGENT_SPACES);             // Required for dynamic lighting
		SET_FLAGS2(MATERIAL_VAR2_DIFFUSE_BUMPMAPPED_MODEL);         // Required for dynamic lighting
		SET_FLAGS2(MATERIAL_VAR2_NEEDS_BAKED_LIGHTING_SNAPSHOTS);   // Required for ambient cube
		SET_FLAGS2(MATERIAL_VAR2_USES_ENV_CUBEMAP);
		SET_FLAGS2(MATERIAL_VAR2_SUPPORTS_FLASHLIGHT);              // "Required for flashlight"
		SET_FLAGS2(MATERIAL_VAR2_USE_FLASHLIGHT);                   // "Required for flashlight"
	}
	else // Set material var2 flags specific to brushes
	{
		SET_FLAGS2(MATERIAL_VAR2_LIGHTING_LIGHTMAP);                // Required for lightmaps
		SET_FLAGS2(MATERIAL_VAR2_LIGHTING_BUMPED_LIGHTMAP);         // Required for lightmaps
		// We always need this because of the flashlight.
		SET_FLAGS2(MATERIAL_VAR2_NEEDS_TANGENT_SPACES);
		SET_FLAGS2(MATERIAL_VAR2_USES_ENV_CUBEMAP);
		SET_FLAGS2(MATERIAL_VAR2_SUPPORTS_FLASHLIGHT);              // "Required for flashlight"
		SET_FLAGS2(MATERIAL_VAR2_USE_FLASHLIGHT);                   // "Required for flashlight"
	}
}

//==================================================================================================
// Load Textures
//==================================================================================================
void ShaderInit(CBaseVSShader *pShader, IMaterialVar **params, PBR_Vars_t &info, bool bIsWVT = false, bool bIsModel = false)
{
	pShader->LoadTexture(info.m_nFlashlightTexture, TEXTUREFLAGS_SRGB);

	pShader->LoadTexture(info.m_nBaseTexture, TEXTUREFLAGS_SRGB);

	LoadNormalTexture(info.m_nNormalTexture)
	LoadTextureWithCheck(info.m_nBentNormal, 0)

	// Adding this flag probably does not work...
	if (IsParamDefined(info.m_nEnvMap))
	{
		pShader->LoadCubeMap(info.m_nEnvMap, 0); // TEXTUREFLAGS_ALL_MIPS
	}

	if (params[info.m_nSpecularTexture]->IsDefined())
	{
		pShader->LoadTexture(info.m_nSpecularTexture, 0);
	}
	else
	{
		LoadTextureWithCheck(info.m_nMRAOTexture, 0);
	}
	
	LoadTextureWithCheck(info.m_nDetailTexture, 0)
	else
	{
		LoadTextureWithCheck(info.m_nVectorLayerTexture, 0);
		LoadTextureWithCheck(info.m_nPropertiesTexture, 0);
		if (bIsWVT)
		{
			LoadTextureWithCheck(info.m_nVectorLayerTexture2, 0);
			LoadTextureWithCheck(info.m_nPropertiesTexture2, 0);
		}
	}

	LoadTextureWithCheck(info.m_nEmissionTexture, TEXTUREFLAGS_SRGB)
	LoadTextureWithCheck(info.m_nLightMap, 0)

	if (bIsWVT)
	{
		pShader->LoadTexture(info.m_nBaseTexture2, TEXTUREFLAGS_SRGB);
		LoadNormalTexture(info.m_nNormalTexture2)

		// Why?
		// We expect an MRAOtexture, however we don't need to load one if we are using a Specular texture.
		if (params[info.m_nSpecularTexture2]->IsDefined())
		{
			pShader->LoadTexture(info.m_nSpecularTexture2);
		}
		else
		{
			LoadTextureWithCheck(info.m_nMRAOTexture2, 0);
		}

		LoadTextureWithCheck(info.m_nEmissionTexture2, TEXTUREFLAGS_SRGB)
		LoadTextureWithCheck(info.m_nBlendModulateTexture, 0)
	}
	
	// We do this twice now. Is this at the correct location?
	if (bIsModel)
	{
		if (IsParamDefined(info.m_nWrinkleCompress) && IsParamDefined(info.m_nWrinkleStretch))
		{
			pShader->LoadTexture(info.m_nWrinkleCompress, TEXTUREFLAGS_SRGB);
			pShader->LoadTexture(info.m_nWrinkleStretch, TEXTUREFLAGS_SRGB);
		}

		if (g_pConfig->UseBumpmapping() && IsParamDefined(info.m_nWrinkleCompressBump) && IsParamDefined(info.m_nWrinkleStretchBump))
		{
			pShader->LoadTexture(info.m_nWrinkleCompressBump, 0);
			pShader->LoadTexture(info.m_nWrinkleStretchBump, 0);
		}

		LoadTextureWithCheck(info.m_nSSSTexture, 0)
		/*
		// If the SSS Texture does not have an alpha channel, it means there is no blue. Overwrite the tint so we don't use the white alpha channel!
		if (IsTextureLoaded(info.m_nSSSTexture))
		{
			ITexture *pSSSTexture = params[info.m_nSSSTexture]->GetTextureValue();
			// Check if the texture has flags that indicate an alpha channel
			
			if (((pSSSTexture->GetFlags() & TEXTUREFLAGS_ONEBITALPHA) ? false : true) || ((pSSSTexture->GetFlags() & TEXTUREFLAGS_EIGHTBITALPHA) ? false : true))
			{
				float SSSTintTemp[3] = { 0, 0, 0 };
				GetVec3ParamValue(info.m_nSSSTint, SSSTintTemp);
				SSSTintTemp[2] = 0.0f; // Now the blue is gone c:, Now put it back in the vmt.
				SetVec3ParamValue(info.m_nSSSTint, SSSTintTemp)
			}

			if (pSSSTexture->GetFlags() & TEXTUREFLAGS_SRGB ? true : false)
			{
				Warning("%s is flagged to have an alpha channel but is also sRGB O.o\n Alpha channel's are linear. So you have 2/3 gamma channels and a linear one. Fix this or expect graphical issues!!!\n", params[info.m_nMultipurpose7]->GetStringValue());
			}
		}
		*/
	}
};

//==================================================================================================
// Determine, well, basically everything. Declare Shaders and Set PSREG's.
//==================================================================================================
void DrawGeneric(CBaseVSShader *pShader, IMaterialVar **params, IShaderDynamicAPI *pShaderAPI, IShaderShadow *pShaderShadow, PBR_Vars_t &info, VertexCompressionType_t vertexCompression, CBasePerMaterialContextData **pContextDataPtr,
	bool bIsBrush, bool bIsModel, bool bIsWVT, bool bIsOverride, bool bIsSSS, int iShaderLookup, bool bCSM, bool bHasAlphaBlending)
{
	// iShaderNumber is used to determine which shader to use down the line, that way, we only need to evaluate some things once instead of multiple times.
	// This will literally save us like 13 If-Statements or so, and using a lookup table is always faster than if statements.
	// 1 = brush_mrao, 2 = brush_spec, 3 = model_mrao, 4 = model_spec, 5 = override, 6 = wvt_mrao, 7 = wvt_spec, 8 = sss_mrao, 9 = sss_spec
	int iShaderNumber = iShaderLookup;

	//==================================================================================================
	// Setting up booleans
	//==================================================================================================
	// Flags
	bool bHasFlashlight				=	pShader->UsingFlashlight(params);
	bool bSelfIllum					=	!bHasFlashlight		&&	IS_FLAG_SET(MATERIAL_VAR_SELFILLUM); // No SelfIllum under the flashlight
	bool bAlphatest					=							IS_FLAG_SET(MATERIAL_VAR_ALPHATEST);
	bool bNormalMapAlphaEnvMapMask	=	!bHasFlashlight		&&	IS_FLAG_SET(MATERIAL_VAR_NORMALMAPALPHAENVMAPMASK); // No Envmapping under the flashlight
	bool bBaseAlphaEnvMapMask		=	!bHasFlashlight		&&	IS_FLAG_SET(MATERIAL_VAR_BASEALPHAENVMAPMASK); // No Envmapping under the flashlight
	bool bIsDecal					=							IS_FLAG_SET(MATERIAL_VAR_DECAL);
	bool bHasBaseTexture			=							IsTextureLoaded(info.m_nBaseTexture);				
	bool bHasBaseTexture2			=	bIsWVT				&&	IsTextureLoaded(info.m_nBaseTexture2);
	bool bHasNormalTexture			=							IsTextureLoaded(info.m_nNormalTexture);
	bool bHasNormalTexture2			=	bIsWVT				&&	IsTextureLoaded(info.m_nNormalTexture2);
	bool bHasDetailTexture			=							IsTextureLoaded(info.m_nDetailTexture);			
	bool bHasMRAOTexture			=	!bIsOverride		&&	IsTextureLoaded(info.m_nMRAOTexture);
	bool bHasMRAOTexture2			=	bIsWVT				&&  IsTextureLoaded(info.m_nMRAOTexture2);
	bool bHasSpecularTexture		=	bHasMRAOTexture		&&	IsTextureLoaded(info.m_nSpecularTexture);
	bool bHasSpecularTexture2		=	bIsWVT				&&	IsTextureLoaded(info.m_nSpecularTexture2);
	bool bHasEmissionTexture		=	!bIsOverride	&& !bHasFlashlight	&&	IsTextureLoaded(info.m_nEmissionTexture);												
	bool bHasEmissionTexture2		=	bIsWVT			&& !bHasFlashlight	&&	IsTextureLoaded(info.m_nEmissionTexture2);												
	bool bHasEnvMap					=	!bHasFlashlight		&&	IsTextureLoaded(info.m_nEnvMap);
	bool bLightMappedModel			=	bIsModel			&&	IsTextureLoaded(info.m_nLightMap);		
	bool bHasBlendModulateTexture	=	bIsWVT				&&	IsTextureLoaded(info.m_nBlendModulateTexture);
	bool bHasSSSTexture				=	bIsModel			&&	IsTextureLoaded(info.m_nSSSTexture);
	bool bHasSheen					=	bIsModel			&&	GetBoolParamValue(info.m_nSheen);
	bool bHasHullShell				=	bIsModel			&&	GetBoolParamValue(info.m_nHullShell);
	bool bHasLightMapUVs			=	bIsModel			&&	GetBoolParamValue(info.m_nLightMapUVs);
	bool bHasParallaxCorrection		=   bHasEnvMap			&&	GetBoolParamValue(info.m_nEnvMapParallax);
	bool bHasParallaxMapping		=							GetBoolParamValue(info.m_bHasParallax);
	bool bHasBaseTextureWrinkle		=	bHasBaseTexture		&&	IsTextureLoaded(info.m_nWrinkleCompress)		&& IsTextureLoaded(info.m_nWrinkleStretch);
	bool bHasNormalTextureWrinkle	=	bHasNormalTexture	&&	IsTextureLoaded(info.m_nWrinkleCompressBump)	&& IsTextureLoaded(info.m_nWrinkleStretchBump);
	bool bHasAnyWrinkleMapping		=	bHasNormalTextureWrinkle || bHasBaseTextureWrinkle;
	bool bHasBentNormal				=							IsTextureLoaded(info.m_nBentNormal);
	bool bHasPropertiesTexture		=							IsTextureLoaded(info.m_nPropertiesTexture);
	bool bHasVectorLayer			=	bHasPropertiesTexture&&	IsTextureLoaded(info.m_nVectorLayerTexture);
	bool bRequiresProperties		=	bHasPropertiesTexture && !bHasDetailTexture; // || bHasVectorLayer && !bHasDetailTexture; // Not supported in combination with Detail.
	bool bHasPropertiesTexture2		=	bIsWVT && IsTextureLoaded(info.m_nPropertiesTexture2);
	bool bHasVectorLayer2			=	bIsWVT && bHasPropertiesTexture2 && IsTextureLoaded(info.m_nVectorLayerTexture2);
	bool bHasSeamless				=	bIsWVT && GetBoolParamValue(info.m_nSeamless);

	// Needed for things I do not fully understand right now
	bool IsOpaque(bIsFullyOpaque, info.m_nBaseTexture, bAlphatest);
	bIsFullyOpaque = bIsFullyOpaque && !bHasAlphaBlending; // tiny hack for true transparency

	// If we have a SpecularTexture, we are using Specular/Glossiness variant of the Shader.
	if (!bIsOverride && bHasSpecularTexture)
	{
		iShaderNumber++;
	}

	// Purpose : Int to tell the Shader what Mask to use.
	//	0 = No Cubemap
	//	1 = $EnvMap no Masking
	//	2 = $EnvMapMask
	//	3 = $BaseAlphaEnvMapMask
	//	4 = $NormalMapAlphaEnvMapMask
	//	Order is important, we set the Combo to 3 for the non-bumped Shader and avoid writing a SKIP:
	//	bHasEnvMap will cause 0 if false, 1 if true. We then override based on the rest.
	int iEnvMapMode = 0;
	int iSelfIllumMode = bSelfIllum; // 0 if no selfillum, 1 if selfillum is on. 2 is $SelfIllum_EnvMapMask_Alpha
	if (bIsOverride) // only used on Override.
	{
		iEnvMapMode = ComputeEnvMapMode(bHasEnvMap, bHasSpecularTexture, bBaseAlphaEnvMapMask, bNormalMapAlphaEnvMapMask);

		// All of these conditions MUST be true.
		if (bHasSpecularTexture && bSelfIllum && GetBoolParamValue(info.m_nSelfIllum_EnvMapMask_Alpha))
		{
			iSelfIllumMode = 2;
		}
	}

#ifdef DEBUG_MRAO
	int i_mat_mrao = mat_mrao.GetInt();

	// really for debugging only...
	if (i_mat_mrao != 0)
	{
		iShaderNumber = 5; // Is this the correct number?
	}
#endif

	bool bHasBumpTextureTransform		= params[info.m_nBumpTransform			]->MatrixIsIdentity();
	bool bHasBlendTextureTransform		= params[info.m_nBlendModulateTransform	]->MatrixIsIdentity();
	bool bHasDetailTextureTransform		= params[info.m_nDetailTextureTransform	]->MatrixIsIdentity();
	bool bHasEMMTextureTransform		= params[info.m_nEnvMapMaskTransform	]->MatrixIsIdentity();

	// WVT
	bool bHasBumpTextureTransform2		= params[info.m_nBumpTransform2			]->MatrixIsIdentity();
	bool bHasBaseTexture2Transform		= params[info.m_nBaseTextureTransform2	]->MatrixIsIdentity();

//////////////////////////////////////////////////////////////////////////////////////////////////
//										SNAPSHOTTING											//
//////////////////////////////////////////////////////////////////////////////////////////////////
	if (pShader->IsSnapshotting())
	{
		const bool bFastVertexTextures = g_pHardwareConfig->HasFastVertexTextures();

		if (bAlphatest)
		{
			pShaderShadow->EnableAlphaTest(true);
			pShaderShadow->AlphaFunc(SHADER_ALPHAFUNC_GEQUAL, params[info.m_nAlphaTestReference]->GetFloatValue()); // This will by default be 0
			pShaderShadow->EnableAlphaWrites(true);	
		}
	/*

	TODO: Test this stuff?

	if (bHasParallaxMapping)
	{
//		note :	OverrideDepthEnable already disables DepthTest. The second variable overrides value change... so probably depthwrites?
//				Either way, we will probably need both for parallax mapping.
//		pShaderShadow->EnableDepthTest(true);
		pShaderShadow->EnableDepthWrites(true);
		pShader->OverrideDepthEnable(true, true);
	}
	*/

		//==================================================================================================
		// Set up Samplers
		//==================================================================================================
		// I'm tired of your bullshit
		ITexture *pBaseTexture = params[info.m_nBaseTexture]->GetTextureValue();
		EnableSampler(SAMPLER_BASETEXTURE, pBaseTexture->GetFlags() & TEXTUREFLAGS_SRGB ? true : false)
		EnableSampler(SAMPLER_NORMALTEXTURE, false)
		EnableSampler(SHADER_SAMPLER6, false) // MRAOS/SPECULAR Sampler
		if (bHasFlashlight)
		{
			EnableFlashlightSamplers(bHasFlashlight, SAMPLER_SHADOWDEPTH, SAMPLER_RANDOMROTATION, SAMPLER_FLASHLIGHTCOOKIE, info.m_nBaseTexture)
		}
		else
		{
			EnableSamplerWithCheck(bHasEnvMap, SHADER_SAMPLER14, false) // !pShader->IsHDREnabled()
		}
		EnableSampler(SHADER_SAMPLER15, bHasFlashlight || bHasEmissionTexture) // H++ only runs LDR.

		EnableSamplerWithCheck(bHasEmissionTexture, SHADER_SAMPLER13, true)
		
		// H++ only runs LDR. We check for Brushes too in case we ever get model lightmapping preview...
		EnableSampler(SAMPLER_LIGHTMAP, (g_pHardwareConfig->GetHDRType() == HDR_TYPE_NONE ? true : false) && bIsBrush)

		if (bHasDetailTexture)
		{
			ITexture *pDetailTexture = params[info.m_nDetailTexture]->GetTextureValue();
			EnableSampler(SAMPLER_DETAILTEXTURE, pDetailTexture->GetFlags() & TEXTUREFLAGS_SRGB ? true : false)
		}
		else
		{
			if (bRequiresProperties) // Vector Layer + Wetness Porosity
			{
				EnableSampler(SAMPLER_DETAILTEXTURE, true) // Yes. Always please. VectorLayer Texture.
				EnableSampler(SHADER_SAMPLER8, false) // Properties Sampler
				EnableSampler(SHADER_SAMPLER9, false)
				EnableSampler(SHADER_SAMPLER10, true)
			}
		}

		EnableSamplerWithCheck(bHasBentNormal, SHADER_SAMPLER5, false)
	
		if (bHasSSSTexture || bHasBlendModulateTexture) // SSS and BlendModulate are on the same Sampler ( WVT can't have SSS )
		{
			// Sampler12
			pShaderShadow->EnableTexture(SHADER_SAMPLER12, true);
			// Since we store a color on the blue channel of the SSS Texture, it is never sRGB. Otherwise we get 3/4 gamma corrected channels...
			pShaderShadow->EnableSRGBRead(SHADER_SAMPLER12, false);
		}
	
		if (bIsWVT)
		{
			EnableSampler(SAMPLER_BASETEXTURE2, true)
			EnableSampler(SAMPLER_NORMALTEXTURE2, false)
			EnableSampler(SHADER_SAMPLER7, false) // MRAOS/SPECULAR Sampler
			EnableSamplerWithCheck(bHasEmissionTexture2, SHADER_SAMPLER15, true)
		}
	
		if (bIsModel)
		{
			if (bFastVertexTextures)
			{
				GetVertexShaderFormat_ModelMorphed()
			}
			else
			{
				GetVertexShaderFormat_Model()
			}
			
			if (bHasAnyWrinkleMapping)
			{
				EnableSampler(SHADER_SAMPLER1, true)
				EnableSampler(SHADER_SAMPLER3, true)
				EnableSampler(SHADER_SAMPLER7, false)
				EnableSampler(SHADER_SAMPLER11, false)
			}
		}
		else
		{
			if (bIsWVT)
			{
				GetVertexShaderFormat_Displacement()
			}
			else
			{
				GetVertexShaderFormat_Brush()
			}
		}

		// Shaders below ps20b had to use a Gamma Lookup table to convert Linear to sRGB. This is done on the pixelshaders now.
		pShaderShadow->EnableSRGBWrite(true);

		//==================================================================================================
		// Declare Static Shaders ( lookup table ), value of iShaderNumber evaluated before Snapshotting
		//==================================================================================================
		// Required for Morphing
		if (bIsModel && bFastVertexTextures)
			SET_FLAGS2(MATERIAL_VAR2_USES_VERTEXID); // Use the Vertex ID Stream

		auto DeclareStatic = DeclareStatics[iShaderNumber];

		DeclareStatic(pShaderShadow, bHasDetailTexture, (bIsOverride && bHasSpecularTexture), bHasLightMapUVs, bHasBlendModulateTexture,
		bHasFlashlight, (bHasEmissionTexture || bHasEmissionTexture2), bHasParallaxMapping, bHasParallaxCorrection,
		bHasSheen, bHasHullShell, bHasAnyWrinkleMapping, iEnvMapMode,
		iSelfIllumMode, bIsDecal, bIsModel, bHasBentNormal, bHasSSSTexture, bRequiresProperties, bHasSeamless);
	
		//	Same thing, but manual
		//	std::function<void(IShaderShadow*, bool)> DeclareStatic = DeclareStatics[iCurrentShader];
	}
	else // end of snapshotting
	{
		// Getting the light state
		LightState_t LightState;
		pShaderAPI->GetDX9LightState(&LightState);
		int iNumLights = LightState.m_nNumLights;

		if (mat_printvalues.GetBool())
		{


	//		Warning("%d, %d . First is memory Address bool, second one the actual bool. \n", m_bStaticLightVertex, LightState.m_bStaticLight);
		}



		//===========================================================================//
		// Bind Textures
		//===========================================================================//
		BindTextureWithCheckAndFallback(bHasBaseTexture, SAMPLER_BASETEXTURE, info.m_nBaseTexture, info.m_nBaseTextureFrame, TEXTURE_WHITE)
		BindTextureWithCheck(bHasEmissionTexture, SHADER_SAMPLER13, info.m_nEmissionTexture, 0)

// if mat_fullbright 2. Bind a standard white texture...
#ifdef DEBUG_FULLBRIGHT2 
		if (mat_fullbright.GetInt() == 2 && !IS_FLAG_SET(MATERIAL_VAR_NO_DEBUG_OVERRIDE))
			BindTextureStandard(SAMPLER_BASETEXTURE, TEXTURE_GREY)
		if (bIsWVT)
			BindTextureStandard(SAMPLER_BASETEXTURE2, TEXTURE_GREY)
#endif

#ifdef DETAILTEXTURING
		BindTextureWithCheck(bHasDetailTexture, SAMPLER_DETAILTEXTURE, info.m_nDetailTexture, info.m_nDetailFrame)
#endif

#ifdef CUBEMAPS
		BindTextureWithCheck(bHasEnvMap && mat_specular.GetBool(), SHADER_SAMPLER14, info.m_nEnvMap, info.m_nEnvMapFrame)
#endif

		BindTextureWithCheckAndFallback(bHasNormalTexture, SAMPLER_NORMALTEXTURE, info.m_nNormalTexture, info.m_nBumpFrame, TEXTURE_NORMALMAP_FLAT)
		BindTextureWithCheck(bHasMRAOTexture, SHADER_SAMPLER6, info.m_nMRAOTexture, info.m_MRAOTextureFrame)
		BindTextureWithCheck(bHasSpecularTexture, SHADER_SAMPLER6, info.m_nSpecularTexture, info.m_MRAOTextureFrame)

		if (bRequiresProperties)
		{
			// VectorLayer / Wetness.Porosity.VectorMask.VectorRoughness
			BindTextureWithCheck(bRequiresProperties, SHADER_SAMPLER8, info.m_nPropertiesTexture, 0)
			BindTextureWithCheck(bHasVectorLayer, SHADER_SAMPLER4, info.m_nVectorLayerTexture, 0)

			if (bIsWVT)
			{
				BindTextureWithCheck(bHasPropertiesTexture2, SHADER_SAMPLER9, info.m_nPropertiesTexture2, 0)
				BindTextureWithCheck(bHasVectorLayer2, SHADER_SAMPLER10, info.m_nVectorLayerTexture2, 0)
			}
		}

		if (bIsModel)
		{
			if (bLightMappedModel)
			{
				BindTextureWithoutCheck(SAMPLER_LIGHTMAP, info.m_nLightMap, 0);
			}

			if (bHasAnyWrinkleMapping)
			{
				if (bHasBaseTextureWrinkle)
				{
					BindTextureWithoutCheck(SHADER_SAMPLER1, info.m_nWrinkleCompress, info.m_nBaseTextureFrame)
					BindTextureWithoutCheck(SHADER_SAMPLER3, info.m_nWrinkleStretch, info.m_nBaseTextureFrame)
				}
				else
				{
					BindTextureWithCheckAndFallback(bHasBaseTexture, SHADER_SAMPLER1, info.m_nBaseTexture, info.m_nBaseTextureFrame, TEXTURE_WHITE)
					BindTextureWithCheckAndFallback(bHasBaseTexture, SHADER_SAMPLER3, info.m_nBaseTexture, info.m_nBaseTextureFrame, TEXTURE_WHITE)
				}

				if (bHasNormalTextureWrinkle)
				{
					BindTextureWithoutCheck(SHADER_SAMPLER7, info.m_nWrinkleCompress, info.m_nBaseTextureFrame)
					BindTextureWithoutCheck(SHADER_SAMPLER11, info.m_nWrinkleStretch, info.m_nBaseTextureFrame)
				}
				else
				{
					BindTextureWithCheckAndFallback(bHasNormalTexture, SHADER_SAMPLER7, info.m_nNormalTexture, info.m_nBumpFrame, TEXTURE_NORMALMAP_FLAT)
					BindTextureWithCheckAndFallback(bHasNormalTexture, SHADER_SAMPLER11, info.m_nNormalTexture, info.m_nBumpFrame, TEXTURE_NORMALMAP_FLAT)
				}
			}

			BindTextureWithCheck(bHasBentNormal, SHADER_SAMPLER5, info.m_nBentNormal, 0)
			BindTextureWithCheck(bHasSSSTexture, SHADER_SAMPLER12, info.m_nSSSTexture, 0)
		}
		else // else its a brush or displacement
		{
			BindTextureStandard(SAMPLER_LIGHTMAP, TEXTURE_LIGHTMAP_BUMPED); // 

			// This used to be below. However, we check for bIsBrush here anyways... Why would we do it again?
			// TODO: Supposedly brushes don't receive either... so why does this even exist?
			LightState.m_bAmbientLight = false;
			LightState.m_nNumLights = 0;

			if (bIsWVT)
			{
				BindTextureWithCheck(bHasBlendModulateTexture, SAMPLER_BLENDMODULATE, info.m_nBlendModulateTexture, 0)
				BindTextureWithCheckAndFallback(bHasBaseTexture2, SAMPLER_BASETEXTURE2, info.m_nBaseTexture2, info.m_nBaseTextureFrame, TEXTURE_WHITE)
				BindTextureWithCheckAndFallback(bHasNormalTexture2, SAMPLER_NORMALTEXTURE2, info.m_nNormalTexture2, info.m_nBumpFrame2, TEXTURE_NORMALMAP_FLAT)

				BindTextureWithCheck(bHasMRAOTexture2, SHADER_SAMPLER7, info.m_nMRAOTexture2, info.m_MRAOTextureFrame)
				BindTextureWithCheck(bHasSpecularTexture2, SHADER_SAMPLER7, info.m_nSpecularTexture2, info.m_MRAOTextureFrame)

				BindTextureWithCheck(bHasEmissionTexture2, SHADER_SAMPLER15, info.m_nEmissionTexture2, 0)
			}
		}

#ifdef SDK2013MP // Required. SP doesn't have it.
#ifdef DEBUG_LUXELS
		bool bDebugLuxels = mat_luxels.GetBool();
		if ((bDebugLuxels && !bIsModel) || (bDebugLuxels && bLightMappedModel))
		{
			pShaderAPI->BindStandardTexture(SAMPLER_LIGHTMAP, TEXTURE_DEBUG_LUXELS);
		}
#endif
#endif

		//===========================================================================//
		// Prepare floats for Constant Registers
		//===========================================================================//
		// f4Empty is just float4 Name = {1,1,1,1};
		// Yes I have some excessive problems with macro overusage...
		f4Empty(f4BaseTextureTint_DECE) // Di-Electric-Co-Efficient
		f4Empty(f4BaseTextureTint2_WetnessBias2)
		f4Empty(f4DetailTint_BlendFactor)
		f4Empty(f4EmissionTint_WetnessBias)
		f4Empty(f4EnvMapTint_LightScale)
		f4Empty(f4EnvMapSaturation_Contrast)
		f4Empty(f4DetailBlendMode_Empty) // yzw empty
		f4Empty(f4ParallaxControls)
		f4Empty(f4MRAOControls)
		f4Empty(f4MRAO2Controls) 
		f4Empty(f4SSSTint_Empty)
		f4Empty(f4SSSControls)
		f4Empty(f4VariousControls1)
		f4Empty(f4VectorLayer_Similarity)
		f4Empty(f4VectorLayer_Similarity2)
		BOOL BBools[16] = { false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false };
		
		// Lets start with vec3's
		GetVec3ParamValue(info.m_nBaseTextureTint, f4BaseTextureTint_DECE);
		GetVec3ParamValue(info.m_nDetailTint, f4DetailTint_BlendFactor);
		GetVec3ParamValue(info.m_nEmissionTint, f4EmissionTint_WetnessBias);
		GetVec3ParamValue(info.m_nEnvMapTint, f4EnvMapTint_LightScale);
		GetVec3ParamValue(info.m_nEnvMapSaturation, f4EnvMapSaturation_Contrast);
		GetVec3ParamValue(info.m_nMRAOBias, f4MRAOControls);
		if (bRequiresProperties)
		{
			BBools[3] = bHasVectorLayer;
			BBools[4] = GetBoolParamValue(info.m_nWetnessPorosity);
			GetVec3ParamValue(info.m_nLayerVector, f4VectorLayer_Similarity);
			f4VectorLayer_Similarity[3] = GetFloatParamValue(info.m_nLayerVectorSimilarity);
			f4EmissionTint_WetnessBias[3] = GetFloatParamValue(info.m_nWetnessBias);
			if (bIsWVT)
			{
				GetVec3ParamValue(info.m_nLayerVector2, f4VectorLayer_Similarity2);
				f4VectorLayer_Similarity2[3] = GetFloatParamValue(info.m_nLayerVectorSimilarity2);
			}
		}

		if (bIsWVT)
		{
			GetVec3ParamValue(info.m_nBaseTextureTint2, f4BaseTextureTint2_WetnessBias2);
			f4BaseTextureTint2_WetnessBias2[3] = GetFloatParamValue(info.m_nWetnessBias2);
			GetVec3ParamValue(info.m_nMRAO2Bias, f4MRAO2Controls);
			BBools[6] = bHasSeamless;
			BBools[8] = bHasBlendModulateTexture;
			BBools[10] = pShaderAPI->InEditorMode();
		}
		
		// Now Other Vecs
		if (bHasSSSTexture)
		{
			GetVec3ParamValue(info.m_nSSSTint, f4SSSTint_Empty);
			f4SSSControls[0] = GetFloatParamValue(info.m_nSSSScale);
			f4SSSControls[1] = GetFloatParamValue(info.m_nSSSPower);
			f4SSSControls[2] = GetFloatParamValue(info.m_nSSSEmission);
			f4SSSControls[3] = GetFloatParamValue(info.m_nSSSIntensity);
		}

		// Floats
		f4BaseTextureTint_DECE[3]		= GetFloatParamValue(info.m_nDiElectricCoefficient);
		f4DetailTint_BlendFactor[3]		= GetFloatParamValue(info.m_nDetailBlendFactor);
		f4EnvMapTint_LightScale[3]		= GetFloatParamValue(info.m_nEnvMapLightScale);
		f4EnvMapSaturation_Contrast[3]	= GetFloatParamValue(info.m_nEnvMapContrast);
		f4MRAOControls[3]				= GetFloatParamValue(info.m_nSheenIntensity);
		if (bHasParallaxMapping)
		{
		f4ParallaxControls[0] = GetFloatParamValue(info.m_nParallaxHeight);
		f4ParallaxControls[1] = GetFloatParamValue(info.m_nParallaxMaxOffset);
		f4ParallaxControls[2] = GetFloatParamValue(info.m_nParallaxIntensity);
		f4ParallaxControls[3] = GetFloatParamValue(info.m_nParallaxInterval);
		}

#ifdef DefaultMRAOTexture
		// Basically , if there is no mrao we bind ~~a white texture~~ Nothing ( Sampler will be 0 0 0 0), and then ~~scale~~ bias ( +- ) that.
		if (!bHasMRAOTexture && !bHasSpecularTexture)
		{
			f4MRAOControls[0] = 0.00f; // We don't want metallic
			f4MRAOControls[1] = 1.00f; // We want full roughness
			f4MRAOControls[2] = 1.00f; // We want full AO
			f4MRAOControls[3] = 0.50f; // Some Sheen, only used if Sheen is enabled.
		}
#endif

		// Ints
		f4DetailBlendMode_Empty[0] = GetIntParamValue(info.m_nDetailBlendMode);

#ifdef PARALLAXCORRECTEDCUBEMAPS
		SetUpPCC(bHasParallaxCorrection, info.m_nEnvMapOrigin, info.m_nEnvMapParallaxOBB1, info.m_nEnvMapParallaxOBB2, info.m_nEnvMapParallaxOBB3, 39, 38)
#endif

		//==================================================================================================
		// Set Vertexshader Constant Registers (VSREG's...?) Also some other stuff
		//==================================================================================================
		// Always having this
		pShader->SetVertexShaderTextureTransform(VERTEX_SHADER_SHADER_SPECIFIC_CONST_13, info.m_nBaseTextureTransform);

		if (bHasBumpTextureTransform)
			pShader->SetVertexShaderTextureTransform(VERTEX_SHADER_SHADER_SPECIFIC_CONST_15, info.m_nBumpTransform);
		else
			pShader->SetVertexShaderTextureTransform(VERTEX_SHADER_SHADER_SPECIFIC_CONST_15, info.m_nBaseTextureTransform);

		if (bHasDetailTextureTransform)
			pShader->SetVertexShaderTextureScaledTransform(VERTEX_SHADER_SHADER_SPECIFIC_CONST_17, info.m_nDetailTextureTransform, info.m_nDetailScale);
		else
			pShader->SetVertexShaderTextureScaledTransform(VERTEX_SHADER_SHADER_SPECIFIC_CONST_17, info.m_nBaseTextureTransform, info.m_nDetailScale);

		if (bHasEMMTextureTransform)
			pShader->SetVertexShaderTextureTransform(VERTEX_SHADER_SHADER_SPECIFIC_CONST_19, info.m_nEnvMapMaskTransform);
		else
			pShader->SetVertexShaderTextureTransform(VERTEX_SHADER_SHADER_SPECIFIC_CONST_19, info.m_nBaseTextureTransform);

		if (bIsWVT)
		{
			if (bHasBaseTexture2Transform)
				pShader->SetVertexShaderTextureTransform(242, info.m_nBaseTextureTransform2);
			else
				pShader->SetVertexShaderTextureTransform(242, info.m_nBaseTextureTransform);


			if (bHasBumpTextureTransform2)
				pShader->SetVertexShaderTextureTransform(244, info.m_nBumpTransform2);
			else if (bHasBaseTexture2Transform)
				pShader->SetVertexShaderTextureTransform(242, info.m_nBaseTextureTransform2);
			else
				pShader->SetVertexShaderTextureTransform(242, info.m_nBaseTextureTransform);

			if (bHasBlendTextureTransform)
				pShader->SetVertexShaderTextureTransform(VERTEX_SHADER_SHADER_SPECIFIC_CONST_10, info.m_nBlendModulateTransform);
			else
				pShader->SetVertexShaderTextureTransform(VERTEX_SHADER_SHADER_SPECIFIC_CONST_10, info.m_nBaseTextureTransform);

			f4Empty(f4SeamlessScale)
			f4SeamlessScale[0] = GetFloatParamValue(info.m_nSeamlessScale) != 0.0f;

			pShaderAPI->SetVertexShaderConstant(VERTEX_SHADER_SHADER_SPECIFIC_CONST_30, f4SeamlessScale);
		}


		if (bIsModel && g_pHardwareConfig->HasFastVertexTextures())
		{
			// This is a function that was previously... locked behind an #ifndef and shouldn't have been able to be used...
			SetHWMorphVertexShaderState(pShaderAPI, VERTEX_SHADER_SHADER_SPECIFIC_CONST_10, VERTEX_SHADER_SHADER_SPECIFIC_CONST_11, SHADER_VERTEXTEXTURE_SAMPLER0);
		
			bool bUnusedTexCoords[3] = { false, false, !pShaderAPI->IsHWMorphingEnabled() || !bIsDecal };
			pShaderAPI->MarkUnusedVertexFields(0, 3, bUnusedTexCoords);

		}

		// This does a lot of things for us!!
		// Just don't touch constants <32 and you should be fine c:
		bool bFlashlightShadows = false;
		SetupStockConstantRegisters(bHasFlashlight, SAMPLER_FLASHLIGHTCOOKIE, SAMPLER_RANDOMROTATION, info.m_nFlashlightTexture, info.m_nFlashlightTextureFrame, bFlashlightShadows)
//		BBools[1] = bFlashlightShadows; // bHasBlendModulateTexture

		int iFogIndex = 0;
		bool bWriteDepthToAlpha = false;
		bool bWriteWaterFogToAlpha = false;
		SetupDynamicComboVariables(iFogIndex, bWriteDepthToAlpha, bWriteWaterFogToAlpha, bIsFullyOpaque)

		//==================================================================================================
		// Set Pixelshader Constant Registers (PSREG's)
		//==================================================================================================
				pShaderAPI->SetPixelShaderConstant(32, f4BaseTextureTint_DECE, 1);
				if (bHasDetailTexture)
				{
					pShaderAPI->SetPixelShaderConstant(33, f4DetailTint_BlendFactor, 1);
				}
				else
				{
					pShaderAPI->SetPixelShaderConstant(33, f4VectorLayer_Similarity, 1);
				}	
				pShaderAPI->SetPixelShaderConstant(34, f4EmissionTint_WetnessBias, 1);
				pShaderAPI->SetPixelShaderConstant(35, f4EnvMapTint_LightScale, 1);
		//		pShaderAPI->SetPixelShaderConstant(36, f4EnvMapFresnelRanges_, 1);
				pShaderAPI->SetPixelShaderConstant(37, f4DetailBlendMode_Empty, 1);
		//										   38, PCC EnvMap Origin
		//										   39, PCC Bounding	Box
		//										   40, PCC Bounding	Box
		//										   41, PCC Bounding	Box
		//		pShaderAPI->SetPixelShaderConstant(42, f4PhongTint_Boost, 1);
		//		pShaderAPI->SetPixelShaderConstant(43, f4PhongFresnelRanges_Exponent, 1);
		//		pShaderAPI->SetPixelShaderConstant(44, f4RimLightControls, 1);
		//		pShaderAPI->SetPixelShaderConstant(45, f4SelfIllumFresnelMinMaxExp, 1);
				pShaderAPI->SetPixelShaderConstant(46, f4EnvMapSaturation_Contrast, 1);
		//		pShaderAPI->SetPixelShaderConstant(47, g_BaseTexture2Tint_Factor
		//		pShaderAPI->SetPixelShaderConstant(48, g_Detail2Tint_BlendFactor
		//		pShaderAPI->SetPixelShaderConstant(49, g_SelfIllum2Tint_Factor
				pShaderAPI->SetPixelShaderConstant(50, f4ParallaxControls, 1);
				pShaderAPI->SetPixelShaderConstant(51, f4MRAOControls, 1);
//				pShaderAPI->SetPixelShaderConstant(52, f4SSSTint_Empty, 1);
//				pShaderAPI->SetPixelShaderConstant(53, f4SSSControls, 1);

				if (bIsWVT)
				{
					pShaderAPI->SetPixelShaderConstant(56, f4BaseTextureTint2_WetnessBias2, 1);
					pShaderAPI->SetPixelShaderConstant(57, f4MRAO2Controls, 1);
					pShaderAPI->SetPixelShaderConstant(58, f4VectorLayer_Similarity2, 1);
				}
				else
				{
					if (bHasSSSTexture)
					{
						pShaderAPI->SetPixelShaderConstant(56, f4SSSTint_Empty, 1);
						pShaderAPI->SetPixelShaderConstant(57, f4SSSControls, 1);
					}
				}

		// ALWAYS! Required for Ambient Light and more.
		pShaderAPI->SetBooleanPixelShaderConstant(0, BBools, 16, true);

		if (bHasEnvMap)
		{
			int iEnvMapLOD = 6;
			auto envTexture = params[info.m_nEnvMap]->GetTextureValue(); 		// Determine the detaillevel used for the cubemap
			if (envTexture)
			{	// Get power of 2 of texture width
				int width = envTexture->GetMappingWidth();
				int mips = 0;
				while (width >>= 1)
					++mips;
				// Cubemap has 4 sides so 2 mips less
				iEnvMapLOD = mips;
			}
			// Dealing with very high and low resolution cubemaps
			if (iEnvMapLOD > 12)	iEnvMapLOD = 12;
			if (iEnvMapLOD < 4)		iEnvMapLOD = 4;
			// .w is unused
// 			vEyePos_SpecExponent[3] = iEnvMapLOD; // Lux already sets eyepos
			f4VariousControls1[1] = iEnvMapLOD;
		}

		if (mat_printvalues.GetBool())
		{
			ITexture *pEnvMap = params[info.m_nEnvMap]->GetTextureValue();

			Warning("%s now prints its values.. \n", params[info.m_nMultipurpose7]->GetStringValue());
			Warning("Envmap is currently %s \n", pEnvMap->GetName());
		}
/*
#ifdef DEBUG_MRAO
		switch (i_mat_mrao)
		{
		case 1:
			f4VariousControls1[0] = 1;
				break;
		case 2:
			f4VariousControls1[0] = 2;
				break;
		case 3:
			f4VariousControls1[0] = 3;
				break;
		default:
			f4VariousControls1[0] = 1;
				break;
		}

		// Hack to make debug shader work with specular textures.
		if (bHasSpecularTexture)
		{
			f4VariousControls1[0] += 3;
		}
#endif
*/
		f4VariousControls1[0] = GetFloatParamValue(info.m_nParallaxInterval);
//		f4VariousControls1[1] = ENVMAPLOD its supposed to be on g_EyePos but we had to move it for LUX
		f4VariousControls1[2] = GetFloatParamValue(info.m_nMicroShadows);
		pShaderAPI->SetPixelShaderConstant(55, f4VariousControls1, 1);

		
		pShaderAPI->SetPixelShaderStateAmbientLightCube(PSREG_AMBIENT_CUBE, LightState.m_bAmbientLight);
		pShaderAPI->CommitPixelShaderLighting(PSREG_LIGHT_INFO_ARRAY);

		//==================================================================================================
		// Declare Dynamic Shaders ( Using Lookup Table )
		// Basically, this will ( hopefully ) be way faster than spamming if-statements
		// ( It should be faster, we are using an array with a lookup table now... )
		//==================================================================================================
		auto DeclareDynamic = DeclareDynamics[iShaderNumber];

		DeclareDynamic(pShaderAPI,
		bWriteWaterFogToAlpha,	bWriteDepthToAlpha, (pShaderAPI->GetPixelFogCombo()), bFlashlightShadows,
		false, (pShaderAPI->GetCurrentNumBones() > 0 ? true : false), pShaderAPI->IsHWMorphingEnabled() && g_pHardwareConfig->HasFastVertexTextures(), false, iNumLights, bIsModel);

		/*
		// More flashlight related stuff
		if (bHasFlashlight)
		{
			VMatrix worldToTexture;
			float atten[4], pos[4], tweaks[4];

			const FlashlightState_t &flashlightState = pShaderAPI->GetFlashlightState(worldToTexture);
			SetFlashLightColorFromState(flashlightState, pShaderAPI, 28);

	//		pShader->BindTexture(SAMPLER_FLASHLIGHTCOOKIE, flashlightState.m_pSpotlightTexture, flashlightState.m_nSpotlightTextureFrame);

			// Set the flashlight attenuation factors
			atten[0] = flashlightState.m_fConstantAtten;
			atten[1] = flashlightState.m_fLinearAtten;
			atten[2] = flashlightState.m_fQuadraticAtten;
			atten[3] = flashlightState.m_FarZ;
			pShaderAPI->SetPixelShaderConstant(13, atten, 1);

			// Set the flashlight origin
			pos[0] = flashlightState.m_vecLightOrigin[0];
			pos[1] = flashlightState.m_vecLightOrigin[1];
			pos[2] = flashlightState.m_vecLightOrigin[2];
			pShaderAPI->SetPixelShaderConstant(14, pos, 1);

			pShaderAPI->SetPixelShaderConstant(15, worldToTexture.Base(), 4);

			// Tweaks associated with a given flashlight
			tweaks[0] = ShadowFilterFromState(flashlightState);
			tweaks[1] = ShadowAttenFromState(flashlightState);
			pShader->HashShadow2DJitter(flashlightState.m_flShadowJitterSeed, &tweaks[2], &tweaks[3]);
			pShaderAPI->SetPixelShaderConstant(2, tweaks, 1);
		
			// Dimensions of screen, used for screen-space noise map sampling
			float vScreenScale[4] = { 1280.0f / 32.0f, 720.0f / 32.0f, 0, 0 };
			int nWidth, nHeight;
			pShaderAPI->GetBackBufferDimensions(nWidth, nHeight);
			vScreenScale[0] = (float)nWidth / 32.0f;
			vScreenScale[1] = (float)nHeight / 32.0f;
			pShaderAPI->SetPixelShaderConstant(31, vScreenScale, 1);
		}
		*/
	} // THIS } MUST BE BEFORE BEFORE pShader->Draw();

	if (mat_printvalues.GetBool())
	{
		mat_printvalues.SetValue(0);
	}
	
	pShader->Draw();
};

// WRD : Brainchild of Totterynine. Declare param declarations separately then call that from within shader declaration, makes it possible to easily run multiple shaders in one file
BEGIN_VS_SHADER(PBR, "PBR Shader for Brushes")

BEGIN_SHADER_PARAMS
LuxPBR_ParameterDeclaration()
END_SHADER_PARAMS

LuxPBR_Params()

SHADER_INIT_PARAMS()
{
	PBR_Vars_t vars;
	LuxPBR_Link_Params(vars);
	InitParams(this, params, pMaterialName, vars, IS_FLAG_SET(MATERIAL_VAR_MODEL));
}

SHADER_FALLBACK
{
	if (g_pHardwareConfig->GetDXSupportLevel() < 90)
	{
		Warning("Wait, that's Illegal \n Game run at DXLevel < 90");
		return "Wireframe";
	}
	return 0;
}

SHADER_INIT
{
	PBR_Vars_t vars;
	LuxPBR_Link_Params(vars);
	ShaderInit(this, params, vars, GetBoolParamValue(vars.m_bUseWVT), IS_FLAG_SET(MATERIAL_VAR_MODEL));
}

SHADER_DRAW
{
	PBR_Vars_t vars;
	LuxPBR_Link_Params(vars);

	bool bIsModel = IS_FLAG_SET(MATERIAL_VAR_MODEL);
	bool bIsBrush = !bIsModel; // If not a model, brush or displacement
	bool bIsWVT = bIsBrush && GetBoolParamValue(vars.m_bUseWVT);
	bool bIsOverride = false;
	bool bIsSSS = false;
	// Draw the Shader like usual.
	DrawGeneric(this, params, pShaderAPI, pShaderShadow, vars, vertexCompression, pContextDataPtr,
		bIsBrush, bIsModel, bIsWVT, bIsOverride, bIsSSS, bIsModel ? 2 : (bIsWVT ? 5 : 0), true, false); // 5 = Displacement MRAO, 0 = Brush MRAO, 2 is model mrao/spec



	//PBRSetupVars(vars);
#ifdef AGSCSM
		if ((!r_csm_enable_shaderside.GetBool() && pShaderShadow == NULL) || UsingFlashlight(params))
		{
			Draw(false);
		}
		else
		{
			FWD_Globallight_Draw_Internal(this, params, pShaderAPI, pShaderShadow, vertexCompression, pContextDataPtr, vars, true, bIsModel);
		}
		
#endif

	//if (bDrawLightingPass)
	//{
	//	Warning("3");
		
	//}
	//else
	//{
	//	Draw(false);
	//}
	
}
END_SHADER



BEGIN_VS_SHADER_FLAGS(PBR_Model, "", SHADER_NOT_EDITABLE)

BEGIN_SHADER_PARAMS
LuxPBR_ParameterDeclaration()
END_SHADER_PARAMS

LuxPBR_Params()

SHADER_INIT_PARAMS()
{
	PBR_Vars_t vars;
	LuxPBR_Link_Params(vars);
	InitParams(this, params, pMaterialName, vars, true);
}

SHADER_FALLBACK
{
	if (g_pHardwareConfig->GetDXSupportLevel() < 90)
	{
		Warning("Wait, that's Illegal \n Game run at DXLevel < 90");
		return "Wireframe";
	}
	return 0;
}

SHADER_INIT
{
	PBR_Vars_t vars;
	LuxPBR_Link_Params(vars);
	ShaderInit(this, params, vars, false, true);
}

SHADER_DRAW
{
	PBR_Vars_t vars;
	LuxPBR_Link_Params(vars);

	bool bIsWVT = false;
	bool bIsOverride = false;
	bool bIsSSS = false;

	// Draw the Shader like usual.
		DrawGeneric(this, params, pShaderAPI, pShaderShadow, vars, vertexCompression, pContextDataPtr,
			false, true, bIsWVT, bIsOverride, bIsSSS, 2, true, false); // 2 = model_mrao or specular
}
END_SHADER

BEGIN_VS_SHADER(PBR_OVERRIDE, "PBR Shader for Brushes and models and wvt...?")

BEGIN_SHADER_PARAMS
LuxPBR_ParameterDeclaration()
END_SHADER_PARAMS

LuxPBR_Params()

SHADER_INIT_PARAMS()
{
	PBR_Vars_t vars;
	LuxPBR_Link_Params(vars);
	InitParams(this, params, pMaterialName, vars);
}

SHADER_FALLBACK
{
	if (g_pHardwareConfig->GetDXSupportLevel() < 90)
	{
		Warning("Wait, that's Illegal \n Game run at DXLevel < 90");
		return "Wireframe";
	}
	return 0;
}

SHADER_INIT
{
	PBR_Vars_t vars;
	LuxPBR_Link_Params(vars);
	ShaderInit(this, params, vars);
}

SHADER_DRAW
{
	PBR_Vars_t vars;
	LuxPBR_Link_Params(vars);

	bool bIsModel		= (IS_FLAG_SET(MATERIAL_VAR_MODEL) != 0);
	bool bIsBrush		= !bIsModel; // inverse of model means brush
	bool bIsWVT			= false;
	bool bIsOverride	= true;
	bool bIsSSS			= false;
	// Draw the Shader like usual.
	DrawGeneric(this, params, pShaderAPI, pShaderShadow, vars, vertexCompression, pContextDataPtr,
		bIsBrush, bIsModel, bIsWVT, bIsOverride, bIsSSS, 4, true, false); // 4 = override
}
END_SHADER

#endif // #ifdef PHYSICALLY_BASED_RENDERING