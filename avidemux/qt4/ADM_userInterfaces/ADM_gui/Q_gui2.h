#ifndef Q_gui2_h
#define Q_gui2_h

#include <QtGui/QSlider>
#include <QtGui/QWidget>

#include "ADM_qslider.h"
#include "ui_gui2.h"
#include "gui_action.hxx"
/**
    \enum MenuType
*/
enum MenuType
{
    MENU_ACTION,
    MENU_SEPARATOR

};
/**
    \struct MenuEntry
*/
typedef struct
{
    MenuType   type;
    const char *text;
    QAction    *action;
    Action     event;
    const char *icon; 
}MenuEntry;
/**
    \class MainWindow
*/
class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow();
	virtual ~MainWindow();	
	void buildCustomMenu(void);
    bool buildMyMenu(void);
    bool buildMenu(QMenu *root,MenuEntry *menu, int nb);
    void searchMenu(QAction * action,MenuEntry *menu, int nb);
    
	Ui_MainWindow ui;
protected:
    QMenu *jsMenu;
    QMenu *pyMenu;
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
    void searchFileMenu(QAction * action);
    void searchEditMenu(QAction * action);
    void searchVideoMenu(QAction * action);
    void searchAudioMenu(QAction * action);
    void searchHelpMenu(QAction * action);
    void searchToolMenu(QAction * action);
    void searchGoMenu(QAction * action);

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
