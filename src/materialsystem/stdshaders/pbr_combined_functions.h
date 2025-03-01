//==================================================================================================
//
// Physically Based Rendering Header for brushes and models
//
//==================================================================================================

//  Source gives us four light colors and positions in an
//  array of three of these structures like so:
//
//       x		y		z      w
//    +------+------+------+------+
//    |       L0.rgb       |      |
//    +------+------+------+      |
//    |       L0.pos       |  L3  |
//    +------+------+------+  rgb |
//    |       L1.rgb       |      |
//    +------+------+------+------+
//    |       L1.pos       |      |
//    +------+------+------+      |
//    |       L2.rgb       |  L3  |
//    +------+------+------+  pos |
//    |       L2.pos       |      |
//    +------+------+------+------+
//

struct PixelShaderLightInfo
{
	float4 color;
	float4 pos;
};

#define cOverbright 2.0f
#define cOOOverbright 0.5f

#define LIGHTTYPE_NONE				0
#define LIGHTTYPE_SPOT				1
#define LIGHTTYPE_POINT				2
#define LIGHTTYPE_DIRECTIONAL		3

//===========================================================================//
//	Declaring FLOAT PixelShader Constant Registers ( PSREG's )
//	There can be a maximum of 224 according to Microsoft Documentation
//	Ranging from 0-223. After this follows b0-b15 and i0 - i15
//===========================================================================//
const float3 cAmbientCube[6]				: register(c4);
// 5-used by cAmbientCube[1]
// 6-used by cAmbientCube[2]
// 7-used by cAmbientCube[3]
// 8-used by cAmbientCube[4]
// 9-used by cAmbientCube[5]
PixelShaderLightInfo cLightInfo[3]			: register(c20); // 4th light spread across w's
// 21--------used by cLightInfo[1]
// 22--------used by cLightInfo[2]
// 23--------used by cLightInfo[2]
// 24--------used by cLightInfo[3]
// 25--------used by cLightInfo[3]
// const float4 cLightScale					: register(c30); // common_fxc.h

#define			f1DiElectricCoefficient		(g_BaseTextureTint_Factor.w)
const float4	g_EmissionTint_WetnessBias : register(c34);
#define			f3EmissionTint				(g_EmissionTint_WetnessBias.xyz)

const float4 g_ParallaxControls : register(c50);
#define f1ParallaxHeight			(g_ParallaxControls.x)
#define f1ParallaxMaxOffset			(g_ParallaxControls.y)
#define f1ParallaxIntensity			(g_ParallaxControls.z)
#define f1ParallaxInterval			(g_ParallaxControls.w)

const float4 g_MRAOControls : register(c51);
#define f1MetallicBias				(g_MRAOControls.x)
#define f1RoughnessBias				(g_MRAOControls.y)
#define f1AOBias					(g_MRAOControls.z)
#define f1SheenScale				(g_MRAOControls.w)

const float4 g_VariousControls1 : register(c55);

#if defined(DISPLACEMENT) // Not compatible with SSS ( imagine putting SSS on mostly flat surfaces... )
const float4 BaseTextureTint2_WetnessBias2 : register(c56);
#define			f3BaseTexture2Tint	(BaseTextureTint2_WetnessBias2.xyz)
const float4 g_MRAOControls2 : register(c57);
#define f1Metallic2Bias				(g_MRAOControls2.x)
#define f1Roughness2Bias			(g_MRAOControls2.y)
#define f1AO2Bias					(g_MRAOControls2.z)
#endif

// Not compatible with DetailTexture because we need the sampler.
#if SPECIALPROPERTIES
const float4	g_Vector_LayerFactor			: register(c33);
#define			f3DesiredVector					(g_Vector_LayerFactor.xyz)
#define			f1VectorLayerFactor				(g_Vector_LayerFactor.w)
#define			f1WetnessBias					(g_EmissionTint_WetnessBias.w)
#define			bVectorBlend					Bools[3]
#define			bWetnessPorosity				Bools[4]

const float4 g_Vector_LayerFactor2 : register(c58);
#define			f3DesiredVector2				(g_Vector_LayerFactor2.xyz)
#define			f1VectorLayerFactor2			(g_Vector_LayerFactor2.w)

sampler Sampler_VectorLayer		: register(s4);
sampler Sampler_Properties		: register(s8);

// Need these on WVT...
#if defined(DISPLACEMENT)
sampler Sampler_Properties2		: register(s9);
sampler Sampler_VectorLayer2	: register(s10);
#define			f1WetnessBias2					(BaseTextureTint2_WetnessBias2.w)
#endif
#endif

sampler Sampler_BentNormal	: register(s5);
sampler Sampler_MRAO		: register(s6);
#if defined(MODEL)
sampler Sampler_LightMap	: register(s11); // LIGHTMAPS only without SSS ( model lightmapping and brush lightmapping )
#endif

