#ifndef Q_gui2_h
#define Q_gui2_h

#include <QtGui/QSlider>
#include <QtGui/QWidget>

#include "ADM_qslider.h"
#include "T_thumbSlider.h"
#include "ui_gui2.h"
#include "gui_action.hxx"

#define MKICON(x) ":/new/prefix1/pics/"#x".png"
#include "myOwnMenu.h"
/**
    \class MainWindow
*/
class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow();
	virtual ~MainWindow();	
    bool buildRecentMenu(void);
	void buildCustomMenu(void);
    bool buildMyMenu(void);
    bool buildMenu(QMenu *root,MenuEntry *menu, int nb);
    void searchMenu(QAction * action,MenuEntry *menu, int nb);
    
	Ui_MainWindow ui;
protected:
    QMenu *jsMenu;
    QMenu *pyMenu;
    QMenu *recentFiles;
    QMenu *recentProjects;
    QAction *recentFileAction[4];
    ThumbSlider *thumbSlider;
    
public slots:
	void timeChanged(int);
	void buttonPressed(void);
	void customPy(void);
    void customJs(void);
	void toolButtonPressed(bool z);

	void comboChanged(int z);
	void sliderValueChanged(int u);
	void sliderMoved(int value);
	void sliderReleased(void);
	void volumeChange( int u );
	void audioToggled(bool checked);
	void previewModeChanged(QAction *action);
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
    void searchToolBar(QAction *);
protected:
	void clearCustomMenu(void);
	bool eventFilter(QObject* watched, QEvent* event);
	void mousePressEvent(QMouseEvent* event);
	void dragEnterEvent(QDragEnterEvent *event);
	void dropEvent(QDropEvent *event);
    void closeEvent ( QCloseEvent * event )  ;
    void customScript(int pool,int base,QObject *ptr);
};
#endif	// Q_gui2_h
