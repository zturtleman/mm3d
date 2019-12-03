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

#include "tool.h"

#include "log.h"

//int Tool::s_allocated = 0; //toolbox.cc

Tool::Tool(ToolType tt, int count, const char *path)
	:
parent(),m_type(tt),m_args(count),m_path(path)
{
	s_allocated++;
}
Tool::~Tool(){ s_allocated--; }

Tool::ToolCoordT Tool::addPosition
(Model::PositionTypeE type, double x, double y, double z)
{
	return addPosition(type,nullptr,x,y,z);
}
Tool::ToolCoordT Tool::addPosition
(Model::PositionTypeE type, const char *name, 
double x, double y, double z, 
double xrot, double yrot, double zrot, int boneId)
{
	Model::Position pos;
	pos.type  = type;
	pos.index = ~0;

	//Matrix m = parent->getParentViewInverseMatrix();
	const Matrix &m = parent->getParentViewInverseMatrix();

	double tranVec[4] = { x,y,z,1.0 };

	m.apply(tranVec);

	double rotVec[3] = { xrot,yrot,zrot };
	Matrix rot;
	rot.setRotation(rotVec);
	rot = rot *m;
	rot.getRotation(rotVec);

	Model *model = parent->getModel();

	switch (type)
	{
	case Model::PT_Vertex:
		pos.index = model->addVertex(tranVec[0],tranVec[1],tranVec[2]);
		break;
	case Model::PT_Joint:
		pos.index = model->addBoneJoint(name,tranVec[0],tranVec[1],tranVec[2],
					rotVec[0],rotVec[1],rotVec[2],boneId);
		break;
	case Model::PT_Point:
		pos.index = model->addPoint(name,tranVec[0],tranVec[1],tranVec[2],
					rotVec[0],rotVec[1],rotVec[2],boneId);
		break;
	case Model::PT_Projection:
		pos.index = model->addProjection(name,Model::TPT_Cylinder,tranVec[0],tranVec[1],tranVec[2]);
		break;
	default:
		log_error("don't know how to add a point of type %d\n",
					static_cast<int>(type));
		break;
	}

	ToolCoordT tc;

	tc.pos = pos;
	tc.coords[0] = x;
	tc.coords[1] = y;
	tc.coords[2] = z;

	return tc;
}

void Tool::movePosition
(const Model::Position &pos,double x, double y, double z)
{
	//Matrix m = parent->getParentViewInverseMatrix();
	const Matrix &m = parent->getParentViewInverseMatrix();

	double tranVec[4] = { x,y,z,1.0 };

	/*
	log_debug("orig position is %f %f %f\n",
			(float)tranVec[0],
			(float)tranVec[1],
			(float)tranVec[2]);
			*/

	m.apply3(tranVec);
	tranVec[0] += m.get(3,0);
	tranVec[1] += m.get(3,1);
	tranVec[2] += m.get(3,2);

	/*
	log_debug("tran position %f,%f,%f\n",
			(float)tranVec[0],
			(float)tranVec[1],
			(float)tranVec[2]);
			*/

	parent->getModel()->movePosition(pos,
			tranVec[0],tranVec[1],tranVec[2]);
}

void Tool::makeToolCoordList
(ToolCoordList &list, const pos_list &positions)
{
	Model *model = parent->getModel();
	const Matrix &mat = parent->getParentViewMatrix();

	ToolCoordT tc;
	pos_list::const_iterator it;
	for(it = positions.begin(); it!=positions.end(); it++)
	{
		tc.pos = (*it);
		model->getPositionCoords(tc.pos,tc.coords);

		mat.apply3(tc.coords);
		tc.coords[0] += mat.get(3,0);
		tc.coords[1] += mat.get(3,1);
		tc.coords[2] += mat.get(3,2);

		list.push_back(tc);
	}
}

