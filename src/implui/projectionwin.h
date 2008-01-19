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


#ifndef __POINTWIN_H
#define __POINTWIN_H

#include "projectionwin.base.h"
#include "model.h"

#include <QDialog>
#include <QCloseEvent>

class ViewPanel;
class TextureWidget;

class ProjectionWin : public QDialog, public Ui::ProjectionWinBase, public Model::Observer
{
   Q_OBJECT

   public:
      ProjectionWin( Model * model, QWidget * parent, ViewPanel * viewPanel );
      virtual ~ProjectionWin();

      void refreshProjectionDisplay();
      void addProjectionTriangles();

      // Model::Observer methods
      void modelChanged( int changeBits );

   public slots:
      void show();
      void setModel( Model * m );
      void zoomIn();
      void zoomOut();
      void undoEvent();
      void redoEvent();
      void helpNowEvent();

   protected slots:
      void closeEvent( QCloseEvent * e );
      void typeChangedEvent(int);
      void addFacesEvent();
      void removeFacesEvent();
      void applyProjectionEvent();
      void resetClickedEvent();
      void renameClickedEvent();
      void projectionIndexChangedEvent(int);
      void zoomChangeEvent();
      void zoomLevelChangedEvent( QString zoomStr );
      void rangeChangedEvent();
      void seamChangedEvent( double xDiff, double yDiff );

   protected:
      void initWindow();
      void applyProjection();
      int  getSelectedProjection();
      void operationComplete( const char * opname );

      Model  * m_model;
      ViewPanel * m_viewPanel;
      TextureWidget * m_textureWidget;
      int      m_undoCount;
      int      m_redoCount;
      bool     m_inUndo;
      bool     m_ignoreChange;
};

#endif // __POINTWIN_H
