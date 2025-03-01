#include "BaseVSShader.h"
#include "cpp_shader_constant_register_map.h"

// Defining samplers
const Sampler_t SAMPLER_BASETEXTURE = SHADER_SAMPLER0;
const Sampler_t SAMPLER_BASETEXTURE2 = SHADER_SAMPLER3;
const Sampler_t SAMPLER_NORMAL = SHADER_SAMPLER1;
const Sampler_t SAMPLER_NORMAL2 = SHADER_SAMPLER12;
const Sampler_t SAMPLER_ENVMAP = SHADER_SAMPLER2;
const Sampler_t SAMPLER_SHADOWDEPTH = SHADER_SAMPLER4;
const Sampler_t SAMPLER_RANDOMROTATION = SHADER_SAMPLER5;
const Sampler_t SAMPLER_FLASHLIGHT = SHADER_SAMPLER6;
const Sampler_t SAMPLER_LIGHTMAP = SHADER_SAMPLER7;
const Sampler_t SAMPLER_MRAO = SHADER_SAMPLER10;
const Sampler_t SAMPLER_MRAO2 = SHADER_SAMPLER13;
const Sampler_t SAMPLER_EMISSIVE = SHADER_SAMPLER11;
const Sampler_t SAMPLER_EMISSIVE2 = SHADER_SAMPLER14;

// Convars
static ConVar mat_fullbright("mat_fullbright", "0", FCVAR_CHEAT);
static ConVar mat_specular("mat_specular", "1", FCVAR_CHEAT);
static ConVar mat_pbr_force_20b("mat_pbr_force_20b", "0", FCVAR_CHEAT);
static ConVar mat_pbr_parallaxmap("mat_pbr_parallaxmap", "1");

// Variables for this shader
struct PBR_Vars_t
{
    PBR_Vars_t()
    {
        memset(this, 0xFF, sizeof(*this));
    }

    int baseTexture;
    int baseTexture2;
    int baseColor;
    int normalTexture;
    int bumpMap;
    int bumpMap2;
    int envMap;
    int baseTextureFrame;
    int baseTextureFrame2;
    int baseTextureTransform;
    int useParallax;
    int parallaxDepth;
    int parallaxCenter;
    int alphaTestReference;
    int flashlightTexture;
    int flashlightTextureFrame;
    int emissionTexture;
    int emissionTexture2;
    int mraoTexture;
    int mraoTexture2;
    int mraoScale;
    int mraoScale2;
    int emissionScale;
    int emissionScale2;
    int hsv;
    int hsv_blend;
};