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
#include "pbr_glass.h"

// We need all of these
#include "../CommonFunctions.h"
#include "../../cpp_shader_constant_register_map.h"

// this is required for our Function Array's
#include <functional>

// Includes for PS30
#include "pbr_mrao_glass_ps30.inc"

// Includes for VS30
#include "pbr_brush_vs30.inc"
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

#ifdef SDK2013MP
#ifdef DEBUG_LUXELS
static ConVar mat_luxels("mat_luxels", "0", FCVAR_CHEAT);
#endif
#endif

// Sampler List :
// s00 = BaseTexture
// s01 =
// s02 = NormalMap
// s03 =
// s04 = Vector Layer			( sRGB Color, AO )
// s05 =
// s06 = MRAO
// s07 =
// s08 = Properties Texture		( Wetness, Porosity, VectorMask, VectorRoughness )
// s09 =
// s10 =	
// s11 = Lightmap Sampler
// s12 =
// s13 =
// s14 =
// s15 = Cubemap

// COLOR is for brushes. COLOR2 for models
#define LuxPBR_Glass_Params() void LuxPBR_Link_Params(PBR_Glass_Vars_t &info)	\
{																	\
	info.m_nMultipurpose7			= MATERIALNAME;					\
	info.m_nIOR						= IOR;							\
	info.m_nRefraction				= REFRACTION;					\
	info.m_nBumpMap					= BUMPMAP;						\
	info.m_nBumpFrame				= BUMPFRAME;					\
	info.m_nBumpTransform			= BUMPTRANSFORM;				\
	info.m_nNormalTexture			= NORMALTEXTURE;				\
	info.m_nMRAOTexture				= MRAOTEXTURE;					\
	info.m_nSpecularTexture			= SPECULARTEXTURE;				\
	info.m_nBaseTextureTint			= BASETEXTURETINT;				\
	info.m_nMRAOBias				= MRAOBIAS;						\
	info.m_MRAOTextureFrame			= MRAOFRAME;					\
	info.m_nDiElectricCoefficient	= DIELECTRICCOEFFICIENT;		\
	info.m_nMicroShadows			= MICROSHADOWS;					\
	info.m_nWetnessPorosity			= WETNESSPOROSITY;				\
	info.m_nWetnessBias				= WETNESSBIAS;					\
	info.m_nPropertiesTexture		= PROPERTIESTEXTURE;			\
	info.m_nVectorLayerTexture		= VECTORLAYERTEXTURE;			\
	info.m_nLayerVector				= LAYERVECTOR;					\
	info.m_nLayerVectorSimilarity	= LAYERVECTORSIMILARITY;		\
																	\
	Link_GlobalParameters()											\
	Link_MiscParameters()											\
	Link_LightmappingParameters()									\
	Link_EnvironmentMapParameters()									\
	Link_ParallaxCorrectionParameters()								\
}//-----------------------------------------------------------------|

