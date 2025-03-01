#include "cbase.h"
#include "materialsystem/imaterialvar.h"
#include "baseplayer_shared.h"

#include "game_controls/FloatSlider.h"
#include "vgui_controls/TextEntry.h"
#include "vgui_controls/Frame.h"
#include "vgui/IInput.h"
#include "ienginevgui.h"

#include "tier0/memdbgon.h"

// The material we are trying to modify.
CMaterialReference test_mat;

// WRD: I made these global because VS was complaining when I put them in the floatslider class
// and this works so I'm gonna leave it at that... :)
IMaterialVar *BasetextureTintParam;
float BasetextureTintNew[3] = { 1, 1, 1 };

IMaterialVar *MRAOTintParam;
float MRAOTintNew[3] = { 1, 1, 1 };

//-----------------------------------------------------------------------------
// CPreciseSlider
// A drop-in replacement for the slider class that contains a text entry that
// can be used to read and set the current value.
// Also provides mousewheel support.   
// WRD : I ripped this from g-string's c_lights.cpp and renamed it
//-----------------------------------------------------------------------------
class CPreciseSlider : public vgui::FloatSlider
{
	DECLARE_CLASS_SIMPLE(CPreciseSlider, vgui::FloatSlider);

public:
	CPreciseSlider(Panel *parent, const char *panelName);
	~CPreciseSlider();

	virtual void SetValue(float value, bool bTriggerChangeMessage = true);

	virtual void OnSizeChanged(int wide, int tall);

	virtual void GetTrackRect(int &x, int &y, int &w, int &h);

	virtual void SetEnabled(bool state);

protected:

	MESSAGE_FUNC_PARAMS(OnTextNewLine, "TextNewLine", data);

	virtual void OnMouseWheeled(int delta);

private:

	vgui::TextEntry	*m_pTextEntry;

	int				 m_nTextEntryWidth;
	int				 m_nSpacing;
};

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CPreciseSlider::CPreciseSlider(Panel *parent, const char *panelName) : BaseClass(parent, panelName)
{
	m_pTextEntry = new vgui::TextEntry(this, "PrecisionEditPanel");
	m_pTextEntry->SendNewLine(true);
	m_pTextEntry->SetCatchEnterKey(true);
	m_pTextEntry->AddActionSignalTarget(this);

	m_nTextEntryWidth = 32;
	m_nSpacing = 8;
	AddActionSignalTarget(parent);
}

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
CPreciseSlider::~CPreciseSlider()
{
}

//-----------------------------------------------------------------------------
// Override OnSizeChanged to update text entry size as well
//-----------------------------------------------------------------------------
void CPreciseSlider::OnSizeChanged(int wide, int tall)
{
	m_pTextEntry->SetBounds(wide - (m_nSpacing + m_nTextEntryWidth) + m_nSpacing, 0, m_nTextEntryWidth, tall - 12);

	BaseClass::OnSizeChanged(wide, tall);
}

//-----------------------------------------------------------------------------
// Override GetTrackRect in order to adjust for the text entry 
//-----------------------------------------------------------------------------
void CPreciseSlider::GetTrackRect(int &x, int &y, int &w, int &h)
{
	int wide, tall;
	GetPaintSize(wide, tall);

	x = 0;
	y = 8;
	w = wide - (_nobSize + m_nTextEntryWidth + m_nSpacing);
	h = 4;
}

//-----------------------------------------------------------------------------
// Override SetValue to update the text entry data
//-----------------------------------------------------------------------------
void CPreciseSlider::SetValue(float value, bool bTriggerChangeMessage)
{
	BaseClass::SetValue(value, bTriggerChangeMessage);

	char szValueString[256];
	sprintf(szValueString, "%.3f", _value);
	m_pTextEntry->SetText(szValueString);
}

//-----------------------------------------------------------------------------
// Override SetEnabled to also effect the text entry field
//-----------------------------------------------------------------------------
void CPreciseSlider::SetEnabled(bool state)
{
	BaseClass::SetEnabled(state);
	m_pTextEntry->SetEnabled(state);
}

//-----------------------------------------------------------------------------
// Handle updates from the text entry field
//-----------------------------------------------------------------------------
void CPreciseSlider::OnTextNewLine(KeyValues *data)
{
	char buf[256];
	m_pTextEntry->GetText(buf, 256);

	float value;
	sscanf(buf, "%f", &value);

	SetValue(value);
}

