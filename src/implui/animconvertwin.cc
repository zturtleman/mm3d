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


#include "animconvertwin.h"
#include "helpwin.h"

#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>
#include <QtWidgets/QShortcut>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QLineEdit>

#define COLUMN_ORIGINALNAME 0
#define COLUMN_NAME 1
#define COLUMN_FRAMECOUNT 2

AnimConvertFrameCountDelegate::AnimConvertFrameCountDelegate( QObject *parent )
   : QStyledItemDelegate( parent )
{
}

QWidget *AnimConvertFrameCountDelegate::createEditor( QWidget *parent,
   const QStyleOptionViewItem &/* option */,
   const QModelIndex &/* index */ ) const
{
   QSpinBox *editor = new QSpinBox( parent );
   editor->setFrame( false );
   editor->setMinimum( 0 );
   editor->setMaximum( 9999 );

   return editor;
}

void AnimConvertFrameCountDelegate::setEditorData( QWidget *editor,
   const QModelIndex &index ) const
{
   int value = index.model()->data( index, Qt::EditRole ).toInt();

   QSpinBox *spinBox = static_cast<QSpinBox*>( editor );
   spinBox->setValue( value );
}

void AnimConvertFrameCountDelegate::setModelData( QWidget *editor, QAbstractItemModel *model,
   const QModelIndex &index ) const
{
   QSpinBox *spinBox = static_cast<QSpinBox*>( editor );
   spinBox->interpretText();
   int value = spinBox->value();

   model->setData( index, value, Qt::EditRole );
}

void AnimConvertFrameCountDelegate::updateEditorGeometry( QWidget *editor,
   const QStyleOptionViewItem &option, const QModelIndex &/* index */ ) const
{
   editor->setGeometry( option.rect );
}


AnimConvertWindow::AnimConvertWindow( QWidget * parent )
   : QDialog( parent ),
     m_frameCountDelegate( NULL ),
     m_model( NULL ),
     m_mode( Model::AnimationModeE::ANIMMODE_NONE ),
     m_animIndicies()
{
   setupUi( this );
   setModal( true );

   QShortcut * help = new QShortcut( QKeySequence( tr("F1", "Help Shortcut")), this );
   connect( help, SIGNAL(activated()), this, SLOT(helpNowEvent()) );

   m_frameCountDelegate = new AnimConvertFrameCountDelegate();
   m_animTable->setItemDelegateForColumn( COLUMN_FRAMECOUNT, m_frameCountDelegate );
   m_animTable->horizontalHeader()->resizeSection( COLUMN_ORIGINALNAME, 200 );
   m_animTable->horizontalHeader()->resizeSection( COLUMN_NAME, 200 );
   m_animTable->horizontalHeader()->resizeSection( COLUMN_FRAMECOUNT, 30 );
   m_animTable->setSelectionMode( QAbstractItemView::SelectionMode::NoSelection );
   m_animTable->setEditTriggers( QAbstractItemView::CurrentChanged );
}

AnimConvertWindow::~AnimConvertWindow()
{
}

void AnimConvertWindow::setAnimationData( Model * model, Model::AnimationModeE mode, const std::list<unsigned> & animIndicies )
{
   m_model = model;
   m_mode = mode;
   m_animIndicies = animIndicies;

   switch ( m_mode )
   {
      case Model::ANIMMODE_SKELETAL:
         m_convertLabel->setText( tr( "Convert Skeletal to Frame:" ) );
         m_animTable->horizontalHeaderItem( COLUMN_ORIGINALNAME )->setText( tr( "Skeletal Animation" ) );
         break;
      case Model::ANIMMODE_FRAME:
         m_convertLabel->setText( tr( "Convert Frame to Frame:" ) );
         m_animTable->horizontalHeaderItem( COLUMN_ORIGINALNAME )->setText( tr( "Frame Animation" ) );
         break;
      case Model::ANIMMODE_FRAMERELATIVE:
         m_convertLabel->setText( tr( "Convert Frame Relative to Frame:" ) );
         m_animTable->horizontalHeaderItem( COLUMN_ORIGINALNAME )->setText( tr( "Frame Relative Animation" ) );
         break;
      default:
         m_convertLabel->setText( tr( "Convert Unknown Type to Frame:" ) );
         m_animTable->horizontalHeaderItem( COLUMN_ORIGINALNAME )->setText( tr( "Unknown Type Animation" ) );
         break;
   }

   m_animTable->setRowCount( m_animIndicies.size() );

   unsigned row = 0;
   std::list<unsigned>::iterator it;

   for ( it = m_animIndicies.begin(), row = 0; it != m_animIndicies.end(); it++, row++ ) {
      unsigned t = *it;
      const char * name = m_model->getAnimName( m_mode, t );
      unsigned frameCount = m_model->getAnimFrameCount( m_mode, t );

      QTableWidgetItem *originalNameItem = new QTableWidgetItem( QString::fromUtf8( name ) );
      originalNameItem->setFlags( originalNameItem->flags() & ~Qt::ItemIsEditable );
      m_animTable->setItem( row, COLUMN_ORIGINALNAME, originalNameItem );

      QTableWidgetItem *nameItem = new QTableWidgetItem( QString::fromUtf8( name ) );
      m_animTable->setItem( row, COLUMN_NAME, nameItem );

      QString frameCountText;
      frameCountText.setNum( frameCount );

      QTableWidgetItem *frameCountItem = new QTableWidgetItem( frameCountText );
      m_animTable->setItem( row, COLUMN_FRAMECOUNT, frameCountItem );
   }
}

void AnimConvertWindow::convertClicked()
{
   unsigned row = 0;
   std::list<unsigned>::iterator it;

   for ( it = m_animIndicies.begin(), row = 0; it != m_animIndicies.end(); it++, row++ ) {
      unsigned t = *it;

      QTableWidgetItem *nameItem = m_animTable->item( row, COLUMN_NAME );
      QString newName = nameItem->text();

      QTableWidgetItem *frameCountItem = m_animTable->item( row, COLUMN_FRAMECOUNT );
      unsigned newFrameCount = frameCountItem->text().toInt();

      m_model->convertAnimToFrame( m_mode, t, newName.toUtf8().data(), newFrameCount );
   }

   accept();
}

void AnimConvertWindow::cancelClicked()
{
   reject();
}

void AnimConvertWindow::helpNowEvent()
{
   HelpWin * win = new HelpWin( "olh_animconvertwin.html", true );
   win->show();
}

