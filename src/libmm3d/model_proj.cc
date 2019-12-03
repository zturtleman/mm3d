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
#include "texture.h"
#include "texmgr.h"

#ifdef MM3D_EDIT
#include "modelundo.h"
#include "modelstatus.h"
#endif // MM3D_EDIT

const char *Model::getProjectionName(unsigned proj)const
{
	if(proj<m_projections.size())
	{
		return m_projections[proj]->m_name.c_str();
	}
	else
	{
		return nullptr;
	}
}

bool Model::setProjectionName(unsigned proj, const char *name)
{
	if(proj<m_projections.size()&&name&&name[0])
	{
		MU_SetProjectionName *undo = new MU_SetProjectionName();
		undo->setName(proj,name,m_projections[proj]->m_name.c_str());
		sendUndo(undo);

		m_projections[proj]->m_name = name;
		return true;
	}
	else
	{
		return false;
	}
}

int Model::getProjectionType(unsigned proj)const
{
	if(proj<m_projections.size())
	{
		return m_projections[proj]->m_type;
	}
	return -1;
}

bool Model::setProjectionType(unsigned proj, int type)
{
	if(proj<m_projections.size())
	{
		MU_SetProjectionType *undo = new MU_SetProjectionType();
		undo->setType(proj,type,m_projections[proj]->m_type);
		sendUndo(undo);

		m_projections[proj]->m_type = type;

		applyProjection(proj);
		return true;
	}
	else
	{
		return false;
	}
}

void Model::setTriangleProjection(unsigned triangleNum, int proj)
{
	if(proj<(int)m_projections.size()
		  &&triangleNum<m_triangles.size())
	{
		// negatigve is valid (means "none")
		MU_SetTriangleProjection *undo = new MU_SetTriangleProjection();
		undo->setTriangleProjection(triangleNum,proj,m_triangles[triangleNum]->m_projection);
		sendUndo(undo);

		m_triangles[triangleNum]->m_projection = proj;
	}
}

int Model::getTriangleProjection(unsigned triangleNum)const
{
	if(triangleNum<m_triangles.size())
	{
		return m_triangles[triangleNum]->m_projection;
	}
	return -1;
}

bool Model::getProjectionCoords(unsigned proj, double *coord)const
{
	if(proj<m_projections.size())
	{
		coord[0] = m_projections[proj]->m_pos[0];
		coord[1] = m_projections[proj]->m_pos[1];
		coord[2] = m_projections[proj]->m_pos[2];
		return true;
	}
	return false;
}

bool Model::setProjectionUp(unsigned proj, const double *upVec)
{
	//log_debug("projection up: %f,%f,%f\n",
	//		upVec[0],upVec[1],upVec[2]);

	if(proj<m_projections.size())
	{
		MU_SetProjectionUp *undo = new MU_SetProjectionUp();
		undo->setProjectionUp(proj,upVec,m_projections[proj]->m_upVec);
		sendUndo(undo);

		m_projections[proj]->m_upVec[0] = upVec[0];
		m_projections[proj]->m_upVec[1] = upVec[1];
		m_projections[proj]->m_upVec[2] = upVec[2];

		applyProjection(proj);
		return true;
	}
	return false;
}

bool Model::getProjectionUp(unsigned proj, double *upVec)const
{
	if(proj<m_projections.size())
	{
		upVec[0] = m_projections[proj]->m_upVec[0];
		upVec[1] = m_projections[proj]->m_upVec[1];
		upVec[2] = m_projections[proj]->m_upVec[2];
		return true;
	}
	return false;
}

bool Model::setProjectionSeam(unsigned proj, const double *seamVec)
{
	if(proj<m_projections.size())
	{
		MU_SetProjectionSeam *undo = new MU_SetProjectionSeam();
		undo->setProjectionSeam(proj,seamVec,m_projections[proj]->m_seamVec);
		sendUndo(undo);

		m_projections[proj]->m_seamVec[0] = seamVec[0];
		m_projections[proj]->m_seamVec[1] = seamVec[1];
		m_projections[proj]->m_seamVec[2] = seamVec[2];

		applyProjection(proj);
		return true;
	}
	return false;
}

bool Model::getProjectionSeam(unsigned proj, double *seamVec)const
{
	if(proj<m_projections.size())
	{
		seamVec[0] = m_projections[proj]->m_seamVec[0];
		seamVec[1] = m_projections[proj]->m_seamVec[1];
		seamVec[2] = m_projections[proj]->m_seamVec[2];
		return true;
	}
	return false;
}

