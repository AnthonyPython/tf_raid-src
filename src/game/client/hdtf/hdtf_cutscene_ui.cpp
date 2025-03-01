#include "cbase.h"
#include "hdtf_cutscene_ui.h"
#include "engine/IEngineSound.h"
#include "filesystem.h"
#include "ienginevgui.h"
#include "vgui/ISurface.h"
#include "vgui/IInput.h"
#include "vgui/ILocalize.h"
#include <vgui/IVGui.h>
#include "GameUI/hdtf_ui.h"
#include "input.h"
#include "usermessages.h"
#include "hud_closecaption.h"
#include "iclientmode.h"

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

#define TIME_TO_CLOSE	0.80f
#define TIME_TO_FADE	0.65f

static ConVar snd_cutscene_volume("snd_cutscene_volume", "0.5", FCVAR_CLIENTDLL | FCVAR_ARCHIVE, "Cutscene volume");
static ConVar hdtf_cutscenetool_linger_to_third("hdtf_cutscenetool_linger_to_third", "1", FCVAR_CLIENTDLL | FCVAR_ARCHIVE, "Whether or not cutscene tool should remap subtitle duration to match start time of one after the next subtitle.");
static ConVar hdtf_cutscenetool_leeway_time("hdtf_cutscenetool_leeway_time", "0.5", FCVAR_CLIENTDLL | FCVAR_ARCHIVE, "If hdtf_cutscenetool_linger_to_third is used, this is the time we should subtract from third's start time. In seconds.");
static ConVar hdtf_cutscenetool_start_offset("hdtf_cutscenetool_start_offset", "-10", FCVAR_CLIENTDLL | FCVAR_ARCHIVE, "Number of frames to add to start of the recorded subtitle block.");

HDTFCutscenePlayer::HDTFCutscenePlayer() : BaseClass(NULL)
{
	vgui::VPANEL pParent = enginevgui->GetPanel(PANEL_CLIENTDLL);
	SetParent(pParent);

	SetKeyBoardInputEnabled(true);
	SetMouseInputEnabled(false);

	SetProportional(false);
	SetVisible(true);
	SetPaintBackgroundEnabled(false);
	SetPaintBorderEnabled(false);

	SetCursor(NULL);

	int wide, tall;
	vgui::surface()->GetScreenSize(wide, tall);
	SetSize(wide, tall);

	m_eState = PLAY_NONE;

	m_bESCDown = false;
	m_flESCTimer = 0.f;

	vgui::ivgui()->AddTickSignal(GetVPanel(), 0);

	m_bSubtitlesAvailable = false;
	m_iCurrentSubtitleToken = -1;
}

HDTFCutscenePlayer::~HDTFCutscenePlayer()
{
	if (m_SubtitleTimingData.isEnabled)
	{
		DumpRecordedSubtitleTimings();
	}

	if (g_pVideo && m_VideoMaterial)
	{
		g_pVideo->DestroyVideoMaterial(m_VideoMaterial);
		m_VideoMaterial = NULL;
	}

	while (m_pSubtitles.Count() > 0)
	{
		FCutsceneSubtitle *i = m_pSubtitles[0];
		delete i;
		m_pSubtitles.Remove(0);
	}
}

void HDTFCutscenePlayer::ApplySchemeSettings(vgui::IScheme *scheme)
{
	BaseClass::ApplySchemeSettings(scheme);

	m_hFont = scheme->GetFont("CutsceneHelp");

	g_pVGuiLocalize->ConstructString(m_pHelpSkip, 128, g_pVGuiLocalize->Find("#HDTF_Cutscene_Skip"), 0);
	g_pVGuiLocalize->ConstructString(m_pHelpPause, 128, g_pVGuiLocalize->Find("#HDTF_Cutscene_Pause"), 0);
}

