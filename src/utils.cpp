#include "utils.hpp"
#include "common.hpp"
#include "main.hpp"


Widget *Utils::MakeWidget(const char *id, const char *type, const char *style, Widget *root)
{
    rco::Element search;
    rco::Element widgetInfo;
    rco::Element styleInfo;

    search.id = id;
    widgetInfo.hash = search.GetHash(&search.id);

    search.id = style;
    styleInfo.hash = search.GetHash(&search.id);

    return topmenu->CreateWidgetWithStyle(root, type, &widgetInfo, &styleInfo);
}

Widget *Utils::MakeWidget(SceInt32 id, const char *type, const char *style, Widget *root)
{
    rco::Element search;
    rco::Element widgetInfo;
    rco::Element styleInfo;

    widgetInfo.hash = id;

    search.id = style;
    styleInfo.hash = search.GetHash(&search.id);

    return topmenu->CreateWidgetWithStyle(root, type, &widgetInfo, &styleInfo);
}

SceInt32 Utils::SetWidgetLabel(const char *text, Widget *widget)
{
    if(text == NULL || widget == NULL || text[0] == '\0' /* empty string */ ) return 0;
    wstring wstr;
    common::Utf8ToUtf16(text, &wstr);
    print("Assigning to 0x%X text: %s wstr: %ls\n", widget->elem.hash, text, wstr.c_str());

    return widget->SetLabel(&wstr);
}

Widget *Utils::AddFromTemplate(Plugin *p, Widget *targetRoot, const char *id)
{
    rco::Element e;
    e.id = id;
    e.hash = e.GetHash(&e.id);

    Plugin::TemplateOpenParam tinit;
    p->TemplateOpen(targetRoot, &e, &tinit);

    return targetRoot->GetChild(targetRoot->childNum - 1);
}

SceUInt32 Utils::GetHashById(const char *id)
{
    rco::Element e;
    e.id = id;
    return e.GetHash(&e.id);
}

SceVoid Utils::HideAllChildren(Widget *widget)
{
    for(int i = 0; i < widget->childNum; i++)
    {
        Widget *w = widget->GetChild(i);
        if(w != NULL) 
        {
            w->PlayEffect(0, effect::EffectType_Reset);
            w->animationStatus &= 0x80;
        }
    }
}

bool checkFileExist(const char *path)
{
    return paf::LocalFile::Exists(path);
}

SceVoid Utils::PrintAllChildren(Widget *widget, int offset)
{
    for (int i = 0; i < widget->childNum; i++)
    {
        for (int x = 0; x < offset; x++) print(" ");
        wstring str;
        widget->GetChild(i)->GetLabel(&str);
        print("%d: 0x%X (%s, \"%ls\")\n", i, widget->GetChild(i)->elem.hash, widget->GetChild(i)->name(), str.c_str());
        Utils::PrintAllChildren(widget->GetChild(i), offset + 4);
    }
    if(widget->childNum == 0 && offset == 0) print("No Children to Print!\n");
}