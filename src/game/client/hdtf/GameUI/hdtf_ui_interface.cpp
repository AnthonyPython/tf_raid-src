#include "cbase.h"
#include "hdtf_ui_interface.h"
#include "hdtf_ui.h"
#include "ui_loading.h"

// Derived class of override interface
class CHDTFInterface : public IHDTFInterface
{
private:
	HDTFUI_Root *MainMenu;

public:
	int UI_Interface(void)
	{
		MainMenu = NULL;
		return 0;
	}

	void Create(vgui::VPANEL parent)
	{
		// Create immediately
		MainMenu = new HDTFUI_Root(parent);
		guiloading = new HDTFUI_Loading(parent);
	}

	vgui::VPANEL GetPanel(void)
	{
		if (!MainMenu)
			return NULL;
		return MainMenu->GetVPanel();
	}

	void Destroy(void)
	{
		if (MainMenu)
		{
			MainMenu->SetParent((vgui::Panel *)NULL);
			delete MainMenu;
		}
	}
};

static CHDTFInterface g_SMenu;
IHDTFInterface *HDTFUI = (IHDTFInterface *)&g_SMenu;