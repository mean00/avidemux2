#ifndef T_filesel_h
#define T_filesel_h

#include <QAbstractButton>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QWidget>

namespace ADM_Qt4Factory
{
	typedef enum 
	{
		ADM_FILEMODE_DIR,
		ADM_FILEMODE_READ,
		ADM_FILEMODE_WRITE
	} ADM_fileMode;

	class ADM_Qfilesel : public QWidget
	{
		Q_OBJECT

	public slots:
		void buttonPressed(QAbstractButton *s);

	public:
		QLineEdit *edit;
		QDialogButtonBox *button;
		QLabel *text;
		ADM_fileMode fileMode;
		const char *defaultSuffix;
		const char *selectDesc;

		ADM_Qfilesel(const char *title, std::string &entry, QGridLayout *layout, int line, ADM_fileMode mode, const char * defaultSuffix, const char* selectDesc);
		~ADM_Qfilesel();
	};
}
#endif	// T_filesel_h
