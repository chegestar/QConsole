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
#include <QDebug>
#include <QVBoxLayout>
#include <QApplication>
#include <QScrollBar>
#include <QDesktopWidget>

#define WRITE_ONLY QIODevice::WriteOnly

QSize PopupListWidget::sizeHint() const
{
    QAbstractItemModel *model = this->model();
    QAbstractItemDelegate *delegate = this->itemDelegate();
    const QStyleOptionViewItem sovi;
    QMargins margin = this->contentsMargins();

    const int vOffset = margin.top() + margin.bottom();
    const int hOffset = margin.left() + margin.right();

    bool vScrollOn = false;
    int height = 0;
    int width = 0;
    for (int i=0; i<this->count(); ++i) {
        QModelIndex index = model->index(i, 0);
        QSize itemSizeHint = delegate->sizeHint(sovi, index);
        if (itemSizeHint.width() > width)
            width = itemSizeHint.width();

        // height
        const int nextHeight = height + itemSizeHint.height();
        if (nextHeight + vOffset < this->maximumHeight())
            height = nextHeight;
        else {
            // early termination
            vScrollOn = true;
            break;
        }
    }

    QSize sizeHint(width + hOffset, 0);
    sizeHint.setHeight(height + vOffset);
    if (vScrollOn) {
        int scrollWidth = this->verticalScrollBar()->sizeHint().width();
        sizeHint.setWidth(sizeHint.width() + scrollWidth);
    }
    return sizeHint;
}

PopupCompleter::PopupCompleter(const QStringList& sl, QWidget *parent)
    : QDialog(parent, Qt::Popup)
{
    setModal(true);

    listWidget_ = new PopupListWidget();
    listWidget_->setMaximumHeight(200);
    qDebug() << "sizeHint(): " << listWidget_->sizeHint();
    Q_FOREACH(QString str, sl) {
        QListWidgetItem *item = new QListWidgetItem;
        item->setText(str);
        listWidget_->addItem(item);
    }
    qDebug() << "sizeHint(): " << listWidget_->sizeHint();
    listWidget_->setFixedSize(listWidget_->sizeHint());

    QLayout *layout = new QVBoxLayout();
    layout->setSizeConstraint(QLayout::SetFixedSize);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(listWidget_);

    setLayout(layout);

    // connect signal
    connect(listWidget_, SIGNAL(itemActivated(QListWidgetItem *)),
            SLOT(onItemActivated(QListWidgetItem*)));
}

PopupCompleter::~PopupCompleter()
{
}

void PopupCompleter::showEvent(QShowEvent */*event*/)
{
    listWidget_->setFocus();
}

void PopupCompleter::onItemActivated(QListWidgetItem *event)
{
    selected_ = event->text();
    done(QDialog::Accepted);
}

/**
 * @brief execute PopupCompleter at appropriate position.
 *
 * @param parent Parent of this popup completer. usually QConsole.
 * @return see QDialog::exec
 * @see QDialog::exec
 */
int PopupCompleter::exec(QTextEdit *parent)
{
    QSize popupSizeHint = this->sizeHint();
    QRect cursorRect = parent->cursorRect();
    QPoint globalPt = parent->mapToGlobal(cursorRect.bottomRight());
    QDesktopWidget *dsk = QApplication::desktop();
    QRect screenGeom = dsk->screenGeometry(dsk->screenNumber(this));
    if (globalPt.y() + popupSizeHint.height() > screenGeom.height()) {
        globalPt = parent->mapToGlobal(cursorRect.topRight());
        globalPt.setY(globalPt.y() - popupSizeHint.height());
    }
    this->move(globalPt);
    this->setFocus();
    return QDialog::exec();
}

/**
 * returns a common word of the given list
 *
 * @param list String list
 *
 * @return common word in the given string.
 */
static
QString getCommonWord(QStringList& list)
{
    QChar ch;
    QVector<QString> strarray = list.toVector();
    QString common;
    int col = 0,  min_len;
    bool cont = true;

    // get minimum length
    min_len = strarray.at(0).size();
    for (int i=1; i<strarray.size(); ++i) {
        const int len = strarray.at(i).size();
        if (len < min_len)
            min_len = len;
    }

    while(col < min_len) {
        ch = strarray.at(0)[col];
        for (int i=1; i<strarray.size(); ++i) {
            const QString& current_string = strarray.at(i);
            if (ch != current_string[col])
            {
                cont = false;
                break;
            }
        }
        if (!cont)
            break;

        common.push_back(ch);
        ++col;
    }
    return common;
}

////////////////////////////////////////////////////////////////////////////////

//Clear the console
void QConsole::clear()
{
   QTextEdit::clear();
}

//Reset the console
void QConsole::reset(const QString &welcomeText)
{
    clear();
    append(welcomeText);
    append("");

    //init attributes
    historyIndex = 0;
    history.clear();
    recordedScript.clear();
}

//QConsole constructor (init the QTextEdit & the attributes)
QConsole::QConsole(QWidget *parent, const QString &welcomeText)
    : QTextEdit(parent), errColor_(Qt::red),
    outColor_(Qt::blue), completionColor(Qt::darkGreen),
    promptLength(0), promptParagraph(0)
{
    QPalette palette = QApplication::palette();
    setCmdColor(palette.text().color());
    //Disable undo/redo
    setUndoRedoEnabled(false);
    //resets the console
    reset(welcomeText);

    const int tabwidth = QFontMetrics(currentFont()).width('a') * 4;
    setTabStopWidth(tabwidth);

    //set the style of the QTextEdit
#ifdef __APPLE__
    setCurrentFont(QFont("Monaco"));
#else
    QFont f;
    f.setFamily("Courier");
    setCurrentFont(f);
#endif
}

