#ifndef Q_gui2_h
#define Q_gui2_h

#define MKICON(x) ":/new/prefix1/pics/"#x".png"
#define MKOSXICON(x) ":/new/prefix1/pics/"#x".icns"

#include <vector>
#include <QtCore/QUrl>
#include <QSlider>
#include <QWidget>
#include <QtCore/QTimer>
#include <string>

#include "ADM_mwNavSlider.h"
#include "T_thumbSlider.h"
#include "ui_gui2.h"
#include "gui_action.hxx"
#include "myOwnMenu.h"
#include "IScriptEngine.h"
#include "prefs.h"
#include "avi_vars.h"

#define ENABLE_EVENT_FILTER

#define NAVIGATION_ACTION_LOCK_THRESHOLD	(4)

#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
    #define BROKEN_PALETTE_PROPAGATION
#endif
/**
 * \class myQApplication
 * \brief make sure the checkCrash & friends are done after Qt init
 * @return 
 */
extern void checkCrashFile(void);
extern int automation(void);
extern int global_argc;
extern char **global_argv;
extern void ADM_ExitCleanup(void);

class myQApplication : public QApplication
{
    Q_OBJECT
        public:
                virtual int exec()
                {
                    connect(&timer, SIGNAL(timeout()), this, SLOT(postInit()));
                    timer.setSingleShot(true);
                    timer.start(10);
                    return QApplication::exec();

                }
                
                virtual ~myQApplication();
                myQApplication(int &argc, char **argv) : QApplication(argc,argv)
                {
#ifdef __APPLE__
                    ready = false;
#endif
                }
#ifdef __APPLE__
private:
                bool ready;
                QList<QUrl> fileOpenQueue;
                bool event(QEvent *event) override;
                void handleFileOpenRequests(void);
#endif
        protected:
                QTimer timer;
        public slots:
                void postInit(void)
                {
                        ADM_info("myQApplication exec\n");
                        ADM_info("Checking for crash...\n");
                        checkCrashFile();
                        if (global_argc >= 2)
                                automation();
#ifdef __APPLE__
                        else
                                ready = true;
                        handleFileOpenRequests();
#endif
                }
                void cleanup()
                {
                  //ADM_ExitCleanup();
                }
};
/**
 * \enum ADM_dragState
 */
enum ADM_dragState
{
    dragState_Normal=0, // no drag
    dragState_Active=1, // drag on, we process the refresh
    dragState_HoldOff=2 // Cdrag off, we ignore the refresh
};

/**
    \class MainWindow
*/
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    Ui_MainWindow ui;

    MainWindow(const std::vector<IScriptEngine*>& scriptEngines);
    virtual ~MainWindow();

    void buildCustomMenu(void);
    void buildRecentMenu(void);
    void buildRecentProjectMenu(void);
    void updateActionShortcuts(void);
    void volumeWidgetOperational(void);
    void calcDockWidgetDimensions(uint32_t &width, uint32_t &height);
    void setBlockZoomChangesFlag(bool block);
    bool getBlockResizingFlag(void);
    void setBlockResizingFlag(bool block);
    void setResizeThreshold(int value);
    void setActZoomCalledFlag(bool called);
    void setZoomToFit(void);
    void updateZoomIndicator(void);
    void initWidgetsVisibility(void);
    void initStatusBar(void);
    void updateStatusBarInfo(void);
    void updateStatusBarDisplayInfo(const char * display);
    void updateStatusBarDecoderInfo(const char * decoder);
    void updateStatusBarZoomInfo(int zoom);
    void notifyStatusBar(const char * lead, const char * msg, int timeout = 2500);

    void setLightTheme(void);
    void setDarkTheme(void);

    void currentTimeAddActionButons(bool all);
    void totalTimeAddActionButons(bool all);
    void selectionDurationAddActionButons(bool all);
    void selectionMarkerAAddActionButons(bool all);
    void selectionMarkerBAddActionButons(bool all);

    static void updateCheckDone(int version, const std::string &date, const std::string &downloadLink);
    static MainWindow *mainWindowSingleton;

    QString statusBarFrame_Type;

    // Show precision timings in time fields tooltips
    void updatePTSToolTips(void);
    bool showPTSToolTips;

    // Show extra buttons in time fields
    void updateWidgetActionButtons(void);
    bool showExtraButtons;

    // Allow time fields editing via keyboard
    void updateTimeFieldsReadOnly(void);
    bool isCurrentTimeFieldEditable;
    bool isTotalTimeFieldEditable;
    bool isSelectionTimeFieldEditable;
    bool isMarkerATimeFieldEditable;
    bool isMarkerBTimeFieldEditable;

