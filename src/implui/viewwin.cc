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

#include "pluginmgr.h"
#include "viewwin.h"
#include "viewpanel.h"
#include "model.h"
#include "tool.h"
#include "toolbox.h"
#include "cmdmgr.h"
#include "log.h"

#include "msg.h"
#include "modelstatus.h"
#include "filtermgr.h"
#include "misc.h"
#include "animwin.h"
#include "misc.h"
#include "version.h"
#include "texmgr.h"
#include "sysconf.h"
#include "projectionwin.h"
#include "transformwin.h"
#include "texturecoord.h"

#include "luascript.h"
#include "luaif.h"

extern void viewwin_menubarfunc(int);
static void viewwin_mrumenufunc(int);
static void viewwin_geomenufunc(int);
extern void viewwin_toolboxfunc(int);

static int
viewwin_mruf_menu=0,viewwin_mrus_menu=0,
viewwin_file_menu=0,viewwin_view_menu=0,
viewwin_tool_menu=0,viewwin_modl_menu=0,
viewwin_geom_menu=0,viewwin_mats_menu=0,
viewwin_infl_menu=0,viewwin_help_menu=0,
viewwin_deletecmd=0,viewwin_interlock=1,
viewwin_toolbar = 0;

std::vector<MainWin*> viewwin_list(0); //extern

static utf8 viewwin_title = "Untitled"; //"Maverick Model 3D"

void MainWin::modelChanged(int changeBits) // Model::Observer method
{
	if(_window_title_asterisk==model->getSaved())
	_rewrite_window_title();
				
	//REMOVE ME
	//This is to replace DecalManager functionality
	//until it can be demonstrated Model::m_changeBits
	//is set by every change.
	//https://github.com/zturtleman/mm3d/issues/90
	if(!changeBits)
	{
		views.modelUpdatedEvent();
		if(_texturecoord_win) _texturecoord_win->modelChanged(0);
		return;
	}

	views.modelUpdatedEvent(); 
	sidebar.modelChanged(changeBits);

	if(_projection_win) _projection_win->modelChanged(changeBits);
	if(_texturecoord_win) _texturecoord_win->modelChanged(changeBits);
	//if(_transform_win) = _transform_win->modelChanged(changeBits);
}

extern MainWin* &viewwin(int id=glutGetWindow())
{
	MainWin **o = nullptr;
	for(auto&ea:viewwin_list) if(ea->glut_window_id==id)
	o = &ea; //return ea;
	assert(o); return *o;
}
 
static void viewwin_mru(int id, char *add=nullptr)
{
	glutSetMenu(id); 
	
	id = id==viewwin_mruf_menu?0:100;

	utf8 cfg = "script_mru"+(id==100?0:7);

	std::string &mru = config.get(cfg);

	int lines = std::count(mru.begin(),mru.end(),'\n');

	if(!mru.empty()&&mru.back()!='\n') 
	{
		lines++; mru.push_back('\n');
	}

	if(add)
	{
		assert(*add); //Untitled?

		size_t n,len = strlen(add);

		add[len] = '\n';
		#ifdef WIN32
		for(char*p=add;*p;p++) if(*p=='\\') *p = '/';
		#endif
		if(n=mru.find(add,0,len+1))
		{	
			if(n!=mru.npos) mru.erase(n,len+1);
			else lines++;
			mru.insert(mru.begin(),add,add+len+1);
		}
		add[len] = '\0';

		if(!n) return; //ILLUSTRATING

		if(lines>10) mru.erase(mru.rfind('\n'));
	}

	size_t r,s = 0;
	int i,iN = glutGet(GLUT_MENU_NUM_ITEMS);
	char *p = const_cast<char*>(mru.c_str());
	for(i=0;i<lines;i++,p[s++]='\n',id++)
	{
		p[s=mru.find('\n',r=s)] = '\0';
		if(i>=iN) glutAddMenuEntry(p+r,id);
		else glutChangeToMenuEntry(1+i,p+r,id);
	}

	if(add) config.set(cfg,mru);
}

static bool viewwin_close_func_quit = false;
static void viewwin_close_func()
{		
	MainWin *w = viewwin();
	if(!viewwin_close_func_quit)
	if(!w->save_work_prompt()) return;

	//REMOVE ME
	//viewpanel_display_func is sometimes entered on closing
	//after removal from viewwin_list.
	//glutHideWindow();
	Widgets95::glut::set_glutDisplayFunc(nullptr);

	//viewwin_list.remove(w); //C++
	std::swap(viewwin(w->glut_window_id),viewwin_list.back());
	viewwin_list.pop_back();

	//~MainWin can't do this since Model expects a 
	//GLX context and GLX expects onscreen windows.
	delete w->_swap_models(nullptr);

	//close_ui_by_create_id should cover these as well.
	//if(w->_animation_win) w->_animation_win->close();
	//if(w->_transform_win) w->_transform_win->close();
	//if(w->_projection_win) w->_projection_win->close();
	//if(w->_texturecoord_win) w->_texturecoord_win->close();
	Widgets95::e::close_ui_by_create_id(w->glut_window_id); //F1
	Widgets95::e::close_ui_by_parent_id(w->glut_window_id);

	//HACK: Wait for close_ui_by_parent_id to finish up in
	//the idle stage.
	glutext::glutModalLoop(); delete w;
}
bool MainWin::quit()
{
	bool doSave = false;
	
	#ifndef CODE_DEBUG	
	for(auto ea:viewwin_list)
	if(!ea->model->getSaved())
	{
		 char response = msg_warning_prompt
		 (::tr("Some models are unsaved.  Save before exiting?"));
		 if(response=='C') return false; 
		 if(response=='Y') doSave = true;
	}
	#endif // CODE_DEBUG

	for(auto ea:viewwin_list)
	if(doSave&&!ea->model->getSaved())
	{
		if(!ea->save_work()) return false;
	}	

	/*Crashes wxWidgets.
	//NOTE: close won't take immediate effect.
	for(auto ea:viewwin_list)
	Widgets95::e::close_ui_by_parent_id(ea->glut_window_id);
	viewwin_list.clear();*/
	viewwin_close_func_quit = true;
	while(!viewwin_list.empty())
	{
		glutSetWindow(viewwin_list.back()->glut_window_id);
		viewwin_close_func();
	}

	return true;
}

