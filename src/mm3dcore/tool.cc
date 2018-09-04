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


#include "tool.h"

#include "log.h"

int Tool::s_allocated = 0;

Tool::Tool()
{
   s_allocated++;
}

Tool::~Tool()
{
   s_allocated--;
}

Tool::ToolCoordT Tool::addPosition( Tool::Parent * parent,
        Model::PositionTypeE type, const char * name,
        double x, double y, double z,
        double xrot, double yrot, double zrot,
        int boneId )
{
    Model::Position pos;
    pos.type  = type;
    pos.index = ~0;

    Matrix m = parent->getParentViewInverseMatrix();

    double tranVec[4] = { x, y, z, 1.0 };

    m.apply( tranVec );

    double rotVec[3] = { xrot, yrot, zrot };
    Matrix rot;
    rot.setRotation( rotVec );
    rot = rot * m;
    rot.getRotation( rotVec );

    Model * model = parent->getModel();

    switch ( type )
    {
        case Model::PT_Vertex:
            pos.index = model->addVertex( tranVec[0], tranVec[1], tranVec[2] );
            break;
        case Model::PT_Joint:
            pos.index = model->addBoneJoint( name, tranVec[0], tranVec[1], tranVec[2], 
                    rotVec[0], rotVec[1], rotVec[2], boneId );
            break;
        case Model::PT_Point:
            pos.index = model->addPoint( name, tranVec[0], tranVec[1], tranVec[2], 
                    rotVec[0], rotVec[1], rotVec[2], boneId );
            break;
        case Model::PT_Projection:
            pos.index = model->addProjection( name, Model::TPT_Cylinder, tranVec[0], tranVec[1], tranVec[2] );
            break;
        default:
            log_error( "don't know how to add a point of type %d\n", 
                    static_cast<int>( type ) );
            break;
    }

    ToolCoordT tc;

    tc.pos = pos;
    tc.oldCoords[0] = tc.newCoords[0] = x;
    tc.oldCoords[1] = tc.newCoords[1] = y;
    tc.oldCoords[2] = tc.newCoords[2] = z;

    return tc;
}

void Tool::movePosition( Tool::Parent * parent,
        const Model::Position & pos,
        double x, double y, double z )
{
    Matrix m = parent->getParentViewInverseMatrix();

    double tranVec[4] = { x, y, z, 1.0 };

    /*
    log_debug( "orig position is %f %f %f\n",
            (float) tranVec[0],
            (float) tranVec[1],
            (float) tranVec[2] );
            */

    m.apply3( tranVec );
    tranVec[0] += m.get(3,0);
    tranVec[1] += m.get(3,1);
    tranVec[2] += m.get(3,2);

    /*
    log_debug( "tran position %f,%f,%f\n", 
            (float) tranVec[0],
            (float) tranVec[1],
            (float) tranVec[2] );
            */

    parent->getModel()->movePosition( pos, 
            tranVec[0], tranVec[1], tranVec[2] );
}

void Tool::makeToolCoordList( Parent * parent, ToolCoordList & list, 
        const std::list< Model::Position > & positions )
{
    Model * model = parent->getModel();
    const Matrix & mat = parent->getParentViewMatrix();

    ToolCoordT tc;
    std::list< Model::Position >::const_iterator it;
    for ( it = positions.begin(); it != positions.end(); it++ )
    {
        tc.pos = (*it);
        model->getPositionCoords( tc.pos, tc.oldCoords );

        mat.apply3( tc.oldCoords );
        tc.oldCoords[0] += mat.get(3,0);
        tc.oldCoords[1] += mat.get(3,1);
        tc.oldCoords[2] += mat.get(3,2);

        tc.newCoords[0] = tc.oldCoords[0];
        tc.newCoords[1] = tc.oldCoords[1];
        tc.newCoords[2] = tc.oldCoords[2];

        list.push_back( tc );
    }
}

