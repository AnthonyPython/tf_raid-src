#include "shaderlib/cshader.h"
class pbr_brush_vs30_Static_Index
{
private:
	int m_nDETAILTEXTURE_UV;
#ifdef _DEBUG
	bool m_bDETAILTEXTURE_UV;
#endif
public:
	void SetDETAILTEXTURE_UV( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nDETAILTEXTURE_UV = i;
#ifdef _DEBUG
		m_bDETAILTEXTURE_UV = true;
#endif
	}
	void SetDETAILTEXTURE_UV( bool i )
	{
		m_nDETAILTEXTURE_UV = i ? 1 : 0;
#ifdef _DEBUG
		m_bDETAILTEXTURE_UV = true;
#endif
	}
private:
	int m_nENVMAPMASK_UV;
#ifdef _DEBUG
	bool m_bENVMAPMASK_UV;
#endif
public:
	void SetENVMAPMASK_UV( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nENVMAPMASK_UV = i;
#ifdef _DEBUG
		m_bENVMAPMASK_UV = true;
#endif
	}
	void SetENVMAPMASK_UV( bool i )
	{
		m_nENVMAPMASK_UV = i ? 1 : 0;
#ifdef _DEBUG
		m_bENVMAPMASK_UV = true;
#endif
	}
private:
	int m_nPARALLAXMAPPING;
#ifdef _DEBUG
	bool m_bPARALLAXMAPPING;
#endif
public:
	void SetPARALLAXMAPPING( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nPARALLAXMAPPING = i;
#ifdef _DEBUG
		m_bPARALLAXMAPPING = true;
#endif
	}
	void SetPARALLAXMAPPING( bool i )
	{
		m_nPARALLAXMAPPING = i ? 1 : 0;
#ifdef _DEBUG
		m_bPARALLAXMAPPING = true;
#endif
	}
public:
	pbr_brush_vs30_Static_Index( )
	{
#ifdef _DEBUG
		m_bDETAILTEXTURE_UV = false;
#endif // _DEBUG
		m_nDETAILTEXTURE_UV = 0;
#ifdef _DEBUG
		m_bENVMAPMASK_UV = false;
#endif // _DEBUG
		m_nENVMAPMASK_UV = 0;
#ifdef _DEBUG
		m_bPARALLAXMAPPING = false;
#endif // _DEBUG
		m_nPARALLAXMAPPING = 0;
	}
	int GetIndex()
	{
		// Asserts to make sure that we aren't using any skipped combinations.
		// Asserts to make sure that we are setting all of the combination vars.
#ifdef _DEBUG
		bool bAllStaticVarsDefined = m_bDETAILTEXTURE_UV && m_bENVMAPMASK_UV && m_bPARALLAXMAPPING;
		Assert( bAllStaticVarsDefined );
#endif // _DEBUG
		return ( 1 * m_nDETAILTEXTURE_UV ) + ( 2 * m_nENVMAPMASK_UV ) + ( 4 * m_nPARALLAXMAPPING ) + 0;
	}
};
#define shaderStaticTest_pbr_brush_vs30 vsh_forgot_to_set_static_DETAILTEXTURE_UV + vsh_forgot_to_set_static_ENVMAPMASK_UV + vsh_forgot_to_set_static_PARALLAXMAPPING + 0
class pbr_brush_vs30_Dynamic_Index
{
public:
	pbr_brush_vs30_Dynamic_Index()
	{
	}
	int GetIndex()
	{
		// Asserts to make sure that we aren't using any skipped combinations.
		// Asserts to make sure that we are setting all of the combination vars.
#ifdef _DEBUG
#endif // _DEBUG
		return 0;
	}
};
#define shaderDynamicTest_pbr_brush_vs30 0
