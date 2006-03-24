/***************************************************************************
                          qtclconsole.h  -  description
                             -------------------
    begin                : mar mar 15 2005
    copyright            : (C) 2005 by Houssem BDIOUI
    email                : houssem.bdioui@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QTCLCONSOLE_H
#define QTCLCONSOLE_H

#include <tcl.h>

#include "qconsole.h"

/**An emulated singleton class console for Tcl within a Qt application (based on the QConsole class)
 *@author Houssem BDIOUI
 */

class QtclConsole : public QConsole
{
    public:
    //destructor
    ~QtclConsole();
    //callback method that implements the history command
    int showHistory( ClientData client_data, Tcl_Interp* interp, int argc, const char *argv[]);
    //callback method that implements the set_prompt command
    int setPrompt( ClientData client_data, Tcl_Interp* interp, int argc, const char *argv[]);
    //callback method that calls the saveScript() method
    int saveScript( ClientData client_data, Tcl_Interp* interp, int argc, const char *argv[]);
    //get the QtclConsole instance
    static QtclConsole *getInstance(QWidget *parent = NULL, const char *name = NULL);

    private:
    //Tcl interpreter
    Tcl_Interp *interp;
    //The instance
    static QtclConsole *theInstance;

    private:
    //Return false if the tcl command is incomplete (e.g. unmatched braces)
    bool isCommandComplete(QString command);
    //private constructor
    QtclConsole(QWidget *parent = NULL, const char *name = NULL);
    //execute a validated command
    QString interpretCommand(QString command, int *res);
    //give suggestions to autocomplete a command
    QStringList autocompleteCommand(QString cmd);
};

#endif
