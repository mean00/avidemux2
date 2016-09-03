#ifndef Q_gui2_h
#define Q_gui2_h

#define MKICON(x) ":/new/prefix1/pics/"#x".png"

#include <vector>
#include <QtCore/QUrl>
#include <QSlider>
#include <QWidget>
#include <QtCore/QTimer>
#include <string>

#include "ADM_qslider.h"
#include "T_thumbSlider.h"
#include "ui_gui2.h"
#include "gui_action.hxx"
#include "myOwnMenu.h"
#include "IScriptEngine.h"
#include "prefs.h"

#define ENABLE_EVENT_FILTER

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
                     
                }
               
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
                }
                void cleanup()
                {
                  //ADM_ExitCleanup();
                  emit quit();
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
        static void updateCheckDone(int version, const std::string &date, const std::string &downloadLink);
        static MainWindow *mainWindowSingleton;

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


public slots:
        void updateAvailableSlot(int version, std::string date, std::string url);
        void dragTimerTimeout(void);
        void actionSlot(Action a)
        {
            HandleAction(a);
        }
        void sendAction(Action a)
        {
            //printf("Sending internal event %d\n",(int)a);
            emit actionSignal(a);
        }
	void timeChanged(int);
        void checkChanged(int);
	void buttonPressed(void);
	void toolButtonPressed(bool z);

	void comboChanged(int z);
	void sliderValueChanged(int u);
	void sliderMoved(int value);
	void sliderReleased(void);
        void sliderPressed(void);
        void sliderWheel(int way);
	void volumeChange( int u );
	void audioToggled(bool checked);
	void previewModeChanged(int status);
	void previousIntraFrame(void);
	void nextIntraFrame(void);
	void timeChangeFinished(void);
	void currentFrameChanged(void);
	void currentTimeChanged(void);

    void thumbSlider_valueEmitted(int value);

    void searchFileMenu(QAction * action);
    void searchEditMenu(QAction * action);
    void searchVideoMenu(QAction * action);
    void searchAudioMenu(QAction * action);
    void searchHelpMenu(QAction * action);
    void searchToolMenu(QAction * action);
    void searchViewMenu(QAction * action);
    void searchGoMenu(QAction * action);
    void searchRecentFiles(QAction * action);
    void searchRecentProjects(QAction * action);
    void searchToolBar(QAction *);

    void scriptFileActionHandler();
    void scriptReferenceActionHandler();

     void closeEvent(QCloseEvent *event)
        {
                printf("Close event!\n");
                //QMainWindow::closeEvent(event);
                sendAction(ACT_EXIT);
        }

signals:
        void actionSignal(Action a);
        void updateAvailable(int version,const std::string date,const std::string downloadLink);
protected:
        ADM_dragState    dragState;        
        QTimer dragTimer;
	const  std::vector<IScriptEngine*>& _scriptEngines;

	void addScriptDirToMenu(QMenu* scriptMenu, const QString& dir, const QStringList& fileExts);
	void addScriptEnginesToFileMenu(std::vector<MenuEntry>& fileMenu);
	void addScriptShellsToToolsMenu(vector<MenuEntry>& toolMenu);
	void addScriptReferencesToHelpMenu();
        bool buildMyMenu(void);
        bool buildMenu(QMenu *root,MenuEntry *menu, int nb);
	void buildRecentMenu(QMenu *menu,std::vector<std::string>files, QAction **actions);
        void searchMenu(QAction * action,MenuEntry *menu, int nb);
	void searchRecentFiles(QAction *action, QAction **actionList, int firstEventId);
#ifdef   ENABLE_EVENT_FILTER      
	bool eventFilter(QObject* watched, QEvent* event);
#endif
	void mousePressEvent(QMouseEvent* event);
	void dragEnterEvent(QDragEnterEvent *event);
	void dropEvent(QDropEvent *event);
	void openFiles(QList<QUrl>);
};
#endif	// Q_gui2_h