#if SSS // Only want this with SSS
const float4 g_SSSTint_ : register(c56);
#define f3SSSTint				(g_SSSTint_.xyz)

const float4 g_SSSControls : register(c57);
#define f1SSSPower				(g_SSSControls.x)
#define f1SSSScale				(g_SSSControls.y)
#define f1SSSEmission			(g_SSSControls.z)
#define f1SSSIntensity			(g_SSSControls.w)

sampler Sampler_SSS			: register(s12);
#endif
sampler Sampler_Emission	: register(s13);

//////////////////////////////////////////////////////////////////////
//					ACTUAL FUNCTIONS USED ON PBR					//
//////////////////////////////////////////////////////////////////////

// Universal Constants
static const float PI = 3.141592;
static const float ONE_OVER_PI = 0.318309;
static const float EPSILON = 0.00001;

// Ripped from vertexlitgeneric header. And less function-calls
float3 AmbientLight(const float3 worldNormal, const float3 cAmbientCube[6])
{
	float3 linearColor, nSquared = worldNormal * worldNormal;
	float3 isNegative = (worldNormal >= 0.0) ? 0 : nSquared;
	float3 isPositive = (worldNormal >= 0.0) ? nSquared : 0;
	linearColor = isPositive.x * cAmbientCube[0] + isNegative.x * cAmbientCube[1] +
		isPositive.y * cAmbientCube[2] + isNegative.y * cAmbientCube[3] +
		isPositive.z * cAmbientCube[4] + isNegative.z * cAmbientCube[5];
	return linearColor;
}

// Unreal Engine 4 approximation of the Fresnel. Kudos 
// I ripped this specific code from Tottery's Coolsource
float3 UE4Fresnel(float HdV, float3 F0)
{
	return F0 + (1.0f.xxx - F0)*pow(2, (-5.55473f*HdV - 6.98316)*HdV);
}

// I stole from Tottery again, please forgive me.
// Ok but then... Tottery responded to "I don't want to steal your code"
// with "Its not my code"
float3 FresnelRoughness(float theta, float3 F0, float roughness)
{
	return F0 + (max(1.0f.xxx - roughness, F0) - F0)*pow(2, (-5.55473f*theta - 6.98316)*theta);
}

// GGX
float Distribution(float NdH, float a)
{
	// Evil Roughness double squared
	float a2 = a*a;
	float denom = ((NdH*NdH) * (a2 - 1) + 1);
	denom *= denom;
	return a2 / (denom*PI);
}

// Schlick-GGX
float GeometrySub(float NdV, float k)
{
	float denom = (NdV*(1 - k) + k);
	return NdV / denom;
}

// TODO : return to float then square instead of two times the call. ( Does the Compiler optimise this away? )
float Geometry(float NdV, float NdL, float k)
{
	return GeometrySub(NdV, k) * GeometrySub(NdL, k);
}

// Couldn't make a it a #define...
float3 StaticUnrealDiffuseIBL(float3 Albedo, float3 Specular, float Roughness, float3 ViewDir, float3 NormalDir, float3 LightColor)
{
	// We expect NormalDir and ViewDir to already be normalized!!!
	float NdV = max(0.0, dot(NormalDir, ViewDir));

	// 1.0f -
	// UE4Fresnel( max(0.0f, dot(normal, outgoingLightDirection)), f3Specular)
	float3 F = 1.0f - FresnelRoughness(NdV, Specular, Roughness);
	float3 diffuse = min(1.0f, (Albedo * F * LightColor));
	return diffuse;
}

float3 UnrealDiffuseIBL(const float3 Albedo, const float3 Specular, const float Roughness, const float3 ViewDir, const float3 NormalDir, const float3 LightDir, const float3 LightColor)
{
	// We expect NormalDir and ViewDir to already be normalized!!!
	float NdV = max(dot(NormalDir, ViewDir), 0.0f);

	float3 H = normalize(LightDir + ViewDir);
	float NdH = max(dot(NormalDir, H), 0.0f);
	float NdL = max(dot(NormalDir, LightDir), 0.0f);
	float HdV = max(dot(H, ViewDir), 0.0f);

	float RoughnessSquared = Roughness * Roughness;
	float k = (Roughness + 1);
	k = (k*k) / 8;

	float D = Distribution(NdH, RoughnessSquared);
	float G = Geometry(NdV, NdL, k);

	float fFresnel = UE4Fresnel(HdV, Specular);
	float fFresnelR = FresnelRoughness(HdV, Specular, Roughness);
	float fiFresnel = 1.0f - fFresnel;
	float fiFresnelR = 1.0f - fFresnelR;

	float3 fCookTorrance = (D*G*fFresnelR) / (4 * NdV*NdL);

	float3 fLambert = (Albedo / PI) * fiFresnel;
	// fFresnel * 
	return (fCookTorrance + fLambert) * LightColor * NdL; // BRDF Based Rendering Diffuse Fucking
}