bool Model::getProjectionRange(unsigned proj,
		double &xmin, double &ymin, double &xmax, double &ymax)const
{
	if(proj<m_projections.size())
	{
		xmin = m_projections[proj]->m_range[0][0];
		ymin = m_projections[proj]->m_range[0][1];
		xmax = m_projections[proj]->m_range[1][0];
		ymax = m_projections[proj]->m_range[1][1];
		return true;
	}
	return false;
}

bool Model::setProjectionRange(unsigned proj,
		double xmin, double ymin, double xmax, double ymax)
{
	if(proj<m_projections.size())
	{
		TextureProjection *ptr = m_projections[proj];

		MU_SetProjectionRange *undo = new MU_SetProjectionRange();
		undo->setProjectionRange(proj,
				xmin,ymin,xmax,ymax,
				ptr->m_range[0][0],ptr->m_range[0][1],
				ptr->m_range[1][0],ptr->m_range[1][1]);
		sendUndo(undo);

		ptr->m_range[0][0] = xmin;
		ptr->m_range[0][1] = ymin;
		ptr->m_range[1][0] = xmax;
		ptr->m_range[1][1] = ymax;

		applyProjection(proj);
		return true;
	}
	return false;
}

bool Model::moveProjection(unsigned p, double x, double y, double z)
{
	if(m_animationMode==ANIMMODE_NONE)
	{
		if(p<m_projections.size())
		{
			double old[3];

			old[0] = m_projections[p]->m_pos[0];
			old[1] = m_projections[p]->m_pos[1];
			old[2] = m_projections[p]->m_pos[2];

			m_projections[p]->m_pos[0] = x;
			m_projections[p]->m_pos[1] = y;
			m_projections[p]->m_pos[2] = z;

			applyProjection(p);

			MU_MovePrimitive *undo = new MU_MovePrimitive;
			undo->addMovePrimitive(MU_MovePrimitive::MT_Projection,p,x,y,z,
					old[0],old[1],old[2]);
			sendUndo(undo);

			return true;
		}
	}
	return false;
}

void	Model::setProjectionScale(unsigned proj, double scale)
{
	if(proj<m_projections.size())
	{
		TextureProjection *p = m_projections[proj];
		double seamVec[3];
		double upVec[3];

		int i;
		for(i = 0; i<3; i++)
		{
			upVec[i]	= p->m_upVec[i];
			seamVec[i] = p->m_seamVec[i];
		}

		normalize3(upVec);
		normalize3(seamVec);

		for(i = 0; i<3; i++)
		{
			upVec[i]	*= scale;
			seamVec[i] *= scale;
		}

		{
			MU_SetProjectionUp *undo = new MU_SetProjectionUp();
			undo->setProjectionUp(proj,upVec,p->m_upVec);
			sendUndo(undo);
		}

		{
			MU_SetProjectionSeam *undo = new MU_SetProjectionSeam();
			undo->setProjectionSeam(proj,seamVec,p->m_seamVec);
			sendUndo(undo);
		}

		m_projections[proj]->m_upVec[0] = upVec[0];
		m_projections[proj]->m_upVec[1] = upVec[1];
		m_projections[proj]->m_upVec[2] = upVec[2];

		m_projections[proj]->m_seamVec[0] = seamVec[0];
		m_projections[proj]->m_seamVec[1] = seamVec[1];
		m_projections[proj]->m_seamVec[2] = seamVec[2];

		applyProjection(proj);
	}
}

double Model::getProjectionScale(unsigned proj)const
{
	if(proj<m_projections.size())
	{
		return mag3(m_projections[proj]->m_upVec);
	}
	return 0.0;
}

void Model::applyProjection(unsigned int proj)
{
	if(proj<m_projections.size())
	{
		switch(m_projections[proj]->m_type)
		{
			case TPT_Cylinder:
				applyCylinderProjection(proj);
				break;
			case TPT_Sphere:
				applySphereProjection(proj);
				break;
			case TPT_Plane:
				applyPlaneProjection(proj);
				break;
			default:
				break;
		}
	}
}

typedef struct _TriangleTexCoords_t 
{
	bool	set;

	double vc[3];

	double uac; // cos of angle from up vector to vertex
	double sac; // cos of angle from seam vector to vertex
	double pac; // cos of angle from plane vector to vertex

	double u;
	double v;
} TriangleTexCoordsT;

