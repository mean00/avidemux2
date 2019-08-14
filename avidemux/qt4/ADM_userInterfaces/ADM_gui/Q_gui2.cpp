/***************************************************************************
    copyright            : (C) 2001 by mean
    email                : fixounet@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QtCore/QFileInfo>
#include <QtCore/QMimeData>
#include <QtCore/QUrl>
#include <QKeyEvent>
#include <QResizeEvent>
#include <QGraphicsView>
#include <QtCore/QDir>
#include <QMessageBox>
#include <QClipboard>
#include "ADM_cpp.h"
#define MENU_DECLARE
#include "Q_gui2.h"
#include "ADM_default.h"
#include "ADM_toolkitQt.h"

#include "DIA_fileSel.h"
#include "ADM_vidMisc.h"
#include "ADM_last.h"
#include "prefs.h"
#include "avi_vars.h"

#include "ADM_render/GUI_renderInternal.h"
#include "ADM_render/GUI_render.h"
#include "ADM_coreVideoEncoderInternal.h"
#include "ADM_muxerProto.h"
#include "T_vumeter.h"
#include "DIA_coreToolkit.h"
#include "GUI_ui.h"
#include "config.h"
#include "ADM_preview.h"
#include "DIA_defaultAskAvisynthPort.hxx"
#include "ADM_systemTrayProgress.h"
#include "../../ADM_update/include/ADM_update.h"
using namespace std;

#define ADM_SLIDER_REFRESH_PERIOD 500
#define ADM_LARGE_SCALE     (10LL*1000LL)
#define ADM_SCALE_INCREMENT (ADM_LARGE_SCALE/100LL)
// if the dimensions of an autozoomed video are smaller than the available space,
// don't adjust zoom unless the window has been enlarged beyond a threshold
#define RESIZE_THRESHOLD 20

#if defined(USE_SDL) && ( !defined(_WIN32) && !defined(__APPLE__))
    #define SDL_ON_LINUX
#endif

#ifdef USE_OPENGL
extern bool ADM_glHasActiveTexture(void);
void UI_Qt4InitGl(void);
void UI_Qt4CleanGl(void);
bool  openGLStarted=false;
#endif

MainWindow *MainWindow::mainWindowSingleton=NULL;

extern int global_argc;
extern char **global_argv;

extern int automation(void );
extern void sendAction(Action a);
extern int encoderGetEncoderCount (void);
extern const char *encoderGetIndexedName (uint32_t i);
uint32_t audioEncoderGetNumberOfEncoders(void);
const char  *audioEncoderGetDisplayName(int i);
extern void checkCrashFile(void);
extern bool A_checkSavedSession(bool load);
extern void UI_QT4VideoWidget(QFrame *frame);
extern void loadTranslator(void);
extern void initTranslator(void);
extern void destroyTranslator(void);
extern ADM_RENDER_TYPE UI_getPreferredRender(void);
extern int A_openVideo(const char *name);
extern int A_appendVideo(const char *name);

int SliderIsShifted=0;
static void setupMenus(void);
static int shiftKeyHeld=0;
static ADM_mwNavSlider *slider=NULL;
static int _upd_in_progres=0;
static int currentFps = 0;
static int frameCount = 0;
static int currentFrame = 0;
bool     ADM_ve6_getEncoderInfo(int filter, const char **name, uint32_t *major,uint32_t *minor,uint32_t *patch);
uint32_t ADM_ve6_getNbEncoders(void);
void UI_refreshCustomMenu(void);
admUITaskBarProgress *QuiTaskBarProgress;
extern admUITaskBarProgress *createADMTaskBarProgress();
QWidget *QuiMainWindows=NULL;
QWidget *VuMeter=NULL;
QGraphicsView *drawWindow=NULL;

extern void saveCrashProject(void);
extern uint8_t AVDM_setVolume(int volume);
extern bool AVDM_hasVolumeControl(void);
extern bool ADM_QPreviewCleanup(void);
extern void vdpauCleanup();
extern bool A_loadDefaultSettings(void);;

extern void ADM_ExitCleanup(void);

static bool uiRunning=false;
static bool uiIsMaximized=false;

static bool needsResizing=false;

#define WIDGET(x)  (((MainWindow *)QuiMainWindows)->ui.x)

#define CONNECT(object,zzz) connect( (ui.object),SIGNAL(triggered()),this,SLOT(buttonPressed()));
#define CONNECT_TB(object,zzz) connect( (ui.object),SIGNAL(clicked(bool)),this,SLOT(toolButtonPressed(bool)));
#define DECLARE_VAR(object,signal_name) {#object,signal_name},

#include "translation_table.h"
/*
    Declare the table converting widget name to our internal signal
*/
typedef struct
{
    const char *name;
    Action     action;
}adm_qt4_translation;

const adm_qt4_translation myTranslationTable[]=
{
#define PROCESS DECLARE_VAR
    LIST_OF_BUTTONS
#undef PROCESS
};
static Action searchTranslationTable(const char *name);
#define SIZEOF_MY_TRANSLATION sizeof(myTranslationTable)/sizeof(adm_qt4_translation)

int UI_readCurTime(uint16_t &hh, uint16_t &mm, uint16_t &ss, uint16_t &ms);
void UI_updateFrameCount(uint32_t curFrame);
void UI_updateTimeCount(uint32_t curFrame,uint32_t fps);

class FileDropEvent : public QEvent
{
public:
    QList<QUrl> files;

    FileDropEvent(QList<QUrl> files) : QEvent(QEvent::User)
    {
        this->files = files;
    }
};

void MainWindow::comboChanged(int z)
{
    QObject *obj = sender();

    if (obj == ui.comboBoxVideo)
        sendAction (ACT_VIDEO_CODEC_CHANGED) ;
    else if (obj == ui.comboBoxAudio)
        sendAction (ACT_AUDIO_CODEC_CHANGED) ;
    setMenuItemsEnabledState();
}
/**
 * \fn sliderValueChanged
 * @param u
 */
void MainWindow::sliderValueChanged(int u)
{
  
    if(_upd_in_progres)
      return;
    if(refreshCapEnabled)
        switch(dragState)
        { 
            default:
            case dragState_Normal:
                sendAction(ACT_Scale);
                break;
            case dragState_Active:
                dragTimer.stop();
                dragTimer.start(refreshCapValue);
                dragState=dragState_HoldOff;
                break;            
            case dragState_HoldOff:
              break;            
          }
    else
         sendAction(ACT_Scale);
}

/**
 * 
 * @param version
 * @param date
 * @param url
 */
void MainWindow::updateAvailableSlot(int version, std::string date, std::string url)
{
    QMessageBox msgBox;
    int a,b,c;
    a=version/10000;
    b=(version-a*10000)/100;
    c=version%100;
    QString versionString=QString("%1.%2.%3").arg(a).arg(b).arg(c);
    QString msg=QT_TRANSLATE_NOOP("qgui2","<b>New version available</b><br> Version %1<br>Released on %2.<br>You can download it here<br> <a href='%3'>%3</a><br><br><small> You can disable autoupdate in preferences.</small>");
    msg=msg.arg(versionString,date.c_str(),url.c_str(),url.c_str());
    msgBox.setText(msg);
    msgBox.setTextFormat(Qt::RichText);
    msgBox.exec();  
}
/**
 * \fn dragTimerTimeout
 */
void MainWindow::dragTimerTimeout(void)
{
  ADM_info("Drag timeout\n");
  switch(dragState)
    {
        default:
        case dragState_Normal:           
        case dragState_Active:           
            break;            
        case dragState_HoldOff:
            dragState=dragState_Active;
            sendAction(ACT_Scale);
            break;
    }
}
/**
 * \fn sliderMoved
 */
void MainWindow::sliderMoved(int value)
{
    //ADM_info("Moved\n");
    SliderIsShifted = shiftKeyHeld;
}
/**
 * \fn sliderReleased
 */
void MainWindow::sliderReleased(void)
{
  //ADM_info("Released\n");
    SliderIsShifted = 0;
    dragTimer.stop();
    dragState=dragState_Normal;
    sendAction(ACT_Scale);
}
/**
 * \fn sliderPressed
 */
void MainWindow::sliderPressed(void)
{
    if(playing)
        sendAction(ACT_PlayAvi); // stop playback
    dragTimer.stop();
    dragState=dragState_Active;
//  ADM_info("Pressed\n");
}
/**
 * \fn sliderWheel
 */
void MainWindow::sliderWheel(int way)
{ 
    if(way>0)
    {
        sendAction(ACT_NextKFrame);
        return;
    }
    if(way<0)
        sendAction(ACT_PreviousKFrame);
    
}

void MainWindow::thumbSlider_valueEmitted(int value)
{
        if (value > 0)
                nextIntraFrame();
        else
                previousIntraFrame();
}

void MainWindow::volumeChange( int u )
{
    if (_upd_in_progres || !ui.toolButtonAudioToggle->isChecked())
        return;

    _upd_in_progres++;

    int vol = ui.horizontalSlider_2->value();

    AVDM_setVolume(vol);
    _upd_in_progres--;
}

void MainWindow::audioToggled(bool checked)
{
    if (checked)
        AVDM_setVolume(ui.horizontalSlider_2->value());
    else
        AVDM_setVolume(0);
}

void MainWindow::previewModeChangedFromMenu(bool flop)
{
    ui.toolBar->actions().at(5)->setChecked(flop);
    sendAction(ACT_PreviewChanged);
}

void MainWindow::previewModeChangedFromToolbar(bool flop)
{
    ui.menuVideo->actions().at(3)->setChecked(flop);
    sendAction(ACT_PreviewChanged);
}

void MainWindow::timeChangeFinished(void)
{
    this->setFocus(Qt::OtherFocusReason);
}

void MainWindow::currentFrameChanged(void)
{
//    sendAction(ACT_JumpToFrame);

    this->setFocus(Qt::OtherFocusReason);
}

void MainWindow::currentTimeChanged(void)
{
    sendAction(ACT_GotoTime);

    this->setFocus(Qt::OtherFocusReason);
}

/**
    \fn currentTimeToClipboard
*/
void MainWindow::currentTimeToClipboard(void)
{
    QString ct = QString();
    ct = ui.currentTime->text();
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->clear();
    clipboard->setText(ct);
}

/**
    \fn setRefreshCap
*/
void MainWindow::setRefreshCap(void)
{
    refreshCapEnabled=false;
    refreshCapValue=0;
    prefs->get(FEATURES_CAP_REFRESH_ENABLED,&refreshCapEnabled);
    prefs->get(FEATURES_CAP_REFRESH_VALUE,&refreshCapValue);
}

