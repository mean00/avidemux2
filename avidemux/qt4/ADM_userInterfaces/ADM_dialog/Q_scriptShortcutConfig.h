#ifndef Q_scriptShortcutConfig_h
#define Q_scriptShortcutConfig_h

#include "ui_scriptShortcutConfig.h"

#define SCRIPT_SHORTCUT_CONFIG_LIST_XMACRO \
    X(Home) \
    X(End) \
    X(Left) \
    X(Up) \
    X(Right) \
    X(Down) \
    X(PageUp) \
    X(PageDown) \
    X(0) \
    X(1) \
    X(2) \
    X(3) \
    X(4) \
    X(5) \
    X(6) \
    X(7) \
    X(8) \
    X(9) \
    // end_of_list

#define SCRIPT_NUMERIC_SHORTCUT_CONFIG_LIST_XMACRO \
    X(0) \
    X(1) \
    X(2) \
    X(3) \
    X(4) \
    X(5) \
    X(6) \
    X(7) \
    X(8) \
    X(9) \
    // end_of_list

class scriptShortcutConfigDialog : public QDialog
{
    Q_OBJECT

public:
    Ui_ScriptShortcutConfigDialog ui;

public:
    scriptShortcutConfigDialog(QWidget* parent);
    ~scriptShortcutConfigDialog();
    void showEvent(QShowEvent *event);
private slots:
    #define X(key) void comboBoxAlt ## key ## _activated(int index);
    SCRIPT_SHORTCUT_CONFIG_LIST_XMACRO
    #undef X
};
#endif    // Q_scriptShortcutConfig_h
