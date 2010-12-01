/***************************************************************************
                          qconsole.cpp  -  description
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

#include "qconsole.h"
#include <QFile>
#include <QTextStream>
//#include <QDebug>

#define WRITE_ONLY QIODevice::WriteOnly

//Clear the console
void QConsole::clear()
{
   QTextEdit::clear();
}

//Reset the console
void QConsole::reset()
{
    clear();

#ifdef __APPLE__
    setCurrentFont(QFont("Monaco"));
#endif

    //init attributes
    historyIndex = 0;
    history.clear();
    recordedScript.clear();
}

//QConsole constructor (init the QTextEdit & the attributes)
QConsole::QConsole(QWidget *parent, const char */*name*/, bool initInterceptor)
    : QTextEdit(parent), cmdColor(Qt::black), errColor(Qt::red),
    outColor(Qt::blue), completionColor(Qt::darkGreen),
    stdoutInterceptor(NULL), stderrInterceptor(NULL)
{
    //resets the console
    reset();

    if (initInterceptor)
    {
        //Initialize the interceptors
        stdoutInterceptor = new Interceptor(this);
        stdoutInterceptor->initialize(1);
        connect(stdoutInterceptor, SIGNAL(received(QTextStream *)), SLOT(displayPrompt()));

        stderrInterceptor = new Interceptor(this);
        stderrInterceptor->initialize(2);
        connect(stderrInterceptor, SIGNAL(received(QTextStream *)), SLOT(displayPrompt()));
    }

    const int tabwidth = QFontMetrics(currentFont()).width('a') * 4;
    setTabStopWidth(tabwidth);
}

//Sets the prompt and cache the prompt length to optimize the processing speed
void QConsole::setPrompt(QString newPrompt, bool display)
{
    prompt = newPrompt;
    promptLength = prompt.length();
    //display the new prompt
    if (display)
        displayPrompt();
}

//Displays the prompt and move the cursor to the end of the line.
void QConsole::displayPrompt()
{
    //flush the stdout/stderr before displaying the prompt
    if (stdoutInterceptor)
    {
        setTextColor(outColor);
        stdReceived(stdoutInterceptor->textStream());
    }
    if (stderrInterceptor)
    {
        setTextColor(errColor);
        stdReceived(stderrInterceptor->textStream());
    }
    //displays the prompt
    setTextColor(cmdColor);
    QTextCursor cur = textCursor();
    cur.insertText(prompt);
    cur.movePosition(QTextCursor::EndOfLine);

    //Saves the paragraph number of the prompt
    promptParagraph = cur.blockNumber();
}

//displays redirected stdout/stderr
void QConsole::stdReceived(QTextStream *s)
{
    QString line;
    line = s->readAll();
    textCursor().insertText(line);
}

//Reimplemented mouse press event
void QConsole::mousePressEvent( QMouseEvent *e )
{
    //Saves the old position of the cursor before any mouse click
    oldPosition = textCursor().position();
    //Call the parent implementation
    QTextEdit::mousePressEvent( e );
}

//Reimplemented mouse release event
void QConsole::mouseReleaseEvent( QMouseEvent *e )
{
    //Call the parent implementation
    QTextEdit::mouseReleaseEvent( e );

    //Undo the new cursor position if it is out of the edition zone
    if (!isInEditionZone()) {
        QTextCursor cur = textCursor();
        cur.setPosition(oldPosition);
        setTextCursor(cur);
    }
}

//give suggestions to autocomplete a command (should be reimplemented)
//the return value of the function is the string list of all suggestions
QStringList QConsole::autocompleteCommand(QString)
{
    return QStringList();
}

//Treat the tab key & autocomplete the current command
void QConsole::handleTabKeyPress()
{
    QString command = getCurrentCommand();
    QStringList sl = autocompleteCommand(command);
    QString str = sl.join(" ");
    if (sl.count() == 0)
        textCursor().insertText("\t");
    else if (sl.count() == 1)
        replaceCurrentCommand(sl[0]);
    else if (sl.count() > 1)
    {
        setTextColor(completionColor);
        append(sl.join(", ") + "\n");
        setTextColor(cmdColor);
        displayPrompt();
        textCursor().insertText(command);
    }
}

// If return pressed, do the evaluation and append the result
void QConsole::handleReturnKeyPress()
{
    //Get the command to validate
    QString command = getCurrentCommand();
    //execute the command and get back its text result and its return value
    if (isCommandComplete(command))
        execCommand(command, false);
    else
    {
        append("");
        moveCursor(QTextCursor::EndOfLine);
    }
}

// 반환값이 true이면 더 이상의 처리를 종료한다.
bool QConsole::handleBackspaceKeyPress()
{
    QTextCursor cur = textCursor();
    const int col = cur.columnNumber();
    const int blk = cur.blockNumber();
    if (blk == promptParagraph && col == promptLength)
        return true;
    return false;
}

