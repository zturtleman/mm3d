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


#ifndef __OBJPROMPT_H
#define __OBJPROMPT_H

#include "objprompt.base.h"
#include "modelfilter.h"
#include "objfilter.h"

#include <QtGui/QDialog>

class Model;

class ObjPrompt : public QDialog, public Ui::ObjPromptBase
{
   Q_OBJECT

   public:
      ObjPrompt();
      virtual ~ObjPrompt();

      void setOptions( ObjFilter::ObjOptions * o );
      void getOptions( ObjFilter::ObjOptions * o );

   public slots:
      void helpNowEvent();

   protected:

};

bool objprompt_show( Model * model, ModelFilter::Options * o );

#endif // __OBJPROMPT_H