/**
    \fn ctor
*/
MainWindow::MainWindow(const vector<IScriptEngine*>& scriptEngines) : _scriptEngines(scriptEngines), QMainWindow()
{
    MainWindow::mainWindowSingleton=this;
    qtRegisterDialog(this);
    ui.setupUi(this);
    dragState=dragState_Normal;
    recentFiles = NULL;
    recentProjects = NULL;

#if defined(__APPLE__) && defined(USE_SDL)
    //ui.actionAbout_avidemux->setMenuRole(QAction::NoRole);
    //ui.actionPreferences->setMenuRole(QAction::NoRole);
    //ui.actionQuit->setMenuRole(QAction::NoRole);
#endif
        //
        connect( this,SIGNAL(actionSignal(Action )),this,SLOT(actionSlot(Action )));
        //
        connect(this, SIGNAL(updateAvailable(int,std::string,std::string)),this,SLOT(updateAvailableSlot(int,std::string,std::string)));

    /*
    Connect our button to buttonPressed
    */
#define PROCESS CONNECT_TB
    LIST_OF_BUTTONS
#undef PROCESS

    //ACT_VideoCodecChanged
    connect( ui.comboBoxVideo,SIGNAL(activated(int)),this,SLOT(comboChanged(int)));
    connect( ui.comboBoxAudio,SIGNAL(activated(int)),this,SLOT(comboChanged(int)));

    // Slider
    slider=ui.horizontalSlider;
        ADM_mwNavSlider *qslider=(ADM_mwNavSlider *)slider;
    slider->setMinimum(0);
    slider->setMaximum(ADM_LARGE_SCALE);
#if !(defined(__APPLE__) && QT_VERSION >= QT_VERSION_CHECK(5,10,0))
        slider->setTickInterval(ADM_SCALE_INCREMENT);
        slider->setTickPosition(QSlider::TicksBothSides);
#endif
    connect( slider,SIGNAL(valueChanged(int)),this,SLOT(sliderValueChanged(int)));
    connect( slider,SIGNAL(sliderMoved(int)),this,SLOT(sliderMoved(int)));
    connect( slider,SIGNAL(sliderReleased()),this,SLOT(sliderReleased()));
        connect( slider,SIGNAL(sliderPressed()),this,SLOT(sliderPressed()));
        connect( qslider,SIGNAL(sliderAction(int)),this,SLOT(sliderWheel(int)));
        
        connect( &dragTimer, SIGNAL(timeout()), this, SLOT(dragTimerTimeout()));
    

   // Thumb slider
        ui.sliderPlaceHolder->installEventFilter(this);
        thumbSlider = new ThumbSlider(ui.sliderPlaceHolder);
        connect(thumbSlider, SIGNAL(valueEmitted(int)), this, SLOT(thumbSlider_valueEmitted(int)));

    // Volume slider
    QSlider *volSlider=ui.horizontalSlider_2;
    volSlider->setMinimum(0);
    volSlider->setMaximum(100);
    connect(volSlider,SIGNAL(valueChanged(int)),this,SLOT(volumeChange(int)));
    connect(ui.toolButtonAudioToggle,SIGNAL(clicked(bool)),this,SLOT(audioToggled(bool)));

    // default state
    bool b=0;
    ui.pushButtonVideoConf->setEnabled(b);
    ui.pushButtonVideoFilter->setEnabled(b);
    ui.pushButtonAudioConf->setEnabled(b);
    ui.pushButtonAudioFilter->setEnabled(b);

    /* Time Shift */
    connect(ui.checkBox_TimeShift,SIGNAL(stateChanged(int)),this,SLOT(checkChanged(int)));
    connect(ui.spinBox_TimeValue,SIGNAL(valueChanged(int)),this,SLOT(timeChanged(int)));
    connect(ui.spinBox_TimeValue, SIGNAL(editingFinished()), this, SLOT(timeChangeFinished()));

    QRegExp timeRegExp("^[0-9]{2}:[0-5][0-9]:[0-5][0-9]\\.[0-9]{3}$");
    QRegExpValidator *timeValidator = new QRegExpValidator(timeRegExp, this);
    ui.currentTime->setValidator(timeValidator);
    ui.currentTime->setInputMask("99:99:99.999");
    // set the size of the current time display to fit the content
    QString text=ui.currentTime->text();
    QFontMetrics fm=ui.currentTime->fontMetrics();
    int currentTimeWidth=fm.boundingRect(text).width()+20;
    int currentTimeHeight=ui.currentTime->height();
    ui.currentTime->setFixedSize(currentTimeWidth, currentTimeHeight);
    ui.currentTime->adjustSize();

    //connect(ui.currentTime, SIGNAL(editingFinished()), this, SLOT(currentTimeChanged()));

    // Build file,... menu
    addScriptEnginesToFileMenu(myMenuFile);
    addScriptShellsToToolsMenu(myMenuTool);

    QString rFiles=QString::fromUtf8(QT_TRANSLATE_NOOP("qgui2","Recent Files"));
    QString rProjects=QString::fromUtf8(QT_TRANSLATE_NOOP("qgui2","Recent Projects"));
    
    recentFiles=new QMenu(rFiles, this);
    recentProjects=new QMenu(rProjects, this);
    ui.menuRecent->addMenu(recentFiles);
    ui.menuRecent->addMenu(recentProjects);
    connect(this->recentFiles, SIGNAL(triggered(QAction*)), this, SLOT(searchRecentFiles(QAction*)));
    connect(this->recentProjects, SIGNAL(triggered(QAction*)), this, SLOT(searchRecentProjects(QAction*)));

    addSessionRestoreToRecentMenu(myMenuRecent);

    buildMyMenu();
    buildCustomMenu(); // action lists are populated (i.e. buildActionLists() called) within buildCustomMenu()
    buildButtonLists();

#define AUTOREPEAT_TOOLBUTTON(x) ui.x->setAutoRepeat(true); ui.x->setAutoRepeatDelay(500); ui.x->setAutoRepeatInterval(100);
    AUTOREPEAT_TOOLBUTTON(toolButtonPreviousFrame)
    AUTOREPEAT_TOOLBUTTON(toolButtonNextFrame)
    AUTOREPEAT_TOOLBUTTON(toolButtonPreviousIntraFrame)
    AUTOREPEAT_TOOLBUTTON(toolButtonNextIntraFrame)

    // Crash in some cases addScriptReferencesToHelpMenu();
    connect(ui.menuVideo->actions().at(3),SIGNAL(toggled(bool)),this,SLOT(previewModeChangedFromMenu(bool)));
    connect(ui.toolBar->actions().at(5),SIGNAL(toggled(bool)),this,SLOT(previewModeChangedFromToolbar(bool)));

    this->installEventFilter(this);
    slider->installEventFilter(this);

    //ui.currentTime->installEventFilter(this);

    this->setFocus(Qt::OtherFocusReason);

    setAcceptDrops(true);
#ifndef __APPLE__
    setWindowIcon(QIcon(MKICON(avidemux-icon)));
#else
    setWindowIcon(QIcon(MKOSXICON(avidemux)));
#endif

    // Hook also the toolbar
    connect(ui.toolBar,  SIGNAL(actionTriggered ( QAction *)),this,SLOT(searchToolBar(QAction *)));
    //connect(ui.toolBar_2,SIGNAL(actionTriggered ( QAction *)),this,SLOT(searchToolBar(QAction *)));

    QWidget* dummy0 = new QWidget();
    QWidget* dummy1 = new QWidget();
    QWidget* dummy2 = new QWidget();
    QWidget* dummy3 = new QWidget();
    QWidget* dummy4 = new QWidget();

    ui.codecWidget->setTitleBarWidget(dummy0);
    ui.navigationWidget->setTitleBarWidget(dummy1);
    ui.selectionWidget->setTitleBarWidget(dummy2);
    ui.volumeWidget->setTitleBarWidget(dummy3);
    ui.audioMetreWidget->setTitleBarWidget(dummy4);

    widgetsUpdateTooltips();

    this->adjustSize();

    threshold = RESIZE_THRESHOLD;
    actZoomCalled = false;
    ignoreResizeEvent = false;
    blockResizing = false;
    blockZoomChanges = true;

    QuiTaskBarProgress=createADMTaskBarProgress();
}
/**
    \fn searchToolBar
*/
typedef struct
{
    const char *name;
    Action event;
}toolBarTranslate;