#define LuxPBR_GLASS_ParameterDeclaration()																																																			\
SHADER_PARAM(MATERIALNAME			,	SHADER_PARAM_TYPE_STRING	, "", "Can't access Material Name in the Drawing Function without this. Stores Materialname.")																				\
SHADER_PARAM(BUMPMAP				,	SHADER_PARAM_TYPE_TEXTURE	, "", "Bumpmap required for Lighting on Models. For model lightmaps use both $normaltexture and $bumpmap.")																	\
SHADER_PARAM(BUMPFRAME				,	SHADER_PARAM_TYPE_INTEGER	, "", "Frame Var")																																							\
SHADER_PARAM(BUMPTRANSFORM			,	SHADER_PARAM_TYPE_MATRIX	, "", "Transform Var")																																						\
SHADER_PARAM(NORMALTEXTURE			,	SHADER_PARAM_TYPE_TEXTURE	, "", "Internally used for Bumpmaps. $Bumpmap will get set automatically. Set this to the actual bumpmap preferably.")														\
SHADER_PARAM(MRAOTEXTURE			,	SHADER_PARAM_TYPE_TEXTURE	, "", "MRAO Texture. Red = Metallic, Green = Roughness, Blue = Ambient Occlusion, Alpha = Nothing yet")																		\
SHADER_PARAM(SPECULARTEXTURE		,	SHADER_PARAM_TYPE_TEXTURE	, "", "Specular Texture. Enables Specular/Glossiness behaviour, instead of MRAO. RGB = Specular ( linear range ), Alpha = Glossiness")										\
SHADER_PARAM(BASETEXTURETINT		,	SHADER_PARAM_TYPE_COLOR		, "", "Tint the Basetexture. $Color and $Color2 will ( probably ) not work. This is to be used above those two.")															\
SHADER_PARAM(MRAOBIAS				,	SHADER_PARAM_TYPE_VEC3		, "", "Bias MRAO [Roughness Metallic Ambient Occlusion]. This will add ontop of MRAO. -1.0f will remove, 1.0f will add. Similar to Levels")									\
SHADER_PARAM(MRAOFRAME				,	SHADER_PARAM_TYPE_INTEGER	, "", "Frame Var")																																							\
SHADER_PARAM(DIELECTRICCOEFFICIENT	,	SHADER_PARAM_TYPE_FLOAT		, "", "I cannot explain this. Look it up. Default Value is 0.025, however it usually ranges between 0.01 and 0.04. Essentially forces minimum reflection values!")			\
SHADER_PARAM(MICROSHADOWS			,	SHADER_PARAM_TYPE_FLOAT		, "", "Microshadows will occlude additional areas based on the light direction. 0.0f to 1.0f range. High values might be too strong.")										\
SHADER_PARAM(WETNESSPOROSITY		,	SHADER_PARAM_TYPE_BOOL		, "", "Enables Wetness Porosity Behaviour. Requires $PropertiesTexture!!"				)																					\
SHADER_PARAM(WETNESSBIAS			,	SHADER_PARAM_TYPE_FLOAT		, "", "Bias amount of Wetness ( set to -1 then add on top to slowly increase wetness. )")																					\
SHADER_PARAM(PROPERTIESTEXTURE		,	SHADER_PARAM_TYPE_TEXTURE	, "", "R = Wetness, G = Porosity, B = VecLayerMask, A = VecLayer Roughness")																								\
SHADER_PARAM(VECTORLAYERTEXTURE		,	SHADER_PARAM_TYPE_TEXTURE	, "", "The Texture to use for applying on a Vector. Incompatible with $Detail."			)																					\
SHADER_PARAM(LAYERVECTOR			,	SHADER_PARAM_TYPE_VEC3		, "", "The Normal Vector on which the Texture can appear on. [0 0 1] for upwards faces.")																					\
SHADER_PARAM(LAYERVECTORSIMILARITY	,	SHADER_PARAM_TYPE_FLOAT		, "", "Similarity that $LayerVector and Face Normals need to have. 0.3f is the default.")																					\
SHADER_PARAM(IOR					,	SHADER_PARAM_TYPE_FLOAT		, "", "IOR of the Glass Material.")\
SHADER_PARAM(REFRACTION				,	SHADER_PARAM_TYPE_BOOL		, "", "Enable Refractive Behaviour")\
\
Declare_DisplacementParameters()																																																				\
Declare_MiscParameters()																																																						\
Declare_DetailTextureParameters()																																																				\
Declare_LightmappingParameters()																																																				\
Declare_EnvironmentMapParameters()																																																				\
Declare_ParallaxCorrectionParameters()																																																			\
Declare_EnvMapMaskParameters()//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|




