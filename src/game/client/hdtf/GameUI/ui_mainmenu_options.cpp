#include "cbase.h"
#include "hdtf_ui.h"
#include "ui_mainmenu_options.h"
#include "ui_menu_popup.h"
#include "materialsystem/materialsystem_config.h"
#include "IGameUIFuncs.h"
#include "vgui/IInput.h"
#include "vgui/ISystem.h"
#include "vgui_controls/QueryBox.h"
#include "filesystem.h"

#include "steam/steam_api.h"

#define CONTROL_SPACING 1
#define CONTROL_HEIGHT BUTTONS_HEIGHT

#define INSERT_SPACER() m_iItemOffset += CONTROL_HEIGHT

#define VIEWBOB_INTENSITY_LOW 0.15f
#define VIEWBOB_INTENSITY_MED 0.5f
#define VIEWBOB_INTENSITY_DEF 1.0f

static void UpdateConVar(const char *cvar, int value)
{
	char cmd[64];
	Q_snprintf(cmd, sizeof(cmd), "%s %i", cvar, value);
	engine->ClientCmd_Unrestricted(cmd);
}

static void UpdateConVar(const char *cvar, float value)
{
	char cmd[64];
	Q_snprintf(cmd, sizeof(cmd), "%s %f", cvar, value);
	engine->ClientCmd_Unrestricted(cmd);
}

// some macroses to save space in future
#define ADD_QUALITY_LEVELS2(box, q1, q2) box->AddItem(q1, NULL); \
	box->AddItem(q2, NULL);

#define ADD_QUALITY_LEVELS3(box, q1, q2, q3) box->AddItem(q1, NULL); \
	box->AddItem(q2, NULL); \
	box->AddItem(q3, NULL);

#define ADD_QUALITY_LEVELS4(box, q1, q2, q3, q4) box->AddItem(q1, NULL); \
	box->AddItem(q2, NULL); \
	box->AddItem(q3, NULL); \
	box->AddItem(q4, NULL);

#define ADD_QUALITY_LEVELS6(box, q1, q2, q3, q4, q5, q6) box->AddItem(q1, NULL); \
	box->AddItem(q2, NULL); \
	box->AddItem(q3, NULL); \
	box->AddItem(q4, NULL); \
	box->AddItem(q5, NULL); \
	box->AddItem(q6, NULL);

using namespace vgui;

#pragma region Video options tab

static aspectratio_t g_SupportedRatios[] =
{
	{ 0, 0, 4.f / 3.f },
	{ 1, 1, 16.f / 9.f },
	{ 2, 2, 16.f / 10.f },
	{ 2, 3, 1.f },
};

static preset_t g_GraphicsPresets[4][11] =
{
	// low
	{
		{ SETTING_MODELS, 0 },
		{ SETTING_TEXTURES, 0 },
		{ SETTING_SHADERS, 0 },
		{ SETTING_REFLECTIONS, 0 },
		{ SETTING_SHADOWS, 0 },
		{ SETTING_COLORCORRECTION, 0 },
		{ SETTING_ANTIALIASING, 0 },
		{ SETTING_FILTERING, 0 },
		{ SETTING_MOTIONBLUR, 0 },
		{ SETTING_TONEMAPPING, 0 },
		{ SETTING_BLOOM, 0 }
	},
	// medium
	{
		{ SETTING_MODELS, 1 },
		{ SETTING_TEXTURES, 1 },
		{ SETTING_SHADERS, 0 },
		{ SETTING_REFLECTIONS, 1 },
		{ SETTING_SHADOWS, 1 },
		{ SETTING_COLORCORRECTION, 0 },
		{ SETTING_ANTIALIASING, 1 },
		{ SETTING_FILTERING, 1 },
		{ SETTING_MOTIONBLUR, 0 },
		{ SETTING_TONEMAPPING, 0 },
		{ SETTING_BLOOM, 0 }
	},
	// high
	{
		{ SETTING_MODELS, 2 },
		{ SETTING_TEXTURES, 2 },
		{ SETTING_SHADERS, 1 },
		{ SETTING_REFLECTIONS, 1 },
		{ SETTING_SHADOWS, 1 },
		{ SETTING_COLORCORRECTION, 1 },
		{ SETTING_ANTIALIASING, 2 },
		{ SETTING_FILTERING, 4 },
		{ SETTING_MOTIONBLUR, 1 },
		{ SETTING_TONEMAPPING, 1 },
		{ SETTING_BLOOM, 1 }
	},
	// very high
	{
		{ SETTING_MODELS, 2 },
		{ SETTING_TEXTURES, 3 },
		{ SETTING_SHADERS, 1 },
		{ SETTING_REFLECTIONS, 2 },
		{ SETTING_SHADOWS, 2 },
		{ SETTING_COLORCORRECTION, 1 },
		{ SETTING_ANTIALIASING, 7 },
		{ SETTING_FILTERING, 5 },
		{ SETTING_MOTIONBLUR, 1 },
		{ SETTING_TONEMAPPING, 1 },
		{ SETTING_BLOOM, 1 }
	},
};

MainMenu_Options_Video::MainMenu_Options_Video(vgui::Panel *parent) : HDTFMenuCanvas(parent, "options_VideoTab")
{
	m_pScrollFrame = new Panel(this, "videoScrollFrame");
	m_pScrollHolder = new Panel(m_pScrollFrame, "videoScroll");

	m_pScrollFrame->SetPaintBackgroundEnabled(true);
	m_pScrollFrame->SetBgColor(Color(255, 0, 0, 100));

	m_pResolution = new HDTFUI_LabeledComboBox(m_pScrollHolder, "options_Video_Resolution", "#HDTF_Video_Resolution", 10);
	m_pDisplayMode = new HDTFUI_LabeledComboBox(m_pScrollHolder, "options_Video_Mode", "#HDTF_Video_Mode", 10);

	m_pPreset = new HDTFUI_LabeledComboBox(m_pScrollHolder, "options_Video_Preset", "#HDTF_Video_Preset", 5);
	
	m_pModelQuality = new HDTFUI_LabeledComboBox(m_pScrollHolder, "options_Video_Model", "#HDTF_Video_Models", 3);
	m_pTextureQuality = new HDTFUI_LabeledComboBox(m_pScrollHolder, "options_Video_Texture", "#HDTF_Video_Textures", 4);
	m_pShaderQuality = new HDTFUI_LabeledComboBox(m_pScrollHolder, "options_Video_Shader", "#HDTF_Video_Shaders", 2);
	m_pReflectionsQuality = new HDTFUI_LabeledComboBox(m_pScrollHolder, "options_Video_Reflection", "#HDTF_Video_Reflections", 3);
	m_pShadowQuality = new HDTFUI_LabeledComboBox(m_pScrollHolder, "options_Video_Shadow", "#HDTF_Video_Shadows", 3);
	m_pColorCorrect = new HDTFUI_LabeledComboBox(m_pScrollHolder, "options_Video_CC", "#HDTF_Video_ColorCorrect", 2);
	m_pAAMode = new HDTFUI_LabeledComboBox(m_pScrollHolder, "options_Video_AA", "#HDTF_Video_AA", 4);
	m_pFilteringMode = new HDTFUI_LabeledComboBox(m_pScrollHolder, "options_Video_Filtering", "#HDTF_Video_Filtering", 6);
	m_pVSync = new HDTFUI_LabeledComboBox(m_pScrollHolder, "options_Video_VSync", "#HDTF_Video_VSync", 2);
	m_pMotionBlur = new HDTFUI_LabeledComboBox(m_pScrollHolder, "options_Video_MoBlur", "#HDTF_Video_MotionBlur", 2);
	m_pMCRendering = new HDTFUI_LabeledComboBox(m_pScrollHolder, "options_Video_MCR", "#HDTF_Video_MCRender", 2);
	m_pToneMapping = new HDTFUI_LabeledComboBox(m_pScrollHolder, "options_Video_ToneMapping", "#HDTF_Video_ToneMapping", 3);
	m_pBloom = new HDTFUI_LabeledComboBox(m_pScrollHolder, "options_Video_Bloom", "#HDTF_Video_Bloom", 2);

	m_pGamma = new HDTFUI_LabeledCvarSlider(this, "options_Video_Gamma", "#HDTF_Video_Gamma", "mat_monitorgamma", 1.6f, 2.6f);

	m_pApplyButton = new Button(this, "backButton", "#HDTF_Options_Apply");
	m_pApplyButton->SetCommand("apply");
	m_pApplyButton->AddActionSignalTarget(this);
	m_pApplyButton->SetVisible(true);
	m_pApplyButton->SetZPos(99);

	m_pRecommendedButton = new Button(this, "backButton", "#HDTF_Options_Recomended");
	m_pRecommendedButton->SetCommand("recommended");
	m_pRecommendedButton->AddActionSignalTarget(this);
	m_pRecommendedButton->SetVisible(true);
	m_pRecommendedButton->SetZPos(99);

	m_pResolution->AddActionSignalTarget(this);
	m_pDisplayMode->AddActionSignalTarget(this);

	m_pPreset->AddActionSignalTarget(this);

	m_pModelQuality->AddActionSignalTarget(this);
	m_pTextureQuality->AddActionSignalTarget(this);
	m_pShaderQuality->AddActionSignalTarget(this);
	m_pReflectionsQuality->AddActionSignalTarget(this);
	m_pShadowQuality->AddActionSignalTarget(this);
	m_pColorCorrect->AddActionSignalTarget(this);
	m_pAAMode->AddActionSignalTarget(this);
	m_pFilteringMode->AddActionSignalTarget(this);
	m_pVSync->AddActionSignalTarget(this);
	m_pMotionBlur->AddActionSignalTarget(this);
	m_pMCRendering->AddActionSignalTarget(this);
	m_pToneMapping->AddActionSignalTarget(this);
	m_pBloom->AddActionSignalTarget(this);

	m_pGamma->AddActionSignalTarget(this);

	m_p3rdParty = new HDTFMenuButton(this, "3rdPartyButton", "#HDTF_3rdPartyNotes_Button");
	m_p3rdParty->SetCommand("ShowLegalNotes");

	m_bHasUnsaved = false;
	m_iPreset = 0;

	m_pBlackOut = new HDTF_Blackout(this, "blackout");
	m_pBlackOut->SetVisible(false);

	m_flIgnoreMessagesUntil = 0.f;

	SetupElements();
}

void MainMenu_Options_Video::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	m_pApplyButton->SetFont(pScheme->GetFont("MainMenuButtons", false));
	m_pRecommendedButton->SetFont(pScheme->GetFont("MainMenuButtons", false));
}

void MainMenu_Options_Video::OnCommand(const char *command)
{
	if (FStrEq(command, "apply"))
	{
		ApplyChanges();
		m_pApplyButton->SetEnabled(false);
	}
	else if (FStrEq(command, "recommended"))
	{
		ApplyRecommended();
	}
	else if (FStrEq(command, "ShowLegalNotes"))
	{
		HDTFUI_Popup *popup = new HDTFUI_3rdPartyPopup(guiroot);
		popup->TakeControl();
	}
	else if (FStrEq(command, "ConfirmLeave"))
	{
		m_pApplyButton->SetEnabled(false);
		BaseClass::OnCommand("menu_back");
	}
	else
	{
		BaseClass::OnCommand(command);
	}
}

void MainMenu_Options_Video::OnSelectionChanged(KeyValues *data)
{
	// HACKHACK(wheatley): ignore messages from the frame where we initialized default values for the fields
	if (m_flIgnoreMessagesUntil >= gpGlobals->realtime)
	{
		return;
	}

	if (m_iPreset != m_pPreset->GetActiveItem())
	{
		m_iPreset = m_pPreset->GetActiveItem();

		// set to custom, do nothing
		if (m_iPreset == 0)
			return;

		ApplyPreset(clamp(m_iPreset - 1, 0, 3));
	}

	m_pApplyButton->SetEnabled(true);
}