#ifdef __APPLE__
    void fileOpenWrapper(QList<QUrl> list) { openFiles(list); }
#endif

protected:
    QMenu *jsMenu;
    QMenu *pyMenu;
    QMenu *autoMenu;
    QMenu *recentFiles;
    QMenu *recentProjects;
    QAction *recentFileAction[NB_LAST_FILES];
    QAction *recentProjectAction[NB_LAST_FILES];
    QAction *actionHDRSeparator;
    QAction *displayZoom;
    QAction *defaultThemeAction;
    QAction *lightThemeAction;
    QAction *darkThemeAction;
    QString defaultStyle;
#ifdef BROKEN_PALETTE_PROPAGATION
    std::vector<QMenu *> subMenus;
#endif
    ThumbSlider *thumbSlider;

    bool     refreshCapEnabled;
    uint32_t refreshCapValue;
    unsigned int actionLock;
    unsigned int busyCntr;
    QTimer busyTimer; 

    std::vector<QAction *>ActionsAvailableWhenFileLoaded;
    std::vector<QAction *>ActionsDisabledOnPlayback;
    std::vector<QAction *>ActionsAlwaysAvailable;
    std::vector<QToolButton *>ButtonsAvailableWhenFileLoaded;
    std::vector<QToolButton *>ButtonsDisabledOnPlayback;
    std::vector<QPushButton *>PushButtonsAvailableWhenFileLoaded;
    std::vector<QPushButton *>PushButtonsDisabledOnPlayback;

    unsigned int navigateByTimeButtonsState;
    ADM_dragState dragState;
    QTimer dragTimer;
    int navigateWhilePlayingState;
    Action navigateWhilePlayingAction;
    Action navigateWhilePlayingPendingAction;
    QTimer navigateWhilePlayingTimer;
    const  std::vector<IScriptEngine*>& _scriptEngines;

    QMenu *createPopupMenu(void);

    void addScriptDirToMenu(QMenu* scriptMenu, const QString& dir, const QStringList& fileExts);
    void addScriptEnginesToFileMenu(std::vector<MenuEntry>& fileMenu);
    void addScriptShellsToToolsMenu(vector<MenuEntry>& toolMenu);
    void addScriptReferencesToHelpMenu();
    void addSessionRestoreToRecentMenu(vector<MenuEntry>& menu);
    bool buildMyMenu(void);
    bool buildMenu(QMenu *root,MenuEntry *menu, int nb);
    void buildRecentMenu(QMenu *menu,std::vector<std::string>files, QAction **actions);
    void buildActionLists(void);
    void buildButtonLists(void);
    void updateCodecWidgetControlsState(void);
    void widgetsUpdateTooltips(void);
    void searchMenu(QAction * action,MenuEntry *menu, int nb);
    void searchRecentFiles(QAction *action, QAction **actionList, int firstEventId);
#ifdef ENABLE_EVENT_FILTER
    bool eventFilter(QObject* watched, QEvent* event);
