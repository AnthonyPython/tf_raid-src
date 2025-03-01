#pragma once

#include "ui_mainmenu.h"
#include "ui_menu_button.h"
#include "vgui_controls/Panel.h"
#include "vgui_controls/ComboBox.h"
#include "vgui_controls/Slider.h"
#include "inputsystem/iinputsystem.h"
#include "IGameUIFuncs.h"

#include "vgui/ISurface.h"
#include "vgui/Cursor.h"
#include "vgui/IInput.h"

#include "language.h"

enum optionstabs
{
	TAB_VIDEO
};

struct aspectratio_t
{
	int type;
	int sort;
	float ratio;
};

struct resolutiondescriptor_t
{
	int width;
	int height;
	int ratio;
};

struct AAdescriptor_t
{
	int samples;
	int quality;
};

enum setting
{
	SETTING_MODELS,
	SETTING_TEXTURES,
	SETTING_SHADERS,
	SETTING_REFLECTIONS,
	SETTING_SHADOWS,
	SETTING_COLORCORRECTION,
	SETTING_ANTIALIASING,
	SETTING_FILTERING,
	SETTING_MOTIONBLUR,
	SETTING_TONEMAPPING,
	SETTING_BLOOM
};

struct preset_t
{
	setting param;
	int		qualityLevel;
};

// forward declarations
class MainMenu_Options;
class MainMenu_Options_Video;

static vgui::Panel *g_HoverFocus = NULL;
static vgui::Panel *g_OptionsBindTarget = NULL;

// =================================================================================
// SIMPLE PANEL TO DRAW A COLOR OVER SCREEN
// =================================================================================
class HDTF_Blackout : public vgui::Panel
{
	DECLARE_CLASS_SIMPLE(HDTF_Blackout, vgui::Panel);
public:
	HDTF_Blackout(Panel *parent, const char *panelName) : BaseClass(parent, panelName)
	{
		SetPaintBackgroundEnabled(true);
		SetZPos(99);
	}

protected:
	virtual void PaintBackground()
	{
		vgui::surface()->DrawSetColor(0, 0, 0, 240);
		vgui::surface()->DrawFilledRect(0, 0, GetWide(), GetTall());
	}
};

// =================================================================================
// CUSTOM MENU CONTROL [MODIFIED PAINT FUNCTION]
// =================================================================================
class HDTFUI_Menu : public vgui::Menu
{
	DECLARE_CLASS_SIMPLE(HDTFUI_Menu, vgui::Menu);
public:
	HDTFUI_Menu(Panel *parent, const char *panelName) : BaseClass(parent, panelName) 
	{
		SetPaintBackgroundEnabled(false);
		SetPaintBorderEnabled(false);
		SetZPos(99);
	}
};

// =================================================================================
// CUSTOM COMBO BOX CONTROL [MODIFIED MENU CLASS]
// =================================================================================
class HDTF_ComboBox : public vgui::ComboBox
{
	DECLARE_CLASS_SIMPLE(HDTF_ComboBox, vgui::ComboBox);
public:
	HDTF_ComboBox(vgui::Panel *parent, const char *name, int rows, bool allowedit) : BaseClass(parent, name, rows, allowedit) 
	{
		if (m_pDropDown != NULL)
		{
			m_pDropDown->MarkForDeletion();
			m_pDropDown = NULL;
		}

		m_pDropDown = new HDTFUI_Menu(this, NULL);
		m_pDropDown->AddActionSignalTarget(this);
		m_pDropDown->SetTypeAheadMode(vgui::Menu::TYPE_AHEAD_MODE);
	}

	virtual void OnShowMenu(vgui::Menu *menu)
	{
		KeyValues *pKV = new KeyValues("OnMenuPopulated");
		PostActionSignal(pKV->MakeCopy());
		pKV->deleteThis();
	}

	virtual void OnHideMenu(vgui::Menu *menu)
	{
		KeyValues *pKV = new KeyValues("OnMenuHidden");
		PostActionSignal(pKV->MakeCopy());
		pKV->deleteThis();
	}
};

// =================================================================================
// CUSTOM COMBO BOX CONTROL \W LABEL
// =================================================================================
class HDTFUI_LabeledComboBox : public vgui::Panel
{
	DECLARE_CLASS_SIMPLE(HDTFUI_LabeledComboBox, vgui::Panel);
public:
	HDTFUI_LabeledComboBox(vgui::Panel *parent, const char *name, const char *caption, int rows, bool allowedit = false) 
		: vgui::Panel(parent, name)
	{
		m_pComboBox = new HDTF_ComboBox(this, "labeledComboBox_Box", rows, allowedit);
		m_pLabel = new vgui::Label(this, "labeledComboBox_Label", caption);

		m_pLabel->AddActionSignalTarget(this);
		m_pComboBox->AddActionSignalTarget(this);

		m_iGradient = vgui::surface()->CreateNewTextureID();
		vgui::surface()->DrawSetTextureFile(m_iGradient, "vgui/gradient_h", true, true);

		m_bIsHovered = false;
	}

