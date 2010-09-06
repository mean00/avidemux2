/**
    \file myOwnMenus
    
*/
#ifndef MY_OWN_MENUS
#define MY_OWN_MENUS
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
    const char *text;
    void       *cookie;
    Action     event;
    const char *icon; 
}MenuEntry;
MenuEntry myMenuFile[]= {
            {MENU_ACTION,"Open",    NULL,ACT_OPEN_VIDEO,       MKICON(fileopen)},
            {MENU_ACTION,"Append",  NULL,ACT_APPEND_VIDEO     ,NULL},
            {MENU_ACTION,"Save",    NULL,ACT_SAVE_VIDEO       ,MKICON(filesaveas)},
            {MENU_ACTION,"Close",   NULL,ACT_CLOSE          ,NULL},
            {MENU_SEPARATOR,NULL,NULL,ACT_DUMMY             ,NULL},
            {MENU_SUBMENU,"Js Project",NULL,ACT_DUMMY       ,NULL},

            {MENU_SUBACTION,"Run jsProject",       NULL,ACT_RUN_JS_PROJECT         ,NULL},
            {MENU_SUBACTION,"Save as jsProject",   NULL,ACT_SAVE_JS_PROJECT         ,NULL},

            {MENU_SUBMENU,"tinyPy Project",NULL,ACT_DUMMY       ,NULL},
            {MENU_SUBACTION,"Run pyProject",       NULL,ACT_RUN_PY_PROJECT         ,NULL},
            {MENU_SUBACTION,"Save as pyProject",   NULL,ACT_SAVE_PY_PROJECT         ,NULL},
            {MENU_SEPARATOR,NULL,NULL,ACT_DUMMY             ,NULL},
            {MENU_ACTION,"Information",NULL,ACT_VIDEO_PROPERTIES,MKICON(info)},
            {MENU_SEPARATOR,NULL,NULL,ACT_DUMMY             ,NULL},
            {MENU_ACTION,"Connect to avsproxy",NULL,ACT_AVS_PROXY,NULL},
            {MENU_SEPARATOR,NULL,NULL,ACT_DUMMY             ,NULL},
            {MENU_ACTION,"Quit",    NULL,ACT_EXIT           ,NULL}
        };

MenuEntry myMenuEdit[]= {
            {MENU_ACTION,"Reset Edit",  NULL,ACT_ResetSegments,NULL},
            {MENU_ACTION,"Cut",         NULL,ACT_Cut        ,NULL},
            {MENU_ACTION,"Copy",        NULL,ACT_Copy       ,NULL},
            {MENU_ACTION,"Paste",       NULL,ACT_Paste      ,NULL},
            {MENU_ACTION,"Delete",      NULL,ACT_Delete     ,NULL},
            {MENU_SEPARATOR,NULL,NULL,ACT_DUMMY             ,NULL},
            {MENU_ACTION,"Set Marker A",NULL,ACT_MarkA      ,NULL},
            {MENU_ACTION,"Set Marker B",NULL,ACT_MarkB      ,NULL},
            {MENU_SEPARATOR,NULL,NULL,ACT_DUMMY             ,NULL},
            {MENU_ACTION,"Preferences", NULL,ACT_PREFERENCES,NULL},
        };

MenuEntry myMenuVideo[]= {
            {MENU_ACTION,"Decoder Option",  NULL,ACT_DecoderOption      ,NULL},
            {MENU_ACTION,"PostProcessing",  NULL,ACT_SetPostProcessing  ,NULL},
            {MENU_ACTION,"Filters ",        NULL,ACT_VIDEO_FILTERS      ,NULL},
        };

MenuEntry myMenuAudio[]= {
            {MENU_ACTION,"Select Track",    NULL,ACT_AUDIO_SELECT_TRACK ,NULL},
            {MENU_ACTION,"Save audio",      NULL,ACT_SAVE_AUDIO         ,NULL},
            {MENU_ACTION,"Filters ",        NULL,ACT_AUDIO_FILTERS      ,NULL},
        };

MenuEntry myMenuHelp[]= {
            {MENU_ACTION,"Build Option",    NULL,ACT_BUILT_IN           ,NULL},
            {MENU_ACTION,"Plugins",         NULL,ACT_PLUGIN_INFO        ,NULL},
            {MENU_ACTION,"About ",          NULL,ACT_ABOUT              ,NULL},
        };

MenuEntry myMenuTool[]= {
            {MENU_ACTION,"JavaScript Shell",NULL,ACT_JS_SHELL           ,NULL},
            {MENU_ACTION,"TinyPy Shell",    NULL,ACT_PY_SHELL           ,NULL},
        };

MenuEntry myMenuGo[]= {
            {MENU_ACTION,"Play/Stop",           NULL,ACT_JS_SHELL,       MKICON(player_play)},
            {MENU_ACTION,"Previous Frame",      NULL,ACT_PY_SHELL       ,MKICON(previous)},
            {MENU_ACTION,"Next Frame",          NULL,ACT_PY_SHELL       ,MKICON(next)},
            {MENU_ACTION,"Previous Intra Frame",NULL,ACT_PY_SHELL       ,MKICON(player_rew)},
            {MENU_ACTION,"Next Intra Frame",    NULL,ACT_PY_SHELL       ,MKICON(player_fwd)},
            {MENU_ACTION,"Previous Black Frame",NULL,ACT_PY_SHELL       ,MKICON(prev_black)},
            {MENU_ACTION,"Next Black Frame",    NULL,ACT_PY_SHELL       ,MKICON(next_black)},
            {MENU_ACTION,"First Frame",          NULL,ACT_PY_SHELL      ,MKICON(player_start)},
            {MENU_ACTION,"Last Frame",          NULL,ACT_PY_SHELL       ,MKICON(player_end)},
            {MENU_SEPARATOR,NULL,NULL,ACT_DUMMY,NULL},
            {MENU_ACTION,"Go To Marker A",      NULL,ACT_PY_SHELL       ,MKICON(markA)},
            {MENU_ACTION,"Go To Marker B",      NULL,ACT_PY_SHELL       ,MKICON(markB)},
            {MENU_SEPARATOR,NULL,NULL,ACT_DUMMY,NULL},
            {MENU_ACTION,"Go To Time",          NULL,ACT_PY_SHELL       ,NULL},
        };

MenuEntry myMenuView[]= {
            {MENU_ACTION,"Zoom 1:4",      NULL,ACT_ZOOM_1_4 ,NULL},
            {MENU_ACTION,"Zoom 1:2",      NULL,ACT_ZOOM_1_2 ,NULL},
            {MENU_ACTION,"Zoom 1:1",      NULL,ACT_ZOOM_1_1 ,NULL},
            {MENU_ACTION,"Zoom 2:1",      NULL,ACT_ZOOM_2_1 ,NULL},
        };
#endif
