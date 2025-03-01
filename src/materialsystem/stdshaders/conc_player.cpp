//================================================//
//
// Purpose: concussion studios player material shader
// 
// Last modified: Tholp - Oct 4 2022
//
// $Header: $
// $NoKeywords: $
//=================================================//

#include "BaseVSShader.h"
#include "conc_player_helper.h"
#include "emissive_scroll_blended_pass_helper.h"
#include "cloak_blended_pass_helper.h"
#include "flesh_interior_blended_pass_helper.h"
#include "include/conc_player_ps20b.inc"
#include "include/pbr_vs20.inc"//use the pbr vs for this
#include "pbr_dx9_helper.h"
#include "cpp_shader_constant_register_map.h"

BEGIN_VS_SHADER(Conc_Player, "")
DEFINE_FALLBACK_SHADER(Conc_Player, SDK_VertexLitGeneric)
BEGIN_SHADER_PARAMS
		//Player color Pass
		SHADER_PARAM(PLAYERCOLORMASK, SHADER_PARAM_TYPE_TEXTURE, "dev/black", "")
		SHADER_PARAM(PLAYERCOLOR1, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "")
		SHADER_PARAM(PLAYERCOLOR2, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "")
		SHADER_PARAM(PLAYERCOLOR3, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "")
		// Emissive Scroll Pass
		SHADER_PARAM(EMISSIVEBLENDENABLED, SHADER_PARAM_TYPE_BOOL, "0", "Enable emissive blend pass")
		SHADER_PARAM(EMISSIVEBLENDBASETEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "self-illumination map")
		SHADER_PARAM(EMISSIVEBLENDSCROLLVECTOR, SHADER_PARAM_TYPE_VEC2, "[0.11 0.124]", "Emissive scroll vec")
		SHADER_PARAM(EMISSIVEBLENDSTRENGTH, SHADER_PARAM_TYPE_FLOAT, "1.0", "Emissive blend strength")
		SHADER_PARAM(EMISSIVEBLENDTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "self-illumination map")
		SHADER_PARAM(EMISSIVEBLENDTINT, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "Self-illumination tint")
		SHADER_PARAM(EMISSIVEBLENDFLOWTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "flow map")
		SHADER_PARAM(TIME, SHADER_PARAM_TYPE_FLOAT, "0.0", "Needs CurrentTime Proxy")

		// Cloak Pass
		SHADER_PARAM(CLOAKPASSENABLED, SHADER_PARAM_TYPE_BOOL, "0", "Enables cloak render in a second pass")
		SHADER_PARAM(CLOAKFACTOR, SHADER_PARAM_TYPE_FLOAT, "0.0", "")
		SHADER_PARAM(CLOAKCOLORTINT, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "Cloak color tint")
		SHADER_PARAM(REFRACTAMOUNT, SHADER_PARAM_TYPE_FLOAT, "2", "")


		//misc
		SHADER_PARAM(BUMPMAP, SHADER_PARAM_TYPE_TEXTURE, "models/shadertest/shader1_normal", "bump map")
		SHADER_PARAM(BUMPFRAME, SHADER_PARAM_TYPE_INTEGER, "0", "frame number for $bumpmap")
		SHADER_PARAM(BUMPTRANSFORM, SHADER_PARAM_TYPE_MATRIX, "center .5 .5 scale 1 1 rotate 0 translate 0 0", "$bumpmap texcoord transform")

		//PBR
		SHADER_PARAM(ALPHATESTREFERENCE, SHADER_PARAM_TYPE_FLOAT, "0", "");
		SHADER_PARAM(ENVMAP, SHADER_PARAM_TYPE_ENVMAP, "", "Set the cubemap for this material.");
		SHADER_PARAM(MRAOTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "Texture with metalness in R, roughness in G, ambient occlusion in B, HSV blend in A.");	
		SHADER_PARAM(EMISSIONTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "Emission texture");		
		SHADER_PARAM(NORMALTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "Normal texture (deprecated, use $bumpmap)");		
		SHADER_PARAM(PARALLAX, SHADER_PARAM_TYPE_BOOL, "0", "Use Parallax Occlusion Mapping.");
		SHADER_PARAM(PARALLAXDEPTH, SHADER_PARAM_TYPE_FLOAT, "0.0030", "Depth of the Parallax Map");
		SHADER_PARAM(PARALLAXCENTER, SHADER_PARAM_TYPE_FLOAT, "0.5", "Center depth of the Parallax Map");
		SHADER_PARAM(MRAOSCALE, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "Factors for metalness, roughness, and ambient occlusion");		
		SHADER_PARAM(EMISSIONSCALE, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "Color to multiply emission texture with");
		SHADER_PARAM(HSV, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "HSV color to transform $basetexture texture with");
		SHADER_PARAM(HSV_BLEND, SHADER_PARAM_TYPE_BOOL, "0", "Blend untransformed color and HSV transformed color");
		END_SHADER_PARAMS

	void Setup_Conc_Player(IMaterialVar** params, Conc_Player_Vars_t& info)
	{
		info.m_nBaseTexture = BASETEXTURE;

		info.m_nPlayerColorMask = PLAYERCOLORMASK;
		info.m_nPlayerColor1 = PLAYERCOLOR1;
		info.m_nPlayerColor2 = PLAYERCOLOR2;
		info.m_nPlayerColor3 = PLAYERCOLOR3;

		info.baseColor = IS_FLAG_SET(MATERIAL_VAR_MODEL) ? COLOR2 : COLOR;
		info.normalTexture = NORMALTEXTURE;
		info.bumpMap = BUMPMAP;
		info.baseTextureFrame = FRAME;
		info.baseTextureTransform = BASETEXTURETRANSFORM;
		info.alphaTestReference = ALPHATESTREFERENCE;
		info.flashlightTexture = FLASHLIGHTTEXTURE;
		info.flashlightTextureFrame = FLASHLIGHTTEXTUREFRAME;
		info.envMap = ENVMAP;
		info.emissionTexture = EMISSIONTEXTURE;
		info.mraoTexture = MRAOTEXTURE;
		info.useParallax = PARALLAX;
		info.parallaxDepth = PARALLAXDEPTH;
		info.parallaxCenter = PARALLAXCENTER;
		info.mraoScale = MRAOSCALE;
		info.emissionScale = EMISSIONSCALE;
		info.hsv = HSV;
		info.hsv_blend = HSV_BLEND;

	}
	
	// Emissive Scroll Pass
	void SetupVarsEmissiveScrollBlendedPass(EmissiveScrollBlendedPassVars_t& info)
	{
		info.m_nBlendStrength = EMISSIVEBLENDSTRENGTH;
		info.m_nBaseTexture = EMISSIVEBLENDBASETEXTURE;
		info.m_nFlowTexture = EMISSIVEBLENDFLOWTEXTURE;
		info.m_nEmissiveTexture = EMISSIVEBLENDTEXTURE;
		info.m_nEmissiveTint = EMISSIVEBLENDTINT;
		info.m_nEmissiveScrollVector = EMISSIVEBLENDSCROLLVECTOR;
		info.m_nTime = TIME;
	}

	// Cloak Pass
	void SetupVarsCloakBlendedPass(CloakBlendedPassVars_t& info)
	{
		info.m_nCloakFactor = CLOAKFACTOR;
		info.m_nCloakColorTint = CLOAKCOLORTINT;
		info.m_nRefractAmount = REFRACTAMOUNT;

		// Delete these lines if not bump mapping!
		info.m_nBumpmap = BUMPMAP;
		info.m_nBumpFrame = BUMPFRAME;
		info.m_nBumpTransform = BUMPTRANSFORM;
	}
	SHADER_INIT_PARAMS ()
	{
		if (!params[PLAYERCOLOR1]->IsDefined())
			params[PLAYERCOLOR1]->SetVecValue(1, 1, 1, 1);

		if (!params[PLAYERCOLOR2]->IsDefined())
			params[PLAYERCOLOR2]->SetVecValue(1, 1, 1, 1);

		if (!params[PLAYERCOLOR3]->IsDefined())
			params[PLAYERCOLOR3]->SetVecValue(1, 1, 1, 1);

		if (!params[PLAYERCOLORMASK]->IsDefined())
			params[PLAYERCOLORMASK]->SetStringValue("dev/black");

		Conc_Player_Vars_t vars;
		Setup_Conc_Player(params, vars);

		// Cloak Pass
		if (!params[CLOAKPASSENABLED]->IsDefined())
		{
			params[CLOAKPASSENABLED]->SetIntValue(0);
		}
		else if (params[CLOAKPASSENABLED]->GetIntValue())
		{
			CloakBlendedPassVars_t info;
			SetupVarsCloakBlendedPass(info);
			InitParamsCloakBlendedPass(this, params, pMaterialName, info);
		}

		// Emissive Scroll Pass
		if (!params[EMISSIVEBLENDENABLED]->IsDefined())
		{
			params[EMISSIVEBLENDENABLED]->SetIntValue(0);
		}
		else if (params[EMISSIVEBLENDENABLED]->GetIntValue())
		{
			EmissiveScrollBlendedPassVars_t info;
			SetupVarsEmissiveScrollBlendedPass(info);
			InitParamsEmissiveScrollBlendedPass(this, params, pMaterialName, info);
		}

		if (!params[BUMPMAP]->IsDefined())
			params[BUMPMAP]->SetStringValue("dev/flat_normal");

		// Set a good default mrao texture
		if (!params[MRAOTEXTURE]->IsDefined())
			params[MRAOTEXTURE]->SetStringValue("dev/pbr_mraotexture");

		// PBR relies heavily on envmaps
		if (!params[ENVMAP]->IsDefined())
			params[ENVMAP]->SetStringValue("env_cubemap");

	}
	
	SHADER_INIT
	{
		Conc_Player_Vars_t info;
		Setup_Conc_Player(params, info);

		if (params[info.m_nBaseTexture]->IsDefined())
		{
			LoadTexture(info.m_nBaseTexture, TEXTUREFLAGS_SRGB);
		}

		if (params[PLAYERCOLORMASK]->IsDefined())
			LoadTexture(PLAYERCOLORMASK);

		Assert(info.bumpMap >= 0);
		LoadBumpMap(info.bumpMap);

		if (info.emissionTexture >= 0 && params[EMISSIONTEXTURE]->IsDefined())
			LoadTexture(info.emissionTexture, TEXTUREFLAGS_SRGB);

		Assert(info.envMap >= 0);
		int envMapFlags = g_pHardwareConfig->GetHDRType() == HDR_TYPE_NONE ? TEXTUREFLAGS_SRGB : 0;
		envMapFlags |= TEXTUREFLAGS_ALL_MIPS;
		LoadCubeMap(info.envMap, envMapFlags);

		if (params[CLOAKPASSENABLED]->GetIntValue())
		{
			CloakBlendedPassVars_t info;
			SetupVarsCloakBlendedPass(info);
			InitCloakBlendedPass(this, params, info);
		}



		// Emissive Scroll Pass
		if (params[EMISSIVEBLENDENABLED]->GetIntValue())
		{
			EmissiveScrollBlendedPassVars_t info;
			SetupVarsEmissiveScrollBlendedPass(info);
			InitEmissiveScrollBlendedPass(this, params, info);
		}


		Assert(info.mraoTexture >= 0);
		LoadTexture(info.mraoTexture, 0);

		SET_FLAGS(MATERIAL_VAR_MODEL);
		//SET_FLAGS(MATERIAL_VAR_MULTIPASS);

		SET_FLAGS2(MATERIAL_VAR2_SUPPORTS_HW_SKINNING);             // Required for skinning
		SET_FLAGS2(MATERIAL_VAR2_DIFFUSE_BUMPMAPPED_MODEL);         // Required for dynamic lighting
		SET_FLAGS2(MATERIAL_VAR2_NEEDS_TANGENT_SPACES);             // Required for dynamic lighting
		SET_FLAGS2(MATERIAL_VAR2_LIGHTING_VERTEX_LIT);              // Required for dynamic lighting
		SET_FLAGS2(MATERIAL_VAR2_NEEDS_BAKED_LIGHTING_SNAPSHOTS);   // Required for ambient cube
		//SET_FLAGS2(MATERIAL_VAR2_SUPPORTS_FLASHLIGHT);              // Required for flashlight
		//SET_FLAGS2(MATERIAL_VAR2_USE_FLASHLIGHT);                   // Required for flashlight


		


	}

	SHADER_DRAW
	{

		bool bDrawStandardPass = true;
		if (params[CLOAKPASSENABLED]->GetIntValue() && (pShaderShadow == NULL)) // && not snapshotting
		{
			CloakBlendedPassVars_t info;
			SetupVarsCloakBlendedPass(info);
			if (CloakBlendedPassIsFullyOpaque(params, info))
			{
				bDrawStandardPass = false;
			}
		};

		Conc_Player_Vars_t info;
		Setup_Conc_Player(params, info);

		// Setting up booleans
		bool bHasBaseTexture = (info.m_nBaseTexture != -1) && params[info.m_nBaseTexture]->IsTexture();
		bool bHasNormalTexture = (info.bumpMap != -1) && params[info.bumpMap]->IsTexture();
		bool bHasMraoTexture = (info.mraoTexture != -1) && params[info.mraoTexture]->IsTexture();
		bool bHasEmissionTexture = (info.emissionTexture != -1) && params[info.emissionTexture]->IsTexture();
		bool bHasEnvTexture = (info.envMap != -1) && params[info.envMap]->IsTexture();
		bool bIsAlphaTested = IS_FLAG_SET(MATERIAL_VAR_ALPHATEST) != 0;
		bool bHasMraoScale = (info.mraoScale != -1) && params[info.mraoScale]->IsDefined();
		bool bHasEmissionScale = (info.emissionScale != -1) && params[info.emissionScale]->IsDefined();
		bool bHasHSV = (info.hsv != -1) && params[info.hsv]->IsDefined();
		bool bBlendHSV = bHasHSV && IsBoolSet(info.hsv_blend, params);

		// Determining whether we're dealing with a fully opaque material
		BlendType_t nBlendType = EvaluateBlendRequirements(info.m_nBaseTexture, true);
		bool bFullyOpaque = (nBlendType != BT_BLENDADD) && (nBlendType != BT_BLEND) && !bIsAlphaTested;
		

		if (IS_PARAM_DEFINED(PLAYERCOLORMASK) && bDrawStandardPass)
		{
			float Float4Tint1[4] = { 1, 1, 1, 1 };
			params[PLAYERCOLOR1]->GetVecValue(Float4Tint1, 3);
			float Float4Tint2[4] = { 1, 1, 1, 1 };
			params[PLAYERCOLOR2]->GetVecValue(Float4Tint2, 3);
			float Float4Tint3[4] = { 1, 1, 1, 1 };
			params[PLAYERCOLOR3]->GetVecValue(Float4Tint3, 3);
			/*
			SwapFloatArrayElements(Float4Tint1, 1, 2);//green and blue get swapped in the shader for some reason so this is the lazy approach
			SwapFloatArrayElements(Float4Tint2, 1, 2);
			SwapFloatArrayElements(Float4Tint3, 1, 2);
			*///this fixed itself somehow

			/*
			if (//pShaderShadow != NULL
				//|| (Float4Tint1[0] != 1.000 || Float4Tint1[1] != 1.000 || Float4Tint1[2] != 1.000) 
				//|| (Float4Tint2[0] != 1.000 || Float4Tint2[1] != 1.000 || Float4Tint2[2] != 1.000)
				//|| (Float4Tint3[0] != 1.000 || Float4Tint3[1] != 1.000 || Float4Tint3[2] != 1.000)
				)
			{*/
			SHADOW_STATE
			{
				SetInitialShadowState();

				// If alphatest is on, enable it
				pShaderShadow->EnableAlphaTest(bIsAlphaTested);

				if (info.alphaTestReference != -1 && params[info.alphaTestReference]->GetFloatValue() > 0.0f)
				{
					pShaderShadow->AlphaFunc(SHADER_ALPHAFUNC_GEQUAL, params[info.alphaTestReference]->GetFloatValue());
				}

				SetDefaultBlendingShadowState(info.m_nBaseTexture, true);

				// Setting up samplers
				pShaderShadow->EnableTexture(SAMPLER_BASETEXTURE, true);    // Basecolor texture
				pShaderShadow->EnableSRGBRead(SAMPLER_BASETEXTURE, true);   // Basecolor is sRGB
				pShaderShadow->EnableTexture(SAMPLER_EMISSIVE, true);       // Emission texture
				pShaderShadow->EnableSRGBRead(SAMPLER_EMISSIVE, true);      // Emission is sRGB
				pShaderShadow->EnableTexture(SAMPLER_MRAO, true);           // MRAO texture
				pShaderShadow->EnableSRGBRead(SAMPLER_MRAO, false);         // MRAO isn't sRGB
				pShaderShadow->EnableTexture(SAMPLER_NORMAL, true);         // Normal texture
				pShaderShadow->EnableSRGBRead(SAMPLER_NORMAL, false);       // Normals aren't sRGB

				// Setting up envmap
				if (bHasEnvTexture)
				{
					pShaderShadow->EnableTexture(SAMPLER_ENVMAP, true); // Envmap
					if (g_pHardwareConfig->GetHDRType() == HDR_TYPE_NONE)
					{
						pShaderShadow->EnableSRGBRead(SAMPLER_ENVMAP, true); // Envmap is only sRGB with HDR disabled?
					}
				}

				// Enabling sRGB writing
				// See common_ps_fxc.h line 349
				// PS2b shaders and up write sRGB
				pShaderShadow->EnableSRGBWrite(true);



				int useParallax = params[info.useParallax]->GetIntValue();
				if (!mat_pbr_parallaxmap.GetBool())
				{
					useParallax = 0;
				}

				// We only need the position and surface normal
				unsigned int flags = VERTEX_POSITION | VERTEX_NORMAL | VERTEX_FORMAT_COMPRESSED;
				// We need three texcoords, all in the default float2 size
				pShaderShadow->VertexShaderVertexFormat(flags, 1, 0, 0);

				DECLARE_STATIC_VERTEX_SHADER(pbr_vs20);
				SET_STATIC_VERTEX_SHADER_COMBO(WVT, false);
				SET_STATIC_VERTEX_SHADER(pbr_vs20);

				DECLARE_STATIC_PIXEL_SHADER(conc_player_ps20b);
				//SET_STATIC_PIXEL_SHADER_COMBO(FLASHLIGHT, bHasFlashlight);
				//SET_STATIC_PIXEL_SHADER_COMBO(FLASHLIGHTDEPTHFILTERMODE, nShadowFilterMode);
				//SET_STATIC_PIXEL_SHADER_COMBO(LIGHTMAPPED, bLightMapped);
				SET_STATIC_PIXEL_SHADER_COMBO(EMISSIVE, bHasEmissionTexture);
				//SET_STATIC_PIXEL_SHADER_COMBO(WVT, bIsWVT);
				SET_STATIC_PIXEL_SHADER_COMBO(HSV, bHasHSV);
				SET_STATIC_PIXEL_SHADER_COMBO(HSV_BLEND, bBlendHSV);
				SET_STATIC_PIXEL_SHADER_COMBO(FLASHLIGHT, false);
				SET_STATIC_PIXEL_SHADER(conc_player_ps20b); 



				// Setting up fog
				DefaultFog(); // I think this is correct

				// HACK HACK HACK - enable alpha writes all the time so that we have them for underwater stuff
				pShaderShadow->EnableAlphaWrites(bFullyOpaque);
			};
			DYNAMIC_STATE
			{
				//bool bLightingOnly = mat_fullbright.GetInt() == 2 && !IS_FLAG_SET(MATERIAL_VAR_NO_DEBUG_OVERRIDE);

				// Setting up albedo texture
				if (bHasBaseTexture)
				{
					BindTexture(SAMPLER_BASETEXTURE, info.m_nBaseTexture, info.baseTextureFrame);
				}
				else
				{
					pShaderAPI->BindStandardTexture(SAMPLER_BASETEXTURE, TEXTURE_GREY);
				}

				// Setting up basecolor tint
				Vector4D hsv;
				if (bHasHSV)
				{
					params[info.hsv]->GetVecValue(hsv.Base(), 3);
					hsv.w = pShaderAPI->GetLightMapScaleFactor();
				}
				else
				{
					hsv.Init(1.0f, 1.0f, 1.0f, pShaderAPI->GetLightMapScaleFactor());
				}
				pShaderAPI->SetPixelShaderConstant(PSREG_SELFILLUMTINT, hsv.Base());

				// Setting up mrao scale
				Vector mraoScale;
				if (bHasMraoScale)
				{
					params[info.mraoScale]->GetVecValue(mraoScale.Base(), 3);
				}
				else
				{
					mraoScale.Init(1.0f, 1.0f, 1.0f);
				}
				pShaderAPI->SetPixelShaderConstant(26, mraoScale.Base());

				// Setting up emission scale
				Vector emissionScale;
				if (bHasEmissionScale)
				{
					params[info.emissionScale]->GetVecValue(emissionScale.Base(), 3);
				}
				else
				{
					emissionScale.Init(1.0f, 1.0f, 1.0f);
				}
				pShaderAPI->SetPixelShaderConstant(3, emissionScale.Base());

				// Setting up environment map
				if (bHasEnvTexture)
				{
					BindTexture(SAMPLER_ENVMAP, info.envMap, 0);
				}
				else
				{
					pShaderAPI->BindStandardTexture(SAMPLER_ENVMAP, TEXTURE_BLACK);
				}

				// Setting up emissive texture
				if (bHasEmissionTexture)
				{
					BindTexture(SAMPLER_EMISSIVE, info.emissionTexture, 0);
				}
				else
				{
					pShaderAPI->BindStandardTexture(SAMPLER_EMISSIVE, TEXTURE_BLACK);
				}

				// Setting up normal map
				if (bHasNormalTexture)
				{
					BindTexture(SAMPLER_NORMAL, info.bumpMap, 0);
				}
				else
				{
					pShaderAPI->BindStandardTexture(SAMPLER_NORMAL, TEXTURE_NORMALMAP_FLAT);
				}

				// Setting up mrao map
				if (bHasMraoTexture)
				{
					BindTexture(SAMPLER_MRAO, info.mraoTexture, 0);
				}
				else
				{
					pShaderAPI->BindStandardTexture(SAMPLER_MRAO, TEXTURE_WHITE);
				}


				// Getting the light state
				LightState_t lightState;
				pShaderAPI->GetDX9LightState(&lightState);


				// Getting fog info
				MaterialFogMode_t fogType = pShaderAPI->GetSceneFogMode();
				int fogIndex = (fogType == MATERIAL_FOG_LINEAR_BELOW_FOG_Z) ? 1 : 0;

				// Some debugging stuff
				bool bWriteDepthToAlpha = false;
				bool bWriteWaterFogToAlpha = false;
				if (bFullyOpaque)
				{
					bWriteDepthToAlpha = pShaderAPI->ShouldWriteDepthToDestAlpha();
					bWriteWaterFogToAlpha = (fogType == MATERIAL_FOG_LINEAR_BELOW_FOG_Z);
					AssertMsg(!(bWriteDepthToAlpha && bWriteWaterFogToAlpha),
							"Can't write two values to alpha at the same time.");
				}

				float vEyePos_SpecExponent[4];
				pShaderAPI->GetWorldSpaceCameraPosition(vEyePos_SpecExponent);

				// Determining the max level of detail for the envmap
				int iEnvMapLOD = 6;
				auto envTexture = params[info.envMap]->GetTextureValue();
				if (envTexture)
				{
					// Get power of 2 of texture width
					int width = envTexture->GetMappingWidth();
					int mips = 0;
					while (width >>= 1)
						++mips;

					// Cubemap has 4 sides so 2 mips less
					iEnvMapLOD = mips;
				}

				// Dealing with very high and low resolution cubemaps
				if (iEnvMapLOD > 12)
					iEnvMapLOD = 12;
				if (iEnvMapLOD < 4)
					iEnvMapLOD = 4;

				// This has some spare space
				vEyePos_SpecExponent[3] = iEnvMapLOD;
				pShaderAPI->SetPixelShaderConstant(PSREG_EYEPOS_SPEC_EXPONENT, vEyePos_SpecExponent, 1);

				BindTexture(SHADER_SAMPLER15, PLAYERCOLORMASK);
				pShaderAPI->SetPixelShaderConstant(13, Float4Tint1);
				pShaderAPI->SetPixelShaderConstant(14, Float4Tint2);
				pShaderAPI->SetPixelShaderConstant(15, Float4Tint3);
				pShaderAPI->CommitPixelShaderLighting(PSREG_LIGHT_INFO_ARRAY);
				DECLARE_DYNAMIC_VERTEX_SHADER(pbr_vs20);
				SET_DYNAMIC_VERTEX_SHADER_COMBO(DOWATERFOG, fogIndex);
				SET_DYNAMIC_VERTEX_SHADER_COMBO(SKINNING, pShaderAPI->GetCurrentNumBones() > 0);
				SET_DYNAMIC_VERTEX_SHADER_COMBO(LIGHTING_PREVIEW, pShaderAPI->GetIntRenderingParameter(INT_RENDERPARM_ENABLE_FIXED_LIGHTING) != 0);
				SET_DYNAMIC_VERTEX_SHADER_COMBO(COMPRESSED_VERTS, (int)vertexCompression);
				SET_DYNAMIC_VERTEX_SHADER_COMBO(NUM_LIGHTS, lightState.m_nNumLights);
				SET_DYNAMIC_VERTEX_SHADER(pbr_vs20);

				DECLARE_DYNAMIC_PIXEL_SHADER(conc_player_ps20b);//pbr again lol :)
				SET_DYNAMIC_PIXEL_SHADER_COMBO(NUM_LIGHTS, lightState.m_nNumLights);
				SET_DYNAMIC_PIXEL_SHADER_COMBO(WRITEWATERFOGTODESTALPHA, bWriteWaterFogToAlpha);
				SET_DYNAMIC_PIXEL_SHADER_COMBO(WRITE_DEPTH_TO_DESTALPHA, bWriteDepthToAlpha);
				SET_DYNAMIC_PIXEL_SHADER_COMBO(PIXELFOGTYPE, pShaderAPI->GetPixelFogCombo());
				//SET_DYNAMIC_PIXEL_SHADER_COMBO(FLASHLIGHTSHADOWS, false);//we can always asume this to be false in dmcr ... I think
				SET_DYNAMIC_PIXEL_SHADER(conc_player_ps20b);
				
				// Setting up base texture transform
				SetVertexShaderTextureTransform(VERTEX_SHADER_SHADER_SPECIFIC_CONST_0, info.baseTextureTransform);

				// This is probably important
				SetModulationPixelShaderDynamicState_LinearColorSpace(PSREG_DIFFUSE_MODULATION);

				// Send ambient cube to the pixel shader, force to black if not available
				pShaderAPI->SetPixelShaderStateAmbientLightCube(PSREG_AMBIENT_CUBE, !lightState.m_bAmbientLight);

				// Send lighting array to the pixel shader
				pShaderAPI->CommitPixelShaderLighting(PSREG_LIGHT_INFO_ARRAY);

				// Handle mat_specular 0 (no envmap reflections)
				if (!mat_specular.GetBool())
				{
					pShaderAPI->BindStandardTexture(SAMPLER_ENVMAP, TEXTURE_BLACK); // Envmap
				}

				// Sending fog info to the pixel shader
				pShaderAPI->SetPixelShaderFogParams(PSREG_FOG_PARAMS);

				/*
				float flParams[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
				// Parallax Depth (the strength of the effect)
				flParams[0] = GetFloatParam(info.parallaxDepth, params, 3.0f);
				// Parallax Center (the height at which it's not moved)
				flParams[1] = GetFloatParam(info.parallaxCenter, params, 3.0f);
				pShaderAPI->SetPixelShaderConstant(40, flParams, 1);
				*/
			};
			

		};

		

		Draw();
	}
END_SHADER