	void ApplySchemeSettings(vgui::IScheme *pScheme)
	{
		BaseClass::ApplySchemeSettings(pScheme);

		m_pLabel->SetFont(pScheme->GetFont("MainMenuButtons", false));
		m_pComboBox->SetFont(pScheme->GetFont("MainMenuButtons", false));
		m_pComboBox->SetPaintBorderEnabled(false);
		m_pComboBox->SetZPos(1);
	}

	void PerformLayout()
	{
		const int inset = vgui::scheme()->GetProportionalScaledValue(25);
		const int width = GetWide() - inset;

		m_pLabel->SetPos(inset, 0);
		m_pLabel->SetSize(width, GetTall());

		m_pComboBox->SetPos(inset + width / 2, 0);
		m_pComboBox->SetSize(width / 2, GetTall());
	}

	void OnMessage(const KeyValues *params, vgui::VPANEL panel)
	{
		BaseClass::OnMessage(params, panel);

		// reroute to parent
		if (FStrEq(params->GetName(), "OnMenuPopulated") || FStrEq(params->GetName(), "OnMenuHidden"))
		{
			PostActionSignal(params->MakeCopy());
		}
		else if (FStrEq(params->GetName(), "TextChanged"))
		{
			KeyValues *message = new KeyValues("SelectionChanged", "new", m_pComboBox->GetActiveItem());
			PostActionSignal(message->MakeCopy());
			message->deleteThis();
		}
	}

	void OnThink()
	{
		BaseClass::OnThink();

		const bool hovered =
			(m_pComboBox->IsDropdownVisible()
			|| m_pComboBox->IsCursorOver()
			|| m_pLabel->IsCursorOver()
			|| IsCursorOver())
			&& IsEnabled();

		if (g_HoverFocus != NULL && g_HoverFocus != this)
		{
			return;
		}

		if (m_bIsHovered != hovered)
		{
			m_bIsHovered = hovered;
			OnHoverStateChanged(m_bIsHovered);

			if (m_bIsHovered)
				g_HoverFocus = this;
			else
				g_HoverFocus = NULL;
		}
	}

	void OnHoverStateChanged(bool state)
	{
		if (!IsEnabled())
			return;

		if (state)
		{
			m_pComboBox->SetFgColor(Color(255, 25, 0, 255));
			m_pLabel->SetFgColor(Color(255, 25, 0, 255));
		}
		else
		{
			m_pComboBox->SetFgColor(Color(255, 255, 255, 255));
			m_pLabel->SetFgColor(Color(255, 255, 255, 255));
		}
	}

	bool IsHovered()
	{
		return m_bIsHovered;
	}

	void PaintBackground()
	{
		BaseClass::PaintBackground();

		if (IsHovered())
		{
			vgui::surface()->DrawSetColor(255, 25, 0, 255);
			vgui::surface()->DrawFilledRect(0, 0, GetTall() / 2.5f, GetTall());

			vgui::surface()->DrawSetColor(255, 25, 0, 15);
			vgui::surface()->DrawSetTexture(m_iGradient);
			vgui::surface()->DrawTexturedRect(GetTall() / 2.5f, 0, GetWide() / 2, GetTall());
		}
	}

	void DeleteAllItems() { m_pComboBox->DeleteAllItems(); }
	void ActivateItem(int id) { m_pComboBox->ActivateItem(id); }
	int AddItem(const char *name, const KeyValues *userdata) { return m_pComboBox->AddItem(name, userdata); }
	int GetItemCount() { return m_pComboBox->GetItemCount(); }
	int GetActiveItem() { return m_pComboBox->GetActiveItem(); }
	KeyValues *GetItemUserData(const int id) { return m_pComboBox->GetItemUserData(id); }
	KeyValues *GetActiveItemUserData() { return m_pComboBox->GetActiveItemUserData(); }

private:
	vgui::ComboBox	*m_pComboBox;
	vgui::Label		*m_pLabel;

	bool			m_bIsHovered;
	int				m_iGradient;
};

