#include "cbase.h"
#include "ui_mainmenu.h"
#include "engine/IEngineSound.h"

#include "vgui/ISurface.h"
#include "vgui/IInput.h"
#include "filesystem.h"

// main menu slides (canvases)
#include "ui_mainmenu_main.h"
#include "ui_mainmenu_newgame.h"
#include "ui_mainmenu_options.h"
#include "ui_mainmenu_loadgame.h"

#include "ui_menu_popup.h"
#include "ui_loading.h"

#include "achievementmgr.h"
#include <vgui/ISystem.h>

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;

HDTFMainMenu::HDTFMainMenu(Panel *parent, const char *name) : EditablePanel(parent, name)
{
	SetParent(parent);

	CreateCanvas();

	m_pVideoPath = PickVideo();
	StartBackgroundVideo(m_pVideoPath);

	m_iLoadingBGTexture = surface()->CreateNewTextureID();
	// this should remain console/background01_widescreen because this file used by the engine
	// when game boots up so we always will have correct transition between
	// engine loading and our custom main menu fade-in effect - Wheatley.
	surface()->DrawSetTextureFile(m_iLoadingBGTexture, "console/background01_widescreen", true, true);
	m_flLoadingBGAlpha = 1.f;
}

HDTFMainMenu::~HDTFMainMenu()
{
	// Noodles; Sound shuts down before this is called. And we delete this in CVideoServices::Shutdown()
	/*if ( g_pVideo && m_BGVideoMaterial )
	{
		g_pVideo->DestroyVideoMaterial(m_BGVideoMaterial);
		m_BGVideoMaterial = NULL;
	}*/
}

void HDTFMainMenu::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	int wide, tall;
	surface()->GetScreenSize(wide, tall);
	SetSize(wide, tall);
}

char *HDTFMainMenu::PickVideo()
{
	ConVarRef sv_unlockedchapters("sv_unlockedchapters");
	
	int i = max(0, min(HDTF_CHAPTER_COUNT, sv_unlockedchapters.GetInt()) - 1);

	switch (g_ChapterList[i].m_iAct)
	{
		case 1:
			return "media/background02.bik";
		case 2:
			return "media/background03.bik";
		case 3:
		case 4:
			return "media/background04.bik";
	}

	return "media/background01.bik";
}

// -------------------------------------------------------
// Purpose: background painting (video and cine overlay)
// -------------------------------------------------------
void HDTFMainMenu::PaintBackground()
{
	const int w = GetWide();
	const int h = GetTall();

	const float alphaMultiplier = surface()->DrawGetAlphaMultiplier();

	surface()->DrawSetAlphaMultiplier(1.f);

	// ---------------------------------
	// BACKGROUND VIDEO
	// ---------------------------------
	if (m_pBGMaterial && m_BGVideoMaterial)
	{
		if (m_BGVideoMaterial->Update() == false)
		{
			OnVideoFinished();
		}

		if(engine->IsActiveApp())
		{
			ConVarRef snd_musicvolume("snd_musicvolume");
			m_BGVideoMaterial->SetVolume(snd_musicvolume.GetFloat());
			m_BGVideoMaterial->SetMuted(false);
		}
		else if(!m_BGVideoMaterial->IsMuted())
		{
			m_BGVideoMaterial->SetMuted(true);
		}

		CMatRenderContextPtr pRenderContext(materials);
		pRenderContext->Bind(m_pBGMaterial);

		CMeshBuilder meshBuilder;
		IMesh* pMesh = pRenderContext->GetDynamicMesh(true);
		meshBuilder.Begin(pMesh, MATERIAL_QUADS, 1);

		float canvasLeftX = 0;
		float canvasRightX = w;

		float canvasLeftY = 0;
		float canvasRightY = h;

		float flLeftU = 0.0f;
		float flTopV = 0.0f;

		float flRightU = m_flVideoU - (1.0f / (float)w);
		float flBottomV = m_flVideoV - (1.0f / (float)h);

		float alpha = ((float)GetFgColor()[3] / 255.0f);

		for (int corner = 0; corner < 4; corner++)
		{
			bool bLeft = (corner == 0) || (corner == 3);
			meshBuilder.Position3f((bLeft) ? canvasLeftX : canvasRightX, (corner & 2) ? canvasRightY : canvasLeftY, 0.0f);
			meshBuilder.TexCoord2f(0, (bLeft) ? flLeftU : flRightU, (corner & 2) ? flBottomV : flTopV);
			meshBuilder.Color4f(1.0f, 1.0f, 1.0f, alpha);
			meshBuilder.AdvanceVertex();
		}
		meshBuilder.End();

		pMesh->Draw();
	}

	// ---------------------------------
	// CINEMATIC OVERLAY
	// ---------------------------------
	const int cineOverlayHeight = h / 8;

	surface()->DrawSetColor(0, 0, 0, 255);
	surface()->DrawFilledRect(0, 0, w, cineOverlayHeight);
	surface()->DrawFilledRect(0, h - cineOverlayHeight, w, h);

	// ---------------------------------
	// FADING AWAY LOADING SCREEN
	// ---------------------------------
	if (m_flLoadingBGAlpha > 0.f && !engine->IsDrawingLoadingImage())
	{
		surface()->DrawSetTexture(m_iLoadingBGTexture);
		surface()->DrawSetColor(255, 255, 255, 255 * min(m_flLoadingBGAlpha, 1.f));
		surface()->DrawTexturedRect(0, 0, w, h);

		// MAGIC NUMBER!
		// before game fully loaded frametime seems to stuck at .1
		// can't confirm it's the same on other machines.
		if(gpGlobals->frametime != 0.1f)
			m_flLoadingBGAlpha = Approach(0.f, m_flLoadingBGAlpha, gpGlobals->frametime * 0.45f);
	}

	surface()->DrawSetAlphaMultiplier(alphaMultiplier);
}

