#include "cbase.h"
#include "ui_menu_popup.h"

#include "vgui/ISurface.h"
#include "vgui/IInput.h"
#include "vgui/ILocalize.h"

using namespace vgui;

// ------------------------------------------------------------------------------------------
// GENERIC HDTF MENU POPUP
// ------------------------------------------------------------------------------------------
HDTFUI_Popup::HDTFUI_Popup(
	vgui::Panel *parent,
	const char *name,
	const char *message,
	EPopupType type,
	const char *ok_title,
	const char *cancel_title,
	const char *ok_command,
	const char *cancel_command) : BaseClass(parent, name)
{
	SetParent(parent);

	V_strcpy(m_pOKCommand, ok_command);
	V_strcpy(m_pCancelCommand, cancel_command);

	m_pFrame = new Panel(this, "hdtfui_popup_frame");
	m_pFrame->AddActionSignalTarget(this);

	m_pLabel = new Label(m_pFrame, "hdtfui_popup_label", message);
	// NOTE(wheatley): doesn't work when text have line-breaks :/
	// centers text around longest line, short lines snap to left
	m_pLabel->SetContentAlignment(Label::a_center);

	m_pOK = new Button(m_pFrame, "hdtfui_popup_button_ok", ok_title);
	m_pOK->AddActionSignalTarget(this);
	m_pOK->SetCommand("Command_OK");
	m_pOK->SetPaintBorderEnabled(false);
	m_pOK->SetContentAlignment(Label::a_center);

	if (type == POPUP_PROMPT)
	{
		m_pCancel = new Button(m_pFrame, "hdtfui_popup_button_cancel", cancel_title);
		m_pCancel->AddActionSignalTarget(this);
		m_pCancel->SetCommand("Command_CANCEL");
		m_pCancel->SetPaintBorderEnabled(false);
		m_pCancel->SetContentAlignment(Label::a_center);
	}
	else
	{
		m_pCancel = NULL;
	}
}

void HDTFUI_Popup::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
	m_pLabel->SetFont(pScheme->GetFont("MainMenuPopup"));
	m_pOK->SetFont(pScheme->GetFont("MainMenuButtons"));

	if (m_pCancel)
		m_pCancel->SetFont(pScheme->GetFont("MainMenuButtons"));
}

void HDTFUI_Popup::PerformLayout()
{
	int sw, sh;
	surface()->GetScreenSize(sw, sh);

	SetSize(sw, sh);

	m_iPopupW = 420;
	m_iPopupH = 120;

	m_iPopupX = (sw / 2) - (m_iPopupW / 2);
	m_iPopupY = (sh / 2) - (m_iPopupH / 2);

	m_pFrame->SetSize(m_iPopupW, m_iPopupH);
	m_pFrame->SetPos(m_iPopupX, m_iPopupY);

	const int textMargin = 12;
	m_pLabel->SetPos(textMargin, 0);
	m_pLabel->SetSize(m_iPopupW - textMargin * 2, m_iPopupH - 30);

	if (m_pCancel)
	{
		m_pOK->SetPos(0, m_iPopupH - 30);
		m_pOK->SetSize(m_iPopupW / 2.25f, 30);

		m_pCancel->SetPos(m_iPopupW - (m_iPopupW / 2.25f), m_iPopupH - 30);
		m_pCancel->SetSize(m_iPopupW / 2.25f, 30);
	}
	else
	{
		m_pOK->SetPos(0, m_iPopupH - 30);
		m_pOK->SetSize(m_iPopupW, 30);
	}
}

void HDTFUI_Popup::Paint()
{
	surface()->DrawSetColor(0, 0, 0, 225);
	surface()->DrawFilledRect(0, 0, GetWide(), GetTall());

	surface()->DrawSetColor(25, 25, 25, 255);
	surface()->DrawFilledRect(m_iPopupX, m_iPopupY, m_iPopupX + m_iPopupW, m_iPopupY + m_iPopupH);

	surface()->DrawSetColor(20, 20, 20, 255);
	surface()->DrawFilledRect(m_iPopupX, m_iPopupY + m_iPopupH - 30, m_iPopupX + m_iPopupW, m_iPopupY + m_iPopupH);
}

void HDTFUI_Popup::OnKeyCodeTyped(ButtonCode_t code)
{
	if (code == KEY_ENTER)
	{
		OnCommand("Command_OK");
	}
	else if (code == KEY_ESCAPE)
	{
		OnCommand("Command_CANCEL");
	}
	else
	{
		BaseClass::OnKeyCodeTyped(code);
	}
}

