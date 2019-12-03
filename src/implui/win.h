
#ifndef __WIN_H__
#define __WIN_H__

#include "texwidget.h"

#undef interface
#include <widgets95.h>
using namespace Widgets95::glute;
namespace glutext = Widgets95::glutext;

typedef const char *utf8;

/* Qt supplemental */
static const char *tr(utf8 str,...){ return str; }

extern Widgets95::configure config;
extern Widgets95::configure keycfg;

enum
{
	pic_icon=0,
	pic_play,pic_stop,	
	pic_zoomin,pic_zoomout,
	pic_N
};
extern int pics[pic_N]; //ui.cc

enum
{
	/*standard buttons*/
	id_init=0,id_ok,id_cancel,id_apply,id_yes,id_close, //5

	id_no=8, //ok/cancel/yes/no occupy bits 1 though 4. //8

	id_f1,

	/*printable ASCII range lives here*/

	/*common functions*/
	id_delete=127,id_new,id_name,id_value,id_source,id_browse,

	/*animation operations*/
	id_up,id_down,id_copy,id_split,id_join,id_merge,id_convert,

	/*group operations*/
	id_select,id_assign,id_append,id_remove,id_reset,
		
	id_item,id_subitem,_id_subitems_=id_subitem+9,

	id_sort,id_check,id_bar,

	/*TextureWidget*/
	id_scene,

  //NOTE: THE REST DOUBLE AS keycfg KEYS//

	/*File menu*/
	id_file_new,
	id_file_open,
	id_file_close,
	id_file_save,
	id_file_save_as,
	id_file_export,
	id_file_export_selection,
	id_file_run_script,
	id_file_plugins,
	id_file_prefs, //wxOSX needs to be Preferences.
	id_file_quit, //wxOSX needs to be Quit.

	/*View->Snap menu*/
	id_snap_grid,id_snap_vert,

	/*View->Render Options menu*/
	id_rops_hide_joints,
	id_rops_line_joints,
	id_rops_show_joints,
	id_rops_hide_projections,
	id_rops_show_projections,
	id_rops_show_badtex,
	id_rops_hide_badtex,
	id_rops_show_lines,
	id_rops_hide_lines,
	id_rops_show_backs,
	id_rops_hide_backs,

	/*View menu*/
	id_frame_all,
	id_frame_selection,	
	id_frame_lock,
	id_model_wireframe,
	id_model_flat,
	id_model_smooth,
	id_model_texture,
	id_model_blend,
	id_scene_wireframe,
	id_scene_flat,
	id_scene_smooth,
	id_scene_texture,
	id_scene_blend,
	id_view_1,
	id_view_2,
	id_view_1x2,id_view_2x1,
	id_view_2x2,id_view_3x2,
	id_view_swap,
	id_view_flip,
	id_view_settings,

	/*Tool menu*/
	id_tool_none,
	id_tool_toggle,
	id_tool_recall,

	/*Model menu*/
	id_edit_undo,
	id_edit_redo,
	id_edit_metadata,
	id_transform,
	id_background_settings,
	id_merge_models,
	id_merge_animations,

	/*Materials menu*/
	id_group_settings,
	id_material_settings,
	id_material_cleanup,
	id_refresh_textures,
	id_projection_settings,
	id_uv_editor,
	id_uv_render,

	/*Influences menu*/
	id_joint_settings,
	id_joint_attach_verts,
	id_joint_weight_verts,
	id_joint_remove_bones,
	id_joint_remove_selection,
	id_joint_simplify,
	//These scream for a type mask system!!
	id_joint_select_bones_of,
	id_joint_select_verts_of,
	id_joint_select_points_of,
	id_joint_unnassigned_verts,
	id_joint_unnassigned_points,

	/*Animation menu*/	
	id_animate_settings,
	id_animate_render,
	id_animate_copy,
	id_animate_paste,
	id_animate_copy_selection,
	//id_animate_paste_selection,
	id_animate_rotate,
	id_animate_translate,
	id_animate_clear,
	//Additional
	id_animate_window,
	id_animate_play,id_animate_pause,
	
	/*Help menu*/
	id_help,  //wxOSX needs to be Help.
	id_about,  //wxOSX needs to be About.
	id_license,
};

struct Win : Widgets95::ui
{	
	struct : panel
	{
		virtual void _delete(){}

	}_main_panel;

	virtual void _delete() //override
	{
		if(!modal()&&!subpos()) delete this;
	}

	struct Widget; //YUCK

	virtual Widget *widget() //interface
	{
		return nullptr;
	}

	Win(utf8,Widget*_=nullptr);
	Win(int,int subpos); //subwindow
	void _common_init(bool);
	
	static class Widgets95::e &event;

	static bool basic_special(ui*,int,int);
	static bool basic_keyboard(ui*,int,int);

	void basic_submit(int);

	struct ok_button : button
	{
		ok_button(node *p):button(p,"OK",id_ok){ ralign(); }
	};
	struct cancel_button : button
	{
		cancel_button(node*p):button(p,"Cancel",id_cancel){}
	};
	struct ok_cancel_panel
	{
		row nav;

		ok_button ok; cancel_button cancel;

		ok_cancel_panel(node *p):nav(p),ok(nav),cancel(nav)
		{
			nav.ralign();
		}
	};
	struct f1_titlebar : titlebar
	{	
		f1_titlebar(node *p); //win.cpp

