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

#include "pixmap/dragvertextool.xpm"

#include "glmath.h"
#include "model.h"
#include "modelstatus.h"
#include "log.h"

struct DragVertexTool: public Tool
{
	DragVertexTool():Tool(TT_Other){}

	virtual const char *getName(int)
	{
		return TRANSLATE_NOOP("Tool","Drag Vertex on Edge");
	}

	virtual const char *getKeymap(int){ return "Shift+T"; } //"D"

	virtual const char **getPixmap(int){ return dragvertextool_xpm; }

	virtual void mouseButtonDown(int buttonState, int x, int y);
	virtual void mouseButtonMove(int buttonState, int x, int y);

	//REMOVE ME
	virtual void mouseButtonUp(int buttonState, int x, int y)
	{
		model_status(parent->getModel(),StatusNormal,STATUSTIME_SHORT,
		TRANSLATE("Tool","Drag complete"));

		parent->updateAllViews();
	}	
		int m_vertId;
		double m_coords[3];
		std::vector<Vector> m_vectors;
};

extern Tool *dragvertextool(){ return new DragVertexTool; }

void DragVertexTool::mouseButtonDown(int buttonState, int x, int y)
{
	Model *model = parent->getModel();
	
	int_list vertices; //REMOVE ME
	model->getSelectedVertices(vertices);

	m_vectors.clear();
	if(!vertices.empty()) //Or 1==vertices.size() ???
	{
		const int vid = vertices.front();
		
		model->getVertexCoords(vid,m_coords);
		const Matrix &mat = parent->getParentViewMatrix(); // model to viewport space		
		//log_debug("vertex %d is at (%f,%f,%f)\n",vid,m_coords[0],m_coords[1],m_coords[2]); //???
		mat.apply3x(m_coords);

		int iN = model->getTriangleCount();
		for(int i=0;i<iN;i++)
		{
			bool match = false;

			int tv[3]; for(int j=0;j<3;j++)
			{
				tv[j] = model->getTriangleVertex(i,j);
				if(tv[j]==vid)
				{
					// TODO: could find the same vertex multiple times if
					// edge belongs to more than one triangle, may want
					// to prevent that in the future
					match = true;
				}
			}

			if(match) for(int j=0;j<3;j++) if(tv[j]!=vid)
			{
				double c[3];
				model->getVertexCoords(tv[j],c);
				mat.apply3x(c);

				Vector v(c[0]-m_coords[0],c[1]-m_coords[1],c[2]-m_coords[2]);

				//log_debug("adding vector (%f,%f,%f)\n",v[0],v[1],v[2]); //???

				m_vectors.push_back(v);
			}
		}

		model_status(model,StatusNormal,STATUSTIME_SHORT,
		TRANSLATE("Tool","Dragging selected vertex"));

		m_vertId = vid;
	}
	else
	{
		m_vertId = -1;

		model_status(model,StatusError,STATUSTIME_LONG,
		TRANSLATE("Tool","Must a vertex selected")); //FIX ME
	}
}
void DragVertexTool::mouseButtonMove(int buttonState, int x, int y)
{
	Model *model = parent->getModel();

	Vector newPos;
	parent->getParentXYValue(x,y,newPos[0],newPos[1]);

	//log_debug("pos is (%f,%f,%f)\n",newPos[0],newPos[1],newPos[2]); //???

	newPos[0]-=m_coords[0];
	newPos[1]-=m_coords[1];

	//log_debug("pos vector is (%f,%f,%f)\n",newPos[0],newPos[1],newPos[2]); //???

	double pscale = newPos.mag3();
	newPos.normalize3();

	//log_debug("mouse has moved %f units\n",pscale); //???

	if(!m_vectors.empty())
	{
		Vector best = m_vectors.front();
		double bestDp = 0;
		double ratio = 1;

		for(auto&ea:m_vectors)
		{
			Vector vtry = ea;
			vtry[2] = 0;

			double trylen = vtry.mag3();
			vtry.normalize3();

			double dp = newPos.dot3(vtry);

			//log_debug("  dot3 is %f (%f,%f,%f)\n",d,vtry[0],vtry[1],vtry[2]); //???

			if(fabs(dp)>fabs(bestDp))
			{
				bestDp = dp;
				best = ea;
				ratio = pscale/trylen;
			}
		}

		//log_debug("best vector is (%f,%f,%f)\n",best[0],best[1],best[2]); //???

		best.scale3(bestDp*ratio);

		//log_debug("best scaled is (%f,%f,%f)\n",best[0],best[1],best[2]); //???

		best[0]+=m_coords[0];
		best[1]+=m_coords[1];
		best[2]+=m_coords[2];

		//log_debug("best sum is (%f,%f,%f)\n",best[0],best[1],best[2]); //???

		parent->getParentViewInverseMatrix().apply3x(best);

		//log_debug("best applied is (%f,%f,%f)\n",best[0],best[1],best[2]); //???

		model->moveVertex(m_vertId,best[0],best[1],best[2]);
	}

	parent->updateAllViews();
}
