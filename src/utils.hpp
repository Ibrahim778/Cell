#ifndef _UTILS_H_
#define _UTILS_H_

#include <libdbg.h>
#include <kernel.h>
#include <appmgr.h>
#include <kernel/libkernel.h>
#include <paf.h>

using namespace paf;
using namespace ui;

namespace Utils
{
    template<class T = paf::ui::Widget>
    T *GetChildByHash(Widget *parent, SceUInt32 hash)
    {
        rco::Element e;
        e.hash = hash;
        return (T*)parent->GetChild(&e, 0);
    }
    SceInt32 SetWidgetLabel(const char *text, Widget *widget);
    SceUInt32 GetHashById(const char *id);
    SceVoid SetWidgetColor(Widget *w, float r, float g, float b, float a);
    SceVoid SetWidgetPosition(Widget *wi, float x, float y, float z, float w);
    SceVoid SetWidgetSize(Widget *wi, float x, float y, float z, float w);
    SceVoid PrintAllChildren(Widget *widget, int offset);
    Widget *MakeWidget(SceInt32 id, const char *type, const char *style, Widget *root);
    Widget *MakeWidget(const char *id, const char *type, const char *style, Widget *root);
    SceVoid HideAllChildren(Widget *widget);
    Widget *AddFromTemplate(Plugin *p, Widget *targetRoot, const char *id);
    SceVoid SetWidgetTypeToColor(Widget *widget, const char *type);
};

#endif