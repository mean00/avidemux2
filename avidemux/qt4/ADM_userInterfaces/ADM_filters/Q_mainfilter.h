#ifndef Q_mainfilter_h
#define Q_mainfilter_h

#include <QtGui/QItemDelegate>
#include "ui_mainfilter.h"
#include "ADM_inttype.h"
#include "Q_seekablePreview.h"
class FilterItemEventFilter : public QObject
{
	Q_OBJECT

protected:
	bool eventFilter(QObject *object, QEvent *event);

public:
	FilterItemEventFilter(QWidget *parent = 0);
};

class FilterItemDelegate : public QItemDelegate
{
	Q_OBJECT

private:
	FilterItemEventFilter *filter;

public:
	FilterItemDelegate(QWidget *parent = 0);
	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

class filtermainWindow : public QDialog
{
	Q_OBJECT

public:
	filtermainWindow();
    ~filtermainWindow();
	void buildActiveFilterList(void);

	Ui_mainFilterDialog ui;
	QListWidget *availableList;
	QListWidget *activeList;
protected:
	
	int previewDialogX, previewDialogY;
	Ui_seekablePreviewWindow *previewDialog;
public slots:
	void VCD(bool b);
	void DVD(bool b);
	void SVCD(bool b);
	void halfD1(bool b);
	void add(bool b);
	void up(bool b);
	void down(bool b);
	void remove(bool b);
	void configure(bool b);
	void partial(bool b);
	void activeDoubleClick( QListWidgetItem  *item);
	void allDoubleClick( QListWidgetItem  *item);
	void filterFamilyClick(QListWidgetItem *item);
	void filterFamilyClick(int  item);
	void preview(bool b);
    void closePreview(void);
    // context menu
    void add(void);
    void remove(void);
    void configure(void);

private:
	void setSelected(int sel);
	void displayFamily(uint32_t family);
	void setupFilters(void);
};
#endif	// Q_mainfilter_h
