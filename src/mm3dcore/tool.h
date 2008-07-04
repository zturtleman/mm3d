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


#ifndef __TOOL_H
#define __TOOL_H

#include "glmath.h"
#include "model.h"

#include <vector>
#include <stdlib.h> // for NULL

class Primitive;
class BoundingBox;
class Decal;
class QMainWindow;

// The tool interface allows a Tool::Parent to notify a Tool that it is currently
// being used on the Model.
//
// The Tool receive mouses events such as a press, release, or drag (moving 
// mouse with button down).  The button state indicates which buttons
// are down, as well as which keyboard modifiers are in effect.
//
// Note that commands (non-interactive model operations) are much easier to
// implement.  There are also many similarities in how they are used by 
// Misfit Model.  You should see the documentation in command.h before 
// reading this.
//
// Note that middle mouse button events are used by ModelViewport and
// never passed to the Tool.  The middle mouse button in the button
// state is provided in the unlikely event that this behavior changes.
//

class Tool
{
   public:
      class Parent 
      {
         public:
            enum _ViewDirection_e
            {
               ViewPerspective = 0,
               ViewFront,
               ViewBack,
               ViewLeft,
               ViewRight,
               ViewTop,
               ViewBottom,
               ViewOrtho,
            };
            typedef enum _ViewDirection_e ViewDirectionE;

            virtual ~Parent() {};

            // Get the model that the parent is viewing.  This function 
            // should be called once for every event that requires a 
            // reference to the model.
            virtual Model * getModel() = 0;

            virtual ViewDirectionE getViewDirection() = 0;

            // Call this to force an update on the current model view
            virtual void updateView() = 0;

            // Call this to force an update on 3d views of the current model
            virtual void update3dView() = 0;

            // Call this to force an update on all model views
            virtual void updateAllViews() = 0;

            // The getParentXYValue function returns the mouse coordinates
            // in viewport space (as opposed to model space), X is left and
            // right, Y is up and down, and Z is depth (undefined) regardless
            // of the orientation of the viewport.  Use this function instead
            // of get[XYZ]Value whenever possible.
            // 
            // The value of "selected" indicates if the snap should include selected
            // vertices or not (generally you would use "true" if you want to start
            // a manipulation operation on selected vertices, for update or all
            // other start operations you would set this value to false).
            //
            virtual void getParentXYValue( int x, int y, double & xval, double & yval, bool selected = false ) = 0;

            // The getRawParentXYValue function returns the mouse coordinates
            // in viewport space (as opposed to model space), X is left and
            // right, Y is up and down, and Z is depth (undefined) regardless
            // of the orientation of the viewport.  Use this function instead
            // of get[XYZ]Value whenever possible.
            // 
            // This function ignores snap to vertex/grid settings.
            //
            virtual void getRawParentXYValue( int x, int y, double & xval, double & yval ) = 0;

            // The getParentViewMatrix function returns the Matrix
            // that is applied to the model to produce the parent's
            // viewport.
            virtual const Matrix & getParentViewMatrix() const = 0;

            virtual const Matrix & getParentViewInverseMatrix() const = 0;

            // Get the 3d coordinate that corresponds to an x,y mouse event
            // value (in model space).
            //
            // As the views are 2d and the model is 3d, one of the
            // three coordinates will be undefined.
            // These return false if parent or val is NULL, or the point
            // on the axis in question is undefined for the given view.
            // For example, if the x,y point was provided by the front or back
            // view, the z (depth) component of the coordinate is undefined.
            //
            // In the case of an undefined coordinate, the tool is responsible
            // for providing an appropriate value.  For example, a sphere tool
            // would use the same radius for all three axis, and might center
            // the undefined coordinate on zero (0).
            virtual bool getXValue( int x, int y, double * val ) = 0;
            virtual bool getYValue( int x, int y, double * val ) = 0;
            virtual bool getZValue( int x, int y, double * val ) = 0;

