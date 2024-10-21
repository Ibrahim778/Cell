#include "utils.hpp"
#include "common.hpp"
#include "main.hpp"


Widget *Utils::GetChildByHash(Widget *Parent, SceUInt32 hash)
{
    if(Parent == NULL) return NULL;
    rco::Element e;
    e.hash = hash;
    return Parent->GetChild(&e, 0);
}

Widget *Utils::GetPageByHash(Plugin *p, SceInt32 hash)
{
    rco::Element e;
    e.hash = hash;
    return p->GetPageByHash(&e);
}

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
    if(text == NULL || widget == NULL || text[0] == '\0' /* empty string */ ) return;
    wstring wstr;
    string str;
    str = text;
    ccc::UTF8toUTF16(&str, &wstr);
    print("Assigning to 0x%X text: %s str: %s wstr: %ls\n", widget->elem.hash, text, str.data(), wstr.data());

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

bool checkFileExist(const char *path)
{
    return paf::LocalFile::Exists(path);
}

SceVoid Utils::PrintAllChildren(Widget *widget, int offset)
{
    for (int i = 0; i < widget->childNum; i++)
    {
        for (int i = 0; i < offset; i++) print(" ");
        wstring wstr;
        widget->GetChild(i)->GetLabel(&wstr);
        print(" %d 0x%X (%s, \"%ls\")\n", i, widget->GetChild(i)->elem.hash, widget->GetChild(i)->name(), wstr.data());
        Utils::PrintAllChildren(widget->GetChild(i), offset + 4);
    }
}