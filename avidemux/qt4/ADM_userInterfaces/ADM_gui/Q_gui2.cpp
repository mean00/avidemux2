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
#if QT_VERSION < QT_VERSION_CHECK(5,11,0)
#   include <QDesktopWidget>
#else
#   include <QScreen>
#endif
#include <QClipboard>
#ifdef USE_CUSTOM_TIME_DISPLAY_FONT
#   include <QFontDatabase>
#endif

#ifdef __APPLE__
    #include <QFileOpenEvent>
#endif

#include "ADM_cpp.h"
#define MENU_DECLARE
#include "Q_gui2.h"
#include "ADM_default.h"
#include "ADM_toolkitQt.h"
#include "ADM_QSettings.h"

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
extern bool A_loadDefaultSettings(void);

extern int ADM_clearQtShellHistory(void);
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

class FileDropEvent : public QEvent
{
public:
    QList<QUrl> files;

    FileDropEvent(QList<QUrl> files) : QEvent(QEvent::User)
    {
        this->files = files;
    }
};

#ifdef __APPLE__
/**
 *  \fn event
 *  \brief Queue requests from Finder to open files e.g. when they are dropped onto Avidemux icon in the dock
 */
bool myQApplication::event(QEvent *event)
{
    if(event->type() == QEvent::FileOpen)
    {
        QFileOpenEvent *openEvent = static_cast<QFileOpenEvent *>(event);
        ADM_info("FileOpen event for \"%s\"\n",openEvent->file().toUtf8().constData());
        fileOpenQueue.append(openEvent->url());
        handleFileOpenRequests();
    }
    return QApplication::event(event);
}
/**
 *  \fn handleFileOpenRequests
 */
void myQApplication::handleFileOpenRequests(void)
{
    if(QuiMainWindows && ready && fileOpenQueue.size())
    {
        MainWindow *mw = reinterpret_cast<MainWindow *>(QuiMainWindows);
        mw->fileOpenWrapper(fileOpenQueue);
        fileOpenQueue.clear();
    }
}
#endif

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
    if (dragWhilePlay)
        sendAction(ACT_PlayAvi); // resume playback
}
/**
 * \fn sliderPressed
 */
