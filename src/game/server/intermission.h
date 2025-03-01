#include "cbase.h"

//-----------------------------------------------------------------------------
// Purpose: Intermission class
//-----------------------------------------------------------------------------
class CIntermissionView : public CBaseEntity
{
public:
	DECLARE_CLASS(CIntermissionView, CBaseEntity);

	void Spawn(void);
	bool KeyValue(const char *szKeyName, const char *szValue);
	void Enable(void);
	void FollowTarget(void);
	void Think();

	// Always transmit to clients so they know where to move the view to
	virtual int UpdateTransmitState();

	DECLARE_DATADESC();

private:
	CBasePlayer *pPlayer;
	EHANDLE m_hPlayer;
	EHANDLE m_hTarget;

	// used for moving the camera along a path (rail rides)
	float m_flWait;
	float m_flReturnTime;
	float m_flStopTime;
	float m_moveDistance;
	float m_targetSpeed;
	int	  m_state;
	Vector m_vecMoveDir;


	string_t m_iszTargetAttachment;
	int	  m_iAttachmentIndex;
	bool  m_bSnapToGoal;

	int   m_nPlayerButtons;
	int m_nOldTakeDamage;

private:
	COutputEvent m_OnEndFollow;
};