//==================================================================================================
// Initiate Parameters, mainly making sure something is set for normal maps and cubemaps
//==================================================================================================
void InitParams(CBaseVSShader *pShader, IMaterialVar **params, const char *pMaterialName, PBR_Glass_Vars_t &info, bool bIsModel = false)
{
	// I wanted to debug print some values and needed to know which material they belong to. However pMaterialName is not available to the draw!
	// Well, now it is...
	params[info.m_nMultipurpose7]->SetStringValue(pMaterialName);

	// The usual...
	Flashlight_BorderColorSupportCheck();

	// PBR relies heavily on envmaps
	if (!params[info.m_nEnvMap]->IsDefined())
		params[info.m_nEnvMap]->SetStringValue("env_cubemap");

	//////////////////////////////
	//		GIGA IMPORTANT		//
	//////////////////////////////
	//	Models that want to use Model Lightmapping with Bump Maps previously used $NormalTexture
	//	This is because the Engine checks for $BumpMap ( and $Phong ) to determine whether or not a model should be able to use Model Lightmaps.
	//	Originally we just always Set m_nBumpMap to undefined after settings its StringValue to m_nNormalTexture.
	//	Later we figured out that in order to get Lighting Information for Static Lightsources ( Non-Named Lights, aka those without TargetName )
	//	$BumpMap needs to have some kind of StringValue on it. Yeah guess what the engine checks for...
	//	The damage was already done ( On HDTF for example ) I didn't want to tell everyone to change their vmt's for a feature they don't use.
	//	So now we force set $bumpmap. However to retain Model Lightmapping functionality, models that use
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
	FloatParameterDefault(info.m_nEnvMapSaturation, 1.0f)

		// Default Value is supposed to be 0.025f
		FloatParameterDefault(info.m_nDiElectricCoefficient, 0.025f)

		FloatParameterDefault(info.m_nLayerVectorSimilarity, 0.3f)

		// If in decal mode, no debug override...
		/*
		if (IS_FLAG_SET(MATERIAL_VAR_DECAL) && !bIsModel) // TODO: also on models?
		{
		SET_FLAGS(MATERIAL_VAR_NO_DEBUG_OVERRIDE);
		}
		*/

		if (bIsModel)
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
			//		SET_FLAGS2(MATERIAL_VAR2_NEEDS_TANGENT_SPACES);
			SET_FLAGS2(MATERIAL_VAR2_USES_ENV_CUBEMAP);
			SET_FLAGS2(MATERIAL_VAR2_SUPPORTS_FLASHLIGHT);              // "Required for flashlight"
			SET_FLAGS2(MATERIAL_VAR2_USE_FLASHLIGHT);                   // "Required for flashlight"
		}
		// Always needed on both
//	SET_FLAGS2(MATERIAL_VAR2_NEEDS_POWER_OF_TWO_FRAME_BUFFER_TEXTURE);
	SET_FLAGS2(MATERIAL_VAR2_NEEDS_FULL_FRAME_BUFFER_TEXTURE);
	SET_FLAGS(MATERIAL_VAR_TRANSLUCENT);
}

//==================================================================================================
// Load Textures
//==================================================================================================
void ShaderInit(CBaseVSShader *pShader, IMaterialVar **params, PBR_Glass_Vars_t &info, bool bIsWVT = false, bool bIsModel = false)
{
	pShader->LoadTexture(info.m_nFlashlightTexture, TEXTUREFLAGS_SRGB);
	LoadTextureWithCheck(info.m_nBaseTexture, TEXTUREFLAGS_SRGB)
	LoadNormalTexture(info.m_nNormalTexture)

	// Adding this flag probably does not work...
	pShader->LoadCubeMap(info.m_nEnvMap, TEXTUREFLAGS_ALL_MIPS);

	LoadTextureWithCheck(info.m_nSpecularTexture, 0)
	else
	{
		LoadTextureWithCheck(info.m_nMRAOTexture, 0)
	}

	LoadTextureWithCheck(info.m_nVectorLayerTexture, 0);
	LoadTextureWithCheck(info.m_nPropertiesTexture, 0);
	LoadTextureWithCheck(info.m_nLightMap, 0)
};