void Model::applyCylinderProjection(unsigned int p)
{
	// In this projection scheme vertices near the base of the
	// cylinder have a V of 0. Near the top is a V of 1. Above
	// or below the cylinder result in<0 or>1 respectively
	//
	// Vertices near the seam vector have a U of 0 (or 1)and
	// vertices in the opposite direction from the seam vector
	// have a U of 0.5. Perpendicular to the seam vector
	// is 0.25 or 0.75 (see the "pac" calculations below for
	// details).

	if(p<m_projections.size())
	{
		TextureProjection *proj = m_projections[p];
		double up[3];
		double seam[3];
		double upNormal[3];
		double seamNormal[3];
		double planeNormal[3];  // perpendicular to up and seam vectors

		int i;

		for(i = 0; i<3; i++)
		{
			up[i]	= proj->m_upVec[i];
			seam[i] = proj->m_seamVec[i];
			upNormal[i]	= proj->m_upVec[i];
			seamNormal[i] = proj->m_seamVec[i];
		}

		normalize3(upNormal);
		normalize3(seamNormal);

		double upMag = mag3(proj->m_upVec);

		// Because up and seam are vectors from pos,we can assume
		// that pos is at the origin. The math works out the same.
		double orig[3] = { 0,0,0 };
		calculate_normal(planeNormal,orig,up,seam);

		//log_debug("up	 = %f,%f,%f\n",up[0],	up[1],	up[2]);
		//log_debug("seam  = %f,%f,%f\n",seam[0], seam[1], seam[2]);
		//log_debug("plane = %f,%f,%f\n",planeNormal[0],planeNormal[1],planeNormal[2]);

		unsigned tcount = m_triangles.size();
		for(unsigned t = 0; t<tcount; t++)
		{
			if(m_triangles[t]->m_projection==(int)p)
			{
				TriangleTexCoordsT tc[3];
				double center[3] = {0,0,0};

				int setCount = 0;
				double setAverage = 0.0;

				for(i = 0; i<3; i++)
				{
					tc[i].set = false;
					double vc[3];
					double vcn[3];

					getVertexCoords(m_triangles[t]->m_vertexIndices[i],tc[i].vc);

					center[0] += tc[i].vc[0];
					center[1] += tc[i].vc[1];
					center[2] += tc[i].vc[2];

					vc[0] = tc[i].vc[0]-proj->m_pos[0];
					vc[1] = tc[i].vc[1]-proj->m_pos[1];
					vc[2] = tc[i].vc[2]-proj->m_pos[2];

					vcn[0] = vc[0];
					vcn[1] = vc[1];
					vcn[2] = vc[2];

					normalize3(vcn);

					// uac *mag(vc)= distance along up vec to intersection point
					// distance = vertical texture coord

					tc[i].uac = dot3(vcn,upNormal);
					tc[i].sac = dot3(vcn,seamNormal);
					tc[i].pac = dot3(vcn,planeNormal);

					double d = tc[i].uac *mag3(vc)/upMag;
					tc[i].v = (d/2.0)+0.5;

					// get intersection point so we can determine where we are
					// releative to the seam
					vcn[0] = tc[i].vc[0]-up[0] *d;
					vcn[1] = tc[i].vc[1]-up[1] *d;
					vcn[2] = tc[i].vc[2]-up[2] *d;
					vcn[0] -= proj->m_pos[0];
					vcn[1] -= proj->m_pos[1];
					vcn[2] -= proj->m_pos[2];

					if(fabs(mag3(vcn))>0.0001)
					{
						normalize3(vcn);

						// The acos gives us an indicaton of how far off from
						// the seam vector we are,but not in which direction. 
						// We use the sign of the plane's pac for that.
						double sac = acos(dot3(vcn,seamNormal));
						sac = sac/(2 *PI);
						if(tc[i].pac>0.0)
						{
							sac = 0.0+sac;
						}
						else
						{
							sac = 1.0-sac;
						}
						tc[i].u = sac;

						tc[i].set = true;
						setAverage += tc[i].u;
						setCount++;
					}
				}

				// See if we cross the seam.
				//
				// If pac is the same sign for each vertex it means we 
				// don't cross the seam. If any are different,compare
				// the vector to the center of the plane to the seam
				// vector (dot product greater than 0 means it crosses).

				if(	 !(tc[0].pac>0.0&&tc[1].pac>0.0&&tc[2].pac>0.0)
					  &&!(tc[0].pac<0.0&&tc[1].pac<0.0&&tc[2].pac<0.0))
				{
					// finish average
					center[0] /= 3.0;
					center[1] /= 3.0;
					center[2] /= 3.0;

					// convert to unit vector for dot product
					center[0] = center[0]-proj->m_pos[0];
					center[1] = center[1]-proj->m_pos[1];
					center[2] = center[2]-proj->m_pos[2];
					normalize3(center);

					if(dot3(seamNormal,center)>0.0)
					{
						setAverage = 0.0; // need to recalculate this

						// the "side" variable tracks which whether the wrap
						// occurs on the 0.0 side (-1)or the 1.0 side (+1)
						// We pick a side based on which vertex is farthest
						// from the edge (that vert will be in the normal range).
						int side = 0;
						double dist = 0.0;

						int n;
						for(n = 0; n<3; n++)
						{
							if(tc[n].set)
							{
								double d = 0.0;
								int	 s = 0;
								if(tc[n].u<0.5)
								{
									d = fabs(tc[n].u);
									s = -1;
								}
								else
								{
									d = fabs(1.0-tc[n].u);
									s =  1;
								}

								if(d>dist)
								{
									side = s;
									dist = d;
								}
							}
						}

						// do wrap
						//
						// if side is positive,anything closer to 0 than 1
						// needs to be increased by 1.0
						//
						// if side is negative,anything closer to 1 than 0
						// needs to be decreased by 1.0
						for(int n = 0; n<3; n++)
						{
							if(tc[n].set)
							{
								if(tc[n].u<0.5)
								{
									if(side>0)
									{
										tc[n].u += 1.0;
									}
								}
								else
								{
									if(side<0)
									{
										tc[n].u -= 1.0;
									}
								}

								setAverage += tc[n].u;
							}
						}
					}
				}

				if(setCount<3)
				{
					setAverage = (setCount>0)
						? setAverage/(double)setCount 
						: 0.5;

					for(i = 0; i<3; i++)
					{
						if(!tc[i].set)
						{
							tc[i].u = setAverage;
						}
					}
				}

				// Update range and set texture coords
				float xDiff = proj->m_range[1][0]-proj->m_range[0][0];
				float yDiff = proj->m_range[1][1]-proj->m_range[0][1];
				for(i = 0; i<3; i++)
				{
					float u = tc[i].u *xDiff+proj->m_range[0][0];
					float v = tc[i].v *yDiff+proj->m_range[0][1];
					setTextureCoords(t,i,u,v);
				}
			}
		}
	}
}

