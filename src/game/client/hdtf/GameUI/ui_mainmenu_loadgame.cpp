#include "cbase.h"
#include "ui_mainmenu_loadgame.h"
#include "ui_mainmenu_newgame.h"
#include "ui_loading.h"
#include "ui_menu_popup.h"
#include "filesystem.h"
#include "savegame_version.h"

#include "vgui/ISurface.h"
#include "tier1/utlbuffer.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define MAX_SAVES 128
#define NEW_SAVE_GAME_TIMESTAMP	0xFFFFFFFF

using namespace vgui;

static HDTFUI_SaveGameSlot *g_HoveredSaveSlot = NULL;

const char *GetChapterForMapName(const char *mapName)
{
	int act;
	int chapter;
	int part;
	int argc = sscanf(mapName, "a%ic%ip%i", &act, &chapter, &part);

	if (argc == 3)
	{
		// HARDCODE(wheatley): because tutorial is not actually a chapter it is easier to have this here.
		if (act == 0 && chapter == 0 && Q_strcmp(mapName, "a0c0p0_bootcamp") == 0)
			return "#HDTF_Chapter_Tutorial";

		for (int i = 0; i < ARRAYSIZE(g_ChapterList); i++)
		{
			if (g_ChapterList[i].m_iAct == act && g_ChapterList[i].m_iChapterID == chapter)
			{
				return g_ChapterList[i].m_pChapterName;
			}
		}
	}

	return mapName;
}

HDTFUI_SaveGameSlot::HDTFUI_SaveGameSlot(vgui::Panel *parent, int index) : BaseClass(parent, "saveEntry")
{
	SetParent(parent);

	m_iIndex = index;

	m_pImg = new CTGAImagePanel(this, "saveEntry_Bitmap");
	m_pAutoSaveImg = new ImagePanel(this, "saveEntry_AutosaveImg");
	m_pChapter = new Label(this, "saveEntry_Chapter", "");
	m_pDate = new Label(this, "saveEntry_Date", "");
	m_pType = new Label(this, "saveEntry_Type", "");

	m_pImg->SetMouseInputEnabled(false);
	m_pAutoSaveImg->SetMouseInputEnabled(false);
	m_pChapter->SetMouseInputEnabled(false);
	m_pDate->SetMouseInputEnabled(false);
	m_pType->SetMouseInputEnabled(false);

	SetCommand("LoadGame");

	SetHoverState(false);
}

void HDTFUI_SaveGameSlot::SetupAsNewSave()
{
	SetCommand("SaveGame");
	m_pImg->SetVisible(false);
	m_pAutoSaveImg->SetImage("resource\\autosave");
	m_pAutoSaveImg->SetVisible(true);
	m_pChapter->SetText("#HDTF_Menu_NewSave");
	m_pDate->SetVisible(false);
	m_pType->SetVisible(false);
}

void HDTFUI_SaveGameSlot::SetSaveGame(SaveGameDescription_t save)
{
	m_SaveData = save;

	m_pChapter->SetText(GetChapterForMapName(save.szMapName));
	m_pDate->SetText(save.szFileTime);
	m_pType->SetText(save.szType);

	m_pDate->SetVisible(V_strlen(save.szFileTime) > 0);
	m_pType->SetVisible(V_strlen(save.szType) > 0);

	char imagePath[MAX_PATH];
	Q_strcpy(imagePath, "save/");
	Q_strcat(imagePath, save.szShortName, MAX_PATH);

	char *extension = strstr(imagePath, ".sav");
	if (extension)
	{
		Q_strcpy(extension, ".tga");
	}

	if (g_pFullFileSystem->FileExists(imagePath))
	{
		m_pAutoSaveImg->SetVisible(false);
		m_pImg->SetTGA(imagePath);
		m_pImg->SetVisible(true);
	}
	else
	{
		m_pAutoSaveImg->SetVisible(true);
		m_pAutoSaveImg->SetImage("resource/autosave");
		m_pImg->SetVisible(false);
	}
}

