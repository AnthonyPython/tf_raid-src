#include "cbase.h"
#include "fmod_manager.h"
#include "filesystem.h"
#include "teamplayroundbased_gamerules.h"
#include "KeyValues.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace FMOD;



ConVar* Volume; 

Studio::System* pStudio_System;
Studio::Bank* StringBankData;
Studio::Bank* MasterBankData;
Studio::Bank* TestBankData;
Studio::EventDescription* events;
Studio::EventInstance* envinstance;
System* pSystem;
Sound* pSound;
SoundGroup* pSoundGroup;
Channel* pChannel;
ChannelGroup* pChannelGroup;
FMOD_RESULT		result;
FMOD_RESULT		result_studio;

CFMODManager gFMODMng;
CFMODManager* FMODManager()
{
	return &gFMODMng;
}



CFMODManager::CFMODManager()
{
	m_fFadeDelay = 0.0;
	m_fDefaultVolume = 1.0f;
	m_flSongStart = 0.0f;
	newSoundFileToTransitionTo = "NULL";
	currentSound = "NULL";
	m_bShouldTransition = false;
	m_bFadeIn = false;
	m_bFadeOut = false;
}

CFMODManager::~CFMODManager()
{
	m_fFadeDelay = 0.0;
	m_fDefaultVolume = 0.0f;
	m_flSongStart = 0.0f;
	newSoundFileToTransitionTo = "NULL";
	currentSound = "NULL";
	m_bShouldTransition = false;
	m_bFadeIn = false;
	m_bFadeOut = false;
}

// Starts FMOD
void CFMODManager::InitFMOD(void)
{

	Volume = cvar->FindVar("volume");

	result = System_Create(&pSystem); // Create the main system object.

	

	unsigned int version;
	pSystem->getVersion(&version);

	Studio::System::create(&pStudio_System);

	result_studio = pStudio_System->initialize(100, FMOD_STUDIO_INIT_NORMAL | FMOD_STUDIO_INIT_LIVEUPDATE, FMOD_INIT_NORMAL, 0);

	if (result_studio == FMOD_OK && pStudio_System->isValid())
	{
		ConDColorMsg(Color(0, 128, 255, 255),"FMOD STUDIO CREATED SUCCESSFULLY, version %u\n", version);
	}
	else
	{
		ConDColorMsg(Color(0, 128, 255, 255),"FMOD STUDIO: Failed to initialize properly!\n");
	}

	if (result != FMOD_OK)
		Warning("FMOD ERROR: System creation failed!\n");
	else
		DevMsg("FMOD system successfully created.\n");


	result = pSystem->init(100, FMOD_INIT_NORMAL, 0);   // Initialize FMOD system.

	
	
	if (result != FMOD_OK)
	{
		Warning("FMOD ERROR: Failed to initialize properly!\n");
	}
	else
	{
		DevMsg("FMOD initialized successfully.\n");
		pSystem->set3DSettings(1.0f, 0.01905f,1.0f);
			
	}


	// Set 3D settings to factor hammer units to meters (will it be fine if I'll put that in here?)
	pSystem->set3DSettings(1.0f, 52.49343f, 1.0f);

		
}

void CFMODManager::ForceLoadSampleData()
{
	pStudio_System->flushSampleLoading();
}

// Stops FMOD
void CFMODManager::ExitFMOD(void)
{

	result_studio = pStudio_System->release();

	if (result_studio != FMOD_OK)
		Warning("FMOD STUDIO ERROR: System did not terminate properly!\n");
	else
		DevMsg("FMOD STUDIO system terminated successfully.\n");

	result = pSystem->release();

	if (result != FMOD_OK)
		Warning("FMOD ERROR: System did not terminate properly!\n");
	else
		DevMsg("FMOD system terminated successfully.\n");

}

// Returns the full path of a specified sound file in the /sounds folder
void* CFMODManager::GetFullPathToSound(const char* pathToFileFromModFolder, int* pLength)
{
	/*
		char* resultpath = new char[512];
	*/
	char fullpath[512];
	Q_snprintf(fullpath, sizeof(fullpath), "sound/%s", pathToFileFromModFolder);
	/*	filesystem->GetLocalPath(fullpath, fullpath, sizeof(fullpath));

		Q_snprintf(resultpath, 512, "%s", fullpath);
		// convert backwards slashes to forward slashes
		for (int i = 0; i < 512; i++)
		{
			if (resultpath[i] == '\\')
				resultpath[i] = '/';
		}

	*/
	void* buffer = NULL;

	int length = filesystem->ReadFileEx(fullpath, "GAME", &buffer, true, true);

	if (pLength)
	{
		*pLength = length;
	}

	return buffer;
}

