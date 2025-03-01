#include "shaderlib/cshader.h"
class pbr_model_vs30_Static_Index
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
	int m_nLIGHTMAP_UV;
#ifdef _DEBUG
	bool m_bLIGHTMAP_UV;
#endif
public:
	void SetLIGHTMAP_UV( int i )
	{
		Assert( i >= 0 && i <= 0 );
		m_nLIGHTMAP_UV = i;
#ifdef _DEBUG
		m_bLIGHTMAP_UV = true;
#endif
	}
	void SetLIGHTMAP_UV( bool i )
	{
		m_nLIGHTMAP_UV = i ? 1 : 0;
#ifdef _DEBUG
		m_bLIGHTMAP_UV = true;
#endif
	}
private:
	int m_nDECAL;
#ifdef _DEBUG
	bool m_bDECAL;
#endif
public:
	void SetDECAL( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nDECAL = i;
#ifdef _DEBUG
		m_bDECAL = true;
#endif
	}
	void SetDECAL( bool i )
	{
		m_nDECAL = i ? 1 : 0;
#ifdef _DEBUG
		m_bDECAL = true;
#endif
	}
public:
	pbr_model_vs30_Static_Index( )
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
		m_bLIGHTMAP_UV = false;
#endif // _DEBUG
		m_nLIGHTMAP_UV = 0;
#ifdef _DEBUG
		m_bDECAL = false;
#endif // _DEBUG
		m_nDECAL = 0;
	}
	int GetIndex()
	{
		// Asserts to make sure that we aren't using any skipped combinations.
		// Asserts to make sure that we are setting all of the combination vars.
#ifdef _DEBUG
		bool bAllStaticVarsDefined = m_bDETAILTEXTURE_UV && m_bENVMAPMASK_UV && m_bLIGHTMAP_UV && m_bDECAL;
		Assert( bAllStaticVarsDefined );
#endif // _DEBUG
		return ( 4 * m_nDETAILTEXTURE_UV ) + ( 8 * m_nENVMAPMASK_UV ) + ( 16 * m_nLIGHTMAP_UV ) + ( 16 * m_nDECAL ) + 0;
	}
};
#define shaderStaticTest_pbr_model_vs30 vsh_forgot_to_set_static_DETAILTEXTURE_UV + vsh_forgot_to_set_static_ENVMAPMASK_UV + vsh_forgot_to_set_static_LIGHTMAP_UV + vsh_forgot_to_set_static_DECAL + 0
class pbr_model_vs30_Dynamic_Index
{
private:
	int m_nMORPHING;
#ifdef _DEBUG
	bool m_bMORPHING;
#endif
public:
	void SetMORPHING( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nMORPHING = i;
#ifdef _DEBUG
		m_bMORPHING = true;
#endif
	}
	void SetMORPHING( bool i )
	{
		m_nMORPHING = i ? 1 : 0;
#ifdef _DEBUG
		m_bMORPHING = true;
#endif
	}
private:
	int m_nSKINNING;
#ifdef _DEBUG
	bool m_bSKINNING;
#endif
public:
	void SetSKINNING( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nSKINNING = i;
#ifdef _DEBUG
		m_bSKINNING = true;
#endif
	}
	void SetSKINNING( bool i )
	{
		m_nSKINNING = i ? 1 : 0;
#ifdef _DEBUG
		m_bSKINNING = true;
#endif
	}
public:
	pbr_model_vs30_Dynamic_Index()
	{
#ifdef _DEBUG
		m_bMORPHING = false;
#endif // _DEBUG
		m_nMORPHING = 0;
#ifdef _DEBUG
		m_bSKINNING = false;
#endif // _DEBUG
		m_nSKINNING = 0;
	}
	int GetIndex()
	{
		// Asserts to make sure that we aren't using any skipped combinations.
		// Asserts to make sure that we are setting all of the combination vars.
#ifdef _DEBUG
		bool bAllDynamicVarsDefined = m_bMORPHING && m_bSKINNING;
		Assert( bAllDynamicVarsDefined );
#endif // _DEBUG
		return ( 1 * m_nMORPHING ) + ( 2 * m_nSKINNING ) + 0;
	}
};
#define shaderDynamicTest_pbr_model_vs30 vsh_forgot_to_set_dynamic_MORPHING + vsh_forgot_to_set_dynamic_SKINNING + 0