//Sets the prompt and cache the prompt length to optimize the processing speed
void QConsole::setPrompt(const QString &newPrompt, bool display)
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
    //displays the prompt
    setTextColor(cmdColor_);
    QTextCursor cur = textCursor();
    cur.insertText(prompt);
    cur.movePosition(QTextCursor::EndOfLine);
    setTextCursor(cur);
    //Saves the paragraph number of the prompt
    promptParagraph = cur.blockNumber();
}

void QConsole::restoreOldPosition()
{
    QTextCursor cur = textCursor();
    cur.setPosition(oldPosition);
    setTextCursor(cur);
}


//Reimplemented mouse press event
void QConsole::mousePressEvent( QMouseEvent *e )
{
    if (isInEditionZone()) {
        //Saves the old position of the cursor before any mouse click
        oldPosition = textCursor().position();
    }
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
        restoreOldPosition();
    }
}

//Give suggestions to autocomplete a command (should be reimplemented)
//the return value of the function is the string list of all suggestions
QStringList QConsole::suggestCommand(const QString&, QString& prefix)
{
    prefix = "";
    return QStringList();
}

//Treat the tab key & autocomplete the current command
void QConsole::handleTabKeyPress()
{
    QString command = getCurrentCommand();
    QString commandPrefix;
    QStringList sl = suggestCommand(command, commandPrefix);
    if (sl.count() == 0)
        textCursor().insertText("\t");
    else {
        if (sl.count() == 1)
            replaceCurrentCommand(commandPrefix + sl[0]);
        else
        {
            // common word completion
            QString commonWord = getCommonWord(sl);
            command = commonWord;

#ifdef USE_POPUP_COMPLETER
            PopupCompleter *popup = new PopupCompleter(sl);
            if (popup->exec(this) == QDialog::Accepted)
                replaceCurrentCommand(commandPrefix + popup->selected());
            delete popup;
#else
            setTextColor(completionColor);
            append(sl.join(", ") + "\n");
            setTextColor(cmdColor);
            displayPrompt();
            textCursor().insertText(commandPrefix + command);
#endif
        }
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
            if (!isInEditionZone())
            {
                return;
            }
            break;
        }
    } else {
        switch (e->key()) {
        case Qt::Key_Tab:
            handleTabKeyPress();
            return;

        case Qt::Key_Enter:
        case Qt::Key_Return:
            handleReturnKeyPress();
            // ignore return key
            return;

        case Qt::Key_Left:
        case Qt::Key_Backspace:
            if (handleBackspaceKeyPress())
                return;
            break;

        case Qt::Key_Home:
            setHome();
            return;

        case Qt::Key_Down:
            if (history.count())
            {
                if (++historyIndex >= history.size())
                    historyIndex = history.size() - 1;
                replaceCurrentCommand(history[historyIndex]);
            }
            return;

        case Qt::Key_Up:
            if (history.count())
            {
                if (historyIndex)
                    historyIndex--;
                replaceCurrentCommand(history[historyIndex]);
            }
            return;

        default:
            if (!isInEditionZone())
            {
                return;
            }
            break;
        }
    }

    oldPosition = textCursor().position();
    QTextEdit::keyPressEvent( e );
    if (!isInEditionZone()) {
        restoreOldPosition();
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
void QConsole::replaceCurrentCommand(const QString &newCommand)
{
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::StartOfLine);
    cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, promptLength);
    cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
    cursor.insertText(newCommand);
}

//default implementation: command always complete
bool QConsole::isCommandComplete(const QString &)
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
QString QConsole::interpretCommand(const QString &command, int *res)
{
    //Add the command to the recordedScript list
    if (!*res)
        recordedScript.append(command);
    //update the history and its index
    QString modifiedCommand = command;
    modifiedCommand.replace("\n", "\\n");
    history.append(modifiedCommand);
    historyIndex = history.size();
    //emit the commandExecuted signal
    Q_EMIT commandExecuted(modifiedCommand);
    return "";
}

//execCommand(QString) executes the command and displays back its result
bool QConsole::execCommand(const QString &command, bool writeCommand,
                           bool showPrompt, QString *result)
{
    //Display the prompt with the command first
    if (writeCommand)
    {
        if (getCurrentCommand() != "")
        {
            append("");
            displayPrompt();
        }
        textCursor().insertText(command);
    }
    //execute the command and get back its text result and its return value
    int res = 0;
    QString strRes = interpretCommand(command, &res);
    //According to the return value, display the result either in red or in blue
    if (res == 0)
        setTextColor(outColor_);
    else
        setTextColor(errColor_);

    if (result)
    {
        *result = strRes;
    }

    if (!(strRes.isEmpty() || strRes.endsWith("\n")))
        strRes.append("\n");

    append(strRes);
    moveCursor(QTextCursor::End);
    //Display the prompt again
    if (showPrompt)
        displayPrompt();
    return !res;
}

//saves a file script
int QConsole::saveScript(const QString &fileName)
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
int QConsole::loadScript(const QString &fileName)
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
    restoreOldPosition();
    QTextEdit::paste();
}
