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

#ifdef MM3D_EDIT
#include "modelstatus.h"
#include "undomgr.h"
#include "modelundo.h"
#endif // MM3D_EDIT

bool Model::setPositionBoneJoint(const Position &pos, int joint)
{
	removeAllPositionInfluences(pos);

	if(joint>=0)
	{
		return addPositionInfluence(pos,joint,IT_Custom,1.0);
	}
	return false;
}

bool Model::setVertexBoneJoint(unsigned vertex, int joint)
{
	removeAllVertexInfluences(vertex);

	if(joint>=0)
	{
		return addVertexInfluence(vertex,joint,IT_Custom,1.0);
	}
	return false;
}

bool Model::setPointBoneJoint(unsigned point, int joint)
{
	removeAllPointInfluences(point);

	if(joint>=0)
	{
		return addPointInfluence(point,joint,IT_Custom,1.0);
	}
	return false;
}

int_list Model::getBoneJointVertices(int joint)const
{
	int_list rval;

	if(joint>=-1&&joint<(signed)m_joints.size())
	{
		//infl_list ilist;
		infl_list::const_iterator it;

		for(unsigned v = 0; v<m_vertices.size(); v++)
		{
			//getVertexInfluences(v,ilist);
			const infl_list &ilist = getVertexInfluences(v);

			for(it = ilist.begin(); it!=ilist.end(); it++)
			{
				if((*it).m_boneId==joint)
				{
					rval.push_back(v);
					break;
				}
			}
		}
	}

	return rval;
}

bool Model::addPositionInfluence(const Position &pos, unsigned joint,InfluenceTypeE type, double weight)
{
	switch (pos.type)
	{
		case PT_Vertex:
			return addVertexInfluence(pos.index,joint,type,weight);

		case PT_Point:
			return addPointInfluence(pos.index,joint,type,weight);

		default:
			break;
	}

	return false;
}

bool Model::addVertexInfluence(unsigned vertex, unsigned joint,InfluenceTypeE type, double weight)
{
	if(vertex<m_vertices.size()&&joint<m_joints.size())
	{
		m_changeBits |= AddOther;

		Vertex *v = m_vertices[vertex];
		infl_list::iterator it;
		for(it = v->m_influences.begin(); it!=v->m_influences.end(); it++)
		{
			if((*it).m_boneId==(int)joint)
			{
				InfluenceT oldInf = (*it);

				(*it).m_weight = weight;
				(*it).m_type = type;

				InfluenceT newInf = (*it);

				MU_UpdatePositionInfluence *undo = new MU_UpdatePositionInfluence();
				Position pos;
				pos.type = PT_Vertex;
				pos.index = vertex;
				undo->updatePositionInfluence(pos,newInf,oldInf);
				sendUndo(undo);

				calculateRemainderWeight(v->m_influences);

				return true;
			}
		}

		InfluenceT inf;
		inf.m_boneId = (int)joint;
		inf.m_weight = weight;
		inf.m_type = type;

		if(v->m_influences.size()<MAX_INFLUENCES)
		{
			MU_SetPositionInfluence *undo = new MU_SetPositionInfluence();
			Position pos;
			pos.type = PT_Vertex;
			pos.index = vertex;
			undo->setPositionInfluence(true,pos,v->m_influences.size(),inf);
			sendUndo(undo);

			v->m_influences.push_back(inf);

			calculateRemainderWeight(v->m_influences);

			return true;
		}
	}

	return false;
}

bool Model::addPointInfluence(unsigned point, unsigned joint,InfluenceTypeE type, double weight)
{
	if(point<m_points.size()&&joint<m_joints.size())
	{
		m_changeBits |= AddOther;

		Point *p = m_points[point];
		infl_list::iterator it;
		for(it = p->m_influences.begin(); it!=p->m_influences.end(); it++)
		{
			if((*it).m_boneId==(int)joint)
			{
				InfluenceT oldInf = (*it);

				(*it).m_weight = weight;
				(*it).m_type = type;

				InfluenceT newInf = (*it);

				MU_UpdatePositionInfluence *undo = new MU_UpdatePositionInfluence();
				Position pos;
				pos.type = PT_Point;
				pos.index = point;
				undo->updatePositionInfluence(pos,newInf,oldInf);
				sendUndo(undo);

				calculateRemainderWeight(p->m_influences);

				return true;
			}
		}

		InfluenceT inf;
		inf.m_boneId = (int)joint;
		inf.m_weight = weight;
		inf.m_type	= type;

		if(p->m_influences.size()<MAX_INFLUENCES)
		{
			MU_SetPositionInfluence *undo = new MU_SetPositionInfluence();
			Position pos;
			pos.type = PT_Point;
			pos.index = point;
			undo->setPositionInfluence(true,pos,p->m_influences.size(),inf);
			sendUndo(undo);

			p->m_influences.push_back(inf);

			calculateRemainderWeight(p->m_influences);

			return true;
		}
	}

	return false;
}