void MainMenu_Options_Video::OnMenuPopulated(KeyValues *data)
{
	m_pBlackOut->SetVisible(true);
}

void MainMenu_Options_Video::OnMenuHidden(KeyValues *data)
{
	m_pBlackOut->SetVisible(false);
}

void MainMenu_Options_Video::PerformLayout()
{
	int wide, tall;
	surface()->GetScreenSize(wide, tall);
	SetSize(wide, tall);
	m_pBlackOut->SetSize(wide, tall);

	int spacing = scheme()->GetProportionalScaledValue(10);
	int xpos = scheme()->GetProportionalScaledValue(45);

	m_pScrollFrame->SetPos(xpos, (tall / 8) + spacing);
	m_pScrollFrame->SetSize((wide / 1.85f) - xpos, tall - ((tall / 8) + spacing) * 2);

	m_pScrollHolder->SetWide(m_pScrollFrame->GetWide());

	m_pApplyButton->SetPos(scheme()->GetProportionalScaledValue(180), tall - (tall / 9));
	m_pApplyButton->SetWide(scheme()->GetProportionalScaledValue(100));
	m_pApplyButton->SetFgColor(Color(255, 255, 255, 255));
	m_pApplyButton->SetSelectedColor(Color(100, 5, 0, 255), Color(0, 0, 0, 0));
	m_pApplyButton->SetArmedColor(Color(255, 25, 0, 255), Color(0, 0, 0, 0));
	m_pApplyButton->SetPaintBorderEnabled(false);

	m_pRecommendedButton->SetPos(scheme()->GetProportionalScaledValue(310), tall - (tall / 9));
	m_pRecommendedButton->SetWide(scheme()->GetProportionalScaledValue(130));
	m_pRecommendedButton->SetFgColor(Color(255, 255, 255, 255));
	m_pRecommendedButton->SetSelectedColor(Color(100, 5, 0, 255), Color(0, 0, 0, 0));
	m_pRecommendedButton->SetArmedColor(Color(255, 25, 0, 255), Color(0, 0, 0, 0));
	m_pRecommendedButton->SetPaintBorderEnabled(false);

	m_iItemOffset = 0;

	AddItemToScroll(m_pResolution);
	AddItemToScroll(m_pDisplayMode);

	INSERT_SPACER();

	AddItemToScroll(m_pPreset);

	INSERT_SPACER();

	AddItemToScroll(m_pModelQuality);
	AddItemToScroll(m_pTextureQuality);
	AddItemToScroll(m_pShaderQuality);
	AddItemToScroll(m_pReflectionsQuality);
	AddItemToScroll(m_pShadowQuality);
	AddItemToScroll(m_pColorCorrect);
	AddItemToScroll(m_pAAMode);
	AddItemToScroll(m_pFilteringMode);
	AddItemToScroll(m_pVSync);
	AddItemToScroll(m_pMotionBlur);
	AddItemToScroll(m_pMCRendering);
	AddItemToScroll(m_pToneMapping);
	AddItemToScroll(m_pBloom);

	INSERT_SPACER();

	AddItemToScroll(m_pGamma);

	INSERT_SPACER();

	AddItemToScroll(m_p3rdParty);

	m_iMaxScroll = m_pScrollHolder->GetTall() - m_pScrollFrame->GetTall();
}

void MainMenu_Options_Video::AddItemToScroll(Panel *panel)
{
	panel->SetParent(m_pScrollHolder);
	panel->SetSize(m_pScrollHolder->GetWide(), CONTROL_HEIGHT);
	panel->SetPos(0, m_iItemOffset);

	m_iItemOffset += CONTROL_HEIGHT + CONTROL_SPACING;
	m_pScrollHolder->SetTall(m_iItemOffset);
}

void MainMenu_Options_Video::SetupElements()
{
	FillResolutionList();

	m_pPreset->AddItem("#HDTF_Options_Custom", NULL);
	m_pPreset->AddItem("#HDTF_Options_Low", NULL);
	m_pPreset->AddItem("#HDTF_Options_Medium", NULL);
	m_pPreset->AddItem("#HDTF_Options_High", NULL);
	m_pPreset->AddItem("#HDTF_Options_Ultra", NULL);

	ADD_QUALITY_LEVELS2(m_pDisplayMode, "#HDTF_Video_Mode_F", "#HDTF_Video_Mode_W")
	ADD_QUALITY_LEVELS3(m_pModelQuality, "#HDTF_Options_Low", "#HDTF_Options_Medium", "#HDTF_Options_High")
	ADD_QUALITY_LEVELS4(m_pTextureQuality, "#HDTF_Options_Low", "#HDTF_Options_Medium", "#HDTF_Options_High", "#HDTF_Options_Ultra")
	ADD_QUALITY_LEVELS2(m_pShaderQuality, "#HDTF_Options_Low", "#HDTF_Options_High")
	ADD_QUALITY_LEVELS3(m_pReflectionsQuality, "#HDTF_Options_Low", "#HDTF_Options_Medium", "#HDTF_Options_High")
	ADD_QUALITY_LEVELS3(m_pShadowQuality, "#HDTF_Options_Low", "#HDTF_Options_Medium", "#HDTF_Options_High")
	ADD_QUALITY_LEVELS2(m_pColorCorrect, "#HDTF_Options_Disabled", "#HDTF_Options_Enabled")
	
	
	m_pAAMode->AddItem("#HDTF_Options_AA_0", NULL);
	m_iAAQuantity = 0;
	m_pAAQuality[m_iAAQuantity] = { 1, 0 };
	m_iAAQuantity++;

	if (materials->SupportsMSAAMode(2))
	{
		m_pAAMode->AddItem("#HDTF_Options_AA_1", NULL);
		m_pAAQuality[m_iAAQuantity] = { 2, 0 };
		m_iAAQuantity++;
	}

	if (materials->SupportsMSAAMode(4))
	{
		m_pAAMode->AddItem("#HDTF_Options_AA_2", NULL);
		m_pAAQuality[m_iAAQuantity] = { 4, 0 };
		m_iAAQuantity++;
	}

	if (materials->SupportsMSAAMode(6))
	{
		m_pAAMode->AddItem("#HDTF_Options_AA_3", NULL);
		m_pAAQuality[m_iAAQuantity] = { 6, 0 };
		m_iAAQuantity++;
	}

	if (materials->SupportsMSAAMode(8))
	{
		m_pAAMode->AddItem("#HDTF_Options_AA_4", NULL);
		m_pAAQuality[m_iAAQuantity] = { 8, 0 };
		m_iAAQuantity++;
	}

	if (materials->SupportsCSAAMode(4, 2))
	{
		m_pAAMode->AddItem("#HDTF_Options_AA_4_CSAA", NULL);
		m_pAAQuality[m_iAAQuantity] = { 4, 2 };
		m_iAAQuantity++;
	}

	if (materials->SupportsCSAAMode(4, 4))
	{
		m_pAAMode->AddItem("#HDTF_Options_AA_5_CSAA", NULL);
		m_pAAQuality[m_iAAQuantity] = { 4, 4 };
		m_iAAQuantity++;
	}

	if (materials->SupportsCSAAMode(8, 2))
	{
		m_pAAMode->AddItem("#HDTF_Options_AA_6_CSAA", NULL);
		m_pAAQuality[m_iAAQuantity] = { 8, 2 };
		m_iAAQuantity++;
	}

	// at this point I thought that macroses for that was a bad idea...
	ADD_QUALITY_LEVELS6(
		m_pFilteringMode, 
		"#HDTF_Options_TF_0", 
		"#HDTF_Options_TF_1", 
		"#HDTF_Options_TF_2", 
		"#HDTF_Options_TF_3", 
		"#HDTF_Options_TF_4",
		"#HDTF_Options_TF_5")

	ADD_QUALITY_LEVELS2(m_pVSync, "#HDTF_Options_Disabled", "#HDTF_Options_Enabled")
	ADD_QUALITY_LEVELS2(m_pMotionBlur, "#HDTF_Options_Disabled", "#HDTF_Options_Enabled")
	ADD_QUALITY_LEVELS2(m_pMCRendering, "#HDTF_Options_Disabled", "#HDTF_Options_Enabled")

	ConVarRef mat_dxlevel("mat_dxlevel", true);

	ADD_QUALITY_LEVELS2(m_pToneMapping, "#HDTF_Options_Disabled", "#HDTF_Options_Enabled")

	ADD_QUALITY_LEVELS2(m_pBloom, "#HDTF_Options_Disabled", "#HDTF_Options_Enabled")

	SetActiveValues();

	m_pApplyButton->SetEnabled(false);
}

void MainMenu_Options_Video::OnSwitchedTo()
{
	SetActiveValues();
}

bool MainMenu_Options_Video::CanLeave()
{
	if (m_pApplyButton->IsEnabled())
	{
		HDTFUI_Popup* popup = new HDTFUI_Popup(
			this,
			"video_options_not_saved",
			"#HDTF_SettingsNotSaved",
			POPUP_PROMPT,
			"#HDTF_SettingsLeave",
			"#HDTF_SettingsStay",
			"ConfirmLeave");

		popup->AddActionSignalTarget(this);
		popup->TakeControl();

		return false;
	}

	return true;
}

