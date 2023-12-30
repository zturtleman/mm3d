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


#include "iqeprompt.h"
#include "iqefilter.h"
#include "model.h"

#include "helpwin.h"

#include <QtWidgets/QSpinBox>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QShortcut>

IqePrompt::IqePrompt()
   : QDialog( NULL )
{
   setupUi( this );
   setModal( true );

   QShortcut * help = new QShortcut( QKeySequence( tr("F1", "Help Shortcut")), this );
   connect( help, SIGNAL(activated()), this, SLOT(helpNowEvent()) );
}

IqePrompt::~IqePrompt()
{
}

void IqePrompt::setOptions( IqeFilter::IqeOptions * opts, Model * model )
{
   m_saveAsPlayer->setEnabled( opts->m_playerSupported );
   m_saveAsPlayer->setChecked( opts->m_saveAsPlayer );
   m_saveAnimationCfg->setChecked( opts->m_saveAnimationCfg );
   m_saveMeshes->setChecked( opts->m_saveMeshes );
   m_savePointsJoint->setChecked( opts->m_savePointsJoint );
   m_saveSkeleton->setChecked( opts->m_saveSkeleton );
   m_saveAnimations->setChecked( opts->m_saveAnimations );
   m_animList->clear();

   unsigned count = model->getAnimCount( Model::ANIMMODE_SKELETAL );
   for ( unsigned t = 0; t < count; t++ )
   {
      m_animList->insertItem( t, QString::fromUtf8( model->getAnimName( Model::ANIMMODE_SKELETAL, t ) ) );
      m_animList->item(t)->setSelected( true );
   }
}

void IqePrompt::getOptions( IqeFilter::IqeOptions * opts )
{
   opts->m_saveAsPlayer     = m_saveAsPlayer->isChecked();
   opts->m_saveAnimationCfg = m_saveAnimationCfg->isChecked();
   opts->m_saveMeshes     = m_saveMeshes->isChecked();
   opts->m_savePointsJoint= m_savePointsJoint->isChecked();
   opts->m_saveSkeleton   = m_saveSkeleton->isChecked();
   opts->m_saveAnimations = m_saveAnimations->isChecked();
   opts->m_animations.clear();

   unsigned count = m_animList->count();
   for ( unsigned t = 0; t < count; t++ )
   {
      if ( m_animList->item( t )->isSelected() )
      {
         opts->m_animations.push_back( t );
      }
   }
}

void IqePrompt::helpNowEvent()
{
   HelpWin * win = new HelpWin( "olh_iqeprompt.html", true );
   win->show();
}

bool iqeprompt_show( Model * model, const char * const filename, ModelFilter::Options * o )
{
   bool rval = false;
   IqePrompt p;

   IqeFilter::IqeOptions * opts = dynamic_cast< IqeFilter::IqeOptions * >( o );
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
