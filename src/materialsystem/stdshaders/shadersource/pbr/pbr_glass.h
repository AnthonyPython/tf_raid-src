//========= ShiroDkxtro2's --ACROHS Ultimate Shaders Project-- ============//
//
//	Description :	What is this?
//					PBR. Look it up!
//
//	Initial D.	:	05.04.2023 DMY
//	Last Change :	05.04.2023 DMY
//
//	Purpose of this File :	Physically Based Glass rendering for models and brushes
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

struct PBR_Glass_Vars_t
{
	PBR_Glass_Vars_t() { memset(this, 0xFF, sizeof(*this)); }

	Declare_GloballyDefinedStructVars()

	// Not using the Declare function because it has some unused things.
	int m_nBumpMap;
	int m_nBumpFrame;
	int m_nBumpTransform;
	int m_nNormalTexture;
	Declare_MiscStructVars()

#ifdef MODEL_LIGHTMAPPING
	Declare_LightmappingStructVars()
#endif

	Declare_EnvironmentMapStructVars()
#ifdef PARALLAXCORRECTEDCUBEMAPS
		Declare_ParallaxCorrectionStructVars()
#endif

	// All texture maps
	int m_nMRAOTexture;
	int m_nSpecularTexture;		// I hate Roman

	// Tint, Tint everything
	int m_nBaseTextureTint;
	int m_nMRAOBias;

	// Texture Frames
	int m_MRAOTextureFrame;

	// Overwrite for Dielectric coefficient. Default is 0.04
	int m_nDiElectricCoefficient;
	
	int m_nMicroShadows;
	
	int m_nWetnessPorosity;
	int m_nWetnessBias;
	int m_nPropertiesTexture;
	int m_nVectorLayerTexture;
	int m_nLayerVector;
	int m_nLayerVectorSimilarity;
	int m_nIOR;
	int m_nRefraction;
};