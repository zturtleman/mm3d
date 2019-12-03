
#include "mm3dtypes.h" //PCH

#include <typeinfo>
#ifdef __GNUG__
#include <cstdlib>
#include <memory>
#include <cxxabi.h> //win_help
#endif

#include "helpwin.h"
#include "tool.h" //ButtonState
#include "texturecoord.h" //widget
#include "log.h" 

class Widgets95::e &Win::event = Widgets95::e;

//These appear directly under APPDATA:
//%APPDATA%/daedalus3d-mm3d-config.ini
//%APPDATA%/daedalus3d-mm3d-keycfg.ini
Widgets95::configure config("daedalus3d-mm3d-config");
Widgets95::configure keycfg("daedalus3d-mm3d-keycfg");

//REMOVE ME
Win::f1_titlebar::f1_titlebar(node *p):titlebar(p,msg())
{
	id(id_f1); 
	
	_can_activate = true; //HACK

	drop(Widgets95::ui_button_drop);	

	space<top>(4); //HACK: Line up with OK.
}

static void win_close()
{
	auto w = (Win*)Widgets95::e::find_ui_by_window_id();
	auto c = w->main->find([](Win::control *p)
	{
		//Note: Radio button IDs fall into this range.
		return (p->id()==id_cancel||p->id()==id_close)
		&&dynamic_cast<Win::button*>(p);
	});	
	if(!c) c = w->main->find([](Win::control *p)
	{
		return p->id()==id_ok&&dynamic_cast<Win::button*>(p);
	});
	if(c) w->active_callback(c);
	else w->basic_submit(id_close);
}
static void win_enter()
{
	auto w = (Win*)Widgets95::e::find_ui_by_window_id();
	auto c = w->main->find([](Win::control *p)
	{
		//Note: Radio button IDs fall into this range.
		/*Unfortunately id_apply is too ambiguous.
		//TODO? Maybe ctrl+space.
		return (p->id()==id_ok||p->id()==id_apply)
		&&dynamic_cast<Win::button*>(p)&&p->enabled();*/
		return id_ok==p->id()&&dynamic_cast<Win::button*>(p);
	});
	if(c) w->active_callback(c);
	else w->basic_submit(id_ok);
}

static void win_help(const char *typeid_name)
{
	#ifdef __GNUC__
	int _; (void)_;
	typeid_name = abi::__cxa_demangle(typeid_name,0,0,&_);
	#endif
	
	//MSVC puts "class " etc. in front of class names.
	const char *p,*trim = typeid_name;
	while(p=strchr(trim,' ')) trim = p+1;

	utf8 index = trim;
	utf8 ext = ".html";
	switch(*index) //Exception?
	{
	case 'E': //REMOVE ME

		if(!strcmp(index,"ExtrudeWin"))
		{
			index = "commands"; ext = ".html#extrude";
		}
		break;
	}
	//2019: Changing this behavior, so that the help window
	//is consistent, and to be able to work with the window.
	//HelpWin(index,ext).return_on_close();
	new HelpWin(index,ext);
		
	#ifdef __GNUC__
	std::free((void*)typeid_name);
	#endif
}
bool Win::basic_special(ui *w, int k, int)
{
	switch(k)
	{
	case GLUT_KEY_F1:

		//Make center-open consistent.
		extern bool viewwin_menu_origin;
		viewwin_menu_origin = false;

		if(w->subpos())
		{
			new HelpWin(); //Assuming top-level.
			break;
		}

		if(w->main->find([](control *p)
		{
			return p->id()==id_f1&&dynamic_cast<f1_titlebar*>(p);
		}))
		{
			//Seems to work for modal too, at least on Win32.
			//if(!w->modal())
			{
				//Make window sibling.
				glutSetWindow(w->glut_create_id());
				glutPopWindow();
			}
			win_help(typeid(*w).name());
		}
		break;

	case GLUT_KEY_F2:
	case GLUT_KEY_F3:
	case GLUT_KEY_F4:

		if(w->subpos())
		{
			glutSetWindow(w->glut_parent_id());
			extern void viewpanel_special_func(int,int,int);
			viewpanel_special_func(k,0,0);
			break;
		}
		return true;

	case GLUT_KEY_F6:

		#if defined(_DEBUG) && defined(WIDGETS_95_BUILD_EXAMPLES)
		Widgets95::xcv_example(6);
		#endif
		break;

	case GLUT_KEY_F11:

		if(w->subpos()) 
		glutSetWindow(w->glut_parent_id());

		glutFullScreen(); //UNFINISHED
		break;

	default: return true;	
	}
	return false;
}
bool Win::basic_keyboard(ui *w, int k, int m)
{
	if(w->subpos()) switch(k)
	{
	//case ' ': //UNUSED
	case 127: //Delete?
		
		glutSetWindow(w->glut_parent_id());
		extern void viewwin_menubarfunc(int);
		viewwin_menubarfunc(k);
		return false;
	}
	else switch(k)
	{
	case '\r': 

		win_enter(); return false;

	case 27: //ESC

		win_close(); return false;

	case 'z': case 'y': //Hardcoded undo/redo.
		
		extern void viewwin_undo(int,bool);
		if(m&GLUT_ACTIVE_CTRL&&!w->modal())
		viewwin_undo(w->glut_create_id(),k=='z'&&~m&GLUT_ACTIVE_SHIFT);
		else break; return false;
	}
	return true; 
}

