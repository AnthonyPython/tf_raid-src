#include "cbase.h"

#define SF_FORCE_NO_SKIP ( 1 << 0 )

class CPointCutscene : public CPointEntity
{
public:
	DECLARE_CLASS(CPointCutscene, CPointEntity);
	DECLARE_DATADESC();

	CPointCutscene()
	{}

	void InputStartScene(inputdata_t &inputData);
};

LINK_ENTITY_TO_CLASS(point_cutscene, CPointCutscene);

BEGIN_DATADESC(CPointCutscene)

DEFINE_INPUTFUNC(FIELD_STRING, "StartScene", InputStartScene),

END_DATADESC()

void CPointCutscene::InputStartScene(inputdata_t &inputData)
{
	const char *sceneName = inputData.value.String();

	CRecipientFilter filter;
	filter.AddAllPlayers();
	UserMessageBegin(filter, "CutsceneStart");
	WRITE_STRING(sceneName);
	WRITE_BOOL(!(m_spawnflags & SF_FORCE_NO_SKIP));
	MessageEnd();
}

#ifdef DEBUG
CON_COMMAND(hdtf_playcutscene, "Plays a cuscene. hdtf_playcutscene <scene name>")
{
	if (args.ArgC() < 1)
		return;

	CRecipientFilter filter;
	filter.AddAllPlayers();
	UserMessageBegin(filter, "CutsceneStart");
	WRITE_STRING(args[1]);
	MessageEnd();
}
#endif