void HDTFUI_SaveGameSlot::PerformLayout()
{
	IScheme *pScheme = scheme()->GetIScheme(GetScheme());
	m_pChapter->SetFont(pScheme->GetFont("MainMenuChapterTitle"));

	m_iLineWidth = 6;

	int ypos = -3;

	m_pChapter->SetPos(m_iLineWidth * 2, ypos);
	m_pChapter->SizeToContents();
	ypos += m_pChapter->GetTall() - 5;

	if (m_pDate->IsVisible())
	{
		m_pDate->SetPos(m_iLineWidth * 2, ypos);
		m_pDate->SizeToContents();
		ypos += m_pDate->GetTall() - 2;
	}

	if (m_pType->IsVisible())
	{
		m_pType->SetPos(m_iLineWidth * 2, ypos);
		m_pType->SizeToContents();
		ypos += m_pType->GetTall() - 2;
	}

	int w = 152;
	int h = 86;

	m_pImg->SetPos(m_iLineWidth * 2, ypos);
	m_pImg->SetSize(w, h);

	m_pAutoSaveImg->SetPos(m_iLineWidth * 2, ypos);
	m_pAutoSaveImg->SetSize(w, h);

	ypos += h;

	SetWide(max(m_pChapter->GetWide(), max(m_pDate->GetWide(), max(m_pType->GetWide(), w))) + m_iLineWidth * 2);
	SetTall(ypos);
}

void HDTFUI_SaveGameSlot::OnMousePressed(MouseCode code)
{
	if (IsHovered() && code == MOUSE_LEFT)
	{
		KeyValues *actionMessage = new KeyValues("Command", "command", m_pCommand);
		PostActionSignal(actionMessage->MakeCopy());
		actionMessage->deleteThis();
	}
	else if (!IsHovered() && code == MOUSE_LEFT)
	{
		if (g_HoveredSaveSlot != NULL)
		{
			g_HoveredSaveSlot->SetHoverState(false);
		}

		SetHoverState(true);
		g_HoveredSaveSlot = this;

		KeyValues *actionMessage = new KeyValues("Command", "command", "SaveHovered");
		PostActionSignal(actionMessage->MakeCopy());
		actionMessage->deleteThis();
	}
	else
	{
		BaseClass::OnMousePressed(code);
	}
}

void HDTFUI_SaveGameSlot::OnMouseDoublePressed(vgui::MouseCode code)
{
	if (code == MOUSE_LEFT)
	{
		KeyValues *actionMessage = new KeyValues("Command", "command", m_pCommand);
		PostActionSignal(actionMessage->MakeCopy());
		actionMessage->deleteThis();
	}
}

void HDTFUI_SaveGameSlot::OnHoverStateChanged(bool state)
{
	if (state)
	{
		m_pChapter->SetFgColor(Color(255, 25, 0, 255));
		m_pDate->SetFgColor(Color(255, 25, 0, 255));
		m_pType->SetFgColor(Color(255, 25, 0, 255));
	}
	else
	{
		m_pChapter->SetFgColor(Color(255, 255, 255, 255));
		m_pDate->SetFgColor(Color(255, 255, 255, 255));
		m_pType->SetFgColor(Color(255, 255, 255, 255));
	}
}

void HDTFUI_SaveGameSlot::Paint()
{
	if (IsHovered())
		surface()->DrawSetColor(255, 25, 0, 255);
	else
		surface()->DrawSetColor(255, 255, 255, 255);

	surface()->DrawFilledRect(0, 0, m_iLineWidth, GetTall());
}

MainMenu_LoadGame::MainMenu_LoadGame(Panel *parent, const char *name) : BaseClass(parent, name)
{
	SetParent(parent);

	m_pScrollFrame = new Panel(this, "scrollHolder");
	m_pScrollHolder = new Panel(m_pScrollFrame, "scrollHolderFrame");

	m_pDeleteButton = new Button(this, "backButton", "#HDTF_LoadGame_Delete");
	m_pDeleteButton->SetCommand("DeleteSave");
	m_pDeleteButton->AddActionSignalTarget(this);
	m_pDeleteButton->SetVisible(true);
	m_pDeleteButton->SetZPos(99);
	m_pDeleteButton->SetEnabled(false);
	m_pDeleteButton->SetPaintBorderEnabled(false);

	m_iScrollSize = 1;
	m_iMaxScroll = 0;
	m_iScrollOffset = 0;
}

MainMenu_LoadGame::~MainMenu_LoadGame()
{
	m_pSaves.PurgeAndDeleteElements();
}

void MainMenu_LoadGame::OnBeforeSwitchTo()
{
	LookForSaves();
}

void MainMenu_LoadGame::OnSwitchedFrom()
{
	if (g_HoveredSaveSlot)
	{
		g_HoveredSaveSlot->SetHoverState(false);
		g_HoveredSaveSlot = NULL;
		m_pDeleteButton->SetEnabled(false);
	}

	SetReady(false);
}