		static utf8 msg(){ return "Press F1 for help"; }

		virtual bool activate(int how)
		{
			if(how==ACTIVATE_MOUSE) basic_special(ui(),1,0);
			return false;
		}
	};
	struct f1_ok_panel
	{
		row nav;

		f1_titlebar f1; ok_button ok;

		f1_ok_panel(node *p):nav(p),f1(nav),ok(nav)
		{
			nav.expand();
		}
	};
	struct f1_ok_cancel_panel
	{
		row nav;

		f1_titlebar f1; ok_cancel_panel ok_cancel;

		f1_ok_cancel_panel(node *p):nav(p),f1(nav),ok_cancel(nav)
		{
			nav.expand();
		}
	};

	struct zoom_set
	{
		zoom_set(node *frame)
			:
		nav(frame),
		value(nav,"",'='),
		in(nav,"",'+'),out(nav,"",'-')
		{
			nav.ralign().space(1);	
			in.picture(pics[pic_zoomin]);
			out.picture(pics[pic_zoomout]);
			typedef TextureWidget tw;
			value.edit(tw::zoom_min,1.0,tw::zoom_max);
			value.spinner.set_speed(-0.00001);
		}

		row nav;
		spinbox value;
		button in,out; //":/images/zoomin.xpm"
	};

	struct slider_value 
	{
		row nav; bar slider;
		
		textbox value; //spinbox value; //FIX ME

		void set(int val)
		{
			slider.set_int_val(val); value.set_int_val(val);
		}
		void set(double val)
		{
			slider.set_float_val(val); value.set_float_val(val);
		}

		template<class T>
		void init(utf8 name, double min, double max, T val)
		{
			slider.spin(val).set_range(min,max); 
			value.edit(val).limit(min,max).name(name);
		}
		template<class T>
		slider_value(node *frame, utf8 name, double min, double max, T val, int id=id_value)
			:
		nav(frame),
		slider(nav,"",bar::horizontal,id,slider_cb),value(nav,"",id,value_cb)
		{
			init(name,min,max,val); slider.style(bar::sunken).space<top>(2);
		}

		static void slider_cb(bar *c)
		{
			((textbox*)c->next())->set_val((double)*c); c->ui()->active_callback(c);
		}
		static void value_cb(textbox *c)
		{
			((bar*)c->prev())->set_val((double)*c); c->ui()->active_callback(c);
		}
	};

	struct multisel_item : li::item
	{
		int m;

		multisel_item(int id, utf8 text):item(id,text),m()
		{}

		virtual int impl(int s)
		{
			if(~s&impl_multisel) switch(s)
			{
			case impl_features: return impl_multisel;
			}
			else switch(s&3)
			{
			case impl_clr: m&=~impl_multisel; break;
			case impl_set: m|=impl_multisel; break;
			case impl_xor: m^=impl_multisel; break;
			}
			return m;
		}
	};
	struct checkbox_item : multisel_item
	{
		using multisel_item::multisel_item; //C++11

		typedef void (*cbcb)(int,checkbox_item&);

		virtual int impl(int s)
		{
			if(~s&impl_checkbox) switch(s)
			{
			case impl_features:
				
				return impl_multisel|impl_checkbox;
			}
			else 
			{
				int cmp = m; switch(s&3)
				{
				case impl_clr: m&=~impl_checkbox; break;
				case impl_set: m|=impl_checkbox; break;
				case impl_xor: m^=impl_checkbox; break;
				}
				if(cmp!=m)
				if(parent())
				if(cbcb f=(cbcb)list().user_cb)
				f(s,*this);
				else assert((cbcb)f);
			}
			return multisel_item::impl(s);
		}
	};
	
	/* QMessageBox supplemental */
	static int InfoBox(utf8,utf8,int=id_ok,int=0);
	static int ErrorBox(utf8,utf8,int=id_ok,int=0);
	static int WarningBox(utf8,utf8,int=id_ok,int=0);	

	/* QFileDialog supplemental */
	typedef Widgets95::string FileBox;

	template<class T> /* QInputDialog supplemental */
	static int EditBox(T*,utf8,
	//GCC warning wants defaults in this declaration.
	utf8=nullptr,double=0,double=0,cb=cb());
};

struct EditWin : Win /* QInputDialog supplemental */
{
	void submit(control*);

	template<class T>
	//FIX ME: How to extend EditWin requires
	//more consideration. This is a big mess.
	//I changed how it works for AnimEditWin.
	EditWin(T *lv, utf8 t, utf8 l, double l1, double l2, cb v, bool init)
		:
	Win(t),swap(),valid(v),
	msg(main,"",false),
	nav(main),	
	edit(nav,"",lv),
	ok_cancel(main)
	{
		msg.set_text(l);

		if(l1!=l2) edit.limit(l1,l2);

		active_callback = &EditWin::submit; 

		if(init) submit(main); //id_init
	}

	void *swap; cb valid;

	wordproc msg;
	panel nav;	
	spinbox edit;
	ok_cancel_panel ok_cancel;
};
template<class T> /* QInputDialog supplemental */
inline int Win::EditBox(T *value, utf8 title, utf8 label,
double limit1, double limit2, cb valid)
{			
	//NOTE: This used to be much more, however I ended up
	//gutting it for AnimEditWin.
	return EditWin(value,title,label,limit1,limit2,valid,true)
	.return_on_close();
}

#endif //__WIN_H__