// WRD : We do this because :
// 1. Static models do not have ambient cubes, it appears...
// 2. Brushes don't have ambient cubes, it appears...
// 3. We don't have ambient cubes, it appears...
// 4. Ambient Cubes are dependent on the player position and viewvector... ( this is also the case with cubemaps, but less noticable )
// 4. ____ Ambient cubes at this point. This is literally the same thing but we aren't dependent on visleafs
// By doing this we can get an actual snapshot of the environment, and that more than one per area, and we can overwrite too
// Plus we avoid this heresy of static models and brushes... RIP ambient cubes!
// Create an ambient cube from the envmap
void setupEnvMapAmbientCube(out float3 EnvAmbientCube[6], sampler EnvmapSampler)
{
	float4 directionPosX = { 1, 0, 0, 12 }; float4 directionNegX = { -1, 0, 0, 12 };
	float4 directionPosY = { 0, 1, 0, 12 }; float4 directionNegY = { 0, -1, 0, 12 };
	float4 directionPosZ = { 0, 0, 1, 12 }; float4 directionNegZ = { 0, 0, -1, 12 };
	EnvAmbientCube[0] = ENV_MAP_SCALE * texCUBElod(EnvmapSampler, directionPosX).rgb;
	EnvAmbientCube[1] = ENV_MAP_SCALE * texCUBElod(EnvmapSampler, directionNegX).rgb;
	EnvAmbientCube[2] = ENV_MAP_SCALE * texCUBElod(EnvmapSampler, directionPosY).rgb;
	EnvAmbientCube[3] = ENV_MAP_SCALE * texCUBElod(EnvmapSampler, directionNegY).rgb;
	EnvAmbientCube[4] = ENV_MAP_SCALE * texCUBElod(EnvmapSampler, directionPosZ).rgb;
	EnvAmbientCube[5] = ENV_MAP_SCALE * texCUBElod(EnvmapSampler, directionNegZ).rgb;
}

/*
// GGX/Towbridge-Reitz normal distribution function
// Uses Disney's reparametrization of alpha = roughness^2
float ndfGGX(float cosLh, float roughness)
{
    float alpha   = roughness * roughness;
    float alphaSq = alpha * alpha;

    float denom = (cosLh * cosLh) * (alphaSq - 1.0) + 1.0;
    return alphaSq / (PI * denom * denom);
}

// Single term for separable Schlick-GGX below
float gaSchlickG1(float cosTheta, float k)
{
    return cosTheta / (cosTheta * (1.0 - k) + k);
}

// Schlick-GGX approximation of geometric attenuation function using Smith's method
float gaSchlickGGX(float cosLi, float cosLo, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0; // Epic suggests using this roughness remapping for analytic lights
    return gaSchlickG1(cosLi, k) * gaSchlickG1(cosLo, k);
}
*/

float3 PBR_ComputeLight(float3 f3LightIn, float3 f3WorldPos, float3 f3WorldNormal, float3 f3LightColor)
{
	float3 f3Result = float3(0.0f, 0.0f, 0.0f);
	float f1dot_ = dot(f3WorldNormal, f3LightIn); // Potentially negative dot

	f1dot_ = saturate(f1dot_);

	f3Result = float3(f1dot_, f1dot_, f1dot_);

	// Order doesn't matter thanks to multiplication...
	f3Result *= f3LightColor;

	return f3Result;
}