toolBarTranslate toolbar[]=
{
{"actionOpen",              ACT_OPEN_VIDEO},
{"actionSave_video",        ACT_SAVE_VIDEO},
{"actionProperties",        ACT_VIDEO_PROPERTIES},
{"actionLoad_run_project",  ACT_RUN_SCRIPT},
{"actionSave_project",      ACT_SAVE_PY_SCRIPT},
{"actionPlayFiltered",      ACT_PreviewChanged},

{NULL,ACT_DUMMY}
};
void MainWindow::searchToolBar(QAction *action)
{
        toolBarTranslate *t=toolbar;

        char *name=ADM_strdup(action->objectName().toUtf8().constData());
        while(t->name)
        {
            if(!strcmp(name,t->name))
            {
                sendAction(t->event);
                ADM_dealloc( name);
                return;
            }
            t++;
        }
        ADM_warning("Toolbar:Cannot handle %s\n",name);
        ADM_dealloc( name);
}
/**
    \fn buildFileMenu
*/
bool MainWindow::buildMenu(QMenu *root,MenuEntry *menu, int nb)
{
    bool alt=false, swpud=false;
    if(menu==&myMenuEdit[0] || menu==&myMenuGo[0])
    {
        prefs->get(KEYBOARD_SHORTCUTS_USE_ALTERNATE_KBD_SHORTCUTS,&alt);
        prefs->get(KEYBOARD_SHORTCUTS_SWAP_UP_DOWN_KEYS,&swpud);
    }
    QMenu *subMenu=NULL;
    for(int i=0;i<nb;i++)
    {
        MenuEntry *m=menu+i;
        QString qs;
        if(m->translated)
            qs=QString::fromUtf8(m->text.c_str());
        else
            qs=QString::fromUtf8(QT_TRANSLATE_NOOP("adm",m->text.c_str()));
        switch(m->type)
        {
            case MENU_SEPARATOR:
                root->addSeparator();
                break;
            case MENU_SUBMENU:
                {
                    subMenu=root->addMenu(qs);
                }
                break;
            case MENU_SUBACTION:
            case MENU_ACTION:
                {
                        QMenu *insert=root;
                        if(m->type==MENU_SUBACTION) insert=subMenu;
                        QAction *a=NULL;
                        if(m->icon)
                        {
                            QIcon icon(m->icon);
                            a=insert->addAction(icon,qs);
                        }else
                            a=insert->addAction(qs);
#if defined(__APPLE__)
                        switch(m->event)
                        {
                            case(ACT_EXIT):
                                a->setMenuRole(QAction::QuitRole);
                                break;
                            case(ACT_PREFERENCES):
                                a->setMenuRole(QAction::PreferencesRole);
                                break;
                            case(ACT_ABOUT):
                                a->setMenuRole(QAction::AboutRole);
                                break;
                            default:
			                    a->setMenuRole(QAction::NoRole);
			            }
#endif 
                        m->cookie=(void *)a;
                        if(m->shortCut)
                        {
                            if(swpud && !strcmp(m->shortCut,"Up"))
                                a->setShortcut(Qt::Key_Down);
                            else if(swpud && !strcmp(m->shortCut,"Down"))
                                a->setShortcut(Qt::Key_Up);

                            if(alt)
                            {
                                std::string sc="";
                                switch(m->event)
                                {
                                    case ACT_MarkA:
                                        prefs->get(KEYBOARD_SHORTCUTS_ALT_MARK_A,sc);
                                        break;
                                    case ACT_MarkB:
                                        prefs->get(KEYBOARD_SHORTCUTS_ALT_MARK_B,sc);
                                        break;
                                    case ACT_ResetMarkers:
                                        prefs->get(KEYBOARD_SHORTCUTS_ALT_RESET_MARKERS,sc);
                                        break;
                                    case ACT_GotoMarkA:
                                        prefs->get(KEYBOARD_SHORTCUTS_ALT_GOTO_MARK_A,sc);
                                        break;
                                    case ACT_GotoMarkB:
                                        prefs->get(KEYBOARD_SHORTCUTS_ALT_GOTO_MARK_B,sc);
                                        break;
                                    case ACT_Begin:
                                        prefs->get(KEYBOARD_SHORTCUTS_ALT_BEGIN,sc);
                                        break;
                                    case ACT_End:
                                        prefs->get(KEYBOARD_SHORTCUTS_ALT_END,sc);
                                        break;
                                    default:
                                        sc=std::string(m->shortCut);
                                }
                                QString qsc=QString::fromUtf8(sc.c_str());
                                a->setShortcut(QKeySequence(qsc));
                                break;
                            }
                            QKeySequence s(m->shortCut);
                            a->setShortcut(s);
                        }
                        break;
                }
            default:
                break;
        }
    }
    return true;
}

/**
    buildFileMenu
*/
bool MainWindow::buildMyMenu(void)
{
    connect( ui.menuFile,SIGNAL(triggered(QAction*)),this,SLOT(searchFileMenu(QAction*)));
    buildMenu(ui.menuFile, &myMenuFile[0], myMenuFile.size());

    connect( ui.menuRecent,SIGNAL(triggered(QAction*)),this,SLOT(searchRecentMenu(QAction*)));
    buildMenu(ui.menuRecent, &myMenuRecent[0], myMenuRecent.size());

    connect( ui.menuEdit,SIGNAL(triggered(QAction*)),this,SLOT(searchEditMenu(QAction*)));
    buildMenu(ui.menuEdit, &myMenuEdit[0], myMenuEdit.size());

    connect( ui.menuVideo,SIGNAL(triggered(QAction*)),this,SLOT(searchVideoMenu(QAction*)));
    buildMenu(ui.menuVideo, &myMenuVideo[0], myMenuVideo.size());

    connect( ui.menuAudio,SIGNAL(triggered(QAction*)),this,SLOT(searchAudioMenu(QAction*)));
    buildMenu(ui.menuAudio, &myMenuAudio[0], myMenuAudio.size());

    connect( ui.menuHelp,SIGNAL(triggered(QAction*)),this,SLOT(searchHelpMenu(QAction*)));
    buildMenu(ui.menuHelp, &myMenuHelp[0], myMenuHelp.size());

    connect( ui.menuTools,SIGNAL(triggered(QAction*)),this,SLOT(searchToolMenu(QAction*)));

    if (myMenuTool.size() > 0)
    {
        buildMenu(ui.menuTools, &myMenuTool[0], myMenuTool.size());
    }

    connect( ui.menuGo,SIGNAL(triggered(QAction*)),this,SLOT(searchGoMenu(QAction*)));
    buildMenu(ui.menuGo, &myMenuGo[0], myMenuGo.size());

    connect( ui.menuView,SIGNAL(triggered(QAction*)),this,SLOT(searchViewMenu(QAction*)));
    buildMenu(ui.menuView, &myMenuView[0], myMenuView.size());

    ui.menuVideo->actions().at(3)->setCheckable(true);

    return true;
}

/**
    \fn buildActionLists
*/
void MainWindow::buildActionLists(void)
{
    ActionsAvailableWhenFileLoaded.clear();
    ActionsDisabledOnPlayback.clear();
    ActionsAlwaysAvailable.clear();

    // Make a list of the items that are enabled/disabled depending if video is loaded or  not
    //-----------------------------------------------------------------------------------
    for(int i=1;i<6;i++)
        ActionsAvailableWhenFileLoaded.push_back(ui.menuFile->actions().at(i));

    ActionsAvailableWhenFileLoaded.push_back(ui.menuFile->actions().at(this->_scriptEngines.size()*2+7)); // "Information"

    for(int i=3;i<11;i++)
    {
        if(i==5 || i==7) continue;
        ActionsAvailableWhenFileLoaded.push_back(ui.menuEdit->actions().at(i));
    }

    for(int i=1;i<ui.menuView->actions().size();i++)
    { // disable zoom if no video is loaded
        ActionsAvailableWhenFileLoaded.push_back(ui.menuView->actions().at(i));
    }

    bool canSave=!!ADM_mx_getNbMuxers();
    for(int i=3-canSave;i<ui.toolBar->actions().size();i++)
    { // disable "Save" and "Information" buttons in the toolbar if no video is loaded, "Save" also if we've got zero muxers
        ActionsAvailableWhenFileLoaded.push_back(ui.toolBar->actions().at(i));
    }

#define PUSH_FULL_MENU_LOADED(x,tailOffset) for(int i=0;i<ui.x->actions().size()-tailOffset;i++)    ActionsAvailableWhenFileLoaded.push_back(ui.x->actions().at(i));
#define PUSH_FULL_MENU_PLAYBACK(x,tailOffset) for(int i=0;i<ui.x->actions().size()-tailOffset;i++)    ActionsDisabledOnPlayback.push_back(ui.x->actions().at(i));

    PUSH_FULL_MENU_LOADED(menuAudio,2)
    PUSH_FULL_MENU_LOADED(menuAuto,0)
    PUSH_FULL_MENU_LOADED(menuGo,0)

    // Item disabled on playback
    for(int i=1;i<ui.menuView->actions().size();i++)
    { // allow hiding widgets during playback
        ActionsDisabledOnPlayback.push_back(ui.menuView->actions().at(i));
    }

    for(int i=1;i<ui.menuGo->actions().size();i++)
    { // let "Play/Stop" stay enabled during playback
        ActionsDisabledOnPlayback.push_back(ui.menuGo->actions().at(i));
    }

    PUSH_FULL_MENU_PLAYBACK(menuFile,1)
    PUSH_FULL_MENU_PLAYBACK(menuEdit,0)
    PUSH_FULL_MENU_PLAYBACK(menuVideo,0)
    PUSH_FULL_MENU_PLAYBACK(menuAudio,0)
    PUSH_FULL_MENU_PLAYBACK(menuAuto,0)
    PUSH_FULL_MENU_PLAYBACK(menuHelp,0)
    PUSH_FULL_MENU_PLAYBACK(toolBar,0)

    if(recentFiles)
        for(int i=0;i<recentFiles->actions().size();i++)
            ActionsDisabledOnPlayback.push_back(recentFiles->actions().at(i));
    if(recentProjects)
        for(int i=0;i<recentProjects->actions().size();i++)
            ActionsDisabledOnPlayback.push_back(recentProjects->actions().at(i));

    ActionsDisabledOnPlayback.push_back(ui.menuRecent->actions().back());

    // "Always available" below doesn't override the list of menu items disabled during playback

#define PUSH_ALWAYS_AVAILABLE(menu,entry)   ActionsAlwaysAvailable.push_back( ui.menu->actions().at(entry));

    PUSH_ALWAYS_AVAILABLE(menuFile,0)
    PUSH_ALWAYS_AVAILABLE(menuFile,7)
    PUSH_ALWAYS_AVAILABLE(menuFile,11)

    PUSH_ALWAYS_AVAILABLE(menuEdit,12)
    PUSH_ALWAYS_AVAILABLE(menuEdit,14)
    PUSH_ALWAYS_AVAILABLE(menuEdit,15)            

    PUSH_ALWAYS_AVAILABLE(toolBar,1)

#define PUSH_FULL_MENU_ALWAYS_AVAILABLE(menu) for(int i=0;i<ui.menu->actions().size();i++)    ActionsAlwaysAvailable.push_back(ui.menu->actions().at(i));

    PUSH_FULL_MENU_ALWAYS_AVAILABLE(menuHelp)

    if(recentFiles)
        for(int i=0;i<recentFiles->actions().size();i++)
            ActionsAlwaysAvailable.push_back(recentFiles->actions().at(i));
    if(recentProjects)
        for(int i=0;i<recentProjects->actions().size();i++)
            ActionsAlwaysAvailable.push_back(recentProjects->actions().at(i));
}

