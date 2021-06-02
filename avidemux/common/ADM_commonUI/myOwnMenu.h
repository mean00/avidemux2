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
    bool       translated; // If true, circumvent translator in buildMenu, the string is already translated.

}MenuEntry;

#ifdef MENU_DECLARE
static const MenuEntry _myMenuFile[] = {
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Open"),               NULL,ACT_OPEN_VIDEO,    MKICON(fileopen),"Ctrl+O",0},
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Append"),             NULL,ACT_APPEND_VIDEO,  NULL,"Ctrl+A",0},
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Save"),               NULL,ACT_SAVE_VIDEO,    MKICON(filesaveas),"Ctrl+S",0},
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Queue"),              NULL,ACT_SAVE_QUEUE,    NULL,"Ctrl+U",0},
            {MENU_SUBMENU,QT_TRANSLATE_NOOP("adm","Save as Image"),     NULL,ACT_DUMMY,         NULL,NULL,0},
            {MENU_SUBACTION,QT_TRANSLATE_NOOP("adm","Save as BMP"),     NULL,ACT_SAVE_BMP,      NULL,"Ctrl+M",0},
            {MENU_SUBACTION,QT_TRANSLATE_NOOP("adm","Save as PNG"),     NULL,ACT_SAVE_PNG,      NULL,"Ctrl+P",0},
            {MENU_SUBACTION,QT_TRANSLATE_NOOP("adm","Save as JPEG"),    NULL,ACT_SAVE_JPG,      NULL,"Ctrl+E",0},
            {MENU_SUBACTION,QT_TRANSLATE_NOOP("adm","Save Selection as JPEG"),NULL,ACT_SAVE_BUNCH_OF_JPG,NULL,NULL,0},
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Close"),              NULL,ACT_CLOSE,         NULL,"Ctrl+W",0},
            {MENU_SEPARATOR,"-",NULL,ACT_DUMMY,NULL,NULL,1},
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Information"),        NULL,ACT_VIDEO_PROPERTIES, MKICON(info),"Alt+Return",0},
            {MENU_SEPARATOR,"-",NULL,ACT_DUMMY,NULL,NULL,1},
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Connect to avsproxy"),NULL,ACT_AVS_PROXY,NULL,0},
            {MENU_SEPARATOR,"-",NULL,ACT_DUMMY,NULL,NULL,1},
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Quit"),               NULL,ACT_EXIT,           NULL,"Ctrl+Q",0}
        };

std::vector<MenuEntry> myMenuFile(_myMenuFile, _myMenuFile + sizeof(_myMenuFile) / sizeof(_myMenuFile[0]));

static const MenuEntry _myMenuRecent[] = {
            {MENU_SEPARATOR,"-",NULL,ACT_DUMMY,NULL,NULL,1},
#ifdef __APPLE__
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Clear recent items"), NULL,ACT_CLEAR_RECENT,  NULL,"Ctrl+Shift+Backspace",0}
#else
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Clear recent items"), NULL,ACT_CLEAR_RECENT,  NULL,"Ctrl+Shift+Delete",0}
#endif
        };

std::vector<MenuEntry> myMenuRecent(_myMenuRecent, _myMenuRecent + sizeof(_myMenuRecent) / sizeof(_myMenuRecent[0]));

static const MenuEntry _myMenuEdit[] = {
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Undo"),               NULL,ACT_Undo,          NULL,"Ctrl+Z",0},
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Redo"),               NULL,ACT_Redo,          NULL,"Ctrl+Y",0},
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Reset Edit"),         NULL,ACT_ResetSegments, NULL,NULL,0},
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Cut"),                NULL,ACT_Cut,           NULL,"Ctrl+X",0},
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Copy"),               NULL,ACT_Copy,          NULL,"Ctrl+C",0},
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Paste"),              NULL,ACT_Paste,         NULL,"Ctrl+V",0},
#ifdef __APPLE__
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Delete"),             NULL,ACT_Delete,        NULL,"Backspace",0},
#else
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Delete"),             NULL,ACT_Delete,        NULL,"Delete",0},
#endif
            {MENU_SEPARATOR,"-",NULL,ACT_DUMMY,NULL,NULL,1},
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Set Marker A"),       NULL,ACT_MarkA,         NULL,"Ctrl+PgUp",0},
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Set Marker B"),       NULL,ACT_MarkB,         NULL,"Ctrl+PgDown",0},
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Reset Markers"),      NULL,ACT_ResetMarkers,  NULL,"Ctrl+Home",0},
            {MENU_SEPARATOR,"-",NULL,ACT_DUMMY,NULL,NULL,1},
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Pr&eferences"),       NULL,ACT_PREFERENCES,   NULL,NULL,0},
            {MENU_SEPARATOR,"-",NULL,ACT_DUMMY,NULL,NULL,1},
#ifdef __APPLE__
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Save current settings as default"), NULL,ACT_SaveAsDefault,NULL,"Ctrl+Shift+D",0},
#else
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Save current settings as default"), NULL,ACT_SaveAsDefault,NULL,"Ctrl+Alt+D",0},
#endif
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Load saved settings"),              NULL,ACT_LoadDefault,  NULL,"Ctrl+R",0}
        };

std::vector<MenuEntry> myMenuEdit(_myMenuEdit, _myMenuEdit + sizeof(_myMenuEdit) / sizeof(_myMenuEdit[0]));

