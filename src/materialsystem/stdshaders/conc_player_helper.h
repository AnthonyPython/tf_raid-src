#include <string.h>

struct Conc_Player_Vars_t
{
	Conc_Player_Vars_t() { memset(this, 0xFF, sizeof(*this)); }
	int m_nBaseTexture;
	int m_nDiffuseWarpTexture;
	int m_nSelfIllumEnvMapMask_Alpha;
	int m_nAmbientOnly;
	int m_nBaseMapAlphaPhongMask;
	int m_nEnvmapFresnel;
	int m_nPlayerColorMask;
	int m_nPlayerColor1;
	int m_nPlayerColor2;
	int m_nPlayerColor3;
	int baseColor;
	int normalTexture;
	int bumpMap;
	int bumpMap2;
	int envMap;
	int baseTextureFrame;
	int baseTextureTransform;
	int useParallax;
	int parallaxDepth;
	int parallaxCenter;
	int alphaTestReference;
	int flashlightTexture;
	int flashlightTextureFrame;
	int emissionTexture;
	int mraoTexture;
	int mraoScale;
	int emissionScale;
	int hsv;
	int hsv_blend;
};

void SwapFloatArrayElements(float *vec, int pos1, int pos2)
{
	float tmp1 = vec[pos1];
	float tmp2 = vec[pos2];
	vec[pos1] = tmp2;
	vec[pos2] = tmp1;

}