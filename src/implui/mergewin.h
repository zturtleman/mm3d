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

#ifndef __MERGEWIN_H
#define __MERGEWIN_H

#include "mergewin.base.h"

#include <QtGui/QDialog>

class Model;

class MergeWindow : public QDialog, public Ui::MergeWinBase
{
      Q_OBJECT
   public:
      MergeWindow( Model *, QWidget * parent = NULL );
      virtual ~MergeWindow();

      void getRotation( double * vec );
      void getTranslation( double * vec );
      bool getIncludeTexture() { return m_textureInclude->isChecked(); };
      bool getIncludeAnimation() { return m_animInclude->isChecked(); };
      bool getAnimationMerge() { return m_animInclude->isChecked() && m_animMerge->isChecked(); };

   public slots:
      void helpNowEvent();

      void includeAnimEvent( bool o );

      void accept();
      void reject();

   protected:
      Model  * m_model;
};

#endif // __MERGEWIN_H
