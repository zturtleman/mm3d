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


#include "animeditwin.h"
#include "helpwin.h"

#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>
#include <QtWidgets/QShortcut>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QCheckBox>

#define COLUMN_NAME 0
#define COLUMN_FRAMECOUNT 1
#define COLUMN_FPS 2
#define COLUMN_LOOPING 3

AnimEditFrameCountDelegate::AnimEditFrameCountDelegate( QObject *parent )
   : QStyledItemDelegate( parent )
{
}

QWidget *AnimEditFrameCountDelegate::createEditor( QWidget *parent,
   const QStyleOptionViewItem &/* option */,
   const QModelIndex &/* index */ ) const
{
   QSpinBox *editor = new QSpinBox( parent );
   editor->setFrame( false );
   editor->setMinimum( 0 );
   editor->setMaximum( 9999 );

   return editor;
}

void AnimEditFrameCountDelegate::setEditorData( QWidget *editor,
   const QModelIndex &index ) const
{
   int value = index.model()->data( index, Qt::EditRole ).toInt();

   QSpinBox *spinBox = static_cast<QSpinBox*>( editor );
   spinBox->setValue( value );
}

void AnimEditFrameCountDelegate::setModelData( QWidget *editor, QAbstractItemModel *model,
   const QModelIndex &index ) const
{
   QSpinBox *spinBox = static_cast<QSpinBox*>( editor );
   spinBox->interpretText();
   int value = spinBox->value();

   model->setData( index, value, Qt::EditRole );
}

void AnimEditFrameCountDelegate::updateEditorGeometry( QWidget *editor,
   const QStyleOptionViewItem &option, const QModelIndex &/* index */ ) const
{
   editor->setGeometry( option.rect );
}


AnimEditWindow::AnimEditWindow( QWidget * parent )
   : QDialog( parent ),
     m_frameCountDelegate( NULL ),
     m_model( NULL ),
     m_mode( Model::AnimationModeE::ANIMMODE_NONE ),
     m_animIndicies()
{
   setupUi( this );
   setModal( true );

   m_frameCountDelegate = new AnimEditFrameCountDelegate();
   m_animTable->setItemDelegateForColumn( COLUMN_FRAMECOUNT, m_frameCountDelegate );
   m_animTable->horizontalHeader()->resizeSection( COLUMN_NAME, 300 );
   m_animTable->horizontalHeader()->resizeSection( COLUMN_FRAMECOUNT, 100 );
   m_animTable->horizontalHeader()->resizeSection( COLUMN_FPS, 100 );
   m_animTable->horizontalHeader()->resizeSection( COLUMN_LOOPING, 50 );
   m_animTable->setSelectionMode( QAbstractItemView::SelectionMode::NoSelection );
   m_animTable->setEditTriggers( QAbstractItemView::CurrentChanged );
}

AnimEditWindow::~AnimEditWindow()
{
}

void AnimEditWindow::setAnimationData( Model * model, Model::AnimationModeE mode, const std::list<unsigned> & animIndicies )
{
   m_model = model;
   m_mode = mode;
   m_animIndicies = animIndicies;

   switch ( m_mode )
   {
      case Model::ANIMMODE_SKELETAL:
         m_convertLabel->setText( tr( "Editing Skeletal animations:" ) );
         break;
      case Model::ANIMMODE_FRAME:
         m_convertLabel->setText( tr( "Editing Frame animations:" ) );
         break;
      case Model::ANIMMODE_FRAMERELATIVE:
         m_convertLabel->setText( tr( "Editing Frame Relative animations:" ) );
         break;
      default:
         m_convertLabel->setText( tr( "Editing Unknown Type animations:" ) );
         break;
   }

   m_animTable->setRowCount( m_animIndicies.size() );

   unsigned row = 0;
   std::list<unsigned>::iterator it;

   for ( it = m_animIndicies.begin(), row = 0; it != m_animIndicies.end(); it++, row++ ) {
      unsigned t = *it;
      const char * name = m_model->getAnimName( m_mode, t );
      unsigned frameCount = m_model->getAnimFrameCount( m_mode, t );
      double fps = m_model->getAnimFPS( m_mode, t );
      bool loop = m_model->getAnimLooping( m_mode, t );

      QTableWidgetItem *nameItem = new QTableWidgetItem( QString::fromUtf8( name ) );
      m_animTable->setItem( row, COLUMN_NAME, nameItem );

      QString frameCountText;
      frameCountText.setNum( frameCount );

      QTableWidgetItem *frameCountItem = new QTableWidgetItem( frameCountText );
      m_animTable->setItem( row, COLUMN_FRAMECOUNT, frameCountItem );

      QString fpsText;
      fpsText.setNum( fps );

      QTableWidgetItem *fpsItem = new QTableWidgetItem( fpsText );
      m_animTable->setItem( row, COLUMN_FPS, fpsItem );

      QString loopingText;
      loopingText.setNum( loop );

      QCheckBox *loopingItem = new QCheckBox();
      loopingItem->setChecked( loop );
      m_animTable->setCellWidget( row, COLUMN_LOOPING, loopingItem );
   }
}

void AnimEditWindow::okClicked()
{
   unsigned row = 0;
   std::list<unsigned>::iterator it;

   for ( it = m_animIndicies.begin(), row = 0; it != m_animIndicies.end(); it++, row++ ) {
      unsigned t = *it;

      QTableWidgetItem *nameItem = m_animTable->item( row, COLUMN_NAME );
      QString newName = nameItem->text();

      QTableWidgetItem *frameCountItem = m_animTable->item( row, COLUMN_FRAMECOUNT );
      unsigned newFrameCount = frameCountItem->text().toInt();

      QTableWidgetItem *fpsItem = m_animTable->item( row, COLUMN_FPS );
      double newFPS = fpsItem->text().toDouble();

      QCheckBox *loopingItem = static_cast<QCheckBox*>( m_animTable->cellWidget( row, COLUMN_LOOPING ) );
      bool newLoop = loopingItem->isChecked();

      m_model->setAnimName( m_mode, t, newName.toUtf8().data() );
      m_model->setAnimFrameCount( m_mode, t, newFrameCount );
      m_model->setAnimFPS( m_mode, t, newFPS );
      m_model->setAnimLooping( m_mode, t, newLoop );
   }

   // This isn't complete until Animation Sets exits.
   //m_model->operationComplete( tr( "Edit Animations", "operation complete" ).toUtf8() );

   accept();
}

void AnimEditWindow::cancelClicked()
{
   reject();
}
