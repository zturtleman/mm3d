/*  MM3D Misfit/Maverick Model 3D
 *
 * Copyright (c)2004-2007 Kevin Worcester
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

#include "model.h"
#include "log.h"
#include "msg.h"
#include "modelstatus.h"
#include "command.h"

struct DuplicateCommand : Command
{
	virtual const char *getName(int)
	{
		return TRANSLATE_NOOP("Command","Duplicate"); 
	}

	//NOTE: Delete is Ctrl+Shift+D. Ctrl+F is avoided to 
	//not accidentally hit D. Ctrl+S will save the model.
	virtual const char *getKeymap(int){ return "Ctrl+D"; }

	virtual bool activated(int, Model *model);
};

extern Command *dupcmd(){ return new DuplicateCommand; }

bool DuplicateCommand::activated(int arg, Model *model)
{
	int_list tri;
	model->getSelectedTriangles(tri);
	int_list joints;
	model->getSelectedBoneJoints(joints);
	int_list points;
	model->getSelectedPoints(points);

	int_list vert;

	std::unordered_map<int,int> vertMap;
	std::unordered_map<int,int> triMap;
	std::unordered_map<int,int> jointMap;
	std::unordered_map<int,int> pointMap;

	int_list::iterator lit;

	if(!tri.empty())
	{
		model_status(model,StatusNormal,STATUSTIME_SHORT,
		TRANSLATE("Command","Selected primitives duplicated"));

		model->getSelectedVertices(vert);

		// Duplicated vertices
		log_debug("Duplicating %d vertices\n",vert.size());
		for(lit = vert.begin(); lit!=vert.end(); lit++)
		{
			double coords[3];
			model->getVertexCoords(*lit,coords);
			int nv = model->addVertex(coords[0],coords[1],coords[2]);

			if(model->isVertexFree(*lit))
			{
				model->setVertexFree(nv,true);
			}

			vertMap[*lit] = nv;
		}

		// Duplicate faces
		log_debug("Duplicating %d faces\n",tri.size());
		for(lit = tri.begin(); lit!=tri.end(); lit++)
		{
			unsigned v[3];

			for(int t = 0; t<3; t++)
			{
				v[t] = model->getTriangleVertex(*lit,t);
			}
			int nt = model->addTriangle(vertMap[v[0]] ,vertMap[v[1]],vertMap[v[2]]);

			triMap[*lit] = nt;
		}

		// Duplicate texture coords
		log_debug("Duplicating %d face texture coordinates\n",tri.size());
		for(lit = tri.begin(); lit!=tri.end(); lit++)
		{
			float s;
			float t;

			for(unsigned i = 0; i<3; i++)
			{
				model->getTextureCoords((unsigned)*lit,i,s,t);
				model->setTextureCoords((unsigned)triMap[*lit],i,s,t);
			}
		}

		if(model->getGroupCount())
		{
			// Set groups
			log_debug("Setting %d triangle groups\n",tri.size());
			for(lit = tri.begin(); lit!=tri.end(); lit++)
			{
				//FIX ME: getTriangleGroup is dumb!!
				//FIX ME: getTriangleGroup is dumb!!
				//FIX ME: getTriangleGroup is dumb!!
				//FIX ME: getTriangleGroup is dumb!!

				// This works,even if triangle group==-1
				int gid = model->getTriangleGroup(*lit);
				if(gid>=0)
				{
					model->addTriangleToGroup(gid,triMap[*lit]);
				}
			}
		}

	}

	if(!joints.empty())
	{

		// Duplicated joints
		log_debug("Duplicating %d joints\n",joints.size());
		for(lit = joints.begin(); lit!=joints.end(); lit++)
		{
			int parent = model->getBoneJointParent(*lit);

			// TODO this will not work if parent joint comes after child
			// joint.  That shouldn't happen... but...
			if(model->isBoneJointSelected(parent))
			{
				parent = jointMap[parent];
			}

			// If joint is root joint,assign duplicated joint to be child
			// of original
			if(parent==-1)
			{
				parent = 0;
			}

			double coord[3];
			double rot[3] = { 0,0,0 };
			model->getBoneJointCoords(*lit,coord);

			int nj = model->addBoneJoint(model->getBoneJointName(*lit),
						coord[0],coord[1],coord[2],rot[0],rot[1],rot[2],parent);
			jointMap[*lit] = nj;

			// Assign duplicated vertices to duplicated bone joints
			int_list vertlist = model->getBoneJointVertices(*lit);
			int_list::iterator vit;
			for(vit = vertlist.begin(); vit!=vertlist.end(); vit++)
			{
				if(model->isVertexSelected(*vit))
				{
					model->setVertexBoneJoint(vertMap[*vit],nj);
				}
			}
		}
	}

	if(!points.empty())
	{
		// Duplicated points
		log_debug("Duplicating %d points\n",points.size());
		for(lit = points.begin(); lit!=points.end(); lit++)
		{
			int parent = model->getPointBoneJoint(*lit);

			if(model->isBoneJointSelected(parent))
			{
				parent = jointMap[parent];
			}

			double coord[3];
			double rot[3] = { 0,0,0 };
			model->getPointCoords(*lit,coord);
			model->getPointRotation(*lit,rot);

			int np = model->addPoint(model->getPointName(*lit),
						coord[0],coord[1],coord[2],rot[0],rot[1],rot[2],parent);
			pointMap[*lit] = np;
		}
	}

	model->unselectAll();
	// Select vertices
	log_debug("reselecting vertices\n");
	for(lit = vert.begin(); lit!=vert.end(); lit++)
	{
		model->selectVertex(vertMap[*lit]);
	}
	// Select faces
	log_debug("reselecting faces\n");
	for(lit = tri.begin(); lit!=tri.end(); lit++)
	{
		model->selectTriangle(triMap[*lit]);
	}
	// Select bone joints
	log_debug("reselecting bone joints\n");
	for(lit = joints.begin(); lit!=joints.end(); lit++)
	{
		model->selectBoneJoint(jointMap[*lit]);
	}
	// Select points
	log_debug("reselecting points\n");
	for(lit = points.begin(); lit!=points.end(); lit++)
	{
		model->selectPoint(pointMap[*lit]);
	}

	model->invalidateNormals();

	if(joints.empty()&&tri.empty()&&points.empty())
	{
		model_status(model,StatusError,STATUSTIME_LONG,TRANSLATE("Command","You must have at least 1 face,joint,or point selected to Duplicate"));
		return false;
	}
	else
	{
		model_status(model,StatusNormal,STATUSTIME_SHORT,TRANSLATE("Command","Duplicate complete"));
	}

	return true;
}