// Returns the name of the current ambient sound being played
// If there is an error getting the name of the ambient sound or if no ambient sound is currently being played, returns "NULL"
const char* CFMODManager::GetCurrentSoundName(void)
{
	return currentSound;
}

// Handles all fade-related sound stuffs
// Called every frame when the client is in-game
void CFMODManager::FadeThink(void)
{
	if (m_bFadeOut)
	{
		if (gpGlobals->curtime >= m_fFadeDelay)
		{
			float tempvol;
			pChannel->getVolume(&tempvol);

			if (tempvol > 0.0)
			{
				pChannel->setVolume(tempvol - 0.05);
				m_fDefaultVolume = tempvol - 0.05f;
				m_fFadeDelay = gpGlobals->curtime + 0.1;
			}
			else
			{
				pChannel->setVolume(0.0);
				m_fDefaultVolume = 0.0f;
				m_bFadeOut = false;
				m_fFadeDelay = 0.0;
			}
		}
	}
	else if (m_bShouldTransition)
	{
		int iLenght = 0;
		void* buffer = GetFullPathToSound(newSoundFileToTransitionTo, &iLenght);

		FMOD_CREATESOUNDEXINFO info;
		memset(&info, 0, sizeof(info));
		info.length = iLenght;
		info.cbsize = sizeof(info);

		result = pSystem->createStream((const char*)buffer, FMOD_OPENMEMORY, &info, &pSound);

		if (result != FMOD_OK)
		{
			Warning("FMOD: Failed to create stream of sound '%s' ! (ERROR NUMBER: %i)\n", newSoundFileToTransitionTo, result);
			newSoundFileToTransitionTo = "NULL";
			m_bShouldTransition = false;
			return;
		}

		result = pSystem->playSound(pSound, pChannelGroup, false, &pChannel);

		if (result != FMOD_OK)
		{
			Warning("FMOD: Failed to play sound '%s' ! (ERROR NUMBER: %i)\n", newSoundFileToTransitionTo, result);
			newSoundFileToTransitionTo = "NULL";
			m_bShouldTransition = false;
			return;
		}

		currentSound = newSoundFileToTransitionTo;
		newSoundFileToTransitionTo = "NULL";
		m_bShouldTransition = false;
	}
	else if (m_bFadeIn)
	{
		if (gpGlobals->curtime >= m_fFadeDelay)
		{
			float tempvol;
			pChannel->getVolume(&tempvol);

			if (tempvol < 1.0)
			{
				pChannel->setVolume(tempvol + 0.05);
				m_fDefaultVolume = tempvol + 0.05f;
				m_fFadeDelay = gpGlobals->curtime + 0.1;
			}
			else
			{
				pChannel->setVolume(1.0);
				m_fDefaultVolume = 1.0f;
				m_bFadeIn = false;
				m_fFadeDelay = 0.0;
			}
		}
	}
}

const char* GetFullBankPath(const char* BankName)
{

	char* resultpath = new char[FILESYSTEM_MAX_SEARCH_PATHS];
	char* resultpathnew = new char[FILESYSTEM_MAX_SEARCH_PATHS];

	Q_snprintf(resultpath, FILESYSTEM_MAX_SEARCH_PATHS, "sound/fmod/Desktop/%s", BankName);

	filesystem->GetLocalPath(resultpath, resultpathnew, FILESYSTEM_MAX_SEARCH_PATHS);

	// convert backwards slashes to forward slashes
	for (int i = 0; i < FILESYSTEM_MAX_SEARCH_PATHS; i++)
	{
		if (resultpathnew[i] == '\\')
			resultpathnew[i] = '/';
	}

	return resultpathnew;
}

FMOD::Studio::Bank* CFMODManager::LoadBankFile(Studio::Bank* bank, const char* FileName)
{
	FMOD_RESULT result;
	const char* path = GetFullBankPath(FileName);
	result = pStudio_System->loadBankFile(path, FMOD_STUDIO_LOAD_BANK_NORMAL, &bank);
	if (result == FMOD_OK)
	{
		DevMsg("FMOD BANK OF NAME: %s LOADED\n", FileName);
		return bank;
	}
	else if (result == FMOD_ERR_EVENT_ALREADY_LOADED)
	{
		DevMsg("FMOD BANK %s IS ALREADY LOADED\n", FileName);
		return bank;
	}
	else
	{
		Warning("FMOD BANK NOT LOADED\n");
		return NULL;
	}
}