// -------------------------------------------------------
// Purpose: starts background video
// -------------------------------------------------------
void HDTFMainMenu::StartBackgroundVideo(const char *video)
{
	if (!g_pVideo)
		return;

	if (m_BGVideoMaterial)
	{
		g_pVideo->DestroyVideoMaterial(m_BGVideoMaterial);
		m_BGVideoMaterial = NULL;
	}

	if (!filesystem->FileExists(video))
	{
		Warning("Failed to load background video '%s'!\n", video);

		if (filesystem->FileExists("media/background01.bik"))
		{
			Warning("Found video for ACT 0 - falling back.\n");
			m_pVideoPath = "media/background01.bik";
			video = m_pVideoPath;
		}
		else
		{
			return;
		}
	}

	if (g_pVideo)
	{
		m_BGVideoMaterial = g_pVideo->CreateVideoMaterial("MainMenuBackground", video, "GAME",
			VideoPlaybackFlags::DEFAULT_MATERIAL_OPTIONS,
			VideoSystem::DETERMINE_FROM_FILE_EXTENSION, true);
	}

	if (!m_BGVideoMaterial)
		return;

	m_BGVideoMaterial->GetVideoTexCoordRange(&m_flVideoU, &m_flVideoV);
	m_BGVideoMaterial->SetLooping(true);
	m_pBGMaterial = m_BGVideoMaterial->GetMaterial();
}

// -------------------------------------------------------
// Purpose: destroys video material
// -------------------------------------------------------
void HDTFMainMenu::EndBackgroundVideo()
{
	if (g_pVideo && m_BGVideoMaterial)
	{
		m_BGVideoMaterial->StopVideo();
		g_pVideo->DestroyVideoMaterial(m_BGVideoMaterial);
		m_BGVideoMaterial = NULL;
	}
}

// -------------------------------------------------------
// Purpose: restarts the video upon completion
// -------------------------------------------------------
void HDTFMainMenu::OnVideoFinished()
{
	// UNDONE: there's a native way to do so
	// I'll leave this method here just in case.

	// we just loop our video
	//EndBackgroundVideo();
	//StartBackgroundVideo(m_pVideoPath);
}

// -------------------------------------------------------
// Purpose: restart video playback
// -------------------------------------------------------
void HDTFMainMenu::RestartBGVideo()
{
	// checks if we unlocked a new act and need to
	// update our background video
	m_pVideoPath = PickVideo();
	EndBackgroundVideo();
	StartBackgroundVideo(m_pVideoPath);
}

// -------------------------------------------------------
// Purpose: create all canvases and fill them up
// -------------------------------------------------------
void HDTFMainMenu::CreateCanvas()
{
	canvas_Main = new MainMenu_Main(this, "canvas_Main");
	canvas_NewGame = new MainMenu_NewGame(this, "canvas_NewGame");

	canvas_NewGame->SetVisible(false);

	guiroot->ForceActiveCanvas(canvas_Main);
}

