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


#include "objprompt.h"
#include "objfilter.h"
#include "model.h"

#include "helpwin.h"

#include <QSpinBox>
#include <QCheckBox>
#include <QShortcut>

ObjPrompt::ObjPrompt()
   : QDialog( NULL )
{
   setupUi( this );
   setModal( true );

   QShortcut * help = new QShortcut( QKeySequence( tr("F1", "Help Shortcut")), this );
   connect( help, SIGNAL(activated()), this, SLOT(helpNowEvent()) );
}

ObjPrompt::~ObjPrompt()
{
}

void ObjPrompt::setOptions( ObjFilter::ObjOptions * opts )
{
   m_normalsValue->setChecked( opts->m_saveNormals );
   m_placesValue->setValue( opts->m_places );
   m_texPlacesValue->setValue( opts->m_texPlaces );
   m_normalPlacesValue->setValue( opts->m_normalPlaces );
}

void ObjPrompt::getOptions( ObjFilter::ObjOptions * opts )
{
   opts->m_saveNormals   = m_normalsValue->isChecked();
   opts->m_places        = m_placesValue->value();
   opts->m_texPlaces     = m_texPlacesValue->value();
   opts->m_normalPlaces  = m_normalPlacesValue->value();
}

void ObjPrompt::helpNowEvent()
{
   HelpWin * win = new HelpWin( "olh_objprompt.html", true );
   win->show();
}

// This function takes a ModelFilter::Options argument, downcasts it
// to an ObjOptions object, and uses it to prompt the user for
// options for the OBJ file filter.
//
// If you want to provide options for your format in a plugin
// you'll need to call setOptionsPrompt on your filter after you
// create it in your plugin_init function.
//
// This function is registered with the ObjFilter class when the
// filter is created in stdfilters.cc.
bool objprompt_show( Model * model, ModelFilter::Options * o )
{
   bool rval = false;
   ObjPrompt p;

   ObjFilter::ObjOptions * opts = dynamic_cast< ObjFilter::ObjOptions * >( o );
   if ( opts )
   {
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

