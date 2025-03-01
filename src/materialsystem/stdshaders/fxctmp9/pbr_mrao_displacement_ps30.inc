#include "shaderlib/cshader.h"
class pbr_mrao_displacement_ps30_Static_Index
{
private:
	int m_nFLASHLIGHT;
#ifdef _DEBUG
	bool m_bFLASHLIGHT;
#endif
public:
	void SetFLASHLIGHT( int i )
	{
		Assert( i >= 0 && i <= 1 );
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
	int m_nDETAILTEXTURE;
#ifdef _DEBUG
	bool m_bDETAILTEXTURE;
#endif
public:
	void SetDETAILTEXTURE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nDETAILTEXTURE = i;
#ifdef _DEBUG
		m_bDETAILTEXTURE = true;
#endif
	}
	void SetDETAILTEXTURE( bool i )
	{
		m_nDETAILTEXTURE = i ? 1 : 0;
#ifdef _DEBUG
		m_bDETAILTEXTURE = true;
#endif
	}
private:
	int m_nEMISSION;
#ifdef _DEBUG
	bool m_bEMISSION;
#endif
public:
	void SetEMISSION( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nEMISSION = i;
#ifdef _DEBUG
		m_bEMISSION = true;
#endif
	}
	void SetEMISSION( bool i )
	{
		m_nEMISSION = i ? 1 : 0;
#ifdef _DEBUG
		m_bEMISSION = true;
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
		Assert( i >= 0 && i <= 1 );
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
	int m_nPCC;
#ifdef _DEBUG
	bool m_bPCC;
#endif
public:
	void SetPCC( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nPCC = i;
#ifdef _DEBUG
		m_bPCC = true;
#endif
	}
	void SetPCC( bool i )
	{
		m_nPCC = i ? 1 : 0;
#ifdef _DEBUG
		m_bPCC = true;
#endif
	}
private:
	int m_nPARALLAXINTERVAL;
#ifdef _DEBUG
	bool m_bPARALLAXINTERVAL;
#endif
public:
	void SetPARALLAXINTERVAL( int i )
	{
		Assert( i >= 0 && i <= 0 );
		m_nPARALLAXINTERVAL = i;
#ifdef _DEBUG
		m_bPARALLAXINTERVAL = true;
#endif
	}
	void SetPARALLAXINTERVAL( bool i )
	{
		m_nPARALLAXINTERVAL = i ? 1 : 0;
#ifdef _DEBUG
		m_bPARALLAXINTERVAL = true;
#endif
	}
public:
	pbr_mrao_displacement_ps30_Static_Index( )
	{
#ifdef _DEBUG
		m_bFLASHLIGHT = false;
#endif // _DEBUG
		m_nFLASHLIGHT = 0;
#ifdef _DEBUG
		m_bDETAILTEXTURE = false;
#endif // _DEBUG
		m_nDETAILTEXTURE = 0;
#ifdef _DEBUG
		m_bEMISSION = false;
#endif // _DEBUG
		m_nEMISSION = 0;
#ifdef _DEBUG
		m_bSPECIALPROPERTIES = false;
#endif // _DEBUG
		m_nSPECIALPROPERTIES = 0;
#ifdef _DEBUG
		m_bPCC = false;
#endif // _DEBUG
		m_nPCC = 0;
#ifdef _DEBUG
		m_bPARALLAXINTERVAL = false;
#endif // _DEBUG
		m_nPARALLAXINTERVAL = 0;
	}
	int GetIndex()
	{
		// Asserts to make sure that we aren't using any skipped combinations.
		// Asserts to make sure that we are setting all of the combination vars.
#ifdef _DEBUG
		bool bAllStaticVarsDefined = m_bFLASHLIGHT && m_bDETAILTEXTURE && m_bEMISSION && m_bSPECIALPROPERTIES && m_bPCC && m_bPARALLAXINTERVAL;
		Assert( bAllStaticVarsDefined );
#endif // _DEBUG
		return ( 16 * m_nFLASHLIGHT ) + ( 32 * m_nDETAILTEXTURE ) + ( 64 * m_nEMISSION ) + ( 128 * m_nSPECIALPROPERTIES ) + ( 256 * m_nPCC ) + ( 512 * m_nPARALLAXINTERVAL ) + 0;
	}
};
#define shaderStaticTest_pbr_mrao_displacement_ps30 psh_forgot_to_set_static_FLASHLIGHT + psh_forgot_to_set_static_DETAILTEXTURE + psh_forgot_to_set_static_EMISSION + psh_forgot_to_set_static_SPECIALPROPERTIES + psh_forgot_to_set_static_PCC + psh_forgot_to_set_static_PARALLAXINTERVAL + 0
class pbr_mrao_displacement_ps30_Dynamic_Index
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
	pbr_mrao_displacement_ps30_Dynamic_Index()
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
		m_bCASCADED_SHADOW = false;
#endif // _DEBUG
		m_nCASCADED_SHADOW = 0;
	}
	int GetIndex()
	{
		// Asserts to make sure that we aren't using any skipped combinations.
		// Asserts to make sure that we are setting all of the combination vars.
#ifdef _DEBUG
		bool bAllDynamicVarsDefined = m_bWRITEWATERFOGTODESTALPHA && m_bPIXELFOGTYPE && m_bWRITE_DEPTH_TO_DESTALPHA && m_bFLASHLIGHTSHADOWS && m_bCASCADED_SHADOW;
		Assert( bAllDynamicVarsDefined );
#endif // _DEBUG
		return ( 1 * m_nWRITEWATERFOGTODESTALPHA ) + ( 2 * m_nPIXELFOGTYPE ) + ( 4 * m_nWRITE_DEPTH_TO_DESTALPHA ) + ( 8 * m_nFLASHLIGHTSHADOWS ) + ( 16 * m_nCASCADED_SHADOW ) + 0;
	}
};
#define shaderDynamicTest_pbr_mrao_displacement_ps30 psh_forgot_to_set_dynamic_WRITEWATERFOGTODESTALPHA + psh_forgot_to_set_dynamic_PIXELFOGTYPE + psh_forgot_to_set_dynamic_WRITE_DEPTH_TO_DESTALPHA + psh_forgot_to_set_dynamic_FLASHLIGHTSHADOWS + psh_forgot_to_set_dynamic_CASCADED_SHADOW + 0