static const MenuEntry _myMenuVideo[] = {
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Decoder Option"),     NULL,ACT_DecoderOption,     NULL,NULL,0},
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","PostProcessing"),     NULL,ACT_SetPostProcessing, NULL,NULL,0},
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Filters"),            NULL,ACT_VIDEO_FILTERS,     NULL,"Ctrl+Alt+F",0},
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Play filtered"),      NULL,ACT_PreviewChanged,    NULL,NULL,0}
        };

std::vector<MenuEntry> myMenuVideo(_myMenuVideo, _myMenuVideo + sizeof(_myMenuVideo) / sizeof(_myMenuVideo[0]));

static const MenuEntry _myMenuAudio[] = {
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Select Track"),       NULL,ACT_AUDIO_SELECT_TRACK,NULL,NULL,0},
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Save audio"),         NULL,ACT_SAVE_AUDIO,        NULL,NULL,0},
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Filters"),            NULL,ACT_AUDIO_FILTERS,     NULL,NULL,0},
        };

std::vector<MenuEntry> myMenuAudio(_myMenuAudio, _myMenuAudio + sizeof(_myMenuAudio) / sizeof(_myMenuAudio[0]));

static const MenuEntry _myMenuHelp[] = {
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Build Option"),       NULL,ACT_BUILT_IN,          NULL,NULL,0},
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Plugins"),            NULL,ACT_PLUGIN_INFO,       NULL,NULL,0},
#ifdef _WIN32
            {MENU_SEPARATOR,"-",NULL,ACT_DUMMY,NULL,NULL,1},
            {MENU_SUBMENU,QT_TRANSLATE_NOOP("adm","&Advanced"),         NULL,ACT_DUMMY,             NULL,NULL,0},
            {MENU_SUBACTION,QT_TRANSLATE_NOOP("adm","Open Application &Log"),NULL,ACT_OPEN_APP_LOG, NULL, NULL,0},
            {MENU_SUBACTION,QT_TRANSLATE_NOOP("adm","Open Application Data &Folder"),NULL,ACT_OPEN_APP_FOLDER,NULL,NULL,0},
#endif
            {MENU_SEPARATOR,"-",NULL,ACT_DUMMY,NULL,NULL,1},
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","About"),              NULL,ACT_ABOUT,             NULL,NULL,0}
        };

std::vector<MenuEntry> myMenuHelp(_myMenuHelp, _myMenuHelp + sizeof(_myMenuHelp) / sizeof(_myMenuHelp[0]));
std::vector<MenuEntry> myMenuTool;

static const MenuEntry _myMenuGo[] = {
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Play/Stop"),           NULL,ACT_PlayAvi,         MKICON(player_play),   "Space",0},
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Previous Frame"),      NULL,ACT_PreviousFrame,   MKICON(back),          "Left",0},
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Next Frame"),          NULL,ACT_NextFrame,       MKICON(forward),       "Right",0},
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Previous Intra Frame"),NULL,ACT_PreviousKFrame,  MKICON(player_rew),    "Down",0},
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Next Intra Frame"),    NULL,ACT_NextKFrame,      MKICON(player_fwd),    "Up",0},
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Previous Cut Point"),  NULL,ACT_PrevCutPoint,    MKICON(prev_cut),      NULL,0},
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Next Cut Point"),      NULL,ACT_NextCutPoint,    MKICON(next_cut),      NULL,0},
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Previous Black Frame"),NULL,ACT_PrevBlackFrame,  MKICON(prev_black),    NULL,0},
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Next Black Frame"),    NULL,ACT_NextBlackFrame,  MKICON(next_black),    NULL,0},
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","First Frame"),         NULL,ACT_Begin,           MKICON(player_start),  "Home",0},
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Last Frame"),          NULL,ACT_End,             MKICON(player_end),    "End",0},
            {MENU_SEPARATOR,"-",NULL,ACT_DUMMY,NULL,NULL,1},
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Go To Marker A"),      NULL,ACT_GotoMarkA,       MKICON(markA),         "PgUp",0},
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Go To Marker B"),      NULL,ACT_GotoMarkB,       MKICON(markB),         "PgDown",0},
            {MENU_SEPARATOR,"-",NULL,ACT_DUMMY, NULL,NULL,1},
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Go To Time"),          NULL,ACT_GotoTime,        NULL,                  "Ctrl+T",0}
        };

std::vector<MenuEntry> myMenuGo(_myMenuGo, _myMenuGo + sizeof(_myMenuGo) / sizeof(_myMenuGo[0]));

static const MenuEntry _myMenuView[] = {
            {MENU_SEPARATOR,"-",NULL,ACT_DUMMY,NULL,NULL,1},
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Zoom 1:4"),       NULL,ACT_ZOOM_1_4,    NULL,"4",0},
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Zoom 1:2"),       NULL,ACT_ZOOM_1_2,    NULL,"3",0},
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Zoom 1:1"),       NULL,ACT_ZOOM_1_1,    NULL,"2",0},
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Zoom 2:1"),       NULL,ACT_ZOOM_2_1,    NULL,"1",0},
            {MENU_SEPARATOR,"-",NULL,ACT_DUMMY,NULL,NULL,1},
            {MENU_ACTION,QT_TRANSLATE_NOOP("adm","Fit to window"),  NULL,ACT_ZOOM_FIT_IN, NULL,"0",0},
        };

std::vector<MenuEntry> myMenuView(_myMenuView, _myMenuView + sizeof(_myMenuView) / sizeof(_myMenuView[0]));

#endif