// Monte Carlo integration, approximate analytic version based on Dimitar Lazarov's work
// https://www.unrealengine.com/en-US/blog/physically-based-shading-on-mobile
float3 EnvBRDFApprox(float3 SpecularColor, float Roughness, float NoV)
{
    const float4 c0 = { -1, -0.0275, -0.572, 0.022 };
    const float4 c1 = { 1, 0.0425, 1.04, -0.04 };
    float4 r = Roughness * c0 + c1;
    float a004 = min(r.x * r.x, exp2(-9.28 * NoV)) * r.x + r.y;
    float2 AB = float2(-1.04, 1.04) * a004 + r.zw;
    return SpecularColor * AB.x + AB.y;
}
//
//// Based on Uncharted 4's Technical Art Siggraph
//// http://advances.realtimerendering.com/other/2016/naughty_dog/index.html
//// Note : This probably only really works for models. Brushes will not be able to get the Light Vector
float ApplyMicroShadows(float AmbientOcclusion, float3 NormalVector, float3 IncomingLightVector)
{
	float aperture = 2.0f * (AmbientOcclusion * AmbientOcclusion);
	float MicroShadow = saturate(abs(dot(IncomingLightVector, NormalVector)) + aperture - 1.0f);
	return MicroShadow;
}
//
//// Detail'blend'modes
//#define DETAIL_MOD2X								0	// mod2x
//#define DETAIL_ADDITIVE							1	// base.rgb + detail.rgb*fblend
//#define DETAIL_OVER_BASE							2	// Lerp detail.rgb with base.rgb based on detail.w * blendfactor
//#define DETAIL_FADE								3	// straight fade between base and detail.
//#define DETAIL_UNDER_BASE							4   // use base alpha for blend over detail
//#define DETAIL_ADD_SELFILLUM						5   // CombinedLighting+detail.rgb post lighting
//#define DETAIL_ADD_SELFILLUM_THRESHOLD_FADE		6	// 
//#define DETAIL_MOD2X_SELECT_TWO_PATTERNS			7	// use alpha channel of base to select between mod2x channels in r+a of detail
//#define DETAIL_MULTIPLY							8	// Detail.rgb * Basetexture.rgb
//#define DETAIL_MASK_BASE_BY_ALPHA					9   // use alpha channel of detail to mask base
//#define DETAIL_SSBUMP_BUMP						10	// use detail to modulate lighting as an ssbump
//#define DETAIL_SSBUMP_NOBUMP						11	// detail is an ssbump but use it as an albedo. shader does the magic here - no user needs to specify mode 11
//#define DETAIL_OVER_ROUGHNESS						12	// detail modifies roughness
//#define DETAIL_OVER_METALLIC						13	// detail modifies metallic
//#define DETAIL_OVER_MR							14	// detail modifies metallic but inverses is applied to roughness to adjust reflectivity levels
//#define DETAIL_RETROREFLECTIVE					15	// RGB retroreflectivity mask. boost lighting if viewangle and light angle are close to eachother
//#define DETAIL_IRIDESCENCE						16	// Iridescence texture, interferes with lighting results! --TODO: maybe scrap the last part
//#define DETAIL_VECTOR_LAYER						17	// Snow\Dust Layer, or whatever else you want. The Detailtint defines the vector which should be applied. 0,0,1 would be up facing surfaces.
//#define DETAIL_WETNESS_POROSITY					18	// Based on Uncharted 4's Technical Art Siggraph http://advances.realtimerendering.com/other/2016/naughty_dog/index.html
//
//// What our new detail function requires :
//// BaseTexture,			float4 format
//// DetailTexture,		Sampler only	-- Sampler needed for Iridescence.
//// Detail UV,			float2 format	-- Required for sampler ( Iridescence requires it for Noise texture lookup )
//// BlendFactor,			float1 format
//// Blendmode,			integr format
//// NormalVector,		float3 format	-- for snowmode
//// DetailTint,			float3 format	-- for everything but especially snowmode.
//// MRAO,				float3 format	-- Wetness, R, M, MR blendmodi
//// ViewVector			Vector format	-- Retroreflections, Iridescence
//// Light to Surface		Vector format	-- Retroreflections, Iridescence
//
////0
//float4 Detail_Mod2X(float4 BaseTexture, float4, DetailTexture, float BlendFactor)
//
////1
//float4 Detail_Additive(float4 BaseTexture, float4, DetailTexture, float BlendFactor)
//
////2
//float4 Detail_Over_Base(float4 BaseTexture, float4, DetailTexture, float BlendFactor)
//
////3
//float4 Detail_Fade(float4 BaseTexture, float4, DetailTexture, float BlendFactor)
//
////4
//float4 Detail_Under_Base(float4 BaseTexture, float4, DetailTexture, float BlendFactor)
//
////5
//float4 Detail_Add_Selfillum(float4 BaseTexture, float4, DetailTexture, float BlendFactor)
//
////6
//float4 Detail_Add_Selfillum_Threshold_Fade(float4 BaseTexture, float4, DetailTexture, float BlendFactor)
//
////7
//float4 Detail_Mod2X_TwoPatterns(float4 BaseTexture, float4, DetailTexture, float BlendFactor)
//
////8
//float4 Detail_Multiply(float4 BaseTexture, float4, DetailTexture, float BlendFactor)
//
////9
//float4 Detail_MaskByBaseAlpha(float4 BaseTexture, float4, DetailTexture, float BlendFactor)
//
////10
//float4 Detail_SSBump_Bump(float4 BaseTexture, float4, DetailTexture, float BlendFactor)
//
////11
//float4 Detail_SSBump_NoBump(float4 BaseTexture, float4, DetailTexture, float BlendFactor)
//
////==========================
//// New Blendmodes
////==========================
//
////12
//float4 Detail_Roughness(float4 f4MRAO, float f4DetailTexture, float f1DetailBlendFactor)
//
////13
//float4 Detail_Metallic(float4 f4MRAO, float f4DetailTexture, float f1DetailBlendFactor)
//
////14
//float4 Detail_Met(float4 f4MRAO, float f4DetailTexture, float f1DetailBlendFactor)
//
////15
//float4 Detail_Retroreflective(	float4, f4DetailTexture, float3 f3DetailTint, float f1BlendFactor,
//								float3 f3NormalVector, float3 f3ViewVector, float3 f3LightVector,
//								float3 f3LightColor, float f1LightAttenuation, float f1DistanceToSurface)
//{
//// Want to clamp to 0 because normals that face away are going to be negative... -1 * -1 equals 1. So facing away basically means illuminated if we don't clamp it
//// We only max() one of these because 0 * -infinity is still equal to 0
//	float f1VectorSimilarity = dot(f3NormalVector, f3ViewVector) * max( 0.0f, dot(f3NormalVector, f3LightVector));
//
//	// We make the effect fall off, if we get close to the surface
//	// Note BlendFactor = Threshold - distance in units!!!
//	if (f1DistanceToSurface <= f1BlendFactor)
//	{
//		// Calculate a factor that makes the effect disappear at a distance of 50 units. Negative values not allowed. ( when going closer than 50 units )
//		f1VectorSimilarity *= max(0.0f, (f1DistanceToSurface - 50.0f) / (f1BlendFactor - 50.0f));
//	}
//
//	// Detailtexture is both color of the surface and the mask.
//	// Multiply, aka make sure the surface should be visible
//	float3 f3RetroreflectiveLighting = f4DetailTexture.xyz * f1VectorSimilarity
//	// DetailTint is both the strength of the effect and tint! We mix in Light Attenuation to make sure the surface actually receives light.
//	f3RetroreflectiveLighting *= (f1LightAttenuation * 2.0f * f3LightColor) * f3DetailTint;
//
//	return float4 Result(f4RetroreflectiveLighting.xyz, 0);
//}
//
////16
//float4 Detail_Iridescence(sampler DetailTextureSampler, float f1NoiseTexture,		float3 f3DetailTint,
//							float f1BlendFactor,		float3 f3NormalVector,		float3 f3ViewVector)
//{
//	float f1DistortedLookup = dot(f3NormalVector, f3ViewVector);
//	// Adding some distortion from normal UV coordinates : note, we could in theory use noise() however generating perlin noise on the shader is more expensive than texture lookup
//	// We also skipped some conditioning, so the detailtexture is loaded using normal UV's. Therefore we already *did* the lookup...
//	// Too bad that we have to do it again...
//	f1DistortedLookup *= f1NoiseTexture;
//	// Strength of the iridescence :
//	f1DistortedLookup *= f1BlendFactor;
//
//	// Alpha is useless here. Double distortion...
//	float4 f4Iridescence = tex2D(DetailTextureSampler, f1DistortedLookup);
//
//	// Not sure what purposes this will serve, the Iridescence is supposed to rainbow'ish... Could filter some colors and reuse the same Iridescence Texture?
//	f4Iridescence *= f3DetailTint;
//	return f4Iridescence;
//}
//
////17
//float4 Detail_VectorLayer(float4 f4BaseTexture, float4 f4DetailTexture, float3 f3DetailTint, float f1BlendFactor, float3 f3NormalVector)
//{
//	float f1FacingCorrectDirection = dot(f3NormalVector, f3DetailTint);
//	// Hope this gives some control about the strength
//	f1FacingCorrectDirection = clamp(0.0f, 1.0f, f1FacingCorrectDirection);
//
//	float3 f3Result = ((1.0f - f1FacingCorrectDirection) * f4BaseTexture.xyz) + (f1FacingCorrectDirection*f4DetailTexture.xyz);
//	return float4(f3Result, 0);
//}
//
////18
//float4 Detail_Wetness_Porosity()
//
//float4 TextureCombine(float4 baseColor, float4 detailColor, int combine_mode,
//	float fBlendFactor)
//{
//	if (combine_mode == TCOMBINE_RGB_EQUALS_BASE_x_DETAILx2) // 0
//		baseColor.rgb *= lerp(float3(1, 1, 1), 2.0*detailColor.rgb, fBlendFactor);
//
//	if (combine_mode == TCOMBINE_RGB_ADDITIVE) // 1
//		baseColor.rgb += fBlendFactor * detailColor.rgb;
//
//	if (combine_mode == TCOMBINE_DETAIL_OVER_BASE) // 2
//	{
//		float fblend = fBlendFactor * detailColor.a;
//		baseColor.rgb = lerp(baseColor.rgb, detailColor.rgb, fblend);
//	}
//
//	if (combine_mode == TCOMBINE_FADE) // 3
//	{
//		baseColor = lerp(baseColor, detailColor, fBlendFactor);
//	}
//
//	if (combine_mode == TCOMBINE_BASE_OVER_DETAIL) // 4
//	{
//		float fblend = fBlendFactor * (1 - baseColor.a);
//		baseColor.rgb = lerp(baseColor.rgb, detailColor.rgb, fblend);
//		baseColor.a = detailColor.a;
//	}
//
//	if (combine_mode == TCOMBINE_MOD2X_SELECT_TWO_PATTERNS) // 7
//	{
//		float3 dc = lerp(detailColor.r, detailColor.a, baseColor.a);
//		baseColor.rgb *= lerp(float3(1, 1, 1), 2.0*dc, fBlendFactor);
//	}
//
//	if (combine_mode == TCOMBINE_MULTIPLY) // 8
//	{
//		baseColor = lerp(baseColor, baseColor*detailColor, fBlendFactor);
//	}
//
//	if (combine_mode == TCOMBINE_MASK_BASE_BY_DETAIL_ALPHA) // 9
//	{
//		baseColor.a = lerp(baseColor.a, baseColor.a*detailColor.a, fBlendFactor);
//	}
//	if (combine_mode == TCOMBINE_SSBUMP_NOBUMP) // 11
//	{
//		baseColor.rgb = baseColor.rgb * dot(detailColor.rgb, 2.0 / 3.0);
//	}
//	return baseColor;
//}
//
//float3 TextureCombinePostLighting(float3 lit_baseColor, float4 detailColor, int combine_mode,
//	float fBlendFactor)
//{
//	if (combine_mode == TCOMBINE_RGB_ADDITIVE_SELFILLUM)
//		lit_baseColor += fBlendFactor * detailColor.rgb;
//	if (combine_mode == TCOMBINE_RGB_ADDITIVE_SELFILLUM_THRESHOLD_FADE)
//	{
//		// fade in an unusual way - instead of fading out color, remap an increasing band of it from
//		// 0..1
//		//if (fBlendFactor > 0.5)
//		//	lit_baseColor += min(1, (1.0/fBlendFactor)*max(0, detailColor.rgb-(1-fBlendFactor) ) );
//		//else
//		//	lit_baseColor += 2*fBlendFactor*2*max(0, detailColor.rgb-.5);
//
//		float f = fBlendFactor - 0.5;
//		float fMult = (f >= 0) ? 1.0 / fBlendFactor : 4 * fBlendFactor;
//		float fAdd = (f >= 0) ? 1.0 - fMult : -0.5*fMult;
//		lit_baseColor += saturate(fMult * detailColor.rgb + fAdd);
//	}
//	return lit_baseColor;
//}

