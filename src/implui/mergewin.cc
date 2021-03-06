/*  Maverick Model 3D
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

#include <QtWidgets/QCheckBox>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QShortcut>

#include <stdio.h>
#include <stdlib.h>

MergeWindow::MergeWindow( Model * model, Model * existingModel, QWidget * parent )
   : QDialog( parent ),
     m_model( model ),
     m_existingModel( existingModel )
{
   setupUi( this );
   setModal( true );

   QShortcut * help = new QShortcut( QKeySequence( tr("F1", "Help Shortcut")), this );
   connect( help, SIGNAL(activated()), this, SLOT(helpNowEvent()) );

   int t;
   // index 0 is <origin>
   for ( t = 0; t < m_existingModel->getPointCount(); t++ )
   {
      m_pointName->insertItem( t + 1,  QString::fromUtf8( m_existingModel->getPointName(t) ) );
   }

   list<int> points;
   m_existingModel->getSelectedPoints( points );
   if ( ! points.empty() )
   {
      m_pointName->setCurrentIndex( points.front() + 1 );
   }
   else
   {
      m_pointName->setCurrentIndex( 0 );
   }
}

MergeWindow::~MergeWindow()
{
}

void MergeWindow::helpNowEvent()
{
   HelpWin * win = new HelpWin( "olh_mergewin.html", true );
   win->show();
}

int MergeWindow::getPoint()
{
   return m_pointName->currentIndex() - 1;
}

void MergeWindow::getRotation( double * vec )
{
   if ( vec )
   {
      vec[0] = m_rotX->text().toDouble() * PIOVER180;
      vec[1] = m_rotY->text().toDouble() * PIOVER180;
      vec[2] = m_rotZ->text().toDouble() * PIOVER180;
   }
}

void MergeWindow::getTranslation( double * vec )
{
   if ( vec )
   {
      vec[0] = m_transX->text().toDouble();
      vec[1] = m_transY->text().toDouble();
      vec[2] = m_transZ->text().toDouble();
   }
}

void MergeWindow::includeAnimEvent( bool o )
{
   m_animMerge->setEnabled( o );
   m_animAppend->setEnabled( o );
}

void MergeWindow::accept()
{
   m_model->operationComplete( tr( "Merge models", "operation complete" ).toUtf8() );
   QDialog::accept();
}

void MergeWindow::reject()
{
   m_model->undoCurrent();
   DecalManager::getInstance()->modelUpdated( m_model );
   QDialog::reject();
}

