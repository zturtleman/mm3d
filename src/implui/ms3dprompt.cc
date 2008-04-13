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


#include "ms3dprompt.h"
#include "ms3dfilter.h"
#include "model.h"
#include "mq3compat.h"
#include "mm3dport.h"
#include "helpwin.h"

#include <qradiobutton.h>
#include <qlineedit.h>

Ms3dPrompt::Ms3dPrompt()
   : Ms3dPromptBase( NULL, "", true ),
     m_accel( new QAccel(this) )
{
   m_accel->insertItem( QKeySequence( tr("F1", "Help Shortcut")), 0 );
   connect( m_accel, SIGNAL(activated(int)), this, SLOT(helpNowEvent(int)) );
}

Ms3dPrompt::~Ms3dPrompt()
{
}

void Ms3dPrompt::setOptions( Ms3dFilter::Ms3dOptions * opts )
{
   switch ( opts->m_subVersion )
   {
      case 0:
      default:
         m_subVersion0->setChecked( true );
         break;
      case 1:
         m_subVersion1->setChecked( true );
         break;
      case 2:
         m_subVersion2->setChecked( true );
         break;
   }
   char str[20];
   PORT_snprintf( str, sizeof(str), "%X", opts->m_vertexExtra );
   m_vertexExtra->setText( str );

   // TODO joint color
   //PORT_snprintf( str, sizeof(str), "%X", opts->m_jointColor );
   //m_jointColor->setText( str );

   updateExtraEnabled();
}

void Ms3dPrompt::getOptions( Ms3dFilter::Ms3dOptions * opts )
{
   opts->m_subVersion = 0;
   if ( m_subVersion1->isChecked() )
      opts->m_subVersion = 1;
   if ( m_subVersion2->isChecked() )
      opts->m_subVersion = 2;

   uint32_t val = 0xffffffff;
   sscanf( m_vertexExtra->text().utf8(), "%X", &val);
   opts->m_vertexExtra   = val;

   // TODO joint color
   //val = 0xffffffff;
   //sscanf( m_jointColor->text().utf8(), "%X", &val);
   //opts->m_vertexExtra   = val;
}

void Ms3dPrompt::helpNowEvent( int id )
{
   HelpWin * win = new HelpWin( "olh_ms3dprompt.html", true );
   win->show();
}

void Ms3dPrompt::subVersionChangedEvent( int id )
{
   updateExtraEnabled();
}

void Ms3dPrompt::updateExtraEnabled()
{
   m_vertexExtra->setEnabled( m_subVersion2->isChecked() );
   // TODO joint color
   //m_jointColor->setEnabled( !m_subVersion0->isChecked() );
}

// This function takes a ModelFilter::Options argument, downcasts it
// to an Ms3dOptions object, and uses it to prompt the user for
// options for the MS3D file filter.
//
// This function is registered with the Ms3dFilter class when the
// filter is created in stdfilters.cc.
bool ms3dprompt_show( Model * model, ModelFilter::Options * o )
{
   bool rval = false;
   Ms3dPrompt p;

   Ms3dFilter::Ms3dOptions * opts
      = dynamic_cast< Ms3dFilter::Ms3dOptions * >( o );
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