void Model::applySphereProjection(unsigned int p)
{
	// In this projection scheme vertices are mapped based on
	// their vectors relative to the center of the sphere.
	// Anything on the up vector is a 1 (above center)or 0 
	// (below center).
	//
	// The U coord is determined the same way as the cylinder.
	// Vertices near the seam vector have a U of 0 (or 1)and
	// vertices in the opposite direction from the seam vector
	// have a U of 0.5. Perpendicular to the seam vector
	// is 0.25 or 0.75 (see the "pac" calculations below for
	// details).

	if(p<m_projections.size())
	{
		TextureProjection *proj = m_projections[p];
		double up[3];
		double seam[3];
		double upNormal[3];
		double seamNormal[3];
		double planeNormal[3];  // perpendicular to up and seam vectors

		int i;

		for(i = 0; i<3; i++)
		{
			up[i]	= proj->m_upVec[i];
			seam[i] = proj->m_seamVec[i];
			upNormal[i]	= proj->m_upVec[i];
			seamNormal[i] = proj->m_seamVec[i];
		}

		normalize3(upNormal);
		normalize3(seamNormal);

		// Because up and seam are vectors from pos,we can assume
		// that pos is at the origin. The math works out the same.
		double orig[3] = { 0,0,0 };
		calculate_normal(planeNormal,orig,up,seam);

		//log_debug("up	 = %f,%f,%f\n",up[0],	up[1],	up[2]);
		//log_debug("seam  = %f,%f,%f\n",seam[0], seam[1], seam[2]);
		//log_debug("plane = %f,%f,%f\n",planeNormal[0],planeNormal[1],planeNormal[2]);

		unsigned tcount = m_triangles.size();
		for(unsigned t = 0; t<tcount; t++)
		{
			if(m_triangles[t]->m_projection==(int)p)
			{
				TriangleTexCoordsT tc[3];
				double center[3] = {0,0,0};
				
				int setCount = 0;
				double setAverage = 0.0;

				for(i = 0; i<3; i++)
				{
					tc[i].set = false;
					double vc[3];
					double vcn[3];

					getVertexCoords(m_triangles[t]->m_vertexIndices[i],tc[i].vc);

					center[0] += tc[i].vc[0];
					center[1] += tc[i].vc[1];
					center[2] += tc[i].vc[2];

					vc[0] = tc[i].vc[0]-proj->m_pos[0];
					vc[1] = tc[i].vc[1]-proj->m_pos[1];
					vc[2] = tc[i].vc[2]-proj->m_pos[2];

					vcn[0] = vc[0];
					vcn[1] = vc[1];
					vcn[2] = vc[2];

					normalize3(vcn);

					// uac *mag(vc)= distance along up vec to intersection point
					// 1-(acos(uac)/PI)= vertical texture coord

					tc[i].uac = dot3(vcn,upNormal);
					tc[i].sac = dot3(vcn,seamNormal);
					tc[i].pac = dot3(vcn,planeNormal);

					double d = tc[i].uac *mag3(vc);
					tc[i].v = 1.0-(acos(tc[i].uac)/PI);

					// get intersection point so we can determine where we are
					// releative to the seam
					vcn[0] = tc[i].vc[0]-upNormal[0] *d;
					vcn[1] = tc[i].vc[1]-upNormal[1] *d;
					vcn[2] = tc[i].vc[2]-upNormal[2] *d;
					vcn[0] -= proj->m_pos[0];
					vcn[1] -= proj->m_pos[1];
					vcn[2] -= proj->m_pos[2];

					if(fabs(mag3(vcn))>0.0001)
					{
						normalize3(vcn);

						// The acos gives us an indicaton of how far off from
						// the seam vector we are,but not in which direction. 
						// We use the sign of the plane's pac for that.
						double sac = acos(dot3(vcn,seamNormal));
						sac = sac/(2 *PI);
						if(tc[i].pac>0.0)
						{
							sac = 0.0+sac;
						}
						else
						{
							sac = 1.0-sac;
						}
						tc[i].u = sac;

						tc[i].set = true;
						setAverage += tc[i].u;
						setCount++;
					}
				}

				// See if we cross the seam.
				//
				// If pac is the same sign for each vertex it means we 
				// don't cross the seam. If any are different,compare
				// the vector to the center of the plane to the seam
				// vector (dot product greater than 0 means it crosses).

				if(	 !(tc[0].pac>0.0&&tc[1].pac>0.0&&tc[2].pac>0.0)
					  &&!(tc[0].pac<0.0&&tc[1].pac<0.0&&tc[2].pac<0.0))
				{
					// finish average
					center[0] /= 3.0;
					center[1] /= 3.0;
					center[2] /= 3.0;

					// convert to unit vector for dot product
					center[0] = center[0]-proj->m_pos[0];
					center[1] = center[1]-proj->m_pos[1];
					center[2] = center[2]-proj->m_pos[2];
					normalize3(center);

					if(dot3(seamNormal,center)>0.0)
					{
						setAverage = 0.0; // need to recalculate this

						// the "side" variable tracks which whether the wrap
						// occurs on the 0.0 side (-1)or the 1.0 side (+1)
						// We pick a side based on which vertex is farthest
						// from the edge (that vert will be in the normal range).
						int side = 0;
						double dist = 0.0;

						int n;
						for(n = 0; n<3; n++)
						{
							if(tc[n].set)
							{
								double d = 0.0;
								int	 s = 0;
								if(tc[n].u<0.5)
								{
									d = fabs(tc[n].u);
									s = -1;
								}
								else
								{
									d = fabs(1.0-tc[n].u);
									s =  1;
								}

								if(d>dist)
								{
									side = s;
									dist = d;
								}
							}
						}

						// do wrap
						//
						// if side is positive,anything closer to 0 than 1
						// needs to be increased by 1.0
						//
						// if side is negative,anything closer to 1 than 0
						// needs to be decreased by 1.0
						for(int n = 0; n<3; n++)
						{
							if(tc[n].set)
							{
								if(tc[n].u<0.5)
								{
									if(side>0)
									{
										tc[n].u += 1.0;
									}
								}
								else
								{
									if(side<0)
									{
										tc[n].u -= 1.0;
									}
								}

								setAverage += tc[n].u;
							}
						}
					}
				}

				if(setCount<3)
				{
					setAverage = (setCount>0)
						? setAverage/(double)setCount 
						: 0.5;

					for(i = 0; i<3; i++)
					{
						if(!tc[i].set)
						{
							tc[i].u = setAverage;
						}
					}
				}

				// Update range and set texture coords
				float xDiff = proj->m_range[1][0]-proj->m_range[0][0];
				float yDiff = proj->m_range[1][1]-proj->m_range[0][1];
				for(i = 0; i<3; i++)
				{
					float u = tc[i].u *xDiff+proj->m_range[0][0];
					float v = tc[i].v *yDiff+proj->m_range[0][1];
					setTextureCoords(t,i,u,v);
				}
			}
		}
	}
}

