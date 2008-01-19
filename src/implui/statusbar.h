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


#ifndef __STATUSBAR_H
#define __STATUSBAR_H

#include "statusbar.base.h"

#include "modelstatus.h"

#include <QPalette>
#include <QWidget>

#include <map>
#include <list>

class Model;

class StatusBar : public QWidget, public Ui::StatusBarBase, public StatusObject
{
   Q_OBJECT

   public:

      static StatusObject * getStatusBarFromModel( Model * model );

      StatusBar( Model * model, QWidget * parent );
      virtual ~StatusBar();

      Model * getModel() { return m_model; };
      void setModel( Model * m_model );

      // Status text
      void setText( const char * str );
      void addText( StatusTypeE type, unsigned ms, const char * str );

      // Second (optional) argument is num selected
      void setVertices(   unsigned v, unsigned sv = 0 );
      void setFaces(      unsigned f, unsigned sf = 0 );
      void setGroups(     unsigned g, unsigned sg = 0 );
      void setBoneJoints( unsigned b, unsigned sb = 0 );
      void setPoints(     unsigned p, unsigned sp = 0 );
      void setTextures(   unsigned t, unsigned st = 0 );

   public slots:
      void timerExpired();

   protected:
      struct _TextQueueItem_t
      {
         StatusTypeE type;
         unsigned ms;
         QString str;
      };
      typedef struct _TextQueueItem_t TextQueueItemT;

      Model * m_model;
      QPalette m_palette;

      std::list<TextQueueItemT> m_queue;
      bool m_queueDisplay;

      static std::map< Model *, StatusBar * > s_modelMap;
};

#endif // __STATUSBAR_H
