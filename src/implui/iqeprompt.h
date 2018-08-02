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


#ifndef __IQEPROMPT_H
#define __IQEPROMPT_H

#include "iqeprompt.base.h"
#include "modelfilter.h"
#include "iqefilter.h"

#include <QtWidgets/QDialog>

class Model;

class IqePrompt : public QDialog, public Ui::IqePromptBase
{
   Q_OBJECT

   public:
      IqePrompt();
      virtual ~IqePrompt();

      void setOptions( IqeFilter::IqeOptions * o, Model * model );
      void getOptions( IqeFilter::IqeOptions * o );

   public slots:
      void helpNowEvent();

   protected:

};

bool iqeprompt_show( Model * model, ModelFilter::Options * o );

#endif // __IQEPROMPT_H
