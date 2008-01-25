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

#include <QComboBox>
#include <QMessageBox>
#include <QPushButton>
#include <QInputDialog>
#include <QLineEdit>
#include <QSlider>
#include <QLabel>
#include <QShortcut>

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
   double x = atof( m_transX->text().toUtf8() );
   double y = atof( m_transY->text().toUtf8() );
   double z = atof( m_transZ->text().toUtf8() );

   Matrix m;
   m.setTranslation( x, y, z );

   applyMatrix( m,  tr("Matrix Translate") );
}

void TransformWindow::rotateEulerEvent()
{
   double vec[3];
   vec[0] = atof( m_rotX->text().toUtf8() );
   vec[1] = atof( m_rotY->text().toUtf8() );
   vec[2] = atof( m_rotZ->text().toUtf8() );

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
   vec[0] = atof( m_axisX->text().toUtf8() );
   vec[1] = atof( m_axisY->text().toUtf8() );
   vec[2] = atof( m_axisZ->text().toUtf8() );
   double angle = atof( m_angle->text().toUtf8() );

   angle = angle * PIOVER180; // convert to radians

   Matrix m;
   m.setRotationOnAxis( vec, angle );

   applyMatrix( m, tr("Matrix Rotate On Axis") );
}

void TransformWindow::scaleEvent()
{
   double x = atof( m_scaleX->text().toUtf8() );
   double y = atof( m_scaleY->text().toUtf8() );
   double z = atof( m_scaleZ->text().toUtf8() );

   Matrix m;
   m.set( 0, 0, x );
   m.set( 1, 1, y );
   m.set( 2, 2, z );

   applyMatrix( m, tr("Matrix Scale") );
}

void TransformWindow::matrixEvent()
{
   Matrix m;
   m.set( 0, 0, atof( m_00->text().toUtf8() ) );
   m.set( 0, 1, atof( m_01->text().toUtf8() ) );
   m.set( 0, 2, atof( m_02->text().toUtf8() ) );
   m.set( 0, 3, atof( m_03->text().toUtf8() ) );
   m.set( 1, 0, atof( m_10->text().toUtf8() ) );
   m.set( 1, 1, atof( m_11->text().toUtf8() ) );
   m.set( 1, 2, atof( m_12->text().toUtf8() ) );
   m.set( 1, 3, atof( m_13->text().toUtf8() ) );
   m.set( 2, 0, atof( m_20->text().toUtf8() ) );
   m.set( 2, 1, atof( m_21->text().toUtf8() ) );
   m.set( 2, 2, atof( m_22->text().toUtf8() ) );
   m.set( 2, 3, atof( m_23->text().toUtf8() ) );
   m.set( 3, 0, atof( m_30->text().toUtf8() ) );
   m.set( 3, 1, atof( m_31->text().toUtf8() ) );
   m.set( 3, 2, atof( m_32->text().toUtf8() ) );
   m.set( 3, 3, atof( m_33->text().toUtf8() ) );

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