static utf8 viewwin_key_sequence(utf8 pf, utf8 name, utf8 def="")
{
	char buf[64*2]; 	
	int i,j,iN = snprintf(buf,sizeof(buf),"%s_%s",pf,name);	
	for(i=0,j=0;i<iN;i++) switch(buf[i])
	{
	case '.': break; 
	case '\n': //RotateTextureCommand
	case ' ': buf[i] = '_'; 		
	default: buf[j++] = tolower(buf[i]);
	}
	buf[j] = '\0'; 
	//return keycfg.get(buf,*def?TRANSLATE("KeyConfig",def,name):def);
	return keycfg.get(buf,def);
}
static void viewwin_toolbar_title(std::string &s, Tool *tool, int i)
{
	//NOTE: This is really a title; not a tooltip.
	utf8 name = tool->getName(i);
	utf8 ks = viewwin_key_sequence("tool",name,tool->getKeymap(i));
	s.append(TRANSLATE("Tool",name));
	if(*ks) s.append(" (").append(ks).push_back(')');
//	if(*ks) s.append("\t").append(ks);
}
static void viewwin_synthetic_hotkey(std::string &s, Tool *tool, int i)
{
	utf8 ks = viewwin_key_sequence("tool",tool->getName(i),tool->getKeymap(i));
	if(*ks) s.append(1,'\t').append(ks);
}
static void viewwin_synthetic_hotkey(std::string &s, Command *cmd, int i)
{
	utf8 ks = viewwin_key_sequence("cmd",cmd->getName(i),cmd->getKeymap(i));
	if(*ks) s.append(1,'\t').append(ks);
}
static utf8 viewwin_menu_entry(std::string &s, utf8 key, utf8 n, utf8 t, utf8 def="", bool clr=true)
{
	//utf8 ks = keycfg.get(key,*def?TRANSLATE("KeyConfig",def,t):def);
	utf8 ks = keycfg.get(key,def);
	if(clr) s.clear(); s+=::tr(n,t);
	if(*ks) s.append(1,'\t').append(ks); return s.c_str();
}
static utf8 viewwin_menu_radio(std::string &o, bool O, utf8 key, utf8 n, utf8 t, utf8 def="")
{
	o = O?'O':'o'; o.push_back('|'); return viewwin_menu_entry(o,key,n,t,def,false);
}
static utf8 viewwin_menu_check(std::string &o, bool X, utf8 key, utf8 n, utf8 t, utf8 def="")
{
	o = X?'X':'x'; o.push_back('|'); return viewwin_menu_entry(o,key,n,t,def,false);
}
void MainWin::_init_menu_toolbar() //2019
{
	std::string o; //radio	
	#define E(id,...) viewwin_menu_entry(o,#id,__VA_ARGS__),id_##id
	#define O(on,id,...) viewwin_menu_radio(o,on,#id,__VA_ARGS__),id_##id
	#define X(on,id,...) viewwin_menu_check(o,on,#id,__VA_ARGS__),id_##id

	if(!viewwin_mruf_menu) //static menu(s)
	{
		//UNFINISHED
		/* Most Recently Used */		
		viewwin_mruf_menu = glutCreateMenu(viewwin_mrumenufunc);
		viewwin_mru(viewwin_mruf_menu);

		#ifdef HAVE_LUALIB //UNUSED			
		viewwin_mrus_menu = glutCreateMenu(viewwin_mrumenufunc);		
		viewwin_mru(viewwin_mrus_menu);
		#endif

		viewwin_file_menu = glutCreateMenu(viewwin_menubarfunc);

	glutAddMenuEntry(E(file_new,"&New...","File|New","Ctrl+N"));
	glutAddMenuEntry(E(file_open,"&Open...","File|Open","Ctrl+O"));	
	//UNFINISHED
	glutAddSubMenu(::tr("&Recent Models","File|Recent Models"),viewwin_mruf_menu);
	//SLOT(close()),g_keyConfig.getKey("viewwin_file_close"));
	glutAddMenuEntry(E(file_close,"Close","File|Close"));
	glutAddMenuEntry();
	glutAddMenuEntry(E(file_save,"&Save","File|Save","Ctrl+S"));
	glutAddMenuEntry(E(file_save_as,"Save &As...","File|Save As"));
	glutAddMenuEntry(E(file_export,"&Export...","File|Export"));
	glutAddMenuEntry(E(file_export_selection,"Export Selected...","File|Export Selected"));	
	glutAddMenuEntry();		
	#ifdef HAVE_LUALIB //UNUSED
	glutAddMenuEntry(E(file_run_script,"Run &Script...","File|Run Script"));
	glutAddSubMenu(::tr("&Recent Scripts","File|Recent Script"),viewwin_mrus_menu);
	#endif
	glutAddMenuEntry(E(file_plugins,"Plugins...","File|Plugins"));
	//Will wxWidgets detection ignore the ellipsis?
	glutAddMenuEntry(E(file_prefs,"&Preferences...","")); //wxOSX requires this.
	glutext::glutMenuEnable(id_file_prefs,0); //UNIMPLEMENTED
	glutAddMenuEntry();		
	//TODO: Ctrl+Q, Alt+F4
	utf8 quit = "Ctrl+Q";
	#ifdef WIN32
	quit = "Alt+F4";
	#endif
	glutAddMenuEntry(E(file_quit,"&Quit","File|Quit",quit));
	}	

	if(!viewwin_view_menu) //static menu (NEW)
	{
		//_view_menu is pretty long compared to the others. The
		//purpose of this submenu is to collect the static menu
		//items.
		viewwin_view_menu = glutCreateMenu(viewwin_menubarfunc);	

		//glutAddMenuEntry(E(frame_all,"Frame All","View|Frame","Home"));
		glutAddMenuEntry(X(true,frame_lock,"Interlock","","Ctrl+Shift+E"));
		glutAddMenuEntry(E(frame_all,"Enhance","View|Frame","Shift+E"));
		glutAddMenuEntry(E(frame_selection,"Enhance Selection","View|Frame","E"));
		glutAddMenuEntry();
		glutAddMenuEntry(E(view_swap,"Change Sides","View|Viewports","Shift+Tab"));
		glutAddMenuEntry(E(view_flip,"Bottom on Top","View|Viewports","Shift+Q"));
	}
		bool r,s,t,u,v;
				
		//* SUB MENU */ //View->Render Options		
		_rops_menu = glutCreateMenu(viewwin_menubarfunc);	
		{	
			int conf = config.get("ui_draw_joints",2); //(int)Model::JOINTMODE_BONES
			if(conf!=1) conf = 2;
			r = conf==0; //Model::JOINTMODE_NONE;
			s = conf==1; //Model::JOINTMODE_LINES;
			t = conf==2; //Model::JOINTMODE_BONES;
			glutAddMenuEntry(O(r,rops_hide_joints,"Hide Joints","View|Hide Joints"));			
			glutAddMenuEntry(O(s,rops_line_joints,"Draw Joint Lines","View|Draw Joint Lines"));			
			glutAddMenuEntry(O(t,rops_show_joints,"Draw Joint Bones","View|Draw Joint Bones"));

		glutAddMenuEntry();

			r = true; s = false;
			glutAddMenuEntry(O(r,rops_show_projections,"Draw Texture Projections","View|Draw Texture Projections"));
			glutAddMenuEntry(O(s,rops_hide_projections,"Hide Texture Projections","View|Hide Texture Projections"));
			
		glutAddMenuEntry();
	
			r = config.get("ui_render_bad_textures",true);
			s = !r;
			glutAddMenuEntry(O(r,rops_show_badtex,"Use Red Error Texture","View|Use Red Error Texture"));	
			glutAddMenuEntry(O(s,rops_hide_badtex,"Use Blank Error Texture","View|Use Blank Error Texture"));

		glutAddMenuEntry();

			r = config.get("ui_render_3d_selections",false);
			s = !r;
			glutAddMenuEntry(O(r,rops_show_lines,"Render 3D Lines","View|Render 3D Lines","Shift+W"));
			glutAddMenuEntry(O(s,rops_hide_lines,"Hide 3D Lines","View|Hide 3D Lines"));		
			
		glutAddMenuEntry();

			r = !config.get("ui_render_backface_cull",false);
			s = !r;	
			glutAddMenuEntry(O(r,rops_show_backs,"Draw Back-facing Triangles","View|Draw Back-facing Triangles","Shift+F"));
			glutAddMenuEntry(O(s,rops_hide_backs,"Hide Back-facing Triangles","View|Hide Back-facing Triangles")); 
		}

		_view_menu = glutCreateMenu(viewwin_menubarfunc);	
	
	glutAddSubMenu("Reconfigure",viewwin_view_menu); //NEW
	glutAddMenuEntry();
	//2019: Snap To had been a Tool menu item.
	//It seems to fit better in the View menu.
	//Plus, it has to be unique to the window.
	/* SUB MENU */ //Tools->Snap To		
	//_snap_menu = glutCreateMenu(viewwin_menubarfunc);	
	{	
		r = config.get("ui_snap_vertex",false);
		s = config.get("ui_snap_grid",false);
		//glutAddMenuEntry(X(r,snap_vert,"Vertex","View|Snap to Vertex","Shift+V"));
		glutAddMenuEntry(X(r,snap_vert,"Snap to Vertex","View|Snap to Vertex","Shift+V"));
		//glutAddMenuEntry(X(s,snap_grid,"Grid","View|Snap to Grid","Shift+G"));
		glutAddMenuEntry(X(s,snap_grid,"Snap to Grid","View|Snap to Grid","Shift+G"));
		glutAddMenuEntry(E(view_settings,"Grid Settings...","View|Grid Settings"));
	}		
	//glutAddSubMenu(::tr("Snap To"),_snap_menu);
	glutAddMenuEntry();	
	glutAddSubMenu(::tr("Render Options","View|Render Options"),_rops_menu);	
	glutAddMenuEntry();
	glutAddMenuEntry(O(0,scene_wireframe,"3D Wireframe","View|3D"));
	glutAddMenuEntry(O(0,scene_flat,"3D Flat","View|3D"));
	glutAddMenuEntry(O(0,scene_smooth,"3D Smooth","View|3D"));
	glutAddMenuEntry(O(true,scene_texture,"3D Texture","View|3D"));
	glutAddMenuEntry(O(0,scene_blend,"3D Alpha Blend","View|3D"));
	glutAddMenuEntry();		
	glutAddMenuEntry(O(true,model_wireframe,"Canvas Wireframe","View|Canvas","W"));
	glutAddMenuEntry(O(0,model_flat,"Canvas Flat","View|Canvas"));
	glutAddMenuEntry(O(0,model_smooth,"Canvas Smooth","View|Canvas"));
	glutAddMenuEntry(O(0,model_texture,"Canvas Texture","View|Canvas"));
	glutAddMenuEntry(O(0,model_blend,"Canvas Alpha Blend","View|Canvas"));
	glutAddMenuEntry();
		r = s = t = u = v = false;
		switch(config.get("ui_viewport_count",0))
		{
		case 1: r = true; break;
		case 2: (config.get("ui_viewport_tall",0)?t:s) = true; break;
		case 4: default: u = true; break;
		case 6: v = true;
		}			
		if(r) _curr_view = id_view_1;
		if(s) _curr_view = id_view_1x2;
		if(t) _curr_view = id_view_2x1;
		if(u) _curr_view = id_view_2x2;
		if(v) _curr_view = id_view_3x2;
		_prev_view = u?id_view_1:id_view_2x2;
	glutAddMenuEntry(O(r,view_1,"1 View","View|Viewports","Shift+1"));
	glutAddMenuEntry(O(0,view_2,"2 Views","View|Viewports","Shift+2"));
	glutAddMenuEntry(O(s,view_1x2,"2 Views (Wide)","View|Viewports","Shift+3"));
	glutAddMenuEntry(O(t,view_2x1,"2 Views (Tall)","View|Viewports","Shift+4"));
	glutAddMenuEntry(O(u,view_2x2,"2x2 Views","View|Viewports","Q"));	
	glutAddMenuEntry(O(v,view_3x2,"3x2 Views","View|Viewports","Ctrl+Shift+3"));		
			
	//REMOVE ME
	//NOTE: The selected tool holds a state, but the 
	//menu itself is not expected to change.
	toolbox.registerAllTools(); if(!viewwin_toolbar)
	{
		viewwin_toolbar = glutCreateMenu(viewwin_toolboxfunc);
	
			//FINISH ME
			//https://github.com/zturtleman/mm3d/issues/57

		int sm,id;

		//None (Esc) is first because Esc is top-left on keyboard.
		toolbox.setCurrentTool();
		Tool *tool = toolbox.getCurrentTool();
		int pixmap = glutext::glutCreateImageList((char**)tool->getPixmap(0));
		viewwin_toolbar_title(o="o|",tool,0);
		glutAddMenuEntry(o.c_str(),id_tool_none);
		glutext::glutMenuEnableImage(id_tool_none,pixmap);
		for(int pass=1;pass<=2;pass++)
		{
			int creators = id = 0;
			tool = toolbox.getFirstTool();
			for(;tool;tool=toolbox.getNextTool())	
			if(!tool->isSeparator())
			{
				int iN = tool->getToolCount();

				if(tool->isCreateTool()) creators++;

				//This is designed to put select-tools in the middle of 
				//the toolbar because their shortcuts are in the middle
				//of the keyboard.
				if((pass==1)==(creators<=3&&!tool->isSelectTool()))
				for(int i=0;i<iN;i++)
				{
					viewwin_toolbar_title(o="o|",tool,i);
					glutAddMenuEntry(o.c_str(),id+i);
					glutext::glutUpdateImageList(pixmap,(char**)tool->getPixmap(i));
					glutext::glutMenuEnableImage(id+i,pixmap);
				}

				id+=iN;
			}
		}
		glutext::glutDestroyImageList(pixmap);
	
		viewwin_tool_menu = glutCreateMenu(viewwin_toolboxfunc);

		sm = id = 0; 

		bool sep = false;
		utf8 path = nullptr;

		tool = toolbox.getFirstTool();
		for(;tool;tool=toolbox.getNextTool())	
		if(!tool->isSeparator())
		{
			utf8 p = tool->getPath(); 
			if(p!=path)
			{					
				if(sm) glutSetMenu(viewwin_tool_menu);
				if(sm) glutAddSubMenu(::tr(path,"Tool"),sm);
			}
			if(sep)
			{
				sep = false; glutAddMenuEntry();				
			}
			if(p!=path)
			{
				path = p; 
				sm = p?glutCreateMenu(viewwin_toolboxfunc):0;
			}

			int iN = tool->getToolCount(); 
			for(int i=0;i<iN;i++)
			{
				o = TRANSLATE("Tool",tool->getName(i));
				viewwin_synthetic_hotkey(o,tool,i);
				glutAddMenuEntry(o.c_str(),id+i);
			}
			id+=iN;
		}
		else sep = true;
		if(sm) glutSetMenu(viewwin_tool_menu);
		if(sm) glutAddSubMenu(::tr(path,"Tool"),sm);

		glutAddMenuEntry();
		glutAddMenuEntry(E(tool_none,"None","","Esc"));
		glutAddMenuEntry(E(tool_toggle,"Toggle Tool","","Tab"));
		glutAddMenuEntry(E(tool_recall,"Switch Tool","","Space"));
	}

	if(!viewwin_modl_menu) //static menu(s)
	{
		viewwin_modl_menu = glutCreateMenu(viewwin_menubarfunc);	

	glutAddMenuEntry(E(edit_undo,"Undo","Undo shortcut","Ctrl+Z"));
	glutAddMenuEntry(E(edit_redo,"Redo","Redo shortcut","Ctrl+Y"));
	glutAddMenuEntry();
	glutAddMenuEntry(E(edit_metadata,"Edit Model Meta Data...","Model|Edit Model Meta Data"));
	//SLOT(transformWindowEvent()),g_keyConfig.getKey("viewwin_model_transform"));
	glutAddMenuEntry(E(transform,"Transform Model...","Model|Transform Model"));
	//SEEMS UNNCESSARY
	//glutAddMenuEntry(::tr("Boolean Operation...","Model|Boolean Operation"),id_modl_boolop);
	glutAddMenuEntry();
	glutAddMenuEntry(E(background_settings,"Set Background Image...","Model|Set Background Image","Ctrl+Shift+Back"));
	glutAddMenuEntry(E(merge_models,"Merge...","Model|Merge"));
	glutAddMenuEntry(E(merge_animations,"Import Animations...","Model|Import Animations"));

		viewwin_geom_menu = glutCreateMenu(viewwin_geomenufunc);

			//FINISH ME
			//https://github.com/zturtleman/mm3d/issues/57

		bool sep = false;
		int sm = 0, id = 0;
		utf8 path = nullptr;
		auto cmgr = CommandManager::getInstance();
		for(Command*cmd=cmgr->getFirstCommand();cmd;cmd=cmgr->getNextCommand())
		if(!cmd->isSeparator())
		{
			utf8 p = cmd->getPath(); 
			if(p!=path)
			{					
				if(sm) glutSetMenu(viewwin_geom_menu);
				if(sm) glutAddSubMenu(::tr(path,"Command"),sm);
			}
			if(sep)
			{
				sep = false; glutAddMenuEntry();				
			}
			if(p!=path)
			{
				path = p; 
				sm = p?glutCreateMenu(viewwin_geomenufunc):0;
			}
			
			int iN = cmd->getCommandCount(); 
			for(int i=0;i<iN;i++)
			{
				utf8 n = cmd->getName(i);
				
				//REMOVE ME
				//wxWidgets only allows one shortcut per command.
				if(!memcmp(n,"Dele",4)) viewwin_deletecmd = id+i; 

				o = TRANSLATE("Command",n);
				viewwin_synthetic_hotkey(o,cmd,i);
				glutAddMenuEntry(o.c_str(),id+i);
			}
			id+=iN;
		}
		else sep = true;

		if(sm) glutSetMenu(viewwin_geom_menu);
		if(sm) glutAddSubMenu(::tr(path,"Command"),sm);

		viewwin_mats_menu = glutCreateMenu(viewwin_menubarfunc);	

	glutAddMenuEntry(E(group_settings,"Edit Groups...","Materials|Edit Groups","Ctrl+G"));	
	glutAddMenuEntry(E(material_settings,"Edit Materials...","Materials|Edit Materials","Ctrl+M"));
	glutAddMenuEntry(E(material_cleanup,"Clean Up...","Materials|Clean Up"));
	glutAddMenuEntry();
	glutAddMenuEntry(E(uv_editor,"Edit Texture Coordinates...","Materials|Edit Texture Coordinates","M")); //Ctrl+E
	glutAddMenuEntry(E(projection_settings,"Edit Projection...","Materials|Edit Projection"));	
	glutAddMenuEntry();
	glutAddMenuEntry(E(refresh_textures,"Reload Textures","Materials|Reload Textures"));
	glutAddMenuEntry(E(uv_render,"Paint Texture...","Materials|Paint Texture"));
				
		viewwin_infl_menu = glutCreateMenu(viewwin_menubarfunc);	

	glutAddMenuEntry(E(joint_settings,"Edit Joints...","Joints|Edit Joints","J")); 
	glutAddMenuEntry(E(joint_attach_verts,"Assign Selected to Joint","Joints|Assign Selected to Joint","Ctrl+B"));
	glutAddMenuEntry(E(joint_weight_verts,"Auto-Assign Selected...","Joints|Auto-Assign Selected","Shift+Ctrl+B")); 
	glutAddMenuEntry(E(joint_remove_bones,"Remove All Influences from Selected","Joints|Remove All Influences from Selected")); 
	glutAddMenuEntry(E(joint_remove_selection,"Remove Selected Joint from Influencing","Joints|Remove Selected Joint from Influencing")); 
	glutAddMenuEntry();
	//FIX ME
	//Should be selection based.
	glutAddMenuEntry(E(joint_simplify,"Convert Multiple Influences to Single","Joints|Convert Multiple Influences to Single")); 
	glutAddMenuEntry();
	//FIX ME
	//Selection model should use a mask.
	glutAddMenuEntry(E(joint_select_bones_of,"Select Joint Influences","Joints|Select Joint Influences")); 
	glutAddMenuEntry(E(joint_select_verts_of,"Select Influenced Vertices","Joints|Select Influenced Vertices")); 	
	glutAddMenuEntry(E(joint_select_points_of,"Select Influenced Points","Joints|Select Influenced Points")); 
	glutAddMenuEntry(E(joint_unnassigned_verts,"Select Unassigned Vertices","Joints|Select Unassigned Vertices")); 
	glutAddMenuEntry(E(joint_unnassigned_points,"Select Unassigned Points","Joints|Select Unassigned Points")); 
	}		
		_anim_menu = glutCreateMenu(viewwin_menubarfunc);	

	//I intend to remove these two.
	//glutAddMenuEntry(::tr("Start Animation Mode","Animation|Start Animation Mode"),id_anim_open);
	//glutAddMenuEntry(::tr("Stop Animation Mode","Animation|Stop Animation Mode"),id_anim_close);	
	//glutAddMenuEntry();
	//SLOT(animSetWindowEvent()),g_keyConfig.getKey("viewwin_anim_animation_sets"));
	glutAddMenuEntry(E(animate_settings,"Animation Sets...","Animation|Animation Sets"));		
	glutAddMenuEntry();
	glutAddMenuEntry(E(animate_copy,"Copy Animation Frame","Animation|Copy Animation Frame"));
	//glutAddMenuEntry(E(animate_paste,"Paste Animation Frame","Animation|Paste Animation Frame"));	
	//I think maybe this belongs below?
	//https://github.com/zturtleman/mm3d/issues/65#issuecomment-522878969
	//glutAddMenuEntry(E(animate_clear,"Clear Animation Frame","Animation|Clear Animation Frame"));
	//glutAddMenuEntry();
	glutAddMenuEntry(E(animate_copy_selection,"Copy Selected Keyframes","Animation|Copy Animation Frame"));
	//glutAddMenuEntry(E(animate_paste_selection,"Paste Selected Keyframes","Animation|Paste Animation Frame"));
	glutAddMenuEntry(E(animate_paste,"Paste Animation Frame(s)","Animation|Paste Animation Frame(s)"));
	//Look like no-op to me. See viewwin.h notes?
	//https://github.com/zturtleman/mm3d/issues/65#issuecomment-522878969
	glutAddMenuEntry();
	glutAddMenuEntry(E(animate_rotate,"Set Rotation Keyframe","Animation|Set Rotation Keyframe"));
	glutAddMenuEntry(E(animate_translate,"Set Translation Keyframe","Animation|Set Translation Keyframe"));	
	glutAddMenuEntry(E(animate_clear,"Clear Animation Frame","Animation|Clear Animation Frame"));
	glutAddMenuEntry();
	glutAddMenuEntry(E(animate_play,"Play Animation","","K"));
	glutAddMenuEntry(E(animate_render,"Save Animation Images...","Animation|Save Animation Images"));	
	glutAddMenuEntry(E(animate_window,"Animator Window...","Animation|Animation Window","A"));

		extern void animwin_enable_menu(int=0);
		animwin_enable_menu(); //2019

	if(!viewwin_help_menu) //static menu
	{
		//TODO: According to wxOSX Apple moves Help and About menu items
		//to be in the first menu or a special menu of some kind. In that
		//case this menu may duplicate tha functionality or may degenerate 
		//to Help->License, which could be crummy.

		viewwin_help_menu = glutCreateMenu(viewwin_menubarfunc);	

	glutAddMenuEntry(E(help,"&Contents","Help|Contents","F1"));	
	glutAddMenuEntry(::tr("&License","Help|License"),id_license);		
	glutAddMenuEntry(::tr("&About","Help|About"),id_about);
	}
		
	_menubar = glutCreateMenu(viewwin_menubarfunc);
	glutAddSubMenu(::tr("&File","menu bar"),viewwin_file_menu);
	glutAddSubMenu(::tr("&View","menu bar"),_view_menu);
	glutAddSubMenu(::tr("&Tools","menu bar"),viewwin_tool_menu);
	glutAddSubMenu(::tr("&Model","menu bar"),viewwin_modl_menu);
	glutAddSubMenu(::tr("&Geometry","menu bar"),viewwin_geom_menu);
	glutAddSubMenu(::tr("Mate&rials","menu bar"),viewwin_mats_menu);
	glutAddSubMenu(::tr("&Influences","menu bar"),viewwin_infl_menu);
	glutAddSubMenu(::tr("&Animation","menu bar"),_anim_menu);	
	glutAddSubMenu(::tr("&Help","menu bar"),viewwin_help_menu);
	glutAddSubMenu("",viewwin_toolbar);
	glutAttachMenu(glutext::GLUT_NON_BUTTON);

	#undef E //viewwin_menu_entry
	#undef O //viewwin_menu_radio
	#undef X //viewwin_menu_check
}

