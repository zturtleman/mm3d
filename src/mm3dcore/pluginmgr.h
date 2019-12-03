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


#ifndef __PLUGINMGR_H
#define __PLUGINMGR_H

/*#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif*/

#include "mm3dtypes.h"
#include "mm3dconfig.h"

/*#ifdef WIN32
typedef HMODULE LibHandle;
#else
typedef void *LibHandle;
#endif*/
typedef void *LibHandle;

class PluginManager
{
	public:
		enum _PluginStatus_e
		{
			PluginActive,
			PluginUserDisabled,
			PluginVersionDisabled,
			PluginNotPlugin, //???
			PluginError
		};
		typedef enum _PluginStatus_e PluginStatusE;

		static PluginManager *getInstance();
		static void release();

		bool getInitializeAll()		{ return m_initialize; };
		void setInitializeAll(bool o){ m_initialize = o;	 };

		void disable(const char *pluginName);
		bool isDisabled(const char *pluginName);

		void loadPlugins();
		bool loadPlugin(const char *pluginDir);
		bool loadPluginDir(const char *pluginDir);
		bool unloadPlugins();

		int_list getPluginIds();

		const char * getPluginName(int id);
		const char * getPluginVersion(int id);
		const char * getPluginDescription(int id);
		PluginStatusE getPluginStatus(int id);

	protected:
		struct _PluginData_t
		{
			public:
				int  m_id;
				bool m_enabled;
				PluginStatusE m_status;
				std::string m_name;
				bool (*m_initFunction)();
				bool (*m_uninitFunction)();
				const char *(*m_versionFunction)();
				const char *(*m_descFunction)();
				LibHandle m_fileHandle; //LibHandle (HMODULE)
		};
		typedef _PluginData_t PluginDataT;

		typedef std::vector<PluginDataT *> PluginDataList;

		int getNextId(){ return m_nextId++; };

		std::string fileToName(const char *);

		PluginManager();
		~PluginManager();

		static PluginManager *s_instance;

		int m_nextId;
		bool m_initialize;
		PluginDataList m_plugins;
		std::list<std::string> m_disabled;
};

extern int init_plugins();
extern int uninit_plugins();

#endif // __PLUGINMGR_H
