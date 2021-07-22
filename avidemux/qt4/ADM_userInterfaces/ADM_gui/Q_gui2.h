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
    void syncToolbarsMenu(void);
    static void updateCheckDone(int version, const std::string &date, const std::string &downloadLink);
    static MainWindow *mainWindowSingleton;

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

    ADM_dragState dragState;
    QTimer dragTimer;
    const  std::vector<IScriptEngine*>& _scriptEngines;

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

private slots:
    void timeChanged(int);
    void timeChangeFinished(void);
    void checkChanged(int);
    void comboChanged(int z);

    void buttonPressed(void);
    void toolButtonPressed(bool z);

    void previewModeChangedFromMenu(bool status);
    void previewModeChangedFromToolbar(bool status);

    void currentTimeChanged(void);

    void sliderValueChanged(int u);
    void sliderMoved(int value);
    void sliderReleased(void);
    void sliderPressed(void);
    void sliderWheel(int way);

    void dragTimerTimeout(void);
    void busyTimerTimeout(void);
    void sendAction(Action a);
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
