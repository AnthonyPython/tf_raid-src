#include "cbase.h"

class CGetSkillEnt : public CLogicalEntity
{
public:
	DECLARE_CLASS(CGetSkillEnt, CLogicalEntity);
	DECLARE_DATADESC();

	void CheckSkill(inputdata_t& inputData);

private:

	COutputEvent m_onIsEasy, m_onIsMedium, m_onIsHard, m_CheckSkill;

};

LINK_ENTITY_TO_CLASS(difficulty_check, CGetSkillEnt)

BEGIN_DATADESC(CGetSkillEnt)


DEFINE_OUTPUT(m_onIsEasy, "IsEasy"),
DEFINE_OUTPUT(m_onIsMedium, "IsMedium"),
DEFINE_OUTPUT(m_onIsHard,"IsHard"),

DEFINE_INPUTFUNC(FIELD_VOID, "Check_Difficulty", CheckSkill),

END_DATADESC()


void CGetSkillEnt::CheckSkill(inputdata_t& inputData)
{
	ConVarRef skill("skill");

	const char* m_sDifficulty = NULL;

	if (skill.IsValid())
	{
		//Easy
		if (skill.GetInt() == 0 || skill.GetInt() == 1)
		{
			m_onIsEasy.FireOutput(this, this);
			m_sDifficulty = "Easy";
		}
		//Medium
		else if (skill.GetInt() == 2)
		{
			m_onIsMedium.FireOutput(this, this);
			m_sDifficulty = "Medium";
		}
		//Hard
		else if (skill.GetInt() == 3)
		{
			m_onIsHard.FireOutput(this, this);
			m_sDifficulty = "Hard";
		}

		if (skill.GetInt() <= 3)
		{
			DevMsg("Current Difficulty is %s\n", m_sDifficulty);
		}
		else
		{
			DevMsg("Unknown Skill Level, Current Skill level is = %d\n", skill.GetInt());
		}

	}

}