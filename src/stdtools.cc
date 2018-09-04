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


#include "stdtools.h"
#include "config.h"

// Tools to include in build
#include "scaletool.h"
#include "sheartool.h"
#include "selectvertextool.h"
#include "selectfacetool.h"
#include "selectconnectedtool.h"
#include "selectgrouptool.h"
#include "selectbonetool.h"
#include "selectpointtool.h"
#include "selectprojtool.h"
#include "movetool.h"
#include "dragvertextool.h"
#include "rotatetool.h"
#include "atrneartool.h"
#include "atrfartool.h"
#include "vertextool.h"
#include "rectangletool.h"
#include "jointtool.h"
#include "pointtool.h"
#include "projtool.h"
#include "cubetool.h"
#include "ellipsetool.h"
#include "extrudetool.h"
#include "cylindertool.h"
#include "torustool.h"
#include "polytool.h"
#include "bgscaletool.h"
#include "bgmovetool.h"

#include "toolbox.h"
#include "log.h"


static void _new_std_tools( Toolbox * toolbox )
{
   log_debug( "initializing standard tools\n" );

   ::Tool * tool;

   // These are cleaned up by the toolbox

   tool = new SelectVertexTool();
   toolbox->registerTool( tool );

   tool = new SelectFaceTool();
   toolbox->registerTool( tool );

   tool = new SelectConnectedTool();
   toolbox->registerTool( tool );

   tool = new SelectGroupTool();
   toolbox->registerTool( tool );

   tool = new SelectBoneTool();
   toolbox->registerTool( tool );

   tool = new SelectPointTool();
   toolbox->registerTool( tool );

   tool = new SelectProjectionTool();
   toolbox->registerTool( tool );

   tool = new ToolSeparator();
   toolbox->registerTool( tool );

   tool = new MoveTool();
   toolbox->registerTool( tool );

   tool = new RotateTool();
   toolbox->registerTool( tool );

   tool = new ScaleTool();
   toolbox->registerTool( tool );

   tool = new ShearTool();
   toolbox->registerTool( tool );

   tool = new ExtrudeTool();
   toolbox->registerTool( tool );

   tool = new DragVertexTool();
   toolbox->registerTool( tool );

   tool = new ToolSeparator();
   toolbox->registerTool( tool );

   tool = new AttractNearTool();
   toolbox->registerTool( tool );

   tool = new AttractFarTool();
   toolbox->registerTool( tool );

   tool = new BgMoveTool();
   toolbox->registerTool( tool );

   tool = new BgScaleTool();
   toolbox->registerTool( tool );

   tool = new ToolSeparator();
   toolbox->registerTool( tool );

   tool = new VertexTool();
   toolbox->registerTool( tool );

   tool = new CubeTool();
   toolbox->registerTool( tool );

   tool = new EllipsoidTool();
   toolbox->registerTool( tool );

   tool = new CylinderTool();
   toolbox->registerTool( tool );

   tool = new TorusTool();
   toolbox->registerTool( tool );

   tool = new PolyTool();
   toolbox->registerTool( tool );

   tool = new RectangleTool();
   toolbox->registerTool( tool );

   tool = new JointTool();
   toolbox->registerTool( tool );

   tool = new PointTool();
   toolbox->registerTool( tool );

   tool = new ProjectionTool();
   toolbox->registerTool( tool );

}

int init_std_tools()
{
   Toolbox::registerToolFunction( _new_std_tools );
   return 0;
}