bool Model::removePositionInfluence(const Position &pos, unsigned joint)
{
	switch (pos.type)
	{
		case PT_Vertex:
			return removeVertexInfluence(pos.index,joint);

		case PT_Point:
			return removePointInfluence(pos.index,joint);

		default:
			break;
	}

	return false;
}

bool Model::removeVertexInfluence(unsigned vertex, unsigned joint)
{
	if(vertex<m_vertices.size())
	{
		m_changeBits |= AddOther;

		Vertex *v = m_vertices[vertex];
		infl_list::iterator it;

		int count = 0;
		for(it = v->m_influences.begin(); it!=v->m_influences.end(); it++)
		{
			if((*it).m_boneId==(int)joint)
			{
				MU_SetPositionInfluence *undo = new MU_SetPositionInfluence();
				Position pos;
				pos.type = PT_Vertex;
				pos.index = vertex;
				undo->setPositionInfluence(false,pos,count,*it);
				sendUndo(undo);

				v->m_influences.erase(it);

				calculateRemainderWeight(v->m_influences);

				return true;
			}
			count++;
		}
	}

	return false;
}

bool Model::removePointInfluence(unsigned point, unsigned joint)
{
	if(point<m_points.size())
	{
		m_changeBits |= AddOther;

		Point *p = m_points[point];
		infl_list::iterator it;

		int count = 0;
		for(it = p->m_influences.begin(); it!=p->m_influences.end(); it++)
		{
			if((*it).m_boneId==(int)joint)
			{
				MU_SetPositionInfluence *undo = new MU_SetPositionInfluence();
				Position pos;
				pos.type = PT_Point;
				pos.index = point;
				undo->setPositionInfluence(false,pos,count,*it);
				sendUndo(undo);

				p->m_influences.erase(it);

				calculateRemainderWeight(p->m_influences);

				return true;
			}
			count++;
		}
	}

	return false;
}

bool Model::removeAllPositionInfluences(const Position &pos)
{
	switch (pos.type)
	{
		case PT_Vertex:
			return removeAllVertexInfluences(pos.index);

		case PT_Point:
			return removeAllPointInfluences(pos.index);

		default:
			break;
	}

	return false;
}

bool Model::removeAllVertexInfluences(unsigned vertex)
{
	if(vertex<m_vertices.size())
	{
		infl_list &l = m_vertices[vertex]->m_influences;

		while(!l.empty())
		{
			Position pos;
			pos.type = PT_Vertex;
			pos.index = vertex;
			removePositionInfluence(pos,l.back().m_boneId);
		}
		return true;
	}

	return false;
}

bool Model::removeAllPointInfluences(unsigned point)
{
	if(point<m_points.size())
	{
		infl_list &l = m_points[point]->m_influences;

		while(!l.empty())
		{
			Position pos;
			pos.type = PT_Point;
			pos.index = point;
			removePositionInfluence(pos,l.back().m_boneId);
		}
		return true;
	}

	return false;
}

bool Model::getPositionInfluences(const Position &pos,infl_list &l)const
{
	switch (pos.type)
	{
		case PT_Vertex:
			return getVertexInfluences(pos.index,l);

		case PT_Point:
			return getPointInfluences(pos.index,l);

		default:
			break;
	}

	return false;
}

bool Model::getVertexInfluences(unsigned vertex,infl_list &l)const
{
	if(vertex<m_vertices.size())
	{
		l = m_vertices[vertex]->m_influences;
		return true;
	}

	return false;
}

