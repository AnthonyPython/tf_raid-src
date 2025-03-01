#pragma once

#include "ui_mainmenu.h"
#include "vgui_controls/Panel.h"
#include "vgui_controls/Label.h"
#include "vgui_controls/EditablePanel.h"
#include "vgui_controls/ImagePanel.h"
#include <vgui/ILocalize.h>

#define HDTF_CHAPTER_COUNT 16
#define HDTF_ACT_COUNT 5
#define HDTF_LAST_ACT (HDTF_ACT_COUNT - 1)

struct chapterdescriptor_t
{
	const char	*m_pChapterName;
	int			m_iChapterID;
	int			m_iAct;
	const char	*m_pMapName;
};

static chapterdescriptor_t g_ChapterList[]
{
	{ "#HDTF_Prologue_Title", 0, 0, "a0c0p1_blackmesa" },
//	{ "#HDTF_Prologue_Title", 0, 0, "a0c0p1" },
	{ "#HDTF_Chapter1_Title", 1, 1, "a1c1p1_hospital_1" },
//	{ "#HDTF_Chapter1_Title", 1, 1, "a1c1p1" },
	{ "#HDTF_Chapter2_Title", 2, 1, "a1c2p1_downtown_1" },
//	{ "#HDTF_Chapter2_Title", 2, 1, "a1c2p1" },
	{ "#HDTF_Chapter3_Title", 3, 1, "a1c3p1_canal" },
//	{ "#HDTF_Chapter3_Title", 3, 1, "a1c3p1" },
	{ "#HDTF_Chapter4_Title", 4, 1, "a1c4p2_mesa" },
//	{ "#HDTF_Chapter4_Title", 4, 1, "a1c4p1" },
	{ "#HDTF_Chapter5_Title", 5, 1, "a1c5p1_pier_1" },
//	{ "#HDTF_Chapter5_Title", 5, 1, "a1c5p1" },
	{ "#HDTF_Chapter6_Title", 1, 2, "a2c1p1_alaska" },
//	{ "#HDTF_Chapter6_Title", 1, 2, "a2c1p1" },
	{ "#HDTF_Chapter7_Title", 2, 2, "a2c2p1_lasers" },
//	{ "#HDTF_Chapter7_Title", 2, 2, "a2c2p1" },
	{ "#HDTF_Chapter8_Title", 1, 3, "a3c1p1_ship" },
//	{ "#HDTF_Chapter8_Title", 1, 3, "a3c1p1" },
	{ "#HDTF_Chapter9_Title", 2, 3, "a3c2p1_ci17-1" },
//	{ "#HDTF_Chapter9_Title", 2, 3, "a3c2p1" },
	{ "#HDTF_Chapter10_Title", 3, 3, "a3c3p1_blackmesaeast" },
//	{ "#HDTF_Chapter10_Title", 3, 3, "a3c3p1" },
	{ "#HDTF_Chapter11_Title", 4, 3, "a3c4p1_cave" },
//	{ "#HDTF_Chapter11_Title", 4, 3, "a3c4p1" },
	{ "#HDTF_Chapter12_Title", 5, 3, "a3c5p1_badcountry" },
//	{ "#HDTF_Chapter12_Title", 5, 3, "a3c5p1" },
	{ "#HDTF_Chapter13_Title", 6, 3, "a3c6p1_novapro" },
//	{ "#HDTF_Chapter13_Title", 6, 3, "a3c6p1" },
	{ "#HDTF_Chapter14_Title", 7, 3, "a3c7p1_prison_1" },
//	{ "#HDTF_Chapter14_Title", 7, 3, "a3c7p1" },
	{ "#HDTF_Epilogue_Title", 8, 3, "a3c8p1_forest" },
//	{ "#HDTF_Epilogue_Title", 8, 3, "a4c1p1" },
};

class MainMenu_ChapterEntry : public vgui::Panel
{
	DECLARE_CLASS_SIMPLE(MainMenu_ChapterEntry, vgui::Panel);
public:
	MainMenu_ChapterEntry(vgui::Panel *parent, const char *name, chapterdescriptor_t chapter, int index);
	~MainMenu_ChapterEntry();

	void ApplySchemeSettings(vgui::IScheme *pScheme);
	void PerformLayout();
	void SetChapter(chapterdescriptor_t chapter, int index);
	void Paint();

	virtual void OnMousePressed(vgui::MouseCode code);

	void OnCursorEntered()
	{
		if (!m_bIsHovered)
		{
			m_bIsHovered = true;
			OnHoverStateChanged(true);
		}
	}

	void OnCursorExited()
	{
		if (m_bIsHovered)
		{
			m_bIsHovered = false;
			OnHoverStateChanged(false);
		}
	}

	void OnHoverStateChanged(bool state);

	bool IsHovered()
	{
		return m_bIsHovered;
	}

private:
	vgui::Label			*m_pChapterLabel;
	vgui::Label			*m_pChapterTitle;
	vgui::ImagePanel	*m_pChapterImage;

	KeyValues			*actionMessage;

	int					m_iLineWidth;
	bool				m_bIsHovered;
	bool				m_bIsEnabled;
};

class MainMenu_ActEntry : public vgui::Panel
{
	DECLARE_CLASS_SIMPLE(MainMenu_ActEntry, vgui::Panel);
public:
	MainMenu_ActEntry(vgui::Panel *parent, const char *name, int number);

	void ApplySchemeSettings(vgui::IScheme *pScheme);
	void PerformLayout();
	void Paint();

private:
	vgui::Label	*m_pLabel;
};

class MainMenu_NewGame : public HDTFMenuCanvas
{
	DECLARE_CLASS_SIMPLE(MainMenu_NewGame, HDTFMenuCanvas);
public:
	MainMenu_NewGame(vgui::Panel *parent, const char *name);
	~MainMenu_NewGame();

	void			ReloadChapters();
	float			GetAlphaMultiplier() { return m_flTransparency; }

	virtual void	OnSwitchedTo();

protected:
	virtual void	ApplySchemeSettings(vgui::IScheme *pScheme);
	virtual void	OnMouseWheeled(int delta);
	virtual void	PerformLayout();
	virtual void	OnTick();
	virtual void	Paint();
	virtual void	OnCommand(const char *command);

	void ToggleCommentaryMode();
	void StartGame(int fromChapter, bool inCommentaryMode = false);

private:
	CUtlVector<vgui::Panel *> m_pChapterList;

	vgui::Button	*m_pCommentaryButton;
	vgui::Label		*m_pCommentaryTitle;
	vgui::Panel		*m_pChapterHolder;
	vgui::Panel		*m_pChapterFrame;
	int				m_iScrollOffset;
	int				m_iMaxScroll;
	int				m_iScrollSize;
	int				m_iChapterCount;

	bool			m_bIsInCommentaryMode;
	int				m_iSelectedCommentaryChapter;
	float			m_flTransparency;
	bool			m_bTargetCommentaryMode;

	MESSAGE_FUNC_PARAMS(OnStartNewGame, "StartNewGame", data);
};
