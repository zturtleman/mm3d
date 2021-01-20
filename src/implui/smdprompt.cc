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


#include "smdprompt.h"
#include "smdfilter.h"
#include "model.h"

#include "helpwin.h"

#include <QtWidgets/QSpinBox>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QShortcut>

SmdPrompt::SmdPrompt()
   : QDialog( NULL )
{
   setupUi( this );
   setModal( true );

   QShortcut * help = new QShortcut( QKeySequence( tr("F1", "Help Shortcut")), this );
   connect( help, SIGNAL(activated()), this, SLOT(helpNowEvent()) );
}

SmdPrompt::~SmdPrompt()
{
}

void SmdPrompt::setOptions( SmdFilter::SmdOptions * opts, Model * model )
{
   m_saveMeshes->setChecked( opts->m_saveMeshes );
   m_saveAnimation->setChecked( !opts->m_saveMeshes );
   m_savePointsJoint->setChecked( opts->m_savePointsJoint );
   m_singleVertexInfluence->setChecked( !opts->m_multipleVertexInfluences );
   m_multipleVertexInfluences->setChecked( opts->m_multipleVertexInfluences );

   m_singleVertexInfluence->setEnabled( opts->m_saveMeshes );
   m_multipleVertexInfluences->setEnabled( opts->m_saveMeshes );
   m_animList->setEnabled( !opts->m_saveMeshes );

   m_animList->clear();

   unsigned count = model->getAnimCount( Model::ANIMMODE_SKELETAL );
   for ( unsigned t = 0; t < count; t++ )
   {
      m_animList->insertItem( t, QString::fromUtf8( model->getAnimName( Model::ANIMMODE_SKELETAL, t ) ) );
   }
   if ( count > 0 )
   {
      m_animList->item(0)->setSelected( true );
   }
}

void SmdPrompt::getOptions( SmdFilter::SmdOptions * opts )
{
   opts->m_saveMeshes     = m_saveMeshes->isChecked();
   opts->m_savePointsJoint= m_savePointsJoint->isChecked();
   opts->m_multipleVertexInfluences = m_multipleVertexInfluences->isChecked();
   opts->m_animations.clear();

   if ( opts->m_saveMeshes )
   {
      opts->m_animations.push_back( UINT_MAX );
   }
   else
   {
      unsigned count = m_animList->count();
      for ( unsigned t = 0; t < count; t++ )
      {
         if ( m_animList->item( t )->isSelected() )
         {
            opts->m_animations.push_back( t );
         }
      }
   }
}

void SmdPrompt::helpNowEvent()
{
   HelpWin * win = new HelpWin( "olh_smdprompt.html", true );
   win->show();
}

void SmdPrompt::saveMeshesChangedEvent()
{
   m_singleVertexInfluence->setEnabled( m_saveMeshes->isChecked() );
   m_multipleVertexInfluences->setEnabled( m_saveMeshes->isChecked() );

   m_animList->setEnabled( m_saveAnimation->isChecked() );
}

bool smdprompt_show( Model * model, const char * const filename, ModelFilter::Options * o )
{
   bool rval = false;
   SmdPrompt p;

   SmdFilter::SmdOptions * opts = dynamic_cast< SmdFilter::SmdOptions * >( o );
   if ( opts )
   {
      opts->setOptionsFromModel( model );
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
