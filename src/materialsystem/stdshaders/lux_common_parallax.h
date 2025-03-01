//
//========= ShiroDkxtro2's --ACROHS Ultimate Shaders Project-- ============//
//
//	Initial D.	:	24.01.2023 DMY
//	Last Change :	22.03.2023 DMY
//
//	Purpose of this File :	Functions related to parallax mapping.
//
//===========================================================================//

#ifndef LUX_COMMON_PARALLAX_H_
#define LUX_COMMON_PARALLAX_H_
/*
float2 f2ParallaxOffset(float2 f2TextureCoordinate, float3 f3RelativeViewDirection, sampler Sampler, float f1ParallaxScale, float3 f3NormalDirection, float3 f3ViewVector)
{
	static const int iMaxSamples = 12;
	static const int iMinSamples = 1;

	float f1Length = length(f3RelativeViewDirection);
	float f1ParallaxLength = sqrt(f1Length * f1Length - f3RelativeViewDirection.z * f3RelativeViewDirection.z) / f3RelativeViewDirection.z;
	float2 f2ParallaxDirection = normalize(f3RelativeViewDirection.xy);
	float2 f2ParallaxOffsetTS = f2ParallaxDirection * f1ParallaxLength;
	f2ParallaxOffsetTS *= f1ParallaxScale * lerp(0, 1, dot(f3ViewVector, f3NormalDirection));

	// Compute all the derivatives:
	float2 dx = ddx(f2TextureCoordinate);
	float2 dy = ddy(f2TextureCoordinate);

	// Consider truncing instead?
	int nNumSteps = (int)lerp(iMinSamples, iMaxSamples, dot(f3ViewVector, f3NormalDirection));

	float f1CurrentHeight = 0.0;
	float f1StepSize = 1.0 / (float)nNumSteps;
	float f1PrevHeight = 1.0;

	int    iStepIndex = 0;
	bool   bCondition = true;

	float2 vTexOffsetPerStep = f1StepSize * f2ParallaxOffsetTS;
	float2 vTexCurrentOffset = f2TextureCoordinate;
	float  fCurrentBound = 1.0;
	float  fParallaxAmount = 0.0;

	float2 pt1 = 0;
	float2 pt2 = 0;

//	float2 texOffset2 = 0;

	while (iStepIndex < nNumSteps)
	{
		vTexCurrentOffset -= vTexOffsetPerStep;

		// Sample height map which in this case is stored in the alpha channel of the normal map:
		f1CurrentHeight = tex2Dgrad(Sampler, vTexCurrentOffset, dx, dy).w;

		fCurrentBound -= f1StepSize;

		if (f1CurrentHeight > fCurrentBound)
		{
			pt1 = float2(fCurrentBound, f1CurrentHeight);
			pt2 = float2(fCurrentBound + f1StepSize, f1PrevHeight);

//			texOffset2 = vTexCurrentOffset - vTexOffsetPerStep;

//			iStepIndex = nNumSteps + 1;
			// This will do exactly the same thing as the above.
			break;
		}
		else
		{
			iStepIndex++;
			f1PrevHeight = f1CurrentHeight;
		}
	}   // End of while ( iStepIndex < nNumSteps )

	float fDelta2 = pt2.x - pt2.y;
	float fDelta1 = pt1.x - pt1.y;
	float f1ParallaxAmount = (pt1.x * fDelta2 - pt2.x * fDelta1) / (fDelta2 - fDelta1);

	// Result :
	float2 f2ParallaxOffset = f2ParallaxOffsetTS * (1 - f1ParallaxAmount);
	return f2ParallaxOffset;
}
*/



// POM
// ShiroDkxtro2 : Can confirm this works but FPS is terrible of course. 24 samples looks identical to 16 layers and has the same layering issues that 16 layers has...
float2 OffsetCoordinates(float2 texCoord, float3 viewDir, sampler2D texSampler, float heightScale, float maxOffset)
{
	float layerNum = 32.0;
	float layerDepth = 1.0 / layerNum;

	float lastHeight = 0;
	float lastDepth = 0;

	float2 offset = -viewDir.xy * heightScale;

	for (float depth = 1.0; depth >= -0.1; depth -= layerDepth)
	{
		float currentHeight = tex2D(texSampler, texCoord + offset * depth).w;
		if (depth < currentHeight)
		{
			float2 lastOffset = offset * lastDepth;
			float c = currentHeight - depth;
			float l = lastDepth - lastHeight;
			float mixValue = c / (c + l);
			offset = lerp(offset * depth, lastOffset, mixValue);
			break;
		}
		lastHeight = currentHeight;
		lastDepth = depth;
	}

	return offset;
}
//PIM
// ShiroDkxtro2 : works but there is a sort of "infinite" drop issue and the layering is quite apparently in most cases when compared to POM
/*
float2 GetParallaxOffset(float3 f3RelativeViewDirection, float f1ParallaxHeight, float f1ParallaxMaxOffset, sampler2D Sampler_NormalTexture, float2 UV)
{
	float2 f2Offset = float2(0, 0);
	float f1CurrentHeight = 0;
	float f1LayerHeight = f1ParallaxHeight / 24.0f;
	float MaxOffset = f1ParallaxMaxOffset / 24.0f;

	for (int i = 0; i<24; i++)
	{
		// Get the height at the current layer
		f1CurrentHeight = 1.0f - tex2D(Sampler_NormalTexture, UV + f2Offset).w;

		// Calculate the offset for the current layer
		float f1ParallaxOffset = (f1CurrentHeight - f1ParallaxHeight) * MaxOffset;

		// Add the offset to the running total
		f2Offset += -f3RelativeViewDirection.xy * f1ParallaxOffset;

		// Break if conditions are ideal
		if (f1CurrentHeight > f1ParallaxHeight)
			break;
	}

	return f2Offset;
}
*/
#endif // End of LUX_COMMON_PARALLAX_H_