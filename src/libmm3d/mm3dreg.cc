#ifdef WIN32

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

#include "mm3dreg.h"

#include <stdio.h>
#include <stdlib.h>

namespace Mm3dReg
{

RegErrorE Mm3dRegKey::winToMm3dError( LONG err )
{
    switch ( err )
    {
        case ERROR_SUCCESS:
            return RE_NONE;
            break;
        case ERROR_FILE_NOT_FOUND:
            return RE_NO_KEY;
            break;
        case ERROR_ACCESS_DENIED:
            return RE_ACCESS_DENIED;
            break;
        case ERROR_INVALID_HANDLE:
            return RE_INVALID_HANDLE;
            break;
        default:
            break;
    }

    return RE_UNKNOWN;
}

RegErrorE Mm3dRegKey::deleteKey( HKEY hKey, const char * keyName )
{
    LONG err = RegDeleteKey( hKey, keyName );
    return winToMm3dError( err );
}

int Mm3dRegKey::getRegistryInt( HKEY hKey, 
        const char * keyName, const char * valueName )
{
    Mm3dRegKey key( hKey, keyName, RA_READ );
    if ( key.getLastError() == RE_NONE )
    {
        int val = 0;
        key.getValueInt( valueName, val );
        return val;
    }
    else
    {
        return 0;
    }
}

std::string Mm3dRegKey::getRegistryString( HKEY hKey, 
        const char * keyName, const char * valueName )
{
    Mm3dRegKey key( hKey, keyName, RA_READ );
    if ( key.getLastError() == RE_NONE )
    {
        std::string val = "";
        key.getValueString( valueName, val );
        return val;
    }
    else
    {
        return "";
    }
}

RegErrorE Mm3dRegKey::setRegistryInt( HKEY hKey, 
        const char * keyName, const char * valueName, int val )
{
    RegErrorE err = RE_NONE;
    Mm3dRegKey key( hKey, keyName, RA_WRITE );
    err = key.getLastError();
    if ( err == RE_NONE )
    {
        err = key.setValueInt( valueName, val );
    }
    return err;
}

RegErrorE Mm3dRegKey::setRegistryString( HKEY hKey, 
        const char * keyName, const char * valueName, const std::string & val )
{
    RegErrorE err = RE_NONE;
    Mm3dRegKey key( hKey, keyName, RA_WRITE );
    err = key.getLastError();
    if ( err == RE_NONE )
    {
        err = key.setValueString( valueName, val );
    }
    return err;
}

Mm3dRegKey::Mm3dRegKey( HKEY hKey, const char * subKey, RegAccessE ra )
    : m_hkey( 0 )
{
    openKey( hKey, subKey, ra );
}

Mm3dRegKey::~Mm3dRegKey()
{
    if ( m_hkey )
    {
        RegCloseKey( m_hkey );
        m_hkey = 0;
    }
}

RegErrorE Mm3dRegKey::deleteSubKey( const char * keyName )
{
    RegErrorE rval = RE_NONE;
    if ( m_hkey )
    {
        LONG err = RegDeleteKey( m_hkey, keyName );
        rval = winToMm3dError( err );
        m_error = rval;
    }
    return rval;
}

RegErrorE Mm3dRegKey::deleteValue( const char * valueName )
{
    RegErrorE rval = RE_NONE;
    if ( m_hkey )
    {
        LONG err = RegDeleteValue( m_hkey, valueName );
        rval = winToMm3dError( err );
        m_error = rval;
    }
    return rval;
}

RegErrorE Mm3dRegKey::getValueType( 
        const char * value, DWORD & valueType )
{
    if ( m_hkey )
    {
        LONG err = ERROR_SUCCESS;
        RegErrorE rval = RE_NONE;

        valueType = 0;
        err = RegQueryValueEx( m_hkey, value, NULL, &valueType, NULL, NULL );

        rval = winToMm3dError( err );

        m_error = rval;
        return rval;
    }
    else
    {
        return RE_NO_KEY;
    }
}

RegErrorE Mm3dRegKey::getValueString( 
        const char * value, std::string & valueStr )
{
    if ( m_hkey )
    {
        LONG err = ERROR_SUCCESS;
        RegErrorE rval = RE_NONE;

        DWORD valueType = REG_SZ;
        DWORD ptrLen = 128;
        char * ptr = NULL;
        do {
            if ( ptr )
            {
                free( ptr );
            }
            ptr = (char *) malloc( ptrLen );
            err = RegQueryValueEx( m_hkey, value, NULL, &valueType, (LPBYTE) ptr, &ptrLen );
        } while ( err == ERROR_MORE_DATA );

        rval = winToMm3dError( err );

        if ( err == ERROR_SUCCESS )
        {
            if ( valueType == REG_SZ )
            {
                valueStr = ptr;
            }
            else
            {
                rval = RE_WRONG_TYPE;
            }
        }
        free( ptr );

        m_error = rval;
        return rval;
    }
    else
    {
        return RE_NO_KEY;
    }
}

RegErrorE Mm3dRegKey::getValueInt( 
        const char * value, int & valueInt )
{
    if ( m_hkey )
    {
        LONG err = ERROR_SUCCESS;
        RegErrorE rval = RE_NONE;

        DWORD valueType = REG_DWORD;
        DWORD valueDWord = 0;
        DWORD ptrLen = sizeof( valueDWord );
        err = RegQueryValueEx( m_hkey, value, NULL, &valueType, (LPBYTE) &valueDWord, &ptrLen );

        rval = winToMm3dError( err );

        if ( err == ERROR_SUCCESS )
        {
            if ( valueType == REG_DWORD )
            {
                valueInt = (int) valueDWord;
            }
            else
            {
                rval = RE_WRONG_TYPE;
            }
        }

        m_error = rval;
        return rval;
    }
    else
    {
        return RE_NO_KEY;
    }
}

RegErrorE Mm3dRegKey::setValueString( 
        const char * value, const std::string & valueStr )
{
    if ( m_hkey )
    {
        LONG err = ERROR_SUCCESS;
        RegErrorE rval = RE_NONE;

        DWORD len = valueStr.size();
        err = RegSetValueEx( m_hkey, value, 0, REG_SZ, (LPBYTE) valueStr.c_str(), len );

        rval = winToMm3dError( err );

        m_error = rval;
        return rval;
    }
    else
    {
        return RE_NO_KEY;
    }
}

RegErrorE Mm3dRegKey::setValueInt( 
        const char * value, int valueInt )
{
    if ( m_hkey )
    {
        LONG err = ERROR_SUCCESS;
        RegErrorE rval = RE_NONE;

        DWORD valueDWord = (DWORD) valueInt;
        err = RegSetValueEx( m_hkey, value, 0, REG_DWORD, (LPBYTE) &valueDWord, sizeof( valueInt ) );

        rval = winToMm3dError( err );

        m_error = rval;
        return rval;
    }
    else
    {
        return RE_NO_KEY;
    }
}

RegErrorE Mm3dRegKey::getValueNames( RegStringList & nameList )
{
    nameList.clear();

    if ( m_hkey )
    {
        LONG err = ERROR_SUCCESS;
        RegErrorE rval = RE_NONE;

        DWORD keyCount;
        DWORD keyLen;

        err = RegQueryInfoKey( m_hkey, NULL, NULL, NULL,
                NULL, NULL, NULL,
                &keyCount, &keyLen, NULL,
                NULL, NULL );

        keyLen += 10;

        if ( err == ERROR_SUCCESS )
        {
            char * ptr = (char *) malloc( keyLen );
            for ( DWORD t = 0; t < keyCount; t++ )
            {
                DWORD readLen = keyLen;
                err = RegEnumValue( m_hkey, t, ptr, &readLen,
                        NULL, NULL, NULL, NULL );

                if ( err == ERROR_SUCCESS )
                {
                    nameList.push_back( ptr );
                }
            }
            free( ptr );
        }
        else
        {
            rval = winToMm3dError( err );
        }

        m_error = rval;
        return rval;
    }
    else
    {
        return RE_NO_KEY;
    }
}

RegErrorE Mm3dRegKey::getSubKeyNames( RegStringList & nameList )
{
    nameList.clear();

    if ( m_hkey )
    {
        LONG err = ERROR_SUCCESS;
        RegErrorE rval = RE_NONE;

        DWORD keyCount;
        DWORD keyLen;

        err = RegQueryInfoKey( m_hkey, NULL, NULL, NULL,
                &keyCount, &keyLen, NULL,
                NULL, NULL, NULL,
                NULL, NULL );

        keyLen += 10;

        if ( err == ERROR_SUCCESS )
        {
            char * ptr = (char *) malloc( keyLen );
            for ( DWORD t = 0; t < keyCount; t++ )
            {
                DWORD readLen = keyLen;
                err = RegEnumKeyEx( m_hkey, t, ptr, &readLen,
                        NULL, NULL, NULL, NULL );

                if ( err == ERROR_SUCCESS )
                {
                    nameList.push_back( ptr );
                }
            }
            free( ptr );
        }
        else
        {
            rval = winToMm3dError( err );
        }

        m_error = rval;
        return rval;
    }
    else
    {
        return RE_NO_KEY;
    }
}

RegErrorE Mm3dRegKey::openKey( 
        HKEY hKey, const char * subKey, RegAccessE ra )
{
    LONG err = ERROR_SUCCESS;
    RegErrorE rval = RE_NONE;
    switch ( ra )
    {
        case RA_READ:
            err = RegOpenKeyEx( hKey, subKey, 0, KEY_READ, &m_hkey );
            break;
        case RA_WRITE:
            err = RegCreateKeyEx( hKey, subKey, 0, NULL, REG_OPTION_NON_VOLATILE, 
                    KEY_WRITE, NULL, &m_hkey, NULL );
            break;
        default:
            err = RegCreateKeyEx( hKey, subKey, 0, NULL, REG_OPTION_NON_VOLATILE, 
                    KEY_ALL_ACCESS, NULL, &m_hkey, NULL );
            break;
    }

    rval = winToMm3dError( err );

    m_error = rval;
    return rval;
}

};

#endif // WIN32
