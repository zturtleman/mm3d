/*  Maverick Model 3D
 * 
 *  Copyright (c) 2007-2008 Kevin Worcester
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


// This file tests selection methods in the Model class.

#include <QtTest/QtTest>

#include "test_common.h"

#include "model.h"
#include "texture.h"
#include "modelstatus.h"
#include "log.h"
#include "mm3dfilter.h"

#include "local_array.h"
#include "local_ptr.h"
#include "release_ptr.h"


class ModelSelectTest : public QObject
{
   Q_OBJECT
private:

private slots:

   void initTestCase()
   {
      log_enable_debug( false );
   }

   // FIXME add some real tests

   void testConnectedWithHidden()
   {
      local_ptr<Model> m = loadModelOrDie( "data/model_hidden_test.mm3d" );

      m->setSelectionMode( Model::SelectConnected );

      Matrix mat;
      mat.loadIdentity();

      m->selectInVolumeMatrix( mat, -1.0, -1.0, 1.0, 1.0 );
      QVERIFY_EQ( 36, (int) m->getSelectedTriangleCount() );
   }

};

QTEST_MAIN(ModelSelectTest)
#include "model_select_test.moc"

