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

#ifdef WIN32

#ifndef __MM3DREG_H
#define __MM3DREG_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>
#include <vector>

namespace Mm3dReg
{

enum _RegAccess_e 
{
    RA_READ,
    RA_WRITE,
    RA_ALL,
};
typedef enum _RegAccess_e RegAccessE;

enum _RegError_e 
{
    RE_NONE,
    RE_NO_KEY,
    RE_ACCESS_DENIED,
    RE_INVALID_HANDLE,
    RE_WRONG_TYPE,
    RE_UNKNOWN,
};
typedef enum _RegError_e RegErrorE;

typedef std::vector< std::string > RegStringList;

class Mm3dRegKey
{
    public:
        Mm3dRegKey( HKEY hKey, const char * subKey, RegAccessE ra = RA_ALL );
        virtual ~Mm3dRegKey( );

        //-------------------------------------------------------
        // Static convenience functions
        //
        static RegErrorE winToMm3dError( LONG err );

        static RegErrorE deleteKey( HKEY hkey, const char * keyName );

        // Get a single value
        static int getRegistryInt( HKEY hKey, const char * subKey, const char * valueName );
        static std::string getRegistryString( HKEY hKey, const char * subKey, const char * valueName );
        
        // Set a single value
        static RegErrorE setRegistryInt( HKEY hKey, const char * subKey, const char * valueName, int val );
        static RegErrorE setRegistryString( HKEY hKey, const char * subKey, const char * valueName, const std::string & val );

        //-------------------------------------------------------
        // Member functions
        //
        RegErrorE deleteSubKey( const char * keyName );
        RegErrorE deleteValue( const char * valueName );

        HKEY getHKey() const { return m_hkey; };

        RegErrorE getLastError() const { return m_error; };

        RegErrorE getValueType( const char * value, DWORD & valueType );
        RegErrorE getValueString( const char * value, std::string & valueStr );
        RegErrorE getValueInt( const char * value, int & valueInt );

        RegErrorE setValueString( const char * value, const std::string & valueStr );
        RegErrorE setValueInt( const char * value, int valueInt );

        RegErrorE getValueNames(  RegStringList & nameList );
        RegErrorE getSubKeyNames( RegStringList & nameList );

    protected:

        RegErrorE openKey( HKEY hKey, const char * subKey, RegAccessE ra );

        RegErrorE m_error;

        HKEY m_hkey;
};

};

#endif // __MM3DREG_H

#endif // WIN32
