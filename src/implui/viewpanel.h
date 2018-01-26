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


#ifndef __VIEWPANEL_H
#define __VIEWPANEL_H

#include <QtWidgets/QWidget>
#include <QtWidgets/QGridLayout>

#include "modelviewport.h"

class QGridLayout;
class ModelView;
class Model;
class Toolbox;

class ViewPanel : public QWidget
{
   Q_OBJECT

   public:
      enum
      {
         VIEWPORT_STATE_MAX = 9
      };

      ViewPanel( Toolbox * toolbox, QWidget * parent = NULL );
      virtual ~ViewPanel();

      void freeTextures();
      void setModel( Model * model );
      void frameArea( double x1, double y1, double z1, double x2, double y2, double z2 );

      unsigned getModelViewCount() { return m_viewCount; };
      ModelView * getModelView( unsigned index ) { return (index < m_viewCount) ? m_modelView[index] : NULL; };

   public slots:

      void modelUpdatedEvent();

      void wireframeEvent();
      void flatEvent();
      void smoothEvent();
      void textureEvent();
      void alphaEvent();

      void canvasWireframeEvent();
      void canvasFlatEvent();
      void canvasSmoothEvent();
      void canvasTextureEvent();
      void canvasAlphaEvent();

      void view1();
      void view1x2();
      void view2x1();
      void view2x2();
      void view2x3();
      void view3x2();
      void view3x3();

   public slots:

      void viewportSaveStateEvent(int, const ModelViewport::ViewStateT &);
      void viewportRecallStateEvent(int);

   protected:

      void deleteViews();
      void makeViews();
      void setDefaultViewDirections();

      Model       * m_model;
      QGridLayout * m_gridLayout;
      ModelView   * m_modelView[9];
      ModelViewport::ViewStateT    m_viewState[ VIEWPORT_STATE_MAX ];

      unsigned      m_viewCount;
      bool          m_tall;

      Toolbox     * m_toolbox;
};

#endif // __VIEWPANEL_H
