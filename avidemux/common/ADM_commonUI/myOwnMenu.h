/**
    \file myOwnMenus

*/
#pragma once
#include <vector>
#include <string>

/**
    \enum MenuType
*/
enum MenuType
{
    MENU_ACTION,
    MENU_SEPARATOR,
    MENU_SUBACTION,
    MENU_SUBMENU
};
/**
    \struct MenuEntry
*/
typedef struct
{
    MenuType   type;
    std::string text;
    void       *cookie;
    Action     event;
    const char *icon;
    const char *shortCut;
}MenuEntry;

#ifdef MENU_DECLARE
static const MenuEntry _myMenuFile[] = {
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Open"),               NULL,ACT_OPEN_VIDEO,       MKICON(fileopen), "Ctrl+O"},
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Append"),             NULL,ACT_APPEND_VIDEO     ,NULL,             "Ctrl+A"},
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Save"),               NULL,ACT_SAVE_VIDEO       ,MKICON(filesaveas),"Ctrl+S"},
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Queue"),              NULL,ACT_SAVE_QUEUE       ,NULL              ,"Ctrl+U"},
            {MENU_SUBMENU,QT_TRANSLATE_NOOP("adm","Save as Image"),     NULL,ACT_DUMMY    ,NULL,NULL},
            {MENU_SUBACTION,QT_TRANSLATE_NOOP("adm","Save as BMP"),     NULL,ACT_SAVE_BMP ,NULL,"Ctrl+M"},
            {MENU_SUBACTION,QT_TRANSLATE_NOOP("adm","Save as JPEG"),    NULL,ACT_SAVE_JPG ,NULL,"Ctrl+E"},
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Close"),              NULL,ACT_CLOSE          ,NULL,                "Ctrl+W"},
            {MENU_SEPARATOR,QT_TRANSLATE_NOOP("adm","-"),               NULL,ACT_DUMMY             ,NULL,NULL},
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Information"),        NULL,ACT_VIDEO_PROPERTIES,                 MKICON(info),NULL},
            {MENU_SEPARATOR,QT_TRANSLATE_NOOP("adm","-"),               NULL,ACT_DUMMY             ,NULL,NULL},
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Connect to avsproxy"),NULL,ACT_AVS_PROXY,NULL},
            {MENU_SEPARATOR,QT_TRANSLATE_NOOP("adm","-"),               NULL,ACT_DUMMY             ,NULL,NULL},
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Quit"),               NULL,ACT_EXIT           ,NULL,"Ctrl+Q"}
        };

std::vector<MenuEntry> myMenuFile(_myMenuFile, _myMenuFile + sizeof(_myMenuFile) / sizeof(_myMenuFile[0]));

static const MenuEntry _myMenuEdit[] = {
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Undo"),               NULL,ACT_Undo       ,NULL,"Ctrl+Z"},
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Redo"),               NULL,ACT_Redo       ,NULL,"Ctrl+Y"},
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Reset Edit"),         NULL,ACT_ResetSegments,NULL,NULL},
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Cut"),                NULL,ACT_Cut        ,NULL,"Ctrl+X"},
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Copy"),               NULL,ACT_Copy       ,NULL,"Ctrl+C"},
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Paste"),              NULL,ACT_Paste      ,NULL,"Ctrl+V"},
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Delete"),             NULL,ACT_Delete     ,NULL,"Delete"},
            {MENU_SEPARATOR,"-",NULL,ACT_DUMMY             ,NULL,       NULL},
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Set Marker A"),       NULL,ACT_MarkA      ,NULL,"Ctrl+PgUp"},
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Set Marker B"),       NULL,ACT_MarkB      ,NULL,"Ctrl+PgDown"},
            {MENU_SEPARATOR,"-",NULL,ACT_DUMMY             ,NULL,       NULL},
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Pr&eferences"),       NULL,ACT_PREFERENCES,NULL,NULL},
            {MENU_SEPARATOR,"-",NULL,ACT_DUMMY             ,NULL,NULL},
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Save current settings as default"), NULL,ACT_SaveAsDefault,NULL,"Ctrl+Alt+D"},
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Load saved settings"),              NULL,ACT_LoadDefault,  NULL,"Ctrl+R"}
        };

std::vector<MenuEntry> myMenuEdit(_myMenuEdit, _myMenuEdit + sizeof(_myMenuEdit) / sizeof(_myMenuEdit[0]));

static const MenuEntry _myMenuVideo[] = {
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Decoder Option"),     NULL,ACT_DecoderOption      ,NULL,NULL},
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","PostProcessing"),     NULL,ACT_SetPostProcessing  ,NULL,NULL},
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Filters"),            NULL,ACT_VIDEO_FILTERS      ,NULL,"Ctrl+Alt+F"},
        };

