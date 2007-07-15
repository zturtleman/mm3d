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


#include "stdcmds.h"
#include "config.h"
#include "cmdmgr.h"
#include "hidecmd.h"
#include "deletecmd.h"
#include "dupcmd.h"
#include "capcmd.h"
#include "copycmd.h"
#include "pastecmd.h"
#include "edgedivcmd.h"
#include "edgeturncmd.h"
#include "extrudecmd.h"
#include "faceoutcmd.h"
#include "flipcmd.h"
#include "flattencmd.h"
#include "selectfreecmd.h"
#include "invertcmd.h"
#include "invnormalcmd.h"
//#include "jointcmd.h"
#include "pointcmd.h"
//#include "assignjointcmd.h"
#include "subdividecmd.h"
#include "makefacecmd.h"
#include "rotatetexcmd.h"
#include "weldcmd.h"
#include "unweldcmd.h"
#include "snapcmd.h"
#include "simplifycmd.h"
#include "aligncmd.h"
#include "spherifycmd.h"
#include "log.h"
#include "cmdmgr.h"

int init_std_cmds( CommandManager * cmdMgr )
{
   log_debug( "initializing standard commands\n" );

   Command * cmd;

   cmd = new CopyCommand();
   cmdMgr->registerCommand( cmd );

   cmd = new PasteCommand();
   cmdMgr->registerCommand( cmd );

   // ----------------------

   cmd = new SeparatorCommand();
   cmdMgr->registerCommand( cmd );

   // Vertices

   cmd = new WeldCommand();
   cmdMgr->registerCommand( cmd );

   cmd = new UnweldCommand();
   cmdMgr->registerCommand( cmd );

   cmd = new SnapCommand();
   cmdMgr->registerCommand( cmd );

   cmd = new MakeFaceCommand();
   cmdMgr->registerCommand( cmd );

   cmd = new SelectFreeCommand();
   cmdMgr->registerCommand( cmd );

   // Faces

   cmd = new EdgeTurnCommand();
   cmdMgr->registerCommand( cmd );

   cmd = new EdgeDivideCommand();
   cmdMgr->registerCommand( cmd );

   cmd = new SubdivideCommand();
   cmdMgr->registerCommand( cmd );

   cmd = new RotateTextureCommand();
   cmdMgr->registerCommand( cmd );

#ifdef HAVE_QT4
   cmd = new SeparatorCommand();
   cmdMgr->registerCommand( cmd );
#endif // HAVE_QT4

   // Meshes

   cmd = new AlignCommand();
   cmdMgr->registerCommand( cmd );

   cmd = new SimplifyMeshCommand();
   cmdMgr->registerCommand( cmd );

   cmd = new CapCommand();
   cmdMgr->registerCommand( cmd );

   cmd = new SpherifyCommand();
   cmdMgr->registerCommand( cmd );

   // Normals

   cmd = new InvertNormalCommand();
   cmdMgr->registerCommand( cmd );

   cmd = new FaceOutCommand();
   cmdMgr->registerCommand( cmd );

   // ----------------------

   cmd = new SeparatorCommand();
   cmdMgr->registerCommand( cmd );

   // Other Geometry Commands

   cmd = new HideCommand();
   cmdMgr->registerCommand( cmd );

   cmd = new DeleteCommand();
   cmdMgr->registerCommand( cmd );

   cmd = new FlipCommand();
   cmdMgr->registerCommand( cmd );

   cmd = new FlattenCommand();
   cmdMgr->registerCommand( cmd );

   cmd = new DuplicateCommand();
   cmdMgr->registerCommand( cmd );

   cmd = new ExtrudeCommand();
   cmdMgr->registerCommand( cmd );

   cmd = new InvertSelectionCommand();
   cmdMgr->registerCommand( cmd );

   // ----------------------

   cmd = new PointCommand();
   cmdMgr->registerCommand( cmd );

   return 0;
}

