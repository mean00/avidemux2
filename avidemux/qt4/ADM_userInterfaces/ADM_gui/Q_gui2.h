#ifndef Q_gui2_h
#define Q_gui2_h

#include <QtGui/QSlider>
#include <QtGui/QWidget>

#include "ADM_qslider.h"
#include "ui_gui2.h"

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow();
	virtual ~MainWindow();	
	void buildCustomMenu(void);
	
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

protected:
	void clearCustomMenu(void);
	bool eventFilter(QObject* watched, QEvent* event);
	void mousePressEvent(QMouseEvent* event);
	void dragEnterEvent(QDragEnterEvent *event);
	void dropEvent(QDropEvent *event);
    void closeEvent ( QCloseEvent * event )  ;
};
#endif	// Q_gui2_h
