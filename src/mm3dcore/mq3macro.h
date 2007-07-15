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


#ifndef __MQ3MACRO_H
#define __MQ3MACRO_H

#include "config.h"

#ifdef HAVE_QT4

#define QAccel Q3Accel
#define QDockWindow Q3DockWindow
#define QDragObject Q3DragObject
//#define QGroupBox Q3GroupBox
#define QListBox Q3ListBox
#define QListView Q3ListView
#define QListViewItem Q3ListViewItem
#define QMainWindow Q3MainWindow
#define QPopupMenu Q3PopupMenu
//#define setCaption setWindowTitle

#else

#define SmallIconSize QIconSet::Small

#endif // HAVE_QT4

#endif // __MQ3MACRO_H