void MainMenu_LoadGame::LookForSaves()
{
	char saveDir[MAX_PATH];
	V_snprintf(saveDir, sizeof(saveDir), "save/*.sav");

	m_iMaxScroll = 0;
	m_iScrollOffset = 0;

	m_pSaves.PurgeAndDeleteElements();
	m_SaveGames.RemoveAll();

	FileFindHandle_t handle;
	const char *fileName = g_pFullFileSystem->FindFirst(saveDir, &handle);
	while (fileName)
	{
		char filePath[MAX_PATH];
		Q_snprintf(filePath, sizeof(filePath), "save/%s", fileName);

		if (!g_pFullFileSystem->FileExists(filePath, "MOD"))
		{
			fileName = g_pFullFileSystem->FindNext(handle);
			continue;
		}

		SaveGameDescription_t data;
		if (ParseSaveData(filePath, fileName, data))
		{
			m_SaveGames.AddToTail(data);
		}

		fileName = g_pFullFileSystem->FindNext(handle);
	}

	g_pFullFileSystem->FindClose(handle);

	qsort(m_SaveGames.Base(), m_SaveGames.Count(), sizeof(SaveGameDescription_t), &SaveGameSortFunc);

	for (int i = 0; i < m_SaveGames.Count() && i < MAX_SAVES; i++)
	{
		AddToSaveList(i);
	}

	InvalidateLayout();
}

void MainMenu_LoadGame::AddToSaveList(int index)
{
	HDTFUI_SaveGameSlot *slot = new HDTFUI_SaveGameSlot(m_pScrollHolder, index);
	slot->SetSaveGame(m_SaveGames[index]);
	slot->AddActionSignalTarget(this);
	m_pSaves.AddToTail(slot);
}

void MainMenu_LoadGame::AddNoSavesLabel()
{
	Label *label = new Label(m_pScrollHolder, "loadGame_NoSaves", "#HDTF_LoadGame_NoSaves");
	label->SetFont(m_hNoChapterFont);
	m_pSaves.AddToTail(label);
}

bool MainMenu_LoadGame::ParseSaveData(char const *pszFileName, char const *pszShortName, SaveGameDescription_t &save)
{
	char    szMapName[SAVEGAME_MAPNAME_LEN];
	char    szComment[SAVEGAME_COMMENT_LEN];
	char    szElapsedTime[SAVEGAME_ELAPSED_LEN];

	if (!pszFileName || !pszShortName)
		return false;

	Q_strncpy(save.szShortName, pszShortName, sizeof(save.szShortName));
	Q_strncpy(save.szFileName, pszFileName, sizeof(save.szFileName));

	FileHandle_t fh = g_pFullFileSystem->Open(pszFileName, "rb", "MOD");
	if (fh == FILESYSTEM_INVALID_HANDLE)
		return false;

	int readok = SaveReadNameAndComment(fh, szMapName, szComment);
	g_pFullFileSystem->Close(fh);

	if (!readok)
	{
		return false;
	}

	Q_strncpy(save.szMapName, szMapName, sizeof(save.szMapName));

	int i;
	i = strlen(szComment);
	Q_strncpy(szElapsedTime, "??", sizeof(szElapsedTime));
	if (i >= 6)
	{
		Q_strncpy(szElapsedTime, (char *)&szComment[i - 6], 7);
		szElapsedTime[6] = '\0';

		int minutes = atoi(szElapsedTime);
		int seconds = atoi(szElapsedTime + 4);

		if (minutes)
		{
			Q_snprintf(szElapsedTime, sizeof(szElapsedTime), "%d %s %d seconds", minutes, minutes > 1 ? "minutes" : "minute", seconds);
		}
		else
		{
			Q_snprintf(szElapsedTime, sizeof(szElapsedTime), "%d seconds", seconds);
		}

		int n;

		n = i - 6;
		szComment[n] = '\0';

		n--;

		while ((n >= 1) &&
			szComment[n] &&
			szComment[n] == ' ')
		{
			szComment[n--] = '\0';
		}
	}

	const char *pszType = "";
	if (strstr(pszFileName, "quick"))
	{
		pszType = "#GameUI_QuickSave";
	}
	else if (strstr(pszFileName, "autosave"))
	{
		pszType = "#GameUI_AutoSave";
	}

	Q_strncpy(save.szType, pszType, sizeof(save.szType));
	Q_strncpy(save.szComment, szComment, sizeof(save.szComment));
	Q_strncpy(save.szElapsedTime, szElapsedTime, sizeof(save.szElapsedTime));

	long fileTime = g_pFullFileSystem->GetFileTime(pszFileName);
	char szFileTime[32];
	g_pFullFileSystem->FileTimeToString(szFileTime, sizeof(szFileTime), fileTime);
	char *newline = strstr(szFileTime, "\n");
	if (newline)
	{
		*newline = 0;
	}
	Q_strncpy(save.szFileTime, szFileTime, sizeof(save.szFileTime));
	save.iTimestamp = fileTime;
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: timestamp sort function for savegames
//-----------------------------------------------------------------------------
int MainMenu_LoadGame::SaveGameSortFunc(const void *lhs, const void *rhs)
{
	const SaveGameDescription_t *s1 = (const SaveGameDescription_t *)lhs;
	const SaveGameDescription_t *s2 = (const SaveGameDescription_t *)rhs;

	if (s1->iTimestamp < s2->iTimestamp)
		return 1;
	else if (s1->iTimestamp > s2->iTimestamp)
		return -1;

	// timestamps are equal, so just sort by filename
	return strcmp(s1->szFileName, s2->szFileName);
}

void MainMenu_LoadGame::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	m_pDeleteButton->SetFont(pScheme->GetFont("MainMenuButtons"));
	m_hNoChapterFont = pScheme->GetFont("MainMenuPopup");
}