static bool win_widget_keyboard(Widgets95::ui *ui, int kb, int cm)
{
	int x = Widgets95::e.curr_x;
	int y = Widgets95::e.curr_y;
	if(((Win*)ui)->widget()->keyPressEventUI(+kb,cm,x,y))
	return true;
	if(kb>0) 
	return Win::basic_keyboard(ui,kb,cm);
	else 
	return Win::basic_special(ui,-kb,cm);
}
static bool win_widget_special(Widgets95::ui *ui, int kb, int cm)
{
	return win_widget_keyboard(ui,-kb,cm);
}
static bool win_widget_mouse(Widgets95::ui *ui, int bt, int st, int x, int y)
{
	int cm = Widgets95::e.curr_modifiers; 
	if(st==GLUT_DOWN)
	return !((Win*)ui)->widget()->mousePressEventUI(bt,cm,x,y);
	else return !((Win*)ui)->widget()->mouseReleaseEventUI(bt,cm,x,y);
}
static bool win_widget_motion(Widgets95::ui *ui, int x, int y)
{
	int cm = Widgets95::e.curr_modifiers; 
	return !((Win*)ui)->widget()->mouseMoveEventUI(cm,x,y);
}
static bool win_widget_wheel(Widgets95::ui *ui, int wh, int x, int y, int cm)
{
	return !((Win*)ui)->widget()->wheelEventUI(wh,cm,x,y);
}

//Reminder: Theoretically this is a moving target since 
//the size may change a few times before it's displayed.
static void win_reshape(Widgets95::ui *ui, int w, int h)
{	
	if(!ui->seen()) if(1) //Mouse?
	{
		glutSetWindow(ui->glut_window_id());

		int x = glutGet(glutext::GLUT_X);
		int y = glutGet(glutext::GLUT_Y);
	
		x-=glutGet(GLUT_WINDOW_WIDTH)/2;
		y-=glutGet(GLUT_WINDOW_HEIGHT)/2;

		glutPositionWindow(x,y);
	}
	/*else //Center?
	{
		glutSetWindow(ui->glut_create_id());

		int cx = glutGet(GLUT_WINDOW_X);
		int cy = glutGet(GLUT_WINDOW_Y);
		int cw = glutGet(GLUT_WINDOW_WIDTH);
		int ch = glutGet(GLUT_WINDOW_HEIGHT);

		glutSetWindow(ui->glut_window_id());

		glutPositionWindow(cx+(cw-w)/2,cy+(ch-h)/2);		
	}*/
}
static void win_reshape2(Widgets95::ui *ui, int w, int h)
{	
	if(!ui->seen())
	{
		//Note: this nudges the window to be fully onscreen.
		int x = glutGet(glutext::GLUT_X);
		int y = glutGet(glutext::GLUT_Y);
		glutSetWindow(ui->glut_window_id());
		glutPositionWindow(x-4,y);
	}
}
static void win_reveal(int i)
{
	//This is just in case ui::seen fails win_reshape.
	if(auto*ui=Widgets95::e::find_ui_by_window_id(i))	
	if(!ui->seen())
	{
		log_debug("Plan-B win_reveal had to show window");

		ui->show(); 
	}
}

