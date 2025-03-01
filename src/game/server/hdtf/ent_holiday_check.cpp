//===== Copyright � 1996 - 2005, Valve Corporation, All rights reserved. ========
//
// Purpose: Checks if its a specific holiday, then
//			fires an output when reached.
//
//===============================================================================

#include "cbase.h"
#include "filesystem.h"
#include <ctime>
#include "world.h"

enum Holiday
{
	NONE = 0,
	APRIL,
	HALLOWEEN,
	CHRISTMAS,
	ANNIVERSARY
};

unsigned int IsHoliday()
{
	time_t ltime = time(0);
	const time_t* ptime = &ltime;
	struct tm* today = localtime(ptime);
	if (today)
	{
		if ((today->tm_mon == 2 && today->tm_mday >= 1) || (today->tm_mon == 3 && today->tm_mday <= 31))
		{
			return Holiday::ANNIVERSARY;
		}
		else if ((today->tm_mon == 3 && today->tm_mday >= 12) || (today->tm_mon == 4 && today->tm_mday <= 31))
		{
			return Holiday::APRIL;
		}
		else if (today->tm_mon == 9 && today->tm_mday >= 1)
		{
			return Holiday::HALLOWEEN;
		}
		else if ((today->tm_mon == 11 && today->tm_mday >= 1) || (today->tm_mon == 0 && today->tm_mday <= 7))
		{
			return Holiday::CHRISTMAS;
		}
	}
	return Holiday::NONE;
}




class CHolidayEntity : public CLogicalEntity, public CAutoGameSystem
{
public:
	DECLARE_CLASS(CHolidayEntity, CLogicalEntity);
	DECLARE_DATADESC();

	// Input function
	//void Is_holiday(inputdata_t& inputData);
	virtual void Spawn(void);
	void IsHoliday_custom_check();
	//virtual void LevelInitPostEntity()
	//{
	//	IsHoliday_custom_check();
	//}

	void Is_Custom_Holiday(inputdata_t& inputData);
	void FireOutputCustom(void);

private:

	COutputEvent	m_OnRickMay, m_OnHalloween, m_OnChristmas, m_OnAnniversary, m_OnCustomHoliday;	// Output event

};


LINK_ENTITY_TO_CLASS(holiday_check, CHolidayEntity);

// Start of our data description for the class
BEGIN_DATADESC(CHolidayEntity)

// Links our input name from Hammer to our input member function
//DEFINE_INPUTFUNC(FIELD_VOID, "Is_holiday", Is_holiday),
DEFINE_INPUTFUNC(FIELD_STRING,"Is_Custom_Holiday", Is_Custom_Holiday),

// Links our output member to the output name used by Hammer
DEFINE_OUTPUT(m_OnRickMay, "OnRickMayHoliday"),
DEFINE_OUTPUT(m_OnHalloween, "OnHalloweenHoliday"),
DEFINE_OUTPUT(m_OnChristmas, "OnChristmasHoliday"),
DEFINE_OUTPUT(m_OnAnniversary, "Is_anniversary_holiday"),
DEFINE_OUTPUT(m_OnCustomHoliday,"OnCustomHoliday"),

END_DATADESC()


// Checks the date to fire a holiday specific output
// so basically either through spawning it in game or just starting a map that
// already has one in it. - Nbc66
void CHolidayEntity::Spawn()
{

	CLogicalEntity::Spawn();

	// Fire an output event
	switch(IsHoliday())
	{
		case Holiday::ANNIVERSARY:
			m_OnAnniversary.FireOutput(this, this);
		case Holiday::APRIL:
			m_OnRickMay.FireOutput(this, this);
			break;
		case Holiday::HALLOWEEN:
			m_OnHalloween.FireOutput(this, this);
			break;
		case Holiday::CHRISTMAS:
			m_OnChristmas.FireOutput(this, this);
			break;
		default:
			break;
	}

}

void CHolidayEntity::FireOutputCustom(void)
{
	m_OnCustomHoliday.FireOutput(this, this);
}


void CHolidayEntity::Is_Custom_Holiday(inputdata_t& inputData)
{
	CWorld* pWorld = GetWorldEntity();

	if (pWorld && pWorld->IsHoliday())
	{
		ConDColorMsg(Color(251, 175, 65, 255), "Holiday Name Input string: %s \n Holiday Name World Ent string: %s\n", inputData.value.String(), pWorld->GetHolidayName());

		if (!V_strcmp(pWorld->GetHolidayName(), inputData.value.String()))
		{
			m_OnCustomHoliday.FireOutput(this, this);
		}

	}
	else
	{
		ConDColorMsg(Color(251, 175, 65, 255), "No Active Holiday\nNo Output Fired From %s", this->GetEntityName().ToCStr());
	}
}