static void viewwin_reshape_func(int x, int y)
{
	if(!viewwin_list.empty()) //Exit?
	viewwin()->reshape(x,y);
}
bool MainWin::reshape(int x, int y)
{
	glutSetWindow(glut_window_id);

	int xx,wx = glutGet(GLUT_WINDOW_WIDTH);
	int yy,wy = glutGet(GLUT_WINDOW_HEIGHT);

	if(x==10000){ x = wx; y = wy; } //HACK

	//FIX ME
	//248 is hardcoded. Want to calculate.
	//NOTE: Depends on text in view menus.
	int m = views.viewsM;
	int n = views.viewsN/m;
	int sbw = sidebar.width();
	enum{ extra=1 }; //PERFECTLY BALANCED
	int mx = std::max(extra+243*m+sbw,x); //520
	int my = std::max(520/2*n,y); //520
		
	if(mx!=x||my!=y) goto wrong_shape;
	
	mx = glutGet(GLUT_SCREEN_WIDTH);
	my = glutGet(GLUT_SCREEN_HEIGHT);

	xx = std::min(mx,x);
	yy = std::min(my,y);

	if(xx!=x||yy!=y)
	{
		mx = xx; my = yy; 
		goto wrong_shape;
	}	
	else if(wx!=x||wy!=y) 
	{
		mx = x; my = y; 
	
		wrong_shape:

		glutReshapeWindow(mx,my);

		//glutPostRedisplay();

		return false;
	}

	if(!sidebar.seen()) //Center?
	{
		glutPositionWindow((mx-x)/2,(my-y)/2);
	}
	else if(!glutGet(glutext::GLUT_WINDOW_MANAGED))
	{
		config.set("ui_viewwin_width",x);
		config.set("ui_viewwin_height",y);
	}

	//Inform the central drawing area of its size.
	Widgets95::e::set_viewport_area
	(nullptr,nullptr,(int*)views.shape+0,(int*)views.shape+1);
	int c = (views.shape[0]-extra)/m;
	int c0 = (views.shape[0]-extra)-c*m+c;
	for(int i=0;i<m-1;i++,c0=c)
	{
		//TODO: I think locking the row should do.
		views.views[i]->distance(c0,true).repack();
		if(m!=views.viewsN)
		views.views[i+m]->distance(c0,true).repack();
	}
	views.bar1.portside_row.lock(views.shape[0],false);
	views.bar2.portside_row.lock(views.shape[0],false);
	views.bar1.clip(views.shape[0],false,true);
	views.bar2.clip(x,false,true);
	//NOTE: Some X servers won't be able to handle this
	//clipping over the bottom bar. It will need to cut
	//off higher up.
	//FIX ME: !true is because sometimes the z-order
	//mysteriously changes. See SideBar::SideBar too.
	sidebar.clip(sbw,y-views.status.nav.drop()-2,true);

	glutPostRedisplay(); return true;
}

