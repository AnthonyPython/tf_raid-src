#pragma once

#include "ui_mainmenu.h"
#include "ui_menu_button.h"
#include "tga_image_panel.h"

#include "vgui_controls/Panel.h"
#include "vgui_controls/BitmapImagePanel.h"

#define SAVEGAME_MAPNAME_LEN 32
#define SAVEGAME_COMMENT_LEN 80
#define SAVEGAME_ELAPSED_LEN 32

struct SaveGameDescription_t
{
	char szShortName[64];
	char szFileName[128];
	char szMapName[SAVEGAME_MAPNAME_LEN];
	char szComment[SAVEGAME_COMMENT_LEN];
	char szType[64];
	char szElapsedTime[SAVEGAME_ELAPSED_LEN];
	char szFileTime[32];
	unsigned int iTimestamp;
	unsigned int iSize;
};

class HDTFUI_SaveGameSlot : public vgui::Panel
{
	DECLARE_CLASS_SIMPLE(HDTFUI_SaveGameSlot, vgui::Panel);
public:
	HDTFUI_SaveGameSlot(vgui::Panel *parent, int index);

	void SetupAsNewSave();
	void SetSaveGame(SaveGameDescription_t save);
	void SetSaveIndex(int index) { m_iIndex = index; }
	void PerformLayout();

	void SetCommand(const char *command)
	{
		V_strcpy(m_pCommand, command);
	}

	void SetHoverState(bool state)
	{
		if (m_bIsHovered != state)
		{
			m_bIsHovered = state;
			OnHoverStateChanged(m_bIsHovered);
		}
	}

	void OnHoverStateChanged(bool state);

	bool IsHovered()
	{
		return m_bIsHovered;
	}

	int GetSaveIndex() { return m_iIndex; }

protected:
	virtual void	Paint();
	virtual void	OnMousePressed(vgui::MouseCode code);
	virtual void	OnMouseDoublePressed(vgui::MouseCode code);

private:
	char					m_pCommand[64];
	int						m_iLineWidth;
	bool					m_bIsHovered;

	SaveGameDescription_t	m_SaveData;
	int						m_iIndex;

	CTGAImagePanel			*m_pImg;
	vgui::ImagePanel		*m_pAutoSaveImg;
	vgui::Label				*m_pChapter;
	vgui::Label				*m_pDate;
	vgui::Label				*m_pType;
};

class MainMenu_LoadGame : public HDTFMenuCanvas
{
	DECLARE_CLASS_SIMPLE(MainMenu_LoadGame, HDTFMenuCanvas);
public:
	MainMenu_LoadGame(vgui::Panel *parent, const char *name);
	~MainMenu_LoadGame();

	void			OnBeforeSwitchTo();
	void			OnSwitchedFrom();

	static int		SaveReadNameAndComment(FileHandle_t f, char* name, char* comment);

protected:
	virtual void	ApplySchemeSettings(vgui::IScheme *pScheme);
	virtual void	PerformLayout();
	virtual void	OnMouseWheeled(int delta);
	virtual void	OnCommand(const char *command);

	virtual void		LookForSaves();
	bool				ParseSaveData(char const *pszFileName, char const *pszShortName, SaveGameDescription_t &save);
	virtual void		AddToSaveList(int saveIndex);
	virtual void		AddNoSavesLabel();
	static int __cdecl	SaveGameSortFunc(const void *lhs, const void *rhs);
	void				DeleteSaveGame(const char *fileName);
	void				FindSaveSlot(char *buffer, int bufsize);

	CUtlVector<SaveGameDescription_t>	m_SaveGames;
	CUtlVector<vgui::Panel *>			m_pSaves;

	vgui::HFont		m_hNoChapterFont;

	vgui::Button	*m_pDeleteButton;

	vgui::Panel		*m_pScrollHolder;
	vgui::Panel		*m_pScrollFrame;
	int				m_iScrollOffset;
	int				m_iMaxScroll;
	int				m_iScrollSize;
	int				m_iChapterCount;
};