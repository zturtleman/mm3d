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


#include "errorobj.h"

ErrorObject g_errorObject;

ErrorObject::ErrorObject( QObject * parent )
   : QObject( parent )
{
}

ErrorObject::~ErrorObject()
{
}

QString ErrorObject::getModelErrorString( Model::ModelErrorE err, Model * model )
{
   switch ( err )
   {
      case Model::ERROR_NONE:
         return tr("Success");
      case Model::ERROR_CANCEL:
         return tr("Canceled");
      case Model::ERROR_UNKNOWN_TYPE:
         return tr("Unrecognized file extension (unknown type)");
      case Model::ERROR_UNSUPPORTED_OPERATION:
         return tr("Operation not supported for this file type");
      case Model::ERROR_BAD_ARGUMENT:
         return tr("Invalid argument (internal error)");
      case Model::ERROR_NO_FILE:
         return tr("File does not exist");
      case Model::ERROR_NO_ACCESS:
         return tr("Permission denied");
      case Model::ERROR_FILE_OPEN:
         return tr("Could not open file");
      case Model::ERROR_FILE_READ:
         return tr("Could not read from file");
      case Model::ERROR_BAD_MAGIC:
         return tr("File is the wrong type or corrupted");
      case Model::ERROR_UNSUPPORTED_VERSION:
         return tr("Unsupported version");
      case Model::ERROR_BAD_DATA:
         return tr("File contains invalid data");
      case Model::ERROR_UNEXPECTED_EOF:
         return tr("Unexpected end of file");
      case Model::ERROR_EXPORT_ONLY:
         return tr("Write not supported, try \"Export...\"");
      case Model::ERROR_FILTER_SPECIFIC:
         if ( model )
         {
            return model->getFilterSpecificError();
         }
         else
         {
            return Model::getLastFilterSpecificError();
         }
      case Model::ERROR_UNKNOWN:
         return tr("Unknown error");
      default:
         break;
   }
   return QString("FIXME: Untranslated model error");
}

QString ErrorObject::getTextureErrorString( Texture::ErrorE err )
{
   switch ( err )
   {
      case Texture::ERROR_NONE:
         return tr("Success");
      case Texture::ERROR_NO_FILE:
         return tr("File does not exist");
      case Texture::ERROR_NO_ACCESS:
         return tr("Permission denied");
      case Texture::ERROR_FILE_OPEN:
         return tr("Could not open file");
      case Texture::ERROR_FILE_READ:
         return tr("Could not read from file");
      case Texture::ERROR_FILE_WRITE:
         return tr("Could not write file");
      case Texture::ERROR_BAD_MAGIC:
         return tr("File is the wrong type or corrupted");
      case Texture::ERROR_UNSUPPORTED_VERSION:
         return tr("Unsupported version");
      case Texture::ERROR_BAD_DATA:
         return tr("File contains invalid data");
      case Texture::ERROR_UNEXPECTED_EOF:
         return tr("Unexpected end of file");
      case Texture::ERROR_UNSUPPORTED_OPERATION:
         return tr("This operation is not supported");
      case Texture::ERROR_BAD_ARGUMENT:
         return tr("Invalid argument (internal error)");
      case Texture::ERROR_UNKNOWN:
         return tr("Unknown error");
      default:
         break;
   }
   return QString("FIXME: Untranslated texture error");
}

QString modelErrStr( Model::ModelErrorE err, Model * model )
{
   return g_errorObject.getModelErrorString( err, model );
}

QString textureErrStr( Texture::ErrorE err )
{
   return g_errorObject.getTextureErrorString( err );
}
