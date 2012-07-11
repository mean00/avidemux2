#include "FileAction.h"

FileAction::FileAction(const QString& text, const QString& filePath, QObject* parent) : QAction(text, parent)
{
	this->_filePath = filePath;
}

const QString& FileAction::filePath()
{
	return this->_filePath;
}