void MainMenu_LoadGame::PerformLayout()
{
	int wide, tall;
	surface()->GetScreenSize(wide, tall);
	SetSize(wide, tall);

	int spacing = 10;
	int xpos = scheme()->GetProportionalScaledValue(45);

	m_pScrollFrame->SetPos(xpos, (tall / 8) + spacing);
	m_pScrollFrame->SetSize(wide - xpos, tall - ((tall / 8) + spacing) * 2);

	m_pScrollHolder->SetWide(m_pScrollFrame->GetWide());

	m_pDeleteButton->SetPos(scheme()->GetProportionalScaledValue(180), tall - (tall / 9));
	m_pDeleteButton->SetWide(scheme()->GetProportionalScaledValue(100));
	m_pDeleteButton->SetFgColor(Color(255, 255, 255, 255));
	m_pDeleteButton->SetSelectedColor(Color(100, 5, 0, 255), Color(0, 0, 0, 0));
	m_pDeleteButton->SetArmedColor(Color(255, 25, 0, 255), Color(0, 0, 0, 0));
	m_pDeleteButton->SetPaintBorderEnabled(false);

	if (!m_SaveGames.Count() && !m_pSaves.Count())
	{
		AddNoSavesLabel();
	}

	int ypos = 0;
	for (int i = 0; i < m_pSaves.Count(); i++)
	{
		Panel *pEntry = m_pSaves.Element(i);
		Label *isLabel = dynamic_cast<Label *>(pEntry);

		if (isLabel)
		{
			isLabel->SizeToContents();
		}
		else
		{
			pEntry->PerformLayout();
		}
		
		pEntry->SetPos(0, ypos);

		m_iScrollSize = pEntry->GetTall() / 3;

		ypos += pEntry->GetTall() + spacing;
	}

	m_pScrollHolder->SetTall(ypos);

	m_iMaxScroll = m_pScrollHolder->GetTall() - m_pScrollFrame->GetTall();

	m_iScrollOffset = max(0, min(m_iMaxScroll, m_iScrollOffset));
	m_pScrollHolder->SetPos(0, -m_iScrollOffset);

	SetReady(true);
}

void MainMenu_LoadGame::OnMouseWheeled(int delta)
{
	m_iScrollOffset = max(0, min(m_iMaxScroll, m_iScrollOffset - m_iScrollSize * delta));
	m_pScrollHolder->SetPos(0, -m_iScrollOffset);
}

