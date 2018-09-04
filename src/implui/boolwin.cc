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


#include "boolwin.h"

#include "model.h"
#include "log.h"
#include "modelstatus.h"
#include "decalmgr.h"
#include "helpwin.h"
#include "viewpanel.h"
#include "modelstatus.h"

#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>
#include <QtWidgets/QRadioButton>

#include <stdlib.h>

#include <list>

using std::list;

BoolWin::BoolWin( Model * model, ViewPanel * panel, QWidget * parent )
   : QWidget( parent ),
     m_model( model ),
     m_panel( panel ),
     m_operation( Model::BO_Union )
{
   setupUi( this );

   m_unionButton->setChecked( true );
   updateOperationButton();
   clearObject();
}

BoolWin::~BoolWin()
{
}

void BoolWin::setModel( Model * m )
{
   m_model = m;
   clearObject();
}

void BoolWin::operationChangedEvent( bool o)
{
   if ( o )
   {
      if ( m_fuseButton->isChecked() )
      {
         m_operation = Model::BO_Union;
      }
      else if ( m_unionButton->isChecked() )
      {
         m_operation = Model::BO_UnionRemove;
      }
      else if ( m_subtractButton->isChecked() )
      {
         m_operation = Model::BO_Subtraction;
      }
      else if ( m_intersectButton->isChecked() )
      {
         m_operation = Model::BO_Intersection;
      }

      updateOperationButton();
   }
}

void BoolWin::doOperationEvent()
{
   std::list<int> al;
   std::list<int> bl;

   unsigned tcount = m_model->getTriangleCount();

   for ( unsigned t = 0; t < tcount; t++ )
   {
      if ( m_model->isTriangleSelected( t ) )
      {
         bl.push_back( t );
      }
      else if ( m_model->isTriangleMarked( t ) )
      {
         al.push_back( t );
      }
   }

   if ( !al.empty() && !bl.empty() )
   {
      m_model->booleanOperation( m_operation, al, bl );

      QString opStr;
      switch ( m_operation )
      {
      case Model::BO_UnionRemove:
         opStr = tr( "Union", "boolean operation" );
         break;

      case Model::BO_Subtraction:
         opStr = tr( "Subtraction", "boolean operation" );
         break;

      case Model::BO_Intersection:
         opStr = tr( "Intersection", "boolean operation" );
         break;

      case Model::BO_Union:
      default:
         opStr = tr( "Union", "boolean operation" );
         break;
      }

      m_model->operationComplete( opStr.toUtf8() );
      clearObject();
      m_panel->modelUpdatedEvent();
   }
   else
   {
      if ( bl.empty() )
      {
         model_status( m_model, StatusError, STATUSTIME_LONG, tr("You must have at least once face selected").toUtf8() );
      }
      else
      {
         model_status( m_model, StatusError, STATUSTIME_LONG, tr("Object A triangles are still selected").toUtf8() );
      }
   }
}

void BoolWin::setObjectEvent()
{
   std::list<int> tris;

   m_model->getSelectedTriangles( tris );

   if ( !tris.empty() )
   {
      m_opButton->setEnabled( true );

      QString labelStr;
      labelStr.sprintf( "Object: %d Faces", (int) tris.size() );
      m_setLabel->setText( labelStr );

      m_model->clearMarkedTriangles();
      std::list<int>::iterator it;
      for ( it = tris.begin(); it != tris.end(); it++ )
      {
         m_model->setTriangleMarked( *it, true );
      }
   }
   else
   {
      m_opButton->setEnabled( false );
   }
}

void BoolWin::updateOperationButton()
{
   switch ( m_operation )
   {
      case Model::BO_UnionRemove:
         m_opButton->setText( tr( "Union With Selected", "boolean operation" ) );
         break;

      case Model::BO_Subtraction:
         m_opButton->setText( tr( "Subtract Selected", "boolean operation" ) );
         break;

      case Model::BO_Intersection:
         m_opButton->setText( tr( "Intersect With Selected", "boolean operation" ) );
         break;

      case Model::BO_Union:
      default:
         m_opButton->setText( tr( "Fuse Selected", "boolean operation" ) );
         break;
   }
}

void BoolWin::clearObject()
{
   m_setLabel->setText( tr( "Select faces to set", "Select faces to set as 'A' Object in boolean operation" ) );
   m_opButton->setEnabled( false );
}

