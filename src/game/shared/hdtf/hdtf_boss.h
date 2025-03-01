#ifndef HDTF_BOSS_H
#define HDTF_BOSS_H

#ifdef _WIN32
#pragma once
#endif

#ifdef CLIENT_DLL

#include "c_ai_basenpc.h"

#else

#include "ai_basenpc.h"

#endif

#ifdef CLIENT_DLL

#define CAI_BaseNPC C_AI_BaseNPC
#define CHDTF_BossBase C_HDTF_BossBase
#define CHDTF_Boss C_HDTF_Boss

#endif

class CHDTF_BossBase
{
public:
	static CAI_BaseNPC *m_hBoss;
};

template<class BaseNPC>
class CHDTF_Boss : public BaseNPC, public CHDTF_BossBase
{
public:
	DECLARE_CLASS_NOFRIEND( CHDTF_Boss, BaseNPC );

	CHDTF_Boss( )
	{ }

#ifdef CLIENT_DLL

	virtual void Spawn( )
	{
		Assert( m_hBoss == NULL );
		BaseClass::Spawn( );
		m_hBoss = this;
	}

	virtual void UpdateOnRemove( )
	{
		BaseClass::UpdateOnRemove( );
		m_hBoss = NULL;
	}

#endif

private:
	CHDTF_Boss( const CHDTF_Boss & );
};

#endif // HDTF_BOSS_H