// -------------------------------------------------------
// Purpose: processes the commands
// -------------------------------------------------------
void HDTFMainMenu::OnCommand(const char* command)
{
	// main tab switches
	if (FStrEq(command, "menu_newgame"))
	{
		guiroot->SwitchCanvas(canvas_NewGame);
	}
	else if (FStrEq(command, "menu_loadgame"))
	{
		guiroot->SwitchCanvas(guiroot->GetLoadGameCanvas());
	}
	else if (FStrEq(command, "menu_options"))
	{
		//guiroot->GetGameUI()->SendMainMenuCommand("OpenOptionsDialog");
		guiroot->SwitchCanvas(guiroot->GetOptionsCanvas());
	}
	else if (FStrEq(command, "menu_main"))
	{
		guiroot->SwitchCanvas(canvas_Main);
	}
	else if (FStrEq(command, "menu_quit"))
	{
		HDTFUI_Popup *popup = new HDTFUI_Popup(
			this,
			"quit_game_queue",
			"#HDTF_Quit_Message",
			POPUP_PROMPT,
			"#HDTF_Menu_Yes",
			"#HDTF_Menu_No",
			"ConfirmExit");

		popup->AddActionSignalTarget(this);

		popup->TakeControl();
	}
	else if (FStrEq(command, "ConfirmExit"))
	{
		CAchievementMgr *pAchievementMgr = dynamic_cast<CAchievementMgr *>(engine->GetAchievementMgr());
		if (pAchievementMgr)
		{
			// see achievements_hdtf.cpp
			const int HDTF_QUIT_GAME = 202;
			CBaseAchievement *pAchievement = pAchievementMgr->GetAchievementByID(HDTF_QUIT_GAME);
			if (pAchievement && !pAchievement->IsAchieved())
				pAchievementMgr->AwardAchievement(pAchievement->GetAchievementID());
		}

		guiroot->GetGameUI()->SendMainMenuCommand("QuitNoConfirm");
	}
	else if (FStrEq(command, "menu_continue"))
	{
		ActionContinueGame();
	}
	else if (FStrEq(command, "menu_open_twitter"))
	{
		OpenWebsiteOverlay("https://twitter.com/Royal_Rudius");
	}
	else if (FStrEq(command, "menu_open_discord"))
	{
		OpenWebsiteOverlay("https://discord.com/invite/QTShMyC");
	}
	else if (FStrEq(command, "menu_open_patreon"))
	{
		OpenWebsiteOverlay("https://www.patreon.com/HDTF");
	}
	else
	{
		BaseClass::OnCommand(command);
	}
}

// -------------------------------------------------------
// Purpose: passes alpha multiplier to root panel
// -------------------------------------------------------
float HDTFMainMenu::GetAlphaMultiplier()
{
	return 1.f - m_flLoadingBGAlpha;
}

struct _SaveFileDesc
{
	char fileName[MAX_PATH];
	long timestamp;
};

static int _SortSaveFiles(const _SaveFileDesc *lhs, const _SaveFileDesc *rhs)
{
	if (lhs->timestamp < rhs->timestamp)
		return 1;
	else if (lhs->timestamp > rhs->timestamp)
		return -1;

	return strcmp(lhs->fileName, rhs->fileName);
}

void HDTFMainMenu::ActionContinueGame()
{
	CUtlVector<_SaveFileDesc> saveFiles;

	FileFindHandle_t searchHandle;
	const char saveFileWildcard[] = "save/*.sav";
	const char* fileName = filesystem->FindFirst(saveFileWildcard, &searchHandle);
	while (fileName)
	{
		_SaveFileDesc desc = {};
		Q_strcpy(desc.fileName, fileName);

		char filePath[MAX_PATH];
		Q_snprintf(filePath, sizeof(filePath), "save/%s", fileName);
		desc.timestamp = filesystem->GetFileTime(filePath);

		saveFiles.AddToTail(desc);

		fileName = filesystem->FindNext(searchHandle);
	}
	filesystem->FindClose(searchHandle);

	if (saveFiles.Count() == 0)
	{
		Warning("[HDTFUI] Main menu received 'continue game' command but no save files were found!\n");
		return;
	}

	saveFiles.Sort(_SortSaveFiles);
	const _SaveFileDesc* latestSaveGame = &saveFiles.Head();
	assert(latestSaveGame != nullptr);

	char mapName[SAVEGAME_MAPNAME_LEN];
	char _unusedComment[SAVEGAME_COMMENT_LEN];

	char saveFilePath[MAX_PATH];
	Q_snprintf(saveFilePath, sizeof(saveFilePath), "save/%s", latestSaveGame->fileName);
	FileHandle_t fileHandle = filesystem->Open(saveFilePath, "rb");
	
	if (fileHandle == FILESYSTEM_INVALID_HANDLE)
	{
		Warning("[HDTFUI] Failed to open save file '%s' for reading!\n", saveFilePath);
		return;
	}

	const int readResult = MainMenu_LoadGame::SaveReadNameAndComment(fileHandle, mapName, _unusedComment);

	filesystem->Close(fileHandle);

	if (!readResult)
	{
		Warning("[HDTFUI] Failed to parse save file '%s'!\n", saveFilePath);
		return;
	}

	StopBackgroundVideo();

	guiloading->PrepareForLoading(mapName);

	char command[128];
	V_snprintf(command, sizeof(command), "progress_enable\nload %s\n", latestSaveGame->fileName);
	engine->ClientCmd_Unrestricted(command);
}

void HDTFMainMenu::OpenWebsiteOverlay(const char* msz_URL)
{
	// if the steam overlay is active just open up there
	if (steamapicontext->SteamUtils()->IsOverlayEnabled())
	{
		steamapicontext->SteamFriends()->ActivateGameOverlayToWebPage(msz_URL);
	}
	else
	{
		system()->ShellExecute("open", msz_URL);
	}
	
}