static int viewwin_init()
{
	//The width needs to be established to get correct number of rows in 
	//toolbar so the height setting is correct.
	glutInitWindowSize 
	(config.get("ui_viewwin_width",640),config.get("ui_viewwin_height",520));
	glutInitDisplayMode(GLUT_RGB|GLUT_DOUBLE|GLUT_DEPTH);
	return glutCreateWindow(viewwin_title);
}
MainWin::MainWin(Model *model):
model(/*model*/),		
glut_window_id(viewwin_init()),
//NOTE: Compilers (MSVC) may not like "this".
//Makes parent/child relationships headaches.
views(*this),sidebar(*this),
_animation_win(),
_transform_win(),
_projection_win(),
_texturecoord_win(),
_prev_tool(3),_curr_tool(1),
_prev_mode(id_model_texture),
_prev_view(),_curr_view()
{
	if(!model) model = new Model;
	
	model->setUndoEnabled(false);

	viewwin_list.push_back(this);

	glutext::glutSetWindowIcon(pics[pic_icon]);
	

		_swap_models(model);


	views.setCurrentTool(toolbox.getCurrentTool(),0);
	//views.status.setText(::tr("Press F1 for help using any window"));
	views.status.setText(::tr(Win::f1_titlebar::msg()));
		

		_init_menu_toolbar(); //LONG SUBROUTINE		


	model->setUndoEnabled(true);
	model->clearUndo(); //???

	glutSetWindow(glut_window_id);
	glutext::glutCloseFunc(viewwin_close_func); 
	Widgets95::glut::set_glutReshapeFunc(viewwin_reshape_func);
	glutPopWindow(); //Make spacebar operational.
		
	viewwin_toolboxfunc(id_tool_none);	

	#ifdef WIN32
	//The toolbar is 1px too close for comfort.
	//Making room for shadow rules.
	//views.bar1.main->space<3>()++;
	views.timeline.space<3>(1);
	sidebar.main->space<3>()++;
	#endif

	views.timeline.drop(views.bar1.exterior_row.lock
	(false,sidebar.anim_panel.media_nav.drop()+1).drop()+1);
	views.params.nav.space<Win::top>(2);
	views.params.nav.lock(false,views.bar1.exterior_row.drop());
	//reshape(config.get("ui_viewwin_width",0),config.get("ui_viewwin_height",0));
	reshape(glutGet(GLUT_INIT_WINDOW_WIDTH),glutGet(GLUT_INIT_WINDOW_HEIGHT));
}
MainWin::~MainWin()
{
	//NOTE: viewwin_close_func implements teardown logic
	//prior to the C++ destructor stage.

	log_debug("deleting view window\n");

	/*Not using ContextT since lists/contexts are shared.
	//views.freeTextures();
	log_debug("freeing texture for viewport\n");
	model->removeContext(static_cast<ContextT>(this));*/

	log_debug("deleting view window %08X, model %08X\n",this,model);

	if(viewwin_list.empty())
	{
		#ifdef _DEBUG
		config.flush(); //keycfg.flush();
		#ifdef WIN32
		//wxFileConfig and Model::Background sometimes crash
		//deallocating std::string.
		TerminateProcess(GetModuleHandle(nullptr),0); //Fast. 
		#endif	
		#endif
	}

	//GLX can't have its OpenGL context at this stage.
	//delete _swap_models(nullptr);
	assert(!model);

	//glutHideWindow();
	for(int*m=&_view_menu;m<=&_menubar;m++)
	glutDestroyMenu(*m);

	//Could do this... Win::_delete is currently letting
	//nonmodal top-level windows be deleted. That includes
	//the help window, etc.
	//delete _animation_win;
	//delete _transform_win;
	//delete _projection_win;
	//delete _texturecoord_win;
}