void Model::applyPlaneProjection(unsigned int p)
{
	// In this projection scheme vertices are mapped based on
	// their vectors relative to a square projected into 3D.
	// 
	// The U and V coordinates are assigned based on the
	// distance from the center of the square in 2D space.
	// The up vector is V,the left vector is U.

	if(p<m_projections.size())
	{
		TextureProjection *proj = m_projections[p];
		double up[3];
		double seam[3];
		double upNormal[3];
		double seamNormal[3];
		double planeNormal[3];  // perpendicular to up and seam vectors

		int i;

		for(i = 0; i<3; i++)
		{
			up[i]	= proj->m_upVec[i];
			seam[i] = proj->m_seamVec[i];
			upNormal[i]	= proj->m_upVec[i];
			seamNormal[i] = proj->m_seamVec[i];
		}

		normalize3(upNormal);
		normalize3(seamNormal);

		double upMag = mag3(proj->m_upVec);

		// Because up and seam are vectors from pos,we can assume
		// that pos is at the origin. The math works out the same.
		double orig[3] = { 0,0,0 };
		calculate_normal(planeNormal,orig,up,seam);

		//log_debug("up	 = %f,%f,%f\n",up[0],	up[1],	up[2]);
		//log_debug("seam  = %f,%f,%f\n",seam[0], seam[1], seam[2]);
		//log_debug("plane = %f,%f,%f\n",planeNormal[0],planeNormal[1],planeNormal[2]);

		unsigned tcount = m_triangles.size();
		for(unsigned t = 0; t<tcount; t++)
		{
			if(m_triangles[t]->m_projection==(int)p)
			{
				TriangleTexCoordsT tc[3];
				double center[3] = {0,0,0};

				for(i = 0; i<3; i++)
				{
					double vc[3];
					double vcn[3];

					getVertexCoords(m_triangles[t]->m_vertexIndices[i],tc[i].vc);

					center[0] += tc[i].vc[0];
					center[1] += tc[i].vc[1];
					center[2] += tc[i].vc[2];

					vc[0] = tc[i].vc[0]-proj->m_pos[0];
					vc[1] = tc[i].vc[1]-proj->m_pos[1];
					vc[2] = tc[i].vc[2]-proj->m_pos[2];

					vcn[0] = vc[0];
					vcn[1] = vc[1];
					vcn[2] = vc[2];

					normalize3(vcn);

					// uac *mag(vc)= distance along up vec to intersection point
					// distance = vertical texture coord

					tc[i].uac = dot3(vcn,upNormal);
					//tc[i].sac = dot3(vcn,seamNormal);
					tc[i].pac = dot3(vcn,planeNormal);

					double d;

					d = tc[i].uac *mag3(vc)/upMag;
					tc[i].v = (d/2.0)+0.5;

					d = tc[i].pac *mag3(vc)/upMag;
					tc[i].u = (d/2.0)+0.5;
				}

				// Update range and set texture coords
				float xDiff = proj->m_range[1][0]-proj->m_range[0][0];
				float yDiff = proj->m_range[1][1]-proj->m_range[0][1];
				for(i = 0; i<3; i++)
				{
					float u = tc[i].u *xDiff+proj->m_range[0][0];
					float v = tc[i].v *yDiff+proj->m_range[0][1];
					setTextureCoords(t,i,u,v);
				}
			}
		}
	}
}