//===========================================================================//
//	TODO: New Flashlight Shadow Function
//===========================================================================//

#if SHEEN
//===========================================================================//
//	Sheen rendering.
//	Adapted from AshikhminDV, CharlieD_AshikhminV & CharlieDV
//	Thanks to knarkowicz for uploading this to <https://www.shadertoy.com/view/4tfBzn#>
//===========================================================================//
float AshikhminD(float ndoth, float RoughnessSquared)
{
	float cos2h = ndoth * ndoth;
	float sin2h = 1.0f - cos2h;
	float sin4h = sin2h * sin2h;
	return (sin4h + 4.0f * exp(-cos2h / (sin2h * RoughnessSquared))) / (PI * (1.0f + 4.0f * RoughnessSquared) * sin4h);
}

float AshikhminV(float ndotv, float ndotl)
{
	return 1.0f / (4.0f * (ndotl + ndotv - ndotl * ndotv));
}

float3 SheenDiffuseIBL(const float3 Albedo, const float3 Specular, const float Roughness, const float3 ViewDir, const float3 NormalDir, const float3 LightDir, const float3 LightColor)
{
	// We expect NormalDir and ViewDir to already be normalized!!!
	float NdV = max(dot(NormalDir, ViewDir), 0.0f);

	float3 H = normalize(LightDir + ViewDir);
	float NdH = max(dot(NormalDir, H), 0.0f);
	float NdL = max(dot(NormalDir, LightDir), 0.0f);
	float HdV = max(dot(H, ViewDir), 0.0f);

	float RoughnessSquared = Roughness * Roughness;
	float k = (Roughness + 1);
	k = (k*k) / 8;

	float D = AshikhminD(NdH, RoughnessSquared); // (1.0f - (ndh * ndh))² + 4 * exp( -(ndh * ndh) / (1.0f - (ndh * ndh)) * roughness²)) / (pi * ( 1.0f + 4.0f * roughness²) * (1.0f - (ndh * ndh))²)
	float G = AshikhminV(NdV, NdL); // 1 / ( 4 * ( ndl + ndv - ndl * ndv )

	float fFresnel = UE4Fresnel(HdV, Specular);
	float fFresnelR = FresnelRoughness(HdV, Specular, Roughness);
	float fiFresnel = 1.0f - fFresnel;
	float fiFresnelR = 1.0f - fFresnelR;

	float3 fCookTorrance = (D*G*fFresnelR) / (4 * NdV*NdL);

	float3 fLambert = (Albedo / PI) * fiFresnel;
	// fFresnel * 
	return (fCookTorrance + fLambert) * LightColor * NdL; // BRDF Based Rendering Diffuse Fucking
}
#endif

