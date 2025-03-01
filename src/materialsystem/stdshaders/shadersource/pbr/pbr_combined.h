//========= ShiroDkxtro2's --ACROHS Ultimate Shaders Project-- ============//
//
//	Description :	What is this?
//					PBR. Look it up!
//
//	Initial D.	:	25.09.2022 DMY
//	Last Change :	05.04.2023 DMY
//
//	Purpose of this File :	PBR for Brushes, Models and displacements.
//
//===========================================================================//

// All Shaders use the same struct and sampler definitions. This way multipass CSM doesn't need to re-setup the struct
#include "../../ACROHS_Shared.h"

//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------
class CBaseVSShader;
class IMaterialVar;
class IShaderDynamicAPI;
class IShaderShadow;

struct PBR_Vars_t
{
	PBR_Vars_t() { memset(this, 0xFF, sizeof(*this)); }

	Declare_GloballyDefinedStructVars()
	Declare_DisplacementStructVars()
	// Not using the Declare function because it has some unused things.
	int m_nBumpMap;
	int m_nBumpFrame;
	int m_nBumpTransform;
	int m_nNormalTexture;
	Declare_MiscStructVars()
	Declare_DetailTextureStructVars()

#ifdef MODEL_LIGHTMAPPING
	Declare_LightmappingStructVars()
#endif

	Declare_EnvironmentMapStructVars()
#ifdef PARALLAXCORRECTEDCUBEMAPS
	Declare_ParallaxCorrectionStructVars()
#endif

	// Required for PBR_Override
	Declare_EnvMapMaskStructVars()
	int m_nSelfIllum_EnvMapMask_Alpha;

	// All texture maps
	int m_nMRAOTexture;
	int m_nEmissionTexture;
	int m_nSpecularTexture;		// I hate Roman
	
	// Tint, Tint everything
	int m_nBaseTextureTint;
	int m_nBaseTextureTint2;
	int m_nMRAOBias;
	int m_nMRAO2Bias;
	int m_nEmissionTint;
	
	// Texture Frames
	int m_MRAOTextureFrame;

	// Additionally required things for Displacements.
	int m_nNormalTexture2;
	int m_nMRAOTexture2;
	int m_nEmissionTexture2;
	int m_nSpecularTexture2;

	// THE Frostbite SSS
	// Experimental Feature
	int m_nSSSTexture;
	int m_nSSSTint;
	int m_nSSSScale;
	int m_nSSSPower;
	int m_nSSSEmission;		
	int m_nSSSIntensity;

	// Parallax Interval Mapping related
	int m_bHasParallax;
	int m_nParallaxHeight;
	int m_nParallaxMaxOffset;
	int m_nParallaxIntensity;
	int m_nParallaxInterval;

	// Overwrite for Dielectric coefficient. Default is 0.04
	int m_nDiElectricCoefficient;

	int m_nSheen;
	int m_nSheenIntensity;
	int m_nHullShell;
	int m_nAlphaBlending;

	// Stock Skin Shader stuff
	int m_nWrinkleCompress;
	int m_nWrinkleStretch;
	int m_nWrinkleCompressBump;
	int m_nWrinkleStretchBump;

	int m_nMicroShadows;
	int m_nBentNormal;

	int m_nWetnessPorosity;
	int m_nWetnessBias;
	int m_nPropertiesTexture;
	int m_nVectorLayerTexture;
	int m_nLayerVector;
	int m_nLayerVectorSimilarity;
	int m_nLayerVector2;
	int m_nLayerVectorSimilarity2;

	// WVT stuff
	int m_nWetnessBias2;
	int m_nPropertiesTexture2;
	int m_nVectorLayerTexture2;
	int m_nSeamless;
	int m_nSeamlessScale;
	int m_nSeamlessSecondary;
	
	int m_bUseWVT; // Used Internally...
};