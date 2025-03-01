#ifdef WIN32
#pragma once
#endif
#include "cbase.h"
#include "string_t.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifdef GAME_DLL


class CFModEnt : public CBaseEntity
{

public:
	
	DECLARE_SERVERCLASS();
	DECLARE_CLASS(CFModEnt, CBaseEntity);
	 // Don't actually type this line!

	DECLARE_DATADESC();

	int UpdateTransmitState()	// always send to all clients
	{
		return SetTransmitState(FL_EDICT_ALWAYS);
	}

	CNetworkVar(float, SetParam1);
	CNetworkVar(float, SetParam2);
	CNetworkVar(float, SetParam3);
	CNetworkVar(float, SetParam4);
	CNetworkVar(string_t, s_BankFileName);
	CNetworkVar(string_t, s_EventName);
	CNetworkVar(string_t, s_SetparamName1);
	CNetworkVar(string_t, s_SetparamName2);
	CNetworkVar(string_t, s_SetparamName3);
	CNetworkVar(string_t, s_SetparamName4);

	CNetworkVar(int, b_Started);
	
	void Activate(void);

	// Constructor
	CFModEnt()
	{

		//SetParam1 = 0;
		//SetParam2 = 0;
		//SetParam3 = 0;
		//SetParam4 = 0;

		//s_EventName = "\0";
		//s_BankFileName = "\0";
		//s_SetparamName1 = "\0";
		//s_SetparamName2 = "\0";
		//s_SetparamName3 = "\0";
		//s_SetparamName4 = "\0";

		//b_Started = false;
	
	}

	// Input function
	//void LoadBankFile(inputdata_t& inputData);
	//void SetEventName(inputdata_t& inputData);
	void SetParam1to(inputdata_t& inputData);
	void SetParam2to(inputdata_t& inputData);
	void SetParam3to(inputdata_t& inputData);
	void SetParam4to(inputdata_t& inputData);

	void Start(inputdata_t& inputdata) { b_Started = 0; b_Started = 1; };
	void Stop(inputdata_t& inputdata) { b_Started = 1; b_Started = 0; };

	virtual void Spawn();


	string_t m_sSourceEntName;
	CNetworkVar(EHANDLE, m_hSoundSource);	// entity from which the sound comes

private:




	//int	m_nThreshold;	// Count at which to fire our output
	//int	m_nCounter;	// Internal counter


	COutputEvent	m_OnThreshold;	// Output event when the counter reaches the threshold

};









LINK_ENTITY_TO_CLASS(logic_fmod_music, CFModEnt);


BEGIN_DATADESC(CFModEnt)

// For save/load
//DEFINE_FIELD(m_nCounter, FIELD_INTEGER),

// As above, and also links our member variable to a Hammer keyvalue
DEFINE_KEYFIELD(s_BankFileName, FIELD_STRING, "BankFile"),
//Set paramName
DEFINE_KEYFIELD(s_EventName    , FIELD_STRING, "Event_path"),
DEFINE_KEYFIELD(m_sSourceEntName, FIELD_STRING, "SourceEntityName"),
DEFINE_KEYFIELD(s_SetparamName1, FIELD_STRING, "SetparamName1"),
DEFINE_KEYFIELD(s_SetparamName2, FIELD_STRING, "SetparamName2"),
DEFINE_KEYFIELD(s_SetparamName3, FIELD_STRING, "SetparamName3"),
DEFINE_KEYFIELD(s_SetparamName4, FIELD_STRING, "SetparamName4"),

// Links our input name from Hammer to our input member function
DEFINE_INPUTFUNC(FIELD_FLOAT, "SetParameter1To", SetParam1to),

DEFINE_INPUTFUNC(FIELD_FLOAT, "SetParameter2To", SetParam2to),
DEFINE_INPUTFUNC(FIELD_FLOAT, "SetParameter3To", SetParam3to),
DEFINE_INPUTFUNC(FIELD_FLOAT, "SetParameter4To", SetParam4to),

//DEFINE_INPUTFUNC(FIELD_VOID, "LoadBank", LoadBankFile),
//DEFINE_INPUTFUNC(FIELD_VOID, "SetEventName", SetEventName),
DEFINE_INPUTFUNC(FIELD_VOID, "Start", Start),
DEFINE_INPUTFUNC(FIELD_VOID, "Stop", Stop),

// Links our output member variable to the output name used by Hammer
//DEFINE_OUTPUT(m_OnThreshold, "OnThreshold"),

DEFINE_FIELD(SetParam1, FIELD_FLOAT),
DEFINE_FIELD(SetParam2, FIELD_FLOAT),
DEFINE_FIELD(SetParam3, FIELD_FLOAT),
DEFINE_FIELD(SetParam4, FIELD_FLOAT),

