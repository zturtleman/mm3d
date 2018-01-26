/*  Misfit Model 3D
 * 
 *  Copyright (c) 2009 Kevin Worcester
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


#include "groupclean.h"

#include "model.h"
#include "log.h"
#include "modelstatus.h"
#include "decalmgr.h"
#include "helpwin.h"

#include <QtWidgets/QLineEdit>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QShortcut>

#include <stdlib.h>


using std::list;
using std::map;

GroupCleanWin::GroupCleanWin( Model * model, QWidget * parent )
   : QDialog( parent ),
     m_model( model )
{
   setAttribute( Qt::WA_DeleteOnClose );
   setModal( true );
   setupUi( this );

   QShortcut * help = new QShortcut( QKeySequence( tr("F1", "Help Shortcut")), this );
   connect( help, SIGNAL(activated()), this, SLOT(helpNowEvent()) );
}

GroupCleanWin::~GroupCleanWin()
{
}

void GroupCleanWin::helpNowEvent()
{
   HelpWin * win = new HelpWin( "olh_groupclean.html", true );
   win->show();
}

void GroupCleanWin::accept()
{
   if ( m_mergeMaterials->isChecked() )
   {
      m_model->mergeIdenticalMaterials();
   }
   if ( m_removeMaterials->isChecked() )
   {
      m_model->removeUnusedMaterials();
   }
   if ( m_mergeGroups->isChecked() )
      m_model->mergeIdenticalGroups();
   {
   }
   if ( m_removeGroups->isChecked() )
   {
      m_model->removeUnusedGroups();
   }
   m_model->operationComplete( tr( "Group Clean-up", "operation complete" ).toUtf8() );
   QDialog::accept();
}

void GroupCleanWin::reject()
{
   m_model->undoCurrent();
   DecalManager::getInstance()->modelUpdated( m_model );
   QDialog::reject();
}