/**
    \fn buildButtonLists
*/
void MainWindow::buildButtonLists(void)
{
    ButtonsAvailableWhenFileLoaded.clear();
    ButtonsDisabledOnPlayback.clear();
    PushButtonsAvailableWhenFileLoaded.clear();
    PushButtonsDisabledOnPlayback.clear();

#define ADD_BUTTON_LOADED(x)    ButtonsAvailableWhenFileLoaded.push_back(ui.x);
#define ADD_BUTTON_PLAYBACK(x)    ButtonsDisabledOnPlayback.push_back(ui.x);

    ADD_BUTTON_LOADED(toolButtonPlay)
    ADD_BUTTON_LOADED(toolButtonPreviousFrame)
    ADD_BUTTON_LOADED(toolButtonNextFrame)
    ADD_BUTTON_LOADED(toolButtonPreviousIntraFrame)
    ADD_BUTTON_LOADED(toolButtonNextIntraFrame)
    ADD_BUTTON_LOADED(toolButtonSetMarkerA)
    ADD_BUTTON_LOADED(toolButtonSetMarkerB)
    ADD_BUTTON_LOADED(toolButtonPreviousBlackFrame)
    ADD_BUTTON_LOADED(toolButtonNextBlackFrame)
    ADD_BUTTON_LOADED(toolButtonFirstFrame)
    ADD_BUTTON_LOADED(toolButtonLastFrame)
    ADD_BUTTON_LOADED(toolButtonBackOneMinute)
    ADD_BUTTON_LOADED(toolButtonForwardOneMinute)

    ADD_BUTTON_PLAYBACK(toolButtonPreviousFrame)
    ADD_BUTTON_PLAYBACK(toolButtonNextFrame)
    ADD_BUTTON_PLAYBACK(toolButtonPreviousIntraFrame)
    ADD_BUTTON_PLAYBACK(toolButtonNextIntraFrame)
    ADD_BUTTON_PLAYBACK(toolButtonSetMarkerA)
    ADD_BUTTON_PLAYBACK(toolButtonSetMarkerB)
    ADD_BUTTON_PLAYBACK(toolButtonPreviousBlackFrame)
    ADD_BUTTON_PLAYBACK(toolButtonNextBlackFrame)
    ADD_BUTTON_PLAYBACK(toolButtonFirstFrame)
    ADD_BUTTON_PLAYBACK(toolButtonLastFrame)
    ADD_BUTTON_PLAYBACK(toolButtonBackOneMinute)
    ADD_BUTTON_PLAYBACK(toolButtonForwardOneMinute)

#define ADD_PUSHBUTTON_LOADED(x)    PushButtonsAvailableWhenFileLoaded.push_back(ui.x);
#define ADD_PUSHBUTTON_PLAYBACK(x)    PushButtonsDisabledOnPlayback.push_back(ui.x);

    ADD_PUSHBUTTON_LOADED(pushButtonTime)
    ADD_PUSHBUTTON_LOADED(pushButtonJumpToMarkerA)
    ADD_PUSHBUTTON_LOADED(pushButtonJumpToMarkerB)

    ADD_PUSHBUTTON_PLAYBACK(pushButtonTime)
    ADD_PUSHBUTTON_PLAYBACK(pushButtonJumpToMarkerA)
    ADD_PUSHBUTTON_PLAYBACK(pushButtonJumpToMarkerB)

    ADD_PUSHBUTTON_PLAYBACK(pushButtonDecoderConf)
    ADD_PUSHBUTTON_PLAYBACK(pushButtonVideoConf)
    ADD_PUSHBUTTON_PLAYBACK(pushButtonVideoFilter)
    ADD_PUSHBUTTON_PLAYBACK(pushButtonAudioConf)
    ADD_PUSHBUTTON_PLAYBACK(pushButtonAudioFilter)
    ADD_PUSHBUTTON_PLAYBACK(pushButtonFormatConfigure)
}

/**
    \fn setMenuItemsEnabledState
    \brief disable or enable some of the menu items
*/
void MainWindow::setMenuItemsEnabledState(void)
{
    if(playing) // this actually doesn't work as it should
    {
        int n=ActionsDisabledOnPlayback.size();
        for(int i=0;i<n;i++)
            ActionsDisabledOnPlayback[i]->setEnabled(false);

        int ntb=ButtonsDisabledOnPlayback.size();
        for(int i=0;i<ntb;i++)
            ButtonsDisabledOnPlayback[i]->setEnabled(false);

        ui.toolButtonPlay->setIcon(QIcon(MKICON(player_stop)));
        ui.menuGo->actions().at(0)->setIcon(QIcon(MKICON(player_stop)));

        int npb=PushButtonsDisabledOnPlayback.size();
        for(int i=0;i<npb;i++)
            PushButtonsDisabledOnPlayback[i]->setEnabled(false);

        ui.checkBox_TimeShift->setEnabled(false);
        ui.spinBox_TimeValue->setEnabled(false);

        if(getPreviewMode()!=ADM_PREVIEW_NONE)
            slider->setEnabled(false);

        return;
    }

    bool vid, undo, redo, paste, restore;
    vid=undo=redo=paste=restore=false;
    if(avifileinfo)
        vid=true; // a video is loaded

    int n=ActionsAvailableWhenFileLoaded.size();
    for(int i=0;i<n;i++)
        ActionsAvailableWhenFileLoaded[i]->setEnabled(vid);

    int ntb=ButtonsAvailableWhenFileLoaded.size();
    for(int i=0;i<ntb;i++)
        ButtonsAvailableWhenFileLoaded[i]->setEnabled(vid);

    int npb=PushButtonsAvailableWhenFileLoaded.size();
    for(int i=0;i<npb;i++)
        PushButtonsAvailableWhenFileLoaded[i]->setEnabled(vid);

    ui.menuFile->actions().at(2)->setEnabled(vid && !!ADM_mx_getNbMuxers()); // disable saving video if there are no muxers
    if(vid)
    {
        undo=video_body->canUndo();
        redo=video_body->canRedo();
        paste=1-(video_body->clipboardEmpty());
    }
    ui.menuEdit->actions().at(0)->setEnabled(undo); // menu item "Undo"
    ui.menuEdit->actions().at(1)->setEnabled(redo); // menu item "Redo"
    if(!vid || (!undo && !redo)) // if no edits have been performed, disable "Reset Edit" menu item
    {
        restore=A_checkSavedSession(false);
        ui.menuEdit->actions().at(2)->setEnabled(false);
    }else
    {
        ui.menuEdit->actions().at(2)->setEnabled(true);
    }
    ui.menuEdit->actions().at(5)->setEnabled(paste); // "Paste"

    n=ActionsAlwaysAvailable.size();
    for(int i=0;i<n;i++)
        ActionsAlwaysAvailable[i]->setEnabled(true);

    ui.toolButtonPlay->setIcon(QIcon(MKICON(player_play)));
    ui.menuGo->actions().at(0)->setIcon(QIcon(MKICON(player_play)));

    bool haveRecentItems=false;
    if(recentFiles && recentFiles->actions().size())
        haveRecentItems=true;
    if(recentProjects && recentProjects->actions().size())
        haveRecentItems=true;
    ui.menuRecent->actions().back()->setEnabled(haveRecentItems);

    n=0;
    if(recentFiles)
        n++;
    if(recentProjects)
        n++;
    if(ui.menuRecent->actions().size()>(n+2)) // "Restore session" entry exists
        ui.menuRecent->actions().at(n+1)->setEnabled(restore);

    ui.selectionDuration->setEnabled(vid);
    slider->setEnabled(vid);

    updateCodecWidgetControlsState();
    // actions performed by the code above may result in a window resize event,
    // which in turn may initiate unwanted zoom changes e.g. when stopping playback
    // or loading a video with small dimensions, so ignore just this one resize event
    ignoreResizeEvent = true;
}

/**
    \fn updateCodecWidgetControlsState
*/
void MainWindow::updateCodecWidgetControlsState(void)
{
    bool b=false;
    // currently only lavc provides some decoder options
    if(avifileinfo && !strcmp(video_body->getVideoDecoderName(),"Lavcodec"))
        b=true;
    ui.pushButtonDecoderConf->setEnabled(b);
    // take care of the "Decoder Options" item in the menu "Video"
    ui.menuVideo->actions().at(0)->setEnabled(b);
    // post-processing is available only for software decoding and old codecs
    if(b && !(isMpeg12Compatible(avifileinfo->fcc) || isMpeg4Compatible(avifileinfo->fcc)))
        b=false;
    ui.menuVideo->actions().at(1)->setEnabled(b);

    b=false;
    if(ui.comboBoxVideo->currentIndex())
        b=true;
    ui.pushButtonVideoConf->setEnabled(b);
    if(avifileinfo)
    {
        ui.pushButtonVideoFilter->setEnabled(b);
        // take care of the "Filter" item in the menu "Video" as well
        ui.menuVideo->actions().at(2)->setEnabled(b);
        ui.menuVideo->actions().at(3)->setEnabled(b);
        ui.toolBar->actions().at(5)->setEnabled(b);
    }else
    {
        ui.pushButtonVideoFilter->setEnabled(false);
        ui.menuVideo->actions().at(2)->setEnabled(false);
        ui.menuVideo->actions().at(3)->setEnabled(false);
        ui.toolBar->actions().at(5)->setEnabled(false);
    }

    b=false;
    if(avifileinfo && video_body->getDefaultEditableAudioTrack())
    {
        ui.menuAudio->actions().at(1)->setEnabled(true);
        if(ui.comboBoxAudio->currentIndex())
            b=true;
    }else
    {   // disable "Save Audio" item in the menu "Audio" if we have no audio tracks
        ui.menuAudio->actions().at(1)->setEnabled(false);
    }
    ui.pushButtonAudioConf->setEnabled(b);
    ui.pushButtonAudioFilter->setEnabled(b);
    // take care of the "Filter" item in the menu "Audio"
    ui.menuAudio->actions().at(2)->setEnabled(b);

    // reenable the controls below unconditionally after playback
    ui.checkBox_TimeShift->setEnabled(true);
    ui.spinBox_TimeValue->setEnabled(true);
    // disable the "Save" button in the toolbar and the "Configure" button for the output format if we have no muxers
    b=false;
    bool gotMuxers=(bool)ADM_mx_getNbMuxers();
    if(avifileinfo && gotMuxers)
        b=true;
    ui.toolBar->actions().at(2)->setEnabled(b);
    ui.pushButtonFormatConfigure->setEnabled(gotMuxers);
}

