#include "shaderlib/cshader.h"
class pbr_mrao_glass_ps30_Static_Index
{
private:
	int m_nFLASHLIGHT;
#ifdef _DEBUG
	bool m_bFLASHLIGHT;
#endif
public:
	void SetFLASHLIGHT( int i )
	{
		Assert( i >= 0 && i <= 0 );
		m_nFLASHLIGHT = i;
#ifdef _DEBUG
		m_bFLASHLIGHT = true;
#endif
	}
	void SetFLASHLIGHT( bool i )
	{
		m_nFLASHLIGHT = i ? 1 : 0;
#ifdef _DEBUG
		m_bFLASHLIGHT = true;
#endif
	}
private:
	int m_nSPECIALPROPERTIES;
#ifdef _DEBUG
	bool m_bSPECIALPROPERTIES;
#endif
public:
	void SetSPECIALPROPERTIES( int i )
	{
		Assert( i >= 0 && i <= 0 );
		m_nSPECIALPROPERTIES = i;
#ifdef _DEBUG
		m_bSPECIALPROPERTIES = true;
#endif
	}
	void SetSPECIALPROPERTIES( bool i )
	{
		m_nSPECIALPROPERTIES = i ? 1 : 0;
#ifdef _DEBUG
		m_bSPECIALPROPERTIES = true;
#endif
	}
private:
	int m_nREFRACTION;
#ifdef _DEBUG
	bool m_bREFRACTION;
#endif
public:
	void SetREFRACTION( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nREFRACTION = i;
#ifdef _DEBUG
		m_bREFRACTION = true;
#endif
	}
	void SetREFRACTION( bool i )
	{
		m_nREFRACTION = i ? 1 : 0;
#ifdef _DEBUG
		m_bREFRACTION = true;
#endif
	}
private:
	int m_nBRUSH;
#ifdef _DEBUG
	bool m_bBRUSH;
#endif
public:
	void SetBRUSH( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nBRUSH = i;
#ifdef _DEBUG
		m_bBRUSH = true;
#endif
	}
	void SetBRUSH( bool i )
	{
		m_nBRUSH = i ? 1 : 0;
#ifdef _DEBUG
		m_bBRUSH = true;
#endif
	}
public:
	pbr_mrao_glass_ps30_Static_Index( )
	{
#ifdef _DEBUG
		m_bFLASHLIGHT = false;
#endif // _DEBUG
		m_nFLASHLIGHT = 0;
#ifdef _DEBUG
		m_bSPECIALPROPERTIES = false;
#endif // _DEBUG
		m_nSPECIALPROPERTIES = 0;
#ifdef _DEBUG
		m_bREFRACTION = false;
#endif // _DEBUG
		m_nREFRACTION = 0;
#ifdef _DEBUG
		m_bBRUSH = false;
#endif // _DEBUG
		m_nBRUSH = 0;
	}
	int GetIndex()
	{
		// Asserts to make sure that we aren't using any skipped combinations.
		// Asserts to make sure that we are setting all of the combination vars.
#ifdef _DEBUG
		bool bAllStaticVarsDefined = m_bFLASHLIGHT && m_bSPECIALPROPERTIES && m_bREFRACTION && m_bBRUSH;
		Assert( bAllStaticVarsDefined );
#endif // _DEBUG
		return ( 80 * m_nFLASHLIGHT ) + ( 80 * m_nSPECIALPROPERTIES ) + ( 80 * m_nREFRACTION ) + ( 160 * m_nBRUSH ) + 0;
	}
};
#define shaderStaticTest_pbr_mrao_glass_ps30 psh_forgot_to_set_static_FLASHLIGHT + psh_forgot_to_set_static_SPECIALPROPERTIES + psh_forgot_to_set_static_REFRACTION + psh_forgot_to_set_static_BRUSH + 0
class pbr_mrao_glass_ps30_Dynamic_Index
{
private:
	int m_nWRITEWATERFOGTODESTALPHA;
#ifdef _DEBUG
	bool m_bWRITEWATERFOGTODESTALPHA;
#endif
public:
	void SetWRITEWATERFOGTODESTALPHA( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nWRITEWATERFOGTODESTALPHA = i;
#ifdef _DEBUG
		m_bWRITEWATERFOGTODESTALPHA = true;
#endif
	}
	void SetWRITEWATERFOGTODESTALPHA( bool i )
	{
		m_nWRITEWATERFOGTODESTALPHA = i ? 1 : 0;
#ifdef _DEBUG
		m_bWRITEWATERFOGTODESTALPHA = true;
#endif
	}
private:
	int m_nPIXELFOGTYPE;
#ifdef _DEBUG
	bool m_bPIXELFOGTYPE;
#endif
public:
	void SetPIXELFOGTYPE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nPIXELFOGTYPE = i;
#ifdef _DEBUG
		m_bPIXELFOGTYPE = true;
#endif
	}
	void SetPIXELFOGTYPE( bool i )
	{
		m_nPIXELFOGTYPE = i ? 1 : 0;
#ifdef _DEBUG
		m_bPIXELFOGTYPE = true;
#endif
	}
private:
	int m_nWRITE_DEPTH_TO_DESTALPHA;
#ifdef _DEBUG
	bool m_bWRITE_DEPTH_TO_DESTALPHA;
#endif
public:
	void SetWRITE_DEPTH_TO_DESTALPHA( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nWRITE_DEPTH_TO_DESTALPHA = i;
#ifdef _DEBUG
		m_bWRITE_DEPTH_TO_DESTALPHA = true;
#endif
	}
	void SetWRITE_DEPTH_TO_DESTALPHA( bool i )
	{
		m_nWRITE_DEPTH_TO_DESTALPHA = i ? 1 : 0;
#ifdef _DEBUG
		m_bWRITE_DEPTH_TO_DESTALPHA = true;
#endif
	}
