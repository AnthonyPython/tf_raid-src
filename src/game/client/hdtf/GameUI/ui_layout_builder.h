#pragma once

namespace vgui
{
    class Panel;
}

class LayoutBuilder
{
public:
    virtual void InsertItem(vgui::Panel* panel) = 0;
};