/**
    \fn updateActionShortcuts
*/
void MainWindow::updateActionShortcuts(void)
{
    std::vector<MenuEntry *> defaultShortcuts;
    std::vector<QAction *> listOfActionsToUpdate;
    bool alt=false, swpud=false;
    prefs->get(KEYBOARD_SHORTCUTS_USE_ALTERNATE_KBD_SHORTCUTS,&alt);
    prefs->get(KEYBOARD_SHORTCUTS_SWAP_UP_DOWN_KEYS,&swpud);

    ui.menuGo->actions().at(4-swpud)->setShortcut(Qt::Key_Up);
    ui.menuGo->actions().at(3+swpud)->setShortcut(Qt::Key_Down);

    for(int i=8;i<11;i++)
    {
        defaultShortcuts.push_back(&myMenuEdit[i]);
        listOfActionsToUpdate.push_back(ui.menuEdit->actions().at(i));
    }

    for(int i=7;i<12;i++)
    {
        if(i==9) continue;
        defaultShortcuts.push_back(&myMenuGo[i]);
        listOfActionsToUpdate.push_back(ui.menuGo->actions().at(i));
    }

    int n=listOfActionsToUpdate.size();
    for(int i=0;i<n;i++)
    {
        MenuEntry *m=defaultShortcuts.at(i);
        QAction *a=listOfActionsToUpdate.at(i);
        if(alt)
        {
            std::string sc="";
            switch(m->event)
            {
                case ACT_MarkA:
                    prefs->get(KEYBOARD_SHORTCUTS_ALT_MARK_A,sc);
                    break;
                case ACT_MarkB:
                    prefs->get(KEYBOARD_SHORTCUTS_ALT_MARK_B,sc);
                    break;
                case ACT_ResetMarkers:
                    prefs->get(KEYBOARD_SHORTCUTS_ALT_RESET_MARKERS,sc);
                    break;
                case ACT_GotoMarkA:
                    prefs->get(KEYBOARD_SHORTCUTS_ALT_GOTO_MARK_A,sc);
                    break;
                case ACT_GotoMarkB:
                    prefs->get(KEYBOARD_SHORTCUTS_ALT_GOTO_MARK_B,sc);
                    break;
                case ACT_Begin:
                    prefs->get(KEYBOARD_SHORTCUTS_ALT_BEGIN,sc);
                    break;
                case ACT_End:
                    prefs->get(KEYBOARD_SHORTCUTS_ALT_END,sc);
                    break;
                default:
                    sc=std::string(m->shortCut);
            }
            QString qsc=QString::fromUtf8(sc.c_str());
            a->setShortcut(QKeySequence(qsc));
        }else
        {
            QKeySequence s(m->shortCut);
            a->setShortcut(s);
        }
    }

    widgetsUpdateTooltips();

}

/**
    \fn widgetsUpdateTooltips
    \brief Update tooltips showing tunable action shortcuts in the navigation and selection widgets
*/
void MainWindow::widgetsUpdateTooltips(void)
{
    std::vector<QString> ListOfShortcuts;
    for(int i=0;i<12;i++)
    {
        if(i==5 || i==6 || i==9) continue;
        QKeySequence seq=ui.menuGo->actions().at(i)->shortcut();
        QString s=seq.toString().toUpper();
        ListOfShortcuts.push_back(s);
    }

    for(int i=8;i<10;i++)
    {
        QKeySequence seq=ui.menuEdit->actions().at(i)->shortcut();
        QString s=seq.toString().toUpper();
        ListOfShortcuts.push_back(s);
    }

    QString tt;

    tt=QString(QT_TRANSLATE_NOOP("qgui2","Play/Stop"))+QString(" [")+ListOfShortcuts[0]+QString("]");
    ui.toolButtonPlay->setToolTip(tt);

    tt=QString(QT_TRANSLATE_NOOP("qgui2","Go to previous frame"))+QString(" [")+ListOfShortcuts[1]+QString("]");
    ui.toolButtonPreviousFrame->setToolTip(tt);

    tt=QString(QT_TRANSLATE_NOOP("qgui2","Go to next frame"))+QString(" [")+ListOfShortcuts[2]+QString("]");
    ui.toolButtonNextFrame->setToolTip(tt);

    tt=QString(QT_TRANSLATE_NOOP("qgui2","Go to previous keyframe"))+QString(" [")+ListOfShortcuts[3]+QString("]");
    ui.toolButtonPreviousIntraFrame->setToolTip(tt);

    tt=QString(QT_TRANSLATE_NOOP("qgui2","Go to next keyframe"))+QString(" [")+ListOfShortcuts[4]+QString("]");
    ui.toolButtonNextIntraFrame->setToolTip(tt);

    tt=QString(QT_TRANSLATE_NOOP("qgui2","Set start marker"))+QString(" [")+ListOfShortcuts[9]+QString("]");
    ui.toolButtonSetMarkerA->setToolTip(tt);

    tt=QString(QT_TRANSLATE_NOOP("qgui2","Set end marker"))+QString(" [")+ListOfShortcuts[10]+QString("]");
    ui.toolButtonSetMarkerB->setToolTip(tt);

    // go to black frame tooltips are static, the actions don't have shortcuts

    tt=QString(QT_TRANSLATE_NOOP("qgui2","Go to first frame"))+QString(" [")+ListOfShortcuts[5]+QString("]");
    ui.toolButtonFirstFrame->setToolTip(tt);

    tt=QString(QT_TRANSLATE_NOOP("qgui2","Go to last frame"))+QString(" [")+ListOfShortcuts[6]+QString("]");
    ui.toolButtonLastFrame->setToolTip(tt);

    tt=QString(QT_TRANSLATE_NOOP("qgui2","Go to marker A"))+QString(" [")+ListOfShortcuts[7]+QString("]");
    ui.pushButtonJumpToMarkerA->setToolTip(tt);

    tt=QString(QT_TRANSLATE_NOOP("qgui2","Go to marker B"))+QString(" [")+ListOfShortcuts[8]+QString("]");
    ui.pushButtonJumpToMarkerB->setToolTip(tt);

    // special case one minute back and forward buttons, their action shortcuts are not defined via myOwnMenu.h
    bool swpud=false;
    prefs->get(KEYBOARD_SHORTCUTS_SWAP_UP_DOWN_KEYS,&swpud);
    QString back, forward;
    if(!swpud)
    {
        back="DOWN";
        forward="UP";
    }else
    {
        back="UP";
        forward="DOWN";
    }
    tt=QString(QT_TRANSLATE_NOOP("qgui2","Backward one minute"))+QString(" [CTRL+")+back+QString("]");
    ui.toolButtonBackOneMinute->setToolTip(tt);

    tt=QString(QT_TRANSLATE_NOOP("qgui2","Forward one minute"))+QString(" [CTRL+")+forward+QString("]");
    ui.toolButtonForwardOneMinute->setToolTip(tt);
}

/**
 * \fn checkChanged
 * \brief the checkbox protecting timeshift value has changed
 * @param state
 */
void MainWindow::checkChanged(int state)
{
    bool b=true;
    if(state)
        b=true;
    else 
        b=false;
    ui.spinBox_TimeValue->setEnabled(b);
    timeChanged(0);
}
/**
    \fn timeChanged
    \brief Called whenever timeshift is on/off'ed or value changes
*/
void MainWindow::timeChanged(int)
{
    sendAction (ACT_TimeShift) ;
}
/**
    \fn searchMenu
*/
void MainWindow::searchMenu(QAction * action,MenuEntry *menu, int nb)
{
    for(int i=0;i<nb;i++)
    {
        MenuEntry *m=menu+i;
        if(m->cookie==(void*)action)
        {
            sendAction (m->event);
        }
    }
}

/**
    \fn searchFileMenu
*/
#define MKMENU(name) void MainWindow::search##name##Menu(QAction * action) \
    {searchMenu(action, &myMenu##name[0], myMenu##name.size());}

MKMENU(File)
MKMENU(Edit)
MKMENU(Recent)
MKMENU(View)
MKMENU(Tool)
MKMENU(Go)
//MKMENU(Custom)
MKMENU(Audio)
MKMENU(Video)
MKMENU(Help)



/*
      We receive a button press event
*/
void MainWindow::buttonPressed(void)
{
    // Receveid a key press Event, look into table..
    QObject *obj=sender();
    if(!obj) return;
    QString me(obj->objectName());

    Action action=searchTranslationTable(qPrintable(me));


    if(action!=ACT_DUMMY)
        sendAction (action);

}
void MainWindow::toolButtonPressed(bool i)
{
    buttonPressed();
}
#ifdef ENABLE_EVENT_FILTER
bool MainWindow::eventFilter(QObject* watched, QEvent* event)
{
    QKeyEvent *keyEvent;
    bool swpud=false;
    prefs->get(KEYBOARD_SHORTCUTS_SWAP_UP_DOWN_KEYS,&swpud);
    switch (event->type())
    {
        case QEvent::KeyPress:
            keyEvent = (QKeyEvent*)event;

//            if (watched == slider)
            {
                switch (keyEvent->key())
                {
                    case Qt::Key_Left:
                        if ((keyEvent->modifiers() & Qt::ShiftModifier) && (keyEvent->modifiers() & Qt::ControlModifier))
                            sendAction(ACT_Back4Seconds);
                        else if (keyEvent->modifiers() & Qt::ShiftModifier)
                            sendAction(ACT_Back1Second);
                        else if (keyEvent->modifiers() & Qt::ControlModifier)
                            sendAction(ACT_Back2Seconds);
                        else
                            sendAction(ACT_PreviousFrame);

                        return true;
                    case Qt::Key_Right:
                        if ((keyEvent->modifiers() & Qt::ShiftModifier) && (keyEvent->modifiers() & Qt::ControlModifier))
                            sendAction(ACT_Forward4Seconds);
                        else if (keyEvent->modifiers() & Qt::ShiftModifier)
                            sendAction(ACT_Forward1Second);
                        else if (keyEvent->modifiers() & Qt::ControlModifier)
                            sendAction(ACT_Forward2Seconds);
                        else 
                            sendAction(ACT_NextFrame);

                        return true;
                    case Qt::Key_Up:
                        if (keyEvent->modifiers() & Qt::ControlModifier)
                        {
                            if(!swpud)
                                sendAction(ACT_Forward1Mn);
                            else
                                sendAction(ACT_Back1Mn);
                        }else
                        {
                            if(!swpud)
                                sendAction(ACT_NextKFrame);
                            else
                                sendAction(ACT_PreviousKFrame);
                        }
                        return true;
                    case Qt::Key_Down:
                        if (keyEvent->modifiers() & Qt::ControlModifier)
                        {
                            if(!swpud)
                                sendAction(ACT_Back1Mn);
                            else
                                sendAction(ACT_Forward1Mn);
                        }else
                        {
                            if(!swpud)
                                sendAction(ACT_PreviousKFrame);
                            else
                                sendAction(ACT_NextKFrame);
                        }
                        return true;
                    case Qt::Key_C:
                        if ((keyEvent->modifiers() & Qt::ShiftModifier) && (keyEvent->modifiers() & Qt::ControlModifier))
                            currentTimeToClipboard();
                        return true;
                    case Qt::Key_Shift:
                        shiftKeyHeld = 1;
                        break;

                    case Qt::Key_PageUp:
                                                if (keyEvent->modifiers() & Qt::ControlModifier)
                                                        sendAction(ACT_MarkA);
                                                else
                                                        sendAction(ACT_GotoMarkA);
                                                return true;
                    case Qt::Key_PageDown:
                                                if (keyEvent->modifiers() & Qt::ControlModifier)
                                                        sendAction(ACT_MarkB);
                                                else
                                                        sendAction(ACT_GotoMarkB);
                                                return true;
                    default:
                        break;
                }
            }
            /* else */ if (keyEvent->key() == Qt::Key_Space)
            {
                sendAction(ACT_PlayAvi);
                return true;
            }

            break;
        case QEvent::Resize:
            if(watched == QuiMainWindows)
            {
                if(blockZoomChanges || playing || !avifileinfo)
                    break;
                if(ignoreResizeEvent)
                {
                    ignoreResizeEvent = false;
                    break;
                }
                uint32_t reqw, reqh;
                calcDockWidgetDimensions(reqw,reqh);

                uint32_t availw = 1;
                if(QuiMainWindows->width() > reqw)
                    availw = QuiMainWindows->width() - reqw;
                else
                    break;

                uint32_t availh = 1;
                if(QuiMainWindows->height() > reqh)
                    availh = QuiMainWindows->height() - reqh;
                else
                    break;

                uint32_t w = avifileinfo->width;
                uint32_t h = avifileinfo->height;
                if(!w || !h)
                    break;
                // We've got the available space and the video resolution,
                // now calculate the zoom to fit the video into this space.
                float widthRatio = (float)availw / (float)w;
                float heightRatio = (float)availh / (float)h;
                float zoom = (widthRatio < heightRatio ? widthRatio : heightRatio);

                // Detect if the zoom has been likely set automatically on loading a new video or
                // changed using a keyboard shortcut. In this case imitate physical friction
                // requiring the window to be resized beyond certain threshold first.
                float oldzoom = admPreview::getCurrentZoom();
                QSize os = static_cast<QResizeEvent*>(event)->oldSize();
                if(zoom > oldzoom && actZoomCalled)
                {
                    if(QuiMainWindows->width() > os.width())
                        threshold -= QuiMainWindows->width() - os.width();
                    if(QuiMainWindows->height() > os.height())
                        threshold -= QuiMainWindows->height() - os.height();
                    if(threshold > 0)
                        break;
                    if(threshold < 0)
                        threshold = 0;
                }

                if(zoom > oldzoom + .001 || zoom < oldzoom - .001)
                {
                    blockResizing = true;
                    admPreview::setMainDimension(w,h,zoom);
                    actZoomCalled = false;
                    admPreview::samePicture(); // required at least for VDPAU
                    blockResizing = false;
                }
            }
            if (watched == ui.sliderPlaceHolder)
            {
                    thumbSlider->resize(ui.sliderPlaceHolder->width(), 16);
                    thumbSlider->move(0, (ui.sliderPlaceHolder->height() - thumbSlider->height()) / 2);
            }
            break;

        case QEvent::KeyRelease:
            keyEvent = (QKeyEvent*)event;

            if (keyEvent->key() == Qt::Key_Shift)
                shiftKeyHeld = 0;

            break;
        case QEvent::User:
            this->openFiles(((FileDropEvent*)event)->files);
            break;
        default:
            break;
    }

    return QObject::eventFilter(watched, event);
}
#endif
void MainWindow::mousePressEvent(QMouseEvent* event)
{
    this->setFocus(Qt::OtherFocusReason);
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("text/uri-list"))
        event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasUrls())
    {
        QCoreApplication::postEvent(this, new FileDropEvent(event->mimeData()->urls()));

        event->acceptProposedAction();
    }
}