END_DATADESC()


IMPLEMENT_SERVERCLASS_ST(CFModEnt, DT_FMOD_DATA)
SendPropFloat(SENDINFO(SetParam1), 0, SPROP_NOSCALE),
SendPropFloat(SENDINFO(SetParam2), 0, SPROP_NOSCALE),
SendPropFloat(SENDINFO(SetParam3), 0, SPROP_NOSCALE),
SendPropFloat(SENDINFO(SetParam4), 0, SPROP_NOSCALE),

SendPropInt(SENDINFO(b_Started)),

SendPropStringT(SENDINFO(s_EventName)),

SendPropStringT(SENDINFO(s_BankFileName)),


SendPropStringT(SENDINFO(s_SetparamName1)),
SendPropStringT(SENDINFO(s_SetparamName2)),
SendPropStringT(SENDINFO(s_SetparamName3)),
SendPropStringT(SENDINFO(s_SetparamName4)),

SendPropEHandle(SENDINFO(m_hSoundSource)),

END_NETWORK_TABLE()


//void CFModEnt::LoadBankFile(inputdata_t& inputData) 
//{ 
//	const char* bankfile = inputData.value.String();
//	Q_strcpy(STRING(s_BankFileName), bankfile);
//};
//void CFModEnt::SetEventName(inputdata_t& inputData) 
//{
//	const char* Eventname = inputData.value.String();
//	Q_strcpy(s_EventName.GetForModify(), Eventname);
//}
void CFModEnt::SetParam1to(inputdata_t& inputData) 
{
	SetParam1 = inputData.value.Float();
};
void CFModEnt::SetParam2to(inputdata_t& inputData) 
{
	SetParam2 = inputData.value.Float(); 
};
void CFModEnt::SetParam3to(inputdata_t& inputData) 
{ 
	SetParam3 = inputData.value.Float(); 
};
void CFModEnt::SetParam4to(inputdata_t& inputData) 
{ 
	SetParam4 = inputData.value.Float(); 
};


void CFModEnt::Spawn()
{
	DevMsg("SERVER FMOD ENTITY CREATED\n");
	//const char* SetparamName1 = s_SetparamName1.GetForModify();
	//SetparamName1;
	BaseClass::Spawn();
}

void CFModEnt::Activate(void)
{
	BaseClass::Activate();

	// Initialize sound source.  If no source was given, or source can't be found
	// then this is the source
	if (m_hSoundSource.Get() == NULL)
	{
		if (m_sSourceEntName != NULL_STRING)
		{
			m_hSoundSource = gEntList.FindEntityByName(NULL, m_sSourceEntName);
			
		}

		if (m_hSoundSource.Get() == NULL)
		{
			m_hSoundSource = this;
			
		}
		
	}
}

#endif // GAME_DLL

#ifdef CLIENT_DLL

#include "fmod/fmod_manager.h"

#include "fmod.hpp"

ConVar fmod_dissable_window_sound("fmod_dissable_window_sound", "0", FCVAR_CHEAT);


class C_FModEnt : public C_BaseEntity
{
public:

	
	DECLARE_CLASS(C_FModEnt, C_BaseEntity);
	DECLARE_CLIENTCLASS()
	DECLARE_DATADESC()
	
	

	C_FModEnt();
	~C_FModEnt();

	virtual void					Spawn(void);


	ConVar* MusicVolume;
	

	float SetParam1;
	float SetParam2;
	float SetParam3;
	float SetParam4;

	BOOL b_Started;

	char s_BankFileName[MAX_PATH];

	char s_EventName[MAX_PATH];

	char s_SetparamName1[MAX_PATH];
	char s_SetparamName2[MAX_PATH];
	char s_SetparamName3[MAX_PATH];
	char s_SetparamName4[MAX_PATH];

	virtual void OnDataChanged(DataUpdateType_t updateType);
	virtual void ClientThink();
	virtual void OnSave();
	virtual void OnRestore();
	
	EHANDLE m_hSoundSource;	// entity from which the sound comes


private:

	int TimeLinePos;
	int PlayBackState = 2;


	FMOD::Studio::EventDescription* EventDescription = NULL;
	FMOD::Studio::EventInstance* EventInstance = NULL;
	FMOD::Studio::Bank* Bank = NULL;
	FMOD::Studio::Bank* Bank_string = NULL;
	FMOD::Studio::Bank* Bank_master = NULL;
	FMOD_STUDIO_PLAYBACK_STATE state;
	FMOD_RESULT		result_studio_client;
	FMOD::Studio::System* pStudio_System_ent;

};

bool loadedBank = false;

bool eventdesccreated = false;

bool eventinstacncecreated = false;

bool AbelToStop = false;

bool startedallready = false;