Model *MainWin::_swap_models(Model *swap)
{
	if(swap==model) //NEW
	{
		assert(model!=swap); return nullptr; 
	} 

	if(model) model->removeObserver(this);

	if(swap) swap->addObserver(this);

	//FYI: If !swap this is what ~MainWin used
	//to do. viewwin_close_func calls this now.
	std::swap(swap,const_cast<Model*&>(model));		
	views.setModel();
	if(!model) return swap; //Closing?

	sidebar.setModel();
	//sidebar updates animation.
	//if(_animation_win)
	//_animation_win->setModel();
	if(_projection_win)
	_projection_win->setModel();
	if(_texturecoord_win)
	_texturecoord_win->setModel();

	//YUCK
	//https://github.com/zturtleman/mm3d/issues/56
	if(!swap) 
	{
		/*auto jointMode = 
		(Model::DrawJointModeE)config.get("ui_draw_joints",0);
		if(jointMode!=Model::JOINTMODE_LINES)
		jointMode = Model::JOINTMODE_BONES;		
		model->setDrawJoints(jointMode);*/
		//config.set("ui_draw_joints",(int)jointMode); //???		
		if(1==config.get("ui_draw_joints",2))
		model->setDrawOption(Model::DO_BONES,false);
		if(config.get("ui_render_bad_textures",true))
		model->setDrawOption(Model::DO_BADTEX,true);
		if(config.get("ui_render_backface_cull",false))
		model->setDrawOption(Model::DO_BACKFACECULL,true);
		if(config.get("ui_render_3d_selections",false))
		model->setDrawSelection(true);	
		//
		Model::ViewportUnits &vu = model->getViewportUnits();
		vu.inc = config.get("ui_grid_inc",4.0);
		vu.grid = config.get("ui_grid_mode",0);
		vu.inc3d = config.get("ui_3dgrid_inc",4.0);
		vu.lines3d = config.get("ui_3dgrid_count",6);
		if(config.get("ui_3dgrid_xy",false)) vu.xyz3d|=4;
		if(config.get("ui_3dgrid_xz",true)) vu.xyz3d|=2;
		if(config.get("ui_3dgrid_yz",false)) vu.xyz3d|=1;
		if(config.get("ui_snap_grid",false)) vu.snap|=vu.UnitSnap;
		if(config.get("ui_snap_vertex",false)) vu.snap|=vu.VertexSnap;
	}
	else
	{
		glutSetMenu(_rops_menu);
		glutext::glutMenuEnable(id_rops_show_joints,glutext::GLUT_MENU_CHECK);		
		model->setDrawOption(swap->getDrawOptions(),true);
		model->setDrawSelection(swap->getDrawSelection());
		model->getViewportUnits() = swap->getViewportUnits();

		model->setCanvasDrawMode(swap->getCanvasDrawMode());
		model->setPerspectiveDrawMode(swap->getPerspectiveDrawMode());
	}

	_window_title_asterisk = model->getSaved();
	_rewrite_window_title();

		frame(); //NEW
	
	while(model->hasErrors())
	model_status(model,StatusError,STATUSTIME_LONG,"%s",model->popError().c_str());
		
	return swap;
}

bool MainWin::save_work_prompt()
{
	#ifndef CODE_DEBUG
	if(!model->getSaved())
	{
		int ret = Win::InfoBox(::tr("Save first?"),
		::tr("Model has been modified\n"
		"Do you want to save before closing?"),
		id_yes|id_no|id_cancel,id_cancel);
		if(ret==id_yes&&!save_work())
		{
			ret = id_cancel;
		}
		return ret!=id_cancel;
	}		
	#endif
	return true;
}
bool MainWin::save_work()
{
	const char *filename = model->getFilename();
	if(!*filename)
	return MainWin::save(model,false); //save-as
	
	Model::ModelErrorE err =
	FilterManager::getInstance()->writeFile(model,filename,false);
	if(err==Model::ERROR_NONE)
	{
		model->setSaved(true);
		_rewrite_window_title();
		viewwin_mru(viewwin_mruf_menu,(char*)filename);
		return true;
	}
	if(Model::operationFailed(err))
	{
		msg_error("%s:\n%s",filename,modelErrStr(err,model));
	}
	return false;
}
bool MainWin::save(Model *model, bool expdir)
{
	std::string verb = "Save: ";
	verb+=::tr(expdir?"All Exportable Formats":"All Writable Formats");
	verb+=" (";
	verb+=FilterManager::getInstance()->getAllWriteTypes(expdir);
	verb+=");; "; //Qt
	verb+=::tr("All Files(*)");

	const char *title;
	const char *modelFile;
	std::string file; if(expdir)
	{
		title = "Export save";
		modelFile = model->getFilename();
		file = config.get("ui_export_dir");
	}
	else
	{
		title = "Save save file as";
		modelFile = model->getExportFile();
		file = config.get("ui_model_dir");
	}
	if(*modelFile) //???
	{
		std::string fullname,fullpath,basename; //REMOVE US
		normalizePath(modelFile,fullname,fullpath,basename);
		file = fullpath;
	}
	if(file.empty()) file = "."; 
	file = Win::FileBox(file,verb,::tr(title));
	if(!file.empty())
	{	
		if(file.find('.')==file.npos)
		{
			file.append(".mm3d");
		}

		Model::ModelErrorE err =
		FilterManager::getInstance()->writeFile(model,file.c_str(),expdir);

		if(err==Model::ERROR_NONE)
		{
			utf8 cfg; if(expdir)
			{
				cfg = "ui_export_dir";
				model->setExportFile(file.c_str());
			}
			else
			{
				cfg = "ui_model_dir";
				model->setSaved(true);
				model->setFilename(file.c_str());
			}
			
			viewwin_mru(viewwin_mruf_menu,(char*)file.c_str());

			config.set(cfg,file,file.rfind('/'));			

			return true;
		}
		else if(Model::operationFailed(err))
		{
			msg_error(modelErrStr(err,model));
		}
	}
	return false;
}