void Win::_common_init(bool widget)
{
	_main_panel.id(id_init);
	_main_panel.style(bi::none);

	active_callback = &Win::basic_submit;

	if(widget) //TextureWidget?
	{
		//These serve to translate UI codes into Tool 
		//codes.
		special_callback = win_widget_special;
		keyboard_callback = win_widget_keyboard;
		mouse_callback = win_widget_mouse;
		motion_callback = win_widget_motion;
		wheel_callback = win_widget_wheel;
	}
	else special_callback = &Win::basic_special;

	keyboard_callback = &Win::basic_keyboard;
}
Win::Win(utf8 title, Widget *widget):ui(&_main_panel)
{	
	ui::_init(title,0,-1,-1,0);

	_main_panel.space(6,6,6,6,6);

	Win::_common_init(widget!=nullptr);

	bool reset = !widget; //HACK: TextureWidget
	int swap;
	if(reset) 
	swap = glutGetWindow();
	{
		glutSetWindow(glut_window_id());
		glutext::glutSetWindowIcon(0);
		glutext::glutCloseFunc(win_close);
	}
	if(reset) 
	glutSetWindow(swap);
		
	//NOTE: Passing the mouse coordinates to _init
	//doesn't move the window back into the screen.
	//Even if it could the size is TBD.
	
	extern bool viewwin_menu_origin; 

	if(!viewwin_menu_origin) //Center?
	{
		reshape_callback = win_reshape;
	}
	else //Mouse?
	{
		viewwin_menu_origin = false; 

		reshape_callback = win_reshape2;
	}
}
Win::Win(int p, int sp):ui(&_main_panel)
{
	ui::_init(nullptr,sp,0,0,p);

	_main_panel.space(1,1,1,1,1);

	set_main_esc_window(glutGetWindow());

	Win::_common_init(false);
}

void Win::basic_submit(int id)
{
	switch(id)
	{
	case id_close:
	case id_cancel: 
	case id_ok:
	case id_yes:
	case id_no:

		if(modal()) return_on_close(id);
		
		//Close any help windows.
		//NOTE: Only double-nested F1 windows are not
		//under the main windows.
		event.close_ui_by_create_id(glut_window_id());

		close(); break;
	}
}

void EditWin::submit(control *c)
{
	int id = c->id(); switch(id)
	{
	case id_init:

		//NOTE: This was calibrated for AnimEditWin.
		//Assuming relatively short wrapped message.
		//NOTE: 10 forces wrapping, keeping span to
		//OK/Cancel size to prevent 1 char per line.
		msg.title().expand();

		assert(!swap);
		nav.expand();
		edit.expand();
		std::swap(swap,edit._ptr_val);

		//HACK: This fist call can validate the default string, 
		//but is also an opportunity to customize the edit box.
		if(valid)
		{
			refresh();

			glutSetWindow(glut_window_id());			

			valid(c);
			return_on_close(1); //ignore?

			nav.sync_live();

			c->repack();
		}

		return;
		
	case id_ok:

		if(valid)
		{
			valid(edit);
			if(id_ok!=return_on_close())
			{
				return_on_close(1); //ignore?

				return; //FIX ME: Beep?
			}
		}
		else if(edit.text().empty())
		{
			return; //FIX ME: Beep?
		}

		std::swap(swap,edit._ptr_val);
		edit.output_live();
		//break;

	case id_cancel: 

		return_on_close(id);
	}
	if(valid) 
	{
		valid(c);
	}
	else basic_submit(id);
}