#if SSS
//===========================================================================//
//	Lightwrap function ( makes the light go more around the geometry of the model )
//	Adapted from NaughtyDog/Uncharted pdf
//	Thanks to Colin Barré-Brisebois and Marc Bouchard for this implementation!
//	https://advances.realtimerendering.com/other/2016/naughty_dog/NaughtyDog_TechArt_Final.pdf
//===========================================================================//
/* SSS is good enough without!
float3 ApplyLightWrap(float3 f3LightWrapColor, float3 f3TextureNormal, float3 f3VertexNormal, float3 f3LightDir)
{
	static const float f1LightWrapDistance = 0.1f;
	float3 f3WrapLight = f1LightWrapDistance * f3LightWrapColor; // Input Light Color?
	float f1NdotL = dot(f3TextureNormal, f3LightDir); // Potentially negative.
	f1NdotL = lerp(max(f3WrapLight.r, max(f3WrapLight.g, f3WrapLight.b)), 1.0f, f1NdotL);

	float	f1WrapForwardNdotL	=	max(f1NdotL, dot(f3VertexNormal, f3LightDir));
	float3	f3WrapForward		=	lerp( f3WrapLight, float3(1.0f, 1.0f, 1.0f), f1WrapForwardNdotL);
	float3	f3WrapRecede		=	lerp(-f3WrapLight, float3(1.0f, 1.0f, 1.0f), f1NdotL);
	float3	f3WrapedLighting	=	saturate(lerp(f3WrapRecede, f3WrapForward, f3LightWrapColor));

	return f3WrapedLighting;
}
*/