void QConsole::setHome()
{
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::StartOfLine);
    cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, promptLength);
    setTextCursor(cursor);
}
  
//Reimplemented key press event
void QConsole::keyPressEvent( QKeyEvent *e )
{
    // control is pressed
    if (e->modifiers() & Qt::ControlModifier) {
        switch (e->key()) {
        case Qt::Key_C:
            //If Ctrl + C pressed, then undo the current command
            append("");
            displayPrompt();
            break;

        default:
            break;
        }
    } else {
        switch (e->key()) {
        case Qt::Key_Tab:
            handleTabKeyPress();
            return;

        case Qt::Key_Left:
            if (handleBackspaceKeyPress())
                return;
            break;

        case Qt::Key_Return:
            handleReturnKeyPress();
            // return key를 무시한다.
            return;

        case Qt::Key_Backspace:
            if (handleBackspaceKeyPress())
                return;
            break;

        case Qt::Key_Home:
            setHome();
            return;

        case Qt::Key_Down:
            if (++historyIndex >= (uint)history.size())
                historyIndex = history.size() - 1;
            replaceCurrentCommand(history[historyIndex]);
            return;

        case Qt::Key_Up:
            if (historyIndex)
                historyIndex--;
            replaceCurrentCommand(history[historyIndex]);
            return;

        default:
            break;
        }
    }

    oldPosition = textCursor().position();
    QTextEdit::keyPressEvent( e );
    if (!isInEditionZone()) {
        QTextCursor cur = textCursor();
        cur.setPosition(oldPosition);
        setTextCursor(cur);
    }
}

//Get the current command
QString QConsole::getCurrentCommand()
{
    QTextCursor cursor = textCursor();
    //Get the current command: we just remove the prompt
    cursor.movePosition(QTextCursor::StartOfLine);
    cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, promptLength);
    cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
    QString command = cursor.selectedText();
    cursor.clearSelection();
    return command;
}

//Replace current command with a new one
void QConsole::replaceCurrentCommand(QString newCommand)
{
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::StartOfLine);
    cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, promptLength);
    cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
    cursor.insertText(newCommand);
}

//default implementation: command always complete
bool QConsole::isCommandComplete(QString )
{
    return true;
}

//Tests whether the cursor is in th edition zone or not (after the prompt
//or in the next lines (in case of multi-line mode)
bool QConsole::isInEditionZone()
{
    const int para = textCursor().blockNumber();
    const int index = textCursor().columnNumber();
    return (para > promptParagraph) || ( (para == promptParagraph) && (index >= promptLength) );
}

//Basically, puts the command into the history list
//And emits a signal (should be called by reimplementations)
QString QConsole::interpretCommand(QString command, int *res)
{
    //Add the command to the recordedScript list
    if (!*res)
        recordedScript.append(command);
    //update the history and its index
    history.append(command.replace("\n", "\\n"));
    historyIndex = history.size();
    //emit the commandExecuted signal
    Q_EMIT commandExecuted(command);
    return "";
}

//execCommand(QString) executes the command and displays back its result
void QConsole::execCommand(QString command, bool writeCommand, bool showPrompt)
{
    //Display the prompt with the command first
    if (writeCommand)
    {
        if (getCurrentCommand() != "")
            append("");
        displayPrompt();
        textCursor().insertText(command);
    }
    //execute the command and get back its text result and its return value
    int res;
    QString strRes = interpretCommand(command, &res);
    //According to the return value, display the result either in red or in blue
    if (res == 0)
        setTextColor(outColor);
    else
        setTextColor(errColor);

    if (!(strRes.isEmpty() || strRes.endsWith("\n")))
        strRes.append("\n");

    append(strRes);
    moveCursor(QTextCursor::End);
    //Display the prompt again
    if (showPrompt)
        displayPrompt();
}

//saves a file script
int QConsole::saveScript(QString fileName)
{
    QFile f(fileName);
    if (!f.open(WRITE_ONLY))
        return -1;
    QTextStream ts(&f);
    for ( QStringList::Iterator it = recordedScript.begin(); it != recordedScript.end(); ++it)
        ts << *it << "\n";

    f.close();
    return 0;
}

//loads a file script
int QConsole::loadScript(QString fileName)
{
    QFile f(fileName);
    if (!f.open(QIODevice::ReadOnly))
        return -1;
    QTextStream ts(&f);
    QString command;
    while(true)
    {
        command=ts.readLine();
        if (command.isNull())
           break; //done
        execCommand(command, true, false);
    }
    f.close();
    return 0;
}

//Just disable the popup menu
QMenu * QConsole::createPopupMenu (const QPoint &)
{
    return NULL;
}

//Allows pasting with middle mouse button (x window)
//when clicking outside of the edition zone
void QConsole::paste()
{
    //setCursorPosition(oldPara, oldIndex );
    QTextEdit::paste();
}
