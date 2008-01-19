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


#ifndef __ERROROBJ_H
#define __ERROROBJ_H

#include <QObject>
#include <QString>

#include "model.h"
#include "texture.h"

class ErrorObject : public QObject
{
   Q_OBJECT

   public:
      ErrorObject( QObject * parent = NULL );
      virtual ~ErrorObject( );

      QString getModelErrorString( Model::ModelErrorE err, Model * model = NULL );
      QString getTextureErrorString( Texture::ErrorE err );

};

extern ErrorObject g_errorObject;

// Convenience functions
QString modelErrStr( Model::ModelErrorE err, Model * model = NULL );
QString textureErrStr( Texture::ErrorE err );

#endif // __ERROROBJ_H