// =================================================================================
// CUSTOM SLIDER CONTROL \W LABEL
// =================================================================================
class HDTFUI_LabeledCvarSlider : public vgui::Panel
{
	DECLARE_CLASS_SIMPLE(HDTFUI_LabeledCvarSlider, vgui::Panel);
public:
	HDTFUI_LabeledCvarSlider(
		vgui::Panel *parent, 
		const char *name, 
		const char *caption, 
		const char *convar = NULL,
		float min = 0.f,
		float max = 1.f,
		bool showTicks = true)
			: BaseClass(parent, name)
	{
		m_pSlider = new vgui::Slider(this, "labeledSlider_Slider");
		m_pLabel = new vgui::Label(this, "labeledSlider_Label", caption);

		m_pSlider->SetRange(min * 100.f, max * 100.f);

		if (convar != NULL)
		{
			const int buffSize = V_strlen(convar) + 1;
			m_pCvarName = (char *)malloc(buffSize);
			V_memset(m_pCvarName, 0, buffSize);
			V_strcpy(m_pCvarName, convar);
		}
		else
		{
			m_pCvarName = NULL;
		}

		if (showTicks)
		{
			char captionMin[32];
			char captionMax[32];

			Q_snprintf(captionMin, sizeof(captionMin), "%.2f", min);
			Q_snprintf(captionMax, sizeof(captionMax), "%.2f", max);

			m_pSlider->SetNumTicks(10);
			m_pSlider->SetTickCaptions(captionMin, captionMax);
		}

		m_pLabel->AddActionSignalTarget(this);
		m_pSlider->AddActionSignalTarget(this);

		m_iGradient = vgui::surface()->CreateNewTextureID();
		vgui::surface()->DrawSetTextureFile(m_iGradient, "vgui/gradient_h", true, true);
	}
	
	~HDTFUI_LabeledCvarSlider()
	{
		if(m_pCvarName != NULL)
			free(m_pCvarName);
	}

	void ApplySchemeSettings(vgui::IScheme *pScheme)
	{
		BaseClass::ApplySchemeSettings(pScheme);

		m_pLabel->SetFont(pScheme->GetFont("MainMenuButtons", false));

		m_pSlider->SetFgColor(Color(255, 255, 255, 255));
		m_pSlider->SetPaintBorderEnabled(false);
		m_pSlider->SetZPos(1);
	}

	void PerformLayout()
	{
		const int inset = vgui::scheme()->GetProportionalScaledValue(25);
		const int width = GetWide() - inset;

		m_pLabel->SetPos(inset, 0);
		m_pLabel->SetSize(width, GetTall());

		m_pSlider->SetPos(inset + width / 2, 0);
		m_pSlider->SetSize(width / 2, GetTall());
	}

	void OnMessage(const KeyValues *params, vgui::VPANEL panel)
	{
		BaseClass::OnMessage(params, panel);

		if (FStrEq(params->GetName(), "SliderMoved"))
		{
			if (m_pCvarName != NULL)
			{
				KeyValues *message = new KeyValues("SelectionChanged");
				PostActionSignal(message->MakeCopy());
				message->deleteThis();
			}
		}
	}

	void OnThink()
	{
		BaseClass::OnThink();

		const bool hovered =
			(m_pSlider->IsCursorOver()
			|| m_pLabel->IsCursorOver()
			|| IsCursorOver())
			&& IsEnabled();

		if (g_HoverFocus != NULL && g_HoverFocus != this)
		{
			return;
		}

		if (m_bIsHovered != hovered)
		{
			m_bIsHovered = hovered;
			OnHoverStateChanged(m_bIsHovered);

			if (m_bIsHovered)
				g_HoverFocus = this;
			else
				g_HoverFocus = NULL;
		}
	}

	void OnHoverStateChanged(bool state)
	{
		if (!IsEnabled())
			return;

		if (state)
		{
			m_pSlider->SetFgColor(Color(255, 25, 0, 255));
			m_pLabel->SetFgColor(Color(255, 25, 0, 255));
		}
		else
		{
			m_pSlider->SetFgColor(Color(255, 255, 255, 255));
			m_pLabel->SetFgColor(Color(255, 255, 255, 255));
		}
	}

	bool IsHovered()
	{
		return m_bIsHovered;
	}

	void PaintBackground()
	{
		BaseClass::PaintBackground();

		if (IsHovered())
		{
			vgui::surface()->DrawSetColor(255, 25, 0, 255);
			vgui::surface()->DrawFilledRect(0, 0, GetTall() / 2.5f, GetTall());

			vgui::surface()->DrawSetColor(255, 25, 0, 15);
			vgui::surface()->DrawSetTexture(m_iGradient);
			vgui::surface()->DrawTexturedRect(GetTall() / 2.5f, 0, GetWide() / 2, GetTall());
		}
	}