// If a video was loaded or zoom changed while the main window was maximized,
// unmaximizing the window restores it to dimensions not necessarily matching
// the actual dimensions of the video frame. Catch the window state change event
// and resize the window when appropriate.
void MainWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::WindowStateChange)
    {
        QWindowStateChangeEvent* ev = static_cast<QWindowStateChangeEvent*>(event);
        if (true==UI_getNeedsResizingFlag() && ev->oldState() == Qt::WindowMaximized && false==QuiMainWindows->isMinimized())
        {
            uint32_t w=ui.frame_video->width();
            uint32_t h=ui.frame_video->height();
            UI_resize(w,h);
            UI_setNeedsResizingFlag(false);
        }
        // Always adjust zoom on maximize
        if (ev->oldState() != Qt::WindowMaximized && QuiMainWindows->isMaximized())
            ignoreResizeEvent = false;
    }
    QWidget::changeEvent(event);
}

void MainWindow::openFiles(QList<QUrl> urlList)
{
    QFileInfo info;

    for (int fileIndex = 0; fileIndex < urlList.size(); fileIndex++)
    {
        QString fileName = urlList[fileIndex].toLocalFile();
        QFileInfo info(fileName);

        if (info.isFile())
        {
            if (avifileinfo)
                A_appendVideo(fileName.toUtf8().constData());
            else
                A_openVideo(fileName.toUtf8().constData());
            // Set lastdir_read on drag'n'drop here instead of centrally in
            // A_openVideo or in A_appendVideo to better deal with situations
            // where videos are loaded from a project script in a different location.
            // Otherwise lastdir_read is managed by the file selection dialog.
            admCoreUtils::setLastReadFolder(std::string(fileName.toUtf8().constData()));
        }
    }
}

/**
    \fn calcDockWidgetDimensions
    \brief calculate the total width and height occupied by the toolbar and the codec, navigation etc. dock widgets
*/
void MainWindow::calcDockWidgetDimensions(uint32_t &width, uint32_t &height)
{
    uint32_t reqw=18; // 2 x 9px margin
    if(ui.codecWidget->isVisible())
        reqw += ui.codecWidget->frameSize().width() + 6; // with codec widget visible a small extra margin is necessary
    if(ui.toolBar->orientation()==Qt::Vertical && ui.toolBar->isVisible() && false==ui.toolBar->isFloating())
        reqw += ui.toolBar->frameSize().width();
    width = reqw;

    uint32_t reqh=18; // 2 x 9px margin
    if(ui.menubar->isVisible())
        reqh += ui.menubar->height();
    if(ui.toolBar->isVisible() && false==ui.toolBar->isFloating() && ui.toolBar->orientation()==Qt::Horizontal)
        reqh += ui.toolBar->frameSize().height();
    if(ui.navigationWidget->isVisible() || ui.selectionWidget->isVisible() || ui.volumeWidget->isVisible() || ui.audioMetreWidget->isVisible())
       reqh += ui.navigationWidget->frameSize().height();
    height = reqh;
}

/**
    \fn setResizeThreshold
*/
void MainWindow::setResizeThreshold(int value)
{
    threshold = value;
}

/**
    \fn setActZoomCalledFlag
*/
void MainWindow::setActZoomCalledFlag(bool called)
{
    actZoomCalled = called;
}

/**
    \fn setBlockZoomChangesFlag
*/
void MainWindow::setBlockZoomChangesFlag(bool block)
{
    blockZoomChanges = block;
}

/**
    \fn getBlockResizingFlag
*/
bool MainWindow::getBlockResizingFlag(void)
{
    return blockResizing;
}

/**
    \fn setBlockResizingFlag
*/
void MainWindow::setBlockResizingFlag(bool block)
{
    blockResizing = block;
}

void MainWindow::previousIntraFrame(void)
{
    if (ui.spinBox_TimeValue->hasFocus())
        ui.spinBox_TimeValue->stepDown();
    else
        sendAction(ACT_PreviousKFrame);
}

void MainWindow::nextIntraFrame(void)
{
    if (ui.spinBox_TimeValue->hasFocus())
        ui.spinBox_TimeValue->stepUp();
    else
        sendAction(ACT_NextKFrame);
}

/**
    \fn volumeWidgetOperational
*/
void MainWindow::volumeWidgetOperational(void)
{
    // Hide and disable the volume widget if the audio device doesn't support setting volume
    if(!AVDM_hasVolumeControl())
    {
        ui.volumeWidget->setEnabled(false);
        ui.volumeWidget->hide();
        ui.actionViewVolume->setChecked(false);
    }else
    {
        ui.volumeWidget->setEnabled(true);
        ui.volumeWidget->show();
        ui.actionViewVolume->setChecked(true);
    }
}

MainWindow::~MainWindow()
{
    renderDestroy(); // make sure render does not have back link to us
    delete thumbSlider;
    thumbSlider=NULL;
}

static const UI_FUNCTIONS_T UI_Hooks=
    {
        ADM_RENDER_API_VERSION_NUMBER,
        UI_getWindowInfo,
        UI_updateDrawWindowSize,
        UI_rgbDraw,
        UI_getDrawWidget,
        UI_getPreferredRender

    };


static myQApplication *myApplication=NULL;
/**
 * \fn UI_reset
 * \brief reset
 * @return 
 */
bool      UI_reset(void)
{
    UI_setVideoCodec(0);
    UI_setAudioCodec(0);
    UI_setCurrentPreview(false);
    return true;
}

/**
    \fn  UI_Init
    \brief First part of UI initialization

*/
int UI_Init(int nargc, char **nargv)
{
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
    ADM_info("Starting Qt5 GUI...\n");
#else
    ADM_info("Starting Qt4 GUI...\n");
#endif
    initTranslator();

    global_argc=nargc;
    global_argv=nargv;
    ADM_renderLibInit(&UI_Hooks);
#if defined(_WIN32) && QT_VERSION >= QT_VERSION_CHECK(5,11,0)
    // Despite HiDPI scaling being supported from Qt 5.6 on, important aspects
    // like OpenGL support were fixed only in much later versions.
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    myApplication=new myQApplication (global_argc, global_argv);
    myApplication->connect(myApplication, SIGNAL(lastWindowClosed()), myApplication, SLOT(quit()));
    myApplication->connect(myApplication, SIGNAL(aboutToQuit()), myApplication, SLOT(cleanup()));
#ifdef __APPLE__
    Q_INIT_RESOURCE(avidemux_osx);
#elif defined(_WIN32)
    Q_INIT_RESOURCE(avidemux_win32);
#else
    Q_INIT_RESOURCE(avidemux);
#endif
    Q_INIT_RESOURCE(filter);

    loadTranslator();

    return 1;
}

uint8_t initGUI(const vector<IScriptEngine*>& scriptEngines)
{
    MainWindow *mw = new MainWindow(scriptEngines);
    mw->show();

    QuiMainWindows = (QWidget*)mw;

    uint32_t w, h;

    UI_getPhysicalScreenSize(QuiMainWindows, &w,&h);
    printf("The screen seems to be %u x %u px\n",w,h);
    mw->ui.frame_video->setAttribute(Qt::WA_OpaquePaintEvent);

    UI_QT4VideoWidget(mw->ui.frame_video);  // Add the widget that will handle video display
#if QT_VERSION < QT_VERSION_CHECK(5,11,0) // not sure about the version
    mw->ui.frame_video->setAcceptDrops(true); // needed for drag and drop to work on windows
#endif
    admPreview::setMainDimension(0,0,ZOOM_1_1);

    UI_updateRecentMenu();
    UI_updateRecentProjectMenu();

    // Init vumeter
    VuMeter=mw->ui.frameVU;
    UI_InitVUMeter(mw->ui.frameVU);

#ifdef USE_OPENGL
    ADM_info("OpenGL enabled at built time, checking if we should run it..\n");
    bool enabled;
    prefs->get(FEATURES_ENABLE_OPENGL,&enabled);

    if(enabled)
    {
        ADM_info("OpenGL activated, initializing... \n");
        openGLStarted=true;
        UI_Qt4InitGl();
    }else
    {
        ADM_info("OpenGL not activated, not initialized\n");
    }
#else
        ADM_info("OpenGL: Not enabled at built time.\n");
#endif

    return 1;
}
/**
 * \fn UI_closeGui
 */