void HDTFUI_Popup::OnCommand(const char *command)
{
	if (FStrEq(command, "Command_OK"))
	{
		KeyValues *command = new KeyValues("Command", "command", m_pOKCommand);
		PostActionSignal(command->MakeCopy());
		command->deleteThis();
		OnClose();
	}
	else if (FStrEq(command, "Command_CANCEL"))
	{
		KeyValues *command = new KeyValues("Command", "command", m_pCancelCommand);
		PostActionSignal(command->MakeCopy());
		command->deleteThis();
		OnClose();
	}
	else
	{
		BaseClass::OnCommand(command);
	}
}

void HDTFUI_Popup::OnClose()
{
	vgui::input()->SetAppModalSurface(NULL);

	SetVisible(false);

	MarkForDeletion();
}

void HDTFUI_Popup::TakeControl()
{
	MoveToFront();
	MakePopup();

	vgui::input()->SetAppModalSurface(GetVPanel());
}

// ------------------------------------------------------------------------------------------
// 3RD PARTY SOFTWARE INFORMATION POPUP
// ------------------------------------------------------------------------------------------
HDTFUI_3rdPartyPopup::HDTFUI_3rdPartyPopup(vgui::Panel *parent) : BaseClass(
	parent,
	"3rdPartyLegal",
	"#HDTF_3rdPartyNotes")
{
	m_pImage = new ImagePanel(m_pFrame, "BIKLogo");
	m_pImage->SetImage("techcredits/bink");
}

void HDTFUI_3rdPartyPopup::PerformLayout()
{
	int sw, sh;
	surface()->GetScreenSize(sw, sh);

	SetSize(sw, sh);

	m_iPopupW = 600;
	m_iPopupH = 180;

	m_iPopupX = (sw / 2) - (m_iPopupW / 2);
	m_iPopupY = (sh / 2) - (m_iPopupH / 2);

	m_pFrame->SetSize(m_iPopupW, m_iPopupH);
	m_pFrame->SetPos(m_iPopupX, m_iPopupY);

	m_pLabel->SetPos(68, 0);
	m_pLabel->SetSize(m_iPopupW - 68, m_iPopupH - 30);

	if (m_pCancel)
	{
		m_pOK->SetPos(0, m_iPopupH - 30);
		m_pOK->SetSize(m_iPopupW / 2.25f, 30);

		m_pCancel->SetPos(m_iPopupW - (m_iPopupW / 2.25f), m_iPopupH - 30);
		m_pCancel->SetSize(m_iPopupW / 2.25f, 30);
	}
	else
	{
		m_pOK->SetPos(0, m_iPopupH - 30);
		m_pOK->SetSize(m_iPopupW, 30);
	}

	m_pImage->SetSize(64, 64);
	m_pImage->SetPos(3, ((m_iPopupH - 30) / 2) - (m_pImage->GetTall() / 2));
}

// ------------------------------------------------------------------------------------------
// HDTF NEW GAME COMMENTARY WARNING
// ------------------------------------------------------------------------------------------
HDTFUI_CommentaryPopup::HDTFUI_CommentaryPopup(
	vgui::Panel *parent,
	const char *ok_command,
	const char *cancel_command) : BaseClass(
	parent,
	"commentary_warning",
	"#HDTF_NewGame_CommentaryStartWarning",
	POPUP_PROMPT,
	"#HDTF_Menu_OK",
	"#HDTF_Menu_Cancel",
	ok_command,
	cancel_command)
{
}

void HDTFUI_CommentaryPopup::PerformLayout()
{
	int sw, sh;
	surface()->GetScreenSize(sw, sh);

	SetSize(sw, sh);

	m_iPopupW = 480;
	m_iPopupH = 360;

	m_iPopupX = (sw / 2) - (m_iPopupW / 2);
	m_iPopupY = (sh / 2) - (m_iPopupH / 2);

	m_pFrame->SetSize(m_iPopupW, m_iPopupH);
	m_pFrame->SetPos(m_iPopupX, m_iPopupY);

	m_pLabel->SetPos(12, 0);
	m_pLabel->SetSize(m_iPopupW - 24, m_iPopupH - 30);
	m_pLabel->SetWrap(true);

	if (m_pCancel)
	{
		m_pOK->SetPos(0, m_iPopupH - 30);
		m_pOK->SetSize(m_iPopupW / 2.25f, 30);

		m_pCancel->SetPos(m_iPopupW - (m_iPopupW / 2.25f), m_iPopupH - 30);
		m_pCancel->SetSize(m_iPopupW / 2.25f, 30);
	}
	else
	{
		m_pOK->SetPos(0, m_iPopupH - 30);
		m_pOK->SetSize(m_iPopupW, 30);
	}
}