//===========================================================================//
//	Sub-Surface Scattering Approximation
//	Adapted from Frostbite2 / EA
//	Thanks to Colin Barré-Brisebois and Marc Bouchard for this implementation!
// <https://colinbarrebrisebois.com/2011/03/07/gdc-2011-approximating-translucency-for-a-fast-cheap-and-convincing-subsurface-scattering-look/>
//===========================================================================//
float3 ComputeSSSLight(float3 f3LightIn, float3 f3FaceNormal, float3 f3ViewVector, float f1LightAttenuation, float4 f4SSSTexture, float3 f3LightColor, float3 f3Emission, out float3 f3EmissionLight)
{
	// Compute Backscattering
	float3 H = normalize(f3LightIn + f3FaceNormal);
	// -H means its not facing towards the lightsource.
	float I = pow(saturate(dot(f3ViewVector, -H)), f1SSSPower) * f1SSSScale; // Power and Scale terms are Artist input. Defined at the top of the file...

#if EMISSION
	// Raw Emission Color
	f3EmissionLight = f1LightAttenuation.xxx * (I.xxx + f3Emission) * f4SSSTexture.xxx * f1SSSEmission.xxx;
#else
	f3EmissionLight = float3(0,0,0);
#endif
	// r = thickness. RGB is on GBA. That way we can use compression when there is no blue.
	return float3(f1LightAttenuation.xxx * I.xxx * f4SSSTexture.yzw * f4SSSTexture.xxx * f3LightColor * f1SSSIntensity);
}
#endif

#if (SPECIALPROPERTIES && !DETAILTEXTURE)
//===========================================================================//
//	ClampRange function required for Wetness/Porosity Implementation
//	Adapted from NaughtyDog/Uncharted pdf
//	https://advances.realtimerendering.com/other/2016/naughty_dog/NaughtyDog_TechArt_Final.pdf
//===========================================================================//
float ClampRange(float f1Input, float f1Minimum, float f1Maximum)
{
	return saturate((f1Input - f1Minimum) / (f1Maximum - f1Minimum));
}
#endif

