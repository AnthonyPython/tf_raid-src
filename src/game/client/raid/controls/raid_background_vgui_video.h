//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: VGUI panel which can play back video, in-engine
//
//=============================================================================

#ifndef RD_VGUI_VIDEO_H
#define RD_VGUI_VIDEO_H
#ifdef _WIN32
#pragma once
#endif

#include "vgui_video.h"

class CRDVideoPanel : public VideoPanel
{
	DECLARE_CLASS_SIMPLE( CRDVideoPanel, VideoPanel);
public:

	CRDVideoPanel( vgui::Panel *parent, const char *panelName );
	~CRDVideoPanel();

	virtual void OnClose();
	virtual void OnKeyCodePressed( vgui::KeyCode code ){}
	virtual void ApplySettings( KeyValues *inResourceData );
	
	virtual void GetPanelPos( int &xpos, int &ypos );
	virtual void Shutdown();

	float GetStartDelay(){ return m_flStartAnimDelay; }
	float GetEndDelay(){ return m_flEndAnimDelay; }

	virtual bool BeginPlayback( const char *pFilename ) OVERRIDE;

	void SetLoop(bool loop) { m_bLoop = loop; }
	void PlayVideo();

	void StopVideo();
	void StartVideo();
	//We do our own because we need to know when we are visible
	// or not immediately so we can prevent the video from being purged when looping. -AnthonyH

	virtual void SetVisible(bool state);

	bool IsVideoValid() { return m_VideoMaterial ? true : false; }

	bool IsPlaying();

	void SetMedia(const char* pMedia)
	{
		if (pMedia && pMedia[0])
		{
			Q_strncpy(m_szMedia, pMedia, MAX_PATH);
		}
	}

	virtual void OnThink() OVERRIDE;

protected:
	virtual void ReleaseVideo();
	virtual void OnVideoOver();
	char			m_szMedia[MAX_PATH];

private:
	float m_flStartAnimDelay;
	float m_flEndAnimDelay;
	bool m_bLoop;
	bool m_bAudio;
};

#endif // TF_VGUI_VIDEO_H
