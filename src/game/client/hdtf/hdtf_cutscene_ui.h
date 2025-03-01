#pragma once

#include "vgui_controls/Panel.h"
#include "video/ivideoservices.h"

enum EPlayState
{
	PLAY_NONE,
	PLAY_PLAYING,
	PLAY_PAUSED
};

struct FCutsceneSubtitle
{
	char tokenName[512];
	int timestamp;
	int duration;
};

struct FCutsceneTimingToolsData
{
	bool isEnabled = false;
	uint32 currentTokenIndex = 0;
	bool isRecording = false;
	int startFrame;

	char fileName[256];

	CUtlVector<FCutsceneSubtitle *> records;
};

class HDTFCutscenePlayer : public vgui::Panel
{
	DECLARE_CLASS_SIMPLE(HDTFCutscenePlayer, vgui::Panel);
public:
	HDTFCutscenePlayer();
	~HDTFCutscenePlayer();

	bool				StartPlayback(const char *videoName);
	void				TakeControl();

	void				SetSkipAble(bool canSkip = true) { m_bCanSkip = canSkip; }
	bool				IsSkipAble() { return m_bCanSkip; }

protected:
	virtual void		ApplySchemeSettings(vgui::IScheme *scheme);
	virtual void		Paint();
	virtual void		OnTick();
	virtual void		OnKeyCodeTyped(vgui::KeyCode code);
	virtual void		OnKeyCodePressed(vgui::KeyCode code);
	virtual void		OnKeyCodeReleased(vgui::KeyCode code);

	void				PlayPausePlayback();
	void				CancelPlayback();
	void				OnPlaybackFinish();
	void				BeginDispose();

	void				PrepareSubtitles(const char *videoName);
	void				ProcessSubtitles();
	void				DumpRecordedSubtitleTimings();

private:
	EPlayState			m_eState;

	IVideoMaterial		*m_VideoMaterial;
	IMaterial			*m_pMaterial;
	int					m_nPlaybackWidth;
	int					m_nPlaybackHeight;
	float				m_flVideoU;
	float				m_flVideoV;

	vgui::HFont			m_hFont;

	bool				m_bESCDown;
	float				m_flESCTimer;
	float				m_flHelpAlpha;

	wchar_t				m_pHelpSkip[128];
	wchar_t				m_pHelpPause[128];

	bool				m_bCanSkip;

	CUtlVector<FCutsceneSubtitle*> m_pSubtitles;
	bool				m_bSubtitlesAvailable;
	int					m_iCurrentSubtitleToken;

	FCutsceneTimingToolsData	m_SubtitleTimingData;
};