void MainMenu_LoadGame::OnCommand(const char *command)
{
	if (FStrEq(command, "SaveHovered"))
	{
		if(g_HoveredSaveSlot && g_HoveredSaveSlot->GetSaveIndex() != -1)
			m_pDeleteButton->SetEnabled(true);
		else
			m_pDeleteButton->SetEnabled(false);
	}
	else if (FStrEq(command, "LoadGame"))
	{
		if (g_HoveredSaveSlot != NULL)
		{
			SaveGameDescription_t *data = &m_SaveGames[g_HoveredSaveSlot->GetSaveIndex()];
			const char *saveName = data->szShortName;
			if (saveName && saveName[0])
			{
				// we should stop the background video before load
				// or it might glitch when loading started
				HDTFMainMenu *pMenu = dynamic_cast<HDTFMainMenu *>(guiroot->GetMainCanvas());
				if (pMenu)
					pMenu->StopBackgroundVideo();

				guiloading->PrepareForLoading(data->szMapName);

				char command[128];
				V_snprintf(command, sizeof(command), "progress_enable\nload %s\n", saveName);
				engine->ClientCmd_Unrestricted(command);
			}
		}
	}
	else if (FStrEq(command, "SaveGame"))
	{
		if (g_HoveredSaveSlot != NULL)
		{
			if (g_HoveredSaveSlot->GetSaveIndex() == -1)
			{
				OnCommand("SaveGameOverwrite");
			}
			else
			{
				HDTFUI_Popup *popup = new HDTFUI_Popup(
					this,
					"overwrite_save_queue",
					"#HDTF_Save_Overwrite",
					POPUP_PROMPT,
					"#HDTF_Menu_Yes",
					"#HDTF_Menu_No",
					"SaveGameOverwrite");

				popup->AddActionSignalTarget(this);

				popup->TakeControl();
			}
		}
	}
	else if (FStrEq(command, "SaveGameOverwrite"))
	{
		if (g_HoveredSaveSlot != NULL)
		{
			if (g_HoveredSaveSlot->GetSaveIndex() != -1)
			{
				SaveGameDescription_t *data = &m_SaveGames[g_HoveredSaveSlot->GetSaveIndex()];
				DeleteSaveGame(data->szFileName);
			}

			char fileName[128];
			FindSaveSlot(fileName, sizeof(fileName));

			if(fileName && fileName[0])
			{
				char command[256];
				Q_snprintf(command, sizeof(command), "save %s\n", fileName);

				engine->ClientCmd_Unrestricted(command);

				HDTFPauseMenu *pauseMenu = (HDTFPauseMenu *)guiroot->GetPauseMenu();

				guiroot->SwitchCanvasInstant(pauseMenu->GetMainCanvas());

				engine->ExecuteClientCmd("gameui_hide");
			}
		}
	}
	else if (FStrEq(command, "DeleteSave"))
	{
		HDTFUI_Popup *popup = new HDTFUI_Popup(
			this,
			"overwrite_save_queue",
			"#HDTF_Save_Delete",
			POPUP_PROMPT,
			"#HDTF_Menu_Yes",
			"#HDTF_Menu_No",
			"DeleteSaveConfirm");

		popup->AddActionSignalTarget(this);

		popup->TakeControl();
	}
	else if (FStrEq(command, "DeleteSaveConfirm"))
	{
		if (g_HoveredSaveSlot != NULL)
		{
			SaveGameDescription_t *data = &m_SaveGames[g_HoveredSaveSlot->GetSaveIndex()];
			DeleteSaveGame(data->szFileName);
			m_pDeleteButton->SetEnabled(false);
			InvalidateLayout();
		}
	}
	else
	{
		BaseClass::OnCommand(command);
	}
}

void MainMenu_LoadGame::DeleteSaveGame(const char *fileName)
{
	if (!fileName || !fileName[0])
		return;

	// delete the save game file
	g_pFullFileSystem->RemoveFile(fileName, "MOD");

	// delete the associated tga
	char tga[_MAX_PATH];
	Q_strncpy(tga, fileName, sizeof(tga));
	char *ext = strstr(tga, ".sav");
	if (ext)
	{
		strcpy(ext, ".tga");
	}
	g_pFullFileSystem->RemoveFile(tga, "MOD");

	for (int i = 0; i < m_SaveGames.Count(); i++)
	{
		if (FStrEq(m_SaveGames[i].szFileName, fileName))
		{
			m_SaveGames.Remove(i);

			for (int j = 0; j < m_pSaves.Count(); j++)
			{
				HDTFUI_SaveGameSlot *slot = dynamic_cast<HDTFUI_SaveGameSlot *>(m_pSaves[j]);
				if (slot)
				{
					if (i == slot->GetSaveIndex())
					{
						if (g_HoveredSaveSlot == slot)
							g_HoveredSaveSlot = NULL;

						slot->MarkForDeletion();
						m_pSaves.Remove(j);
						break;
					}
				}
			}
		}
	}

	if (m_SaveGames.Count() == 0)
	{
		AddNoSavesLabel();
	}
	else
	{
		// renew the indices
		for (int i = 0; i < m_SaveGames.Count(); i++)
		{
			HDTFUI_SaveGameSlot *slot = dynamic_cast<HDTFUI_SaveGameSlot *>(m_pSaves[i]);
			if (slot)
			{
				slot->SetSaveIndex(i);
			}
		}
	}
}