bool HDTFCutscenePlayer::StartPlayback(const char *videoName)
{
	if (!g_pVideo)
		return false;

	if (!filesystem->FileExists(videoName))
	{
		Warning("Cannot play cutscene '%s' - file not found!\n", videoName);
		BeginDispose();
		return false;
	}

	if (m_VideoMaterial)
	{
		g_pVideo->DestroyVideoMaterial(m_VideoMaterial);
		m_VideoMaterial = NULL;
	}

	m_VideoMaterial = g_pVideo->CreateVideoMaterial("CutsceneRT", videoName, "GAME",
		VideoPlaybackFlags::DEFAULT_MATERIAL_OPTIONS,
		VideoSystem::DETERMINE_FROM_FILE_EXTENSION, true);

	if (!m_VideoMaterial)
		return false;

	enginesound->NotifyBeginMoviePlayback();
	m_VideoMaterial->SetVolume(snd_cutscene_volume.GetFloat());

	int nWidth, nHeight;
	m_VideoMaterial->GetVideoImageSize(&nWidth, &nHeight);
	m_VideoMaterial->GetVideoTexCoordRange(&m_flVideoU, &m_flVideoV);
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

	CHudCloseCaption *hudCloseCaption = GET_HUDELEMENT(CHudCloseCaption);
	if (hudCloseCaption)
	{
		hudCloseCaption->SetParent(this);
		hudCloseCaption->Reset();
		hudCloseCaption->SetIsWithinCutscene(true);
	}

	PrepareSubtitles(videoName);

	m_eState = PLAY_PLAYING;
	return true;
}

void HDTFCutscenePlayer::PlayPausePlayback()
{
	if (m_eState == PLAY_PLAYING)
		m_eState = PLAY_PAUSED;
	else if (m_eState == PLAY_PAUSED)
		m_eState = PLAY_PLAYING;

	m_VideoMaterial->SetPaused(m_eState == PLAY_PAUSED);

	CHudCloseCaption *hudCloseCaption = GET_HUDELEMENT(CHudCloseCaption);
	if (hudCloseCaption)
	{
		hudCloseCaption->SetCutscenePaused(m_eState == PLAY_PAUSED);
	}
}

void HDTFCutscenePlayer::CancelPlayback()
{
	m_VideoMaterial->StopVideo();
	m_eState = PLAY_NONE;
	BeginDispose();
}

void HDTFCutscenePlayer::OnPlaybackFinish()
{
	BeginDispose();
}

void HDTFCutscenePlayer::TakeControl()
{
	MakePopup();

	MoveToFront();
	RequestFocus();
	SetVisible(true);
	SetEnabled(true);

	vgui::surface()->SetMinimized(GetVPanel(), false);

	vgui::input()->SetAppModalSurface(GetVPanel());
	vgui::surface()->RestrictPaintToSinglePanel(GetVPanel());

	engine->ExecuteClientCmd("gameui_preventescapetoshow");
	engine->ClientCmd_Unrestricted("setpause");
}

