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


#ifndef __MODELUTIL_H
#define __MODELUTIL_H

#include "model.h"

extern Model::ModelErrorE modelutil_saveAs( const char * infile, const char * outfile );
extern Model::ModelErrorE modelutil_saveAsFormat( const char * infile, const char * outformat );
extern Model::ModelErrorE modelutil_saveAs( Model * model, const char * outfile );
extern Model::ModelErrorE modelutil_saveAsFormat( Model * model, const char * outformat );

#endif // __MODELUTIL_H