	void SetTickCaptions(const char *left, const char *right) { m_pSlider->SetTickCaptions(left, right); }
	float GetValue() { return ((float)m_pSlider->GetValue()) / 100.f; }
	void SetValue(float value) { m_pSlider->SetValue(value * 100.f); }
	char *GetConVar() { return m_pCvarName; }

private:
	vgui::Slider	*m_pSlider;
	vgui::Label		*m_pLabel;

	char			*m_pCvarName;

	bool			m_bRemaped;
	bool			m_bIsHovered;
	int				m_iGradient;
};

// =================================================================================
// CUSTOM CONTROL TO DISPLAY BOUND KEYS
// =================================================================================
class HDTFUI_LabeledBindEntry : public vgui::Panel
{
	DECLARE_CLASS_SIMPLE(HDTFUI_LabeledBindEntry, vgui::Panel);
public:
	HDTFUI_LabeledBindEntry(vgui::Panel *parent, const char *command = NULL, const char *description = NULL)
		: vgui::Panel(parent, "keybinder")
	{
		m_bUnBound = false;
		m_bCapturing = false;
		m_bIsBlank = false;
		m_bIsHovered  = false;

		m_iBoundKey = BUTTON_CODE_INVALID;
		m_iLastBind = BUTTON_CODE_INVALID;

		if (command == NULL || description == NULL)
			m_bIsBlank = true;

		m_pButton = new vgui::Button(this, "labeledBindEntry_Box", "");
		m_pLabel = new vgui::Label(this, "labeledBindEntry_Label", description);

		m_pCommand[0] = NULL;

		if (!IsBlank())
		{
			Q_strncpy(m_pCommand, command, sizeof(m_pCommand));
			m_pButton->SetCommand("StartCapture");
			m_pButton->AddActionSignalTarget(this);
		}
		else
		{
			m_pButton->SetVisible(false);
		}

		m_pLabel->SetMouseInputEnabled(false);

		m_iGradient = vgui::surface()->CreateNewTextureID();
		vgui::surface()->DrawSetTextureFile(m_iGradient, "vgui/gradient_h", true, true);

		SetKeyBoardInputEnabled(true);
		SetMouseInputEnabled(true);
	}

	void ApplySchemeSettings(vgui::IScheme *pScheme)
	{
		BaseClass::ApplySchemeSettings(pScheme);

		m_pLabel->SetFont(pScheme->GetFont("MainMenuButtons", false));
		m_pButton->SetFont(pScheme->GetFont("MainMenuButtons", false));
		m_pButton->SetPaintBorderEnabled(false);
		m_pButton->SetZPos(1);
	}

	void PerformLayout()
	{
		const int inset = vgui::scheme()->GetProportionalScaledValue(25);
		const int width = GetWide() - inset;

		m_pLabel->SetPos(inset, 0);
		m_pLabel->SetSize(width, GetTall());

		m_pButton->SetPos(width - width / 3, 0);
		m_pButton->SetSize(width / 3, GetTall());
	}

	void OnMessage(const KeyValues *params, vgui::VPANEL panel)
	{
		BaseClass::OnMessage(params, panel);

		// reroute to parent
		if (FStrEq(params->GetName(), "OnMenuPopulated") || FStrEq(params->GetName(), "OnMenuHidden"))
		{
			PostActionSignal(params->MakeCopy());
		}
	}

	void OnCommand(const char *command)
	{
		if (FStrEq(command, "StartCapture"))
		{
			StartCapture();
		}
		else
		{
			BaseClass::OnCommand(command);
		}
	}

	void OnThink()
	{
		BaseClass::OnThink();

		if (IsBlank())
			return;

		const bool hovered =
			(m_pButton->IsCursorOver()
				|| m_pLabel->IsCursorOver()
				|| IsCursorOver())
			&& IsEnabled();

		if (g_HoverFocus != NULL && g_HoverFocus != this)
		{
			return;
		}

		if (m_bIsHovered != hovered)
		{
			m_bIsHovered = hovered;
			OnHoverStateChanged(m_bIsHovered);

			if (m_bIsHovered)
				g_HoverFocus = this;
			else
				g_HoverFocus = NULL;
		}
	}

	void OnHoverStateChanged(bool state)
	{
		if (!IsEnabled() || g_OptionsBindTarget != NULL)
			return;

		if (state)
		{
			m_pButton->SetFgColor(Color(255, 25, 0, 255));
			m_pLabel->SetFgColor(Color(255, 25, 0, 255));
		}
		else
		{
			m_pButton->SetFgColor(Color(255, 255, 255, 255));
			m_pLabel->SetFgColor(Color(255, 255, 255, 255));
		}
	}

