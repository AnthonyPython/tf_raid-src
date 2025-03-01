#include "cbase.h"
#include "ui_linear_layout.h"

#include "vgui_controls/Panel.h"

LinearLayoutBuilder::LinearLayoutBuilder(
    ELayoutDirection direction,
    ECrossAxisAlignment crossAxisAlignment)
{
    this->direction = direction;
    this->crossAxisAlignment = crossAxisAlignment;

    crossAxisSize = 0;
    advance = 0;
    spacing = 0;

    x = 0;
    y = 0;
}

void LinearLayoutBuilder::InsertItem(vgui::Panel* panel)
{
    items.AddToTail(panel);

    PlaceItem(panel);
    Recompute();
}

void LinearLayoutBuilder::InsertSpacer(int spacerSize)
{
    if (spacerSize > 0)
    {
        advance += spacerSize;
        return;
    }

    if (enforcedMainAxisSize > 0)
    {
        advance += enforcedMainAxisSize;
        return;
    }

    if (items.Count() > 0)
    {
        advance += MainAxisSizeFromPanel(items.Tail());
        return;
    }

    // NOTE(wheatley): shouldn't really happen and, probably, shouldn't be 
    // here at all but, eh, whatever.
    advance += spacing;
}

void LinearLayoutBuilder::EnforceMainAxisSize(int mainAxisSize)
{
    enforcedMainAxisSize = mainAxisSize;

    if (enforcedMainAxisSize > 0)
    {
        RepositionItems();
    }
}

int LinearLayoutBuilder::MainAxisSizeFromPanel(vgui::Panel* panel) const
{
    if (panel == NULL)
    {
        return 0;
    }

    switch (direction)
    {
        case ELayoutDirection::Vertical:
            return panel->GetTall();

        case ELayoutDirection::Horizontal:
            return panel->GetWide();
    }

    return 0;
}

int LinearLayoutBuilder::CrossAxisSizeFromPanel(vgui::Panel* panel) const
{
    if (panel == NULL)
    {
        return 0;
    }

    switch (direction)
    {
    case ELayoutDirection::Vertical:
        return panel->GetWide();

    case ELayoutDirection::Horizontal:
        return panel->GetTall();
    }

    return 0;
}

void LinearLayoutBuilder::Recompute()
{
    if (RecomputeMetrics())
    {
        RepositionItems();
    }
}

bool LinearLayoutBuilder::RecomputeMetrics()
{
    bool bHadAnyChanges = false;

    if (crossAxisAlignment != ECrossAxisAlignment::Stretch)
    {
        const int originalCrossAxisSize = crossAxisSize;
        for (vgui::Panel* item : items)
        {
            crossAxisSize = max(crossAxisSize, CrossAxisSizeFromPanel(item));
        }

        bHadAnyChanges |= (originalCrossAxisSize != crossAxisSize);
    }

    return bHadAnyChanges;
}

void LinearLayoutBuilder::RepositionItems()
{
    advance = 0;

    for (vgui::Panel* item : items)
    {
        PlaceItem(item);
    }
}

void LinearLayoutBuilder::PlaceItem(vgui::Panel* item)
{
    int resultX = x;
    int resultY = y;

    if (crossAxisAlignment == ECrossAxisAlignment::Stretch)
    {
        switch (direction)
        {
            case ELayoutDirection::Vertical:
                item->SetWide(crossAxisSize);
                break;

            case ELayoutDirection::Horizontal:
                item->SetTall(crossAxisSize);
                break;
        }
    }

    if (enforcedMainAxisSize > 0)
    {
        switch (direction)
        {
        case ELayoutDirection::Vertical:
            item->SetTall(enforcedMainAxisSize);
            break;

        case ELayoutDirection::Horizontal:
            item->SetWide(enforcedMainAxisSize);
            break;
        }
    }

    switch (direction)
    {
    case ELayoutDirection::Vertical:
        resultY += advance;
        advance += item->GetTall() + spacing;
        break;

    case ELayoutDirection::Horizontal:
        resultX += advance;
        advance += item->GetWide() + spacing;
        break;
    }

    item->SetPos(resultX, resultY);
}