FMOD::Studio::EventDescription* CFMODManager::GetEventDescription(const char* EventPathOrID, FMOD::Studio::EventDescription* EventDescription)
{
	result_studio = pStudio_System->getEvent(EventPathOrID, &EventDescription);
	if (result_studio == FMOD_OK)
	{
		DevMsg("GOT FMOD EVENT OF NAME/ID: %s\n", EventPathOrID);
		return EventDescription;
	}
	else
	{
		DevMsg("FMOD EVENT FAILED TO BE FOUND: %s\n", EventPathOrID);
		return NULL;
	}
}

FMOD::Studio::EventInstance* CFMODManager::GetEventInstance(FMOD::Studio::EventDescription* EventDescription, Studio::EventInstance* envinstance)
{

	result_studio = EventDescription->createInstance(&envinstance);
	if (result_studio == FMOD_OK)
	{
		DevMsg("CREATED FMOD EVENT INSTANCE\n");
		return envinstance;
	}
	return NULL;

}

void CFMODManager::StartEventInstance(Studio::EventInstance* envinstance)
{
	envinstance->start();
}

void CFMODManager::StopEventInstance(Studio::EventInstance* envinstance)
{
	envinstance->stop(FMOD_STUDIO_STOP_ALLOWFADEOUT);
}


FMOD_STUDIO_LOADING_STATE BankLoadingState(Studio::Bank* bank)
{
	FMOD_STUDIO_LOADING_STATE loadingstate;
	bank->getLoadingState(&loadingstate);

	return loadingstate;
}

void TestFmodEvent(const CCommand& args)
{

	char fullpath[512];
	//Q_snprintf(fullpath, sizeof(fullpath), "sound/%s", "fmod/Desktop/Master.strings.bank");

	filesystem->GetLocalPath("sound/fmod/Desktop/Master.strings.bank",fullpath,sizeof(fullpath));

	Q_snprintf(fullpath, 512, "%s", fullpath);
	// convert backwards slashes to forward slashes
	for (int i = 0; i < 512; i++)
	{
		if (fullpath[i] == '\\')
			fullpath[i] = '/';
	}

	result_studio = pStudio_System->loadBankFile(fullpath, FMOD_STUDIO_LOAD_BANK_NORMAL, &StringBankData);

	const char* fullpathtest = GetFullBankPath("Master.bank");

	result_studio = pStudio_System->loadBankFile(fullpathtest, FMOD_STUDIO_LOAD_BANK_NORMAL, &MasterBankData);

	if (result_studio == FMOD_OK)
	{
		DevMsg("FMOD STUDIO BANK LOADED CORRECTLY\n");
	}
	else
	{
		Warning("FMOD STUDIO BANK FAILED TO LOAD, FILE: %s", fullpath);
	}

	while (result_studio == FMOD_ERR_NOTREADY)
	{
		result_studio = pStudio_System->update();
	}
	fullpathtest = GetFullBankPath("Music.bank");
	result_studio = pStudio_System->loadBankFile(fullpathtest, FMOD_STUDIO_LOAD_BANK_NORMAL, &TestBankData);

	if (result_studio == FMOD_OK)
	{
		DevMsg("FMOD STUDIO BANK LOADED CORRECTLY\n");
	}
	else
	{
		Warning("FMOD STUDIO BANK FAILED TO LOAD, FILE: %s", fullpath);
	}

	while (result_studio == FMOD_ERR_NOTREADY)
	{
		result_studio = pStudio_System->update();
	}

	result_studio = pStudio_System->getEvent("{c7f946fd-d695-499b-a820-752799c4921d}", &events);

	FMOD_STUDIO_LOADING_STATE state;

	do 
	{
		MasterBankData->getLoadingState(&state);
		
		pStudio_System->update();

	} while (state == FMOD_STUDIO_LOADING_STATE_LOADING);
	{
		pStudio_System->update();
	}

	

	do
	{
		TestBankData->getLoadingState(&state);

		pStudio_System->update();

	} while (state == FMOD_STUDIO_LOADING_STATE_LOADING);
	{
		pStudio_System->update();
	}
	
	if (result_studio == FMOD_OK)
	{
		DevMsg("FMOD EVENT LOADED CORRECTLY\n");
	}

	if (!envinstance)
	{
		events->createInstance(&envinstance);
	}
	

	//envinstance->setParameterByName("switch", atoi(args.Arg(1)));

	envinstance->start();
	//envinstance->release();

	result_studio = FMOD_OK;

}
ConCommand fmod_test_event("fmod_test_event", TestFmodEvent, "test if fmod events work");