//==================================================================================================
// Determine, well, basically everything. Declare Shaders and Set PSREG's.
//==================================================================================================
void DrawGeneric(CBaseVSShader *pShader, IMaterialVar **params, IShaderDynamicAPI *pShaderAPI, IShaderShadow *pShaderShadow, PBR_Glass_Vars_t &info, VertexCompressionType_t vertexCompression, CBasePerMaterialContextData **pContextDataPtr)
{
	// iShaderNumber is used to determine which shader to use down the line, that way, we only need to evaluate some things once instead of multiple times.
	// This will literally save us like 13 If-Statements or so, and using a lookup table is always faster than if statements.
	// 1 = brush_mrao, 2 = brush_spec, 3 = model_mrao, 4 = model_spec, 5 = override, 6 = wvt_mrao, 7 = wvt_spec, 8 = sss_mrao, 9 = sss_spec
	// int iShaderNumber = 0;

	//==================================================================================================
	// Setting up booleans
	//==================================================================================================
	// Flags
	bool bHasFlashlight = pShader->UsingFlashlight(params);
	bool bIsModel = IS_FLAG_SET(MATERIAL_VAR_MODEL);
//	bool bSelfIllum = !bHasFlashlight		&&	IS_FLAG_SET(MATERIAL_VAR_SELFILLUM); // No SelfIllum under the flashlight
	bool bAlphatest = IS_FLAG_SET(MATERIAL_VAR_ALPHATEST);
//	bool bNormalMapAlphaEnvMapMask = !bHasFlashlight		&&	IS_FLAG_SET(MATERIAL_VAR_NORMALMAPALPHAENVMAPMASK); // No Envmapping under the flashlight
//	bool bBaseAlphaEnvMapMask = !bHasFlashlight		&&	IS_FLAG_SET(MATERIAL_VAR_BASEALPHAENVMAPMASK); // No Envmapping under the flashlight
	bool bIsDecal = IS_FLAG_SET(MATERIAL_VAR_DECAL);
	bool bHasBaseTexture = IsTextureLoaded(info.m_nBaseTexture);
	bool bHasNormalTexture = IsTextureLoaded(info.m_nNormalTexture);
	bool bHasMRAOTexture = IsTextureLoaded(info.m_nMRAOTexture); // This bool has to exist. Binding an int without anything assigned might crash the game.
	bool bHasSpecularTexture = bHasMRAOTexture		&&	IsTextureLoaded(info.m_nSpecularTexture);
	bool bHasEnvMap = !bHasFlashlight		&&	IsTextureLoaded(info.m_nEnvMap); // This bool has to exist. Binding an int without anything assigned might crash the game.							
	bool bLightMappedModel = bIsModel			&&	IsTextureLoaded(info.m_nLightMap);
//	bool bHasLightMapUVs = bIsModel			&&	GetBoolParamValue(info.m_nLightMapUVs);
	bool bHasParallaxCorrection = bHasEnvMap			&&	GetBoolParamValue(info.m_nEnvMapParallax);
	bool bHasPropertiesTexture = IsTextureLoaded(info.m_nPropertiesTexture);
	bool bHasVectorLayer = bHasPropertiesTexture &&	IsTextureLoaded(info.m_nVectorLayerTexture);
	bool bRequiresProperties = bHasPropertiesTexture; // || bHasVectorLayer && !bHasDetailTexture; // Not supported in combination with Detail.

	// Needed for things I do not fully understand right now
	bool IsOpaque(bIsFullyOpaque, info.m_nBaseTexture, bAlphatest);
	bIsFullyOpaque = bIsFullyOpaque; // tiny hack for true transparency

#ifdef DEBUG_MRAO
	int i_mat_mrao = mat_mrao.GetInt();

	// really for debugging only...
	if (i_mat_mrao != 0)
	{
//		iShaderNumber = 5; // Is this the correct number?
	}
#endif

	bool bHasBumpTextureTransform = params[info.m_nBumpTransform]->MatrixIsIdentity();

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
		EnableSampler(SHADER_SAMPLER1, pBaseTexture->GetFlags() & TEXTUREFLAGS_SRGB ? true : false)
			EnableSampler(SAMPLER_NORMALTEXTURE, false)
			EnableSampler(SHADER_SAMPLER0, true) // ?? Is this correct?? Is Framebuffer sRGB?
			EnableSampler(SHADER_SAMPLER6, false) // MRAOS/SPECULAR Sampler
			EnableFlashlightSamplers(bHasFlashlight, SAMPLER_SHADOWDEPTH, SAMPLER_RANDOMROTATION, SAMPLER_FLASHLIGHTCOOKIE, info.m_nBaseTexture)
			EnableSamplerWithCheck(bHasEnvMap, SHADER_SAMPLER15, g_pHardwareConfig->GetHDRType() == HDR_TYPE_NONE ? true : false) // H++ only runs LDR.


			// H++ only runs LDR. We check for Brushes too in case we ever get model lightmapping preview...
			EnableSampler(SAMPLER_LIGHTMAP, (g_pHardwareConfig->GetHDRType() == HDR_TYPE_NONE && !bIsModel) ? true : false)

				if (bRequiresProperties) // Vector Layer + Wetness Porosity
				{
					EnableSampler(SAMPLER_DETAILTEXTURE, true) // Yes. Always please. VectorLayer Texture.
						EnableSampler(SHADER_SAMPLER8, false) // Properties Sampler
						EnableSampler(SHADER_SAMPLER9, false)
						EnableSampler(SHADER_SAMPLER10, true)
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
		}
		else
		{
			GetVertexShaderFormat_Brush()
		}

		// Shaders below ps20b had to use a Gamma Lookup table to convert Linear to sRGB. This is done on the pixelshaders now.
		pShaderShadow->EnableSRGBWrite(true);

		//==================================================================================================
		// Declare Static Shaders ( lookup table ), value of iShaderNumber evaluated before Snapshotting
		//==================================================================================================
		// Required for Morphing
		if (bIsModel && bFastVertexTextures)
			SET_FLAGS2(MATERIAL_VAR2_USES_VERTEXID); // Use the Vertex ID Stream

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
		
		DECLARE_STATIC_PIXEL_SHADER(pbr_mrao_glass_ps30);
		SET_STATIC_PIXEL_SHADER_COMBO(FLASHLIGHT, bHasFlashlight);
		SET_STATIC_PIXEL_SHADER_COMBO(SPECIALPROPERTIES, bRequiresProperties);
		SET_STATIC_PIXEL_SHADER_COMBO(REFRACTION, GetBoolParamValue(info.m_nRefraction));
		SET_STATIC_PIXEL_SHADER_COMBO(BRUSH, !bIsModel);
		SET_STATIC_PIXEL_SHADER(pbr_mrao_glass_ps30);

		/*
		auto DeclareStatic = DeclareStatics[iShaderNumber];

		DeclareStatic(pShaderShadow, bHasDetailTexture, (bIsOverride && bHasSpecularTexture), bHasLightMapUVs, bHasBlendModulateTexture,
			bHasFlashlight, (bHasEmissionTexture || bHasEmissionTexture2), bHasParallaxMapping, bHasParallaxCorrection,
			bHasSheen, bHasHullShell, bHasAnyWrinkleMapping, iEnvMapMode,
			iSelfIllumMode, IS_FLAG_SET(MATERIAL_VAR_DECAL), bIsModel, bHasBentNormal, bHasSSSTexture, bRequiresProperties, bHasSeamless);
*/
		//	Same thing, but manual
		//	std::function<void(IShaderShadow*, bool)> DeclareStatic = DeclareStatics[iCurrentShader];
	}
	else // end of snapshotting
	{
		// Getting the light state
		LightState_t LightState;
		pShaderAPI->GetDX9LightState(&LightState);
		int iNumLights = LightState.m_nNumLights;

		//===========================================================================//
		// Bind Textures
		//===========================================================================//
		BindTextureWithCheckAndFallback(bHasBaseTexture, SHADER_SAMPLER1, info.m_nBaseTexture, info.m_nBaseTextureFrame, TEXTURE_WHITE)
		if (bHasBaseTexture)
		{
		}
			// Basically, "what is behind the current pixel" lookup texture at this stage.
			pShaderAPI->BindStandardTexture(SHADER_SAMPLER0, TEXTURE_FRAME_BUFFER_FULL_TEXTURE_0);

		// if mat_fullbright 2. Bind a standard white texture...
#ifdef DEBUG_FULLBRIGHT2 
		if (mat_fullbright.GetInt() == 2 && !IS_FLAG_SET(MATERIAL_VAR_NO_DEBUG_OVERRIDE))
			BindTextureStandard(SHADER_SAMPLER1, TEXTURE_GREY)
#endif

#ifdef CUBEMAPS
			if (bHasEnvMap)
			{
				BindTextureWithCheckAndFallback(mat_specular.GetBool(), SAMPLER_ENVMAPTEXTURE, info.m_nEnvMap, info.m_nEnvMapFrame, TEXTURE_BLACK)
			}
#endif

		BindTextureWithCheckAndFallback(bHasNormalTexture, SAMPLER_NORMALTEXTURE, info.m_nNormalTexture, info.m_nBumpFrame, TEXTURE_NORMALMAP_FLAT)
			BindTextureWithCheck(bHasMRAOTexture, SHADER_SAMPLER6, info.m_nMRAOTexture, info.m_MRAOTextureFrame)
			BindTextureWithCheck(bHasSpecularTexture, SHADER_SAMPLER6, info.m_nSpecularTexture, info.m_MRAOTextureFrame)

			if (bRequiresProperties)
			{
				// VectorLayer / Wetness.Porosity.VectorMask.VectorRoughness
				BindTextureWithCheck(bRequiresProperties, SHADER_SAMPLER8, info.m_nPropertiesTexture, 0)
					BindTextureWithCheck(bHasVectorLayer, SHADER_SAMPLER4, info.m_nVectorLayerTexture, 0)
			}

		if (bIsModel)
		{
			if (bLightMappedModel)
			{
				BindTextureWithoutCheck(SAMPLER_LIGHTMAP, info.m_nLightMap, 0);
			}
		}
		else // else its a brush or displacement
		{
			BindTextureStandard(SAMPLER_LIGHTMAP, TEXTURE_LIGHTMAP_BUMPED); // 

			// This used to be below. However, we check for bIsBrush here anyways... Why would we do it again?
			// TODO: Supposedly brushes don't receive either... so why does this even exist?
			LightState.m_bAmbientLight = false;
			LightState.m_nNumLights = 0;
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
			f4Empty(f4EmissionTint_WetnessBias)
			f4Empty(f4EnvMapTint_LightScale)
			f4Empty(f4EnvMapSaturation_Contrast)
			f4Empty(f4DetailBlendMode_Empty) // yzw empty
			f4Empty(f4ParallaxControls)
			f4Empty(f4MRAOControls)
			f4Empty(f4VariousControls1)
			f4Empty(f4VectorLayer_Similarity)
			BOOL BBools[16] = { false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false };

		// Lets start with vec3's
		GetVec3ParamValue(info.m_nBaseTextureTint, f4BaseTextureTint_DECE);
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
		}

		// Floats
		f4BaseTextureTint_DECE[3] = GetFloatParamValue(info.m_nDiElectricCoefficient);
		f4EnvMapTint_LightScale[3] = GetFloatParamValue(info.m_nEnvMapLightScale);
		f4EnvMapSaturation_Contrast[3] = GetFloatParamValue(info.m_nEnvMapContrast);

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
		pShaderAPI->SetPixelShaderConstant(33, f4VectorLayer_Similarity, 1);
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
		int w = 0;
		int h = 0;
		pShaderAPI->GetBackBufferDimensions(w, h);
		f4ParallaxControls[0] = w;
		f4ParallaxControls[1] = h;

		pShaderAPI->SetPixelShaderConstant(50, f4ParallaxControls, 1);
		pShaderAPI->SetPixelShaderConstant(51, f4MRAOControls, 1);
		//				pShaderAPI->SetPixelShaderConstant(52, f4SSSTint_Empty, 1);
		//				pShaderAPI->SetPixelShaderConstant(53, f4SSSControls, 1);

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
			Warning("bRequiresProperties : %d \n bHasVectorLayer : %d \n", bRequiresProperties, bHasVectorLayer);
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
		f4VariousControls1[0] = GetFloatParamValue(info.m_nIOR);
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

		if (bIsModel)
		{
			DECLARE_DYNAMIC_VERTEX_SHADER(pbr_model_vs30);
			SET_DYNAMIC_VERTEX_SHADER_COMBO(SKINNING, pShaderAPI->GetCurrentNumBones() > 0 ? true : false);
			SET_DYNAMIC_VERTEX_SHADER_COMBO(MORPHING, pShaderAPI->IsHWMorphingEnabled() && g_pHardwareConfig->HasFastVertexTextures());
			SET_DYNAMIC_VERTEX_SHADER(pbr_model_vs30);
		}
		else
		{
			DECLARE_DYNAMIC_VERTEX_SHADER(pbr_brush_vs30);
			SET_DYNAMIC_VERTEX_SHADER(pbr_brush_vs30);
		}
		DECLARE_DYNAMIC_PIXEL_SHADER(pbr_mrao_glass_ps30);
		SET_DYNAMIC_PIXEL_SHADER_COMBO(NUM_LIGHTS, iNumLights);
		SET_DYNAMIC_PIXEL_SHADER_COMBO(WRITEWATERFOGTODESTALPHA, bWriteWaterFogToAlpha);
		SET_DYNAMIC_PIXEL_SHADER_COMBO(WRITE_DEPTH_TO_DESTALPHA, bWriteDepthToAlpha);
		SET_DYNAMIC_PIXEL_SHADER_COMBO(PIXELFOGTYPE, pShaderAPI->GetPixelFogCombo());
		SET_DYNAMIC_PIXEL_SHADER_COMBO(FLASHLIGHTSHADOWS, bFlashlightShadows);
		SET_DYNAMIC_PIXEL_SHADER_COMBO(LIGHTMAPPED_MODEL, bLightMappedModel);
		SET_DYNAMIC_PIXEL_SHADER_COMBO(CASCADED_SHADOW, false);
		SET_DYNAMIC_PIXEL_SHADER(pbr_mrao_glass_ps30);

		DECLARE_DYNAMIC_VERTEX_SHADER(pbr_brush_vs30);
		SET_DYNAMIC_VERTEX_SHADER(pbr_brush_vs30);

//		auto DeclareDynamic = DeclareDynamics[iShaderNumber];

//		DeclareDynamic(pShaderAPI,
//			bWriteWaterFogToAlpha, bWriteDepthToAlpha, (pShaderAPI->GetPixelFogCombo()), bFlashlightShadows,
//			false, (pShaderAPI->GetCurrentNumBones() > 0 ? true : false), pShaderAPI->IsHWMorphingEnabled() && g_pHardwareConfig->HasFastVertexTextures(), false, iNumLights, bIsModel);

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
BEGIN_VS_SHADER(PBR_TRANSPARENT, "PBR Shader for Brushes")

BEGIN_SHADER_PARAMS
LuxPBR_GLASS_ParameterDeclaration()
END_SHADER_PARAMS

LuxPBR_Glass_Params()

SHADER_INIT_PARAMS()
{
	PBR_Glass_Vars_t vars;
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
	PBR_Glass_Vars_t vars;
	LuxPBR_Link_Params(vars);
	ShaderInit(this, params, vars, false, IS_FLAG_SET(MATERIAL_VAR_MODEL));
}

SHADER_DRAW
{
	PBR_Glass_Vars_t vars;
	LuxPBR_Link_Params(vars);

 	// Draw the Shader like usual.
	DrawGeneric(this, params, pShaderAPI, pShaderShadow, vars, vertexCompression, pContextDataPtr);

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

#endif