static int win_state(int cm)
{	
	int bs = 0;

	if(cm&GLUT_ACTIVE_SHIFT) bs|=Tool::BS_Shift;
	if(cm&GLUT_ACTIVE_CTRL) bs|=Tool::BS_Ctrl;
	if(cm&GLUT_ACTIVE_ALT) bs|=Tool::BS_Alt;

	//Note: ModelViewport can track left/right/down if it needs 
	//to forward that information to Tool::mouseButtonMove, etc.
	return bs;
}

static int win_button(int bt)
{
	switch(bt)
	{
	default: return std::max(0,bt);

	case GLUT_LEFT_BUTTON: return Tool::BS_Left;
	case GLUT_MIDDLE_BUTTON: return Tool::BS_Middle;
	case GLUT_RIGHT_BUTTON: return Tool::BS_Right;
		
	case -GLUT_KEY_LEFT: return Tool::BS_Left;
	case -GLUT_KEY_UP: return Tool::BS_Up;
	case -GLUT_KEY_RIGHT: return Tool::BS_Right;
	case -GLUT_KEY_DOWN: return Tool::BS_Down;
	case -GLUT_KEY_PAGE_UP: return Tool::BS_Special|Tool::BS_Up;
	case -GLUT_KEY_PAGE_DOWN: return Tool::BS_Special|Tool::BS_Down;
	case -GLUT_KEY_HOME: return Tool::BS_Special|Tool::BS_Left;
	case -GLUT_KEY_END: return Tool::BS_Special|Tool::BS_Right;	
	}
}

bool ScrollWidget::mousePressEventUI(int bt, int bs, int x, int y)
{
	bool broadcast = bs&GLUT_ACTIVE_SHIFT;
	if(bt!=GLUT_MIDDLE_BUTTON&&~bs&GLUT_ACTIVE_CTRL)
	broadcast = false;

	getXY(x,y);
	if(broadcast||over(x,y)) 
	mousePressEvent(win_button(bt),win_state(bs),x,y);
	else return false; return !broadcast;
}
bool ScrollWidget::mouseReleaseEventUI(int bt, int bs, int x, int y)
{
	getXY(x,y);
	if(m_activeButton) 
	mouseReleaseEvent(win_button(bt),win_state(bs),x,y);
	else return false; return true;
}
bool ScrollWidget::mouseMoveEventUI(int bs, int x, int y)
{
	getXY(x,y);
	if(m_activeButton||over(x,y)||m_autoOverlay==2)
	mouseMoveEvent(win_state(bs),x,y);
	else return false; return true;
}
bool ScrollWidget::wheelEventUI(int wh, int bs, int x, int y)
{
	bool broadcast = bs&GLUT_ACTIVE_SHIFT;

	getXY(x,y);
	if(broadcast||over(x,y))
	wheelEvent(-wh,win_state(bs),x,y);
	else return false; return !broadcast;
}
bool ScrollWidget::keyPressEventUI(int bt, int bs, int x, int y)
{
	getXY(x,y);
	return keyPressEvent(win_button(bt),win_state(bs),x,y);
}
void ScrollWidget::setTimerUI(int ms, void(*f)(int), int val)
{
	glutTimerFunc(ms,f,val);
}
int ScrollWidget::getElapsedTimeUI()
{
	return glutGet(GLUT_ELAPSED_TIME); 
}
void ScrollWidget::initTexturesUI(int n, unsigned int textures[], char **xpm[])
{
	glGenTextures(n,textures);

	int il = glutext::glutCreateImageList(nullptr,1);
	for(int i=0;i<n;i++,xpm++)
	{
		int *q = *glutext::glutUpdateImageList(il+i,*xpm);
		if(!q){ assert(q); continue; }

		glBindTexture(GL_TEXTURE_2D,textures[i]);

		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);

		//https://github.com/zturtleman/mm3d/issues/85
		glPixelStorei(GL_UNPACK_ALIGNMENT,1);
		
		glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,q[0],q[1],0,q[2],q[3],(void*&)q[4]);
	}
	glutext::glutDestroyImageList(il);
}
void ScrollWidget::setCursorUI(int dir)
{
	switch(dir)
	{
	default: //-1
	
		//setCursor(QCursor(Qt::ArrowCursor));
		glutSetCursor(GLUT_CURSOR_LEFT_ARROW);
		break;

	case 0: //left 

		//setCursor(QCursor(Qt::SizeHorCursor));
		glutSetCursor(GLUT_CURSOR_LEFT_SIDE);
		break;

	case 45:

		//setCursor(QCursor(Qt::SizeFDiagCursor));
		glutSetCursor(GLUT_CURSOR_TOP_LEFT_CORNER);
		break;

	case 90: //up

		//setCursor(QCursor(Qt::SizeVerCursor));
		glutSetCursor(GLUT_CURSOR_TOP_SIDE);
		break;

	case 135:

		//setCursor(QCursor(Qt::SizeBDiagCursor));
		glutSetCursor(GLUT_CURSOR_TOP_RIGHT_CORNER);
		break;

	case 180: //right

		//setCursor(QCursor(Qt::SizeHorCursor));
		glutSetCursor(GLUT_CURSOR_RIGHT_SIDE);
		break;

	case 225: 

		//setCursor(QCursor(Qt::SizeFDiagCursor));
		glutSetCursor(GLUT_CURSOR_BOTTOM_RIGHT_CORNER);
		break;

	case 270: //down

		//setCursor(QCursor(Qt::SizeVerCursor));
		glutSetCursor(GLUT_CURSOR_BOTTOM_SIDE);
		break;

	case 315:

		//setCursor(QCursor(Qt::SizeBDiagCursor));
		glutSetCursor(GLUT_CURSOR_BOTTOM_LEFT_CORNER);
		break;

	case 360: //???

		//setCursor(QCursor(Qt::SizeAllCursor));
		glutSetCursor(glutext::GLUT_CURSOR_UP_DOWN_LEFT_RIGHT);
		break;
	}
}

