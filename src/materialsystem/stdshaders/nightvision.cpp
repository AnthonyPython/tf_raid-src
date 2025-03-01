#include "BaseVSShader.h"
 
#include "nightvision_ps20.inc"
#include "passthrough_vs20.inc"
 
BEGIN_VS_SHADER( NightVision, "Help for NightVision" )

	BEGIN_SHADER_PARAMS
		SHADER_PARAM( BASETEXTURE, SHADER_PARAM_TYPE_TEXTURE, "_rt_FullFrameFB", "Framebuffer" )
		SHADER_PARAM( NVLEVEL, SHADER_PARAM_TYPE_FLOAT, "1.0", "Night vision level" )
	END_SHADER_PARAMS

	SHADER_INIT_PARAMS( )
	{
		SET_FLAGS2( MATERIAL_VAR2_NEEDS_FULL_FRAME_BUFFER_TEXTURE );
	}

	SHADER_INIT
	{
		if( params[BASETEXTURE]->IsDefined( ) )
			LoadTexture( BASETEXTURE );
	}

	SHADER_DRAW
	{
		SHADOW_STATE
		{
			pShaderShadow->EnableTexture( SHADER_SAMPLER0, true );
 
			pShaderShadow->EnableDepthWrites( false );
			int fmt = VERTEX_POSITION;
			pShaderShadow->VertexShaderVertexFormat( fmt, 1, 0, 0 );
			pShaderShadow->SetVertexShader( "passthrough_vs20", 0 );
			pShaderShadow->SetPixelShader( "nightvision_ps20" );
			DefaultFog( );
		}

		DYNAMIC_STATE
		{
			BindTexture( SHADER_SAMPLER0, BASETEXTURE, -1 );
			float Scale = params[NVLEVEL]->GetFloatValue( );
			float vScale[4] = { Scale, Scale, Scale, 1 };
			pShaderAPI->SetPixelShaderConstant( 0, vScale );
		}

		Draw( );
	}

END_SHADER