//-----------------------------------------------------------------------------
// Handle mousewheel updates
//-----------------------------------------------------------------------------
void CPreciseSlider::OnMouseWheeled(int delta)
{
	BaseClass::OnMouseWheeled(delta);

	if (IsEnabled())
	{
		float value = GetValue();

		if (vgui::input()->IsKeyDown(KEY_LCONTROL) || vgui::input()->IsKeyDown(KEY_RCONTROL))
			SetValue(value + delta * 4);
		else
			SetValue(value + delta);
	}
}

class CPBRTweakPanel : vgui::Frame
{
	DECLARE_CLASS_SIMPLE(CPBRTweakPanel, vgui::Frame);
public:
	CPBRTweakPanel(vgui::VPANEL parent);
	~CPBRTweakPanel();

	void	OnMessage(const KeyValues *params, vgui::VPANEL fromPanel);

private:
	void LoadSettings();

	struct
	{
		// All the things we want sliders for...
		// Basetexturetint
		CPreciseSlider* BasetextureTintR;
		CPreciseSlider* BasetextureTintG;
		CPreciseSlider* BasetextureTintB;

		// MRAO
		CPreciseSlider* MRAO_R;
		CPreciseSlider* MRAO_G;
		CPreciseSlider* MRAO_B;

	} Material;
};

CPBRTweakPanel::CPBRTweakPanel(vgui::VPANEL parent) : Frame(NULL, "CPBRTweakPanel")
{

	SetParent(parent);

	// WRD : These ranges here are too large, that is because on a range of 0-1,
	// it will snap between the two ends, with 0-1000 you can smoothly move the slider
	// We just math this away with *0.001 later...
	Material.BasetextureTintR = new CPreciseSlider(this, "BasetextureTintR");
	Material.BasetextureTintR->SetRange(0, 1000);

	Material.BasetextureTintG = new CPreciseSlider(this, "BasetextureTintG");
	Material.BasetextureTintG->SetRange(0, 1000);

	Material.BasetextureTintB = new CPreciseSlider(this, "BasetextureTintB");
	Material.BasetextureTintB->SetRange(0, 1000);

	Material.MRAO_R = new CPreciseSlider(this, "MRAO_R");
	Material.MRAO_R->SetRange(0, 1000);

	Material.MRAO_G = new CPreciseSlider(this, "MRAO_G");
	Material.MRAO_G->SetRange(0, 1000);

	Material.MRAO_B = new CPreciseSlider(this, "MRAO_B");
	Material.MRAO_B->SetRange(0, 1000);

	// Size of the VGUI panel we are opening
	SetSize(128, 256);

	LoadControlSettings("Resource\\PBRUI.res");
	SetDeleteSelfOnClose(true);
	SetTitle("PBR User Interface", true);
	SetVisible(true);
	MoveToFront();
	SetZPos(1500);
	SetMouseInputEnabled(true);
	SetCursorAlwaysVisible(true);
	LoadSettings();
}

CPBRTweakPanel::~CPBRTweakPanel()
{
	SetCursorAlwaysVisible(false);
}