void MainMenu_Options_Video::SetActiveValues()
{
	const MaterialSystem_Config_t &current = materials->GetCurrentConfigForVideoCard();

	ConVarRef mat_dxlevel("mat_dxlevel", true);
	ConVarRef r_rootlod("r_rootlod");
	ConVarRef mat_picmip("mat_picmip");
	ConVarRef mat_trilinear("mat_trilinear");
	ConVarRef mat_forceaniso("mat_forceaniso");
	ConVarRef mat_antialias("mat_antialias");
	ConVarRef mat_aaquality("mat_aaquality");
	ConVarRef mat_vsync("mat_vsync");
	ConVarRef r_flashlightdepthtexture("r_flashlightdepthtexture");
	ConVarRef r_waterforceexpensive("r_waterforceexpensive");
	ConVarRef r_waterforcereflectentities("r_waterforcereflectentities");
	ConVarRef mat_reducefillrate("mat_reducefillrate");
	ConVarRef mat_dynamic_tonemapping("mat_dynamic_tonemapping");
	ConVarRef mat_colorcorrection("mat_colorcorrection");
	ConVarRef mat_motion_blur_enabled("mat_motion_blur_enabled");
	ConVarRef r_shadowrendertotexture("r_shadowrendertotexture");
	ConVarRef mat_queue_mode("mat_queue_mode");
	ConVarRef mat_disable_bloom("mat_disable_bloom");
	ConVarRef mat_monitorgamma("mat_monitorgamma");

	m_pModelQuality->ActivateItem(2 - clamp(r_rootlod.GetInt(), 0, 2));
	m_pTextureQuality->ActivateItem(2 - clamp(mat_picmip.GetInt(), -1, 2));
	m_pShaderQuality->ActivateItem(mat_reducefillrate.GetBool() ? 0 : 1);

	if (r_waterforceexpensive.GetBool())
	{
		if (r_waterforcereflectentities.GetBool())
			m_pReflectionsQuality->ActivateItem(2);
		else
			m_pReflectionsQuality->ActivateItem(1);
	}
	else
	{
		m_pReflectionsQuality->ActivateItem(0);
	}

	if (r_flashlightdepthtexture.GetBool())
	{
		r_shadowrendertotexture.SetValue(1);
		m_pShadowQuality->ActivateItem(2);
	}
	else if (r_shadowrendertotexture.GetBool())
	{
		m_pShadowQuality->ActivateItem(1);
	}
	else
	{
		m_pShadowQuality->ActivateItem(0);
	}

	m_pColorCorrect->ActivateItem(mat_colorcorrection.GetInt());

	int AASamples = mat_antialias.GetInt();
	int AAQuality = mat_aaquality.GetInt();
	for (int i = 0; i < m_iAAQuantity; i++)
	{
		if (AASamples == m_pAAQuality[i].samples && AAQuality == m_pAAQuality[i].quality)
		{
			m_pAAMode->ActivateItem(i);
			break;
		}
	}

	switch (mat_forceaniso.GetInt())
	{
	case 2:
		m_pFilteringMode->ActivateItem(2);
		break;
	case 4:
		m_pFilteringMode->ActivateItem(3);
		break;
	case 8:
		m_pFilteringMode->ActivateItem(4);
		break;
	case 16:
		m_pFilteringMode->ActivateItem(5);
		break;

	case 0:
	default:
		if (mat_trilinear.GetBool())
			m_pFilteringMode->ActivateItem(1);
		else
			m_pFilteringMode->ActivateItem(0);

		break;
	}

	m_pVSync->ActivateItem(mat_vsync.GetInt());
	m_pMotionBlur->ActivateItem(mat_motion_blur_enabled.GetInt());
	m_pMCRendering->ActivateItem((mat_queue_mode.GetInt() == 2) ? 1 : 0);
	m_pToneMapping->ActivateItem(clamp(mat_dynamic_tonemapping.GetInt(), 0, 1));
	m_pBloom->ActivateItem((mat_disable_bloom.GetInt() == 1) ? 0 : 1);

	// find currently active preset; leave it to custom if not macthing any
	m_pPreset->ActivateItem(0);
	for (int i = 0; i < 4; i++)
	{
		int hits = 0;
		for (int j = 0; j < 11; j++)
		{
			preset_t pair = g_GraphicsPresets[i][j];
			switch (pair.param)
			{
			case SETTING_MODELS:
				if (m_pModelQuality->GetActiveItem() == pair.qualityLevel)
					hits++;
				break;

			case SETTING_TEXTURES:
				if (m_pTextureQuality->GetActiveItem() == pair.qualityLevel)
					hits++;
				break;

			case SETTING_SHADERS:
				if (m_pShaderQuality->GetActiveItem() == pair.qualityLevel)
					hits++;
				break;

			case SETTING_REFLECTIONS:
				if (m_pReflectionsQuality->GetActiveItem() == pair.qualityLevel)
					hits++;
				break;

			case SETTING_SHADOWS:
				if (m_pShadowQuality->GetActiveItem() == pair.qualityLevel)
					hits++;
				break;

			case SETTING_COLORCORRECTION:
				if (m_pColorCorrect->GetActiveItem() == pair.qualityLevel)
					hits++;
				break;

			case SETTING_ANTIALIASING:
				if (m_pAAMode->GetActiveItem() == pair.qualityLevel)
					hits++;
				break;

			case SETTING_FILTERING:
				if (m_pFilteringMode->GetActiveItem() == pair.qualityLevel)
					hits++;
				break;

			case SETTING_MOTIONBLUR:
				if (m_pMotionBlur->GetActiveItem() == pair.qualityLevel)
					hits++;
				break;

			case SETTING_TONEMAPPING:
				if (m_pToneMapping->GetActiveItem() == pair.qualityLevel)
					hits++;
				break;

			case SETTING_BLOOM:
				if (m_pBloom->GetActiveItem() == pair.qualityLevel)
					hits++;
				break;

			default:
				break;
			}
		}

		if (hits >= 10)
		{
			m_pPreset->ActivateItem(i + 1);
			break;
		}
	}
	m_iPreset = m_pPreset->GetActiveItem();

	m_pGamma->SetValue(mat_monitorgamma.GetFloat());

	if (current.Windowed())
	{
		m_pGamma->SetVisible(false);
		m_pDisplayMode->ActivateItem(1);
	}
	else
		m_pDisplayMode->ActivateItem(0);


	m_pApplyButton->SetEnabled(false);

	// HACKHACK(wheatley): a horrible hack to prevent apply button being immediately activated upon
	// setting default values to the fields
	m_flIgnoreMessagesUntil = gpGlobals->realtime + gpGlobals->absoluteframetime * 3;
}

void MainMenu_Options_Video::ApplyPreset(int preset)
{
	for (int i = 0; i < 11; i++)
	{
		preset_t pair = g_GraphicsPresets[preset][i];
		switch (pair.param)
		{
		case SETTING_MODELS:
			m_pModelQuality->ActivateItem(pair.qualityLevel);
			break;

		case SETTING_TEXTURES:
			m_pTextureQuality->ActivateItem(pair.qualityLevel);
			break;

		case SETTING_SHADERS:
			m_pShaderQuality->ActivateItem(pair.qualityLevel);
			break;

		case SETTING_REFLECTIONS:
			m_pReflectionsQuality->ActivateItem(pair.qualityLevel);
			break;

		case SETTING_SHADOWS:
			m_pShadowQuality->ActivateItem(pair.qualityLevel);
			break;

		case SETTING_COLORCORRECTION:
			m_pColorCorrect->ActivateItem(pair.qualityLevel);
			break;

		case SETTING_ANTIALIASING:
			m_pAAMode->ActivateItem(min(m_iAAQuantity - 1, pair.qualityLevel));
			break;

		case SETTING_FILTERING:
			m_pFilteringMode->ActivateItem(pair.qualityLevel);
			break;

		case SETTING_MOTIONBLUR:
			m_pMotionBlur->ActivateItem(pair.qualityLevel);
			break;

		case SETTING_TONEMAPPING:
			m_pToneMapping->ActivateItem(pair.qualityLevel);
			break;

		case SETTING_BLOOM:
			m_pBloom->ActivateItem(pair.qualityLevel);
			break;

		default:
			break;
		}
	}
}

void MainMenu_Options_Video::ApplyChanges()
{
	KeyValues *kv = m_pResolution->GetActiveItemUserData();
	char cmd[64];
	Q_snprintf(cmd, sizeof(cmd), 
		"mat_setvideomode %i %i %i\n",
		kv->GetInt("width"),
		kv->GetInt("height"),
		(m_pDisplayMode->GetActiveItem() != 0) ? 1 : 0);
	engine->ClientCmd_Unrestricted(cmd);

	m_pGamma->SetVisible(m_pDisplayMode->GetActiveItem() == 0);

	UpdateConVar("r_rootlod", 2 - m_pModelQuality->GetActiveItem());
	UpdateConVar("mat_picmip", 2 - m_pTextureQuality->GetActiveItem());
	UpdateConVar("mat_reducefillrate", (m_pShaderQuality->GetActiveItem() != 0) ? 0 : 1);

	switch (m_pReflectionsQuality->GetActiveItem())
	{
	case 0:
	default:
		UpdateConVar("r_waterforceexpensive", 0);
		UpdateConVar("r_waterforcereflectentities", 0);
		break;

	case 1:
		UpdateConVar("r_waterforceexpensive", 1);
		UpdateConVar("r_waterforcereflectentities", 0);
		break;

	case 2:
		UpdateConVar("r_waterforceexpensive", 1);
		UpdateConVar("r_waterforcereflectentities", 1);
		break;
	}

	switch (m_pShadowQuality->GetActiveItem())
	{
	case 0:
	default:
		UpdateConVar("r_shadowrendertotexture", 0);
		UpdateConVar("r_flashlightdepthtexture", 0);
		break;

	case 1:
		UpdateConVar("r_shadowrendertotexture", 1);
		UpdateConVar("r_flashlightdepthtexture", 0);
		break;

	case 2:
		UpdateConVar("r_shadowrendertotexture", 1);
		UpdateConVar("r_flashlightdepthtexture", 1);
		break;
	}

	UpdateConVar("mat_colorcorrection", (m_pColorCorrect->GetActiveItem() != 0) ? 1 : 0);

	const int AASelected = m_pAAMode->GetActiveItem();
	UpdateConVar("mat_antialias", m_pAAQuality[AASelected].samples);
	UpdateConVar("mat_aaquality", m_pAAQuality[AASelected].quality);

	UpdateConVar("mat_trilinear", false);
	UpdateConVar("mat_forceaniso", 1);
	switch (m_pFilteringMode->GetActiveItem())
	{
	case 0:
		break;
	case 1:
		UpdateConVar("mat_trilinear", true);
		break;
	case 2:
		UpdateConVar("mat_forceaniso", 2);
		break;
	case 3:
		UpdateConVar("mat_forceaniso", 4);
		break;
	case 4:
		UpdateConVar("mat_forceaniso", 8);
		break;
	case 5:
		UpdateConVar("mat_forceaniso", 16);
		break;
	}

	UpdateConVar("mat_vsync", (m_pVSync->GetActiveItem() != 0) ? 1 : 0);
	UpdateConVar("mat_motion_blur_enabled", (m_pMotionBlur->GetActiveItem() != 0) ? 1 : 0);
	UpdateConVar("mat_queue_mode", (m_pMCRendering->GetActiveItem() != 0) ? 2 : 0);

	UpdateConVar("mat_dynamic_tonemapping", m_pToneMapping->GetActiveItem());

	UpdateConVar("mat_disable_bloom", (m_pBloom->GetActiveItem() != 0) ? 0 : 1);

	UpdateConVar("mat_monitorgamma", m_pGamma->GetValue());

	engine->ClientCmd_Unrestricted("mat_savechanges\n");

	engine->ClientCmd_Unrestricted("host_writeconfig");
}

