//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: VGUI panel which can play back video, in-engine
//
//=============================================================================

#include "cbase.h"
#include <vgui/IVGui.h>
#include <vgui/ISurface.h>
#include <KeyValues.h>
#include "vgui_video.h"
#include "raid_background_vgui_video.h"
#include "engine/IEngineSound.h"


// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


DECLARE_BUILD_FACTORY(CRDVideoPanel);

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
CRDVideoPanel::CRDVideoPanel( vgui::Panel *parent, const char *panelName ) : VideoPanel( 0, 0, 50, 50 )
{
	SetParent( parent );
	SetProportional( true );
	SetKeyBoardInputEnabled( false );

	SetBlackBackground( false );

	// Must be intialized for safety
	m_szMedia[0] = '\0';

	m_flStartAnimDelay = 0.0f;
	m_flEndAnimDelay = 0.0f;
	m_bLoop = false;
	m_bAudio = false;
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
CRDVideoPanel::~CRDVideoPanel()
{
	ReleaseVideo();
}

void CRDVideoPanel::PlayVideo()
{
	BeginPlayback(m_szMedia);
}

void CRDVideoPanel::StopVideo()
{
	if (m_VideoMaterial)
	{
		m_VideoMaterial->StopVideo();
	}
}

void CRDVideoPanel::StartVideo()
{
	if (m_VideoMaterial)
	{
		m_VideoMaterial->StartVideo();
	}
}

void CRDVideoPanel::SetVisible(bool state)
{
	/*if (state == false)
	{
		StopVideo();
	}
	else
	{
		StartVideo();
	}*/
	BaseClass::SetVisible(state);

	


}

bool CRDVideoPanel::IsPlaying()
{
	if (m_VideoMaterial)
	{
		return m_VideoMaterial->IsVideoPlaying();
	}
	return false;
}

void CRDVideoPanel::OnThink()
{
	BaseClass::OnThink();
	if (m_VideoMaterial && m_bLoop)
	{
		if (m_VideoMaterial->Update() == true)
		{
			float cur_vidtime = m_VideoMaterial->GetCurrentVideoTime();
			int cur_vidframe = m_VideoMaterial->GetCurrentFrame();
			int cur_vidframecount = m_VideoMaterial->GetFrameCount();
			float dur_vidtime = m_VideoMaterial->GetVideoDuration();
			if ((cur_vidtime + 0.1) >= dur_vidtime)
			{
				Msg("current vid time: %f \n current frame: %i \n frame count: %i \n video duration: %f", cur_vidtime, cur_vidframe, cur_vidframecount, dur_vidtime);
				m_VideoMaterial->SetTime(0);
				Msg("current vid time: %f \n current frame: %i \n frame count: %i \n video duration: %f", cur_vidtime, cur_vidframe, cur_vidframecount, dur_vidtime);
				//m_VideoMaterial->StartVideo();
			}
			
			
			
		}

	}

}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void CRDVideoPanel::ReleaseVideo()
{
	enginesound->NotifyEndMoviePlayback();

	// Destroy any previously allocated video
	if ( g_pVideo && m_VideoMaterial != NULL )
	{
		g_pVideo->DestroyVideoMaterial( m_VideoMaterial );
		m_VideoMaterial = NULL;
	}
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void CRDVideoPanel::ApplySettings( KeyValues *inResourceData )
{
	BaseClass::ApplySettings( inResourceData );

	SetMedia( inResourceData->GetString( "video", "" ) );
	m_flStartAnimDelay = inResourceData->GetFloat( "start_delay", 0.0 );
	m_flEndAnimDelay = inResourceData->GetFloat( "end_delay", 0.0 );
	m_bLoop = inResourceData->GetBool( "loop", false );
	m_bAudio = inResourceData->GetBool( "audio", false );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CRDVideoPanel::GetPanelPos( int &xpos, int &ypos )
{
	vgui::ipanel()->GetAbsPos( GetVPanel(), xpos, ypos );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CRDVideoPanel::OnVideoOver()
{
	//BaseClass::OnVideoOver();
	if (m_bLoop)
	{
		PlayVideo();
	}
	
	//PostMessage( GetParent(), new KeyValues( "IntroFinished" ) );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CRDVideoPanel::OnClose()
{
	// intentionally skipping VideoPanel::OnClose()
	EditablePanel::OnClose();
}
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CRDVideoPanel::Shutdown()
{
	OnClose();
	ReleaseVideo();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CRDVideoPanel::BeginPlayback( const char *pFilename )
{

	// need working video services
	if (g_pVideo == NULL)
		return false;

	// Create a new video material
	if (m_VideoMaterial != NULL)
	{
		g_pVideo->DestroyVideoMaterial(m_VideoMaterial);
		m_VideoMaterial = NULL;
	}
	VideoPlaybackFlags::EVideoPlaybackFlags_t vidflags = m_bAudio ? VideoPlaybackFlags::LOOP_VIDEO | VideoPlaybackFlags::PRELOAD_VIDEO : VideoPlaybackFlags::LOOP_VIDEO | VideoPlaybackFlags::NO_AUDIO | VideoPlaybackFlags::PRELOAD_VIDEO;

	m_VideoMaterial = g_pVideo->CreateVideoMaterial("VideoMaterial", pFilename, "GAME",
		vidflags,
		VideoSystem::DETERMINE_FROM_FILE_EXTENSION, m_bAllowAlternateMedia);

	if (m_VideoMaterial == NULL)
		return false;

	if (m_VideoMaterial && m_bLoop)
	{

		m_VideoMaterial->SetLooping(true);
	}

	// We want to be the sole audio source
	// FIXME: This may not always be true!

	if (m_bAudio)
	{
		enginesound->NotifyBeginMoviePlayback();
	}
	

	int nWidth, nHeight;
	m_VideoMaterial->GetVideoImageSize(&nWidth, &nHeight);
	m_VideoMaterial->GetVideoTexCoordRange(&m_flU, &m_flV);
	m_pMaterial = m_VideoMaterial->GetMaterial();


	float flFrameRatio = ((float)GetWide() / (float)GetTall());
	float flVideoRatio = ((float)nWidth / (float)nHeight);

	if (flVideoRatio > flFrameRatio)
	{
		m_nPlaybackWidth = GetWide();
		m_nPlaybackHeight = (GetWide() / flVideoRatio);
	}
	else if (flVideoRatio < flFrameRatio)
	{
		m_nPlaybackWidth = (GetTall() * flVideoRatio);
		m_nPlaybackHeight = GetTall();
	}
	else
	{
		m_nPlaybackWidth = GetWide();
		m_nPlaybackHeight = GetTall();
	}

	return true;

	
}