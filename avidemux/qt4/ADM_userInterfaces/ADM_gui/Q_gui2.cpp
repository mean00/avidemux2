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
#include <QGraphicsView>
#include <QtCore/QDir>
#include <QMessageBox>
#include "ADM_cpp.h"
#define MENU_DECLARE
#include "Q_gui2.h"
#include "ADM_default.h"
#include "ADM_toolkitQt.h"

#include "DIA_fileSel.h"
#include "ADM_vidMisc.h"
#include "prefs.h"
#include "avi_vars.h"

#include "ADM_render/GUI_renderInternal.h"
#include "ADM_coreVideoEncoderInternal.h"
#include "ADM_muxerProto.h"
#include "T_vumeter.h"
#include "DIA_coreToolkit.h"
#include "GUI_ui.h"
#include "config.h"
#include "ADM_preview.h"
#include "DIA_defaultAskAvisynthPort.hxx"
#include "ADM_systemTrayProgress.h"
using namespace std;

#define ADM_SLIDER_REFRESH_PERIOD 500
#define ADM_LARGE_SCALE     (10LL*1000LL)
#define ADM_SCALE_INCREMENT (ADM_LARGE_SCALE/100LL)

#if defined(USE_SDL) && ( !defined(_WIN32) && !defined(__APPLE__))
    #define SDL_ON_LINUX
#endif

#ifdef USE_OPENGL
extern bool ADM_glHasActiveTexture(void);
void UI_Qt4InitGl(void);
void UI_Qt4CleanGl(void);
bool  openGLStarted=false;
#endif

extern int global_argc;
extern char **global_argv;

extern int automation(void );
extern void sendAction(Action a);
extern int encoderGetEncoderCount (void);
extern const char *encoderGetIndexedName (uint32_t i);
uint32_t audioEncoderGetNumberOfEncoders(void);
const char  *audioEncoderGetDisplayName(int i);
extern void checkCrashFile(void);
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
static ADM_QSlider *slider=NULL;
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
extern bool ADM_QPreviewCleanup(void);
extern void vdpauCleanup();


static bool uiRunning=false;

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
	{
		bool b=false;
		if(ui.comboBoxVideo->currentIndex())
		{
			b=true;
		}
		ui.pushButtonVideoConf->setEnabled(b);
		ui.pushButtonVideoFilter->setEnabled(b);
		sendAction (ACT_VIDEO_CODEC_CHANGED) ;
	}
	else if (obj == ui.comboBoxAudio)
	{
		bool b=false;
		if(ui.comboBoxAudio->currentIndex())
		{
			b=true;
		}
		ui.pushButtonAudioConf->setEnabled(b);
		ui.pushButtonAudioFilter->setEnabled(b);
		sendAction (ACT_AUDIO_CODEC_CHANGED) ;
	}
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

void MainWindow::previewModeChanged(int  flop)
{
	sendAction(ACT_PreviewChanged);
}

void MainWindow::timeChangeFinished(void)
{
	this->setFocus(Qt::OtherFocusReason);
}

void MainWindow::currentFrameChanged(void)
{
//	sendAction(ACT_JumpToFrame);

	this->setFocus(Qt::OtherFocusReason);
}

void MainWindow::currentTimeChanged(void)
{
	sendAction(ACT_GotoTime);

	this->setFocus(Qt::OtherFocusReason);
}