void UI_closeGui(void)
{
        if(!uiRunning) return;
        uiRunning=false;
        
    QuiMainWindows->close();
    qtUnregisterDialog(QuiMainWindows);
        
}

void destroyGUI(void)
{

}
void callBackQtWindowDestroyed()
{
    
}
/**

*/
bool UI_End(void)
{
    ADM_QPreviewCleanup();
    return true;
}
void UI_refreshCustomMenu(void)
{
    ((MainWindow*)QuiMainWindows)->buildCustomMenu();
}
/**
    \fn UI_applySettings
    \brief Do stuff when closing the preferences dialog
*/
void UI_applySettings(void)
{
    ((MainWindow *)QuiMainWindows)->updateActionShortcuts();
    ((MainWindow *)QuiMainWindows)->volumeWidgetOperational();
    ((MainWindow *)QuiMainWindows)->setRefreshCap();
}
/**
    \fn UI_getCurrentPreview
    \brief Read previewmode from checkable menu actions
*/
int UI_getCurrentPreview(void)
{
    if(WIDGET(menuVideo)->actions().at(3)->isChecked() || WIDGET(toolBar)->actions().at(5)->isChecked())
    {
        printf("Output is ON\n");
        return 1;
    }
    printf("Output is Off\n");
    return 0;
}

/**
    \fn UI_setCurrentPreview
    \brief Update "Play filtered" checkable menu actions with previewmode
*/
void UI_setCurrentPreview(int ne)
{
    WIDGET(menuVideo)->actions().at(3)->setChecked(!!ne);
    WIDGET(toolBar)->actions().at(5)->setChecked(!!ne);
}
/**
        \fn FatalFunctionQt
*/
extern void abortExitHandler();
static void FatalFunctionQt(const char *title, const char *info)
{
    printf("Crash Dump for %s\n",title);
    printf("%s\n",info);
    fflush(stdout);
    
    QMessageBox msgBox;
    msgBox.setText(title);
    msgBox.setInformativeText(QT_TRANSLATE_NOOP("qgui2","The application has encountered a fatal problem\nThe current editing has been saved and will be reloaded at next start"));
    msgBox.setDetailedText(info);
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.exec();
    abortExitHandler(); // Try to cleanup
    abort();
}

/**
    \fn UI_RunApp(void)
    \brief Main entry point for the GUI application
*/
int UI_RunApp(void)
{
    uiRunning=true;
    setupMenus();
    QuiTaskBarProgress->setParent(QuiMainWindows);
    ADM_setCrashHook(&saveCrashProject, &FatalFunctionQt,&abortExitHandler);
    
    ADM_info("Load default settings if any... \n");          
    A_loadDefaultSettings();
    UI_applySettings();
    
    // start update checking..
    bool autoUpdateEnabled=false;
    if(prefs->get(UPDATE_ENABLED,&autoUpdateEnabled))
    {
#ifndef _MSC_VER
        if(autoUpdateEnabled)
        {
            // Mark last check
            struct timeval tp;
            struct timezone tz;
            gettimeofday(&tp,&tz);
            uint32_t days=1+(tp.tv_sec-1472894364)/(60*60*24); // days since 03 sept
            uint32_t lastCheck;
            prefs->get(UPDATE_LASTCHECK,&lastCheck);
            ADM_info("[autoUpdate]Current date %d , last check = %d\n",days,lastCheck);
            if(days>lastCheck)
            {
                prefs->set(UPDATE_LASTCHECK,days);
                prefs->save();
                ADM_checkForUpdate(&MainWindow::updateCheckDone);
            }
        }
#endif
    }
    
    myApplication->exec();
#ifdef USE_OPENGL
    if(openGLStarted)
    {
        ADM_info("OpenGL: Cleaning up\n");
        UI_Qt4CleanGl();
    }
#endif
    destroyTranslator();

    delete QuiMainWindows;
    delete myApplication;

    QuiMainWindows = NULL;
    myApplication = NULL;

    return 1;
}
/**
 * \fn updateCheckDone
 * @param version
 * @param date
 * @param downloadLink
 */
void MainWindow::updateCheckDone(int version, const std::string &date, const std::string &downloadLink)
{
    ADM_info("Version available %d from %s at %s\n",version,date.c_str(),downloadLink.c_str());
    emit mainWindowSingleton->updateAvailable(version,date,downloadLink);
}

/**
    \fn searchTranslationTable(const char *name))
    \brief return the action corresponding to a give button. The translation table is in translation_table.h
*/
Action searchTranslationTable(const char *name)
{
    for(int i=0;i< SIZEOF_MY_TRANSLATION;i++)
    {
        if(!strcmp(name, myTranslationTable[i].name))
        {
            return  myTranslationTable[i].action;
        }
    }
    printf("WARNING: Signal not found in translation table %s\n",name);
    return ACT_DUMMY;
}
/**
    \fn     UI_updateRecentMenu( void )
    \brief  Update the recent submenu with the latest files loaded
*/
void UI_updateRecentMenu( void )
{
    ((MainWindow *)QuiMainWindows)->buildRecentMenu();
    ((MainWindow *)QuiMainWindows)->setMenuItemsEnabledState();
}

void UI_updateRecentProjectMenu()
{
    ((MainWindow *)QuiMainWindows)->buildRecentProjectMenu();
}

/**
  \fn    setupMenus(void)
  \brief Fill in video & audio co
*/
void setupMenus(void)
{
    uint32_t nbVid;
    uint32_t maj,mn,pa;
    const char *name;

    nbVid=ADM_ve6_getNbEncoders();
    WIDGET(comboBoxVideo)->clear();
    printf("Found %d video encoder(s)\n",nbVid);
    for(uint32_t i=0;i<nbVid;i++)
    {
        ADM_ve6_getEncoderInfo(i,&name,&maj,&mn,&pa);
        WIDGET(comboBoxVideo)->addItem(name);
    }

    // And A codec

    uint32_t nbAud;

    nbAud=audioEncoderGetNumberOfEncoders();
    printf("Found %d audio encoder(s)\n",nbAud);
    WIDGET(comboBoxAudio)->clear();
    for(uint32_t i=0;i<nbAud;i++)
    {
        name=audioEncoderGetDisplayName(i);
        WIDGET(comboBoxAudio)->addItem(name);
    }

    /*   Fill in output format window */
    uint32_t nbFormat=ADM_mx_getNbMuxers();

    printf("Found %d format(s)\n",nbFormat);
    for(uint32_t i=0;i<nbFormat;i++)
    {
        const char *name=ADM_mx_getDisplayName(i);
        WIDGET(comboBoxFormat)->addItem(name);
    }
    WIDGET(pushButtonFormatConfigure)->setEnabled((bool)nbFormat);
}
/*
    Return % of scale (between 0 and 1)
*/
double     UI_readScale( void )
{
    double v;
    if(!slider) v=0;
    v= (double)(slider->value());
    v/=ADM_SCALE_INCREMENT;
    return v;
}
void UI_setScale( double val )
{
    if(_upd_in_progres) return;
    _upd_in_progres++;
    bool old=slider->blockSignals(true);
    slider->setValue( (int)(val * ADM_SCALE_INCREMENT));
    slider->blockSignals(old);
    _upd_in_progres--;
}

int UI_readCurFrame(void)
{
    return 0;
}

int UI_readCurTime(uint16_t &hh, uint16_t &mm, uint16_t &ss, uint16_t &ms)
{
    int success = 0;

    QString timeText = WIDGET(currentTime)->text();
    int pos;

    if (WIDGET(currentTime)->validator()->validate(timeText, pos) == QValidator::Acceptable)
    {
        uint32_t frame;

        hh = (uint16_t)timeText.left(2).toUInt();
        mm = (uint16_t)timeText.mid(3, 2).toUInt();
        ss = (uint16_t)timeText.mid(6, 2).toUInt();
        ms = (uint16_t)timeText.right(3).toUInt();

        time2frame(&frame, currentFps, hh, mm, ss, ms);

        if (frame <= frameCount)
            success = 1;
    }

    return success;
}



//*******************************************

/**
    \fn UI_setTitle(char *name)
    \brief Set the main window title, usually name if the file being edited
*/
void UI_setTitle(const char *name)
{
    char *title;
    const char* defaultTitle = "Avidemux";

    if (name && (*name) )
    {
        title = new char[strlen(defaultTitle) + strlen(name) + 3 + 1];

        strcpy(title, name);
        strcat(title, " - ");
        strcat(title, defaultTitle);
    }
    else
    {
        title = new char[strlen(defaultTitle) + 1];

        strcpy(title, defaultTitle);
    }

    QuiMainWindows->setWindowTitle(QString::fromUtf8(title));
    delete [] title;
}

/**
    \fn     UI_setFrameType( uint32_t frametype,uint32_t qp)
    \brief  Display frametype (I/P/B) and associated quantizer
*/

void UI_setFrameType( uint32_t frametype,uint32_t qp)
{
    char string[100];
    char    c='?';
    const char *f="???";
    switch(frametype&AVI_FRAME_TYPE_MASK)
    {
    case AVI_KEY_FRAME: c='I';break;
    case AVI_B_FRAME: c='B';break;
    case 0: c='P';break;
    default:c='?';break;

    }
    switch(frametype&AVI_STRUCTURE_TYPE_MASK)
    {
    case AVI_TOP_FIELD+AVI_FIELD_STRUCTURE: f="TFF";break;
    case AVI_BOTTOM_FIELD+AVI_FIELD_STRUCTURE: f="BFF";break;
    case AVI_FRAME_STRUCTURE: f="FRM";break;
    default:f="???";

                    break;
    }
    sprintf(string,QT_TRANSLATE_NOOP("qgui2","%c-%s (%02d)"),c,f,qp);
    WIDGET(label_8)->setText(string);

}
/**
 * 
 * @return 
 */
admUITaskBarProgress *UI_getTaskBarProgress()
{
    return QuiTaskBarProgress;
}
/**
    \fn     UI_updateFrameCount(uint32_t curFrame)
    \brief  Display the current displayed frame #
*/
void UI_updateFrameCount(uint32_t curFrame)
{

}

