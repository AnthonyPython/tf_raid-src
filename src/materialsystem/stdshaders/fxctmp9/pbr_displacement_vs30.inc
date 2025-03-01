#include "shaderlib/cshader.h"
class pbr_displacement_vs30_Static_Index
{
private:
	int m_nSEAMLESS;
#ifdef _DEBUG
	bool m_bSEAMLESS;
#endif
public:
	void SetSEAMLESS( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nSEAMLESS = i;
#ifdef _DEBUG
		m_bSEAMLESS = true;
#endif
	}
	void SetSEAMLESS( bool i )
	{
		m_nSEAMLESS = i ? 1 : 0;
#ifdef _DEBUG
		m_bSEAMLESS = true;
#endif
	}
private:
	int m_nSEAMLESS_SECONDARY;
#ifdef _DEBUG
	bool m_bSEAMLESS_SECONDARY;
#endif
public:
	void SetSEAMLESS_SECONDARY( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nSEAMLESS_SECONDARY = i;
#ifdef _DEBUG
		m_bSEAMLESS_SECONDARY = true;
#endif
	}
	void SetSEAMLESS_SECONDARY( bool i )
	{
		m_nSEAMLESS_SECONDARY = i ? 1 : 0;
#ifdef _DEBUG
		m_bSEAMLESS_SECONDARY = true;
#endif
	}
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
	int m_nBLENDMODULATE_UV;
#ifdef _DEBUG
	bool m_bBLENDMODULATE_UV;
#endif
public:
	void SetBLENDMODULATE_UV( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nBLENDMODULATE_UV = i;
#ifdef _DEBUG
		m_bBLENDMODULATE_UV = true;
#endif
	}
	void SetBLENDMODULATE_UV( bool i )
	{
		m_nBLENDMODULATE_UV = i ? 1 : 0;
#ifdef _DEBUG
		m_bBLENDMODULATE_UV = true;
#endif
	}
public:
	pbr_displacement_vs30_Static_Index( )
	{
#ifdef _DEBUG
		m_bSEAMLESS = false;
#endif // _DEBUG
		m_nSEAMLESS = 0;
#ifdef _DEBUG
		m_bSEAMLESS_SECONDARY = false;
#endif // _DEBUG
		m_nSEAMLESS_SECONDARY = 0;
#ifdef _DEBUG
		m_bDETAILTEXTURE_UV = false;
#endif // _DEBUG
		m_nDETAILTEXTURE_UV = 0;
#ifdef _DEBUG
		m_bBLENDMODULATE_UV = false;
#endif // _DEBUG
		m_nBLENDMODULATE_UV = 0;
	}
	int GetIndex()
	{
		// Asserts to make sure that we aren't using any skipped combinations.
		// Asserts to make sure that we are setting all of the combination vars.
#ifdef _DEBUG
		bool bAllStaticVarsDefined = m_bSEAMLESS && m_bSEAMLESS_SECONDARY && m_bDETAILTEXTURE_UV && m_bBLENDMODULATE_UV;
		Assert( bAllStaticVarsDefined );
#endif // _DEBUG
		return ( 1 * m_nSEAMLESS ) + ( 2 * m_nSEAMLESS_SECONDARY ) + ( 4 * m_nDETAILTEXTURE_UV ) + ( 8 * m_nBLENDMODULATE_UV ) + 0;
	}
};
#define shaderStaticTest_pbr_displacement_vs30 vsh_forgot_to_set_static_SEAMLESS + vsh_forgot_to_set_static_SEAMLESS_SECONDARY + vsh_forgot_to_set_static_DETAILTEXTURE_UV + vsh_forgot_to_set_static_BLENDMODULATE_UV + 0
class pbr_displacement_vs30_Dynamic_Index
{
public:
	pbr_displacement_vs30_Dynamic_Index()
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
#define shaderDynamicTest_pbr_displacement_vs30 0