void changeparam(const CCommand& args)
{
	result_studio = envinstance->setParameterByName("Area", atoi(args.Arg(1)));

	if (result_studio != FMOD_OK)
	{
		DevMsg("shit broke\n");
	}
}
ConCommand fmod_change_param("fmod_change_param", changeparam, "change event param");



// Called every frame when the client is in-game
void CFMODManager::Think(void)
{
	
	CBasePlayer* player = CBasePlayer::GetLocalPlayer();

	// Get Player Position

	Vector playerPos = { 0,0,0 };

	Vector playerForward, playerRight, playerUp, playerVel = { 0,0,0 };

	if (player)
	{
		playerPos = player->GetAbsOrigin();

		player->GetVectors(&playerForward, &playerRight, &playerUp);

		playerVel = player->GetAbsVelocity();
	}

	FMOD_3D_ATTRIBUTES attributes = { {0} };
	attributes.forward = VectorSourceToFMOD(playerForward);
	attributes.up = VectorSourceToFMOD(playerUp);
	playerPos = ConvertUnitsToMeters(playerPos);
	attributes.position = VectorSourceToFMOD(playerPos);
	attributes.velocity = VectorSourceToFMOD(playerVel);

	pStudio_System->setListenerAttributes(0, &attributes);

	pStudio_System->update();
	if (envinstance && Volume)
	{
		envinstance->setVolume(Volume->GetFloat());
	}

}

// Compares specified ambient sound with the current ambient sound being played
// Returns true if they match, false if they do not or if no sound is being played
bool CFMODManager::IsSoundPlaying(const char* pathToFileFromSoundsFolder)
{
	const char* currentSoundPlaying = GetCurrentSoundName();

	return (strcmp(currentSoundPlaying, pathToFileFromSoundsFolder) == 0 &&
		m_flSongStart + GetSoundLenght() < gpGlobals->realtime);
}

ConVar test_sample("test_sample", "1", FCVAR_ARCHIVE);

// Compares specified ambient sound with the current ambient sound being played
// Returns true if they match, false if they do not or if no sound is being played
bool CFMODManager::IsChannelPlaying(int number)
{
	unsigned int templenght = 0;
	unsigned int position = 0;
	pSound->getLength(&templenght, FMOD_TIMEUNIT_RAWBYTES);
	pChannel->getPosition(&position, FMOD_TIMEUNIT_RAWBYTES);
	return position < (templenght - test_sample.GetInt());
}

// Compares specified ambient sound with the current ambient sound being played
// Returns true if they match, false if they do not or if no sound is being played
float CFMODManager::GetSoundLenght(void)
{
	if (!pSound)
		return 0;

	unsigned int templenght = 0;
	pSound->getLength(&templenght, FMOD_TIMEUNIT_MS);

	float lenght = 0;
	if (templenght)
		lenght = templenght * 0.001f;

	return lenght;
}

// Compares specified ambient sound with the current ambient sound being played
// Returns true if they match, false if they do not or if no sound is being played
unsigned int CFMODManager::GetSoundLenghtPCM(Sound* sound)
{
	unsigned int templenght = 0;
	sound->getLength(&templenght, FMOD_TIMEUNIT_PCM);
	return templenght - 1;
}

