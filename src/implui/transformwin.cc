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


#include "transformwin.h"
#include "textureframe.h"
#include "texwidget.h"
#include "model.h"
#include "texture.h"
#include "texmgr.h"
#include "log.h"
#include "rgbawin.h"
#include "valuewin.h"
#include "decalmgr.h"
#include "msg.h"
#include "3dmprefs.h"
#include "helpwin.h"

#include "mq3compat.h"

#include <qcombobox.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
#include <list>
#include <string>
#include <qinputdialog.h>
#include <qlineedit.h>
#include <qslider.h>
#include <qlabel.h>

using std::list;
using std::string;

TransformWindow::TransformWindow( Model * model, QWidget * parent, const char * name )
   : TransformWindowBase( parent, name ),
     m_accel( new QAccel(this) ),
     m_model( model )
{
}

TransformWindow::~TransformWindow()
{
}

void TransformWindow::helpNowEvent( int id )
{
   HelpWin * win = new HelpWin( "olh_transformwin.html", true );
   win->show();
}

void TransformWindow::close()
{
   TransformWindowBase::hide();
}

void TransformWindow::setModel( Model * m )
{
   m_model = m;
}

void TransformWindow::translateEvent()
{
   double x = atof( m_transX->text().utf8() );
   double y = atof( m_transY->text().utf8() );
   double z = atof( m_transZ->text().utf8() );

   Matrix m;
   m.setTranslation( x, y, z );

   if ( warnNoUndo( matrixIsUndoable( m ) ) )
   {
      m_model->applyMatrix( m, Model::OS_Global, true, m_undoable );
      m_model->operationComplete( tr("Matrix Translate").utf8() );
      DecalManager::getInstance()->modelUpdated( m_model );
   }
}

void TransformWindow::rotateEulerEvent()
{
   double vec[3];
   vec[0] = atof( m_rotX->text().utf8() );
   vec[1] = atof( m_rotY->text().utf8() );
   vec[2] = atof( m_rotZ->text().utf8() );

   vec[0] *= PIOVER180; // convert to radians
   vec[1] *= PIOVER180; // convert to radians
   vec[2] *= PIOVER180; // convert to radians

   Matrix m;
   m.setRotation( vec );

   if ( warnNoUndo( matrixIsUndoable( m ) ) )
   {
      m_model->applyMatrix( m, Model::OS_Global, true, m_undoable );
      m_model->operationComplete( tr("Matrix Rotate").utf8() );
      DecalManager::getInstance()->modelUpdated( m_model );
   }
}

void TransformWindow::rotateQuaternionEvent()
{
   double vec[3];
   vec[0] = atof( m_axisX->text().utf8() );
   vec[1] = atof( m_axisY->text().utf8() );
   vec[2] = atof( m_axisZ->text().utf8() );
   double angle = atof( m_angle->text().utf8() );

   angle = angle * PIOVER180; // convert to radians

   Matrix m;
   m.setRotationOnAxis( vec, angle );

   if ( warnNoUndo( matrixIsUndoable( m ) ) )
   {
      m_model->applyMatrix( m, Model::OS_Global, true, m_undoable );
      m_model->operationComplete( tr("Matrix Rotate On Axis").utf8() );
      DecalManager::getInstance()->modelUpdated( m_model );
   }
}

void TransformWindow::scaleEvent()
{
   double x = atof( m_scaleX->text().utf8() );
   double y = atof( m_scaleY->text().utf8() );
   double z = atof( m_scaleZ->text().utf8() );

   Matrix m;
   m.set( 0, 0, x );
   m.set( 1, 1, y );
   m.set( 2, 2, z );

   if ( warnNoUndo( matrixIsUndoable( m ) ) )
   {
      m_model->applyMatrix( m, Model::OS_Global, true, m_undoable );
      m_model->operationComplete( tr("Matrix Scale").utf8() );
      DecalManager::getInstance()->modelUpdated( m_model );
   }
}

void TransformWindow::matrixEvent()
{
   Matrix m;
   m.set( 0, 0, atof( m_00->text().utf8() ) );
   m.set( 0, 1, atof( m_01->text().utf8() ) );
   m.set( 0, 2, atof( m_02->text().utf8() ) );
   m.set( 0, 3, atof( m_03->text().utf8() ) );
   m.set( 1, 0, atof( m_10->text().utf8() ) );
   m.set( 1, 1, atof( m_11->text().utf8() ) );
   m.set( 1, 2, atof( m_12->text().utf8() ) );
   m.set( 1, 3, atof( m_13->text().utf8() ) );
   m.set( 2, 0, atof( m_20->text().utf8() ) );
   m.set( 2, 1, atof( m_21->text().utf8() ) );
   m.set( 2, 2, atof( m_22->text().utf8() ) );
   m.set( 2, 3, atof( m_23->text().utf8() ) );
   m.set( 3, 0, atof( m_30->text().utf8() ) );
   m.set( 3, 1, atof( m_31->text().utf8() ) );
   m.set( 3, 2, atof( m_32->text().utf8() ) );
   m.set( 3, 3, atof( m_33->text().utf8() ) );

   if ( warnNoUndo( matrixIsUndoable( m ) ) )
   {
      m_model->applyMatrix( m, Model::OS_Global, true, m_undoable );
      m_model->operationComplete( tr("Apply Matrix").utf8() );
      DecalManager::getInstance()->modelUpdated( m_model );
   }
}

bool TransformWindow::matrixIsUndoable( const Matrix & m )
{
   if ( fabs( m.getDeterminant() ) >= 0.0006 ) 
   {
      m_undoable = true;
      return true;
   }
   else
   {
      m_undoable = false;
      return false;
   }
}

bool TransformWindow::warnNoUndo( bool undoable )
{
   if ( !undoable ) 
   {
      if ( QMessageBox::warning( NULL, tr("Transform Cannot Be Undone", "window title"), tr("This transformation cannot be undone.") + QString("\n") + tr("Are you sure you wish to continue?" ),
               tr("Apply Transformation", "button" ), tr("Cancel Transformation", "button" ) ) == 0 )
      {
         return true;
      }
      else
      {
         return false;
      }
   }
   return true;
}