struct MessageWin : Win //UNFINISHED
{	
	static bool enter(ui *w, int key, int m)
	{
		switch(tolower(key))
		{
		case '\r':
		
			event.spacebar_mouse_click();
			return false;

		case 'o': //HACK: Assuming English mnemonics.
		case 'k':
		case 'y': key = id_yes; break;
		case 'n': key = id_no; break;
		case 27: //Esc
		case 'c':
			if(~m&2) key = id_cancel; 
			else ((MessageWin*)w)->msg.key_in('c',m);
			break; 
		default: return true;
		}
		if(m&GLUT_ACTIVE_ALT)
		w->main->find(key)->activate();		
		else ((MessageWin*)w)->basic_submit(key);
		return false;
	}

	MessageWin(utf8 title, utf8 text, int bts, int def)
		:
	Win(title),
	msg(main,pics[pic_icon],text),
	ok_cancel(main)
	{	
		keyboard_callback = enter;

		main_panel()->space(0,0,0,0,0);
		ok_cancel.nav.space(20,10,10,8,10);
		ok_cancel.nav.calign();

		//Off-white border on top. It looks like
		//the Windows 10 message-box and menubar.
		msg.style(~0x08000000);

		if(~bts&id_ok&&~bts&id_yes) 
		ok_cancel.ok.set_hidden();
		if(~bts&id_cancel) 
		ok_cancel.cancel.set_hidden();
		if(bts&id_yes) 
		ok_cancel.ok.id(id_yes).name("Yes");
		if(bts&id_no) no.id(id_no).name("No");
		if(bts&id_no) 
		no.set_parent(ok_cancel.nav,ok_cancel.cancel);

		switch(def)
		{
		case id_ok: case id_yes: 			
		ok_cancel.ok.activate(); break;		
		case id_no: no.activate(); break;
		case id_cancel:
		ok_cancel.cancel.activate(); break;
		}
	}

	int h;
	message msg;
	ok_cancel_panel ok_cancel; button no;
};
int Win::InfoBox(utf8 a, utf8 b, int bts, int def)
{
	return MessageWin(a,b,bts,def).return_on_close();
}
int Win::ErrorBox(utf8 a, utf8 b, int bts, int def)
{
	return MessageWin(a,b,bts,def).return_on_close();
}
int Win::WarningBox(utf8 a, utf8 b, int bts, int def)
{
	return MessageWin(a,b,bts,def).return_on_close();
}
