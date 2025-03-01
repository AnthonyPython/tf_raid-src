#pragma once

#include "vgui_controls/Button.h"
#include "vgui_controls/BitmapImagePanel.h"

class HDTFImageButton : public vgui::Button
{
    DECLARE_CLASS_SIMPLE(HDTFImageButton, vgui::Button);
public:
    HDTFImageButton(vgui::Panel* parent, char const* panelName, char const* fileName = NULL);
    ~HDTFImageButton();

protected:
    virtual void ApplySchemeSettings(vgui::IScheme* pScheme);
    virtual void PerformLayout() override;
    virtual void Paint() override;

    vgui::CBitmapImagePanel* bitmapIamge;
    float hoverAnimation;
};
