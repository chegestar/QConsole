/***************************************************************************
commands.cpp  -  description
-------------------
begin                : lun jun 27 2005
copyright            : (C) 2005 by houssem
email                : houssem@localhost
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

#include "../include/commands.h"
#include <qstringlist.h>
#include <qmessagebox.h>

//Global callback function that implements the msgbox command
int CallQMessageBox( ClientData, Tcl_Interp* interp, int argc, char *argv[])
{
		QString usageMsg = QString("Usage: %1 title text\n").arg(argv[0]);
		// Reset result data
		Tcl_ResetResult(interp);

		//Help message in case of wrong parameters
		if (argc != 3)
		{
				Tcl_AppendResult(interp, qPrintable(usageMsg), (char*) NULL);
				return TCL_ERROR;
		}

		//calls the messagebox with the parameters
		int result = QMessageBox::warning(0, argv[1] , argv[2], QMessageBox::Yes,
				QMessageBox::No, QMessageBox::Cancel);

		//displays the return value
		Tcl_AppendResult(interp, qPrintable(QString("QMessageBox exited with result: %1\n").arg(result)), (char*) NULL);

		return TCL_OK;
}