	bool IsHovered()
	{
		return m_bIsHovered;
	}

	void PaintBackground()
	{
		BaseClass::PaintBackground();

		if (IsBlank())
		{
			const int inset = vgui::scheme()->GetProportionalScaledValue(25);
			const int y = GetTall() - 1;
			vgui::surface()->DrawSetColor(255, 255, 255, 255);
			vgui::surface()->DrawLine(inset, y, GetWide(), y);
		}

		if (IsHovered() && g_OptionsBindTarget == NULL)
		{
			vgui::surface()->DrawSetColor(255, 25, 0, 255);
			vgui::surface()->DrawFilledRect(0, 0, GetTall() / 2.5f, GetTall());

			vgui::surface()->DrawSetColor(255, 25, 0, 15);
			vgui::surface()->DrawSetTexture(m_iGradient);
			vgui::surface()->DrawTexturedRect(GetTall() / 2.5f, 0, GetWide() / 2, GetTall());
		}

		if (m_bCapturing)
		{
			vgui::surface()->DrawSetColor(255, 25, 0, 100);
			vgui::surface()->DrawFilledRect(0, 0, GetWide(), GetTall());
		}
	}

	void StartCapture()
	{
		if (g_OptionsBindTarget != NULL)
			return;

		m_bCapturing = true;

		vgui::input()->SetMouseFocus(GetVPanel());
		vgui::input()->SetMouseCapture(GetVPanel());

		vgui::input()->GetCursorPos(m_iMouseX, m_iMouseY);
		SetCursor(vgui::dc_blank);

		m_pButton->SetVisible(false);
		m_pButton->SetEnabled(false);
		engine->ExecuteClientCmd("gameui_preventescape");

		g_OptionsBindTarget = this;
	}

	void FinishCapture()
	{
		m_bCapturing = false;
		vgui::input()->SetMouseCapture(NULL);
		vgui::input()->SetMouseFocus(NULL);

		SetCursor(vgui::dc_arrow);
		vgui::surface()->SetCursor(vgui::dc_arrow);
		vgui::input()->SetCursorPos(m_iMouseX, m_iMouseY);

		m_pButton->SetEnabled(true);
		m_pButton->SetVisible(true);
		engine->ExecuteClientCmd("gameui_allowescape");

		g_OptionsBindTarget = NULL;
	}

	void OnKeyCodeTyped(vgui::KeyCode code)
	{
		if (m_bCapturing)
		{
			GetParent()->GetParent()->OnKeyCodeTyped(code);
		}
		else
		{
			BaseClass::OnKeyCodeTyped(code);
		}
	}

	void OnMousePressed(vgui::MouseCode code)
	{
		if (!IsBlank() && !m_bCapturing && code == MOUSE_LEFT)
		{
			StartCapture();
			return;
		}

		if (!IsBlank() && !m_bCapturing && code == MOUSE_RIGHT)
		{
			UnBind();
			UpdateBoundKey();
			PostActionSignal(new KeyValues("Command", "command", "ExternalKeyChange"));
			return;
		}

		if (m_bCapturing)
		{
			GetParent()->GetParent()->GetParent()->OnMousePressed(code);
		}
		else
		{
			BaseClass::OnMousePressed(code);
		}
	}

	void UpdateBoundKey()
	{
		if (IsBlank())
			return;

		if (m_iBoundKey != BUTTON_CODE_INVALID)
		{
			if(m_iBoundKey != BUTTON_CODE_NONE && !m_bUnBound)
				m_pButton->SetText(g_pInputSystem->ButtonCodeToString(m_iBoundKey));
			else
				m_pButton->SetText("");

			return;
		}

		m_iBoundKey = gameuifuncs->GetButtonCodeForBind(m_pCommand);
		m_iLastBind = m_iBoundKey;
		
		if (m_iBoundKey != BUTTON_CODE_NONE && m_iBoundKey != BUTTON_CODE_INVALID)
			m_pButton->SetText(g_pInputSystem->ButtonCodeToString(m_iBoundKey));
	}

	void UnbindLast()
	{
		if (m_iLastBind != BUTTON_CODE_INVALID && m_iLastBind != BUTTON_CODE_NONE)
		{
			const char *oldKey = g_pInputSystem->ButtonCodeToString(m_iLastBind);
			char command[64];
			Q_snprintf(command, sizeof(command), "unbind \"%s\"\n", oldKey);
			engine->ClientCmd_Unrestricted(command);
		}
	}

