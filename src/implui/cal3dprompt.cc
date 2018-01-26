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


#include "cal3dprompt.h"
#include "cal3dfilter.h"
#include "model.h"

#include "helpwin.h"

#include <QtWidgets/QRadioButton>
#include <QtWidgets/QShortcut>

Cal3dPrompt::Cal3dPrompt()
   : QDialog( NULL )
{
   setupUi( this );
   setModal( true );

   QShortcut * help = new QShortcut( QKeySequence( tr("F1", "Help Shortcut")), this );
   connect( help, SIGNAL(activated()), this, SLOT(helpNowEvent()) );
}

Cal3dPrompt::~Cal3dPrompt()
{
}

void Cal3dPrompt::setOptions( Cal3dFilter::Cal3dOptions * opts )
{
   if ( opts->m_singleMeshFile )
      m_singleMeshFile->setChecked( true );
   else
      m_separateMeshFiles->setChecked( true );

   if ( opts->m_xmlMatFile )
      m_xmlMatFile->setChecked( true );
   else
      m_binaryMatFile->setChecked( true );
}

void Cal3dPrompt::getOptions( Cal3dFilter::Cal3dOptions * opts )
{
   opts->m_singleMeshFile = m_singleMeshFile->isChecked();
   opts->m_xmlMatFile     = m_xmlMatFile->isChecked();
}

void Cal3dPrompt::helpNowEvent()
{
   HelpWin * win = new HelpWin( "olh_cal3dprompt.html", true );
   win->show();
}

// This function takes a ModelFilter::Options argument, downcasts it
// to an Cal3dOptions cal3dect, and uses it to prompt the user for
// options for the Cal3d file filter.
//
// This function is registered with the Cal3dFilter class when the
// filter is created in stdfilters.cc.
bool cal3dprompt_show( Model * model, ModelFilter::Options * o )
{
   bool rval = false;
   Cal3dPrompt p;

   Cal3dFilter::Cal3dOptions * opts = dynamic_cast< Cal3dFilter::Cal3dOptions * >( o );
   if ( opts )
   {
      opts->setOptionsFromModel( model );
      p.setOptions( opts );

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

