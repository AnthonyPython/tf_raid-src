#pragma once

#include "vgui_controls/Panel.h"
#include "vgui_controls/Button.h"
#include "vgui_controls/Label.h"
#include "vgui_controls/ImagePanel.h"

enum EPopupType
{
	POPUP_MESSAGE,
	POPUP_PROMPT
};

class HDTFUI_Popup : public vgui::Panel
{
	DECLARE_CLASS_SIMPLE(HDTFUI_Popup, vgui::Panel);
public:
	HDTFUI_Popup(
		vgui::Panel *parent,
		const char *name,
		const char *message,
		EPopupType type = POPUP_MESSAGE,
		const char *ok_title = "#HDTF_Menu_OK",
		const char *cancel_title = "#HDTF_Menu_Cancel",
		const char *ok_command = "PopupOK",
		const char *cancel_command = "PopupCancel");

	void			TakeControl();
	void			OnClose();

protected:
	virtual void	ApplySchemeSettings(vgui::IScheme *pScheme);
	virtual void	PerformLayout();
	virtual void	OnCommand(const char *command);
	virtual void	Paint();
	virtual void	OnKeyCodeTyped(ButtonCode_t code);

	int				m_iPopupX;
	int				m_iPopupY;
	int				m_iPopupW;
	int				m_iPopupH;

	char			m_pOKCommand[64];
	char			m_pCancelCommand[64];

	vgui::Panel		*m_pFrame;
	vgui::Label		*m_pLabel;
	vgui::Button	*m_pOK;
	vgui::Button	*m_pCancel;
};

class HDTFUI_3rdPartyPopup : public HDTFUI_Popup
{
	DECLARE_CLASS_SIMPLE(HDTFUI_3rdPartyPopup, HDTFUI_Popup);
public:
	HDTFUI_3rdPartyPopup(vgui::Panel *parent);

protected:
	virtual void		PerformLayout();

	vgui::ImagePanel	*m_pImage;
};

class HDTFUI_CommentaryPopup : public HDTFUI_Popup
{
	DECLARE_CLASS_SIMPLE(HDTFUI_CommentaryPopup, HDTFUI_Popup);
public:
	HDTFUI_CommentaryPopup(
		vgui::Panel *parent,
		const char *ok_command = "PopupOK",
		const char *cancel_command = "PopupCancel");

protected:
	virtual void		PerformLayout();
};