void MainMenu_Options_Video::ApplyRecommended()
{
	KeyValues *pKeyValues = new KeyValues("config");
	materials->GetRecommendedConfigurationInfo(0, pKeyValues);

	int r_rootlod = pKeyValues->GetInt("ConVar.r_rootlod", 0);
	int mat_picmip = pKeyValues->GetInt("ConVar.mat_picmip", 0);
	int mat_reducefillrate = pKeyValues->GetInt("ConVar.mat_reducefillrate", 0);
	int r_waterforceexpensive = pKeyValues->GetInt("ConVar.r_waterforceexpensive", 0);
	int r_waterforcereflectentities = pKeyValues->GetInt("ConVar.r_waterforcereflectentities", 0);
	int r_shadowrendertotexture = pKeyValues->GetInt("ConVar.r_shadowrendertotexture", 0);
	int r_flashlightdepthtexture = pKeyValues->GetInt("ConVar.r_flashlightdepthtexture", 0);
	int mat_colorcorrection = pKeyValues->GetInt("ConVar.mat_colorcorrection", 0);
	int mat_forceaniso = pKeyValues->GetInt("ConVar.mat_forceaniso", 1);
	int mat_trilinear = pKeyValues->GetInt("ConVar.mat_trilinear", 0);
	int mat_antialias = pKeyValues->GetInt("ConVar.mat_antialias", 0);
	int mat_aaquality = pKeyValues->GetInt("ConVar.mat_aaquality", 0);
	int mat_vsync = pKeyValues->GetInt("ConVar.mat_vsync", 1);
	int mat_dynamic_tonemapping = pKeyValues->GetInt("ConVar.mat_dynamic_tonemapping", 1);
	int mat_motion_blur_enabled = pKeyValues->GetInt("ConVar.mat_motion_blur_enabled", 0);
	int mat_queue_mode = pKeyValues->GetInt("ConVar.mat_queue_mode", 0);
	int mat_disable_bloom = pKeyValues->GetInt("ConVar.mat_disable_bloom", 0);

	m_pModelQuality->ActivateItem(2 - clamp(r_rootlod, 0, 2));
	m_pTextureQuality->ActivateItem(2 - clamp(mat_picmip, -1, 2));
	m_pShaderQuality->ActivateItem(mat_reducefillrate ? 0 : 1);

	if (r_waterforceexpensive)
	{
		if (r_waterforcereflectentities)
			m_pReflectionsQuality->ActivateItem(2);
		else
			m_pReflectionsQuality->ActivateItem(1);
	}
	else
	{
		m_pReflectionsQuality->ActivateItem(0);
	}

	if (r_flashlightdepthtexture)
	{
		m_pShadowQuality->ActivateItem(2);
	}
	else if (r_shadowrendertotexture)
	{
		m_pShadowQuality->ActivateItem(1);
	}
	else
	{
		m_pShadowQuality->ActivateItem(0);
	}

	m_pColorCorrect->ActivateItem(mat_colorcorrection);

	int AASamples = mat_antialias;
	int AAQuality = mat_aaquality;
	for (int i = 0; i < m_iAAQuantity; i++)
	{
		if (AASamples == m_pAAQuality[i].samples && AAQuality == m_pAAQuality[i].quality)
		{
			m_pAAMode->ActivateItem(i);
			break;
		}
	}

	switch (mat_forceaniso)
	{
	case 2:
		m_pFilteringMode->ActivateItem(2);
		break;
	case 4:
		m_pFilteringMode->ActivateItem(3);
		break;
	case 8:
		m_pFilteringMode->ActivateItem(4);
		break;
	case 16:
		m_pFilteringMode->ActivateItem(5);
		break;

	case 0:
	default:
		if (mat_trilinear)
			m_pFilteringMode->ActivateItem(1);
		else
			m_pFilteringMode->ActivateItem(0);

		break;
	}

	m_pVSync->ActivateItem(mat_vsync);
	m_pMotionBlur->ActivateItem(mat_motion_blur_enabled);
	m_pMCRendering->ActivateItem((mat_queue_mode == 2) ? 1 : 0);
	m_pToneMapping->ActivateItem(clamp(mat_dynamic_tonemapping, 0, 1));
	m_pBloom->ActivateItem((mat_disable_bloom == 1) ? 0 : 1);

	// find currently active preset; leave it to custom if not macthing any
	m_pPreset->ActivateItem(0);
	for (int i = 0; i < 4; i++)
	{
		int hits = 0;
		for (int j = 0; j < 11; j++)
		{
			preset_t pair = g_GraphicsPresets[i][j];
			switch (pair.param)
			{
			case SETTING_MODELS:
				if (m_pModelQuality->GetActiveItem() == pair.qualityLevel)
					hits++;
				break;

			case SETTING_TEXTURES:
				if (m_pTextureQuality->GetActiveItem() == pair.qualityLevel)
					hits++;
				break;

			case SETTING_SHADERS:
				if (m_pShaderQuality->GetActiveItem() == pair.qualityLevel)
					hits++;
				break;

			case SETTING_REFLECTIONS:
				if (m_pReflectionsQuality->GetActiveItem() == pair.qualityLevel)
					hits++;
				break;

			case SETTING_SHADOWS:
				if (m_pShadowQuality->GetActiveItem() == pair.qualityLevel)
					hits++;
				break;

			case SETTING_COLORCORRECTION:
				if (m_pColorCorrect->GetActiveItem() == pair.qualityLevel)
					hits++;
				break;

			case SETTING_ANTIALIASING:
				if (m_pAAMode->GetActiveItem() == pair.qualityLevel)
					hits++;
				break;

			case SETTING_FILTERING:
				if (m_pFilteringMode->GetActiveItem() == pair.qualityLevel)
					hits++;
				break;

			case SETTING_MOTIONBLUR:
				if (m_pMotionBlur->GetActiveItem() == pair.qualityLevel)
					hits++;
				break;

			case SETTING_TONEMAPPING:
				if (m_pToneMapping->GetActiveItem() == pair.qualityLevel)
					hits++;
				break;

			case SETTING_BLOOM:
				if (m_pBloom->GetActiveItem() == pair.qualityLevel)
					hits++;
				break;

			default:
				break;
			}
		}

		if (hits >= 10)
		{
			m_pPreset->ActivateItem(i + 1);
			break;
		}
	}

	m_pApplyButton->SetEnabled(true);
}

int MainMenu_Options_Video::FindNearestRatio(vmode_t *mode)
{
	float ratio = (float)mode->width / (float)mode->height;
	float minDist = 9999.f;
	int type = 0;

	for (int i = 0; i < ARRAYSIZE(g_SupportedRatios); i++)
	{
		const float dist = fabs(g_SupportedRatios[i].ratio - ratio);

		if (dist < minDist)
		{
			minDist = dist;
			type = g_SupportedRatios[i].type;
		}
	}

	return type;
}

void MainMenu_Options_Video::FormatResolutionName(vmode_t *mode, char *buffer, const int size)
{
	char *ratio = "4:3";

	switch (FindNearestRatio(mode))
	{
	case 1:
		ratio = "16:9";
		break;

	case 2:
	case 3:
		ratio = "16:10";

	default:
		break;
	}

	char *formated = VarArgs("%ix%i [%s]", mode->width, mode->height, ratio);
	const int len = V_strlen(formated);

	V_memset(buffer, 0, size);
	V_memcpy(buffer, formated, min(size, len));
}

void MainMenu_Options_Video::FillResolutionList()
{
	m_pResolution->DeleteAllItems();

	vmode_t *list = NULL;
	int count = 0;

	gameuifuncs->GetVideoModes(&list, &count);
	const MaterialSystem_Config_t &current = materials->GetCurrentConfigForVideoCard();

	CUtlVector<resolutiondescriptor_t *> resolutions;

	for (int i = 0; i < count; i++)
	{
		char name[64];
		FormatResolutionName(list, name, sizeof(name));

		int id = m_pResolution->AddItem(name, new KeyValues("resolution", "width", list->width, "height", list->height));

		if (current.m_VideoMode.m_Width == list->width && current.m_VideoMode.m_Height == list->height)
		{
			m_pResolution->ActivateItem(id);
		}

		list++;
	}
}

#pragma endregion

#pragma region Audio options tab

char *MainMenu_Options_Audio::m_pNewLanguage = (char*)GetLanguageShortName(k_Lang_English);

MainMenu_Options_Audio::MainMenu_Options_Audio(vgui::Panel *parent) : HDTFMenuCanvas(parent, "options_AudioTab")
{
	m_pApplyButton = new Button(this, "backButton", "#HDTF_Options_Apply");
	m_pApplyButton->SetCommand("apply");
	m_pApplyButton->AddActionSignalTarget(this);
	m_pApplyButton->SetVisible(true);
	m_pApplyButton->SetZPos(99);

	m_pSFXVolume = new HDTFUI_LabeledCvarSlider(this, "options_Audio_SFXSlider", "#HDTF_Audio_SFX", "volume");
	m_pCutsceneVolume = new HDTFUI_LabeledCvarSlider(this, "options_Audio_CutsceneSlider", "#HDTF_Audio_Cutscene", "snd_cutscene_volume");
	m_pMusicVolume = new HDTFUI_LabeledCvarSlider(this, "options_Audio_MusicSlider", "#HDTF_Audio_Music", "snd_musicvolume");

	m_pSFXVolume->SetTickCaptions("#HDTF_Audio_Low", "#HDTF_Audio_High");
	m_pCutsceneVolume->SetTickCaptions("#HDTF_Audio_Low", "#HDTF_Audio_High");
	m_pMusicVolume->SetTickCaptions("#HDTF_Audio_Low", "#HDTF_Audio_High");

	m_pSpeakerConfig = new HDTFUI_LabeledComboBox(this, "options_Audio_Config", "#HDTF_Audio_Config", 5);
	m_pSoundQuality = new HDTFUI_LabeledComboBox(this, "options_Audio_Quality", "#HDTF_Audio_Quality", 3);
	m_pLanguage = new HDTFUI_LabeledComboBox(this, "options_Audio_Language", "#HDTF_Audio_Language", 6);
	m_pCaptioning = new HDTFUI_LabeledComboBox(this, "options_Audio_Captioning", "#HDTF_Audio_Captioning", 3);

	m_pSFXVolume->AddActionSignalTarget(this);
	m_pCutsceneVolume->AddActionSignalTarget(this);
	m_pMusicVolume->AddActionSignalTarget(this);

	m_pSpeakerConfig->AddActionSignalTarget(this);
	m_pSoundQuality->AddActionSignalTarget(this);
	m_pLanguage->AddActionSignalTarget(this);
	m_pCaptioning->AddActionSignalTarget(this);

	m_pBlackOut = new HDTF_Blackout(this, "blackout");
	m_pBlackOut->SetVisible(false);

	m_p3rdParty = new HDTFMenuButton(this, "3rdPartyButton", "#HDTF_3rdPartyNotes_Button");
	m_p3rdParty->SetCommand("ShowLegalNotes");

	m_flIgnoreMessagesUntil = 0.f;

	SetupElements();
}

void MainMenu_Options_Audio::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	m_pApplyButton->SetFont(pScheme->GetFont("MainMenuButtons", false));
}

void MainMenu_Options_Audio::OnSelectionChanged(KeyValues *data)
{
	// HACKHACK(wheatley): ignore messages from the frame where we initialized default values for the fields
	if (m_flIgnoreMessagesUntil >= gpGlobals->realtime)
	{
		return;
	}

	m_pApplyButton->SetEnabled(true);
}

void MainMenu_Options_Audio::OnMenuPopulated(KeyValues *data)
{
	m_pBlackOut->SetVisible(true);
}

void MainMenu_Options_Audio::OnMenuHidden(KeyValues *data)
{
	m_pBlackOut->SetVisible(false);
}

void MainMenu_Options_Audio::ArrangeItem(Panel *item, int xpos, int ypos)
{
	int wide = (GetWide() / 1.85f) - scheme()->GetProportionalScaledValue(45);

	item->SetSize(wide, CONTROL_HEIGHT);
	item->SetPos(xpos, ypos + m_iItemOffset);

	m_iItemOffset += CONTROL_HEIGHT + CONTROL_SPACING;
}

void MainMenu_Options_Audio::PerformLayout()
{
	m_iItemOffset = 0;

	int wide, tall;
	surface()->GetScreenSize(wide, tall);
	SetSize(wide, tall);
	m_pBlackOut->SetSize(wide, tall);

	int spacing = scheme()->GetProportionalScaledValue(10);

	int xpos = scheme()->GetProportionalScaledValue(45);
	int ypos = (tall / 8) + spacing;

	ArrangeItem(m_pSFXVolume, xpos, ypos);
	ArrangeItem(m_pCutsceneVolume, xpos, ypos);
	ArrangeItem(m_pMusicVolume, xpos, ypos);

	INSERT_SPACER();

	ArrangeItem(m_pSpeakerConfig, xpos, ypos);
	ArrangeItem(m_pSoundQuality, xpos, ypos);
	ArrangeItem(m_pLanguage, xpos, ypos);
	ArrangeItem(m_pCaptioning, xpos, ypos);

	INSERT_SPACER();

	ArrangeItem(m_p3rdParty, xpos, ypos);

	m_pApplyButton->SetPos(scheme()->GetProportionalScaledValue(180), tall - (tall / 9));
	m_pApplyButton->SetWide(scheme()->GetProportionalScaledValue(100));
	m_pApplyButton->SetFgColor(Color(255, 255, 255, 255));
	m_pApplyButton->SetSelectedColor(Color(100, 5, 0, 255), Color(0, 0, 0, 0));
	m_pApplyButton->SetArmedColor(Color(255, 25, 0, 255), Color(0, 0, 0, 0));
	m_pApplyButton->SetPaintBorderEnabled(false);
}