void HDTFCutscenePlayer::Paint()
{
	BaseClass::Paint();

	if (m_bESCDown)
	{
		m_flHelpAlpha = 2.5f;

		if (IsSkipAble())
		{
			m_flESCTimer += gpGlobals->absoluteframetime;
			if (m_flESCTimer > TIME_TO_CLOSE)
			{
				m_bESCDown = false;
				m_flESCTimer = 0.f;
				BeginDispose();
				return;
			}
		}
	}
	else
	{
		m_flESCTimer = max(m_flESCTimer - gpGlobals->absoluteframetime * 3.f, 0.f);
		m_flHelpAlpha = max(m_flHelpAlpha - gpGlobals->absoluteframetime * 2.f, 0.f);
	}

	const int w = GetWide();
	const int h = GetTall();

	vgui::surface()->DrawSetColor(0, 0, 0, 255);
	vgui::surface()->DrawFilledRect(0, 0, w, h);

	if (m_pMaterial)
	{
		if (m_eState == PLAY_PLAYING)
		{
			if (m_VideoMaterial->Update() == false)
			{
				OnPlaybackFinish();
			}
		}

		int xpos = ((float)(w - m_nPlaybackWidth) / 2);
		int ypos = ((float)(h - m_nPlaybackHeight) / 2);

		// Draw the polys to draw this out
		CMatRenderContextPtr pRenderContext(materials);

		//pRenderContext->MatrixMode(MATERIAL_VIEW);
		//pRenderContext->PushMatrix();
		//pRenderContext->LoadIdentity();

		//pRenderContext->MatrixMode(MATERIAL_PROJECTION);
		//pRenderContext->PushMatrix();
		//pRenderContext->LoadIdentity();

		pRenderContext->Bind(m_pMaterial, NULL);

		CMeshBuilder meshBuilder;
		IMesh* pMesh = pRenderContext->GetDynamicMesh(true);
		meshBuilder.Begin(pMesh, MATERIAL_QUADS, 1);

		float flLeftX = xpos;
		float flRightX = xpos + (m_nPlaybackWidth - 1);

		float flTopY = ypos;
		float flBottomY = ypos + (m_nPlaybackHeight - 1);

		// Map our UVs to cut out just the portion of the video we're interested in
		float flLeftU = 0.0f;
		float flTopV = 0.0f;

		// We need to subtract off a pixel to make sure we don't bleed
		float flRightU = m_flVideoU - (1.0f / (float)m_nPlaybackWidth);
		float flBottomV = m_flVideoV - (1.0f / (float)m_nPlaybackHeight);

		// Get the current viewport size
		//int vx, vy, vw, vh;
		//pRenderContext->GetViewport(vx, vy, vw, vh);

		// map from screen pixel coords to -1..1
		/*flRightX = FLerp(-1, 1, 0, vw, flRightX);
		flLeftX = FLerp(-1, 1, 0, vw, flLeftX);
		flTopY = FLerp(1, -1, 0, vh, flTopY);
		flBottomY = FLerp(1, -1, 0, vh, flBottomY);*/

		for (int corner = 0; corner<4; corner++)
		{
			bool bLeft = (corner == 0) || (corner == 3);
			meshBuilder.Position3f((bLeft) ? flLeftX : flRightX, (corner & 2) ? flBottomY : flTopY, 0.0f);
			meshBuilder.Normal3f(0.0f, 0.0f, 1.0f);
			meshBuilder.TexCoord2f(0, (bLeft) ? flLeftU : flRightU, (corner & 2) ? flBottomV : flTopV);
			meshBuilder.TangentS3f(0.0f, 1.0f, 0.0f);
			meshBuilder.TangentT3f(1.0f, 0.0f, 0.0f);
			meshBuilder.Color4f(1.0f, 1.0f, 1.0f, 1.0f);
			meshBuilder.AdvanceVertex();
		}

		meshBuilder.End();
		pMesh->Draw();

		//pRenderContext->MatrixMode(MATERIAL_VIEW);
		//pRenderContext->PopMatrix();

		//pRenderContext->MatrixMode(MATERIAL_PROJECTION);
		//pRenderContext->PopMatrix();
	}

	// FADE AWAY
	vgui::surface()->DrawSetColor(0, 0, 0, min(1.f, m_flESCTimer / TIME_TO_FADE) * 255.f);
	vgui::surface()->DrawFilledRect(0, 0, w, h);

	// HELP BOX
	vgui::surface()->DrawSetTextColor(255, 255, 255, 240.f * min(1.f, m_flHelpAlpha));
	vgui::surface()->DrawSetTextFont(m_hFont);

	int t1w, t1h, t2w, t2h;
	vgui::surface()->GetTextSize(m_hFont, m_pHelpSkip, t1w, t1h);
	vgui::surface()->GetTextSize(m_hFont, m_pHelpPause, t2w, t2h);

	int hbw = max(t1w, t2w) + 2;
	int hbh = t1h + t2h;

	int hbx = 10;
	int hby = h - 10 - hbh;

	vgui::surface()->DrawSetColor(0, 0, 0, 255.f * min(1.f, m_flHelpAlpha));

	if (IsSkipAble())
		vgui::surface()->DrawFilledRect(hbx, hby, hbx + hbw, hby + hbh);
	else
		vgui::surface()->DrawFilledRect(hbx, hby + t1h - 3, hbx + hbw, hby + hbh);
	
	if (IsSkipAble())
	{
		vgui::surface()->DrawSetTextPos(hbx, hby);
		vgui::surface()->DrawPrintText(m_pHelpSkip, wcslen(m_pHelpSkip));
	}

	vgui::surface()->DrawSetTextPos(hbx, hby + t1h - 3);
	vgui::surface()->DrawPrintText(m_pHelpPause, wcslen(m_pHelpPause));

	if (m_SubtitleTimingData.isEnabled)
	{
		vgui::surface()->DrawSetColor(0, 0, 0, 255);
		vgui::surface()->DrawFilledRect(10, 10, 300, t1h + 14);

		vgui::surface()->DrawSetTextPos(12, 12);
		if (m_SubtitleTimingData.isRecording)
		{
			wchar_t *str = L"RECORDING TOKEN";

			vgui::surface()->DrawSetTextColor(255, 0, 0, 255);
			vgui::surface()->DrawPrintText(str, wcslen(str));
		}
		else
		{
			wchar_t *str = L"IDLE";

			vgui::surface()->DrawSetTextColor(255, 255, 255, 255);
			vgui::surface()->DrawPrintText(str, wcslen(str));
		}
	}
}

void HDTFCutscenePlayer::OnTick()
{
	BaseClass::OnThink();

	if (m_eState == PLAY_PLAYING)
	{
		ProcessSubtitles();
	}
}

