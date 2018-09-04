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


// This file tests the primary functionality of libmm3d/model.h
// Some features of the Model class are tested by other test files.

// TODO General:
//   * Observer add/remove/receive changes
//   * Error code to failure mapping
//   * Push/pop errors
//   * diff (equivalent)
//   * get/set filename
//   * get/set export file
//   * get/set filter-specific error
//   * drawing (right...)
//   * texture/context loading/removing, deleteGLTextures
//   * Invalidate textures
//   * Texture data compare fuzziness
//   * add/delete/get format data
//   * background images get/set image/scale/center
//   * meta data get(by name/index)/set/removeLast(internal)/clear
//   * merge models
//   * merge animations
//   * boolean operations
//   * render options (joint mode, projections, canvas draw mode)
//
// TODO Geometry:
//   * Deletions
//     - Delete selected
//     - Force add/delete
//     x Remove oprhaned vertices (and not free vertices)
//     x Remove flattened triangles
//   * Set/get primitive properties
//     x Vertices
//     x Faces
//     x Groups
//     x Materials
//     - Bone Joints
//     - Points
//     - Texture Projections (seam/up/range/scale/type/rotation(misnamed arg))
//     - Moving
//   - Hiding
//     x Hide selected
//     - Hide unselected
//     x Unhide all
//   * Selection
//      - Volume selection
//        - Selection mode
//        - Interaction with visibility
//        - Selection test
//      - Invert selection
//      - Selection difference
//      - Joint parent selection
//      - Select primitives from other primitives
//      - getSelected{PRIMITIVE} list
//   * Bounding region
//   * Transforms
//     - Translate
//     - Rotate
//     - Apply Matrix (w/undoable or not)
//   * Subdivide/unsubdivide
//   * Simplify mesh
//   * Normals/cosToPoint
//   * Anim Normals
//   * BSP Tree generation
//   * Grouping and group properties
//   * Skeletal structure
//   * Bone joint matrices
//   * Bone joint assignment
//   * Bone joint weighting calculations
//   * Bone primary influence calculations
//   * Auto assign bone joint
//   * getBoneVector
//   * Position accessors
//   * Texture coordinates
//   * Projection mapping
//   * Local matrix
//   
// TODO Undo:
//   * Undo works on everything
//   * Redo works on everything
//   x Undo current
//   * Enable/disable works
//   x Size calculation works
//   x Limits work
//   x Atomic operation names
//   x canUndo/canRedo
//   * setUndoEnabled
//   x Save interaction (getSaved/setSaved)
//   x undoRelease/redoRelease
//
// TODO Animation:
//   * Add/remove animation
//   * Properties (name, fps, etc.)
//   * Add/remove animation frames
//   * Copy/join/split/merge/convert
//   * Move animation
//   * Setting animation time/frame/none
//   * Looping
//   * Set/get animation frame data
//     - Keyframe
//     - Mesh deformation
//     - Clear
//   * Interpolation
//
#include <QtTest/QtTest>

#include "test_common.h"

#include "model.h"
#include "texture.h"
#include "local_array.h"

class ModelTest : public QObject
{
   Q_OBJECT
private:

private slots:

   void testSomething()
   {
   }
};

QTEST_MAIN(ModelTest)
#include "model_test.moc"