/**
    \fn      UI_setFrameCount(uint32_t curFrame,uint32_t total)
    \brief  Display the current displayed frame # and total frame #
*/
void UI_setFrameCount(uint32_t curFrame,uint32_t total)
{

}

/**
    \fn     UI_updateTimeCount(uint32_t curFrame,uint32_t fps)
    \brief  Display the time corresponding to current frame according to fps (fps1000)
*/
void UI_updateTimeCount(uint32_t curFrame,uint32_t fps)
{
    char text[80];
    uint32_t mm,hh,ss,ms;

    frame2time(curFrame,fps, &hh, &mm, &ss, &ms);
    sprintf(text, "%02d:%02d:%02d.%03d", hh, mm, ss, ms);
    WIDGET(currentTime)->setText(text);
}



/**
    \fn UI_setCurrentTime
    \brief Set current PTS of displayed video
*/
void UI_setCurrentTime(uint64_t curTime)
{
  char text[80];
 uint32_t mm,hh,ss,ms;
 uint32_t shorty=(uint32_t)(curTime/1000);

    ms2time(shorty,&hh,&mm,&ss,&ms);
      sprintf(text, "%02d:%02d:%02d.%03d", hh, mm, ss, ms);
    WIDGET(currentTime)->setText(text);

}

/**
    \fn UI_setTotalTime
    \brief SEt the total duration of video
*/
void UI_setTotalTime(uint64_t curTime)
{
  char text[80];
 uint32_t mm,hh,ss,ms;
 uint32_t shorty=(uint32_t)(curTime/1000);

    ms2time(shorty,&hh,&mm,&ss,&ms);
      sprintf(text, "/ %02d:%02d:%02d.%03d", hh, mm, ss, ms);
    WIDGET(totalTime)->setText(text);
    slider->setTotalDuration(curTime);
}
/**
    \fn     UI_setMarkers(uint64_t Ptsa, uint32_t Ptsb )
    \brief  Display frame # for marker A & B
*/
void UI_setMarkers(uint64_t a, uint64_t b)
{
    char text[80];
    uint64_t absoluteA=a,absoluteB=b;
    uint32_t hh,mm,ss,ms;
    uint32_t timems;
    a/=1000;
    b/=1000;

    timems=(uint32_t)(a);
    ms2time(timems,&hh,&mm,&ss,&ms);
    snprintf(text,79,"%02" PRIu32":%02" PRIu32":%02" PRIu32".%03" PRIu32,hh,mm,ss,ms);
    WIDGET(pushButtonJumpToMarkerA)->setText(text);

    timems=(uint32_t)(b);
    ms2time(timems,&hh,&mm,&ss,&ms);
    snprintf(text,79,"%02" PRIu32":%02" PRIu32":%02" PRIu32".%03" PRIu32,hh,mm,ss,ms);
    WIDGET(pushButtonJumpToMarkerB)->setText(text);

    timems=(uint32_t)(b-a);
    ms2time(timems,&hh,&mm,&ss,&ms);
    snprintf(text,79,"%02" PRIu32":%02" PRIu32":%02" PRIu32".%03" PRIu32,hh,mm,ss,ms);
    QString duration=QString("Selection: ")+QString(text);
    WIDGET(selectionDuration)->setText(duration);

    slider->setMarkers(absoluteA, absoluteB);
}

/**
    \fn     UI_getCurrentVCodec(void)
    \brief  Returns the current selected video code in menu, i.e its number (0 being the first)
*/
int     UI_getCurrentVCodec(void)
{
    int i=WIDGET(comboBoxVideo)->currentIndex();
    if(i<0) i=0;
    return i;
}
/**
    \fn     UI_setVideoCodec( int i)
    \brief  Select the video codec which is # x in pulldown menu (starts at zero :copy)
*/

void UI_setVideoCodec( int i)
{
    int b=!!i;
    WIDGET(comboBoxVideo)->setCurrentIndex(i);

    WIDGET(pushButtonVideoConf)->setEnabled(b);
    WIDGET(pushButtonVideoFilter)->setEnabled(b);
}
/**
    \fn     UI_getCurrentACodec(void)
    \brief  Returns the current selected audio code in menu, i.e its number (0 being the first)
*/

int     UI_getCurrentACodec(void)
{
    int i=WIDGET(comboBoxAudio)->currentIndex();
    if(i<0) i=0;
    return i;
}
/**
    \fn     UI_setAudioCodec( int i)
    \brief  Select the audio codec which is # x in pulldown menu (starts at zero :copy)
*/

void UI_setAudioCodec( int i)
{ int b=!!i;
WIDGET(comboBoxAudio)->setCurrentIndex(i);
WIDGET(pushButtonAudioConf)->setEnabled(b);
WIDGET(pushButtonAudioFilter)->setEnabled(b);
}
/**
    \fn     UI_GetCurrentFormat(void)
    \brief  Returns the current selected output format
*/

int     UI_GetCurrentFormat( void )
{
    int i=WIDGET(comboBoxFormat)->currentIndex();
    if(i<0) i=0;
    return (int)i;
}
/**
    \fn     UI_SetCurrentFormat( ADM_OUT_FORMAT fmt )
    \brief  Select  output format
*/
void     UI_SetCurrentFormat( uint32_t fmt )
{
    WIDGET(comboBoxFormat)->setCurrentIndex((int)fmt);
}

/**
      \fn UI_getTimeShift
      \brief get state (on/off) and value for time Shift
*/
bool UI_getTimeShift(int *onoff,int *value)
{
    if(WIDGET(checkBox_TimeShift)->checkState()==Qt::Checked)
        *onoff=1;
    else
        *onoff=0;
    *value=WIDGET(spinBox_TimeValue)->value();
    return 1;
}
/**
      \fn UI_setTimeShift
      \brief get state (on/off) and value for time Shift
*/

bool UI_setTimeShift(int onoff,int value)
{
    if (onoff && value)
        WIDGET(checkBox_TimeShift)->setCheckState(Qt::Checked);
    else
        WIDGET(checkBox_TimeShift)->setCheckState(Qt::Unchecked);
    WIDGET(spinBox_TimeValue)->setValue(value);
    return 1;
}
/**
    \fn UI_setVUMeter
*/
bool UI_setVUMeter( uint32_t volume[6])
{
    UI_vuUpdate( volume);
    return true;
}

/**
    \fn UI_setVolume
*/
bool UI_setVolume(void)
{
    if(WIDGET(toolButtonAudioToggle)->isChecked())
        ((MainWindow *)QuiMainWindows)->volumeChange(0);
    else
        AVDM_setVolume(0);
    return true;
}

/**
    \fn UI_setDecoderName
*/
bool UI_setDecoderName(const char *name)
{
    WIDGET(labelVideoDecoder)->setText(name);
    return true;
}
/**
 * \fn UI_setDisplayName
 * \brief display current displayEngine name
 */
bool UI_setDisplayName(const char *name)
{
    WIDGET(labelDisplay)->setText(name);
    return true;
}

/**
    \fn UI_hasOpengl
*/
bool UI_hasOpenGl(void)
{
#ifndef USE_OPENGL
    return false;
#else
    if(!ADM_glHasActiveTexture()) return false; // ADM_setActiveTexure
    bool enabled;
    prefs->get(FEATURES_ENABLE_OPENGL,&enabled);
    return enabled;
#endif
}
/**
    \fn UI_iconify
*/
void UI_iconify( void )
{
    uiIsMaximized=QuiMainWindows->isMaximized();
    QuiMainWindows->hide();

}
/**
    \fn UI_deiconify
*/
void UI_deiconify( void )
{
    if(uiIsMaximized)
    {
        QuiMainWindows->showMaximized();
    }else
    {
        QuiMainWindows->showNormal();
    }
}
/**
 *    \fn UI_resize
 *    \brief resize the main window for the given dimensions of the video widget
 */
void UI_resize(uint32_t w,uint32_t h)
{
    if(((MainWindow *)QuiMainWindows)->getBlockResizingFlag())
        return;
    uint32_t reqw, reqh;
    ((MainWindow *)QuiMainWindows)->calcDockWidgetDimensions(reqw,reqh);
    reqw += w;
    reqh += h;
    UI_setBlockZoomChangesFlag(true);
    QuiMainWindows->resize(reqw,reqh);
    UI_setBlockZoomChangesFlag(false);
    ((MainWindow *)QuiMainWindows)->setResizeThreshold(RESIZE_THRESHOLD);
    ADM_info("Resizing the main window to %dx%d px\n",reqw,reqh);
}

/**
    \fn UI_getNeedsResizingFlag
*/
bool UI_getNeedsResizingFlag(void)
{
    return needsResizing;
}

/**
    \fn UI_setNeedsResizingFlag
*/
void UI_setNeedsResizingFlag(bool resize)
{
    needsResizing=resize;
}

/**
    \fn UI_setBlockZoomChangesFlag
*/
void UI_setBlockZoomChangesFlag(bool block)
{
    ((MainWindow *)QuiMainWindows)->setBlockZoomChangesFlag(block);
}

/**
    \fn UI_resetZoomThreshold
*/
void UI_resetZoomThreshold(void)
{
    ((MainWindow *)QuiMainWindows)->setResizeThreshold(RESIZE_THRESHOLD);
    ((MainWindow *)QuiMainWindows)->setActZoomCalledFlag(true);
}

/**
    \fn UI_setAudioTrackCount
*/
void UI_setAudioTrackCount( int nb )
{
    char txt[50];
    sprintf(txt,QT_TRANSLATE_NOOP("qgui2"," (%d track(s))"),nb);
    WIDGET(TrackCountLabel)->setText(QString::fromUtf8(txt));
}
/**
 * \fn dtor
 */
myQApplication::~myQApplication()
{
    
    ADM_warning("Cleaning render...\n");
    renderDestroy();
    ADM_warning("Cleaning preview...\n");
    admPreview::cleanUp();
    ADM_ExitCleanup();    
    
#if defined( USE_VDPAU)
  #if (ADM_UI_TYPE_BUILD!=ADM_UI_CLI)
    ADM_warning("cleaning VDPAU...\n");
    vdpauCleanup();
  #else
    ADM_info("Cannot use VDPAU in cli mode %d,%d\n",ADM_UI_TYPE_BUILD,ADM_UI_CLI);
  #endif
#endif

    
#ifdef SDL_ON_LINUX
    ADM_warning("This is SDL on linux, exiting brutally to avoid lock.\n");
    ::exit(0); // 
#endif    
    ADM_warning("Exiting app\n");
}
//********************************************
//EOF