void MainMenu_Options_Audio::SetupElements()
{
	m_pSpeakerConfig->AddItem("#HDTF_Audio_Headphones", new KeyValues("config", "config", 0));
	m_pSpeakerConfig->AddItem("#HDTF_Audio_2Speakers", new KeyValues("config", "config", 2));
	m_pSpeakerConfig->AddItem("#HDTF_Audio_4Speakers", new KeyValues("config", "config", 4));
	m_pSpeakerConfig->AddItem("#HDTF_Audio_5Speakers", new KeyValues("config", "config", 5));
	m_pSpeakerConfig->AddItem("#HDTF_Audio_7Speakers", new KeyValues("config", "config", 7));

	m_pSoundQuality->AddItem("#HDTF_Options_Low", new KeyValues("quality", "quality", 0));
	m_pSoundQuality->AddItem("#HDTF_Options_Medium", new KeyValues("quality", "quality", 1));
	m_pSoundQuality->AddItem("#HDTF_Options_High", new KeyValues("quality", "quality", 2));

	char currentLang[64];
	char availableLang[512];
	availableLang[0] = NULL;

	engine->GetUILanguage(currentLang, sizeof(currentLang));

#if !defined( NO_STEAM )
	if (steamapicontext->SteamApps())
	{
		V_snprintf(currentLang, sizeof(currentLang), steamapicontext->SteamApps()->GetCurrentGameLanguage());
		V_snprintf(availableLang, sizeof(availableLang), steamapicontext->SteamApps()->GetAvailableGameLanguages());
	}
#endif

	m_eActiveLanguage = PchLanguageToELanguage(currentLang);

	if (V_strlen(availableLang))
	{
		CUtlVector<char*> availableLangList;
		V_SplitString(availableLang, ",", availableLangList);

		for (int i = 0; i < availableLangList.Count(); i++)
		{
			const ELanguage languageCode = PchLanguageToELanguage(availableLangList[i]);
			m_pLanguage->AddItem(GetLanguageVGUILocalization(languageCode), new KeyValues("lang", "lang", languageCode));
		}
	}
	else
	{
		m_pLanguage->AddItem(GetLanguageVGUILocalization(m_eActiveLanguage), new KeyValues("lang", "lang", m_eActiveLanguage));
	}

	for (int i = 0; i < m_pLanguage->GetItemCount(); i++)
	{
		KeyValues *kv = m_pLanguage->GetItemUserData(i);
		if (kv && kv->GetInt("lang") == m_eActiveLanguage)
		{
			m_pLanguage->ActivateItem(i);
			break;
		}
	}

	ADD_QUALITY_LEVELS3(m_pCaptioning, "#HDTF_Audio_NoCaptions", "#HDTF_Audio_Full", "#HDTF_Audio_Subtitles")

	SetActiveValues();
}

void MainMenu_Options_Audio::SetActiveValues()
{
	ConVarRef volume("volume");
	ConVarRef snd_cutscene_volume("snd_cutscene_volume");
	ConVarRef snd_musicvolume("snd_musicvolume");
	ConVarRef snd_surround_speakers("snd_surround_speakers");
	ConVarRef snd_pitchquality("snd_pitchquality");
	ConVarRef dsp_slow_cpu("dsp_slow_cpu");
	ConVarRef closecaption("closecaption");
	ConVarRef cc_subtitles("cc_subtitles");

	m_pSFXVolume->SetValue(volume.GetFloat());
	m_pCutsceneVolume->SetValue(snd_cutscene_volume.GetFloat());
	m_pMusicVolume->SetValue(snd_musicvolume.GetFloat());

	int config = snd_surround_speakers.GetInt();

	m_pSpeakerConfig->ActivateItem(1);

	for (int i = 0; i < m_pSpeakerConfig->GetItemCount(); i++)
	{
		KeyValues *kv = m_pSpeakerConfig->GetItemUserData(i);

		if (kv && kv->GetInt("config") == config)
		{
			m_pSpeakerConfig->ActivateItem(i);
			break;
		}
	}

	int quality = 0;
	if (dsp_slow_cpu.GetBool() == false)
	{
		quality = 1;
	}
	if (snd_pitchquality.GetBool())
	{
		quality = 2;
	}

	for (int i = 0; i < m_pSoundQuality->GetItemCount(); i++)
	{
		KeyValues *kv = m_pSoundQuality->GetItemUserData(i);
		if (kv && kv->GetInt("quality") == quality)
		{
			m_pSoundQuality->ActivateItem(i);
			break;
		}
	}

	if (closecaption.GetBool())
	{
		if (cc_subtitles.GetBool())
			m_pCaptioning->ActivateItem(2);
		else
			m_pCaptioning->ActivateItem(1);
	}
	else
	{
		m_pCaptioning->ActivateItem(0);
	}

	m_pApplyButton->SetEnabled(false);

	// HACKHACK(wheatley): a horrible hack to prevent apply button being immediately activated upon
	// setting default values to the fields
	m_flIgnoreMessagesUntil = gpGlobals->realtime + gpGlobals->absoluteframetime * 3;
}

void MainMenu_Options_Audio::ApplyChanges()
{
	ConVarRef volume("volume");
	ConVarRef snd_cutscene_volume("snd_cutscene_volume");
	ConVarRef snd_musicvolume("snd_musicvolume");
	ConVarRef snd_surround_speakers("snd_surround_speakers");
	ConVarRef snd_pitchquality("snd_pitchquality");
	ConVarRef dsp_slow_cpu("dsp_slow_cpu");
	ConVarRef dsp_enhance_stereo("dsp_enhance_stereo");
	ConVarRef cc_subtitles("cc_subtitles");

	volume.SetValue(m_pSFXVolume->GetValue());
	snd_cutscene_volume.SetValue(m_pCutsceneVolume->GetValue());
	snd_musicvolume.SetValue(m_pMusicVolume->GetValue());

	int config = m_pSpeakerConfig->GetActiveItemUserData()->GetInt("config");
	snd_surround_speakers.SetValue(config);

	int quality = m_pSoundQuality->GetActiveItemUserData()->GetInt("quality");
	switch (quality)
	{
	case 0:
		dsp_slow_cpu.SetValue(true);
		snd_pitchquality.SetValue(false);
		break;
	default:
	case 1:
		dsp_slow_cpu.SetValue(false);
		snd_pitchquality.SetValue(false);
		break;
	case 2:
		dsp_slow_cpu.SetValue(false);
		snd_pitchquality.SetValue(true);
		break;
	};

	if (config == 0 && quality == 2)
		dsp_enhance_stereo.SetValue(1);
	else
		dsp_enhance_stereo.SetValue(0);

	int closecaption = 0;

	switch (m_pCaptioning->GetActiveItem())
	{
	default:
	case 0:
		closecaption = 0;
		cc_subtitles.SetValue(0);
		break;
	case 1:
		closecaption = 1;
		cc_subtitles.SetValue(0);
		break;
	case 2:
		closecaption = 1;
		cc_subtitles.SetValue(1);
		break;
	}

	char cmd[64];
	Q_snprintf(cmd, sizeof(cmd), "closecaption %i\n", closecaption);
	engine->ClientCmd_Unrestricted(cmd);

	KeyValues *kv = m_pLanguage->GetActiveItemUserData();
	const ELanguage lang = (ELanguage)(kv ? kv->GetInt("lang") : k_Lang_English);

	if (lang != m_eActiveLanguage && IsPC())
	{
		m_pNewLanguage = (char *)GetLanguageShortName(lang);

		MessageBox *msg = new MessageBox("#HDTF_Options_Restart", "#HDTF_Options_RestartDesc", guiroot);
		if (msg)
		{
			msg->DoModal();
		}
	}

	engine->ClientCmd_Unrestricted("host_writeconfig");
}

void MainMenu_Options_Audio::OnCommand(const char *command)
{
	if (FStrEq(command, "apply"))
	{
		ApplyChanges();
		m_pApplyButton->SetEnabled(false);
	}
	else if (FStrEq(command, "ShowLegalNotes"))
	{
		HDTFUI_Popup *popup = new HDTFUI_3rdPartyPopup(guiroot);
		popup->TakeControl();
	}
	else if (FStrEq(command, "ConfirmLeave"))
	{
		m_pApplyButton->SetEnabled(false);
		BaseClass::OnCommand("menu_back");
	}
	else
	{
		BaseClass::OnCommand(command);
	}
}

void MainMenu_Options_Audio::OnSwitchedTo()
{
	SetActiveValues();
}

bool MainMenu_Options_Audio::CanLeave()
{
	if (m_pApplyButton->IsEnabled())
	{
		HDTFUI_Popup* popup = new HDTFUI_Popup(
			this,
			"audio_options_not_saved",
			"#HDTF_SettingsNotSaved",
			POPUP_PROMPT,
			"#HDTF_SettingsLeave",
			"#HDTF_SettingsStay",
			"ConfirmLeave");

		popup->AddActionSignalTarget(this);
		popup->TakeControl();

		return false;
	}

	return true;
}

#pragma endregion

#pragma region Controls options tab

MainMenu_Options_Controls::MainMenu_Options_Controls(vgui::Panel *parent) : HDTFMenuCanvas(parent, "options_ControlsTab")
{
	m_pApplyButton = new Button(guiroot, "backButton", "#HDTF_Options_Apply");
	m_pApplyButton->SetCommand("apply");
	m_pApplyButton->AddActionSignalTarget(this);
	m_pApplyButton->SetVisible(true);
	m_pApplyButton->SetZPos(99);

	m_pRevertButton = new Button(guiroot, "defaultsButton", "#HDTF_Options_Defaults");
	m_pRevertButton->SetCommand("defaults");
	m_pRevertButton->AddActionSignalTarget(this);
	m_pRevertButton->SetVisible(true);
	m_pRevertButton->SetZPos(99);

	m_pScrollFrame = new Panel(this, "controlsScrollFrame");
	m_pScrollHolder = new Panel(m_pScrollFrame, "controlsScroll");
	
	m_pScrollFrame->SetPaintBackgroundEnabled(true);
	m_pScrollFrame->SetBgColor(Color(255, 0, 0, 100));

	m_pReverseMouse = new HDTFUI_LabeledComboBox(m_pScrollHolder, "options_Controls_MouseRev", "#HDTF_Controls_MouseReverse", 2);
	m_pFilterMouse = new HDTFUI_LabeledComboBox(m_pScrollHolder, "options_Controls_MouseFilter", "#HDTF_Controls_MouseFilter", 2);
	m_pRawInput = new HDTFUI_LabeledComboBox(m_pScrollHolder, "options_Controls_MouseRaw", "#HDTF_Controls_MouseRaw", 2);
	m_pMouseAccelEnabled = new HDTFUI_LabeledComboBox(m_pScrollHolder, "options_Controls_MouseAccel", "#HDTF_Controls_MouseAccel", 2);
	m_pJoystick = new HDTFUI_LabeledComboBox(m_pScrollHolder, "options_Controls_Joystick", "#HDTF_Controls_Joystick", 2);
	m_pJoystickSouthPaw = new HDTFUI_LabeledComboBox(m_pScrollHolder, "options_Controls_JoystickPaw", "#HDTF_Controls_JoystickPaw", 2);
	m_pJoystickReverse = new HDTFUI_LabeledComboBox(m_pScrollHolder, "options_Controls_JoystickRev", "#HDTF_Controls_JoystickRev", 2);

	m_pMouseSensitivity = new HDTFUI_LabeledCvarSlider(m_pScrollHolder, "options_Controls_Sensitivity", "#HDTF_Controls_MSense", "sensitivity", 1.f, 20.f);
	m_pMouseAcceleration = new HDTFUI_LabeledCvarSlider(m_pScrollHolder, "options_Controls_Acceleration", "#HDTF_Controls_MouseAccelPower", "m_customaccel_exponent", 1.f, 1.4f);
	m_pJoystickSensitivityYaw = new HDTFUI_LabeledCvarSlider(m_pScrollHolder, "options_Controls_JoystickSY", "#HDTF_Controls_JoystickSenseY", "joy_yawsensitivity", -0.5f, -7.f);
	m_pJoystickSensitivityPitch = new HDTFUI_LabeledCvarSlider(m_pScrollHolder, "options_Controls_JoystickSP", "#HDTF_Controls_JoystickSenseP", "joy_pitchsensitivity", 0.5f, 7.f);

	m_pBindingsLabel = new HDTFUI_LabeledBindEntry(m_pScrollHolder, NULL, "#HDTF_Controls_KeyBindings");

	m_pMouseSensitivity->AddActionSignalTarget(this);
	m_pMouseAcceleration->AddActionSignalTarget(this);
	m_pJoystickSensitivityYaw->AddActionSignalTarget(this);
	m_pJoystickSensitivityPitch->AddActionSignalTarget(this);

	m_pReverseMouse->AddActionSignalTarget(this);
	m_pFilterMouse->AddActionSignalTarget(this);
	m_pJoystick->AddActionSignalTarget(this);
	m_pJoystickSouthPaw->AddActionSignalTarget(this);
	m_pJoystickReverse->AddActionSignalTarget(this);

	m_pBlackOut = new HDTF_Blackout(this, "blackout");
	m_pBlackOut->SetVisible(false);

	m_iScrollSize = CONTROL_HEIGHT * 3;
	m_flSwitchAllowed = 0.f;

	m_flIgnoreMessagesUntil = 0.f;

	ParseActions();

	SetupElements();
}