private:
	int m_nFLASHLIGHTSHADOWS;
#ifdef _DEBUG
	bool m_bFLASHLIGHTSHADOWS;
#endif
public:
	void SetFLASHLIGHTSHADOWS( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nFLASHLIGHTSHADOWS = i;
#ifdef _DEBUG
		m_bFLASHLIGHTSHADOWS = true;
#endif
	}
	void SetFLASHLIGHTSHADOWS( bool i )
	{
		m_nFLASHLIGHTSHADOWS = i ? 1 : 0;
#ifdef _DEBUG
		m_bFLASHLIGHTSHADOWS = true;
#endif
	}
private:
	int m_nNUM_LIGHTS;
#ifdef _DEBUG
	bool m_bNUM_LIGHTS;
#endif
public:
	void SetNUM_LIGHTS( int i )
	{
		Assert( i >= 0 && i <= 4 );
		m_nNUM_LIGHTS = i;
#ifdef _DEBUG
		m_bNUM_LIGHTS = true;
#endif
	}
	void SetNUM_LIGHTS( bool i )
	{
		m_nNUM_LIGHTS = i ? 1 : 0;
#ifdef _DEBUG
		m_bNUM_LIGHTS = true;
#endif
	}
private:
	int m_nLIGHTMAPPED_MODEL;
#ifdef _DEBUG
	bool m_bLIGHTMAPPED_MODEL;
#endif
public:
	void SetLIGHTMAPPED_MODEL( int i )
	{
		Assert( i >= 0 && i <= 0 );
		m_nLIGHTMAPPED_MODEL = i;
#ifdef _DEBUG
		m_bLIGHTMAPPED_MODEL = true;
#endif
	}
	void SetLIGHTMAPPED_MODEL( bool i )
	{
		m_nLIGHTMAPPED_MODEL = i ? 1 : 0;
#ifdef _DEBUG
		m_bLIGHTMAPPED_MODEL = true;
#endif
	}
private:
	int m_nCASCADED_SHADOW;
#ifdef _DEBUG
	bool m_bCASCADED_SHADOW;
#endif
public:
	void SetCASCADED_SHADOW( int i )
	{
		Assert( i >= 0 && i <= 0 );
		m_nCASCADED_SHADOW = i;
#ifdef _DEBUG
		m_bCASCADED_SHADOW = true;
#endif
	}
	void SetCASCADED_SHADOW( bool i )
	{
		m_nCASCADED_SHADOW = i ? 1 : 0;
#ifdef _DEBUG
		m_bCASCADED_SHADOW = true;
#endif
	}
public:
	pbr_mrao_glass_ps30_Dynamic_Index()
	{
#ifdef _DEBUG
		m_bWRITEWATERFOGTODESTALPHA = false;
#endif // _DEBUG
		m_nWRITEWATERFOGTODESTALPHA = 0;
#ifdef _DEBUG
		m_bPIXELFOGTYPE = false;
#endif // _DEBUG
		m_nPIXELFOGTYPE = 0;
#ifdef _DEBUG
		m_bWRITE_DEPTH_TO_DESTALPHA = false;
#endif // _DEBUG
		m_nWRITE_DEPTH_TO_DESTALPHA = 0;
#ifdef _DEBUG
		m_bFLASHLIGHTSHADOWS = false;
#endif // _DEBUG
		m_nFLASHLIGHTSHADOWS = 0;
#ifdef _DEBUG
		m_bNUM_LIGHTS = false;
#endif // _DEBUG
		m_nNUM_LIGHTS = 0;
#ifdef _DEBUG
		m_bLIGHTMAPPED_MODEL = false;
#endif // _DEBUG
		m_nLIGHTMAPPED_MODEL = 0;
#ifdef _DEBUG
		m_bCASCADED_SHADOW = false;
#endif // _DEBUG
		m_nCASCADED_SHADOW = 0;
	}
	int GetIndex()
	{
		// Asserts to make sure that we aren't using any skipped combinations.
		// Asserts to make sure that we are setting all of the combination vars.
#ifdef _DEBUG
		bool bAllDynamicVarsDefined = m_bWRITEWATERFOGTODESTALPHA && m_bPIXELFOGTYPE && m_bWRITE_DEPTH_TO_DESTALPHA && m_bFLASHLIGHTSHADOWS && m_bNUM_LIGHTS && m_bLIGHTMAPPED_MODEL && m_bCASCADED_SHADOW;
		Assert( bAllDynamicVarsDefined );
#endif // _DEBUG
		return ( 1 * m_nWRITEWATERFOGTODESTALPHA ) + ( 2 * m_nPIXELFOGTYPE ) + ( 4 * m_nWRITE_DEPTH_TO_DESTALPHA ) + ( 8 * m_nFLASHLIGHTSHADOWS ) + ( 16 * m_nNUM_LIGHTS ) + ( 80 * m_nLIGHTMAPPED_MODEL ) + ( 80 * m_nCASCADED_SHADOW ) + 0;
	}
};
#define shaderDynamicTest_pbr_mrao_glass_ps30 psh_forgot_to_set_dynamic_WRITEWATERFOGTODESTALPHA + psh_forgot_to_set_dynamic_PIXELFOGTYPE + psh_forgot_to_set_dynamic_WRITE_DEPTH_TO_DESTALPHA + psh_forgot_to_set_dynamic_FLASHLIGHTSHADOWS + psh_forgot_to_set_dynamic_NUM_LIGHTS + psh_forgot_to_set_dynamic_LIGHTMAPPED_MODEL + psh_forgot_to_set_dynamic_CASCADED_SHADOW + 0
