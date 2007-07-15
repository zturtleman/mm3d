/*  Misfit Model 3D
 * 
 *  Copyright (c) 2004-2007 Kevin Worcester
 * 
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, 
 *  USA.
 *
 *  See the COPYING file for full license text.
 */


#ifndef __MQ3COMPAT_H
#define __MQ3COMPAT_H

#include "config.h"

#ifdef WIN32
#include <windows.h>
#include <winnt.h>
#endif // WIN32

#ifdef HAVE_QT4

#include <qpalette.h>
#include <q3accel.h>
//#include <q3buttongroup.h>
#include <q3dockwindow.h>
#include <q3dragobject.h>
#include <qfiledialog.h>
#include <q3frame.h>
//#include <q3groupbox.h>
#include <q3listbox.h>
#include <q3listview.h>
#include <q3mainwindow.h>
#include <q3popupmenu.h>

using namespace Qt;

#else

#include <qaccel.h>
#include <qbuttongroup.h>
#include <qdockwindow.h>
#include <qdragobject.h>
#include <qfiledialog.h>
#include <qframe.h>
#include <qgroupbox.h>
#include <qlistbox.h>
#include <qlistview.h>
#include <qmainwindow.h>
#include <qpixmap.h>
#include <qpopupmenu.h>

#endif // HAVE_QT4


#endif // __MQ3COMPAT_H
