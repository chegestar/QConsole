/***************************************************************************
                          tclmain.cpp  -  description
                             -------------------
    begin                : mar mar 8 2005
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

#include <qapplication.h>
#include <qmainwindow.h>
#include "commands.h"
#include "commandsManager.h"
#include "qtclconsole.h"
#include "tclnotify.h"

extern void Qtk_InitNotifier( QApplication * );

/* Define the following to fix __ctype_* from GLIBC2.3 and upper
   if not compiled using the same GLIBC */
//#define FIX__CTYPE_

#ifdef FIX__CTYPE_
#include <ctype.h>
__const unsigned short int *__ctype_b;
__const __int32_t *__ctype_tolower;
__const __int32_t *__ctype_toupper;

void ctSetup()
{
  __ctype_b = *(__ctype_b_loc());
  __ctype_toupper = *(__ctype_toupper_loc());
  __ctype_tolower = *(__ctype_tolower_loc());
}
#endif

//The main entry
int main( int argc, char ** argv )
{
#ifdef FIX__CTYPE_
    ctSetup();
#endif
    //init tcl
    Tcl_FindExecutable(argv[0]);
    Tcl_Interp * interp = Tcl_CreateInterp();
    Tcl_Init( interp );

    //Qt application
    QApplication a( argc, argv );
    Tcl_SetServiceMode (TCL_SERVICE_ALL);
    Qtk_InitNotifier( &a );
    //Register the msgbox command
    commandsManager::getInstance(interp)->registerFunction("msgbox" , (commandsManager::commandType) CallQMessageBox, "Shows the Qt message box");
    //Create and show the main window
    QMainWindow mw;
    mw.setWindowTitle("Qt/Tcl Console [By Houssem BDIOUI]");
    mw.setMinimumSize(640, 480);
    //Instantiate and set the focus to the QtclConsole
    QtclConsole *console = QtclConsole::getInstance(&mw,
                                                    "Welcome to <b>Qt / Tcl console</b> !<br>"
                                                    "For any remarks, please mail me at: <font color=blue>houssem.bdioui@gmail.com</font><br><br>");
    mw.setFocusProxy((QWidget*)console);
    mw.setCentralWidget((QWidget*)console);
    mw.show();

    return a.exec();
}