bool window_active = true;

C_FModEnt::C_FModEnt()
{

	//s_SetparamName2;
	//s_SetparamName3;
	//s_SetparamName4;
	//
	EventDescription = NULL;
	EventInstance = NULL;
	Bank = NULL;
	loadedBank = false;
	eventdesccreated = false;
	eventinstacncecreated = false;
	//AbelToStop = false;
	//startedallready = false;
	Bank_string = FMODManager()->LoadBankFile(Bank_string, "Master.strings.bank");
	Bank_master = FMODManager()->LoadBankFile(Bank_master, "Master.bank");
}

C_FModEnt::~C_FModEnt()
{
	EventInstance->release();
	Bank_string->unload();
	Bank_master->unload();
	Bank->unload();
	EventDescription->releaseAllInstances();
}

IMPLEMENT_CLIENTCLASS_DT(C_FModEnt, DT_FMOD_DATA, CFModEnt)
RecvPropFloat(RECVINFO(SetParam1)),
RecvPropFloat(RECVINFO(SetParam2)),
RecvPropFloat(RECVINFO(SetParam3)),
RecvPropFloat(RECVINFO(SetParam4)),

//recieve started

RecvPropInt(RECVINFO(b_Started)),

// recieve string paramnames
RecvPropString(RECVINFO(s_BankFileName)),

RecvPropString(RECVINFO(s_EventName)),

RecvPropString(RECVINFO(s_SetparamName1), 0),
RecvPropString(RECVINFO(s_SetparamName2)),
RecvPropString(RECVINFO(s_SetparamName3)),
RecvPropString(RECVINFO(s_SetparamName4)),
RecvPropEHandle(RECVINFO(m_hSoundSource)),
END_RECV_TABLE()


BEGIN_DATADESC(C_FModEnt)
	DEFINE_FIELD(TimeLinePos,FIELD_INTEGER),
	DEFINE_FIELD(PlayBackState, FIELD_INTEGER),
END_DATADESC()

//Link a global entity name to this class (name used in Hammer etc.)
LINK_ENTITY_TO_CLASS(logic_fmod_music, C_FModEnt);

void C_FModEnt::OnDataChanged(DataUpdateType_t updateType)
{
	BaseClass::OnDataChanged(updateType);


	if (updateType == DATA_UPDATE_DATATABLE_CHANGED || updateType == DATA_UPDATE_CREATED)
	{

		SetNextClientThink(CLIENT_THINK_ALWAYS);
		MusicVolume = cvar->FindVar("snd_musicvolume");
		
		const char* notNull = "\0";
		if (STRING(s_BankFileName) != notNull && !loadedBank)
		{
			char* pszBankFileName = s_BankFileName;
			Bank = FMODManager()->LoadBankFile(Bank, pszBankFileName);
			Bank->loadSampleData();
			loadedBank = true;
		}

		if (STRING(s_BankFileName) != notNull && !eventdesccreated)
		{
			char* pszEventName = s_EventName;
			EventDescription = FMODManager()->GetEventDescription(pszEventName, EventDescription);
			eventdesccreated = true;
		}

		if (eventdesccreated && EventDescription && !eventinstacncecreated)
		{
			EventInstance = FMODManager()->GetEventInstance(EventDescription, EventInstance);
			eventinstacncecreated = true;
		}

		if (STRING(s_BankFileName) != notNull && STRING(s_SetparamName1) != notNull && EventInstance )
		{
			char* pszSetparamName1 = s_SetparamName1;
			EventInstance->setParameterByName(pszSetparamName1, SetParam1);
		}

		if (STRING(s_BankFileName) != notNull && STRING(s_SetparamName2) != notNull && EventInstance)
		{
			char* pszSetparamName2 = s_SetparamName2;
			EventInstance->setParameterByName(pszSetparamName2, SetParam2);
		}
		
		if (STRING(s_BankFileName) != notNull && STRING(s_SetparamName3) && EventInstance)
		{
			char* pszSetparamName3 = s_SetparamName3;
			EventInstance->setParameterByName(pszSetparamName3, SetParam3);
		}
		
		if (STRING(s_BankFileName) != notNull && STRING(s_SetparamName4) && EventInstance)
		{
			char* pszSetparamName4 = s_SetparamName4;
			EventInstance->setParameterByName(pszSetparamName4, SetParam4);
		}

		if (b_Started == 1 && EventInstance && !startedallready)
		{
			EventInstance->start();
			
			AbelToStop = true;
			startedallready = true;
		}

		if (b_Started == 0 && AbelToStop && startedallready)
		{
			EventInstance->stop(FMOD_STUDIO_STOP_ALLOWFADEOUT);
			AbelToStop = false;
			startedallready = false;
		}

		

	}
	
}

