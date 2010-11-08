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
    const char *shortCut;
}MenuEntry;
#ifdef MENU_DECLARE
MenuEntry myMenuFile[]= {
            {MENU_ACTION,"Open",    NULL,ACT_OPEN_VIDEO,       MKICON(fileopen), "Ctrl+O"},
            {MENU_ACTION,"Append",  NULL,ACT_APPEND_VIDEO     ,NULL,             "Ctrl+A"},
            {MENU_ACTION,"Save",    NULL,ACT_SAVE_VIDEO       ,MKICON(filesaveas),"Ctrl+S"},
            {MENU_ACTION,"Queue",   NULL,ACT_SAVE_QUEUE       ,NULL              ,"Ctrl+U"},
            {MENU_SUBMENU,"Save as Image",    NULL,ACT_DUMMY    ,NULL,NULL},
            {MENU_SUBACTION,"Save as BMP",    NULL,ACT_SAVE_BMP ,NULL,NULL},
            {MENU_SUBACTION,"Save as Jpeg",   NULL,ACT_SAVE_JPG ,NULL,NULL},
            {MENU_ACTION,"Close",   NULL,ACT_CLOSE          ,NULL,                "Ctrl+W"},
            {MENU_SEPARATOR,NULL,NULL,ACT_DUMMY             ,NULL,NULL},
            {MENU_SUBMENU,"Js Project",NULL,ACT_DUMMY       ,NULL,NULL},

            {MENU_SUBACTION,"Run jsProject",       NULL,ACT_RUN_JS_PROJECT       ,NULL,NULL},
            {MENU_SUBACTION,"Save as jsProject",   NULL,ACT_SAVE_JS_PROJECT      ,NULL,NULL},

            {MENU_SUBMENU,"tinyPy Project",NULL,ACT_DUMMY       ,NULL},
            {MENU_SUBACTION,"Run pyProject",       NULL,ACT_RUN_PY_PROJECT       ,NULL,NULL},
            {MENU_SUBACTION,"Save as pyProject",   NULL,ACT_SAVE_PY_PROJECT      ,NULL,NULL},
            {MENU_SEPARATOR,NULL,NULL,ACT_DUMMY             ,NULL,NULL},
            {MENU_ACTION,"Information",NULL,ACT_VIDEO_PROPERTIES,                 MKICON(info),NULL},
            {MENU_SEPARATOR,NULL,NULL,ACT_DUMMY             ,NULL,NULL},
            {MENU_ACTION,"Connect to avsproxy",NULL,ACT_AVS_PROXY,NULL},
            {MENU_SEPARATOR,NULL,NULL,ACT_DUMMY             ,NULL,NULL},
            {MENU_ACTION,"Quit",    NULL,ACT_EXIT           ,NULL,"Ctrl+Q"}
        };

MenuEntry myMenuEdit[]= {
            {MENU_ACTION,"Reset Edit",  NULL,ACT_ResetSegments,NULL,NULL},
            {MENU_ACTION,"Cut",         NULL,ACT_Cut        ,NULL,"Ctrl+X"},
            {MENU_ACTION,"Copy",        NULL,ACT_Copy       ,NULL,"Ctrl+C"},
            {MENU_ACTION,"Paste",       NULL,ACT_Paste      ,NULL,"Ctrl+V"},
            {MENU_ACTION,"Delete",      NULL,ACT_Delete     ,NULL,"Delete"},
            {MENU_SEPARATOR,NULL,NULL,ACT_DUMMY             ,NULL,NULL},
            {MENU_ACTION,"Set Marker A",NULL,ACT_MarkA      ,NULL,NULL},
            {MENU_ACTION,"Set Marker B",NULL,ACT_MarkB      ,NULL,NULL},
            {MENU_SEPARATOR,NULL,NULL,ACT_DUMMY             ,NULL,NULL},
            {MENU_ACTION,"Preferences", NULL,ACT_PREFERENCES,NULL,NULL},
        };

