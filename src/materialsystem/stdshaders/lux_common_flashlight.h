//
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
//	Initial D.	:	24.01.2023 DMY
//	Last Change :	19.02.2023 DMY
//
//	Purpose of this File :	Shader Constant Register Declaration for
//							everything you would ever see on a Pixelshader
//
//===========================================================================//

#ifndef LUX_COMMON_FLASHLIGHT_H_
#define LUX_COMMON_FLASHLIGHT_H_

//===========================================================================//
//	Declaring FLOAT PixelShader Constant Registers ( PSREG's )
//	There can be a maximum of 224 according to Microsoft Documentation
//	Ranging from 0-223. After this follows b0-b15 and i0 - i15
//===========================================================================//
const float4 g_ShadowTweaks					: register(c2); // Flashlight ShadowTweaks
const float4 g_FlashlightAttenuationFactors : register(c13);
const float4 g_FlashlightPos				: register(c14);
const float4x4 g_FlashlightWorldToTexture	: register(c15);
// 16--used by g_FlashlightWorldToTexture
// 17--used by g_FlashlightWorldToTexture
// 18--used by g_FlashlightWorldToTexture
const float4 cFlashlightColor				: register(c28); // common_fxc.h
const float4 cFlashlightScreenScale			: register(c31); // common_fxc.h - .zw are currently unused
#define f1FlashlightNoLambertValue			(cFlashlightColor.w) // This is either 0.0 or 2.0

//===========================================================================//
//	Declaring Samplers. We only have 16 on SM3.0. Ranging from 0-15
//	So we have to reuse them depending on what shader we are on and what features we require...
//	Note : Local definitions might be different. We don't always need to have clear names.
//===========================================================================//
//#if FLASHLIGHT && !CASCADED_SHADOWS
sampler Sampler_ShadowDepth		: register(s13);
sampler Sampler_RandomRotation	: register(s14);
sampler Sampler_FlashlightCookie: register(s15);

// Stock function.
float RemapValClamped(float val, float A, float B, float C, float D)
{
	float cVal = (val - A) / (B - A);
	cVal = saturate(cVal);

	return C + (D - C) * cVal;
}