// Abruptly starts playing a specified ambient sound
// In most cases, we'll want to use TransitionAmbientSounds instead
void CFMODManager::PlayLoopingMusic(ChannelGroup* pNewChannelGroup, const char* pLoopingMusic, const char* pIntroMusic, float flDelay, bool fadeIn)
{
	Channel* pTempChannel;
	Channel* pTempLoopChannel;

	if (!pNewChannelGroup)
		return;

	int  outputrate = 0;
	result = pSystem->getSoftwareFormat(&outputrate, 0, 0);
	unsigned int dsp_block_len = 0;
	result = pSystem->getDSPBufferSize(&dsp_block_len, 0);
	unsigned long long clock_start = 0;
	float freq = 0;
	unsigned int slen = 0;

	if (pIntroMusic)
	{
		Sound* pIntroSound = NULL;
		Sound* pLoopingSound = NULL;

		int iLenghtIntro = 0;
		void* vBufferIntro = GetFullPathToSound(pIntroMusic, &iLenghtIntro);

		FMOD_CREATESOUNDEXINFO infoIntro;
		memset(&infoIntro, 0, sizeof(infoIntro));
		infoIntro.length = iLenghtIntro;
		infoIntro.cbsize = sizeof(infoIntro);

		result = pSystem->createStream((const char*)vBufferIntro, FMOD_OPENMEMORY | FMOD_CREATESTREAM, &infoIntro, &pIntroSound);
		pSystem->playSound(pIntroSound, pNewChannelGroup, true, &pTempChannel);

		result = pIntroSound->getDefaults(&freq, 0);

		result = pTempChannel->getDSPClock(0, &clock_start);

		clock_start += (dsp_block_len * 2);

		slen = (unsigned int)(flDelay * 30000.0f);
		slen = (unsigned int)((float)slen / freq * outputrate);
		DevMsg("Initial delay lenght is %d\n", slen);
		clock_start += slen;

		result = pTempChannel->setDelay(clock_start, 0, false);
		result = pTempChannel->setPaused(false);

		result = pLoopingSound->setLoopPoints(0, FMOD_TIMEUNIT_PCM, GetSoundLenghtPCM(pLoopingSound), FMOD_TIMEUNIT_PCM);

		int iLenghtLoop = 0;
		void* vBufferLoop = GetFullPathToSound(pLoopingMusic, &iLenghtLoop);

		FMOD_CREATESOUNDEXINFO infoLoop;
		memset(&infoLoop, 0, sizeof(infoLoop));
		infoLoop.length = iLenghtLoop;
		infoLoop.cbsize = sizeof(infoLoop);

		result = pSystem->createStream((const char*)vBufferLoop, FMOD_OPENMEMORY | FMOD_LOOP_NORMAL | FMOD_CREATESTREAM, &infoLoop, &pLoopingSound);
		pSystem->playSound(pLoopingSound, pNewChannelGroup, true, &pTempLoopChannel);

		result = pIntroSound->getLength(&slen, FMOD_TIMEUNIT_PCM);
		result = pIntroSound->getDefaults(&freq, 0);

		slen = (unsigned int)((float)slen / freq * outputrate);
		DevMsg("Intro delay lenght is %d\n", slen);
		clock_start += slen;

		result = pTempLoopChannel->setDelay(clock_start, 0, false);
		result = pTempLoopChannel->setPaused(false);
		pTempChannel->setChannelGroup(pNewChannelGroup);
		pTempLoopChannel->setChannelGroup(pNewChannelGroup);

	}
	else
	{
		int iLenghtLoop = 0;
		void* vBufferLoop = GetFullPathToSound(pLoopingMusic, &iLenghtLoop);

		FMOD_CREATESOUNDEXINFO infoLoop;
		memset(&infoLoop, 0, sizeof(infoLoop));
		infoLoop.length = iLenghtLoop;
		infoLoop.cbsize = sizeof(infoLoop);

		result = pSound->setLoopPoints(0, FMOD_TIMEUNIT_PCM, GetSoundLenghtPCM(pSound), FMOD_TIMEUNIT_PCM);
		result = pSystem->createStream((const char*)vBufferLoop, FMOD_OPENMEMORY | FMOD_LOOP_NORMAL | FMOD_CREATESTREAM, &infoLoop, &pSound);
		if (result != FMOD_OK)
		{
			Warning("FMOD: Failed to create stream of sound '%s' ! (ERROR NUMBER: %i)\n", pLoopingMusic, result);
			return;
		}
		result = pSystem->playSound(pSound, pNewChannelGroup, true, &pTempChannel);

		result = pTempChannel->getDSPClock(0, &clock_start);

		clock_start += (dsp_block_len * 2);
		pSound->getDefaults(&freq, 0);
		slen = (unsigned int)(flDelay * 30000.0f);
		slen = (unsigned int)((float)slen / freq * outputrate);
		DevMsg("Initial delay lenght is %d\n", slen);
		clock_start += slen;

		result = pTempChannel->setDelay(clock_start, 0, false);

		pTempChannel->setPosition(0, FMOD_TIMEUNIT_PCM);
		pTempChannel->setPaused(false);
		if (result != FMOD_OK)
		{
			Warning("FMOD: Failed to play sound '%s' ! (ERROR NUMBER: %i)\n", pLoopingMusic, result);
			return;
		}
		pTempChannel->setChannelGroup(pNewChannelGroup);
	}
	//	result = pSound->setLoopCount( -1 );
	//	
	m_flSongStart = gpGlobals->realtime;
	currentSound = pLoopingMusic;
}