MenuEntry myMenuVideo[]= {
            {MENU_ACTION,"Decoder Option",  NULL,ACT_DecoderOption      ,NULL,NULL},
            {MENU_ACTION,"PostProcessing",  NULL,ACT_SetPostProcessing  ,NULL,NULL},
            {MENU_ACTION,"Filters ",        NULL,ACT_VIDEO_FILTERS      ,NULL,"Ctrl+Alt+F"},
        };

MenuEntry myMenuAudio[]= {
            {MENU_ACTION,"Select Track",    NULL,ACT_AUDIO_SELECT_TRACK ,NULL,NULL},
            {MENU_ACTION,"Save audio",      NULL,ACT_SAVE_AUDIO         ,NULL,NULL},
            {MENU_ACTION,"Filters ",        NULL,ACT_AUDIO_FILTERS      ,NULL,NULL},
        };

MenuEntry myMenuHelp[]= {
            {MENU_ACTION,"Build Option",    NULL,ACT_BUILT_IN           ,NULL,NULL},
            {MENU_ACTION,"Plugins",         NULL,ACT_PLUGIN_INFO        ,NULL,NULL},
            {MENU_ACTION,"About ",          NULL,ACT_ABOUT              ,NULL,NULL},
        };

MenuEntry myMenuTool[]= {
            {MENU_ACTION,"JavaScript Shell",NULL,ACT_JS_SHELL           ,NULL,NULL},
            {MENU_ACTION,"TinyPy Shell",    NULL,ACT_PY_SHELL           ,NULL,NULL},
        };

MenuEntry myMenuGo[]= {
            {MENU_ACTION,"Play/Stop",           NULL,ACT_PlayAvi        ,MKICON(player_play),   "Space"},
            {MENU_ACTION,"Previous Frame",      NULL,ACT_PreviousFrame  ,MKICON(previous),      "Left"},
            {MENU_ACTION,"Next Frame",          NULL,ACT_NextFrame      ,MKICON(next),          "Right"},
            {MENU_ACTION,"Previous Intra Frame",NULL,ACT_PreviousKFrame ,MKICON(player_rew),    "Down"},
            {MENU_ACTION,"Next Intra Frame",    NULL,ACT_NextKFrame     ,MKICON(player_fwd),    "Up"},
            {MENU_ACTION,"Previous Black Frame",NULL,ACT_PrevBlackFrame ,MKICON(prev_black),    NULL},
            {MENU_ACTION,"Next Black Frame",    NULL,ACT_NextBlackFrame ,MKICON(next_black),    NULL},
            {MENU_ACTION,"First Frame",         NULL,ACT_Begin          ,MKICON(player_start),  "Home"},
            {MENU_ACTION,"Last Frame",          NULL,ACT_End            ,MKICON(player_end),    "End"},
            {MENU_SEPARATOR,NULL,               NULL,ACT_DUMMY          ,NULL,NULL},
            {MENU_ACTION,"Go To Marker A",      NULL,ACT_GotoMarkA      ,MKICON(markA)         ,"PgUp"},
            {MENU_ACTION,"Go To Marker B",      NULL,ACT_GotoMarkB      ,MKICON(markB)         ,"PgDown"},
            {MENU_SEPARATOR,NULL,               NULL,ACT_DUMMY          ,NULL,NULL},
            {MENU_ACTION,"Go To Time",          NULL,ACT_GotoTime       ,NULL,                 "Ctrl+T" },
        };

MenuEntry myMenuView[]= {
            {MENU_ACTION,"Zoom 1:4",      NULL,ACT_ZOOM_1_4 ,NULL,"4"},
            {MENU_ACTION,"Zoom 1:2",      NULL,ACT_ZOOM_1_2 ,NULL,"3"},
            {MENU_ACTION,"Zoom 1:1",      NULL,ACT_ZOOM_1_1 ,NULL,"2"},
            {MENU_ACTION,"Zoom 2:1",      NULL,ACT_ZOOM_2_1 ,NULL,"1"},
        };
#endif
#endif
