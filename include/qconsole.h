/* -*- mode: c++ -*- */
/***************************************************************************
                          qconsole.h  -  description
                             -------------------
    begin                : mar mar 15 2005
    copyright            : (C) 2005 by Houssem BDIOUI
    email                : houssem.bdioui@gmail.com
 ***************************************************************************/

// migrated to Qt4 by YoungTaek Oh. date: Nov 29, 2010

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QCONSOLE_H
#define QCONSOLE_H

#include <QStringList>
#include <QTextEdit>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QMenu>

#include <QDialog>
#include <QListWidget>

#if QT_VERSION < 0x040000
#error "supports only Qt 4.0 or greater"
#endif

/**
 *  Popup Completer class
 *
 *  @author YoungTaek Oh
 *  @todo 1. beautifying
 *        2. icons for classifying words (eg. functions, properties...)
 *        3. bugs?
 *  @note still experimental
 */
class PopupCompleter : public QDialog
{
    Q_OBJECT

public:
    PopupCompleter(const QStringList&, QWidget *parent = 0);
    virtual ~PopupCompleter();

public:
    QString selected(void) { return selected_; }

protected:
    virtual void showEvent(QShowEvent*);

private Q_SLOTS:
    void onItemActivated(QListWidgetItem*);

public:
    QListWidget *listWidget_;
    QString selected_;
};

/**
 * An abstract Qt console
 * @author Houssem BDIOUI
 */
class QConsole : protected QTextEdit
{
    Q_OBJECT
public:
    //constructor
    QConsole(QWidget *parent = NULL, const QString &welcomeText = "");
    //set the prompt of the console
    void setPrompt(const QString &prompt, bool display = true);
    //execCommand(QString) executes the command and displays back its result
    bool execCommand(const QString &command, bool writeCommand = true, bool showPrompt = true);
    //saves a file script
    int saveScript(const QString &fileName);
    //loads a file script
    int loadScript(const QString &fileName);
    //clear & reset the console (useful sometimes)
    void clear();
    void reset(const QString &welcomeText = "");
    //cosmetic methods !
    void setCmdColor(QColor c) {cmdColor = c;}
    void setErrColor(QColor c) {errColor = c;}
    void setOutColor(QColor c) {outColor = c;}
    void setCompletionColor(QColor c) {completionColor = c;}
    void setFont(QFont f) {setCurrentFont(f);}

private:
    // Redefined virtual methods
    void mouseReleaseEvent(QMouseEvent *e);
    void mousePressEvent(QMouseEvent *e);
    //void mouseDoubleClickEvent(QMouseEvent *e);
    void keyPressEvent(QKeyEvent * e);
    void paste();
    //Just to disable the popup menu
    QMenu * createPopupMenu (const QPoint &pos);
    //Return false if the command is incomplete (e.g. unmatched braces)
    virtual bool isCommandComplete(const QString &command);
    //Get the command to validate
    QString getCurrentCommand();

    //Replace current command with a new one
    void replaceCurrentCommand(const QString &newCommand);

    //Test wether the cursor is in the edition zone
    bool isInEditionZone();

    //displays redirected stdout/stderr
    //void stdReceived(QTextIStream *s);

//protected attributes
protected:
    //colors
    QColor cmdColor, errColor, outColor, completionColor;
    // Old cursor position
    int oldPosition;
    // cached prompt length
    int promptLength;
    // The prompt string
    QString prompt;
    // The commands history
    QStringList history;
    //Contains the commands that has succeeded
    QStringList recordedScript;
    // Current history index (needed because afaik QStringList does not have such an index)
    int historyIndex;
    //Holds the paragraph number of the prompt (useful for multi-line command handling)
    int promptParagraph;

protected:
    //execute a validated command (should be reimplemented and called at the end)
    //the return value of the function is the string result
    //res must hold back the return value of the command (0: passed; else: error)
    virtual QString interpretCommand(const QString &command, int *res);
    //give suggestions to autocomplete a command (should be reimplemented)
    //the return value of the function is the string list of all suggestions
    //the returned prefix is useful to complete "sub-commands"
    virtual QStringList suggestCommand(const QString &cmd, QString &prefix);

// Redefined virtual slots
private Q_SLOTS:

    //displays the prompt
    void displayPrompt();

Q_SIGNALS:
    //Signal emitted after that a command is executed
    void commandExecuted(const QString &command);

private:
    void handleTabKeyPress();
    void handleReturnKeyPress();
    bool handleBackspaceKeyPress();
    void setHome();
};

#endif
