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

#ifndef __ANIMEDITWIN_H
#define __ANIMEDITWIN_H

#include "animeditwin.base.h"

#include "model.h"

#include <QtWidgets/QStyledItemDelegate>
#include <QtWidgets/QDialog>

class AnimEditFrameCountDelegate : public QStyledItemDelegate
{
   Q_OBJECT

   public:
      AnimEditFrameCountDelegate( QObject *parent = NULL );

      QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index ) const override;

      void setEditorData( QWidget *editor, const QModelIndex &index ) const override;
      void setModelData( QWidget *editor, QAbstractItemModel *model,
                         const QModelIndex &index ) const override;

      void updateEditorGeometry( QWidget *editor, const QStyleOptionViewItem &option,
                                 const QModelIndex &index ) const override;
};

class AnimEditWindow : public QDialog, public Ui::AnimEditWinBase
{
   Q_OBJECT

   public:
      AnimEditWindow( QWidget * parent = NULL );
      virtual ~AnimEditWindow();

      void setAnimationData( Model * model, Model::AnimationModeE mode, const std::list<unsigned> & animIndicies );

   public slots:
      void helpNowEvent();

      void okClicked();
      void cancelClicked();

   protected:
      AnimEditFrameCountDelegate *m_frameCountDelegate;

      Model *m_model;
      Model::AnimationModeE m_mode;
      std::list<unsigned> m_animIndicies;
};

#endif // __ANIMCONVERTWIN_H
