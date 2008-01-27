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


#include "animconvertwin.h"
#include "helpwin.h"

#include <QtGui/QPushButton>
#include <QtGui/QLabel>
#include <QtGui/QShortcut>
#include <QtGui/QSpinBox>
#include <QtGui/QLineEdit>

AnimConvertWindow::AnimConvertWindow( QWidget * parent )
   : QDialog( parent ),
     m_operation( OP_CONTINUE )
{
   setupUi( this );
   setModal( true );

   QShortcut * help = new QShortcut( QKeySequence( tr("F1", "Help Shortcut")), this );
   connect( help, SIGNAL(activated()), this, SLOT(helpNowEvent()) );
}

AnimConvertWindow::~AnimConvertWindow()
{
}

void AnimConvertWindow::setAnimationData( Model * model, Model::AnimationModeE mode, unsigned animIndex )
{
   switch ( mode )
   {
      case Model::ANIMMODE_SKELETAL:
         m_convertLabel->setText( tr( "Convert Skeletal to Frame:" ) );
         break;
      case Model::ANIMMODE_FRAME:
         m_convertLabel->setText( tr( "Convert Frame to Frame:" ) );
         break;
      case Model::ANIMMODE_FRAMERELATIVE:
         m_convertLabel->setText( tr( "Convert Frame Relative to Frame:" ) );
         break;
      default:
         m_convertLabel->setText( tr( "Convert Unknown Type to Frame:" ) );
         break;
   }

   const char * name = model->getAnimName( mode, animIndex );
   m_origName->setText( QString::fromUtf8( name ) );
   m_newName->setText( QString::fromUtf8( name ) );
   m_newFrameCount->setValue( model->getAnimFrameCount( mode, animIndex ) );

   m_origName->setFocus();
}

QString AnimConvertWindow::getNewName()
{
   return m_newName->text();
}

unsigned AnimConvertWindow::getNewFrameCount()
{
   return m_newFrameCount->value();
}

AnimConvertWindow::OperationE AnimConvertWindow::requestedOperation()
{
   return m_operation;
}

void AnimConvertWindow::continueClicked()
{
   m_operation = OP_CONTINUE;
   accept();
}

void AnimConvertWindow::cancelClicked()
{
   m_operation = OP_CANCEL;
   reject();
}

void AnimConvertWindow::cancelAllClicked()
{
   m_operation = OP_CANCEL_ALL;
   reject();
}

void AnimConvertWindow::helpNowEvent()
{
   HelpWin * win = new HelpWin( "olh_animconvertwin.html", true );
   win->show();
}

