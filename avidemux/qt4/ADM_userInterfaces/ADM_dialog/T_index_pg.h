#ifndef T_index_pg_h
#define T_index_pg_h

#include <QDialog>
#include <QLabel>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QWidget>

class Ui_iDialog
{
public:
	QWidget *verticalLayout;
	QVBoxLayout *vboxLayout;
	QLabel *labelTimeLeft;
	QLabel *labelImages;
	QProgressBar *progressBar;

	void setupUi(QDialog *Dialog);
	void retranslateUi(QDialog *Dialog);
};

namespace Ui {
    class Dialog : public Ui_iDialog {};
};

class Ui_indexingDialog : public QDialog
{
	Q_OBJECT

protected:
	int lock;

public:
	int abted;
	Ui_iDialog ui;
	Ui_indexingDialog(QWidget *parent, const char *name);
	~Ui_indexingDialog();
	void setTime(const char *f);
	void setImage(const char *f);
	void setETA(const char *f);
	void setPercent(float f);    
};
#endif	// T_index_pg_h