bool MainMenu_Options_Controls::CanLeave()
{
	if (g_OptionsBindTarget != NULL || m_flSwitchAllowed != 0.f)
	{
		return false;
	}

	if (m_pApplyButton->IsEnabled())
	{
		HDTFUI_Popup* popup = new HDTFUI_Popup(
			this,
			"controls_options_not_saved",
			"#HDTF_SettingsNotSaved",
			POPUP_PROMPT,
			"#HDTF_SettingsLeave",
			"#HDTF_SettingsStay",
			"ConfirmLeave");

		popup->AddActionSignalTarget(this);
		popup->TakeControl();

		return false;
	}

	return true;
}

void MainMenu_Options_Controls::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	m_pApplyButton->SetFont(pScheme->GetFont("MainMenuButtons", false));
	m_pRevertButton->SetFont(pScheme->GetFont("MainMenuButtons", false));
}

void MainMenu_Options_Controls::OnSelectionChanged(KeyValues *data)
{
	// HACKHACK(wheatley): ignore messages from the frame where we initialized default values for the fields
	if (m_flIgnoreMessagesUntil >= gpGlobals->realtime)
	{
		return;
	}

	m_pApplyButton->SetEnabled(true);
}

void MainMenu_Options_Controls::OnMenuPopulated(KeyValues *data)
{
	m_pBlackOut->SetVisible(true);
}

void MainMenu_Options_Controls::OnMenuHidden(KeyValues *data)
{
	m_pBlackOut->SetVisible(false);
}

void MainMenu_Options_Controls::OnMouseWheeled(int delta)
{
	if (g_OptionsBindTarget != NULL)
	{
		g_OptionsBindTarget->OnMousePressed((delta > 0) ? MOUSE_WHEEL_UP : MOUSE_WHEEL_DOWN);
		return;
	}

	m_iScrollOffset = max(0, min(m_iMaxScroll, m_iScrollOffset - m_iScrollSize * delta));
	m_pScrollHolder->SetPos(0, -m_iScrollOffset);
}

void MainMenu_Options_Controls::ArrangeItem(Panel *item)
{
	item->SetParent(m_pScrollHolder);
	item->SetSize(m_pScrollHolder->GetWide(), CONTROL_HEIGHT);
	item->SetPos(0, m_iItemOffset);

	m_iItemOffset += CONTROL_HEIGHT + CONTROL_SPACING;
	m_pScrollHolder->SetTall(m_iItemOffset);
}

void MainMenu_Options_Controls::PerformLayout()
{
	m_iItemOffset = 0;

	int wide, tall;
	surface()->GetScreenSize(wide, tall);
	// since this is a popup we don't want it to cover the 'back' button
	SetSize(wide, tall - (tall / 8));
	m_pBlackOut->SetSize(wide, tall);

	int spacing = scheme()->GetProportionalScaledValue(10);
	int xpos = scheme()->GetProportionalScaledValue(45);

	m_pScrollFrame->SetPos(xpos, (tall / 8) + spacing);
	m_pScrollFrame->SetSize((wide / 1.85f) - xpos, tall - ((tall / 8) + spacing) * 2);

	m_pScrollHolder->SetWide(m_pScrollFrame->GetWide());

	ArrangeItem(m_pReverseMouse);
	ArrangeItem(m_pFilterMouse);
	ArrangeItem(m_pMouseSensitivity);
	ArrangeItem(m_pRawInput);
	ArrangeItem(m_pMouseAccelEnabled);
	ArrangeItem(m_pMouseAcceleration);

	INSERT_SPACER();

	ArrangeItem(m_pJoystick);
	ArrangeItem(m_pJoystickSouthPaw);
	ArrangeItem(m_pJoystickReverse);
	ArrangeItem(m_pJoystickSensitivityPitch);
	ArrangeItem(m_pJoystickSensitivityYaw);

	INSERT_SPACER();
	ArrangeItem(m_pBindingsLabel);

	for (int i = 0; i < m_pBinders.Count(); i++)
	{
		HDTFUI_LabeledBindEntry *bind = m_pBinders[i];
		if (bind->IsBlank() && i != 0)
		{
			INSERT_SPACER();
		}

		ArrangeItem(bind);
	}

	m_iMaxScroll = m_pScrollHolder->GetTall() - m_pScrollFrame->GetTall();

	m_pApplyButton->SetPos(scheme()->GetProportionalScaledValue(180), tall - (tall / 9));
	m_pApplyButton->SetWide(scheme()->GetProportionalScaledValue(100));
	m_pApplyButton->SetFgColor(Color(255, 255, 255, 255));
	m_pApplyButton->SetSelectedColor(Color(100, 5, 0, 255), Color(0, 0, 0, 0));
	m_pApplyButton->SetArmedColor(Color(255, 25, 0, 255), Color(0, 0, 0, 0));
	m_pApplyButton->SetPaintBorderEnabled(false);

	m_pRevertButton->SetPos(scheme()->GetProportionalScaledValue(280), tall - (tall / 9));
	m_pRevertButton->SetWide(scheme()->GetProportionalScaledValue(100));
	m_pRevertButton->SetFgColor(Color(255, 255, 255, 255));
	m_pRevertButton->SetSelectedColor(Color(100, 5, 0, 255), Color(0, 0, 0, 0));
	m_pRevertButton->SetArmedColor(Color(255, 25, 0, 255), Color(0, 0, 0, 0));
	m_pRevertButton->SetPaintBorderEnabled(false);
}

void MainMenu_Options_Controls::SetupElements()
{
	ADD_QUALITY_LEVELS2(m_pReverseMouse, "#HDTF_Options_Disabled", "#HDTF_Options_Enabled");
	ADD_QUALITY_LEVELS2(m_pFilterMouse, "#HDTF_Options_Disabled", "#HDTF_Options_Enabled");
	ADD_QUALITY_LEVELS2(m_pRawInput, "#HDTF_Options_Disabled", "#HDTF_Options_Enabled");
	ADD_QUALITY_LEVELS2(m_pMouseAccelEnabled, "#HDTF_Options_Disabled", "#HDTF_Options_Enabled");
	ADD_QUALITY_LEVELS2(m_pJoystick, "#HDTF_Options_Disabled", "#HDTF_Options_Enabled");
	ADD_QUALITY_LEVELS2(m_pJoystickSouthPaw, "#HDTF_Options_Disabled", "#HDTF_Options_Enabled");
	ADD_QUALITY_LEVELS2(m_pJoystickReverse, "#HDTF_Options_Disabled", "#HDTF_Options_Enabled");
}

void MainMenu_Options_Controls::SetActiveValues()
{
	ConVarRef m_pitch("m_pitch");
	ConVarRef m_filter("m_filter");
	ConVarRef sensitivity("sensitivity");
	ConVarRef joystick("joystick");
	ConVarRef joy_movement_stick("joy_movement_stick");
	ConVarRef joy_inverty("joy_inverty");
	ConVarRef m_customaccel("m_customaccel");
	ConVarRef m_customaccel_exponent("m_customaccel_exponent");
	ConVarRef m_rawinput("m_rawinput");
	ConVarRef joy_yawsensitivity("joy_yawsensitivity");
	ConVarRef joy_pitchsensitivity("joy_pitchsensitivity");

	m_pReverseMouse->ActivateItem((m_pitch.GetFloat() > 0) ? 0 : 1);
	m_pFilterMouse->ActivateItem((m_filter.GetInt() != 0) ? 1 : 0);
	m_pRawInput->ActivateItem((m_rawinput.GetInt() != 0) ? 1 : 0);
	m_pMouseAccelEnabled->ActivateItem((m_customaccel.GetInt() != 0) ? 1 : 0);
	m_pJoystick->ActivateItem((joystick.GetInt() != 0) ? 1 : 0);
	m_pJoystickSouthPaw->ActivateItem((joy_movement_stick.GetInt() != 0) ? 1 : 0);
	m_pJoystickReverse->ActivateItem((joy_inverty.GetInt() != 0) ? 1 : 0);

	m_pMouseSensitivity->SetValue(sensitivity.GetFloat());
	m_pMouseAcceleration->SetValue(m_customaccel_exponent.GetFloat());
	m_pJoystickSensitivityYaw->SetValue(joy_yawsensitivity.GetFloat());
	m_pJoystickSensitivityPitch->SetValue(joy_pitchsensitivity.GetFloat());

	for (int i = 0; i < m_pBinders.Count(); i++)
	{
		m_pBinders[i]->Revert();
		m_pBinders[i]->UpdateBoundKey();
	}

	m_pApplyButton->SetEnabled(false);

	// HACKHACK(wheatley): a horrible hack to prevent apply button being immediately activated upon
	// setting default values to the fields
	m_flIgnoreMessagesUntil = gpGlobals->realtime + gpGlobals->absoluteframetime * 3;
}

void MainMenu_Options_Controls::ApplyChanges()
{
	ConVarRef m_pitch("m_pitch");
	ConVarRef m_filter("m_filter");
	ConVarRef sensitivity("sensitivity");
	ConVarRef joystick("joystick");
	ConVarRef joy_movement_stick("joy_movement_stick");
	ConVarRef joy_inverty("joy_inverty");
	ConVarRef m_customaccel("m_customaccel");
	ConVarRef m_customaccel_exponent("m_customaccel_exponent");
	ConVarRef m_rawinput("m_rawinput");
	ConVarRef joy_yawsensitivity("joy_yawsensitivity");
	ConVarRef joy_pitchsensitivity("joy_pitchsensitivity");

	m_pitch.SetValue((m_pReverseMouse->GetActiveItem() != 0) ? -0.022f : 0.022f);
	m_filter.SetValue((m_pFilterMouse->GetActiveItem() != 0) ? 1 : 0);
	m_rawinput.SetValue((m_pRawInput->GetActiveItem() != 0) ? 1 : 0);
	m_customaccel.SetValue((m_pMouseAccelEnabled->GetActiveItem() != 0) ? 3 : 0);
	joystick.SetValue((m_pJoystick->GetActiveItem() != 0) ? 1 : 0);
	joy_movement_stick.SetValue((m_pJoystickSouthPaw->GetActiveItem() != 0) ? 1 : 0);
	joy_inverty.SetValue((m_pJoystickReverse->GetActiveItem() != 0) ? 1 : 0);

	sensitivity.SetValue(m_pMouseSensitivity->GetValue());
	m_customaccel_exponent.SetValue(m_pMouseAcceleration->GetValue());
	joy_yawsensitivity.SetValue(m_pJoystickSensitivityYaw->GetValue());
	joy_pitchsensitivity.SetValue(m_pJoystickSensitivityPitch->GetValue());

	// do key bind in two passes:
	// pass 1: unbind all previous bound keys
	// pass 2: bind all keys again
	for (int i = 0; i < m_pBinders.Count(); i++)
	{
		m_pBinders[i]->UnbindLast();
	}

	for (int i = 0; i < m_pBinders.Count(); i++)
	{
		m_pBinders[i]->ApplyBind();
	}

	engine->ClientCmd_Unrestricted("host_writeconfig");
}