void CPBRTweakPanel::OnMessage(const KeyValues *params, vgui::VPANEL fromPanel)
{
	BaseClass::OnMessage(params, fromPanel);

	if (!Q_stricmp("SliderMoved", params->GetName()))
	{
		// TODO : move setting of the parameters to a new universal function... so that this doesn't look so terrible.
		Panel* panel = reinterpret_cast< Panel* >(const_cast< KeyValues* >(params)->GetPtr("panel", NULL));
		if (panel == Material.BasetextureTintR)
		{
			BasetextureTintNew[0] = Material.BasetextureTintR->GetValue() * 0.001; // we do the 1/1000 so we have actual control...
			// WRD: Supposedly... there is a better way? This works, I leave it at that
			BasetextureTintParam->SetVecComponentValue(BasetextureTintNew[0], 0);
			BasetextureTintParam->SetVecComponentValue(BasetextureTintNew[1], 1);
			BasetextureTintParam->SetVecComponentValue(BasetextureTintNew[2], 2);
		}
		else if (panel == Material.BasetextureTintG)
		{
			BasetextureTintNew[1] = Material.BasetextureTintG->GetValue() * 0.001; // we do the 1/1000 so we have actual control...
			BasetextureTintParam->SetVecComponentValue(BasetextureTintNew[0], 0);
			BasetextureTintParam->SetVecComponentValue(BasetextureTintNew[1], 1);
			BasetextureTintParam->SetVecComponentValue(BasetextureTintNew[2], 2);
		}
		else if (panel == Material.BasetextureTintB)
		{
			BasetextureTintNew[2] = Material.BasetextureTintB->GetValue() * 0.001; // we do the 1/1000 so we have actual control...
			BasetextureTintParam->SetVecComponentValue(BasetextureTintNew[0], 0);
			BasetextureTintParam->SetVecComponentValue(BasetextureTintNew[1], 1);
			BasetextureTintParam->SetVecComponentValue(BasetextureTintNew[2], 2);
		}
		else if (panel == Material.MRAO_R)
		{
			MRAOTintNew[0] = Material.MRAO_R->GetValue() * 0.001; // we do the 1/1000 so we have actual control...
			MRAOTintParam->SetVecComponentValue(MRAOTintNew[0], 0);
			MRAOTintParam->SetVecComponentValue(MRAOTintNew[1], 1);
			MRAOTintParam->SetVecComponentValue(MRAOTintNew[2], 2);
		}
		else if (panel == Material.MRAO_G)
		{
			MRAOTintNew[1] = Material.MRAO_G->GetValue() * 0.001; // we do the 1/1000 so we have actual control...
			MRAOTintParam->SetVecComponentValue(MRAOTintNew[0], 0);
			MRAOTintParam->SetVecComponentValue(MRAOTintNew[1], 1);
			MRAOTintParam->SetVecComponentValue(MRAOTintNew[2], 2);
		}
		else if (panel == Material.MRAO_B)
		{
			MRAOTintNew[2] = Material.MRAO_B->GetValue() * 0.001; // we do the 1/1000 so we have actual control...
			MRAOTintParam->SetVecComponentValue(MRAOTintNew[0], 0);
			MRAOTintParam->SetVecComponentValue(MRAOTintNew[1], 1);
			MRAOTintParam->SetVecComponentValue(MRAOTintNew[2], 2);
		}
			//engine->ClientCmd_Unrestricted(("mat_reloadmaterial %s", test_mat->GetName()));
	}
}

void CPBRTweakPanel::LoadSettings()
{
	// TODO: find a way to load default values and/or the values in the vmt.

	//const auto& data = g_pCSMEnvLight->shadowConfigs;
	//Material.BasetextureTintNew->SetVecValue(); Material.BasetextureTintParam[0];
	//Material.BasetextureTintNew[2]->SetValue();
	//near.pForwardOffset->SetValue(data[0].flForwardOffset, false);
	//near.pOrthoSize->SetValue(data[0].flOrthoSize, false);
	//near.pUVOffsetX->SetValue(data[0].flUVOffsetX, false);
	//near.pViewDepthBiasHack->SetValue(data[0].flViewDepthBiasHack, false);

	//far.pForwardOffset->SetValue(data[1].flForwardOffset, false);
	//far.pOrthoSize->SetValue(data[1].flOrthoSize, false);
	//far.pUVOffsetX->SetValue(data[1].flUVOffsetX, false);
	//far.pViewDepthBiasHack->SetValue(data[1].flViewDepthBiasHack, false);
}

CON_COMMAND(PBRUI, "MaterialName")
{
	// If a material is already bound, shut it down so we can bind a new one.
	test_mat.Shutdown();

	// Make sure the User actually put something after the command...
	if (args.ArgC() >= 2) // TODO : what could we with 3+ arguments?
	{
		// TODO: Consider doing STRING() instead or seeing if args[1] might already return as a string
		char MaterialName[256];
		Q_strncpy(MaterialName, args[1], sizeof(MaterialName));
		Q_strlower(MaterialName);

		// Search our material using the name specified after PBRUI
		// NOTE : TEXTURE_GROUP_OTHER appears to work for everything...
		test_mat.Init(materials->FindMaterial(MaterialName, TEXTURE_GROUP_OTHER, false)); // no complaining, we do the check anyways
		
		if (!test_mat->IsErrorMaterial())
		{
			// Find Parameters
			BasetextureTintParam = test_mat->FindVar("$Basetexturetint", NULL);
			MRAOTintParam = test_mat->FindVar("$MRAOTint", NULL);

			// Open our Vgui panel
			new CPBRTweakPanel(enginevgui->GetPanel(PANEL_CLIENTDLL));
		}
		else
		{
			Msg("-I.. -I.. I really Tried to find %s!!! but I fucking failed... Gomen'nasai ;( \n", MaterialName);
			test_mat.Shutdown();
		}
	}
	else
	{
		// TODO: hook mat_crosshair somehow. If that's not possible, do whatever impulse 107 does.
		Msg("No Material specified. \n");
	}

	//materials->ReloadMaterials(NULL);
	//engine->ClientCmd_Unrestricted(("mat_reloadmaterial %s", test_mat->GetName()));
}