void MainWin::merge(Model *model, bool animations_only_non_interactive)
{
	std::string verb = "Open: ";
	verb+=::tr("All Supported Formats","model formats");
	verb+=" (";
	verb+=FilterManager::getInstance()->getAllReadTypes();
	verb+=");; "; //Qt
	verb+=::tr("All Files(*)");

	std::string file = config.get("ui_model_dir");
	if(file.empty()) file = ".";
	file = Win::FileBox(file,verb,::tr("Open model file"));
	if(!file.empty())
	{
		Model::ModelErrorE err;
		Model *merge = new Model();
		if((err=FilterManager::getInstance()->readFile(merge,file.c_str()))==Model::ERROR_NONE)
		{
			model_show_alloc_stats();

			if(!animations_only_non_interactive)
			{
				extern void mergewin(Model*,Model*);
				mergewin(model,merge);
			}
			else model->mergeAnimations(merge);

			viewwin_mru(viewwin_mruf_menu,(char*)file.c_str());

			config.set("ui_model_dir",file,file.rfind('/'));
		}
		else if(Model::operationFailed(err))
		{
			msg_error("%s:\n%s",file.c_str(),modelErrStr(err,merge));
		}
		delete merge;
	}
}

void MainWin::run_script(const char *file)
{
	#ifdef HAVE_LUALIB

	if(!file) return; //???
	
	LuaScript lua;
	LuaContext lc(model);
	luaif_registerfunctions(&lua,&lc);

	std::string scriptfile = file;

	std::string fullname,fullpath,basename; //REMOVE US
	normalizePath(scriptfile.c_str(),fullname,fullpath,basename); //???

	log_debug("running script %s\n",basename.c_str());
	int rval = lua.runFile(scriptfile.c_str());

	viewwin_mru(viewwin_mrus_menu,(char*)file.c_str());

	if(rval==0) //UNFINISHED
	{
		log_debug("script complete,exited normally\n");
		//QString str = ::tr("Script %1 complete").arg(basename.c_str());
		//model_status(model,StatusNormal,STATUSTIME_SHORT,"%s",(const char *)str);
		model_status(model,StatusNormal,STATUSTIME_SHORT,::tr("Script %s complete"),basename.c_str());

	}
	else
	{
		log_error("script complete,exited with error code %d\n",rval);
		//QString str = ::tr("Script %1 error %2").arg(basename.c_str()).arg(lua.error());
		//model_status(model,StatusError,STATUSTIME_LONG,"%s",(const char *)str);
		model_status(model,StatusError,STATUSTIME_LONG,::tr("Script %1 error %2"),basename.c_str(),lua.error());
	}

	model->setNoAnimation();
	model->operationComplete(basename.c_str());

	//views.modelUpdatedEvent();

	#endif // HAVE_LUALIB
}

void MainWin::frame(int scope)
{
	auto os = (Model::OperationScopeE)scope;

	//TODO: Need easy test for no selection.
	double x1,y1,z1,x2,y2,z2;	
	if(model->getBounds(os,&x1,&y1,&z1,&x2,&y2,&z2))
	{
		//NEW: true locks 2d viewports.
		views.frameArea(viewwin_interlock,x1,y1,z1,x2,y2,z2);
	}
	else //NEW: If nothing is selected, frame model. 
	{	
		if(os==Model::OS_Selected) frame(Model::OS_Global);
	}
}

template<class W>
static W *viewwin_position_window(W *w)
{
	glutSetWindow(w->glut_window_id());

	int x = glutGet(glutext::GLUT_X);
	int y = glutGet(glutext::GLUT_Y);
	
	glutPositionWindow(x-4,y); return w;
}
void MainWin::open_texture_window()
{
	if(!model->getSelectedTriangleCount())
	{
		//2019: Seems obtrusive?
		//msg_info(::tr("You must select faces first.\nUse the 'Select Faces' tool.","Notice that user must have faces selected to open 'edit texture coordinates' window"));
		//return;
		model_status(model,StatusError,STATUSTIME_LONG,TRANSLATE("Command","Must select faces"));
	}
	if(!_texturecoord_win)
	{
		_texturecoord_win = new TextureCoordWin(*this);
	}
	viewwin_position_window(_texturecoord_win)->open();
}
void MainWin::open_projection_window()
{
	if(!_projection_win)
	{
		_projection_win = new ProjectionWin(*this);
	}
	viewwin_position_window(_projection_win)->open();
}
void MainWin::open_transform_window()
{
	if(!_transform_win)
	{
		_transform_win = new TransformWin(*this);
	}
	viewwin_position_window(_transform_win)->open();
}
void MainWin::open_animation_system()
{
	if(!_animation_win)
	{
		extern bool viewwin_menu_origin; //YUCK
		viewwin_menu_origin = true;

		//NOTE: On GTK/XWin this briefly flickers the
		//titlebar. I tried to fix it without success.
		_animation_win = new AnimWin(*this,_anim_menu);
		_animation_win->hide();
	}
	_animation_win->open(false);
}
void MainWin::sync_animation_system()
{
	if(_animation_win) _animation_win->open(true); //Undo.
}
void MainWin::open_animation_window()
{
	open_animation_system();

	viewwin_position_window(_animation_win)->show();
}

extern void viewwin_undo(int id, bool undo)
{
	MainWin *w = viewwin(id); 
	if(undo) w->undo(); else w->redo();
}
void MainWin::undo()
{
	log_debug("undo request\n");

	if(model->canUndo())
	{
		//INVESTIGATE ME
		//This string doesn't persist after the operation... might be a bug?
		std::string buf = model->getUndoOpName();

		if(!model->getAnimationMode()) //REMOVE ME
		{
			model->undo();

			if(model->getAnimationMode()) //REMOVE ME
			{
				sync_animation_system();
			}
		}
		else _animation_win->submit(id_edit_undo); //REMOVE ME
		
		model_status(model,StatusNormal,STATUSTIME_SHORT,::tr("Undo %s"),buf.c_str()); //"Undo %1"		

		//REMOVE ME
		if(model->getSelectedBoneJointCount())
		{
			//model->setDrawJoints((Model::DrawJointModeE)config.get("ui_draw_joints",0));
			model->setDrawJoints(true);
			
			views.modelUpdatedEvent(); //HACK //???
		}
	}
	else model_status(model,StatusNormal,STATUSTIME_SHORT,::tr("Nothing to undo"));
}
void MainWin::redo()
{
	log_debug("redo request\n");

	if(model->canRedo())
	{
		//INVESTIGATE ME
		//This string doesn't persist after the operation... might be a bug?
		std::string buf = model->getRedoOpName();

		if(!model->getAnimationMode()) //REMOVE ME
		{
			model->redo();

			if(model->getAnimationMode()) //REMOVE ME
			{
				sync_animation_system();
			}
		}
		else _animation_win->submit(id_edit_redo); //REMOVE ME

		//REMOVE ME
		if(model->getSelectedBoneJointCount()) 
		{
			//model->setDrawJoints((Model::DrawJointModeE)config.get("ui_draw_joints",0));
			model->setDrawJoints(true);

			views.modelUpdatedEvent(); //HACK //???
		}
		
		model_status(model,StatusNormal,STATUSTIME_SHORT,::tr("Redo %s"),buf.c_str()); //"Redo %1"
	}
	else model_status(model,StatusNormal,STATUSTIME_SHORT,::tr("Nothing to redo"));
}