#if defined(DISPLACEMENT)
bool tex2D_Seamless(
sampler SamplerBase, sampler SamplerNormal, sampler Sampler_MRAO,
sampler SamplerBase2, sampler SamplerNormal2, sampler Sampler_MRAO2,
out float4 f4BaseTextureResult, out float4 f4NormalTextureResult, out float4 f4MRAOTextureResult,
out float4 f4BaseTexture2Result, out float4 f4NormalTexture2Result, out float4 f4MRAOTexture2Result,
float3 f3UVWBase, float3 f3UVWBase2, float3 f3UVWNormal, float3 f3UVWNormal2, float3 f3Weights
)
{
	f4BaseTextureResult		= float4(0,0,0,0);
	f4NormalTextureResult	= float4(0,0,0,0);
	f4MRAOTextureResult		= float4(0,0,0,0);
	f4BaseTexture2Result	= float4(0,0,0,0);
	f4NormalTexture2Result	= float4(0,0,0,0);
	f4MRAOTexture2Result	= float4(0,0,0,0);



	// TODO: Explain this. Why - 0.3f? And why the sum
	f3Weights = max(f3Weights - 0.3f, 0.0f);
	// Tthis is probably used to average the three results we get so that we remain in linear space
	f3Weights *= 1.0f / (f3Weights.x + f3Weights.y + f3Weights.z);

	// This is like, absolute giga expensive... wow...
	// Well what won't you do for Seamless coords...
	//[branch]
	if (f3Weights.x > 0)
	{
		f4BaseTextureResult		+= f3Weights.x * tex2D(SamplerBase,		  f3UVWBase.zy);
		f4BaseTexture2Result	+= f3Weights.x * tex2D(SamplerBase2,	 f3UVWBase2.zy);
		f4NormalTextureResult	+= f3Weights.x * tex2D(SamplerNormal,	f3UVWNormal.zy);
		f4NormalTexture2Result	+= f3Weights.x * tex2D(SamplerNormal2, f3UVWNormal2.zy);
		f4MRAOTextureResult		+= f3Weights.x * tex2D(Sampler_MRAO,	  f3UVWBase.zy);
		f4MRAOTexture2Result	+= f3Weights.x * tex2D(Sampler_MRAO2,	 f3UVWBase2.zy);
	}

	//[branch]
	if (f3Weights.y > 0)
	{
		f4BaseTextureResult		+= f3Weights.y * tex2D(SamplerBase,		  f3UVWBase.xz);
		f4BaseTexture2Result	+= f3Weights.y * tex2D(SamplerBase2,	 f3UVWBase2.xz);
		f4NormalTextureResult	+= f3Weights.y * tex2D(SamplerNormal,	f3UVWNormal.xz);
		f4NormalTexture2Result	+= f3Weights.y * tex2D(SamplerNormal2, f3UVWNormal2.xz);
		f4MRAOTextureResult		+= f3Weights.y * tex2D(Sampler_MRAO,	  f3UVWBase.xz);
		f4MRAOTexture2Result	+= f3Weights.y * tex2D(Sampler_MRAO2,	 f3UVWBase2.xz);
	}

	//[branch]
	if (f3Weights.z > 0)
	{
		f4BaseTextureResult		+= f3Weights.z * tex2D(SamplerBase,		  f3UVWBase.xy);
		f4BaseTexture2Result	+= f3Weights.z * tex2D(SamplerBase2,	 f3UVWBase2.xy);
		f4NormalTextureResult	+= f3Weights.z * tex2D(SamplerNormal,	f3UVWNormal.xy);
		f4NormalTexture2Result	+= f3Weights.z * tex2D(SamplerNormal2, f3UVWNormal2.xy);
		f4MRAOTextureResult		+= f3Weights.z * tex2D(Sampler_MRAO,	  f3UVWBase.xy);
		f4MRAOTexture2Result	+= f3Weights.z * tex2D(Sampler_MRAO2,	 f3UVWBase2.xy);
	}

	return true;
}

bool tex2D_SeamlessSecondary(
	sampler SamplerBase, sampler SamplerNormal, sampler Sampler_MRAO,
	out float4 f4BaseTextureResult, out float4 f4NormalTextureResult, out float4 f4MRAOTextureResult,
	float3 f3UVWBase, float3 f3UVWNormal, float3 f3Weights
	)
{
	f4BaseTextureResult		= float4(0,0,0,0);
	f4NormalTextureResult	= float4(0,0,0,0);
	f4MRAOTextureResult		= float4(0,0,0,0);
	// TODO: Explain this. Why - 0.3f? And why the sum
	f3Weights = max(f3Weights - 0.3f, 0.0f);
	// Tthis is probably used to average the three results we get so that we remain in linear space
	f3Weights *= 1.0f / (f3Weights.x + f3Weights.y + f3Weights.z);

	// This is like, absolute giga expensive... wow...
	// Well what won't you do for Seamless coords...
	//[branch]
	if (f3Weights.x > 0)
	{
		f4BaseTextureResult += f3Weights.x * tex2D(SamplerBase, f3UVWBase.zy);
		f4NormalTextureResult += f3Weights.x * tex2D(SamplerNormal, f3UVWNormal.zy);
		f4MRAOTextureResult += f3Weights.x * tex2D(Sampler_MRAO, f3UVWBase.zy);
	}

	//[branch]
	if (f3Weights.y > 0)
	{
		f4BaseTextureResult += f3Weights.y * tex2D(SamplerBase, f3UVWBase.xz);
		f4NormalTextureResult += f3Weights.y * tex2D(SamplerNormal, f3UVWNormal.xz);
		f4MRAOTextureResult += f3Weights.y * tex2D(Sampler_MRAO, f3UVWBase.xz);
	}

	//[branch]
	if (f3Weights.z > 0)
	{
		f4BaseTextureResult += f3Weights.z * tex2D(SamplerBase, f3UVWBase.xy);
		f4NormalTextureResult += f3Weights.z * tex2D(SamplerNormal, f3UVWNormal.xy);
		f4MRAOTextureResult += f3Weights.z * tex2D(Sampler_MRAO, f3UVWBase.xy);
	}

	return true;
}
#endif