bool Model::getPointInfluences(unsigned point,infl_list &l)const
{
	if(point<m_points.size())
	{
		l = m_points[point]->m_influences;
		return true;
	}

	return false;
}

int Model::getPrimaryPositionInfluence(const Position &pos)const
{
	//infl_list l;
	//getPositionInfluences(pos,l);
	if(pos.type!=PT_Vertex&&pos.type!=PT_Point) 
	return -1;
	const infl_list &l = getPositionInfluences(pos);

	Model::InfluenceT inf;
	inf.m_boneId = -1;
	inf.m_weight = -1.0;

	infl_list::const_iterator it;
	for(it = l.begin(); it!=l.end(); it++)
	{
		if(inf<*it)
		{
			inf = *it;
		}
	}
	return inf.m_boneId;
}

int Model::getPrimaryVertexInfluence(unsigned vertex)const
{
	Position pos;
	pos.type = PT_Vertex;
	pos.index = vertex;

	return getPrimaryPositionInfluence(pos);
}

int Model::getPrimaryPointInfluence(unsigned point)const
{
	Position pos;
	pos.type = PT_Point;
	pos.index = point;

	return getPrimaryPositionInfluence(pos);
}

bool Model::setPositionInfluenceType(const Position &pos, unsigned int joint,InfluenceTypeE type)
{
	switch (pos.type)
	{
		case PT_Vertex:
			return setVertexInfluenceType(pos.index,joint,type);

		case PT_Point:
			return setPointInfluenceType(pos.index,joint,type);

		default:
			break;
	}

	return false;
}

bool Model::setVertexInfluenceType(unsigned vertex, unsigned int joint,InfluenceTypeE type)
{
	if(vertex<m_vertices.size()&&joint<m_joints.size())
	{
		m_changeBits |= AddOther;

		Vertex *v = m_vertices[vertex];
		infl_list::iterator it;
		for(it = v->m_influences.begin(); it!=v->m_influences.end(); it++)
		{
			if((*it).m_boneId==(int)joint)
			{
				InfluenceT oldInf = (*it);

				(*it).m_type = type;

				InfluenceT newInf = (*it);

				MU_UpdatePositionInfluence *undo = new MU_UpdatePositionInfluence();
				Position pos;
				pos.type = PT_Vertex;
				pos.index = vertex;
				undo->updatePositionInfluence(pos,newInf,oldInf);
				sendUndo(undo);

				calculateRemainderWeight(v->m_influences);

				return true;
			}
		}
	}

	return false;
}

bool Model::setPointInfluenceType(unsigned point, unsigned int joint,InfluenceTypeE type)
{
	if(point<m_points.size()&&joint<m_joints.size())
	{
		m_changeBits |= AddOther;

		Point *p = m_points[point];
		infl_list::iterator it;
		for(it = p->m_influences.begin(); it!=p->m_influences.end(); it++)
		{
			if((*it).m_boneId==(int)joint)
			{
				InfluenceT oldInf = (*it);

				(*it).m_type = type;

				InfluenceT newInf = (*it);

				MU_UpdatePositionInfluence *undo = new MU_UpdatePositionInfluence();
				Position pos;
				pos.type = PT_Point;
				pos.index = point;
				undo->updatePositionInfluence(pos,newInf,oldInf);
				sendUndo(undo);

				calculateRemainderWeight(p->m_influences);

				return true;
			}
		}
	}
	return false;
}

bool Model::setPositionInfluenceWeight(const Position &pos, unsigned int joint, double weight)
{
	switch (pos.type)
	{
		case PT_Vertex:
			return setVertexInfluenceWeight(pos.index,joint,weight);

		case PT_Point:
			return setPointInfluenceWeight(pos.index,joint,weight);

		default:
			break;
	}

	return false;
}

bool Model::setVertexInfluenceWeight(unsigned vertex, unsigned int joint, double weight)
{
	if(vertex<m_vertices.size()&&joint<m_joints.size())
	{
		m_changeBits |= AddOther;

		Vertex *v = m_vertices[vertex];
		infl_list::iterator it;
		for(it = v->m_influences.begin(); it!=v->m_influences.end(); it++)
		{
			if((*it).m_boneId==(int)joint)
			{
				InfluenceT oldInf = (*it);

				(*it).m_weight = weight;

				InfluenceT newInf = (*it);

				MU_UpdatePositionInfluence *undo = new MU_UpdatePositionInfluence();
				Position pos;
				pos.type = PT_Vertex;
				pos.index = vertex;
				undo->updatePositionInfluence(pos,newInf,oldInf);
				sendUndo(undo);

				calculateRemainderWeight(v->m_influences);

				return true;
			}
		}
	}

	return false;
}