bool MainWin::open(const char *file2, MainWin *window)
{
	std::string buf; if(!file2) //openModelDialog
	{		
		std::string &file = buf; //semi-shadowing

		std::string verb = "Open: ";
		verb+=::tr("All Supported Formats","model formats");
		verb+=" (";
		verb+=FilterManager::getInstance()->getAllReadTypes();
		verb+=");; "; //Qt
		verb+=::tr("All Files(*)");

		file = config.get("ui_model_dir");
		if(file.empty()) file = ".";
		file = Win::FileBox(file,verb,::tr("Open model file"));		
		if(!file.empty())
		config.set("ui_model_dir",file,file.rfind('/'));
		if(file.empty()) return false;
	}
	utf8 file = file2?file2:buf.c_str();
	if(window&&!window->save_work_prompt()) 
	{
		return false;
	}

	bool opened = false;

	log_debug(" file: %s\n",file); //???

	Model *model = new Model();
	auto err = Model::ERROR_NONE;
	if(*file)
	err = FilterManager::getInstance()->readFile(model,file);
	if(err==Model::ERROR_NONE)
	{
		opened = true;

		if(*file)
		model_show_alloc_stats();

		assert(model->getSaved());

		if(window)
		{
			delete window->_swap_models(model);
		}
		else window = new MainWin(model);
		
		//FIX ME
		//NEW: Since "ContextT" is not in play the textures 
		//aren't loaded automatically by model.
		glutSetWindow(window->glut_window_id);
		model->loadTextures();
	
		if(*file)
		viewwin_mru(viewwin_mruf_menu,(char*)file);
	}
	else
	{
		if(Model::operationFailed(err))
		{
			msg_error("%s:\n%s",file,modelErrStr(err,model));
		}
		delete model;
	}

	return opened;
}

void MainWin::_rewrite_window_title()
{
	//HACK: Ensure contiguous storage.
	model->m_filename.push_back('*');

	utf8 path = model->getFilename();

	//TODO: Isolate URL path component.
	utf8 name = strrchr(path,'/');

	name = name?name+1:path;

	bool untitled = '*'==*name;
	bool asterisk = !model->getSaved();

	//NOTE: I've not put the software's name in the title. It could be put
	//back. Or it could be be an option.
	if(untitled&&!asterisk)
	{
		name = viewwin_title;
	}
	else if(untitled)
	{
		name = ::tr("[unnamed]","For filename in title bar (if not set)");
		if(!strcmp(name,"[unnamed]"))
		name = "Untitled";
	}
	else if(!asterisk)
	{
		model->m_filename.back() = '\0';
	}

	glutSetWindow(glut_window_id);
	glutSetWindowTitle(name);

	_window_title_asterisk = asterisk;

	model->m_filename.pop_back(); //*
}

extern bool viewwin_menu_origin = false;
struct viewin_menu
{
	viewin_menu(){ viewwin_menu_origin = true; }
	~viewin_menu(){ viewwin_menu_origin = false; }
};
void viewwin_menubarfunc(int id) //extern
{
	viewwin()->perform_menu_action(id);
}
void MainWin::perform_menu_action(int id)
{
	viewin_menu raii;

	//* marked cases are unsafe to call
	//without glutSetWindow.

	MainWin *w = this; Model *m = model;

	switch(id)
	{
	case 127: //Delete?

		//REMOVE ME
		//Note: The default key combo is Ctrl+Shift+D.
		return viewwin_geomenufunc(viewwin_deletecmd);

		/*File menu*/
	
	case id_file_new:
		
		new MainWin; return;

	case id_file_open: 

		MainWin::open(nullptr,w); return;

	case id_file_close:

		//2019: Changing behavior.
		
		if(1) //Open blank model?
		{
			MainWin::open("",this);
			glutPostRedisplay();
		}
		else //Close window?
		{
			viewwin_close_func(); //*
		}
		return;

	case id_file_save: 
		
		w->save_work(); return;

	case id_file_save_as: 
		
		MainWin::save(model,false); return;

	case id_file_export: 
		
		MainWin::save(model,true); return;

	case id_file_export_selection: 
		
		if(m->getSelectedPointCount()
		+m->getSelectedTriangleCount()
		+m->getSelectedProjectionCount())
		{
			if(Model*tmp=m->copySelected())
			{
				MainWin::save(tmp,true); delete tmp;
			}
		}
		else model_status(m,StatusError,STATUSTIME_LONG,
		::tr("You must have at least 1 face, joint, or point selected to Export Selected"));
		return;

	//case id_file_run_script:
	#ifdef HAVE_LUALIB
	{
		case id_file_run_script:
		std::string file = config.get("ui_script_dir");
		if(file.empty()) file = ".";	
		file = Win::FileBox(file,
		"Lua scripts (*.lua)" ";; " "All Files(*)", //::tr("All Files(*)")
		::tr("Open model file"));
		if(!file.empty())
		{			
			w->run_script(file);
			config.set("ui_script_dir",file,file.rfind('/'));
		}
		return;
	}
	#endif //HAVE_LUALIB

	case id_file_plugins: 
		
		extern void pluginwin(); 
		pluginwin(); return;

	case id_file_quit: 
		
		//TODO: Confirm close all windows if there are more than one.
		if(MainWin::quit()) 
		{
			//qApp->quit();
		}
		return;

	/*View->Render Options menu*/
	case id_rops_hide_joints:

		if(m->getSelectedBoneJointCount())
		if('Y'==msg_info_prompt(::tr("Cannot hide with selected joints.  Unselect joints now?"),"yN"))		
		{
			m->unselectAll();
			m->operationComplete(::tr("Hide bone joints"));
		}
		else return; //break; [[fallthrough]];
		
	case id_rops_line_joints: 
	case id_rops_show_joints:

		model->setDrawOption(Model::DO_BONES,id==id_rops_show_joints);
		if(id-=id_rops_hide_joints)
		config.set("ui_draw_joints",id);
		m->setDrawJoints(id!=0);
		break;

	case id_rops_hide_projections:

		if(model->getSelectedProjectionCount())	
		if('Y'==msg_info_prompt(::tr("Cannot hide with selected projections.  Unselect projections now?"),"yN"))
		{
			m->unselectAll();
			model->operationComplete(::tr("Hide projections"));
		}
		else return; //break; [[fallthrough]];

	case id_rops_show_projections:
		
		id-=id_rops_hide_projections;
		config.set("ui_render_projections",id);
		model->setDrawProjections(!id);
		break;

	case id_rops_show_badtex: 
	case id_rops_hide_badtex: 
		
		id-=id_rops_show_badtex;
		config.set("ui_render_bad_textures",!id);
		//NEW
		model->setDrawOption(Model::DO_BADTEX,!id);
		break;

	case id_rops_show_lines:
	case id_rops_hide_lines: 
		
		id-=id_rops_show_lines;
		//Let Shift+W toggle.
		if(id==!model->getDrawSelection())
		{
			id = !id;
			glutext::glutMenuEnable(id_rops_show_lines+id,glutext::GLUT_MENU_CHECK);
		}		
		config.set("ui_render_3d_selections",!id);
		model->setDrawSelection(!id);
		break;

	case id_rops_show_backs:
	case id_rops_hide_backs:
		
		id-=id_rops_show_backs;
		//Let Shift+F toggle.
		if(id!=!(Model::DO_BACKFACECULL&model->getDrawOptions()))
		{
			id = !id;
			glutext::glutMenuEnable(id_rops_show_backs+id,glutext::GLUT_MENU_CHECK);
		}
		config.set("ui_render_backface_cull",id);
		//NEW
		model->setDrawOption(Model::DO_BACKFACECULL,id!=0);
		break;

	/*View menu*/
	case id_frame_all:
	case id_frame_selection:
		
		w->frame(id==id_frame_all);
		return;

	case id_frame_lock: //EXPERIMENTAL

		viewwin_interlock = glutGet(glutext::GLUT_MENU_CHECKED);
		return;

	//case id_view_props: //REMOVING
	//	
	//	w->sidebar.prop_panel.nav.open(); return;

	case id_model_wireframe:
	case id_model_flat:
	case id_model_smooth:
	case id_model_texture:
	case id_model_blend:
		
		//Let W toggle modes.
		{
			int curr = m->getCanvasDrawMode()+id_model_wireframe;
			if(id==curr) std::swap(id,_prev_mode);			
			else _prev_mode = curr;
			glutext::glutMenuEnable(id,glutext::GLUT_MENU_CHECK);
		}

		id-=id_model_wireframe;		
		m->setCanvasDrawMode((ModelViewport::ViewOptionsE)id);
		break;
	
	case id_scene_wireframe: //id_view_prsp1
	case id_scene_flat:
	case id_scene_smooth:
	case id_scene_texture:
	case id_scene_blend:

		id-=id_scene_wireframe;
		m->setPerspectiveDrawMode((ModelViewport::ViewOptionsE)id);
		break;

		//_view lets Q toggle modes.
	case id_view_1:   _view(id,&ViewPanel::view1);   break;
	case id_view_2:   _view(id,&ViewPanel::view2);   break;
	case id_view_1x2: _view(id,&ViewPanel::view1x2); break;
	case id_view_2x1: _view(id,&ViewPanel::view2x1); break;
	case id_view_2x2: _view(id,&ViewPanel::view2x2); break;
	case id_view_3x2: _view(id,&ViewPanel::view3x2); break;

	case id_view_swap: w->views.rearrange(1); break;
	case id_view_flip: w->views.rearrange(2); break;

	case id_view_settings:
		
		extern void viewportsettings(Model*); 
		viewportsettings(model); break;

	/*View->Snap To menu*/
	case id_snap_grid: //*
	case id_snap_vert: //*
	{
		int x = glutGet(glutext::GLUT_MENU_CHECKED);

		config.set(id==id_snap_grid?"ui_snap_grid":"ui_snap_vertex",x);
		
		Model::ViewportUnits &vu = model->getViewportUnits();

		int y = id==id_snap_grid?vu.UnitSnap:vu.VertexSnap;

		if(x) vu.snap|=y; else vu.snap&=~y; 

		return;
	}
	/*Model menu*/
	case id_edit_undo: w->undo(); break;
	case id_edit_redo: w->redo(); break;
	case id_edit_metadata:
		
		extern void metawin(Model*);
		metawin(m); return;

	case id_transform: 
		
		w->open_transform_window(); return;

	//case id_modl_boolop: //REMOVING 
	//	
	//	w->sidebar.bool_panel.nav.open(); return;

	case id_background_settings: 
		
		extern void backgroundwin(Model*);
		backgroundwin(m); return;

	case id_merge_models:
	case id_merge_animations:

		MainWin::merge(m,id==id_merge_animations); return;

	/*Materials menu*/
	case id_group_settings:

		extern void groupwin(Model*);
		groupwin(m); return;

	case id_material_settings: 

		extern void texwin(Model*);
		texwin(m); return;

	case id_material_cleanup: 
		
		extern void groupclean(Model*);
		groupclean(m); return;

	case id_refresh_textures: 
	
		//NOTE: It does timestamp comparison, but I think
		//it should be limited to the current main window.
		if(TextureManager::getInstance()->reloadTextures())		
		for(auto ea:viewwin_list)
		{
			Model *model = ea->model;
			model->invalidateTextures();
			views.modelUpdatedEvent();
		}
		return;

	case id_projection_settings: 
		
		w->open_projection_window(); return;
	
	case id_uv_editor:

		w->open_texture_window(); return;

	case id_uv_render:
		
		extern void painttexturewin(Model*);
		painttexturewin(m); return;

	/*Influences menu*/
	case id_joint_settings: 
		
		extern void jointwin(Model*);
		jointwin(m); return;
		
	case id_joint_attach_verts:
	case id_joint_weight_verts:
	case id_joint_remove_bones:
	case id_joint_remove_selection:
	case id_joint_simplify:
	case id_joint_select_bones_of:
	case id_joint_select_verts_of:
	case id_joint_select_points_of:
	case id_joint_unnassigned_verts:
	case id_joint_unnassigned_points:

		extern void viewwin_influences(Model*,int);
		viewwin_influences(m,id); return;

	case id_animate_settings: 
		
		extern void animsetwin(MainWin&);
		animsetwin(*this); return;

	case id_animate_render:
		
		extern void animexportwin(Model*,ViewPanel*);
		animexportwin(m,&w->views); return;
		
	case id_animate_window:
		
		open_animation_window(); return;

	case id_animate_copy: 
	case id_animate_copy_selection:
	case id_animate_paste:
	//case id_animate_paste_selection:
	case id_animate_clear:	

	case id_animate_play: case id_animate_pause:
		
		if(!_animation_win)
		open_animation_system();
		_animation_win->submit(id);
		return;
	
	case id_animate_rotate: 
	case id_animate_translate: 
	{
		//I believe this bogus code has the side-effect of making a position
		//uninterpolated. I believe id_animate_clear has the opposite effect.
		bool r = id==id_animate_rotate; Matrix i; double point[3] = {};		
		if(r) m->rotateSelected(i,point); else m->translateSelected(i);
		m->operationComplete(::tr(r?"Set rotation keframe":"Set translation keframe"));
		return;
	}
	
	/*Help menu*/
	case id_help: 
	case id_about:
	case id_license:

		extern void aboutwin(int); 
		aboutwin(id); return;
	}

	views.modelUpdatedEvent(); //HACK //???
}
void MainWin::_view(int i, void (ViewPanel::*mf)())
{
	//95% of this code deal with 2x modes.
	//It's a simple concept otherwise.
	bool two = 2==views.viewsN;
	if(i==_curr_view||two&&i==id_view_2)
	{
		glutext::glutMenuEnable(_prev_view,glutext::GLUT_MENU_CHECK);
		perform_menu_action(_prev_view);			
	}
	else
	{
		(views.*mf)();
		if(i==id_view_2)
		i = views.views1x2?id_view_1x2:id_view_2x1;		
		if(!two||2!=views.viewsN)
		_prev_view = _curr_view;
		_curr_view = i;
	}	
}