void MainWindow::sliderPressed(void)
{
    if(playing)
        sendAction(ACT_PlayAvi); // stop playback
    dragWhilePlay=playing;
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
/**
 * \fn thumbSlider_valueEmitted
 * \brief Slot to handle signals from the thumb slider.
 */
void MainWindow::thumbSlider_valueEmitted(int value)
{
    if (!avifileinfo)
    {
        if (value)
            thumbSlider->reset();
        return;
    }

    if (playing)
    {
        if (getPreviewMode() != ADM_PREVIEW_NONE)
            return;
        sendAction(ACT_PlayAvi); // stop playback;
        dragWhilePlay = true;
        return;
    } else if (dragWhilePlay && !value)
    {
        dragWhilePlay = false;
        if (getPreviewMode() == ADM_PREVIEW_NONE)
            sendAction(ACT_PlayAvi); // resume playback;
        return;
    }

    if (!value) return;

    bool success = true;
    if (value > 0)
        success = admPreview::nextKeyFrame();
    else
        success = admPreview::previousKeyFrame();
    if (success)
    {
        uint64_t total = video_body->getVideoDuration();
        if (total)
        {
            uint64_t pts = admPreview::getCurrentPts();
            UI_setCurrentTime(pts);

            double percentage = pts;
            percentage /= total;
            percentage *= 100;

            UI_setScale(percentage);
        }
        return;
    }
    thumbSlider->reset();
    dragWhilePlay = false;
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
    \fn busyTimerTimeout
*/
void MainWindow::busyTimerTimeout(void)
{
    if ((busyCntr == 0) && QApplication::overrideCursor())
        QApplication::restoreOverrideCursor();
}

/**
    \fn actionSlot
*/
void MainWindow::actionSlot(Action a)
{
    if(a==ACT_PlayAvi) // ugly
    {
        playing=1-playing;
        setMenuItemsEnabledState();
        playing=1-playing;
    }
    if(a>ACT_NAVIGATE_BEGIN && a<ACT_NAVIGATE_END)
    {
        busyTimer.stop();
        busyCntr++;
        if (!QApplication::overrideCursor())
            QApplication::setOverrideCursor(Qt::WaitCursor);
    }
    actionLock++;
    HandleAction(a);
    actionLock--;
    setMenuItemsEnabledState();
    if(a>ACT_NAVIGATE_BEGIN && a<ACT_NAVIGATE_END)
    {
        busyCntr--;
        if (busyCntr==0)
            busyTimer.start(100);
    }
}

/**
    \fn sendAction
*/
void MainWindow::sendAction(Action a)
{
    if(a>ACT_NAVIGATE_BEGIN && a<ACT_NAVIGATE_END && a!=ACT_Scale)
    {
        if (actionLock<=NAVIGATION_ACTION_LOCK_THRESHOLD)
            emit actionSignal(a);
    } else {
        //printf("Sending internal event %d\n",(int)a);
        emit actionSignal(a);
    }
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
    actionLock = 0;
    busyCntr = 0;
    busyTimer.setSingleShot(true);

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
        connect( &busyTimer, SIGNAL(timeout()), this, SLOT(busyTimerTimeout()));
    

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
#if 0 /* it is read-only */
    QRegExp timeRegExp("^[0-9]{2}:[0-5][0-9]:[0-5][0-9]\\.[0-9]{3}$");
    QRegExpValidator *timeValidator = new QRegExpValidator(timeRegExp, this);
    ui.currentTime->setValidator(timeValidator);
    ui.currentTime->setInputMask("99:99:99.999");
#endif
    // set the size of the current time display to fit the content
    QString text = "00:00:00.000"; // Don't translate this.
#ifdef USE_CUSTOM_TIME_DISPLAY_FONT
    ui.currentTime->setFont(QFont("ADM7SEG"));
#endif
    ui.currentTime->setText(text); // Override ui translations to make sure we use point as decimal separator.
    QRect ctrect = ui.currentTime->fontMetrics().boundingRect(text);
    ui.currentTime->setFixedSize(1.15 * ctrect.width(), ui.currentTime->height());

    text = QString("/ ") + text;
    ui.totalTime->setText(text); // Override ui translations here too.

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

    // Add action to show all dock widgets and move the toolbar to its default area
    QAction *restoreDefaults = new QAction(QT_TRANSLATE_NOOP("qgui2","Restore defaults"),this);
    ui.menuToolbars->addSeparator();
    ui.menuToolbars->addAction(restoreDefaults);

    connect(ui.menuToolbars->actions().last(),SIGNAL(triggered(bool)),this,SLOT(restoreDefaultWidgetState(bool)));

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
    ui.currentTime->setTextMargins(0,0,0,0); // some Qt themes mess with text margins

    threshold = RESIZE_THRESHOLD;
    actZoomCalled = false;
    ignoreResizeEvent = false;
    blockResizing = false;
    blockZoomChanges = true;
    dragWhilePlay = false;

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

static toolBarTranslate toolbar[]=
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
    \fn findActionInToolBar
*/
static QAction *findActionInToolBar(QToolBar *tb, Action action)
{
    toolBarTranslate *t = toolbar;
    const char *name = NULL;
    while(t->name)
    {
        if(t->event != action)
        {
            t++;
            continue;
        }
        name = t->name;
        break;
    }
    if(!name)
        return NULL;

    QAction *a = NULL;
    for(int i=0; i < tb->actions().size(); i++)
    {
        a = tb->actions().at(i);
        QString s = a->objectName();
        if(s.isEmpty())
            continue;
        if(!strcmp(s.toUtf8().constData(), name))
            return a;
    }
    return NULL;
}

/**
    \fn getMenuEntryForAction
*/
static const MenuEntry *getMenuEntryForAction(std::vector<MenuEntry> *list, QAction *action)
{
    for(int i=0; i < list->size(); i++)
    {
        MenuEntry *candidate = &list->at(i);
        if(candidate->cookie == (void *)action)
            return candidate;
    }
    return NULL;
}

/**
    \fn findAction
*/
static QAction *findAction(std::vector<MenuEntry> *list, Action action)
{
    for(int i=0; i < list->size(); i++)
    {
        MenuEntry *m = &list->at(i);
        if(m->type != MENU_ACTION && m->type != MENU_SUBACTION) continue;
        if(m->event != action) continue;
        QAction *a = (QAction *)m->cookie;
        return a;
    }
    return NULL;
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
                        ADM_assert(a);
                        a->setObjectName(m->text.c_str());
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
                                    case ACT_ResetMarkerA:
                                        prefs->get(KEYBOARD_SHORTCUTS_ALT_RESET_MARK_A,sc);
                                        break;
                                    case ACT_ResetMarkerB:
                                        prefs->get(KEYBOARD_SHORTCUTS_ALT_RESET_MARK_B,sc);
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
                                    case ACT_Delete:
                                        prefs->get(KEYBOARD_SHORTCUTS_ALT_DELETE,sc);
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

    QAction *a = findAction(&myMenuVideo, ACT_PreviewChanged);
    if(a)
        a->setCheckable(true);

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
#define PUSH_LOADED(x,act) { QAction *a = findAction(&myMenu ##x,act); if(a) ActionsAvailableWhenFileLoaded.push_back(a); }
    PUSH_LOADED(File, ACT_APPEND_VIDEO)
    PUSH_LOADED(File, ACT_SAVE_VIDEO)
    PUSH_LOADED(File, ACT_SAVE_QUEUE)

    PUSH_LOADED(File, ACT_SAVE_BMP)
    PUSH_LOADED(File, ACT_SAVE_PNG)
    PUSH_LOADED(File, ACT_SAVE_JPG)
    PUSH_LOADED(File, ACT_SAVE_BUNCH_OF_JPG)

    PUSH_LOADED(File, ACT_CLOSE)
    PUSH_LOADED(File, ACT_VIDEO_PROPERTIES)

    PUSH_LOADED(Edit, ACT_Copy)

    PUSH_LOADED(Edit, ACT_MarkA)
    PUSH_LOADED(Edit, ACT_MarkB)

    PUSH_LOADED(View, ACT_ZOOM_1_4)
    PUSH_LOADED(View, ACT_ZOOM_1_2)
    PUSH_LOADED(View, ACT_ZOOM_1_1)
    PUSH_LOADED(View, ACT_ZOOM_2_1)
    PUSH_LOADED(View, ACT_ZOOM_FIT_IN)

#define PUSH_LOADED_TOOLBAR(action) { QAction *a = findActionInToolBar(ui.toolBar, action); if(a) ActionsAvailableWhenFileLoaded.push_back(a); }
    if(ADM_mx_getNbMuxers()) // Don't enable "Save" if we've got zero muxers.
        PUSH_LOADED_TOOLBAR(ACT_SAVE_VIDEO)
    PUSH_LOADED_TOOLBAR(ACT_VIDEO_PROPERTIES)
    PUSH_LOADED_TOOLBAR(ACT_PreviewChanged)

#define PUSH_FULL_MENU_LOADED(x,tailOffset) for(int i=0;i<ui.x->actions().size()-tailOffset;i++) \
    { QAction *a = ui.x->actions().at(i); if(a->objectName().isEmpty()) continue; ActionsAvailableWhenFileLoaded.push_back(a); }
#define PUSH_FULL_MENU_PLAYBACK(x,tailOffset) for(int i=0;i<ui.x->actions().size()-tailOffset;i++) \
    { QAction *a = ui.x->actions().at(i); if(a->objectName().isEmpty()) continue; ActionsDisabledOnPlayback.push_back(a); }

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

#define PUSH_ALWAYS_AVAILABLE(menu,event) { QAction *a = findAction(&myMenu ##menu, event); if(a) ActionsAlwaysAvailable.push_back(a); }

    PUSH_ALWAYS_AVAILABLE(File, ACT_OPEN_VIDEO)
    PUSH_ALWAYS_AVAILABLE(File, ACT_AVS_PROXY)
    PUSH_ALWAYS_AVAILABLE(File, ACT_EXIT)

    PUSH_ALWAYS_AVAILABLE(Edit, ACT_PREFERENCES)
    PUSH_ALWAYS_AVAILABLE(Edit, ACT_SaveAsDefault)
    PUSH_ALWAYS_AVAILABLE(Edit, ACT_LoadDefault)

#define PUSH_ALWAYS_AVAILABLE_TOOLBAR(event) { QAction *a = findActionInToolBar(ui.toolBar, event); if(a) ActionsAlwaysAvailable.push_back(a); }

    PUSH_ALWAYS_AVAILABLE_TOOLBAR(ACT_OPEN_VIDEO)

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
    ADD_BUTTON_LOADED(toolButtonPreviousCutPoint)
    ADD_BUTTON_LOADED(toolButtonNextCutPoint)
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
    ADD_BUTTON_PLAYBACK(toolButtonPreviousCutPoint)
    ADD_BUTTON_PLAYBACK(toolButtonNextCutPoint)
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

    bool vid, undo, redo, paste, resetA, resetB;
    vid = undo = redo = paste = resetA = resetB = false;
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

#define ENABLE(x,y,z) { QAction *a = findAction(&myMenu ##x, y); if(a) a->setEnabled(z); }
#define TOOLBAR_ENABLE(x,y) { QAction *a = findActionInToolBar(ui.toolBar, x); if(a) a->setEnabled(y); }
    ENABLE(File, ACT_SAVE_VIDEO, vid && ADM_mx_getNbMuxers()) // disable saving video if there are no muxers
    if(vid)
    {
        undo=video_body->canUndo();
        redo=video_body->canRedo();
        if(video_body->getMarkerAPts())
            resetA = true;
        if(video_body->getMarkerBPts() != video_body->getVideoDuration())
            resetB = true;
        paste=!video_body->clipboardEmpty();
    }
    ENABLE(Edit, ACT_Undo, undo)
    ENABLE(Edit, ACT_Redo, redo)
    ENABLE(Edit, ACT_ResetSegments, vid)
    // TODO: Detect that segment layout matches the default one and disable "Reset Edit" then too.
    ENABLE(Edit, ACT_ResetMarkerA, resetA)
    ENABLE(Edit, ACT_ResetMarkerB, resetB)
    ENABLE(Edit, ACT_ResetMarkers, (resetA || resetB))

    ENABLE(Edit, ACT_Cut, (resetA || resetB))
    ENABLE(Edit, ACT_Delete, (resetA || resetB))
    ENABLE(Edit, ACT_Paste, paste)

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
    ENABLE(Recent, ACT_CLEAR_RECENT, haveRecentItems)
    ENABLE(Recent, ACT_RESTORE_SESSION, A_checkSavedSession(false))

    ui.selectionDuration->setEnabled(vid);
    slider->setEnabled(vid);

    updateCodecWidgetControlsState();
    // actions performed by the code above may result in a window resize event,
    // which in turn may initiate unwanted zoom changes e.g. when stopping playback
    // or loading a video with small dimensions, so ignore just this one resize event
    ignoreResizeEvent = true;
    // en passant reset frame type label if no video is loaded
    if(!vid)
        ui.label_8->setText(QT_TRANSLATE_NOOP("qgui2","?"));
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
    ENABLE(Video, ACT_DecoderOption, b)
    // post-processing is available only for software decoding
    b=false;
    if(avifileinfo && strcmp(video_body->getVideoDecoderName(),"VDPAU")
                   && strcmp(video_body->getVideoDecoderName(),"LIBVA")
                   && strcmp(video_body->getVideoDecoderName(),"DXVA2"))
                   // VideoToolbox decoder always downloads decoded image immediately
        b=true;
    ENABLE(Video, ACT_SetPostProcessing, b)

    b=false;
    if(ui.comboBoxVideo->currentIndex())
        b=true;
    ui.pushButtonVideoConf->setEnabled(b);
    if(avifileinfo)
    {
        ui.pushButtonVideoFilter->setEnabled(b);
        // take care of the "Filter" item in the menu "Video" as well
        ENABLE(Video, ACT_VIDEO_FILTERS, b)
        ENABLE(Video, ACT_PreviewChanged, b)
        TOOLBAR_ENABLE(ACT_PreviewChanged, b)
    }else
    {
        ui.pushButtonVideoFilter->setEnabled(false);
        ENABLE(Video, ACT_VIDEO_FILTERS, false)
        ENABLE(Video, ACT_PreviewChanged, false)
        TOOLBAR_ENABLE(ACT_PreviewChanged, false)
    }

    b=false;
    if(avifileinfo && video_body->getDefaultEditableAudioTrack())
    {
        ENABLE(Audio, ACT_SAVE_AUDIO, true)
        if(ui.comboBoxAudio->currentIndex())
            b=true;
    }else
    {   // disable "Save Audio" item in the menu "Audio" if we have no audio tracks
        ENABLE(Audio, ACT_SAVE_AUDIO, false)
    }
    ui.pushButtonAudioConf->setEnabled(b);
    ui.pushButtonAudioFilter->setEnabled(b);
    // take care of the "Filter" item in the menu "Audio"
    ENABLE(Audio, ACT_AUDIO_FILTERS, b)
#undef ENABLE
    // reenable the controls below unconditionally after playback
    ui.checkBox_TimeShift->setEnabled(true);
    ui.spinBox_TimeValue->setEnabled(true);
    // disable the "Save" button in the toolbar and the "Configure" button for the output format if we have no muxers
    b=false;
    bool gotMuxers=(bool)ADM_mx_getNbMuxers();
    if(avifileinfo && gotMuxers)
        b=true;
    TOOLBAR_ENABLE(ACT_SAVE_VIDEO, b)
    ui.pushButtonFormatConfigure->setEnabled(gotMuxers);
}

/**
    \fn updateActionShortcuts
*/
void MainWindow::updateActionShortcuts(void)
{
    bool alt=false, swpud=false;
    prefs->get(KEYBOARD_SHORTCUTS_USE_ALTERNATE_KBD_SHORTCUTS,&alt);
    prefs->get(KEYBOARD_SHORTCUTS_SWAP_UP_DOWN_KEYS,&swpud);

    {
        QAction *q;

        q = findAction(&myMenuGo, ACT_PreviousKFrame);
        if(q)
            q->setShortcut(swpud ? Qt::Key_Up : Qt::Key_Down);

        q = findAction(&myMenuGo, ACT_NextKFrame);
        if(q)
            q->setShortcut(swpud ? Qt::Key_Down : Qt::Key_Up);

        q = findAction(&myMenuGo, ACT_PrevCutPoint);
        if(q)
            q->setShortcut(Qt::SHIFT | (swpud ? Qt::Key_Up : Qt::Key_Down));

        q = findAction(&myMenuGo, ACT_NextCutPoint);
        if(q)
            q->setShortcut(Qt::SHIFT | (swpud ? Qt::Key_Down : Qt::Key_Up));
    }

    std::vector<MenuEntry *> defaultShortcuts;

    for(int i=0; i < myMenuEdit.size(); i++)
    {
        MenuEntry *m = &myMenuEdit[i];
        // The separator is number 7, but this is a bit more readable
        if(m->type != MENU_ACTION)
            continue;
        if(m->event == ACT_PREFERENCES)
            break;
        switch(m->event)
        {
            case ACT_Delete:
            case ACT_MarkA:
            case ACT_MarkB:
            case ACT_ResetMarkerA:
            case ACT_ResetMarkerB:
            case ACT_ResetMarkers:
                defaultShortcuts.push_back(m);
                break;
            default:break;
        }
    }

    for(int i=0; i < myMenuGo.size(); i++)
    {
        MenuEntry *m = &myMenuGo[i];
        if(m->type != MENU_ACTION)
            continue;
        if(m->event == ACT_GotoTime)
            break;
        switch(m->event)
        {
            case ACT_Begin:
            case ACT_End:
            case ACT_GotoMarkA:
            case ACT_GotoMarkB:
                defaultShortcuts.push_back(m);
                break;
            default:break;
        }
    }

    int n = defaultShortcuts.size();
    for(int i=0;i<n;i++)
    {
        MenuEntry *m=defaultShortcuts.at(i);
        if(!m) continue;
        QAction *a=(QAction *)m->cookie;
        if(!a) continue;
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
                case ACT_ResetMarkerA:
                    prefs->get(KEYBOARD_SHORTCUTS_ALT_RESET_MARK_A,sc);
                    break;
                case ACT_ResetMarkerB:
                    prefs->get(KEYBOARD_SHORTCUTS_ALT_RESET_MARK_B,sc);
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
                case ACT_Delete:
                    prefs->get(KEYBOARD_SHORTCUTS_ALT_DELETE,sc);
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
    \fn getActionShortcutString
*/
static QString getActionShortcutString(QMenu *menu, std::vector<MenuEntry> *list, Action action)
{
    QString s = " ";
    for(int i=0; i < menu->actions().size(); i++)
    {
        QAction *a = menu->actions().at(i);
        const MenuEntry *m = getMenuEntryForAction(list, a);
        if(!m) continue;
        if(m->type != MENU_ACTION) continue;
        if(m->event != action) continue;
        QKeySequence seq = a->shortcut();
        s = seq.toString().toUpper();
        break;
    }
    return s;
}

/**
    \fn widgetsUpdateTooltips
    \brief Update tooltips showing tunable action shortcuts in the navigation and selection widgets
*/
void MainWindow::widgetsUpdateTooltips(void)
{
    QString tt;

#define SHORTCUT(x,y) QString(" [") + getActionShortcutString(ui.menu ##y, &myMenu ##y, x) + QString("]");

    tt = QString(QT_TRANSLATE_NOOP("qgui2","Play/Stop"));
    tt += SHORTCUT(ACT_PlayAvi,Go)
    ui.toolButtonPlay->setToolTip(tt);

    tt = QString(QT_TRANSLATE_NOOP("qgui2","Go to previous frame"));
    tt += SHORTCUT(ACT_PreviousFrame,Go)
    ui.toolButtonPreviousFrame->setToolTip(tt);

    tt = QString(QT_TRANSLATE_NOOP("qgui2","Go to next frame"));
    tt += SHORTCUT(ACT_NextFrame,Go)
    ui.toolButtonNextFrame->setToolTip(tt);

    tt = QString(QT_TRANSLATE_NOOP("qgui2","Go to previous keyframe"));
    tt += SHORTCUT(ACT_PreviousKFrame,Go)
    ui.toolButtonPreviousIntraFrame->setToolTip(tt);

    tt = QString(QT_TRANSLATE_NOOP("qgui2","Go to next keyframe"));
    tt += SHORTCUT(ACT_NextKFrame,Go)
    ui.toolButtonNextIntraFrame->setToolTip(tt);

    tt = QString(QT_TRANSLATE_NOOP("qgui2","Set start marker"));
    tt += SHORTCUT(ACT_MarkA,Edit)
    ui.toolButtonSetMarkerA->setToolTip(tt);

    tt = QString(QT_TRANSLATE_NOOP("qgui2","Set end marker"));
    tt += SHORTCUT(ACT_MarkB,Edit)
    ui.toolButtonSetMarkerB->setToolTip(tt);

    tt = QString(QT_TRANSLATE_NOOP("qgui2","Go to previous cut point"));
    tt += SHORTCUT(ACT_PrevCutPoint,Go)
    ui.toolButtonPreviousCutPoint->setToolTip(tt);

    tt = QString(QT_TRANSLATE_NOOP("qgui2","Go to next cut point"));
    tt += SHORTCUT(ACT_NextCutPoint,Go)
    ui.toolButtonNextCutPoint->setToolTip(tt);

    // go to black frame tooltips are static, the actions don't have shortcuts

    tt = QString(QT_TRANSLATE_NOOP("qgui2","Go to first frame"));
    tt += SHORTCUT(ACT_Begin,Go)
    ui.toolButtonFirstFrame->setToolTip(tt);

    tt = QString(QT_TRANSLATE_NOOP("qgui2","Go to last frame"));
    tt += SHORTCUT(ACT_End,Go)
    ui.toolButtonLastFrame->setToolTip(tt);

    tt = QString(QT_TRANSLATE_NOOP("qgui2","Go to marker A"));
    tt += SHORTCUT(ACT_GotoMarkA,Go)
    ui.pushButtonJumpToMarkerA->setToolTip(tt);

    tt = QString(QT_TRANSLATE_NOOP("qgui2","Go to marker B"));
    tt += SHORTCUT(ACT_GotoMarkB,Go)
    ui.pushButtonJumpToMarkerB->setToolTip(tt);

#undef SHORTCUT

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
    \fn     restoreDefaultWidgetState
    \brief  Show all dock widgets and move toolbar to the default area
*/
void MainWindow::restoreDefaultWidgetState(bool b)
{
    ui.codecWidget->setVisible(true);
    ui.navigationWidget->setVisible(true);
    ui.selectionWidget->setVisible(true);
    ui.volumeWidget->setVisible(true);
    ui.audioMetreWidget->setVisible(true);
    ui.toolBar->setVisible(true);

    syncToolbarsMenu();

    addToolBar(ui.toolBar);

    if(!playing)
        setZoomToFit();
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
                        if (keyEvent->modifiers() & Qt::ShiftModifier)
                        {
                            if(!swpud)
                                sendAction(ACT_NextCutPoint);
                            else
                                sendAction(ACT_PrevCutPoint);
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
                        if (keyEvent->modifiers() & Qt::ShiftModifier)
                        {
                            if(!swpud)
                                sendAction(ACT_PrevCutPoint);
                            else
                                sendAction(ACT_NextCutPoint);
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
                QSize os = static_cast<QResizeEvent *>(event)->oldSize();
                adjustZoom(os.width(),os.height());
                break;
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

/**
 *  \fn windowStateToString
 */
static const char *windowStateToString(const Qt::WindowStates s)
{
    if(s & Qt::WindowMinimized)
        return "minimized";
    else if(s & Qt::WindowMaximized)
        return "maximized";
    else if(s & Qt::WindowFullScreen)
        return "fullscreen";
    else
        return "normal";
}

// If a video was loaded or zoom changed while the main window was maximized,
// unmaximizing the window restores it to dimensions not necessarily matching
// the actual dimensions of the video frame. Catch the window state change event
// and resize the window when appropriate.
void MainWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::WindowStateChange && false==QuiMainWindows->isMinimized())
    {
        QWindowStateChangeEvent *ev = static_cast<QWindowStateChangeEvent*>(event);
        const Qt::WindowStates old = ev->oldState();
        const Qt::WindowStates cur = QuiMainWindows->windowState();
        ADM_info("Window change event: %s -> %s\n", windowStateToString(old), windowStateToString(cur));

        if (!(old & Qt::WindowMinimized)) // Don't do anything on restore from minimized state.
        {
            if (old & Qt::WindowMaximized)
            {
                if (UI_getNeedsResizingFlag())
                {
                    uint32_t w=ui.frame_video->width();
                    uint32_t h=ui.frame_video->height();
                    UI_resize(w,h);
                    UI_setNeedsResizingFlag(false);
                }else
                {
                    setZoomToFit();
                }
            }
            // Always adjust zoom on maximize from normal state.
            if (!(old & Qt::WindowMaximized) && QuiMainWindows->isMaximized())
            {
                ignoreResizeEvent = false;
                adjustZoom(0,0);
            }
        }
    }
    QWidget::changeEvent(event);
}

/**
 *  \fn adjustZoom
 */
bool MainWindow::adjustZoom(int oldWidth, int oldHeight)
{
    if(blockZoomChanges || playing || !avifileinfo)
        return false;
    if(ignoreResizeEvent)
    {
        ignoreResizeEvent = false;
        return false;
    }

    uint32_t reqw, reqh;
    calcDockWidgetDimensions(reqw,reqh);

    if(QuiMainWindows->width() <= reqw)
        return false;

    uint32_t availw = QuiMainWindows->width() - reqw;

    if(QuiMainWindows->height() <= reqh)
        return false;

    uint32_t availh = QuiMainWindows->height() - reqh;

    uint32_t w = avifileinfo->width;
    uint32_t h = avifileinfo->height;
    if(!w || !h)
        return false;
    // We've got the available space and the video resolution,
    // now calculate the zoom to fit the video into this space.
    float widthRatio = (float)availw / (float)w;
    float heightRatio = (float)availh / (float)h;
    float zoom = (widthRatio < heightRatio ? widthRatio : heightRatio);

    // Detect if the zoom has been likely set automatically on loading a new video or
    // changed using a keyboard shortcut. In this case imitate physical friction
    // requiring the window to be resized beyond certain threshold first.
    float oldzoom = admPreview::getCurrentZoom();
    if(oldWidth && oldHeight && zoom > oldzoom && actZoomCalled)
    {
        if(QuiMainWindows->width() > oldWidth)
            threshold -= QuiMainWindows->width() - oldWidth;
        if(QuiMainWindows->height() > oldHeight)
            threshold -= QuiMainWindows->height() - oldHeight;
        if(threshold > 0)
            return false;
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

        return true;
    }
    return false;
}

/**
 *  \fn setZoomToFit
 *  \brief adjust zoom level to fit video into available space at current window size
 */
void MainWindow::setZoomToFit(void)
{
    ignoreResizeEvent=false;
    adjustZoom(0,0);
    UI_setNeedsResizingFlag(false);
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

/**
    \fn volumeWidgetOperational
*/
void MainWindow::volumeWidgetOperational(void)
{
    // Disable the volume widget if the audio device doesn't support setting volume
    ui.volumeWidget->setEnabled(AVDM_hasVolumeControl());
}

/**
    \fn syncToolbarsMenu
    \brief Make sure only visible widgets have check marks
           in the Toolbars submenu of the View menu.
*/
void MainWindow::syncToolbarsMenu(void)
{
#define EXPAND(x) ui.x ## Widget
#define CHECKMARK(x,y) ui.menuToolbars->actions().at(x)->setChecked(EXPAND(y)->isVisible());
    CHECKMARK(0,audioMetre)
    CHECKMARK(1,codec)
    CHECKMARK(2,navigation)
    CHECKMARK(3,selection)
    CHECKMARK(4,volume)
    ui.menuToolbars->actions().at(5)->setChecked(ui.toolBar->isVisible());
#undef CHECKMARK
#undef EXPAND
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
#if !defined(__APPLE__) && QT_VERSION >= QT_VERSION_CHECK(5,11,0) && QT_VERSION < QT_VERSION_CHECK(6,0,0)
    // Despite HiDPI scaling being supported from Qt 5.6 on, important aspects
    // like OpenGL support were fixed only in much later versions.
    // Enabled by default with Qt6.
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
#if !defined(__APPLE__) && !defined(_WIN32) /* Linux */ && QT_VERSION >= QT_VERSION_CHECK(6,0,0)
    // Fix video shown as solid black color with OpenGL display and Qt6.
    QApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
#endif
#if defined(_WIN32) && QT_VERSION >= QT_VERSION_CHECK(5,10,0)
    // Hide unhelpful context help buttons on Windows.
    QApplication::setAttribute(Qt::AA_DisableWindowContextHelpButton);
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

#ifdef USE_CUSTOM_TIME_DISPLAY_FONT
    if(-1 == QFontDatabase::addApplicationFont(":/new/prefix1/fonts/ADM7SEG.ttf"))
        ADM_warning("LCD display font could not be loaded from resource.\n");
#endif
    loadTranslator();

    return 1;
}

uint8_t initGUI(const vector<IScriptEngine*>& scriptEngines)
{
    MainWindow *mw = new MainWindow(scriptEngines);

    bool openglEnabled = false;
#ifdef USE_OPENGL
    prefs->get(FEATURES_ENABLE_OPENGL,&openglEnabled);
    ADM_info("OpenGL enabled at build time, checking whether we should run it... %s.\n",openglEnabled? "yes" : "no");
#else
    ADM_info("OpenGL: Not enabled at build time.\n");
#endif

    bool vuMeterIsHidden = false;
    bool maximize = false;
    QSettings *qset = qtSettingsCreate();
    if(qset)
    {
        qset->beginGroup("MainWindow");
        mw->restoreState(qset->value("windowState").toByteArray());
        maximize = qset->value("showMaximized", false).toBool();
        qset->endGroup();
        // Hack: allow to drop other Qt-specific settings on application restart
        char *dropSettingsOnLaunch = getenv("ADM_QT_DROP_SETTINGS");
        if(dropSettingsOnLaunch && !strcmp("1",dropSettingsOnLaunch))
            qset->clear();
        delete qset;
        qset = NULL;
        // Probing for OpenGL fails if VU meter is hidden, delay hiding it.
        vuMeterIsHidden = mw->ui.audioMetreWidget->isHidden();
        if(openglEnabled && vuMeterIsHidden)
            mw->ui.audioMetreWidget->setVisible(true);
    }

    QuiMainWindows = (QWidget*)mw;

    if(maximize)
    {
        UI_setBlockZoomChangesFlag(false); // unblock zoom to fit
        QuiMainWindows->showMaximized();
    }else
    {
        QuiMainWindows->show();
    }

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
    if(openglEnabled)
    {
        ADM_info("OpenGL activated, initializing... \n");
        openGLStarted=true;
        UI_Qt4InitGl();
        if(vuMeterIsHidden)
            mw->ui.audioMetreWidget->setVisible(false);
    }else
    {
        ADM_info("OpenGL not activated, not initialized\n");
    }
#endif
mw->syncToolbarsMenu();

    return 1;
}
/**
 * \fn UI_closeGui
 */
void UI_closeGui(void)
{
    if(!uiRunning) return;
    uiRunning=false;

    QSettings *qset = qtSettingsCreate();
    if(qset)
    {
        qset->beginGroup("MainWindow");
        qset->setValue("windowState", ((QMainWindow *)QuiMainWindows)->saveState());
        qset->setValue("showMaximized", QuiMainWindows->isMaximized());
        qset->endGroup();
        delete qset;
        qset = NULL;
    }

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
#ifdef USE_DXVA2
    /* After d3d probing on startup, the host frame holding the video window
    cannot be shrunk to zero size unless the main window is resized, becoming
    visible as a white square if the main window is minimized and restored. */
    {
        int w = QuiMainWindows->width();
        int h = QuiMainWindows->height();
        QuiMainWindows->resize(w+1,h);
        QuiMainWindows->resize(w,h);
    }
#endif
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
    if(qp == ADM_IMAGE_UNKNOWN_QP)
        sprintf(string,QT_TRANSLATE_NOOP("qgui2","%c-%s"),c,f);
    else
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
    \fn UI_setSegments
    \brief SEt segments boundaries
*/
void UI_setSegments(uint32_t numOfSegs, uint64_t * segPts)
{
    slider->setSegments(numOfSegs, segPts);
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
    QString duration=QString::fromUtf8(QT_TRANSLATE_NOOP("qgui2","Selection: "))+QString(text);
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
bool UI_setVUMeter( uint32_t volume[8])
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
    \fn UI_navigationButtonsPressed
    \brief Allow to abstain from opening pop-up dialogs while
           a button with auto-repeat enabled is pressed, else
           the mouse release event gets eaten by the pop-up and
           we keep firing the action assigned to the particular
           button forever.
*/
bool UI_navigationButtonsPressed(void)
{
    if(WIDGET(toolButtonPreviousFrame)->isDown())
        return true;
    if(WIDGET(toolButtonNextFrame)->isDown())
        return true;
    if(WIDGET(toolButtonPreviousIntraFrame)->isDown())
        return true;
    if(WIDGET(toolButtonNextIntraFrame)->isDown())
        return true;
    return false;
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
    ADM_info("Resizing the main window to %dx%d px\n",reqw,reqh);
#ifdef _WIN32
    QRect fs = QuiMainWindows->frameGeometry();
#if QT_VERSION < QT_VERSION_CHECK(5,11,0)
    QRect space = QApplication::desktop()->availableGeometry();
#else
    QRect space = QApplication::primaryScreen()->availableGeometry();
#endif

    int x = fs.x() + fs.width() - space.x() - space.width();
    bool move=false;
    if(x > 0) // the right edge of the window doesn't fit into the screen
    {
        move = true;
        if(x < fs.x())
            x = fs.x() - x;
        else
            x = 0;
    }else
    {
        x = fs.x();
    }
    int y = fs.y() + fs.height() - space.y() - space.height();
    if(y > 0) // the bottom edge of the window doesn't fit into the screen
    {
        move = true;
        if(y < fs.y())
            y = fs.y() - y;
        else
            y = 0;
    }else
    {
        y = fs.y();
    }
    if(move)
    {
        if(x < space.x()) x = space.x(); // adjust for taskbar on the left side
        if(y < space.y()) y = space.y(); // adjust for taskbar on the top
        ADM_info("Moving the main window to position (%d, %d)\n",x,y);
        QuiMainWindows->move(x,y);
    }
#endif
    UI_setBlockZoomChangesFlag(false);
    ((MainWindow *)QuiMainWindows)->setResizeThreshold(RESIZE_THRESHOLD);
}

/**
    \fn UI_getMaximumPreviewSize
    \brief Return maximum width and height available for video preview
*/
void UI_getMaximumPreviewSize(uint32_t *availWidth, uint32_t *availHeight)
{
    QSize frme = QuiMainWindows->frameSize();
    int fwidth = frme.width() - QuiMainWindows->width();
    int fheight = frme.height() - QuiMainWindows->height();
    if(fwidth < 0) fwidth = 0;
    if(fheight < 0) fheight = 0;

    uint32_t reqw, reqh, screenWidth, screenHeight;

    UI_getPhysicalScreenSize(QuiMainWindows, &screenWidth, &screenHeight);
    ((MainWindow *)QuiMainWindows)->calcDockWidgetDimensions(reqw,reqh);

    int w = screenWidth - reqw - fwidth;
    int h = screenHeight - reqh - fheight;
    if(w < 0) w = 0;
    if(h < 0) h = 0;

    // If we have to downscale anyway, leave some margin around the window.
    // Opening a window which takes almost the entire desktop may feel intrusive.
#define SHRINK_FACTOR 0.85
    if(avifileinfo && !QuiMainWindows->isMaximized() && (avifileinfo->width > w || avifileinfo->height > h))
    {
        w = (float)w * SHRINK_FACTOR;
        h = (float)h * SHRINK_FACTOR;
    }
#undef SHRINK_FACTOR
    *availWidth = w;
    *availHeight = h;
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
    \fn UI_setZoomToFitIntoWindow
*/
void UI_setZoomToFitIntoWindow(void)
{
    ((MainWindow *)QuiMainWindows)->setZoomToFit();
}

/**
    \fn UI_setAudioTrackCount
*/
void UI_setAudioTrackCount( int nb )
{
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
    QString text=QCoreApplication::translate("qgui2"," (%n track(s))",NULL,nb);
#else
    QString text=QCoreApplication::translate("qgui2"," (%n track(s))",NULL,QCoreApplication::UnicodeUTF8,nb);
#endif
    WIDGET(TrackCountLabel)->setText(text);
}
/**
 * \fn dtor
 */
myQApplication::~myQApplication()
{
    ADM_clearQtShellHistory();
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
