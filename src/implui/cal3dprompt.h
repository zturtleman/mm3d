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


#ifndef __CAL3DPROMPT_H
#define __CAL3DPROMPT_H

#include "cal3dprompt.base.h"
#include "modelfilter.h"
#include "cal3dfilter.h"

#include <QtGui/QDialog>

class Model;

class Cal3dPrompt : public QDialog, public Ui::Cal3dPromptBase
{
   Q_OBJECT

   public:
      Cal3dPrompt();
      virtual ~Cal3dPrompt();

      void setOptions( Cal3dFilter::Cal3dOptions * o );
      void getOptions( Cal3dFilter::Cal3dOptions * o );

   public slots:
      void helpNowEvent();

   protected:
};

bool cal3dprompt_show( Model * model, ModelFilter::Options * o );

#endif // __CAL3DPROMPT_H