bool Model::setPointInfluenceWeight(unsigned point, unsigned int joint, double weight)
{
	if(point<m_points.size()&&joint<m_joints.size())
	{
		m_changeBits |= AddOther;

		Point *p = m_points[point];
		infl_list::iterator it;
		for(it = p->m_influences.begin(); it!=p->m_influences.end(); it++)
		{
			if((*it).m_boneId==(int)joint)
			{
				InfluenceT oldInf = (*it);

				(*it).m_weight = weight;

				InfluenceT newInf = (*it);

				MU_UpdatePositionInfluence *undo = new MU_UpdatePositionInfluence();
				Position pos;
				pos.type = PT_Point;
				pos.index = point;
				undo->updatePositionInfluence(pos,newInf,oldInf);
				sendUndo(undo);

				calculateRemainderWeight(p->m_influences);

				return true;
			}
		}
	}
	return false;
}

double Model::calculatePositionInfluenceWeight(const Position &pos, unsigned joint)const
{
	switch (pos.type)
	{
		case PT_Vertex:
			return calculateVertexInfluenceWeight(pos.index,joint);

		case PT_Point:
			return calculatePointInfluenceWeight(pos.index,joint);

		default:
			break;
	}
	return 0.0;
}

double Model::calculateVertexInfluenceWeight(unsigned vertex, unsigned joint)const
{
	double coord[3] = { 0,0,0 };
	getVertexCoords(vertex,coord);
	return calculateCoordInfluenceWeight(coord,joint);
}

double Model::calculatePointInfluenceWeight(unsigned point, unsigned joint)const
{
	double coord[3] = { 0,0,0 };
	getPointCoords(point,coord);
	return calculateCoordInfluenceWeight(coord,joint);
}

double Model::calculateCoordInfluenceWeight(const double *coord, unsigned joint)const
{
	if(joint>=m_joints.size())
	{
		return 0.0;
	}

	int bcount = m_joints.size();

	int child = -1;
	double cdist = 0.0;
	for(int b = 0; b<bcount; b++)
	{
		if(getBoneJointParent(b)==(int)joint)
		{
			double ccoord[3];
			getBoneJointCoords(b,ccoord);
			double d = distance(ccoord,coord);

			if(child<0||d<cdist)
			{
				child = b;
				cdist = d;
			}
		}
	}

	double bvec[3] = { 0,0,0 };
	double pvec[3] = { 0,0,0 };

	getBoneVector(joint,bvec,coord);

	double jcoord[3] = { 0,0,0 };
	getBoneJointCoords(joint,jcoord);

	pvec[0] = coord[0]-jcoord[0];
	pvec[1] = coord[1]-jcoord[1];
	pvec[2] = coord[2]-jcoord[2];

	normalize3(pvec);

	// get cos from point to bone vector
	double bcos = dot3(pvec,bvec);
	bcos = (bcos+1.0)/2.0;

	if(child<0)
	{
		// no children
		return bcos;
	}

	double cvec[3] = { 0,0,0 };
	getBoneVector(child,cvec,coord);

	// get cos from point to child vector
	double ccoord[3] = { 0,0,0 };
	getBoneJointCoords(child,ccoord);

	pvec[0] = coord[0]-ccoord[0];
	pvec[1] = coord[1]-ccoord[1];
	pvec[2] = coord[2]-ccoord[2];

	normalize3(pvec);
	double ccos = dot3(pvec,cvec);

	ccos = -ccos;
	ccos = (ccos+1.0)/2.0;

	return bcos *ccos;
}


bool Model::autoSetPositionInfluences(const Position &pos, double sensitivity,bool selected)
{
	switch (pos.type)
	{
		case PT_Vertex:
			return autoSetVertexInfluences(pos.index,sensitivity,selected);

		case PT_Point:
			return autoSetPointInfluences(pos.index,sensitivity,selected);

		default:
			break;
	}
	return false;
}

