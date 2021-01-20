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


#include "md3prompt.h"
#include "md3filter.h"
#include "model.h"

#include "helpwin.h"

#include <QtWidgets/QSpinBox>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QShortcut>

Md3Prompt::Md3Prompt()
   : QDialog( NULL )
{
   setupUi( this );
   setModal( true );

   QShortcut * help = new QShortcut( QKeySequence( tr("F1", "Help Shortcut")), this );
   connect( help, SIGNAL(activated()), this, SLOT(helpNowEvent()) );
}

Md3Prompt::~Md3Prompt()
{
}

void Md3Prompt::setOptions( Md3Filter::Md3Options * opts, Model * model )
{
   m_saveAsPlayer->setEnabled( opts->m_playerSupported );
   m_saveAsPlayer->setChecked( opts->m_saveAsPlayer );
   m_saveAnimationCfg->setChecked( opts->m_saveAnimationCfg );
}

void Md3Prompt::getOptions( Md3Filter::Md3Options * opts )
{
   opts->m_saveAsPlayer     = m_saveAsPlayer->isChecked();
   opts->m_saveAnimationCfg = m_saveAnimationCfg->isChecked();
}

void Md3Prompt::helpNowEvent()
{
   HelpWin * win = new HelpWin( "olh_quakemd3prompt.html", true );
   win->show();
}

bool md3prompt_show( Model * model, const char * const filename, ModelFilter::Options * o )
{
   bool rval = false;
   Md3Prompt p;

   Md3Filter::Md3Options * opts = dynamic_cast< Md3Filter::Md3Options * >( o );
   if ( opts )
   {
      opts->setOptionsFromModel( model, filename );
      p.setOptions( opts, model );

      if ( p.exec() )
      {
         rval = true;
         p.getOptions( opts );
      }
   }
   else
   {
      rval = true;
   }
   return rval;
}