//===========================================================================//
// ShiroDkxtro2: This was taken from the SourcePlusPlus repository
//===========================================================================//
float LUX_DoShadowNvidiaPCF5x5Gaussian(const float3 shadowMapPos, const float2 vShadowTweaks)
{
	float fEpsilonX = vShadowTweaks.x;
	float fTwoEpsilonX = 2.0f * fEpsilonX;
	float fEpsilonY = vShadowTweaks.y;
	float fTwoEpsilonY = 2.0f * fEpsilonY;

	float3 shadowMapCenter_objDepth = shadowMapPos;

	float2 shadowMapCenter = shadowMapCenter_objDepth.xy;			// Center of shadow filter
	float objDepth = shadowMapCenter_objDepth.z;					// Object depth in shadow space

	float4 vOneTaps;
	vOneTaps.x = tex2Dproj(Sampler_ShadowDepth, float4(shadowMapCenter + float2(fTwoEpsilonX, fTwoEpsilonY), objDepth, 1)).x;
	vOneTaps.y = tex2Dproj(Sampler_ShadowDepth, float4(shadowMapCenter + float2(-fTwoEpsilonX, fTwoEpsilonY), objDepth, 1)).x;
	vOneTaps.z = tex2Dproj(Sampler_ShadowDepth, float4(shadowMapCenter + float2(fTwoEpsilonX, -fTwoEpsilonY), objDepth, 1)).x;
	vOneTaps.w = tex2Dproj(Sampler_ShadowDepth, float4(shadowMapCenter + float2(-fTwoEpsilonX, -fTwoEpsilonY), objDepth, 1)).x;
	float flOneTaps = dot(vOneTaps, float4(1.0f / 331.0f, 1.0f / 331.0f, 1.0f / 331.0f, 1.0f / 331.0f));

	float4 vSevenTaps;
	vSevenTaps.x = tex2Dproj(Sampler_ShadowDepth, float4(shadowMapCenter + float2(fTwoEpsilonX, 0), objDepth, 1)).x;
	vSevenTaps.y = tex2Dproj(Sampler_ShadowDepth, float4(shadowMapCenter + float2(-fTwoEpsilonX, 0), objDepth, 1)).x;
	vSevenTaps.z = tex2Dproj(Sampler_ShadowDepth, float4(shadowMapCenter + float2(0, fTwoEpsilonY), objDepth, 1)).x;
	vSevenTaps.w = tex2Dproj(Sampler_ShadowDepth, float4(shadowMapCenter + float2(0, -fTwoEpsilonY), objDepth, 1)).x;
	float flSevenTaps = dot(vSevenTaps, float4(7.0f / 331.0f, 7.0f / 331.0f, 7.0f / 331.0f, 7.0f / 331.0f));

	float4 vFourTapsA, vFourTapsB;
	vFourTapsA.x = tex2Dproj(Sampler_ShadowDepth, float4(shadowMapCenter + float2(fTwoEpsilonX, fEpsilonY), objDepth, 1)).x;
	vFourTapsA.y = tex2Dproj(Sampler_ShadowDepth, float4(shadowMapCenter + float2(fEpsilonX, fTwoEpsilonY), objDepth, 1)).x;
	vFourTapsA.z = tex2Dproj(Sampler_ShadowDepth, float4(shadowMapCenter + float2(-fEpsilonX, fTwoEpsilonY), objDepth, 1)).x;
	vFourTapsA.w = tex2Dproj(Sampler_ShadowDepth, float4(shadowMapCenter + float2(-fTwoEpsilonX, fEpsilonY), objDepth, 1)).x;
	vFourTapsB.x = tex2Dproj(Sampler_ShadowDepth, float4(shadowMapCenter + float2(-fTwoEpsilonX, -fEpsilonY), objDepth, 1)).x;
	vFourTapsB.y = tex2Dproj(Sampler_ShadowDepth, float4(shadowMapCenter + float2(-fEpsilonX, -fTwoEpsilonY), objDepth, 1)).x;
	vFourTapsB.z = tex2Dproj(Sampler_ShadowDepth, float4(shadowMapCenter + float2(fEpsilonX, -fTwoEpsilonY), objDepth, 1)).x;
	vFourTapsB.w = tex2Dproj(Sampler_ShadowDepth, float4(shadowMapCenter + float2(fTwoEpsilonX, -fEpsilonY), objDepth, 1)).x;
	float flFourTapsA = dot(vFourTapsA, float4(4.0f / 331.0f, 4.0f / 331.0f, 4.0f / 331.0f, 4.0f / 331.0f));
	float flFourTapsB = dot(vFourTapsB, float4(4.0f / 331.0f, 4.0f / 331.0f, 4.0f / 331.0f, 4.0f / 331.0f));

	float4 v20Taps;
	v20Taps.x = tex2Dproj(Sampler_ShadowDepth, float4(shadowMapCenter + float2(fEpsilonX, fEpsilonY), objDepth, 1)).x;
	v20Taps.y = tex2Dproj(Sampler_ShadowDepth, float4(shadowMapCenter + float2(-fEpsilonX, fEpsilonY), objDepth, 1)).x;
	v20Taps.z = tex2Dproj(Sampler_ShadowDepth, float4(shadowMapCenter + float2(fEpsilonX, -fEpsilonY), objDepth, 1)).x;
	v20Taps.w = tex2Dproj(Sampler_ShadowDepth, float4(shadowMapCenter + float2(-fEpsilonX, -fEpsilonY), objDepth, 1)).x;
	float fl20Taps = dot(v20Taps, float4(20.0f / 331.0f, 20.0f / 331.0f, 20.0f / 331.0f, 20.0f / 331.0f));

	float4 v33Taps;
	v33Taps.x = tex2Dproj(Sampler_ShadowDepth, float4(shadowMapCenter + float2(fEpsilonX, 0), objDepth, 1)).x;
	v33Taps.y = tex2Dproj(Sampler_ShadowDepth, float4(shadowMapCenter + float2(-fEpsilonX, 0), objDepth, 1)).x;
	v33Taps.z = tex2Dproj(Sampler_ShadowDepth, float4(shadowMapCenter + float2(0, fEpsilonY), objDepth, 1)).x;
	v33Taps.w = tex2Dproj(Sampler_ShadowDepth, float4(shadowMapCenter + float2(0, -fEpsilonY), objDepth, 1)).x;
	float fl33Taps = dot(v33Taps, float4(33.0f / 331.0f, 33.0f / 331.0f, 33.0f / 331.0f, 33.0f / 331.0f));

	float flCenterTap = tex2Dproj(Sampler_ShadowDepth, float4(shadowMapCenter, objDepth, 1)).x * (55.0f / 331.0f);

	// Sum all 25 Taps
	return flOneTaps + flSevenTaps + flFourTapsA + flFourTapsB + fl20Taps + fl33Taps + flCenterTap;
}