void C_FModEnt::ClientThink()
{

	if (MusicVolume && EventInstance && eventinstacncecreated)
	{
		if (window_active)
		{
			EventInstance->setVolume(MusicVolume->GetFloat());
		}
		EventInstance->getPlaybackState(&state);

		FMOD_3D_ATTRIBUTES attributes;
		Vector entityForward, entityRight, entityUp, entityVel = { 0,0,0 };

		GetVectors(&entityForward, &entityRight, &entityUp);
		attributes.forward = VectorSourceToFMOD(entityForward);
		attributes.up = VectorSourceToFMOD(entityUp);
		attributes.velocity = VectorSourceToFMOD(entityVel);

		Vector entityPos;

		if (m_hSoundSource.Get() != NULL)
		{
			entityPos = m_hSoundSource.Get()->GetAbsOrigin();
		}
		else
		{
			entityPos = GetAbsOrigin();
		}
		entityPos = ConvertUnitsToMeters(entityPos);

		attributes.position = VectorSourceToFMOD(entityPos);
		EventInstance->set3DAttributes(&attributes);

		if (state == FMOD_STUDIO_PLAYBACK_STOPPED && AbelToStop && startedallready)
		{
			b_Started = 0;
			AbelToStop = false;
			startedallready = false;
		}

		if (!engine->IsActiveApp() && !fmod_dissable_window_sound.GetBool())
		{
			EventInstance->setVolume(0);
			window_active = false;
		}
		else if(!window_active)
		{
			//EventInstance->setPaused(false);
			window_active = true;
		}

	}
	SetNextClientThink(CLIENT_THINK_ALWAYS);
	BaseClass::ClientThink();
}

void C_FModEnt::Spawn()
{
	
}

void C_FModEnt::OnRestore()
{
	if(EventInstance)
	EventInstance->release();

	const char* notNull = "\0";
	if (STRING(s_BankFileName) != notNull && !loadedBank)
	{
		char* pszBankFileName = s_BankFileName;
		Bank = FMODManager()->LoadBankFile(Bank, pszBankFileName);
		Bank->loadSampleData();
		loadedBank = true;
	}

	if (STRING(s_BankFileName) != notNull && !eventdesccreated)
	{
		char* pszEventName = s_EventName;
		EventDescription = FMODManager()->GetEventDescription(pszEventName, EventDescription);
		EventDescription->loadSampleData();
		FMODManager()->ForceLoadSampleData();
		eventdesccreated = true;
	}

	if (eventdesccreated && EventDescription && !eventinstacncecreated)
	{
		EventInstance = FMODManager()->GetEventInstance(EventDescription, EventInstance);
		eventinstacncecreated = true;
	}

	if (STRING(s_BankFileName) != notNull && STRING(s_SetparamName1) != notNull && EventInstance)
	{
		char* pszSetparamName1 = s_SetparamName1;
		EventInstance->setParameterByName(pszSetparamName1, SetParam1);
	}

	if (STRING(s_BankFileName) != notNull && STRING(s_SetparamName2) != notNull && EventInstance)
	{
		char* pszSetparamName2 = s_SetparamName2;
		EventInstance->setParameterByName(pszSetparamName2, SetParam2);
	}

	if (STRING(s_BankFileName) != notNull && STRING(s_SetparamName3) && EventInstance)
	{
		char* pszSetparamName3 = s_SetparamName3;
		EventInstance->setParameterByName(pszSetparamName3, SetParam3);
	}

	if (STRING(s_BankFileName) != notNull && STRING(s_SetparamName4) && EventInstance)
	{
		char* pszSetparamName4 = s_SetparamName4;
		EventInstance->setParameterByName(pszSetparamName4, SetParam4);
	}

	if (EventInstance && PlayBackState == FMOD_STUDIO_PLAYBACK_PLAYING)
	{
		result_studio_client = Bank->loadSampleData();

		FMODManager()->ForceLoadSampleData();

		result_studio_client = EventInstance->start();

		if (result_studio_client == FMOD_OK)
		{
			EventInstance->setTimelinePosition(TimeLinePos);
			b_Started = 1;
		}
	}
	BaseClass::OnRestore();
}

void C_FModEnt::OnSave()
{
	if (EventInstance) 
	{
		EventInstance->getTimelinePosition(&TimeLinePos);
		EventInstance->getPlaybackState(&state);
		if (state == FMOD_STUDIO_PLAYBACK_STOPPED)
		{
			PlayBackState = FMOD_STUDIO_PLAYBACK_STOPPED;
		}
		else
		{
			PlayBackState = FMOD_STUDIO_PLAYBACK_PLAYING;
		}
	}
	BaseClass::OnSave();

}


#endif // CLIENT_DLL