std::vector<MenuEntry> myMenuVideo(_myMenuVideo, _myMenuVideo + sizeof(_myMenuVideo) / sizeof(_myMenuVideo[0]));

static const MenuEntry _myMenuAudio[] = {
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Select Track"),       NULL,ACT_AUDIO_SELECT_TRACK ,NULL,NULL},
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Save audio"),         NULL,ACT_SAVE_AUDIO         ,NULL,NULL},
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Filters"),            NULL,ACT_AUDIO_FILTERS      ,NULL,NULL},
        };

std::vector<MenuEntry> myMenuAudio(_myMenuAudio, _myMenuAudio + sizeof(_myMenuAudio) / sizeof(_myMenuAudio[0]));

static const MenuEntry _myMenuHelp[] = {
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Build Option"),       NULL,ACT_BUILT_IN           ,NULL,NULL},
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Plugins"),            NULL,ACT_PLUGIN_INFO        ,NULL,NULL},
#ifdef _WIN32
			{MENU_SEPARATOR,"-",NULL,ACT_DUMMY             ,NULL,NULL},
			{MENU_SUBMENU, QT_TRANSLATE_NOOP("adm","&Advanced"), NULL, ACT_DUMMY, NULL, NULL},
			{MENU_SUBACTION, QT_TRANSLATE_NOOP("adm","Open Application &Log"), NULL, ACT_OPEN_APP_LOG, NULL, NULL},
			{MENU_SUBACTION, QT_TRANSLATE_NOOP("adm","Open Application Data &Folder"), NULL, ACT_OPEN_APP_FOLDER, NULL, NULL},
#endif
            {MENU_SEPARATOR,"-",                                        NULL,ACT_DUMMY             ,NULL,NULL},
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","About"),              NULL,ACT_ABOUT              ,NULL,NULL},
        };

std::vector<MenuEntry> myMenuHelp(_myMenuHelp, _myMenuHelp + sizeof(_myMenuHelp) / sizeof(_myMenuHelp[0]));
std::vector<MenuEntry> myMenuTool;

static const MenuEntry _myMenuGo[] = {
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Play/Stop"),           NULL,ACT_PlayAvi        ,MKICON(player_play),   "Space"},
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Previous Frame"),      NULL,ACT_PreviousFrame  ,MKICON(previous),      "Left"},
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Next Frame"),          NULL,ACT_NextFrame      ,MKICON(next),          "Right"},
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Previous Intra Frame"),NULL,ACT_PreviousKFrame ,MKICON(player_rew),    "Down"},
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Next Intra Frame"),    NULL,ACT_NextKFrame     ,MKICON(player_fwd),    "Up"},
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Previous Black Frame"),NULL,ACT_PrevBlackFrame ,MKICON(prev_black),    NULL},
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Next Black Frame"),    NULL,ACT_NextBlackFrame ,MKICON(next_black),    NULL},
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","First Frame"),         NULL,ACT_Begin          ,MKICON(player_start),  "Home"},
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Last Frame"),          NULL,ACT_End            ,MKICON(player_end),    "End"},
            {MENU_SEPARATOR,"-",               NULL,ACT_DUMMY          , NULL,NULL},
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Go To Marker A"),      NULL,ACT_GotoMarkA      ,MKICON(markA)         ,"PgUp"},
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Go To Marker B"),      NULL,ACT_GotoMarkB      ,MKICON(markB)         ,"PgDown"},
            {MENU_SEPARATOR,"-",               NULL,ACT_DUMMY          , NULL,NULL},
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Go To Time"),          NULL,ACT_GotoTime       ,NULL,                 "Ctrl+T" },
        };

std::vector<MenuEntry> myMenuGo(_myMenuGo, _myMenuGo + sizeof(_myMenuGo) / sizeof(_myMenuGo[0]));

static const MenuEntry _myMenuView[] = {
            {MENU_ACTION,"Zoom 1:4",      NULL,ACT_ZOOM_1_4 ,NULL,"4"},
            {MENU_ACTION,"Zoom 1:2",      NULL,ACT_ZOOM_1_2 ,NULL,"3"},
            {MENU_ACTION,"Zoom 1:1",      NULL,ACT_ZOOM_1_1 ,NULL,"2"},
            {MENU_ACTION,"Zoom 2:1",      NULL,ACT_ZOOM_2_1 ,NULL,"1"},
        };

std::vector<MenuEntry> myMenuView(_myMenuView, _myMenuView + sizeof(_myMenuView) / sizeof(_myMenuView[0]));

#endif