/**
    \fn ctor
*/
MainWindow::MainWindow(const vector<IScriptEngine*>& scriptEngines) : _scriptEngines(scriptEngines), QMainWindow()
{
	qtRegisterDialog(this);
	ui.setupUi(this);
        dragState=dragState_Normal;
        refreshCapEnabled=false;
        prefs->get(FEATURES_CAP_REFRESH_ENABLED,&refreshCapEnabled);
        prefs->get(FEATURES_CAP_REFRESH_VALUE,&refreshCapValue);

#if defined(__APPLE__) && defined(USE_SDL)
	//ui.actionAbout_avidemux->setMenuRole(QAction::NoRole);
	//ui.actionPreferences->setMenuRole(QAction::NoRole);
	//ui.actionQuit->setMenuRole(QAction::NoRole);
#endif
        //
        connect( this,SIGNAL(actionSignal(Action )),this,SLOT(actionSlot(Action )));
        //
        connect( ui.checkDisplayOut,SIGNAL(stateChanged(int)),this,SLOT(previewModeChanged(int)));

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
        ADM_QSlider *qslider=(ADM_QSlider *)slider;
	slider->setMinimum(0);
	slider->setMaximum(ADM_LARGE_SCALE);
#if 1
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

	//connect(ui.currentTime, SIGNAL(editingFinished()), this, SLOT(currentTimeChanged()));

    // Build file,... menu
        addScriptEnginesToFileMenu(myMenuFile);
	addScriptShellsToToolsMenu(myMenuTool);
        buildMyMenu();
	buildCustomMenu();
	addScriptReferencesToHelpMenu();

    QString rFiles=QString::fromUtf8(QT_TRANSLATE_NOOP("adm","Recent Files"));
    QString rProjects=QString::fromUtf8(QT_TRANSLATE_NOOP("adm","Recent Projects"));
    
    recentFiles=new QMenu(rFiles, this);
    recentProjects=new QMenu(rProjects, this);
    ui.menuRecent->addMenu(recentFiles);
    ui.menuRecent->addMenu(recentProjects);
    connect(this->recentFiles, SIGNAL(triggered(QAction*)), this, SLOT(searchRecentFiles(QAction*)));
	connect(this->recentProjects, SIGNAL(triggered(QAction*)), this, SLOT(searchRecentProjects(QAction*)));

	this->installEventFilter(this);
	slider->installEventFilter(this);

	//ui.currentTime->installEventFilter(this);

	this->setFocus(Qt::OtherFocusReason);

	setAcceptDrops(true);
        setWindowIcon(QIcon(":/new/prefix1/pics/avidemux_icon_small.png"));

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

	this->adjustSize();
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
    QMenu *subMenu=NULL;
    for(int i=0;i<nb;i++)
    {
        MenuEntry *m=menu+i;
        QString qs=QString::fromUtf8(QT_TRANSLATE_NOOP("adm",m->text.c_str()));
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
                        m->cookie=(void *)a;
                        if(m->shortCut)
                        {
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

    return true;
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
//MKMENU(Recent)
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

	switch (event->type())
	{
		case QEvent::KeyPress:
			keyEvent = (QKeyEvent*)event;

//			if (watched == slider)
			{
				switch (keyEvent->key())
				{
					case Qt::Key_Left:
						if (keyEvent->modifiers() == Qt::ShiftModifier)
							sendAction(ACT_Back1Second);
						else if (keyEvent->modifiers() == Qt::ControlModifier)
							sendAction(ACT_Back2Seconds);
						else if (keyEvent->modifiers() == (Qt::ControlModifier | Qt::ShiftModifier))
							sendAction(ACT_Back4Seconds);
						else
							sendAction(ACT_PreviousFrame);

						return true;
					case Qt::Key_Right:
						if (keyEvent->modifiers() == Qt::ShiftModifier) 
							sendAction(ACT_Forward1Second);
						else if (keyEvent->modifiers() == Qt::ControlModifier) 
							sendAction(ACT_Forward2Seconds);
						else if (keyEvent->modifiers() == (Qt::ControlModifier | Qt::ShiftModifier)) 
							sendAction(ACT_Forward4Seconds);
						else 
							sendAction(ACT_NextFrame);

						return true;
					case Qt::Key_Up:
						sendAction(ACT_NextKFrame);
						return true;
					case Qt::Key_Down:
						sendAction(ACT_PreviousKFrame);
						return true;
					case Qt::Key_Shift:
						shiftKeyHeld = 1;
						break;

					case Qt::Key_BracketLeft:
                                                if (keyEvent->modifiers() == Qt::ControlModifier)
                                                        sendAction(ACT_GotoMarkA);
                                                return true;

					case Qt::Key_BracketRight:
						if (keyEvent->modifiers() == Qt::ControlModifier)
							sendAction(ACT_GotoMarkB);
						return true;
				}
			}
			/* else */ if (keyEvent->key() == Qt::Key_Space)
			{
				sendAction(ACT_PlayAvi);
				return true;
			}

			break;
        case QEvent::Resize:
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
		}
	}
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

MainWindow::~MainWindow()
{
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
bool  	UI_reset(void)
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
        ADM_info("Starting QT4 GUI...\n");
	initTranslator();

	global_argc=nargc;
	global_argv=nargv;
	ADM_renderLibInit(&UI_Hooks);
        Q_INIT_RESOURCE(avidemux);
	Q_INIT_RESOURCE(filter);

#if defined(__APPLE__)
 printf("Setting qt plugin folder\n");
 QDir dir(QApplication::applicationDirPath());
 dir.cdUp();
 dir.cdUp();
 dir.cd("plugins");
 printf("New plugin path =%s\n",dir.absolutePath().toUtf8().constData());
 QApplication::setLibraryPaths(QStringList(dir.absolutePath()));
#endif
	myApplication=new myQApplication (global_argc, global_argv);
	myApplication->connect(myApplication, SIGNAL(lastWindowClosed()), myApplication, SLOT(quit()));

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
    mw->ui.frame_video->setAcceptDrops(true); // needed for drag and drop to work on windows
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
    \fn UI_getCurrentPreview(void)
    \brief Read previewmode from comboxbox
*/
int UI_getCurrentPreview(void)
{
    if(WIDGET(checkDisplayOut)->isChecked()) 
    {
        printf("Output is ON\n");
        return 1;
    }
    printf("Output is Off\n");
    return 0;
}

/**
    \fn UI_setCurrentPreview(int ne)
    \brief Update comboxbox with previewmode
*/
void UI_setCurrentPreview(int ne)
{
        WIDGET(checkDisplayOut)->setChecked(!!ne);
}
/**
        \fn FatalFunctionQt
*/
static void FatalFunctionQt(const char *title, const char *info)
{
    printf("Crash Dump for %s\n",title);
    printf("%s\n",info);
    fflush(stdout);
    
    QMessageBox msgBox;
    msgBox.setText(title);
    msgBox.setInformativeText(QT_TR_NOOP("The application has encountered a fatal problem\nThe current editing has been saved and will be reloaded at next start"));
    msgBox.setDetailedText(info);
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.exec();
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
    ADM_setCrashHook(&saveCrashProject, &FatalFunctionQt);
	
          
    ADM_info("Checking for crash... \n");
    
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
	printf("Found %d video encoder(s)\n",nbVid);
	for(uint32_t i=1;i<nbVid;i++)
	{
		ADM_ve6_getEncoderInfo(i,&name,&maj,&mn,&pa);
		WIDGET(comboBoxVideo)->addItem(name);
	}

	// And A codec

	uint32_t nbAud;

    nbAud=audioEncoderGetNumberOfEncoders();
	printf("Found %d audio encoder(s)\n",nbAud);
	for(uint32_t i=1;i<nbAud;i++)
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

}
/*
    Return % of scale (between 0 and 1)
*/
double 	UI_readScale( void )
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
	slider->setValue( (int)(val * ADM_SCALE_INCREMENT));
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
	char	c='?';
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
	sprintf(string,QT_TR_NOOP("%c-%s (%02d)"),c,f,qp);
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
  	sprintf(text, "/%02d:%02d:%02d.%03d", hh, mm, ss, ms);
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
	snprintf(text,79,"%02"PRIu32":%02"PRIu32":%02"PRIu32".%02"PRIu32,hh,mm,ss,ms);
	WIDGET(pushButtonJumpToMarkerA)->setText(text);

	timems=(uint32_t)(b);
    ms2time(timems,&hh,&mm,&ss,&ms);
	snprintf(text,79,"%02"PRIu32":%02"PRIu32":%02"PRIu32".%02"PRIu32,hh,mm,ss,ms);
	WIDGET(pushButtonJumpToMarkerB)->setText(text);

	slider->setMarkers(absoluteA, absoluteB);
}

/**
    \fn     UI_getCurrentVCodec(void)
    \brief  Returns the current selected video code in menu, i.e its number (0 being the first)
*/
int 	UI_getCurrentVCodec(void)
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

int 	UI_getCurrentACodec(void)
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

int 	UI_GetCurrentFormat( void )
{
	int i=WIDGET(comboBoxFormat)->currentIndex();
	if(i<0) i=0;
	return (int)i;
}
/**
    \fn     UI_SetCurrentFormat( ADM_OUT_FORMAT fmt )
    \brief  Select  output format
*/
void 	UI_SetCurrentFormat( uint32_t fmt )
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
    \fn setDecodeName
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
	QuiMainWindows->hide();

}
/**
    \fn UI_deiconify
*/
void UI_deiconify( void )
{
    QuiMainWindows->showNormal();

}
/**
    \fn UI_deiconify
*/
void UI_setAudioTrackCount( int nb )
{
    char txt[50];
    sprintf(txt," (%d track(s))",nb);
     WIDGET(TrackCountLabel)->setText(QString(txt));

}
/**
 * \fn dtor
 */
#ifdef SDL_ON_LINUX
extern void disableExitHandler();
#endif
myQApplication::~myQApplication()
{
    
    ADM_warning("Cleaning render...\n");
    renderDestroy();
    ADM_warning("Cleaning preview...\n");
    admPreview::cleanUp();
    ADM_warning("Cleaning video body...\n");
    if(video_body) video_body->cleanup (); // Delete decoder related to X11 context before purging X11 context
    
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
    disableExitHandler();
    ::exit(0); // 
#endif    
    ADM_warning("Exiting app\n");
}
                
//********************************************
//EOF
