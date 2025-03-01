#pragma once

#include "vgui_controls/Panel.h"
#include "vgui_controls/Label.h"

#define HDTF_VERSION_OVERLAY_ENABLED

class HDTFUI_VersionOverlay : public vgui::Panel, IGameEventListener2
{
    DECLARE_CLASS_SIMPLE(HDTFUI_VersionOverlay, vgui::Panel);
public:
    HDTFUI_VersionOverlay(vgui::VPANEL parent);
    ~HDTFUI_VersionOverlay();

    virtual void FireGameEvent(IGameEvent* event);

protected:
    void            SetMessage(const int mapVersion);
    virtual void	ApplySchemeSettings(vgui::IScheme* pScheme);
    virtual void    PerformLayout();

private:
    vgui::Label* m_pUpperText;
    vgui::Label* m_pLowerText;
};

class HDTFUI_VersionOverlayContainer
{
public:
    void Create(vgui::VPANEL parent)
    {
#ifdef HDTF_VERSION_OVERLAY_ENABLED
        if (m_pInstance == NULL)
        {
            m_pInstance = new HDTFUI_VersionOverlay(parent);
        }
#endif
    }

    void Destroy()
    {
        if (m_pInstance != NULL)
        {
            m_pInstance->SetParent((vgui::Panel*)NULL);
            delete m_pInstance;
            m_pInstance = NULL;
        }
    }

    HDTFUI_VersionOverlay* GetInstance()
    {
        return m_pInstance;
    }

private:
    HDTFUI_VersionOverlay* m_pInstance = NULL;
};

extern HDTFUI_VersionOverlayContainer* g_VersionOverlayContainer;