//ChannelGroup* pNewChannelGroup;

void testsound( const CCommand& args )
{
	pSystem->createChannelGroup("test", &pChannelGroup);
	FMODManager()->PlayLoopingMusic( pChannelGroup, args.ArgS());
}

ConCommand fmod_test_sound("fmod_test_sound", testsound, "test a sound in the fmod system", FCVAR_CHEAT);

// Abruptly starts playing a specified ambient sound
// In most cases, we'll want to use TransitionAmbientSounds instead
void CFMODManager::PlayAmbientSound(const char* pathToFileFromSoundsFolder, bool fadeIn)
{
	int iLenghtLoop = 0;
	void* vBufferLoop = GetFullPathToSound(pathToFileFromSoundsFolder, &iLenghtLoop);

	FMOD_CREATESOUNDEXINFO infoLoop;
	memset(&infoLoop, 0, sizeof(infoLoop));
	infoLoop.length = iLenghtLoop;
	infoLoop.cbsize = sizeof(infoLoop);

	result = pSystem->createStream((const char*)vBufferLoop, FMOD_OPENMEMORY, &infoLoop, &pSound);

	if (result != FMOD_OK)
	{
		Warning("FMOD: Failed to create stream of sound '%s' ! (ERROR NUMBER: %i)\n", pathToFileFromSoundsFolder, result);
		return;
	}

	m_flSongStart = gpGlobals->realtime;
	result = pSystem->playSound(pSound, pChannelGroup, false, &pChannel);

	if (result != FMOD_OK)
	{
		Warning("FMOD: Failed to play sound '%s' ! (ERROR NUMBER: %i)\n", pathToFileFromSoundsFolder, result);
		return;
	}

	if (fadeIn)
	{
		pChannel->setVolume(0.0);
		m_fDefaultVolume = 0.0f;
		m_bFadeIn = true;
	}
	else
	{
		m_fDefaultVolume = 1.0f;
	}

	currentSound = pathToFileFromSoundsFolder;
}

void CFMODManager::StopAmbientSound(ChannelGroup* pNewChannel, bool fadeOut)
{
	if (fadeOut)
	{
		pNewChannel->setVolume(1.0f);

		m_fDefaultVolume = 1.0f;
		m_bFadeOut = true;
	}
	else
	{
		pNewChannel->setVolume(0.0f);
		pNewChannel->stop();
	}

	currentSound = "NULL";
}

void CFMODManager::PlayMusicEnd(ChannelGroup* pNewChannelGroup, const char* pLoopingMusic, bool bDelay, Channel* pLoopingChannel)
{
	Channel* pTempChannel;

	if (!pNewChannelGroup)
		return;

	if (pLoopingMusic)
	{
		Sound* pIntroSound = NULL;

		int iLenght = 0;
		void* vBuffer = GetFullPathToSound(pLoopingMusic, &iLenght);

		FMOD_CREATESOUNDEXINFO info;
		memset(&info, 0, sizeof(info));
		info.length = iLenght;
		info.cbsize = sizeof(info);

		result = pSystem->createStream((const char*)vBuffer, FMOD_CREATESTREAM | FMOD_OPENMEMORY, &info, &pIntroSound);
		pSystem->playSound(pIntroSound, pNewChannelGroup, true, &pTempChannel);

		result = pTempChannel->setPaused(false);
		pTempChannel->setChannelGroup(pNewChannelGroup);
	}

}

void CommSndStp(void)
{
	FMODManager()->StopAllSound();
}
static ConCommand fm_stop_all_sound("fmod_stop_all_sound", CommSndStp, "Stops every channel group", FCVAR_NONE);

// Abruptly stops playing all ambient sounds
void CFMODManager::StopAllSound(void)
{
	//	pChannelGroup->setVolume(0.0f);
	pChannelGroup->stop();
	envinstance->stop(FMOD_STUDIO_STOP_ALLOWFADEOUT);
	//	pChannelGroup->setPaused(true);
}

// Transitions between two ambient sounds if necessary
// If a sound isn't already playing when this is called, don't worry about it
void CFMODManager::TransitionAmbientSounds(const char* pathToFileFromSoundsFolder)
{
	pChannel->setVolume(1.0);
	m_fDefaultVolume = 1.0f;
	newSoundFileToTransitionTo = pathToFileFromSoundsFolder;

	m_bFadeOut = true;
	m_bShouldTransition = true;
	m_bFadeIn = true;
}