            // The addDecal and removeDecal methods are not called directly
            // by tools.  You must use the DecalManager::addDecalToParent
            // function instead.
            //
            // A decal is an object that is not part of a model, but drawn
            // over it to indicate some information to the user.  The rotate
            // tool uses decals to display the center point of rotation.
            // There is no documentation on decals, but you can see the
            // RotateTool, RotatePoint, and DecalManager classes for an example 
            // of decal usage.
            virtual void addDecal( Decal * decal ) = 0;
            virtual void removeDecal( Decal * decal ) = 0;
      };

      enum ButtonState
      {
         BS_Left       = 0x001,
         BS_Middle     = 0x002, // NOTE: You will never receive a middle button event
         BS_Right      = 0x004,
         BS_Shift      = 0x008,
         BS_Alt        = 0x010,
         BS_Ctrl       = 0x020,
         BS_CapsLock   = 0x040,
         BS_NumLock    = 0x080,
         BS_ScrollLock = 0x100
      };

      Tool();
      virtual ~Tool();

      // It is a good idea to override this if you implement
      // a tool as a plugin.
      virtual void release() { delete this; };

      // These functions are used in a manner similar to commands.  
      // See command.h for details.
      virtual int getToolCount() = 0;
      virtual const char * getName( int arg ) = 0;
      virtual const char * getPath() { return ""; }
      virtual void activated( int arg, Model * model, QMainWindow * mainwin ) {}
      virtual void deactivated() {}
      virtual void setModel( Model * m ) {}

      // Is this a place-holder for a menu separator?
      virtual bool isSeparator()     { return false; };

      // Does this tool create new primitives?
      virtual bool isCreation()     { return false; };

      // Does this tool manipulate existing (selected primitives)?
      virtual bool isManipulation() { return false; };

      // Like commands in command.h
      virtual bool getKeyBinding( int arg, int & keyBinding) { return false; };

      // These functions indicate that a mouse event has occured while
      // the Tool is active.
      //
      // You will not get mouse events for the middle button.
      // Have I made myself clear?
      virtual void mouseButtonDown( Parent * parent, int buttonState, int x, int y ) = 0;
      virtual void mouseButtonUp(   Parent * parent, int buttonState, int x, int y ) = 0;
      virtual void mouseButtonMove( Parent * parent, int buttonState, int x, int y ) = 0;

      // This returns the pixmap that appears in the toolbar
      virtual const char ** getPixmap() = 0;

      static int s_allocated;

      struct _ToolCoord_t
      {
          Model::Position pos;
          double newCoords[3];
          double oldCoords[3];
          double dist;
      };
      typedef struct _ToolCoord_t ToolCoordT;
      typedef std::vector< ToolCoordT > ToolCoordList;

   protected:
      // These methods provide functionality that many tools use

      // These functions act like addVertex and addPoint except that they
      // work in the viewport space instead of the model space, use these
      // instead of addVertex/addPoint whenever possible
      ToolCoordT addPosition( Parent * parent,
              Model::PositionTypeE type, const char * name,
              double x, double y, double z,
              double xrot = 0.0, double yrot = 0.0, double zrot = 0.0, int boneId = -1 );
      void movePosition( Parent * parent,
              const Model::Position & pos,
              double x, double y, double z );

      void makeToolCoordList( Parent * parent, ToolCoordList & list, 
              const list< Model::Position > & positions );
};

class ToolSeparator : public Tool
{
   public:
      int getToolCount() { return 1; };
      const char * getName( int arg ) { return ""; };
      bool isSeparator()     { return true; };

      void mouseButtonDown( Parent * parent, int buttonState, int x, int y ) {};
      void mouseButtonUp(   Parent * parent, int buttonState, int x, int y ) {};
      void mouseButtonMove( Parent * parent, int buttonState, int x, int y ) {};
      const char ** getPixmap() { return NULL; };
};

#endif // __TOOL_H
