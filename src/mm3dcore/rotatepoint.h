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


#ifndef __ROTATEPOINT_H__
#define __ROTATEPOINT_H__

#include "glheaders.h"

//2019: Removing RotatePoint to rotatetool.cc.
inline void rotatepoint_draw_manip(float r=0.25f)
{
	//glColor3f(0,1,0);
	glBegin(GL_LINES);

	// XY plane
	glVertex3f(-r,0,0); glVertex3f(0,-r,0);
	glVertex3f(0,-r,0); glVertex3f(r,0,0);
	glVertex3f(r,0,0); glVertex3f(0,r,0);
	glVertex3f(0,r,0); glVertex3f(-r,0,0);

	// YZ plane
	glVertex3f(0,-r,0); glVertex3f(0,0,-r);
	glVertex3f(0,0,-r); glVertex3f(0,r,0);
	glVertex3f(0,r,0); glVertex3f(0,0,r);
	glVertex3f(0,0,r); glVertex3f(0,-r,0);

	// XZ plane
	glVertex3f(-r,0,0); glVertex3f(0,0,-r);
	glVertex3f(0,0,-r); glVertex3f(r,0,0); 
	glVertex3f(r,0,0); glVertex3f(0,0,r);  
	glVertex3f(0,0,r); glVertex3f(-r,0,0);

	glEnd();
}

inline double rotatepoint_adjust_to_nearest(double angle, int degrees=15)
{
	angle/=PIOVER180; // Change to degrees
	angle = degrees*(int)(angle/degrees+(angle<0?-0.5:0.5));
	//log_debug("nearest angle is %f\n",angle); //???
	return angle*PIOVER180;
}

inline double rotatepoint_diff_to_angle(double adjacent, double opposite)
{
	if(adjacent<0.0001&&adjacent>-0.0001)
	{
		adjacent = adjacent>=0?0.0001:-0.0001;
	}

	double angle = atan(opposite/adjacent);

	const double quad = PIOVER180*90;

	if(adjacent<0) if(opposite<0)
	{
		angle = -quad-(quad-angle);
	}
	else angle = quad+quad+angle; return angle;
}

#endif // __ROTATEPOINT_H__