bool Model::autoSetVertexInfluences(unsigned vertex, double sensitivity,bool selected)
{
	double coord[3] = { 0,0,0 };
	getVertexCoords(vertex,coord);
	int_list l;
	if(autoSetCoordInfluences(coord,sensitivity,selected,l))
	{
		removeAllVertexInfluences(vertex);
		int_list::iterator it;
		
		for(it = l.begin(); it!=l.end(); it++)
		{
			double w = calculateVertexInfluenceWeight(vertex,*it);
			addVertexInfluence(vertex,*it,Model::IT_Auto,w);
		}
		return true;
	}
	return false;
}

bool Model::autoSetPointInfluences(unsigned point, double sensitivity,bool selected)
{
	double coord[3] = { 0,0,0 };
	getPointCoords(point,coord);
	int_list l;
	if(autoSetCoordInfluences(coord,sensitivity,selected,l))
	{
		removeAllPointInfluences(point);
		int_list::iterator it;
		
		for(it = l.begin(); it!=l.end(); it++)
		{
			double w = calculatePointInfluenceWeight(point,*it);
			addPointInfluence(point,*it,Model::IT_Auto,w);
		}
		return true;
	}
	return false;
}

bool Model::autoSetCoordInfluences(double *coord, double sensitivity,bool selected,int_list &infList)
{
	int bcount = m_joints.size();

	if(bcount<=0)
	{
		return false;
	}

	int bestJoint = -1;
	int bestChild = -1;
	double bestChildDist = 0;
	double bestDist = 0;
	double bestDot  = 0;

	for(int joint = 0; joint<bcount; joint++)
	{
		if(!selected||m_joints[joint]->m_selected)
		{
			int child = -1;
			double cdist = 0.0;
			for(int b = 0; b<bcount; b++)
			{
				if(getBoneJointParent(b)==(int)joint)
				{
					double ccoord[3];
					getBoneJointCoords(b,ccoord);
					double d = distance(ccoord,coord);

					if(child<0||d<cdist)
					{
						child = b;
						cdist = d;
					}
				}
			}

			double bvec[3] = { 0,0,0 };
			double pvec[3] = { 0,0,0 };

			getBoneVector(joint,bvec,coord);

			double jcoord[3] = { 0,0,0 };
			getBoneJointCoords(joint,jcoord);

			pvec[0] = coord[0]-jcoord[0];
			pvec[1] = coord[1]-jcoord[1];
			pvec[2] = coord[2]-jcoord[2];

			normalize3(pvec);

			double dist = distance(coord,jcoord);

			// get cos from point to bone vector
			double bcos = dot3(pvec,bvec);
			if(bcos>0.0)
			{
				dist *= 0.667;// *= bcos;
			}
			else
			{
				dist *= 2.0;
			}

			if(bestJoint<0||dist<bestDist)
			{
				bestJoint = joint;
				bestDist  = dist;
				bestChild = child;
				bestChildDist = cdist;
				bestDot	= bcos;
			}
		}
	}

	if(bestJoint>=0)
	{
		infList.push_back(bestJoint);
		if(bestChild>=0)
		{
			if((bestChildDist *(1.0-sensitivity))<(bestDist *0.5))
			{
				infList.push_back(bestChild);
			}
		}
		int parent = getBoneJointParent(bestJoint);
		if(parent>0)
		{
			if(((bestDot-1.0))*sensitivity<-0.080)
			{
				infList.push_back(parent);
			}
		}
		return true;
	}
	return false;
}


void Model::calculateRemainderWeight(infl_list &list)const
{
	int	 remainders = 0;
	double remaining = 1.0;

	infl_list::iterator it;

	for(it = list.begin(); remaining>0&&it!=list.end(); it++)
	{
		if((*it).m_type==IT_Remainder)
		{
			remainders++;
		}
		else
		{
			remaining -= (*it).m_weight;
		}
	}

	if(remainders>0&&remaining>0)
	{
		for(it = list.begin(); it!=list.end(); it++)
		{
			if((*it).m_type==IT_Remainder)
			{
				(*it).m_weight = remaining/(double)remainders;
			}
		}
	}
}