	void Revert()
	{
		if (m_iLastBind != BUTTON_CODE_INVALID)
		{
			m_iBoundKey = m_iLastBind;
		}

		m_bUnBound = false;
	}

	void ApplyBind()
	{
		if (IsBlank())
			return;

		if (m_iBoundKey == BUTTON_CODE_NONE || m_iBoundKey == BUTTON_CODE_INVALID)
			return;

		if (m_bUnBound)
		{
			const char *key = g_pInputSystem->ButtonCodeToString(m_iBoundKey);
			char command[64];
			Q_snprintf(command, sizeof(command), "unbind \"%s\"\n", key);
			engine->ClientCmd_Unrestricted(command);
			m_iBoundKey = BUTTON_CODE_NONE;
		}
		else
		{
			const char *key = g_pInputSystem->ButtonCodeToString(m_iBoundKey);
			char command[64];
			Q_snprintf(command, sizeof(command), "bind \"%s\" \"%s\"\n", key, m_pCommand);

			engine->ClientCmd_Unrestricted(command);
			m_iLastBind = m_iBoundKey;
		}
	}

	void			UnBind() { if(!IsBlank()) m_bUnBound = true; };
	bool			IsBlank() { return m_bIsBlank; }
	ButtonCode_t	GetButton() { return m_iBoundKey; }
	const char		*GetCommand() { return m_pCommand; }
	void			ChangeBind(ButtonCode_t code) 
	{
		m_bUnBound = false; 
		m_iBoundKey = code; 
	}

private:
	bool			m_bUnBound;
	ButtonCode_t	m_iBoundKey;
	ButtonCode_t	m_iLastBind;
	char			m_pCommand[64];
	vgui::Button	*m_pButton;
	vgui::Label		*m_pLabel;

	bool			m_bCapturing;
	bool			m_bIsBlank;
	bool			m_bIsHovered;
	int				m_iGradient;

	int				m_iMouseX;
	int				m_iMouseY;
};

// =================================================================================
// VIDEO SETTINGS CANVAS
// =================================================================================
class MainMenu_Options_Video : public HDTFMenuCanvas
{
	DECLARE_CLASS_SIMPLE(MainMenu_Options_Video, HDTFMenuCanvas);
public:
	MainMenu_Options_Video(vgui::Panel *parent);

	void			OnSwitchedTo();
	bool			CanLeave();

protected:
	virtual void	ApplySchemeSettings(vgui::IScheme *pScheme);
	virtual void	PerformLayout();
	virtual void	OnCommand(const char *command);

private:
			int		FindNearestRatio(vmode_t *mode);
			void	FormatResolutionName(vmode_t *mode, char *buffer, const int size);

			void	FillResolutionList();
			void	SetupElements();
			void	SetActiveValues();
			void	ApplyPreset(int preset);
			void	ApplyChanges();
			void	ApplyRecommended();

			void	AddItemToScroll(vgui::Panel *panel);

	MESSAGE_FUNC_PARAMS(OnSelectionChanged, "SelectionChanged", data);

	MESSAGE_FUNC_PARAMS(OnMenuPopulated, "OnMenuPopulated", data);
	MESSAGE_FUNC_PARAMS(OnMenuHidden, "OnMenuHidden", data);

	HDTF_Blackout	*m_pBlackOut;

	vgui::Panel		*m_pScrollHolder;
	vgui::Panel		*m_pScrollFrame;
	int				m_iScrollOffset;
	int				m_iMaxScroll;
	int				m_iScrollSize;
	int				m_iItemOffset;

	bool			m_bShouldUpdateResolution;
	int				m_iTargetWidth;
	int				m_iTargetHeight;

	bool			m_bHasUnsaved;

	HDTFUI_LabeledComboBox		*m_pResolution;
	HDTFUI_LabeledComboBox		*m_pDisplayMode;

	HDTFUI_LabeledComboBox		*m_pPreset;

	HDTFUI_LabeledComboBox		*m_pModelQuality;
	HDTFUI_LabeledComboBox		*m_pTextureQuality;
	HDTFUI_LabeledComboBox		*m_pShaderQuality;
	HDTFUI_LabeledComboBox		*m_pReflectionsQuality;
	HDTFUI_LabeledComboBox		*m_pShadowQuality;
	HDTFUI_LabeledComboBox		*m_pColorCorrect;
	HDTFUI_LabeledComboBox		*m_pAAMode;
	HDTFUI_LabeledComboBox		*m_pFilteringMode;
	HDTFUI_LabeledComboBox		*m_pVSync;
	HDTFUI_LabeledComboBox		*m_pMotionBlur;
	HDTFUI_LabeledComboBox		*m_pMCRendering;
	HDTFUI_LabeledComboBox		*m_pToneMapping;
	HDTFUI_LabeledComboBox		*m_pBloom;

