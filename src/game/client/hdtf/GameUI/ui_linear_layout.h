#pragma once

#include "ui_layout_builder.h"

namespace vgui
{
    class Panel;
}

class LinearLayoutBuilder : public LayoutBuilder
{
public:
    enum class ELayoutDirection : uint8
    {
        Vertical,
        Horizontal,
    };

    enum ECrossAxisAlignment : uint8
    {
        Start,
        Center,
        End,
        Stretch,
    };

    LinearLayoutBuilder(
        ELayoutDirection direction = ELayoutDirection::Vertical,
        ECrossAxisAlignment crossAxisAlignment = ECrossAxisAlignment::Center
    );

    virtual void InsertItem(vgui::Panel* panel) override;
    virtual void InsertSpacer(int spacerSize = 0);

    void SetRootPosition(int x, int y) { this->x = x; this->y = y; }
    void EnforceMainAxisSize(int mainAxisSize);
    void EnforceCrossAxisSize(int crossAxisSize) { this->crossAxisSize = crossAxisSize; }
    void SetSpacing(int spacing) { this->spacing = spacing; }

protected:
    virtual int MainAxisSizeFromPanel(vgui::Panel* panel) const;
    virtual int CrossAxisSizeFromPanel(vgui::Panel* panel) const;

    virtual void Recompute();
    virtual bool RecomputeMetrics();
    virtual void RepositionItems();

    virtual void PlaceItem(vgui::Panel* item);

    ELayoutDirection direction;
    ECrossAxisAlignment crossAxisAlignment;

    int crossAxisSize;
    int advance;
    int spacing;

    int x;
    int y;

    int enforcedMainAxisSize;

    CUtlVector<vgui::Panel*> items;
};