void MainMenu_LoadGame::FindSaveSlot(char *buffer, int bufsize)
{
	buffer[0] = 0;
	char szFileName[512];
	for (int i = 0; i < 1000; i++)
	{
		Q_snprintf(szFileName, sizeof(szFileName), "save/hdtf-%03i.sav", i);

		FileHandle_t fp = g_pFullFileSystem->Open(szFileName, "rb");
		if (!fp)
		{
			// clean up name
			Q_strncpy(buffer, szFileName + 5, bufsize);
			char *ext = strstr(buffer, ".sav");
			if (ext)
			{
				*ext = 0;
			}
			return;
		}
		g_pFullFileSystem->Close(fp);
	}

	Assert(!("Could not generate new save game file"));
}

#define MAKEID(d,c,b,a)	( ((int)(a) << 24) | ((int)(b) << 16) | ((int)(c) << 8) | ((int)(d)) )

int MainMenu_LoadGame::SaveReadNameAndComment(FileHandle_t f, char *name, char *comment)
{
	int i, tag, size, tokenSize, tokenCount;
	char *pSaveData, *pFieldName, **pTokenList;

	g_pFullFileSystem->Read(&tag, sizeof(int), f);
	if (tag != MAKEID('J', 'S', 'A', 'V'))
	{
		return 0;
	}

	g_pFullFileSystem->Read(&tag, sizeof(int), f);
	if (tag != SAVEGAME_VERSION)				// Enforce version for now
	{
		return 0;
	}

	name[0] = '\0';
	comment[0] = '\0';
	g_pFullFileSystem->Read(&size, sizeof(int), f);

	g_pFullFileSystem->Read(&tokenCount, sizeof(int), f);	// These two ints are the token list
	g_pFullFileSystem->Read(&tokenSize, sizeof(int), f);
	size += tokenSize;

	// Sanity Check.
	if (tokenCount < 0 || tokenCount > 1024 * 1024 * 32)
	{
		return 0;
	}

	if (tokenSize < 0 || tokenSize > 1024 * 1024 * 32)
	{
		return 0;
	}

	pSaveData = (char *)new char[size];
	g_pFullFileSystem->Read(pSaveData, size, f);

	int nNumberOfFields;

	char *pData;
	int nFieldSize;

	pData = pSaveData;

	// Allocate a table for the strings, and parse the table
	if (tokenSize > 0)
	{
		pTokenList = new char *[tokenCount];

		// Make sure the token strings pointed to by the pToken hashtable.
		for (i = 0; i<tokenCount; i++)
		{
			pTokenList[i] = *pData ? pData : NULL;	// Point to each string in the pToken table
			while (*pData++);				// Find next token (after next null)
		}
	}
	else
		pTokenList = NULL;

	// short, short (size, index of field name)
	nFieldSize = *(short *)pData;
	pData += sizeof(short);
	pFieldName = pTokenList[*(short *)pData];

	if (stricmp(pFieldName, "GameHeader"))
	{
		delete[] pSaveData;
		return 0;
	};

	// int (fieldcount)
	pData += sizeof(short);
	nNumberOfFields = *(int*)pData;
	pData += nFieldSize;

	// Each field is a short (size), short (index of name), binary string of "size" bytes (data)
	for (i = 0; i < nNumberOfFields; i++)
	{
		// Data order is:
		// Size
		// szName
		// Actual Data

		nFieldSize = *(short *)pData;
		pData += sizeof(short);

		pFieldName = pTokenList[*(short *)pData];
		pData += sizeof(short);

		if (!stricmp(pFieldName, "comment"))
		{
			Q_strncpy(comment, pData, nFieldSize);
		}
		else if (!stricmp(pFieldName, "mapName"))
		{
			Q_strncpy(name, pData, nFieldSize);
		};

		// Move to Start of next field.
		pData += nFieldSize;
	};

	// Delete the string table we allocated
	delete[] pTokenList;
	delete[] pSaveData;

	if (strlen(name) > 0 && strlen(comment) > 0)
		return 1;

	return 0;
}