#endif
    void mousePressEvent(QMouseEvent* event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
    void openFiles(QList<QUrl>);
    void changeEvent(QEvent* event);
    /* Zoom control */
    bool adjustZoom(int width, int height);
    bool blockResizing;
    bool blockZoomChanges;
    bool ignoreResizeEvent; // suppress unwanted zoom changes
    bool actZoomCalled; // zoom was set to a pre-defined fraction by a menu action
    int  threshold; // track how much the window was resized
    /* allow to copy current pts to clipboard using a keyboard shortcut for convenience */
    void currentTimeToClipboard(void);
    bool dragWhilePlay;
    
    QLabel * statusBarInfo;
    int statusBarInfo_Zoom;
    QString statusBarInfo_Display, statusBarInfo_Decoder;

    // Reflects the items order set in View->Toolbars
    enum ADM_Toolbars_Item
    {
        FIRST=0,     // begin (for iterations)
        TOOLBAR=0,    // toolBar
        STATUSBAR=1,  // statusBarWidget
        CODEC=2,      // codecWidget
        NAVIGATION=3, // navigationWidget
        // separator
        AUDIOMETER=5, // audioMeterWidget
        VOLUME=6,     // volumeWidget
        CONTROLS=7,   // controlsWidget
        SELECTION=8,  // selectionWidget
        TIME=9,       // timeWidget
        SLIDER=10,    // sliderWidget
        LAST=10       // end (for iterations)
    };

private:
    QList<QAction*> currentTimeActionButtons;
    QList<QAction*> totalTimeActionButtons;
    QList<QAction*> selectionDurationActionButtons;
    QList<QAction*> selectionMarkerAActionButtons;
    QList<QAction*> selectionMarkerBActionButtons;

    QAction *pushButtonTime;
    QAction *pushButtonSaveScript;
    QAction *pushButtonRunScript;
    QAction *pushButtonAppend;
    QAction *pushButtonUndo;
    QAction *pushButtonRedo;
    QAction *pushButtonCut;
    QAction *pushButtonCopy;
    QAction *pushButtonPaste;
    QAction *pushButtonEditMarkerA;
    QAction *pushButtonEditMarkerB;
    QAction *pushButtonJumpToMarkerA;
    QAction *pushButtonJumpToMarkerB;
    QAction *pushButtonResetMarkerA;
    QAction *pushButtonResetMarkerB;
    QAction *pushButtonResetMarkers;

private slots:
    void timeChanged(int);
    void timeChangeFinished(void);
    void checkChanged(int);
    void comboChanged(int z);

    void seekTime(void);
    void saveScriptAction(void);
    void runScriptAction(void);
    void appendAction(void);
    void undoAction(void);
    void redoAction(void);
    void cutSelection(void);
    void copySelection(void);
    void pasteClipboard(void);
    void editMarkerA(void);
    void editMarkerB(void);
    void gotoMarkerA(void);
    void gotoMarkerB(void);
    void resetMarkerA(void);
    void resetMarkerB(void);
    void resetMarkers(void);

    void buttonPressed(void);
    void toolButtonPressed(bool z);

    void previewModeChangedFromMenu(bool status);
    void previewModeChangedFromToolbar(bool status);

    void currentTimeChanged(void);
    void markerAChanged(void);
    void markerBChanged(void);
    void selectionDurationChanged(void);
    void totalTimeChanged(void);

    void toolBarVisibilityChanged(bool);
    void statusBarVisibilityChanged(bool);
    void codecVisibilityChanged(bool);
    void navigationVisibilityChanged(bool);
    void audioMeterVisibilityChanged(bool);
    void volumeVisibilityChanged(bool);
    void controlsVisibilityChanged(bool);
    void selectionVisibilityChanged(bool);
    void timeVisibilityChanged(bool);
    void sliderVisibilityChanged(bool);

    void sliderValueChanged(int u);
    void sliderMoved(int value);
    void sliderReleased(void);
    void sliderPressed(void);
    void sliderWheel(int way);

    void dragTimerTimeout(void);
    void busyTimerTimeout(void);
    void navigateWhilePlayingTimerTimeout(void);
    void sendAction(Action a);
    void navigateWhilePlaying(Action a);
    void actionSlot(Action a);

    void scriptFileActionHandler(void);
    void scriptReferenceActionHandler(void);

    void searchFileMenu(QAction * action);
    void searchRecentMenu(QAction * action);
    void searchEditMenu(QAction * action);
    void searchVideoMenu(QAction * action);
    void searchAudioMenu(QAction * action);
    void searchHelpMenu(QAction * action);
    void searchToolMenu(QAction * action);
    void searchViewMenu(QAction * action);
    void searchGoMenu(QAction * action);
    void searchRecentFiles(QAction * action);
    void searchRecentProjects(QAction * action);
    void searchToolBar(QAction * action);

    void restoreDefaultWidgetState(bool b);
    void toolbarOrientationChangedSlot(Qt::Orientation hv);
    void setDefaultThemeSlot(bool b);
    void setLightThemeSlot(bool b);
    void setDarkThemeSlot(bool b);

    void closeEvent(QCloseEvent *event)
    {
        printf("Close event!\n");
        //QMainWindow::closeEvent(event);
        sendAction(ACT_EXIT);
    }

public slots:
    void updateAvailableSlot(int version, std::string date, std::string url);
    void setRefreshCap(void);
    void setMenuItemsEnabledState(void);

    void volumeChange(int u);
    void audioToggled(bool checked);

    void thumbSlider_valueEmitted(int value);

signals:
    void actionSignal(Action a);
    void updateAvailable(int version,const std::string date,const std::string downloadLink);
};
#endif	// Q_gui2_h