void MainMenu_Options_Controls::ParseActions()
{
	char szBinding[256];
	char szDescription[256];

	//KeyValues *item;

	// Load the default keys list
	CUtlBuffer buf(0, 0, CUtlBuffer::TEXT_BUFFER);
	if (!filesystem->ReadFile("scripts/kb_act.lst", NULL, buf))
		return;

	const char *data = (const char*)buf.Base();

	//int sectionIndex = 0;
	char token[512];
	while (1)
	{
		data = engine->ParseFile(data, token, sizeof(token));
		// Done.
		if (strlen(token) <= 0)
			break;

		Q_strncpy(szBinding, token, sizeof(szBinding));

		data = engine->ParseFile(data, token, sizeof(token));
		if (strlen(token) <= 0)
		{
			break;
		}

		Q_strncpy(szDescription, token, sizeof(szDescription));

		// Skip '======' rows
		if (szDescription[0] != '=')
		{
			// Flag as special header row if binding is "blank"
			if (!stricmp(szBinding, "blank"))
			{
				HDTFUI_LabeledBindEntry *bind = new HDTFUI_LabeledBindEntry(m_pScrollHolder, NULL, szDescription);
				m_pBinders.AddToTail(bind);
			}
			else
			{
				//// Create a new: blank item
				//item = new KeyValues("Item");

				//// fill in data
				//item->SetString("Action", szDescription);
				//item->SetString("Binding", szBinding);
				//item->SetString("Key", "");

				//// Add to list
				//m_pBinderList->AddItem(sectionIndex, item);
				//item->deleteThis();

				HDTFUI_LabeledBindEntry *bind = new HDTFUI_LabeledBindEntry(m_pScrollHolder, szBinding, szDescription);
				bind->AddActionSignalTarget(this);
				m_pBinders.AddToTail(bind);
			}
		}
	}
}

HDTFUI_LabeledBindEntry	*MainMenu_Options_Controls::GetBinderByBind(const char *bind)
{
	for (int i = 0; i < m_pBinders.Count(); i++)
	{
		if (!strcmp(m_pBinders[i]->GetCommand(), bind))
		{
			return m_pBinders[i];
		}
	}
	return NULL;
}

void MainMenu_Options_Controls::RevertToDefaults()
{
	FileHandle_t fh = g_pFullFileSystem->Open("cfg/config_default.cfg", "rb");
	if (fh == FILESYSTEM_INVALID_HANDLE)
	{
		HDTFUI_Popup *popup = new HDTFUI_Popup(
			this,
			"revert_defaults_error_message",
			"#HDTF_Defaults_Err_Message",
			POPUP_MESSAGE,
			"#HDTF_Menu_OK");

		popup->AddActionSignalTarget(this);
		popup->TakeControl();

		return;
	}

	int size = g_pFullFileSystem->Size(fh);
	CUtlBuffer buf(0, size, CUtlBuffer::TEXT_BUFFER);
	g_pFullFileSystem->Read(buf.Base(), size, fh);
	g_pFullFileSystem->Close(fh);

	// Clear out all current bindings
	for (int i = 0; i < m_pBinders.Count(); i++)
	{
		m_pBinders[i]->ChangeBind(BUTTON_CODE_NONE);
	}

	const char *data = (const char*)buf.Base();

	// loop through all the binding
	while (data != NULL)
	{
		char cmd[64];
		data = engine->ParseFile(data, cmd, sizeof(cmd));
		if (strlen(cmd) <= 0)
			break;

		if (!stricmp(cmd, "bind"))
		{
			// Key name
			char szKeyName[256];
			data = engine->ParseFile(data, szKeyName, sizeof(szKeyName));
			if (strlen(szKeyName) <= 0)
				break; // Error

			char szBinding[256];
			data = engine->ParseFile(data, szBinding, sizeof(szBinding));
			if (strlen(szKeyName) <= 0)
				break; // Error

					   // Find item
			HDTFUI_LabeledBindEntry *item = GetBinderByBind(szBinding);
			if (item)
			{
				// Bind it
				ButtonCode_t code = g_pInputSystem->StringToButtonCode(szKeyName);
				item->ChangeBind(code);
			}
		}
	}

	// Make sure console and escape key are always valid
	HDTFUI_LabeledBindEntry *item = GetBinderByBind("toggleconsole");
	if (item)
	{
		// Bind it
		item->ChangeBind(KEY_BACKQUOTE);
	}
	item = GetBinderByBind("cancelselect");
	if (item)
	{
		// Bind it
		item->ChangeBind(KEY_ESCAPE);
	}

	for (int i = 0; i < m_pBinders.Count(); i++)
	{
		m_pBinders[i]->UpdateBoundKey();
	}
	ApplyChanges();
}

void MainMenu_Options_Controls::OnCommand(const char *command)
{
	if (!stricmp(command, "defaults"))
	{
		HDTFUI_Popup *popup = new HDTFUI_Popup(
			this,
			"revert_defaults_queue",
			"#HDTF_Defaults_Message",
			POPUP_PROMPT,
			"#HDTF_Menu_Yes",
			"#HDTF_Menu_No",
			"ConfirmDefaults");

		popup->AddActionSignalTarget(this);

		popup->TakeControl();
	}
	else if (!stricmp(command, "ConfirmDefaults"))
	{
		RevertToDefaults();
		m_pApplyButton->SetEnabled(false);
	}
	/*else if (!stricmp(command, "DefaultsOK"))
	{
		FillInDefaultBindings();
		m_pBinderList->RequestFocus();
	}
	else if (!m_pBinderList->IsCapturing() && !stricmp(command, "ChangeKey"))
	{
		m_pBinderList->StartCaptureMode(dc_blank);
	}
	else if (!m_pBinderList->IsCapturing() && !stricmp(command, "ClearKey"))
	{
		OnKeyCodePressed(KEY_DELETE);
		m_pBinderList->RequestFocus();
	}*/
	else if (FStrEq(command, "apply"))
	{
		ApplyChanges();
		m_pApplyButton->SetEnabled(false);
	}
	else if (FStrEq(command, "ConfirmLeave"))
	{
		m_pApplyButton->SetEnabled(false);
		BaseClass::OnCommand("menu_back");
	}
	else if (FStrEq(command, "ExternalKeyChange"))
	{
		m_pApplyButton->SetEnabled(true);
	}
	else
	{
		BaseClass::OnCommand(command);
	}
}

void MainMenu_Options_Controls::OnSwitchedTo()
{
	SetActiveValues();
}

void MainMenu_Options_Controls::OnThink()
{
	if (m_flSwitchAllowed > 0.f)
		m_flSwitchAllowed = max(0.f, m_flSwitchAllowed - gpGlobals->absoluteframetime);

	BaseClass::OnThink();
}

void MainMenu_Options_Controls::PaintTraverse(bool repaint, bool allowForce)
{
	if (guiroot && guiroot->IsSwitching())
	{
		const float alphaMultiplier = surface()->DrawGetAlphaMultiplier();
		surface()->DrawSetAlphaMultiplier(1.f - guiroot->GetTransitionValue());

		BaseClass::PaintTraverse(repaint, allowForce);

		surface()->DrawSetAlphaMultiplier(alphaMultiplier);

		return;
	}

	BaseClass::PaintTraverse(repaint, allowForce);
}

void MainMenu_Options_Controls::UnbindButtonsOfKind(ButtonCode_t code)
{
	for (int i = 0; i < m_pBinders.Count(); i++)
	{
		if (m_pBinders[i]->GetButton() == code)
		{
			m_pBinders[i]->UnBind();
			m_pBinders[i]->UpdateBoundKey();
		}
	}
}

void MainMenu_Options_Controls::OnKeyCodeTyped(KeyCode code)
{
	if (g_OptionsBindTarget != NULL)
	{
		HDTFUI_LabeledBindEntry *pBind = (HDTFUI_LabeledBindEntry *)g_OptionsBindTarget;
		if (code == KEY_ESCAPE)
		{
			pBind->FinishCapture();
			pBind->UpdateBoundKey();
			SetSwitchDelay();
			return;
		}

		UnbindButtonsOfKind(code);
		pBind->ChangeBind(code);
		pBind->UpdateBoundKey();
		pBind->FinishCapture();
		m_pApplyButton->SetEnabled(true);
		return;
	}
	else if(code == KEY_UP)
	{
		m_iScrollOffset = max(0, min(m_iMaxScroll, m_iScrollOffset - m_iScrollSize));
		m_pScrollHolder->SetPos(0, -m_iScrollOffset);
	}
	else if (code == KEY_DOWN)
	{
		m_iScrollOffset = max(0, min(m_iMaxScroll, m_iScrollOffset + m_iScrollSize));
		m_pScrollHolder->SetPos(0, -m_iScrollOffset);
	}

	BaseClass::OnKeyCodeTyped(code);
}

void MainMenu_Options_Controls::OnMousePressed(MouseCode code)
{
	if (g_OptionsBindTarget != NULL)
	{
		HDTFUI_LabeledBindEntry *pBind = (HDTFUI_LabeledBindEntry *)g_OptionsBindTarget;

		UnbindButtonsOfKind(code);
		pBind->ChangeBind(code);
		pBind->UpdateBoundKey();
		pBind->FinishCapture();
		m_pApplyButton->SetEnabled(true);
		return;
	}

	BaseClass::OnKeyCodeTyped(code);
}

#pragma endregion

#pragma region Gameplay options tab

MainMenu_Options_Gameplay::MainMenu_Options_Gameplay(vgui::Panel *parent) : HDTFMenuCanvas(parent, "options_AudioTab")
{
	m_pApplyButton = new Button(this, "backButton", "#HDTF_Options_Apply");
	m_pApplyButton->SetCommand("apply");
	m_pApplyButton->AddActionSignalTarget(this);
	m_pApplyButton->SetVisible(true);
	m_pApplyButton->SetZPos(99);

	m_pDifficulty = new HDTFUI_LabeledComboBox(this, "options_Gameplay_Skill", "#HDTF_Gameplay_Skill", 3);
	m_pCrosshair = new HDTFUI_LabeledComboBox(this, "options_Gameplay_Crosshair", "#HDTF_Gameplay_Crosshair", 2);
	m_pHudFadeOut = new HDTFUI_LabeledComboBox(this, "options_Gameplay_HudFadeOut", "#HDTF_Gameplay_HudFadeOut", 2);

	m_pFov = new HDTFUI_LabeledCvarSlider(this, "options_Gameplay_FOV", "#HDTF_Gameplay_FOV", "fov", 75.f, 90.f);

	m_pViewBob = new HDTFUI_LabeledComboBox(this, "options_Gameplay_ViewBob", "#HDTF_Gameplay_ViewBob", 4);
	m_pConsole = new HDTFUI_LabeledComboBox(this, "options_Gameplay_Console", "#HDTF_Gameplay_Console", 2);
	m_pAutoLean = new HDTFUI_LabeledComboBox(this, "options_Gameplay_AutoLean", "#HDTF_Gameplay_AutoLean", 2);
	m_pAutoCrouchJump = new HDTFUI_LabeledComboBox(this, "options_Gameplay_AutoCrouchJump", "#HDTF_Gameplay_AutoCrouchJump", 2);

	m_pDifficulty->AddActionSignalTarget(this);
	m_pCrosshair->AddActionSignalTarget(this);

	m_pBlackOut = new HDTF_Blackout(this, "blackout");
	m_pBlackOut->SetVisible(false);

	m_flIgnoreMessagesUntil = 0.f;

	SetupElements();
}

void MainMenu_Options_Gameplay::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	m_pApplyButton->SetFont(pScheme->GetFont("MainMenuButtons", false));
}

void MainMenu_Options_Gameplay::OnSelectionChanged(KeyValues *data)
{
	// HACKHACK(wheatley): ignore messages from the frame where we initialized default values for the fields
	if (m_flIgnoreMessagesUntil >= gpGlobals->realtime)
	{
		return;
	}

	m_pApplyButton->SetEnabled(true);
}

void MainMenu_Options_Gameplay::OnMenuPopulated(KeyValues *data)
{
	m_pBlackOut->SetVisible(true);
}

