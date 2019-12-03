/*  MM3D Misfit/Maverick Model 3D
 *
 * Copyright (c)2005 Johannes Kroll
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License,or
 * (at your option)any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not,write to the Free Software
 * Foundation,Inc.,59 Temple Place-Suite 330,Boston,MA 02111-1307,
 * USA.
 *
 * See the COPYING file for full license text.
 */

#include "mm3dtypes.h" //PCH

// Integrated into MM3D by Kevin Worcester
// This code is modified from the snap_together plugin written by Johannes Kroll

#include "menuconf.h"
#include "cmdmgr.h"
#include "model.h"
#include "pluginapi.h"
#include "version.h"
#include "log.h"
#include "weld.h"
#include "command.h"

struct SnapCommand : Command
{
	SnapCommand():Command(/*4*/2,GEOM_VERTICES_MENU){}

	virtual const char *getName(int arg)
	{
		switch(arg)
		{
		default: assert(0);
		//NOTE: There was a 3rd submenu here.
		//https://github.com/zturtleman/mm3d/issues/57
		//case 0: return "Snap Vertices Together";
		case 0: return TRANSLATE_NOOP("Command","Snap All Selected");
		case 1: return TRANSLATE_NOOP("Command","Snap All and Weld");
		//REMOVE ME
		//2019: Too chaotic to be useful as is.
//		case 2: return TRANSLATE_NOOP("Command","Snap Nearest Selected");
//		case 3: return TRANSLATE_NOOP("Command","Snap Nearest and Weld");
		}
	}

	virtual const char *getKeymap(int arg)
	{
		switch(arg)
		{
		case 0: return "Ctrl+Shift+V";
		case 1: return "Ctrl+Shift+W";
		}
		return "";
	}

	virtual bool activated(int, Model *model);
};

extern Command *snapcmd(){ return new SnapCommand; }

static void snap_together(Model *model, int_list &selection)
{
	double coord[3]= { 0,0,0 },coord_temp[3];
	int_list::iterator iter;

	for(iter= selection.begin(); iter!=selection.end(); iter++)
	{
		model->getVertexCoords(*iter,coord_temp);
		coord[0]+= coord_temp[0];
		coord[1]+= coord_temp[1];
		coord[2]+= coord_temp[2];
	}

	coord[0]/= selection.size();
	coord[1]/= selection.size();
	coord[2]/= selection.size();

	for(iter= selection.begin(); iter!=selection.end(); iter++)
	{
		model->moveVertex(*iter,coord[0],coord[1],coord[2]);
	}
}

static void snap_together_two(Model *model, int v0, int v1)
{
	double coord_v0[3],coord_v1[3];
	model->getVertexCoords(v0,coord_v0);
	model->getVertexCoords(v1,coord_v1);
	coord_v0[0]= (coord_v0[0]+coord_v1[0])/2;
	coord_v0[1]= (coord_v0[1]+coord_v1[1])/2;
	coord_v0[2]= (coord_v0[2]+coord_v1[2])/2;
	model->moveVertex(v0,coord_v0[0],coord_v0[1],coord_v0[2]);
	model->moveVertex(v1,coord_v0[0],coord_v0[1],coord_v0[2]);
}

// find the nearest vertex in the selection which is not contained in the exclude (already processed) list.
static int find_nearest_vertex(Model *model, int vertex, int_list &selection, int_list &exclude)
{
	double smallest_distance = DBL_MAX;
	int nearest_vertex= -1;
	double coord[3],coord_temp[3];
	int_list::iterator iter;

	model->getVertexCoords(vertex,coord);

	for(iter= selection.begin(); iter!=selection.end(); iter++)
	if(*iter!=vertex)
	{
		int_list::iterator it= exclude.begin();
		while(it!=exclude.end()&&*it!=*iter)it++;
		if(*it!=*iter)
		{
			model->getVertexCoords(*iter,coord_temp);
			double d = distance(coord_temp[0],coord_temp[1],coord_temp[2],coord[0],coord[1],coord[2]);
			if(d<smallest_distance)
			{
				smallest_distance = d;
				nearest_vertex= *iter;
			}
		}
	}
	return nearest_vertex;
}

bool SnapCommand::activated(int arg, Model *model)
{
	int_list selection;
	model->getSelectedVertices(selection);
	if(selection.size()<2)
	return false;
	
	int_list excludelist;
	int_list::iterator sel_iter;

	//I can't see what this does exactly... it seems to snap pairs together.
	//But it doesn't have any kind of miniumum threshold, so it seems crazy.
	/*if(arg>1)
	for(sel_iter = selection.begin(); sel_iter!=selection.end(); sel_iter++)
	{
		int_list::iterator it = excludelist.begin();
		while(it!=excludelist.end()&&*it!=*sel_iter)
		{
			it++;
		}

		if(*it!=*sel_iter)
		{
			int nearest_vertex = find_nearest_vertex(model,*sel_iter,selection,excludelist);
			if(nearest_vertex!=-1)
			{
				snap_together_two(model,*sel_iter,nearest_vertex);
				excludelist.push_back(*sel_iter);
				excludelist.push_back(nearest_vertex);
			}
		}
	}
	else*/ snap_together(model,selection);

	if(arg==1/*||arg==3*/) weldSelectedVertices(model);

	model->deleteOrphanedVertices(); return true;
}