void HDTFCutscenePlayer::BeginDispose()
{
	enginesound->NotifyEndMoviePlayback();

	if (vgui::input()->GetAppModalSurface() == GetVPanel())
	{
		vgui::input()->ReleaseAppModalSurface();
	}

	vgui::surface()->RestrictPaintToSinglePanel(NULL);

	vgui::ivgui()->RemoveTickSignal(GetVPanel());

	CHudCloseCaption *hudCloseCaption = GET_HUDELEMENT(CHudCloseCaption);
	if (hudCloseCaption)
	{
		vgui::Panel *pParent = g_pClientMode->GetViewport();
		hudCloseCaption->SetParent(pParent);
		hudCloseCaption->SetIsWithinCutscene(false);
		hudCloseCaption->Reset();
	}

	SetVisible(false);
	MarkForDeletion();

	engine->ClientCmd_Unrestricted("unpause");
	engine->ExecuteClientCmd("gameui_allowescapetoshow");
}

void HDTFCutscenePlayer::OnKeyCodeTyped(vgui::KeyCode code)
{
	if (code == KEY_ESCAPE)
	{
		m_bESCDown = true;
	}
	else if (code == KEY_SPACE)
	{
		PlayPausePlayback();
	}
	else
	{
		BaseClass::OnKeyCodeTyped(code);
	}
}

void HDTFCutscenePlayer::OnKeyCodePressed(vgui::KeyCode code)
{
	if (code == KEY_ESCAPE ||
		code == KEY_ENTER ||
		code == KEY_XBUTTON_A ||
		code == KEY_XBUTTON_B ||
		code == KEY_XBUTTON_X ||
		code == KEY_XBUTTON_Y ||
		code == KEY_XBUTTON_START ||
		code == KEY_XBUTTON_BACK)
	{
		m_bESCDown = true;
	}
	else
	{
		ConVarRef developer("developer");
		if (developer.GetBool())
		{
			if (code == KEY_Q)
			{
				if (m_SubtitleTimingData.isRecording)
				{
					FCutsceneSubtitle *subtitle = new FCutsceneSubtitle();
					Q_snprintf(subtitle->tokenName, sizeof(subtitle->tokenName), "token_%i", m_SubtitleTimingData.currentTokenIndex);
					subtitle->timestamp = m_SubtitleTimingData.startFrame;
					subtitle->duration = m_VideoMaterial->GetCurrentFrame() - m_SubtitleTimingData.startFrame;
					m_SubtitleTimingData.records.AddToTail(subtitle);

					m_SubtitleTimingData.isRecording = false;
					m_SubtitleTimingData.currentTokenIndex++;
				}
				else
				{
					m_SubtitleTimingData.isEnabled = true;
					m_SubtitleTimingData.isRecording = true;
					m_SubtitleTimingData.startFrame = m_VideoMaterial->GetCurrentFrame();
				}
			}
		}

		BaseClass::OnKeyCodePressed(code);
	}
}

void HDTFCutscenePlayer::OnKeyCodeReleased(vgui::KeyCode code)
{
	if (code == KEY_ESCAPE ||
		code == KEY_ENTER ||
		code == KEY_XBUTTON_A ||
		code == KEY_XBUTTON_B ||
		code == KEY_XBUTTON_X ||
		code == KEY_XBUTTON_Y ||
		code == KEY_XBUTTON_START ||
		code == KEY_XBUTTON_BACK)
	{
		m_bESCDown = false;
	}
	else
	{
		BaseClass::OnKeyCodeReleased(code);
	}
}