	HDTFUI_LabeledCvarSlider	*m_pGamma;

	HDTFMenuButton				*m_p3rdParty;

	vgui::Button			*m_pApplyButton;
	vgui::Button			*m_pRecommendedButton;

	int						m_iAAQuantity;
	AAdescriptor_t			m_pAAQuality[16];

	int						m_iPreset;

	float					m_flIgnoreMessagesUntil;
};

// =================================================================================
// AUDIO SETTINGS CANVAS
// =================================================================================
class MainMenu_Options_Audio : public HDTFMenuCanvas
{
	DECLARE_CLASS_SIMPLE(MainMenu_Options_Audio, HDTFMenuCanvas);
public:
	MainMenu_Options_Audio(vgui::Panel *parent);

	static char		*GetNewLanguage() { return m_pNewLanguage; }

	void			OnSwitchedTo();
	bool			CanLeave();

protected:
	virtual void	ApplySchemeSettings(vgui::IScheme *pScheme);
	virtual void	PerformLayout();
	virtual void	OnCommand(const char *command);

private:
	void	SetupElements();
	void	SetActiveValues();
	void	ApplyChanges();
	void	ArrangeItem(vgui::Panel *item, int xpos, int ypos);

	MESSAGE_FUNC_PARAMS(OnSelectionChanged, "SelectionChanged", data);

	MESSAGE_FUNC_PARAMS(OnMenuPopulated, "OnMenuPopulated", data);
	MESSAGE_FUNC_PARAMS(OnMenuHidden, "OnMenuHidden", data);

	ELanguage					m_eActiveLanguage;
	static char					*m_pNewLanguage;

	int							m_iItemOffset;

	HDTF_Blackout				*m_pBlackOut;

	vgui::Button				*m_pApplyButton;

	HDTFUI_LabeledComboBox		*m_pSpeakerConfig;
	HDTFUI_LabeledComboBox		*m_pSoundQuality;
	HDTFUI_LabeledComboBox		*m_pLanguage;
	HDTFUI_LabeledComboBox		*m_pCaptioning;

	HDTFUI_LabeledCvarSlider	*m_pSFXVolume;
	HDTFUI_LabeledCvarSlider	*m_pCutsceneVolume;
	HDTFUI_LabeledCvarSlider	*m_pMusicVolume;

	HDTFMenuButton				*m_p3rdParty;

	float						m_flIgnoreMessagesUntil;
};

// =================================================================================
// CONTROLS SETTINGS CANVAS
// =================================================================================
class MainMenu_Options_Controls : public HDTFMenuCanvas
{
	DECLARE_CLASS_SIMPLE(MainMenu_Options_Controls, HDTFMenuCanvas);
public:
	MainMenu_Options_Controls(vgui::Panel *parent);

	void			SetBindTarget(HDTFUI_LabeledBindEntry *tgt) { m_pBindLookup = tgt; }
	void			RevertToDefaults();
	void			OnKeyCodeTyped(vgui::KeyCode code);
	void			OnMousePressed(vgui::MouseCode code);

	void			SetSwitchDelay(float delay = 0.15f) { m_flSwitchAllowed = delay; }
	bool			CanLeave();

	void			SetVisible(bool visible)
	{
		BaseClass::SetVisible(visible);
		if (m_pApplyButton)
			m_pApplyButton->SetVisible(visible);

		if (m_pRevertButton)
			m_pRevertButton->SetVisible(visible);
	}

	void			OnSwitchedTo();

protected:
	virtual void	ApplySchemeSettings(vgui::IScheme *pScheme);
	virtual void	PerformLayout();
	virtual void	OnCommand(const char *command);
	virtual void	OnThink();
	virtual void	PaintTraverse(bool repaint, bool allowForce);

private:
	void	SetupElements();
	void	SetActiveValues();
	void	ApplyChanges();
	void	ArrangeItem(vgui::Panel *item);
	void	ParseActions();
	void	OnMouseWheeled(int delta);
	void	UnbindButtonsOfKind(ButtonCode_t code);

	HDTFUI_LabeledBindEntry	*GetBinderByBind(const char *bind);

	MESSAGE_FUNC_PARAMS(OnSelectionChanged, "SelectionChanged", data);

