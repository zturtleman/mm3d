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


#ifndef __MODELFILTER_H
#define __MODELFILTER_H

//------------------------------------------------------------------
// About the ModelFilter class
//------------------------------------------------------------------
//
// The ModelFilter class is a base class for implementing filters to
// import and export models to various formats.  If you implement a
// ModelFilter, you need to register the filter with the FilterManager.
// You only need one instance of your filter.
//
#include "model.h"

typedef float float32_t;

class ModelFilter
{
   public:

      // This class is used to provide model-specific options.  See
      // the ObjFilter class in objfilter.h and the ObjPrompt
      // class in objprompt.h for an example.
      //
      // For simple filters you can typically ignore anything related
      // to the Options class.
      class Options
      {
         public:
            Options();

            // It is a good idea to override this if you implement
            // filter options in a plugin.
            virtual void release() { delete this; };

            static void stats();

         protected:
            virtual ~Options(); // Use release() instead

            static int s_allocated;
      };

      // To prompt a user for filter options, create a function
      // that matches this prototype and call setOptionsPrompt.
      //
      // The Model argument indicates the model that will be saved.
      //
      // The ModelFilter::Options argument contains the options that
      // should be set as default when the prompt is displayed.
      //
      // You must modify the value of the Options argument to match 
      // the options selected by the user.
      //
      // The return value is false if the prompt (save) was cancelled,
      // and true otherwise.
      //
      // For simple filters you can typically ignore anything related
      // to the Options class.
      typedef bool (*OptionsFuncF)( Model *, ModelFilter::Options * );

      ModelFilter();
      virtual ~ModelFilter() {};

      // It is a good idea to override this if you implement
      // a filter as a plugin.
      virtual void release() { delete this; };

      // readFile reads the contents of 'filename' and modifies 'model' to
      // match the description in 'filename'.  This is the import function.
      //
      // The model argument will be an empty model instance.  If the file
      // cannot be loaded, return the appropriate ModelErrorE error code.
      // If the load succeeds, return Model::ERROR_NONE.
      virtual Model::ModelErrorE readFile( Model * model, const char * const filename ) = 0;

      // writeFile writes the contents of 'model' to the file 'filename'.
      //
      // If the model cannot be written to disk, return the appropriate 
      // ModelErrorE error code.  If the write succeeds, return 
      // ModelErrorE::ERROR_NONE.
      //
      // For simple filters you can typically ignore anything related
      // to the Options class.  If you do not provide model specific options
      // with your filter the Options argument will always be NULL.
      virtual Model::ModelErrorE writeFile( Model * model, const char * const filename, Options * o = NULL ) = 0;

      // This function should return true if the filename's extension matches 
      // a type supported by your filter, and your filter has read support.
      //
      // A NULL argument means, do you support write operations for your
      // supported format[s]?
      virtual bool       canRead( const char * filename = NULL ) = 0;

      // This function should return true if the filename's extension matches 
      // a type supported by your filter, your filter has write support, and
      // over-writing a file of this type is not likely to be problematic
      // (for example, if the model has a skeleton and animations and your
      // write support does not include skeleton and animations, you don't
      // want to overwrite the original if the user accidently selects "Save").
      //
      // If write support is limited, you'll want to allow canExport().
      //
      // A NULL argument means, do you support write operations for your
      // supported format[s]?
      virtual bool       canWrite( const char * filename = NULL ) = 0;

      // This function should return true if the filename's extension matches 
      // a type supported by your filter, and your filter has write support.
      // The canExport function may return true even if the canWrite function
      // returns false. If the canWrite function returns true, canExport
      // should also return true.
      //
      // A NULL argument means, do you support write operations for your
      // supported format[s]?
      virtual bool       canExport( const char * filename = NULL ) = 0;

      // This function should return true if the filename's extension matches 
      // a type supported by your filter, regardless of whether your filter
      // is read-only, write-only, or read-write.
      virtual bool       isSupported( const char * file ) = 0;

      // This function returns an STL list of STL strings of filename patterns
      // for which your model supports read operations.  Generally only one 
      // format type should be provided by a single filter.
      virtual std::list< std::string > getReadTypes()  = 0;

      // This function returns an STL list of STL strings of filename patterns
      // for which your model supports write operations.  Generally only one 
      // format type should be provided by a single filter.
      virtual std::list< std::string > getWriteTypes() = 0;

      // This function returns a dynamically allocated object derived
      // from ModelFilter::Options which holds filter-specific options that
      // the user can specify via a prompt at the time of save.
      //
      // To prompt a user for options, you most provide a prompt function 
      // using setOptionsPrompt()
      //
      // For simple filters you can typically ignore anything related
      // to the Options class.
      virtual Options * getDefaultOptions() { return NULL; };

      // This function takes a pointer to a function which displays a prompt
      // to get filter options from the user.
      //
      // For simple filters you can typically ignore anything related
      // to the Options class.
      virtual void setOptionsPrompt( OptionsFuncF f ) { m_optionsFunc = f; };

      // This function returns the pointer to the Options prompt.
      // This may be NULL.
      //
      // For simple filters you can typically ignore anything related
      // to the Options class.
      virtual OptionsFuncF getOptionsPrompt() { return m_optionsFunc; };

      static Model::ModelErrorE errnoToModelError( int err );

   protected:

      // Call these protected methods in your base class if you want direct
      // access to the model's primitive lists.  Treat them with care.
      vector<Model::Vertex *>             & getVertexList( Model * m )     { return m->m_vertices;     };
      vector<Model::Triangle *>           & getTriangleList( Model * m )   { return m->m_triangles;    };
      vector<Model::Group *>              & getGroupList( Model * m )      { return m->m_groups;       };
      vector<Model::Material *>           & getMaterialList( Model * m )   { return m->m_materials;    };
      vector<Model::Joint *>              & getJointList( Model * m )      { return m->m_joints;       };
      vector<Model::Point *>              & getPointList( Model * m )      { return m->m_points;       };
      vector<Model::TextureProjection *>  & getProjectionList( Model * m ) { return m->m_projections;  };
      vector<Model::SkelAnim *>           & getSkelList( Model * m )       { return m->m_skelAnims;    };
      vector<Model::FrameAnim *>          & getFrameList( Model * m )      { return m->m_frameAnims;   };

      // These functions are deprecated.  Don't use them.  Really.  They're
      // probably going to go away when I have some time to clean up the
      // Milkshape filter.
      void setModelInitialized( Model * m, bool o ) { m->m_initialized = o; };
      void setModelNumFrames( Model * m, int numFrames ) { m->m_numFrames = numFrames; };

      OptionsFuncF m_optionsFunc;
};

#endif // __MODELFILTER_H