void viewwin_toolboxfunc(int id) //extern
{	
	int iid = id;

	MainWin *w = viewwin();
	bool def = id_tool_none==id;
	bool cmp = w->toolbox.getCurrentTool()->isNullTool();
	Tool *tool = nullptr; 
	switch(id)
	{
	case id_tool_recall:

		iid = id = w->_prev_tool; break;

	case id_tool_toggle:
	
		def = !cmp;
		iid = id = cmp?w->_curr_tool:id_tool_none; 
		if(cmp) break;

	case id_tool_none:

		w->toolbox.setCurrentTool();
		tool = w->toolbox.getCurrentTool();
		w->views.timeline.redraw();
		id = 0; break;
	}
	if(!tool)
	{
		tool = w->toolbox.getFirstTool();
		for(;tool;tool=w->toolbox.getNextTool())
		{
			int iN = tool->getToolCount(); 
			if(id>=iN||tool->isSeparator())
			id-=iN; else break;
		}
	}
	if(!tool){ assert(0); return; } //???
		
	//REMOVE ME
	//w->toolbox.getCurrentTool()->deactivated();
	w->toolbox.setCurrentTool(tool);
	
	//tool->activated(id);
	w->views.setCurrentTool(tool,id); //NEW
	
	if(!def&&iid!=w->_curr_tool)
	{
		w->_prev_tool = w->_curr_tool;
		w->_curr_tool = iid;
	}

	if(viewwin_toolbar!=glutGetMenu()) //Window menu?
	{
		//NOTE: wxWidets doesn't do this. I can't figure out
		//a sane way to do it without assuming that the menu
		//item IDs are the same for the toolbar's. Even then
		//it's hard to say items definitely belong to a menu.
		glutSetMenu(viewwin_toolbar);
		glutext::glutMenuEnable(iid,glutext::GLUT_MENU_CHECK);
	}

	bool empty = !w->views.params.nav.first_child();
	w->views.params.nav.set_hidden(empty||def);
	w->views.timeline.set_hidden(!def);

	int wx = glutGet(GLUT_WINDOW_X);
	int x = glutGet(glutext::GLUT_X)-wx;
	int div = w->views.shape[0]/7;
	enum{ l=Win::left,c=Win::center,r=Win::right };
	w->views.params.nav.align(x>div*2?x>div*5?r:c:l);

	//TESTING
	//May want to do regardless??
	//Trying to offer visual cues by hiding vertices?
	if(def!=cmp) glutPostWindowRedisplay(w->glut_window_id);
}

static void viewwin_geomenufunc(int id)
{
	viewin_menu raii;

	auto cmgr = CommandManager::getInstance();
	for(Command*cmd=cmgr->getFirstCommand();cmd;cmd=cmgr->getNextCommand())
	{
		int iN = cmd->getCommandCount();
		if(id<iN&&!cmd->isSeparator())
		{
			Model *model = viewwin()->model;
			if(cmd->activated(id,model))
			model->operationComplete(TRANSLATE("Command",cmd->getName(id)));
			return;
		}
		id-=iN;
	}
}

static void viewwin_mrumenufunc(int id)
{
	MainWin *w = viewwin();
	
	utf8 cmp = "script_mru";
	utf8 cfg = cmp+(id==100?0:7);
	if(id>=100) id-=100;
	
	std::string &mru = config.get(cfg);

	size_t pos = 0; while(id-->0)
	{
		pos = mru.find('\n',pos+1); 
	}
	if(~pos)
	{
		if(pos) pos++;
	}
	else{ assert(~pos); return; }
	
	size_t pos2 = mru.find('\n',pos);
	if(~pos2) mru[pos2] = '\0';

	char *str = const_cast<char*>(mru.c_str()+pos);

	#ifdef WIN32
	for(char*p=str;*p;p++) if(*p=='\\') *p = '/';
	#endif

	//viewwin_mru consumes this string.
	std::string buf(str);

	if(cfg==cmp)
	{
		w->run_script(buf.c_str());
	}
	else MainWin::open(buf.c_str(),w);
}