void MainMenu_Options_Gameplay::OnMenuHidden(KeyValues *data)
{
	m_pBlackOut->SetVisible(false);
}

void MainMenu_Options_Gameplay::ArrangeItem(Panel *item, int xpos, int ypos)
{
	int wide = (GetWide() / 1.85f) - scheme()->GetProportionalScaledValue(45);

	item->SetSize(wide, CONTROL_HEIGHT);
	item->SetPos(xpos, ypos + m_iItemOffset);

	m_iItemOffset += CONTROL_HEIGHT + CONTROL_SPACING;
}

void MainMenu_Options_Gameplay::PerformLayout()
{
	m_iItemOffset = 0;

	int wide, tall;
	surface()->GetScreenSize(wide, tall);
	SetSize(wide, tall);
	m_pBlackOut->SetSize(wide, tall);

	int spacing = scheme()->GetProportionalScaledValue(10);

	int xpos = scheme()->GetProportionalScaledValue(45);
	int ypos = (tall / 8) + spacing;

	ArrangeItem(m_pDifficulty, xpos, ypos);
	ArrangeItem(m_pCrosshair, xpos, ypos);
	ArrangeItem(m_pHudFadeOut, xpos, ypos);

	INSERT_SPACER();

	ArrangeItem(m_pFov, xpos, ypos);
	ArrangeItem(m_pViewBob, xpos, ypos);

	INSERT_SPACER();

	ArrangeItem(m_pConsole, xpos, ypos);
	ArrangeItem(m_pAutoLean, xpos, ypos);
	ArrangeItem(m_pAutoCrouchJump, xpos, ypos);

	m_pApplyButton->SetPos(scheme()->GetProportionalScaledValue(180), tall - (tall / 9));
	m_pApplyButton->SetWide(scheme()->GetProportionalScaledValue(100));
	m_pApplyButton->SetFgColor(Color(255, 255, 255, 255));
	m_pApplyButton->SetSelectedColor(Color(100, 5, 0, 255), Color(0, 0, 0, 0));
	m_pApplyButton->SetArmedColor(Color(255, 25, 0, 255), Color(0, 0, 0, 0));
	m_pApplyButton->SetPaintBorderEnabled(false);
}

void MainMenu_Options_Gameplay::SetupElements()
{
	ADD_QUALITY_LEVELS3(m_pDifficulty, "#HDTF_Gameplay_Skill_0", "#HDTF_Gameplay_Skill_1", "#HDTF_Gameplay_Skill_2");
	ADD_QUALITY_LEVELS2(m_pCrosshair, "#HDTF_Options_Disabled", "#HDTF_Options_Enabled");
	ADD_QUALITY_LEVELS2(m_pHudFadeOut, "#HDTF_Options_Disabled", "#HDTF_Options_Enabled");
	ADD_QUALITY_LEVELS4(
		m_pViewBob,
		"#HDTF_Options_Disabled",
		"#HDTF_Options_ViewBobLow",
		"#HDTF_Options_ViewBobMedium",
		"#HDTF_Options_ViewBobNormal");
	ADD_QUALITY_LEVELS2(m_pConsole, "#HDTF_Options_Disabled", "#HDTF_Options_Enabled");
	ADD_QUALITY_LEVELS2(m_pAutoLean, "#HDTF_Options_Disabled", "#HDTF_Options_Enabled");
	ADD_QUALITY_LEVELS2(m_pAutoCrouchJump, "#HDTF_Options_Disabled", "#HDTF_Options_Enabled");
}

void MainMenu_Options_Gameplay::SetActiveValues()
{
	ConVarRef skill("skill");
	ConVarRef crosshair("crosshair");
	ConVarRef r_hud_fade("r_hud_fade");
	ConVarRef fov_desired("fov_desired");
	ConVarRef con_enabled("con_enable");
	ConVarRef cl_viewbob_enabled("cl_viewbob_enabled");
	ConVarRef cl_viewbob_intensity("cl_viewbob_intensity");
	ConVarRef hdtf_autolean("hdtf_autolean");
	ConVarRef hdtf_auto_crouch_jump("hdtf_auto_crouch_jump");

	m_pDifficulty->ActivateItem(clamp(skill.GetInt() - 1, 0, 2));
	m_pCrosshair->ActivateItem((crosshair.GetInt() != 0) ? 1 : 0);
	m_pHudFadeOut->ActivateItem((r_hud_fade.GetInt() != 0) ? 1 : 0);
	m_pConsole->ActivateItem((con_enabled.GetInt() != 0) ? 1 : 0);
	m_pAutoLean->ActivateItem((hdtf_autolean.GetInt() != 0) ? 1 : 0);
	m_pAutoCrouchJump->ActivateItem((hdtf_auto_crouch_jump.GetInt() != 0) ? 1 : 0);

	if (!cl_viewbob_enabled.GetBool())
	{
		m_pViewBob->ActivateItem(0);
	}
	else
	{
		const float intensity = cl_viewbob_intensity.GetFloat();
		if(intensity <= VIEWBOB_INTENSITY_LOW)
			m_pViewBob->ActivateItem(1);
		else if (intensity <= VIEWBOB_INTENSITY_MED)
			m_pViewBob->ActivateItem(2);
		else if (intensity <= VIEWBOB_INTENSITY_DEF)
			m_pViewBob->ActivateItem(3);
	}

	m_pFov->SetValue(fov_desired.GetFloat());

	m_pApplyButton->SetEnabled(false);

	// HACKHACK(wheatley): a horrible hack to prevent apply button being immediately activated upon
	// setting default values to the fields
	m_flIgnoreMessagesUntil = gpGlobals->realtime + gpGlobals->absoluteframetime * 3;
}

void MainMenu_Options_Gameplay::ApplyChanges()
{
	ConVarRef skill("skill");
	ConVarRef crosshair("crosshair");
	ConVarRef r_hud_fade("r_hud_fade");
	ConVarRef fov_desired("fov_desired");
	ConVarRef con_enabled("con_enable");
	ConVarRef cl_viewbob_enabled("cl_viewbob_enabled");
	ConVarRef cl_viewbob_intensity("cl_viewbob_intensity");
	ConVarRef hdtf_autolean("hdtf_autolean");

	skill.SetValue(m_pDifficulty->GetActiveItem() + 1);
	fov_desired.SetValue(m_pFov->GetValue());

	if (m_pViewBob->GetActiveItem() == 0)
	{
		cl_viewbob_enabled.SetValue(false);
	}
	else
	{
		cl_viewbob_enabled.SetValue(true);
		float levels[3] = { VIEWBOB_INTENSITY_LOW, VIEWBOB_INTENSITY_MED, VIEWBOB_INTENSITY_DEF };
		cl_viewbob_intensity.SetValue(levels[m_pViewBob->GetActiveItem() - 1]);
	}

	UpdateConVar("crosshair", m_pCrosshair->GetActiveItem());
	UpdateConVar("r_hud_fade", m_pHudFadeOut->GetActiveItem());
	UpdateConVar("con_enable", m_pConsole->GetActiveItem());
	UpdateConVar("hdtf_autolean", m_pAutoLean->GetActiveItem());
	UpdateConVar("hdtf_auto_crouch_jump", m_pAutoCrouchJump->GetActiveItem());

	engine->ClientCmd_Unrestricted("host_writeconfig");
}

void MainMenu_Options_Gameplay::OnCommand(const char *command)
{
	if (FStrEq(command, "apply"))
	{
		ApplyChanges();
		m_pApplyButton->SetEnabled(false);
	}
	else if (FStrEq(command, "ConfirmLeave"))
	{
		m_pApplyButton->SetEnabled(false);
		BaseClass::OnCommand("menu_back");
	}
	else
	{
		BaseClass::OnCommand(command);
	}
}

void MainMenu_Options_Gameplay::OnSwitchedTo()
{
	SetActiveValues();
}

bool MainMenu_Options_Gameplay::CanLeave()
{
	if (m_pApplyButton->IsEnabled())
	{
		HDTFUI_Popup* popup = new HDTFUI_Popup(
			this,
			"gameplay_options_not_saved",
			"#HDTF_SettingsNotSaved",
			POPUP_PROMPT,
			"#HDTF_SettingsLeave",
			"#HDTF_SettingsStay",
			"ConfirmLeave");

		popup->AddActionSignalTarget(this);
		popup->TakeControl();

		return false;
	}

	return true;
}

#pragma endregion

MainMenu_Options::MainMenu_Options(vgui::Panel *parent, const char *name) : BaseClass(parent, name)
{
	SetParent(parent);

	m_pCanvasVideo = new MainMenu_Options_Video(guiroot);
	m_pCanvasAudio = new MainMenu_Options_Audio(guiroot);
	m_pCanvasControls = new MainMenu_Options_Controls(guiroot);
	m_pCanvasControls->MakePopup(); // in order to accept keyboard events
	m_pCanvasGameplay = new MainMenu_Options_Gameplay(guiroot);

	m_pCanvasVideo->SetZPos(98);
	m_pCanvasAudio->SetZPos(98);
	m_pCanvasControls->SetZPos(98);
	m_pCanvasGameplay->SetZPos(98);

	m_pCanvasVideo->SetVisible(false);
	m_pCanvasAudio->SetVisible(false);
	m_pCanvasControls->SetVisible(false);
	m_pCanvasGameplay->SetVisible(false);

	m_pButtonVideo = new HDTFMenuButton(this, "options_VideoButton", "#HDTF_Options_Video");
	m_pButtonAudio = new HDTFMenuButton(this, "options_VideoButton", "#HDTF_Options_Audio");
	m_pButtonControls = new HDTFMenuButton(this, "options_VideoButton", "#HDTF_Options_Controls");
	m_pButtonGameplay = new HDTFMenuButton(this, "options_VideoButton", "#HDTF_Options_Gameplay");

	m_pButtonVideo->SetCommand("Switch_Video");
	m_pButtonAudio->SetCommand("Switch_Audio");
	m_pButtonControls->SetCommand("Switch_Controls");
	m_pButtonGameplay->SetCommand("Switch_Gameplay");
}

void MainMenu_Options::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	int wide, tall;
	surface()->GetScreenSize(wide, tall);
	SetSize(wide, tall);

	int btnOffsetX = scheme()->GetProportionalScaledValue(45);

	// this equals to logoY + logoH
	int btnStartY =
		scheme()->GetProportionalScaledValue(150) +
		scheme()->GetProportionalScaledValue(60);

	int btnWidth = scheme()->GetProportionalScaledValue(430);
	int btnHeight = BUTTONS_HEIGHT;

	m_pButtonVideo->SetPos(btnOffsetX, btnStartY);
	m_pButtonAudio->SetPos(btnOffsetX, btnStartY + btnHeight * 1);
	m_pButtonControls->SetPos(btnOffsetX, btnStartY + btnHeight * 2);
	m_pButtonGameplay->SetPos(btnOffsetX, btnStartY + btnHeight * 3);

	m_pButtonVideo->SetSize(btnWidth, btnHeight);
	m_pButtonAudio->SetSize(btnWidth, btnHeight);
	m_pButtonControls->SetSize(btnWidth, btnHeight);
	m_pButtonGameplay->SetSize(btnWidth, btnHeight);
}

void MainMenu_Options::OnCommand(const char *command)
{
	if (FStrEq(command, "Switch_Video"))
	{
		guiroot->SwitchCanvas(m_pCanvasVideo);
	}
	else if (FStrEq(command, "Switch_Audio"))
	{
		guiroot->SwitchCanvas(m_pCanvasAudio);
	}
	else if (FStrEq(command, "Switch_Controls"))
	{
		guiroot->SwitchCanvas(m_pCanvasControls);
	}
	else if (FStrEq(command, "Switch_Gameplay"))
	{
		guiroot->SwitchCanvas(m_pCanvasGameplay);
	}
	else
	{
		BaseClass::OnCommand(command);
	}
}
