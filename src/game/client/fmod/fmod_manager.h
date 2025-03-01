#ifndef FMOD_MANAGER_H
#define FMOD_MANAGER_H
#ifdef _WIN32
#pragma once
#endif

#include "fmod.hpp"
#include "fmod_studio.hpp"

using namespace FMOD;

namespace
{
	inline Vector VectorFMODToSource(FMOD_VECTOR& vector)
	{
		Vector vec;

		vec.x = vector.z;
		vec.y = vector.x;
		vec.z = vector.y;

		return vec;
	}

	inline FMOD_VECTOR VectorSourceToFMOD(Vector& vector)
	{
		FMOD_VECTOR vec;

		vec.x = vector.y;
		vec.y = vector.x;
		vec.z = vector.z;

		return vec;
	}

	inline Vector ConvertUnitsToMeters(Vector& vector)
	{
		vector.x *= 0.01905;
		vector.y *= 0.01905;
		vector.z *= 0.01905;

		return vector;
	}

	inline Vector ConvertMetersToUnits(Vector& vector)
	{
		vector.x /= 0.01905;
		vector.y /= 0.01905;
		vector.z /= 0.01905;

		return vector;
	}


}

class CFMODManager
{
public:
	CFMODManager();
	~CFMODManager();

	void InitFMOD();
	void ExitFMOD();

	void FadeThink();
	virtual void Think();

	

	FMOD::Studio::Bank* LoadBankFile(Studio::Bank* bank, const char* FileName);
	FMOD::Studio::EventDescription* GetEventDescription(const char* EventPathOrID, FMOD::Studio::EventDescription* EventDescription);
	FMOD::Studio::EventInstance* GetEventInstance(FMOD::Studio::EventDescription* EventDescription, Studio::EventInstance* envinstance);
	void StartEventInstance(Studio::EventInstance* envinstance);
	void StopEventInstance(Studio::EventInstance* envinstance);
	void ForceLoadSampleData();

	bool IsSoundPlaying(const char* pathToFileFromSoundsFolder);
	bool IsChannelPlaying(int number);

	void PlayAmbientSound(const char* pathToFileFromSoundsFolder, bool fadeIn);
	void PlayLoopingMusic(ChannelGroup* pNewChannelGroup, const char* pLoopingMusic, const char* pIntroMusic = NULL, float flDelay = 0, bool fadeIn = false);
	void StopAmbientSound(ChannelGroup* pNewChannel, bool fadeOut);
	void PlayMusicEnd(ChannelGroup* pNewChannelGroup, const char* pLoopingMusic, bool bDelay = false, Channel* pLoopingChannel = null);
	void StopAllSound(void);
	void TransitionAmbientSounds(const char* pathToFileFromSoundsFolder);
	float GetSoundLenght(void);
	unsigned int GetSoundLenghtPCM(Sound* sound);

	float m_fDefaultVolume;
	void* GetFullPathToSound(const char* pathToFileFromModFolder, int* pLength);
	
	

private:
	void TestFmodEvent(void);
	const char* GetCurrentSoundName(void);

	const char* currentSound;
	const char* introSound;
	const char* loopingSound;
	const char* newSoundFileToTransitionTo;
	bool m_bShouldTransition;
	bool m_bFadeIn;
	bool m_bFadeOut;
	bool m_bIntro;
	float m_fFadeDelay;
	float m_flSongStart;

};

extern CFMODManager* FMODManager();

#endif //FMOD_MANAGER_H