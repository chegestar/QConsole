/***************************************************************************
													qtclconsole.h  -  description
														 -------------------
		begin                : mar mar 15 2005
		copyright            : (C) 2005 by Houssem BDIOUI
		email                : houssem.bdioui@gmail.com
 ***************************************************************************/

 /*
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301  USA
 */

#ifndef QTCLCONSOLE_H
#define QTCLCONSOLE_H

#include <tcl.h>

#include "qconsole.h"
#include "qmutex.h"

/**An emulated singleton class console for Tcl within a Qt application (based on the QConsole class)
 *@author Houssem BDIOUI
 */

class QtclConsole : public QConsole
{
		friend int ConsoleOutput(ClientData, CONST char * buf,
														 int toWrite, int *errorCode);
		friend int ConsoleError(ClientData, CONST char * buf,
														int toWrite, int *errorCode);
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
		static QtclConsole *getInstance(QWidget *parent = NULL, const QString &welcomeText = "");

private:
		//Tcl interpreter
		Tcl_Interp *interp;
		//The instance
		static QtclConsole *theInstance;
		QMutex mutex;

private:
		//Return false if the tcl command is incomplete (e.g. unmatched braces)
		bool isCommandComplete(const QString &command);
		//private constructor
		QtclConsole(QWidget *parent = NULL, const QString &welcomeText = "");
		//execute a validated command
		QString interpretCommand(const QString &command, int *res);
		//give suggestions to autocomplete a command
		QStringList suggestCommand(const QString &cmd, QString& prefix);
};

#endif
