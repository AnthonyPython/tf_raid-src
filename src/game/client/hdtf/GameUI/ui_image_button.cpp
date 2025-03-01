#include "cbase.h"
#include "ui_image_button.h"

#include "vgui/ISurface.h"
#include <vgui/ISystem.h>

using namespace vgui;

HDTFImageButton::HDTFImageButton(Panel* parent, char const* panelName, char const* fileName) 
    : Button(parent, panelName, "")
{
    bitmapIamge = new CBitmapImagePanel(this, "imagePanel", fileName);
    bitmapIamge->DisableMouseInputForThisPanel(true);
    SetTextInset(4, 4);

    hoverAnimation = 0.f;

    bitmapIamge->setImageColor(Color(255, 255, 255, 100));
}

HDTFImageButton::~HDTFImageButton()
{
    if (bitmapIamge != NULL)
    {
        delete bitmapIamge;
    }
}

void HDTFImageButton::ApplySchemeSettings(vgui::IScheme* pScheme)
{
    BaseClass::ApplySchemeSettings(pScheme);

    SetPaintBackgroundEnabled(false);
    SetPaintBorderEnabled(false);
}

void HDTFImageButton::PerformLayout()
{
    Panel::PerformLayout();

    SetBorder(GetBorder(false, false, false, HasFocus()));

    SetFgColor(GetButtonFgColor());
    SetBgColor(GetButtonBgColor());

    int insetX, insetY;
    GetTextInset(&insetX, &insetY);

    bitmapIamge->SetPos(insetX, insetY);
    bitmapIamge->SetSize(GetWide() - insetX * 2, GetTall() - insetY * 2);
}

void HDTFImageButton::Paint()
{
    BaseClass::Paint();

    Color primaryColor = Color(255, 255, 255);
    if (IsSelected())
    {
        primaryColor = Color(255, 50, 0);
    }

    const float animTarget = IsCursorOver() ? 1.f : 0.f;
    if (animTarget != hoverAnimation)
    {
        hoverAnimation = Approach(animTarget, hoverAnimation, gpGlobals->absoluteframetime * 5.f);
    }

    if (hoverAnimation > 0.f)
    {
        const int highlightOpacity = 10;
        const int borderWidth = 2;

        surface()->DrawSetColor(
            primaryColor.r(),
            primaryColor.g(),
            primaryColor.b(),
            hoverAnimation * highlightOpacity
        );
        surface()->DrawFilledRect(0, 0, GetWide(), GetTall() - borderWidth * hoverAnimation);

        surface()->DrawSetColor(
            primaryColor.r(),
            primaryColor.g(),
            primaryColor.b(),
            hoverAnimation * 255
        );
        surface()->DrawFilledRect(0, GetTall() - borderWidth * hoverAnimation, GetWide(), GetTall());
    }

    bitmapIamge->setImageColor(Color(
        primaryColor.r(),
        primaryColor.g(),
        primaryColor.b(),
        100 + 155 * hoverAnimation
    ));
}
