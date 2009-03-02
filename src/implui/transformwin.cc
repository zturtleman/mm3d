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

#include <QtGui/QComboBox>
#include <QtGui/QMessageBox>
#include <QtGui/QPushButton>
#include <QtGui/QInputDialog>
#include <QtGui/QLineEdit>
#include <QtGui/QSlider>
#include <QtGui/QLabel>
#include <QtGui/QShortcut>

#include <list>
#include <string>

using std::list;
using std::string;

TransformWindow::TransformWindow( Model * model, QWidget * parent )
   : QDialog( parent ),
     m_model( model )
{
   setupUi( this );
   QShortcut * help = new QShortcut( QKeySequence( tr("F1", "Help Shortcut")), this );
   connect( help, SIGNAL(activated()), this, SLOT(helpNowEvent()) );
}

TransformWindow::~TransformWindow()
{
}

void TransformWindow::helpNowEvent()
{
   HelpWin * win = new HelpWin( "olh_transformwin.html", true );
   win->show();
}

void TransformWindow::close()
{
   QDialog::hide();
}

void TransformWindow::setModel( Model * m )
{
   m_model = m;
}

void TransformWindow::translateEvent()
{
   double x = m_transX->text().toDouble();
   double y = m_transY->text().toDouble();
   double z = m_transZ->text().toDouble();

   Matrix m;
   m.setTranslation( x, y, z );

   applyMatrix( m,  tr("Matrix Translate") );
}

void TransformWindow::rotateEulerEvent()
{
   double vec[3];
   vec[0] = m_rotX->text().toDouble();
   vec[1] = m_rotY->text().toDouble();
   vec[2] = m_rotZ->text().toDouble();

   vec[0] *= PIOVER180; // convert to radians
   vec[1] *= PIOVER180; // convert to radians
   vec[2] *= PIOVER180; // convert to radians

   Matrix m;
   m.setRotation( vec );

   applyMatrix( m, tr("Matrix Rotate") );
}

void TransformWindow::rotateQuaternionEvent()
{
   double vec[3];
   vec[0] = m_axisX->text().toDouble();
   vec[1] = m_axisY->text().toDouble();
   vec[2] = m_axisZ->text().toDouble();
   double angle = m_angle->text().toDouble();

   angle = angle * PIOVER180; // convert to radians

   Matrix m;
   m.setRotationOnAxis( vec, angle );

   applyMatrix( m, tr("Matrix Rotate On Axis") );
}

void TransformWindow::scaleEvent()
{
   double x = m_scaleX->text().toDouble();
   double y = m_scaleY->text().toDouble();
   double z = m_scaleZ->text().toDouble();

   Matrix m;
   m.set( 0, 0, x );
   m.set( 1, 1, y );
   m.set( 2, 2, z );

   applyMatrix( m, tr("Matrix Scale") );
}

void TransformWindow::matrixEvent()
{
   Matrix m;
   m.set( 0, 0, m_00->text().toDouble() );
   m.set( 0, 1, m_01->text().toDouble() );
   m.set( 0, 2, m_02->text().toDouble() );
   m.set( 0, 3, m_03->text().toDouble() );
   m.set( 1, 0, m_10->text().toDouble() );
   m.set( 1, 1, m_11->text().toDouble() );
   m.set( 1, 2, m_12->text().toDouble() );
   m.set( 1, 3, m_13->text().toDouble() );
   m.set( 2, 0, m_20->text().toDouble() );
   m.set( 2, 1, m_21->text().toDouble() );
   m.set( 2, 2, m_22->text().toDouble() );
   m.set( 2, 3, m_23->text().toDouble() );
   m.set( 3, 0, m_30->text().toDouble() );
   m.set( 3, 1, m_31->text().toDouble() );
   m.set( 3, 2, m_32->text().toDouble() );
   m.set( 3, 3, m_33->text().toDouble() );

   applyMatrix( m, tr("Apply Matrix") );
}

bool TransformWindow::matrixIsUndoable( const Matrix & m )
{
   if ( fabs( m.getDeterminant() ) >= 0.0006 ) 
      return true;
   else
      return false;
}

bool TransformWindow::warnNoUndo( bool undoable )
{
   if ( !undoable ) 
   {
      if ( QMessageBox::warning( NULL, tr("Transform Cannot Be Undone", "window title"), tr("This transformation cannot be undone.") + QString("\n") + tr("Are you sure you wish to continue?" ),
               tr("Apply Transformation", "button" ), tr("Cancel Transformation", "button" ) ) == 0 )
         return true;
      else
         return false;
   }
   return true;
}

void TransformWindow::applyMatrix( const Matrix & m, const QString & action )
{
   bool undoable = matrixIsUndoable( m );
   if ( warnNoUndo( undoable ) )
   {
      Model::OperationScopeE scope = (m_scope->currentIndex() == 0)
         ? Model::OS_Selected : Model::OS_Global;
      m_model->applyMatrix( m, scope, true, undoable );
      m_model->operationComplete( action.toUtf8() );
      DecalManager::getInstance()->modelUpdated( m_model );
   }
}
