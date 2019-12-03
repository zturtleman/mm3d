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

#include <signal.h> //???

#include "pluginmgr.h"
#include "sysconf.h"
//#include "qtmain.h" //???
#include "cmdline.h"
#include "model.h"
#include "log.h"

//REMOVE ME
#include "cmdmgr.h" //???

#include "filtermgr.h"
#include "texmgr.h"

//#include "statusbar.h" //???
#include "modelstatus.h"
//#include "keycfg.h"
#include "translate.h" //"transimp.h"

//show_alloc_stats
//#include "allocstats.h" //???
#include "modelfilter.h"

int free_memory()
{
	return model_free_primitives();
}

void segfault_handler(int sig) //FIX ME
{
	//Why override default behavior just to print a message?
	fprintf(stderr,"Segfault.  Exiting...\n");
	exit(0);
}

int main(int argc,char *argv[])
{
	int rval = 0;
	
	#ifdef WIN32
	// If started from command prompt,print messages there.
	// TODO: If AttachConsole()returns 0 and specified arguments
	// --debug,--warnings,or --errors run AllocConsole()
	//if(AttachConsole(ATTACH_PARENT_PROCESS)||(want log output&&AllocConsole()))
	AttachConsole(ATTACH_PARENT_PROCESS);
	if(GetConsoleWindow());
	{
		freopen("CONOUT$","w",stdout);
		freopen("CONOUT$","w",stderr);
	}
	#endif

	log_profile_init("profile_data.txt");

	//UNDOCUMENTED
	signal(SIGSEGV,segfault_handler); //???

	init_sysconf();

	extern const char *ui_translate(const char*,const char*);
	transll_install_handler(ui_translate);

	//Might do config.get("anything",0) here?
	//init_prefs(); 

	init_cmdline(argc,argv);

	//Wrong file!
	//Q_INIT_RESOURCE(qtuiResources);

	extern int ui_prep(int&,char*[]);
	ui_prep(argc,argv);

	//???
	{
		LOG_PROFILE();

		//???
		{
			LOG_PROFILE_STR("Initialize");

			//REMOVED
			// set up keyboard shortcuts
			//std::string keycfgFile = getMm3dHomeDirectory();
			//keycfgFile += "/keycfg.in";
			//keycfg_load_file(keycfgFile.c_str());
			//keycfg_set_defaults();

			extern void init_std_filters();
			init_std_filters();
			extern void init_std_texture_filters();
			init_std_texture_filters();
			extern void stdtools_init();
			stdtools_init();
			extern void init_std_cmds(CommandManager*);
			init_std_cmds(CommandManager::getInstance());

			init_plugins();

			extern StatusObject *statusbar_get_object(Model*);
			model_status_register_function(statusbar_get_object);
		}

		extern int ui_init(int&,char*[]);
		rval = ui_init(argc,argv);

		//???
		{
			LOG_PROFILE_STR("Uninitialize");

			//REMOVED
			//std::string keycfgFile = getMm3dHomeDirectory();
			//keycfgFile += "/keycfg.out";
			//keycfg_save_file(keycfgFile.c_str());

			shutdown_cmdline();

			model_show_alloc_stats();

			CommandManager::release();
			FilterManager::release();
			TextureManager::release();
			PluginManager::release();

			free_memory();

			model_show_alloc_stats();
			
			//REMOVE ME
			//show_alloc_stats(); //???
			{
				log_debug("\n");
				log_debug("program allocation stats\n"); //???

				//Prints log_debug("Filter Options: %d\n",s_allocated);
				ModelFilter::Options::stats();

				log_debug("\n");
			}

			//Might do config.flush() here?
			//prefs_save();
		}
	}

	log_profile_shutdown();

	return rval;
}
