#include "cbase.h"
#include "ui_version_overlay.h"
#include <vgui/ISurface.h>
#include "networkstringtabledefs.h"

using namespace vgui;

static HDTFUI_VersionOverlayContainer g_SVersionOverlayFactory;
HDTFUI_VersionOverlayContainer* g_VersionOverlayContainer = &g_SVersionOverlayFactory;

static void _VersionOverlayCvarChange(IConVar* var, const char* pOldValue, float flOldValue)
{
    HDTFUI_VersionOverlay* overlay = g_VersionOverlayContainer->GetInstance();
    if (overlay)
    {
        ConVar* cvar = (ConVar*)var;
        overlay->SetVisible(cvar->GetBool());
    }
}
ConVar hdtf_version_overlay("hdtf_version_overlay", "1", FCVAR_HIDDEN, "", _VersionOverlayCvarChange);

HDTFUI_VersionOverlay::HDTFUI_VersionOverlay(VPANEL parent) : Panel(this, "HDTF_VersionOverlay")
{
    SetParent(parent);

    m_pUpperText = new Label(this, "DisclaimerLabel", "");
    m_pLowerText = new Label(this, "DisclaimerLabel", "");

    SetMessage(0);

    gameeventmanager->AddListener(this, "game_newmap", false);

    SetVisible(hdtf_version_overlay.GetBool());
}

HDTFUI_VersionOverlay::~HDTFUI_VersionOverlay()
{
    m_pUpperText->SetParent((Panel*)NULL);
    delete m_pUpperText;
    
    m_pLowerText->SetParent((Panel*)NULL);
    delete m_pLowerText;
}

void HDTFUI_VersionOverlay::FireGameEvent(IGameEvent * pEvent)
{
    if (FStrEq(pEvent->GetName(), "game_newmap"))
    {
        extern INetworkStringTable* g_pStringTableInfoPanel;
        int strIndex = g_pStringTableInfoPanel->FindStringIndex("MapVersion");
        if (strIndex != INVALID_STRING_INDEX)
        {
            int mapVersion = *(int*)g_pStringTableInfoPanel->GetStringUserData(strIndex, NULL);
            SetMessage(mapVersion);
            InvalidateLayout();
        }
    }
}

void HDTFUI_VersionOverlay::SetMessage(const int mapVersion)
{
#if 0
#ifndef HDTF_UI_VERSION_OVERLAY_REV
    char* commitHashShort = "-";
#else
    char* commitHash = HDTF_UI_VERSION_OVERLAY_REV;
    char commitHashShort[9] = { 0 };
    Q_StrSlice(commitHash, 0, 9, commitHashShort, sizeof(commitHashShort));
#endif
#endif

    char versionText[255];
    Q_snprintf(
        versionText,
        sizeof(versionText),
        "HDTF DEV BUILD [COMPILED:%s][MAP VERSION:%int]",
        //"HDTF DEV BUILD [COMPILED:%s][MAP VERSION:%x]",
        __DATE__,
        mapVersion
    );

    m_pUpperText->SetText("THIS BUILD MAY CONTAIN ERRORS AND IS SUBJECT TO CHANGE");
    m_pLowerText->SetText(versionText);
}

void HDTFUI_VersionOverlay::ApplySchemeSettings(vgui::IScheme* pScheme)
{
    BaseClass::ApplySchemeSettings(pScheme);

    m_pUpperText->SetFont(pScheme->GetFont("MainMenuButtons"));
    m_pLowerText->SetFont(pScheme->GetFont("MainMenuButtons"));
}

void HDTFUI_VersionOverlay::PerformLayout()
{
    BaseClass::PerformLayout();

    int w, h;
    surface()->GetScreenSize(w, h);

    SetPos(w / 2, 0);
    SetSize(w / 2, 100);

    int marginTop = 35,
        marginRight = 8,
        spacing = 0;

    m_pUpperText->SizeToContents();
    m_pUpperText->SetPos(
        GetWide() - m_pUpperText->GetWide() - marginRight,
        marginTop
    );

    m_pLowerText->SizeToContents();
    m_pLowerText->SetPos(
        GetWide() - m_pLowerText->GetWide() - marginRight,
        marginTop + m_pUpperText->GetTall() + spacing
    );
}
