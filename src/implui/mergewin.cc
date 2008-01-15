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


#include "mergewin.h"

#include "model.h"
#include "glmath.h"
#include "decalmgr.h"
#include "helpwin.h"

#include <QCheckBox>
#include <QRadioButton>
#include <QPushButton>
#include <QLineEdit>
#include <q3accel.h>
#include <stdio.h>
#include <stdlib.h>

MergeWindow::MergeWindow( Model * model, QWidget * parent )
   : QDialog( parent ),
     m_accel( new Q3Accel(this) ),
     m_model( model )
{
   setupUi( this );
   setModal( true );

   m_accel->insertItem( QKeySequence( tr("F1", "Help Shortcut")), 0 );
   connect( m_accel, SIGNAL(activated(int)), this, SLOT(helpNowEvent(int)) );
}

MergeWindow::~MergeWindow()
{
}

void MergeWindow::helpNowEvent( int id )
{
   HelpWin * win = new HelpWin( "olh_mergewin.html", true );
   win->show();
}

void MergeWindow::getRotation( double * vec )
{
   if ( vec )
   {
      vec[0] = atof( m_rotX->text().latin1() ) * PIOVER180;
      vec[1] = atof( m_rotY->text().latin1() ) * PIOVER180;
      vec[2] = atof( m_rotZ->text().latin1() ) * PIOVER180;
   }
}

void MergeWindow::getTranslation( double * vec )
{
   if ( vec )
   {
      vec[0] = atof( m_transX->text().latin1() );
      vec[1] = atof( m_transY->text().latin1() );
      vec[2] = atof( m_transZ->text().latin1() );
   }
}

void MergeWindow::includeAnimEvent( bool o )
{
   m_animMerge->setEnabled( o );
   m_animAppend->setEnabled( o );
}

void MergeWindow::accept()
{
   m_model->operationComplete( tr( "Merge models", "operation complete" ).utf8() );
   QDialog::accept();
}

void MergeWindow::reject()
{
   m_model->undoCurrent();
   DecalManager::getInstance()->modelUpdated( m_model );
   QDialog::reject();
}

