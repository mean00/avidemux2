#ifndef FileAction_h
#define FileAction_h

#include <QAction>

class FileAction : public QAction
{
	Q_OBJECT

private:
	QString _filePath;

public:
	FileAction(const QString& text, const QString& filePath, QObject* parent);

	const QString& filePath();
};
#endif	// FileAction_h