//===========================================================================//
//	New Flashlight Shadow Function
//===========================================================================//
float3 LUX_DoFlashlight(float3 f3WorldPosition, float3 f3FaceNormal, const bool bDoShadows)
{
	// Speeds up rendering by not doing anything if this is the case.
	// TODO: What the heck is in the .w
	if (g_FlashlightPos.w < 0)
	{
		return float3(0, 0, 0);
	}
	else
	{
		float4 f4FlashlightSpacePosition = mul(float4(f3WorldPosition, 1.0), g_FlashlightWorldToTexture);
		clip(f4FlashlightSpacePosition.w);

		float3 f3ProjCoords = f4FlashlightSpacePosition.xyz / f4FlashlightSpacePosition.w;

		float3 f3FlashlightColor = tex2D(Sampler_FlashlightCookie, f3ProjCoords.xy).xyz;

		// Removed some ifdef for Shadermodels here.
		f3FlashlightColor *= cFlashlightColor.xyz;

		float3	f3Delta = g_FlashlightPos.xyz - f3WorldPosition;
		float3	f3L = normalize(f3Delta);
		float	f1DistSquared = dot(f3Delta, f3Delta);
		float	f1Dist = sqrt(f1DistSquared);

		float	f1FarZ = g_FlashlightAttenuationFactors.w;
		float	f1EndFalloffFactor = RemapValClamped(f1Dist, f1FarZ, 0.6f * f1FarZ, 0.0f, 1.0f);

		// Attenuation for light and to fade out shadow over distance
		float	f1Atten = saturate(dot(g_FlashlightAttenuationFactors.xyz, float3(1.0f, 1.0f / f1Dist, 1.0f / f1DistSquared)));

		// Shadowing and coloring terms
		if (bDoShadows)
		{
			// TODO: Figure out if this was set to 2048 because of forced Shadow Resolution or because its good values...
			float f1Shadow = LUX_DoShadowNvidiaPCF5x5Gaussian(f3ProjCoords, float2(1.0 / 2048.0, 1.0 / 2048.0));
			float f1Attenuated = lerp(saturate(f1Shadow), 1.0f, g_ShadowTweaks.y);	// Blend between fully attenuated and not attenuated

			f1Shadow = saturate(lerp(f1Attenuated, f1Shadow, f1Atten));	// Blend between shadow and above, according to light attenuation
			f3FlashlightColor *= f1Shadow;									// Shadow term
		}

		float3 f3DiffuseLighting = f1Atten;
#if defined(NORMALTEXTURE)
		f3DiffuseLighting *= saturate(dot(f3L.xyz, f3FaceNormal) + f1FlashlightNoLambertValue); // NoLambertValue is either 0 or 2
#else
		f3DiffuseLighting *= saturate(1.0f + f1FlashlightNoLambertValue); // If its 2 we almost get the *PI
#endif
		
		f3DiffuseLighting *= f3FlashlightColor;
		f3DiffuseLighting *= f1EndFalloffFactor;

		return f3DiffuseLighting;
	}
}
//#endif // #if FLASHLIGHT


#endif // End of LUX_COMMON_PS_FXC_H_