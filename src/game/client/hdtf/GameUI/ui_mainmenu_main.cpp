#include "cbase.h"
#include "ui_mainmenu_main.h"
#include "ui_image_button.h"
#include "filesystem.h"

#include "vgui/ISurface.h"

#include "ui_linear_layout.h"

using namespace vgui;

MainMenu_Main::MainMenu_Main(Panel *parent, const char *name) : HDTFMenuCanvas(parent, name)
{
	SetParent(parent);

	btn_Continue = new HDTFMenuButton(this, "btnNewGame", "#HDTF_Menu_Continue");
	btn_NewGame = new HDTFMenuButton(this, "btnNewGame", "#HDTF_Menu_NewGame");
	btn_LoadGame = new HDTFMenuButton(this, "btnLoadGame", "#HDTF_Menu_LoadGame");
	btn_Options = new HDTFMenuButton(this, "btnOptions", "#HDTF_Menu_Options");

	btn_SocialTwitter = new HDTFImageButton(this, "btnSocialTwitter", "socials/twitter");
	btn_SocialDiscord = new HDTFImageButton(this, "btnSocialDiscord", "socials/discord");
	btn_SocialPatreon = new HDTFImageButton(this, "btnSocialPatreon", "socials/patreon");

	btn_Quit = new HDTFMenuButton(this, "btnQuit", "#HDTF_Menu_Quit");

	btn_Continue->SetCommand("menu_continue");
	btn_NewGame->SetCommand("menu_newgame");
	btn_LoadGame->SetCommand("menu_loadgame");
	btn_Options->SetCommand("menu_options");
	btn_SocialTwitter->SetCommand("menu_open_twitter");
	btn_SocialDiscord->SetCommand("menu_open_discord");
	btn_SocialPatreon->SetCommand("menu_open_patreon");
	btn_Quit->SetCommand("menu_quit");

	m_LogoMaterial.Init("vgui/logo", TEXTURE_GROUP_VGUI);

	logoX = 0;
	logoY = 0;
	logoW = 0;
	logoH = 0;

	CheckHasAnySaves();
}

void MainMenu_Main::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
}

void MainMenu_Main::PerformLayout()
{
	int wide, tall;
	surface()->GetScreenSize(wide, tall);
	SetSize(wide, tall);

	logoX = scheme()->GetProportionalScaledValue(45);
	logoY = scheme()->GetProportionalScaledValue(150);

	logoW = scheme()->GetProportionalScaledValue(430);
	logoH = scheme()->GetProportionalScaledValue(60);

	int btnSpacing = 0; //scheme()->GetProportionalScaledValue(3);
	int btnOffsetX = scheme()->GetProportionalScaledValue(45);
	int btnStartY = logoY + logoH + btnSpacing;
	int btnHeight = BUTTONS_HEIGHT;

	btn_Continue->SetVisible(false);

	{
		LinearLayoutBuilder mainLayoutBuilder(
			LinearLayoutBuilder::ELayoutDirection::Vertical,
			LinearLayoutBuilder::ECrossAxisAlignment::Stretch
		);
		mainLayoutBuilder.SetRootPosition(btnOffsetX, btnStartY);
		mainLayoutBuilder.SetSpacing(btnSpacing);
		mainLayoutBuilder.EnforceCrossAxisSize(logoW);
		mainLayoutBuilder.EnforceMainAxisSize(btnHeight);

		if (bHasAnySaves)
		{
			btn_Continue->SetVisible(true);
			mainLayoutBuilder.InsertItem(btn_Continue);
			mainLayoutBuilder.InsertSpacer();
		}
		
		mainLayoutBuilder.InsertItem(btn_NewGame);
		mainLayoutBuilder.InsertItem(btn_LoadGame);
		mainLayoutBuilder.InsertSpacer();
		mainLayoutBuilder.InsertItem(btn_Options);
		mainLayoutBuilder.InsertSpacer();
		mainLayoutBuilder.InsertItem(btn_Quit);
	}

	{
		const int bottomButtonSetY = tall - (tall / 8);

		LinearLayoutBuilder socialLinksLayoutBuilder(
			LinearLayoutBuilder::ELayoutDirection::Horizontal,
			LinearLayoutBuilder::ECrossAxisAlignment::Stretch
		);
		socialLinksLayoutBuilder.SetRootPosition(
			btnOffsetX + scheme()->GetProportionalScaledValue(25),
			bottomButtonSetY + btnHeight);
		socialLinksLayoutBuilder.SetSpacing(20);
		socialLinksLayoutBuilder.EnforceCrossAxisSize(45);
		socialLinksLayoutBuilder.EnforceMainAxisSize(45);

		socialLinksLayoutBuilder.InsertItem(btn_SocialTwitter);
		socialLinksLayoutBuilder.InsertItem(btn_SocialDiscord);
		socialLinksLayoutBuilder.InsertItem(btn_SocialPatreon);
	}
}

MainMenu_Main::~MainMenu_Main()
{
}

void MainMenu_Main::DrawLogo()
{
	CMatRenderContextPtr pRenderContext(materials);
	pRenderContext->Bind(m_LogoMaterial);

	CMeshBuilder meshBuilder;
	IMesh* pMesh = pRenderContext->GetDynamicMesh(true);
	meshBuilder.Begin(pMesh, MATERIAL_QUADS, 1);

	float canvasLeftX = logoX;
	float canvasRightX = logoX + logoW;

	float canvasLeftY = logoY;
	float canvasRightY = logoY + logoH;

	float flLeftU = 0.0f;
	float flTopV = 0.0f;

	float flRightU = 1.f;
	float flBottomV = 1.f;

	for (int corner = 0; corner < 4; corner++)
	{
		bool bLeft = (corner == 0) || (corner == 3);
		meshBuilder.Position3f((bLeft) ? canvasLeftX : canvasRightX, (corner & 2) ? canvasRightY : canvasLeftY, 0.0f);
		meshBuilder.TexCoord2f(0, (bLeft) ? flLeftU : flRightU, (corner & 2) ? flBottomV : flTopV);
		meshBuilder.Color4f(1.f, 1.f, 1.f, surface()->DrawGetAlphaMultiplier());
		meshBuilder.AdvanceVertex();
	}
	meshBuilder.End();

	pMesh->Draw();
}

void MainMenu_Main::Paint()
{
	DrawLogo();
}

void MainMenu_Main::CheckHasAnySaves()
{
	const char saveFileWildcard[] = "save/*.sav";

	FileFindHandle_t searchHandle;
	const char *fileName = filesystem->FindFirst(saveFileWildcard, &searchHandle);
	const bool hasSave = (bool)fileName;
	filesystem->FindClose(searchHandle);

	if (hasSave != bHasAnySaves)
	{
		bHasAnySaves = hasSave;
		InvalidateLayout();
	}
}

void MainMenu_Main::OnBeforeSwitchTo()
{
	CheckHasAnySaves();
}
