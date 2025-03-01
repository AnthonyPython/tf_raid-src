#include "shaderlib/cshader.h"
class lux_sky_hdri_ps30_Static_Index
{
private:
	int m_nMODE;
#ifdef _DEBUG
	bool m_bMODE;
#endif
public:
	void SetMODE( int i )
	{
		Assert( i >= 0 && i <= 2 );
		m_nMODE = i;
#ifdef _DEBUG
		m_bMODE = true;
#endif
	}
	void SetMODE( bool i )
	{
		m_nMODE = i ? 1 : 0;
#ifdef _DEBUG
		m_bMODE = true;
#endif
	}
private:
	int m_nHALF;
#ifdef _DEBUG
	bool m_bHALF;
#endif
public:
	void SetHALF( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nHALF = i;
#ifdef _DEBUG
		m_bHALF = true;
#endif
	}
	void SetHALF( bool i )
	{
		m_nHALF = i ? 1 : 0;
#ifdef _DEBUG
		m_bHALF = true;
#endif
	}
private:
	int m_nSTRETCH;
#ifdef _DEBUG
	bool m_bSTRETCH;
#endif
public:
	void SetSTRETCH( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nSTRETCH = i;
#ifdef _DEBUG
		m_bSTRETCH = true;
#endif
	}
	void SetSTRETCH( bool i )
	{
		m_nSTRETCH = i ? 1 : 0;
#ifdef _DEBUG
		m_bSTRETCH = true;
#endif
	}
private:
	int m_nTOSRGB;
#ifdef _DEBUG
	bool m_bTOSRGB;
#endif
public:
	void SetTOSRGB( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nTOSRGB = i;
#ifdef _DEBUG
		m_bTOSRGB = true;
#endif
	}
	void SetTOSRGB( bool i )
	{
		m_nTOSRGB = i ? 1 : 0;
#ifdef _DEBUG
		m_bTOSRGB = true;
#endif
	}
public:
	lux_sky_hdri_ps30_Static_Index( )
	{
#ifdef _DEBUG
		m_bMODE = false;
#endif // _DEBUG
		m_nMODE = 0;
#ifdef _DEBUG
		m_bHALF = false;
#endif // _DEBUG
		m_nHALF = 0;
#ifdef _DEBUG
		m_bSTRETCH = false;
#endif // _DEBUG
		m_nSTRETCH = 0;
#ifdef _DEBUG
		m_bTOSRGB = false;
#endif // _DEBUG
		m_nTOSRGB = 0;
	}
	int GetIndex()
	{
		// Asserts to make sure that we aren't using any skipped combinations.
		// Asserts to make sure that we are setting all of the combination vars.
#ifdef _DEBUG
		bool bAllStaticVarsDefined = m_bMODE && m_bHALF && m_bSTRETCH && m_bTOSRGB;
		Assert( bAllStaticVarsDefined );
#endif // _DEBUG
		return ( 8 * m_nMODE ) + ( 24 * m_nHALF ) + ( 48 * m_nSTRETCH ) + ( 96 * m_nTOSRGB ) + 0;
	}
};
#define shaderStaticTest_lux_sky_hdri_ps30 psh_forgot_to_set_static_MODE + psh_forgot_to_set_static_HALF + psh_forgot_to_set_static_STRETCH + psh_forgot_to_set_static_TOSRGB + 0
class lux_sky_hdri_ps30_Dynamic_Index
{
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
public:
	lux_sky_hdri_ps30_Dynamic_Index()
	{
#ifdef _DEBUG
		m_bPIXELFOGTYPE = false;
#endif // _DEBUG
		m_nPIXELFOGTYPE = 0;
#ifdef _DEBUG
		m_bWRITE_DEPTH_TO_DESTALPHA = false;
#endif // _DEBUG
		m_nWRITE_DEPTH_TO_DESTALPHA = 0;
#ifdef _DEBUG
		m_bWRITEWATERFOGTODESTALPHA = false;
#endif // _DEBUG
		m_nWRITEWATERFOGTODESTALPHA = 0;
	}
	int GetIndex()
	{
		// Asserts to make sure that we aren't using any skipped combinations.
		// Asserts to make sure that we are setting all of the combination vars.
#ifdef _DEBUG
		bool bAllDynamicVarsDefined = m_bPIXELFOGTYPE && m_bWRITE_DEPTH_TO_DESTALPHA && m_bWRITEWATERFOGTODESTALPHA;
		Assert( bAllDynamicVarsDefined );
#endif // _DEBUG
		return ( 1 * m_nPIXELFOGTYPE ) + ( 2 * m_nWRITE_DEPTH_TO_DESTALPHA ) + ( 4 * m_nWRITEWATERFOGTODESTALPHA ) + 0;
	}
};
#define shaderDynamicTest_lux_sky_hdri_ps30 psh_forgot_to_set_dynamic_PIXELFOGTYPE + psh_forgot_to_set_dynamic_WRITE_DEPTH_TO_DESTALPHA + psh_forgot_to_set_dynamic_WRITEWATERFOGTODESTALPHA + 0