	MESSAGE_FUNC_PARAMS(OnMenuPopulated, "OnMenuPopulated", data);
	MESSAGE_FUNC_PARAMS(OnMenuHidden, "OnMenuHidden", data);

	HDTFUI_LabeledBindEntry		*m_pBindLookup;

	vgui::Panel		*m_pScrollHolder;
	vgui::Panel		*m_pScrollFrame;
	int				m_iScrollOffset;
	int				m_iMaxScroll;
	int				m_iScrollSize;

	int							m_iItemOffset;

	float						m_flSwitchAllowed;

	HDTF_Blackout				*m_pBlackOut;

	vgui::Button				*m_pApplyButton;
	vgui::Button				*m_pRevertButton;

	HDTFUI_LabeledComboBox		*m_pReverseMouse;
	HDTFUI_LabeledComboBox		*m_pFilterMouse;
	HDTFUI_LabeledComboBox		*m_pRawInput;
	HDTFUI_LabeledComboBox		*m_pMouseAccelEnabled;
	HDTFUI_LabeledComboBox		*m_pJoystick;
	HDTFUI_LabeledComboBox		*m_pJoystickSouthPaw;
	HDTFUI_LabeledComboBox		*m_pJoystickReverse;

	HDTFUI_LabeledCvarSlider	*m_pMouseSensitivity;
	HDTFUI_LabeledCvarSlider	*m_pMouseAcceleration;
	HDTFUI_LabeledCvarSlider	*m_pJoystickSensitivityYaw;
	HDTFUI_LabeledCvarSlider	*m_pJoystickSensitivityPitch;

	HDTFUI_LabeledBindEntry		*m_pBindingsLabel;

	CUtlVector<HDTFUI_LabeledBindEntry *> m_pBinders;

	float						m_flIgnoreMessagesUntil;
};

// =================================================================================
// GAMEPLAY SETTINGS CANVAS
// =================================================================================
class MainMenu_Options_Gameplay : public HDTFMenuCanvas
{
	DECLARE_CLASS_SIMPLE(MainMenu_Options_Gameplay, HDTFMenuCanvas);
public:
	MainMenu_Options_Gameplay(vgui::Panel *parent);

	void			OnSwitchedTo();
	bool			CanLeave();

protected:
	virtual void	ApplySchemeSettings(vgui::IScheme *pScheme);
	virtual void	PerformLayout();
	virtual void	OnCommand(const char *command);

private:
	void	SetupElements();
	void	SetActiveValues();
	void	ApplyChanges();
	void	ArrangeItem(vgui::Panel *item, int xpos, int ypos);

	MESSAGE_FUNC_PARAMS(OnSelectionChanged, "SelectionChanged", data);

	MESSAGE_FUNC_PARAMS(OnMenuPopulated, "OnMenuPopulated", data);
	MESSAGE_FUNC_PARAMS(OnMenuHidden, "OnMenuHidden", data);

	int							m_iItemOffset;

	HDTF_Blackout				*m_pBlackOut;

	vgui::Button				*m_pApplyButton;

	HDTFUI_LabeledComboBox		*m_pDifficulty;
	HDTFUI_LabeledComboBox		*m_pCrosshair;
	HDTFUI_LabeledComboBox		*m_pHudFadeOut;

	HDTFUI_LabeledCvarSlider	*m_pFov;

	HDTFUI_LabeledComboBox		*m_pAutoLean;
	HDTFUI_LabeledComboBox		*m_pConsole;
	HDTFUI_LabeledComboBox		*m_pViewBob;

	HDTFUI_LabeledComboBox		*m_pAutoCrouchJump;

	float						m_flIgnoreMessagesUntil;
};

// =================================================================================
// OPTIONS CANVAS
// =================================================================================
class MainMenu_Options : public HDTFMenuCanvas
{
	DECLARE_CLASS_SIMPLE(MainMenu_Options, HDTFMenuCanvas);
public:
	MainMenu_Options(vgui::Panel *parent, const char *name);

protected:
	virtual void	ApplySchemeSettings(vgui::IScheme *pScheme);
	virtual void	OnCommand(const char *command);

private:
	HDTFMenuButton	*m_pButtonVideo;
	HDTFMenuButton	*m_pButtonAudio;
	HDTFMenuButton	*m_pButtonControls;
	HDTFMenuButton	*m_pButtonGameplay;

	HDTFMenuCanvas	*m_pCanvasVideo;
	HDTFMenuCanvas	*m_pCanvasAudio;
	HDTFMenuCanvas	*m_pCanvasControls;
	HDTFMenuCanvas	*m_pCanvasGameplay;
};