void HDTFCutscenePlayer::PrepareSubtitles(const char *videoName)
{
	ConVarRef closecaption("closecaption");
	if (!closecaption.GetBool())
		return;

	char videoBaseName[128];
	Q_FileBase(videoName, videoBaseName, sizeof(videoBaseName));

	char subtitleFileName[256];
	Q_snprintf(subtitleFileName, sizeof(subtitleFileName), "media/%s_subtitles.txt", videoBaseName);

	ConVarRef developer("developer");
	if (developer.GetBool())
	{
		Q_snprintf(
			m_SubtitleTimingData.fileName, 
			sizeof(m_SubtitleTimingData.fileName), 
			"%s", 
			subtitleFileName
		);
	}

	if (!filesystem->FileExists(subtitleFileName))
		return;

	if (developer.GetBool())
	{
		Q_snprintf(
			m_SubtitleTimingData.fileName,
			sizeof(m_SubtitleTimingData.fileName),
			"media/%s_subtitles_new.txt",
			videoBaseName
		);
	}

	KeyValues *pkvFile = new KeyValues("CutsceneSubtitles");
	if (!pkvFile->LoadFromFile(filesystem, subtitleFileName, "MOD"))
	{
		Warning("Could not load cutscene subtitles file: %s\n", subtitleFileName);
		return;
	}

	KeyValues *pkvNode = pkvFile->GetFirstSubKey();
	while (pkvNode)
	{
		const char *tokenName = pkvNode->GetName();

		KeyValues *pTimestamp = pkvNode->FindKey("timestamp");
		KeyValues *pDuration = pkvNode->FindKey("duration");

		if (!pTimestamp || !pDuration)
		{
			Warning("Error parsing node %s: missing timestamp and or duration keys.\n", tokenName);

			pkvNode = pkvNode->GetNextKey();
			continue;
		}

		FCutsceneSubtitle *subtitle = new FCutsceneSubtitle();
		Q_snprintf(subtitle->tokenName, sizeof(subtitle->tokenName), "%s", tokenName);
		subtitle->timestamp = pTimestamp->GetInt();
		subtitle->duration = pDuration->GetInt();
		
		m_pSubtitles.AddToTail(subtitle);

		pkvNode = pkvNode->GetNextKey();
	}

	pkvFile->deleteThis();

	m_bSubtitlesAvailable = true;
	m_iCurrentSubtitleToken = 0;
}

void HDTFCutscenePlayer::ProcessSubtitles()
{
	ConVarRef closecaption("closecaption");
	if (!closecaption.GetBool())
		return;

	if (!m_VideoMaterial || !m_bSubtitlesAvailable || m_iCurrentSubtitleToken == -1)
		return;

	if (m_iCurrentSubtitleToken >= m_pSubtitles.Count())
	{
		Warning("ALERT: current subtitle token index is out of range! Aborting subtitle playback.\n");
		m_bSubtitlesAvailable = false;
		m_iCurrentSubtitleToken = -1;

		return;
	}

	CHudCloseCaption *hudCloseCaption = GET_HUDELEMENT(CHudCloseCaption);
	if (!hudCloseCaption)
		return;

	const int ntime = m_VideoMaterial->GetCurrentFrame();
	const int frameRate = m_VideoMaterial->GetVideoFrameRate().GetIntFPS();
	FCutsceneSubtitle *subtitle = m_pSubtitles[m_iCurrentSubtitleToken];

	if (subtitle->timestamp <= ntime)
	{
		ConVarRef cc_linger_time("cc_linger_time");
		float duration = (subtitle->duration / frameRate) - cc_linger_time.GetFloat();
		hudCloseCaption->ProcessCaption(subtitle->tokenName, duration);

		m_iCurrentSubtitleToken++;

		if (m_iCurrentSubtitleToken >= m_pSubtitles.Count())
		{
			m_iCurrentSubtitleToken = -1;
		}
	}
}

void HDTFCutscenePlayer::DumpRecordedSubtitleTimings()
{
	KeyValues *pkvFile = new KeyValues("CutsceneSubtitles");

	if (hdtf_cutscenetool_linger_to_third.GetBool())
	{
		const int leewayFrames = (int)(m_VideoMaterial->GetVideoFrameRate().GetFPS() * hdtf_cutscenetool_leeway_time.GetFloat());
		for (int i = 0; i < m_SubtitleTimingData.records.Count(); i++)
		{
			int targetTime = m_VideoMaterial->GetFrameCount();
			if (i + 2 < m_SubtitleTimingData.records.Count())
			{
				targetTime = m_SubtitleTimingData.records[i + 2]->timestamp;
			}

			FCutsceneSubtitle *subtitle = m_SubtitleTimingData.records[i];
			subtitle->duration = (targetTime - leewayFrames) - subtitle->timestamp;
		}
	}

	for (int i = 0; i < m_SubtitleTimingData.records.Count(); i++)
	{
		FCutsceneSubtitle *subtitle = m_SubtitleTimingData.records[i];

		KeyValues *pkvNode = new KeyValues(subtitle->tokenName);
		pkvNode->SetInt("timestamp", subtitle->timestamp + hdtf_cutscenetool_start_offset.GetInt());
		pkvNode->SetInt("duration", subtitle->duration);

		pkvFile->AddSubKey(pkvNode);
	}

	pkvFile->SaveToFile(filesystem, m_SubtitleTimingData.fileName, "MOD");

	pkvFile->deleteThis();
}
