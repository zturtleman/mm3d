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
#include "win.h"

#include "animwin.h"
#include "viewwin.h"

#include "model.h"

#include "log.h"
#include "msg.h"
 
struct NewAnim : Win
{		
	void submit(int id)
	{
		if(id!=id_ok||!name.text().empty())
		basic_submit(id);
	}		  

	NewAnim(std::string *lv, int *lv2)
		:
	Win("New Animation"),
	name(main,"Name",lv,id_name),
	type(main,"Animation Type",lv2),
	ok_cancel(main)
	{		
		name.expand();
		type.row_pack();
		type.add_item("Skeletal");
		type.add_item("Frame");
		type.style(bi::etched).expand();

		active_callback = &NewAnim::submit;
	}

	textbox name;
	multiple type;
	ok_cancel_panel ok_cancel;
};

struct AnimWin::Impl
{	
	Impl(AnimWin &win)
		:
	win(win),
	shelf1(win.shelf1),shelf2(win.shelf2),
	sidebar(win.model.sidebar.anim_panel),
	new_animation(sidebar.new_animation),
	sep_animation(sidebar.sep_animation),
	model(win.model),
	mode(),anim(),frame(),
	playing(),autoplay()
	{
		//See AnimPanel::refresh_list.
		//It's easier to let the animation window manage this since
		//it's opaque.
		int swap = sidebar.animation;
		sidebar.animation.clear();
		sidebar.animation.reference(shelf1.animation);				
		sidebar.refresh_list();
		sidebar.animation.select_id(swap);
		shelf1.animation.select_id(-1);
	}

	AnimWin &win;
	shelf1_group &shelf1;
	shelf2_group &shelf2;
	SideBar::AnimPanel &sidebar;
	const int &new_animation; 
	const int &sep_animation; 

	Model *model;
	Model::AnimationModeE mode;
	unsigned anim;
	unsigned frame;	
	bool playing,autoplay;
	int step_t0;
	int step_ms;
	
	void open2(bool undo);
	void refresh_item();
	void refresh_undo();

	void close();

	void play(int=0);
	void pause(){ play(id_animate_pause); }
	bool step();

	void anim_selected(int, bool local=false);
	void anim_deleted(int);

	void frames_edited(int);	
	void set_frame(int);
 
	bool copy(bool selected);
	void paste();


		//UTILITIES//

		
	Model::AnimationModeE item_mode(int i)
	{
		if(i<0) return Model::ANIMMODE_NONE;
		if(i>=sep_animation&&i<new_animation)
		return Model::ANIMMODE_FRAME;
		return Model::ANIMMODE_SKELETAL;
	}
	unsigned item_anim(int i)
	{
		if(i>=sep_animation) i-=sep_animation;
		return i;
	}
	int anim_item()
	{
		if(!mode) return -1;
		if(mode==Model::ANIMMODE_FRAME)
		return (unsigned)sep_animation+anim; return anim;
	}	

		//COPY/PASTE//


	struct KeyframeCopy
	{
		unsigned joint; 
		
		bool isRotation; //???

		double x,y,z;
	};
	struct FrameCopy
	{
		unsigned vertex; double x,y,z;
	};
	struct FramePointCopy
	{
		unsigned point; double x,y,z,rx,ry,rz;
	};	
	std::vector<KeyframeCopy> copy1;
	std::vector<FrameCopy> copy2;
	std::vector<FramePointCopy> copy3;
};

/*UNIMPLEMENTED
static int animwin_tick_interval(int val)
{
	//Worth it? There's a big difference in 5000 and 1000??

	if(val>=25000) return 5000; if(val>=10000) return 1000;
	if(val>=2500) return 500;   if(val>=1000) return 100;
	if(val>=250) return 50;     if(val>=100) return 10;
	if(val>=25) return 5;       return 1;
}*/

extern void animwin_enable_menu(int menu)
{
	void *off,*on = 0; 
	if(menu<=0) menu = -menu;
	else on = glutext::GLUT_MENU_ENABLE;
	off = !on?glutext::GLUT_MENU_ENABLE:0;
		
	if(menu) glutSetMenu(menu);

	//NEW: No good reason to ever turn these two off.
	//glutext::glutMenuEnable(id_animate_settings,off);
	//glutext::glutMenuEnable(id_animate_render,off);
	glutext::glutMenuEnable(id_animate_copy,on); 
	glutext::glutMenuEnable(id_animate_paste,0); // Disabled until copy occurs
	glutext::glutMenuEnable(id_animate_copy_selection,on); 
	//glutext::glutMenuEnable(id_animate_paste_selection,0); // Disabled until copy occurs
	glutext::glutMenuEnable(id_animate_rotate,on);
	glutext::glutMenuEnable(id_animate_translate,on);
	glutext::glutMenuEnable(id_animate_clear,on); 
	glutext::glutMenuEnable(id_animate_play,on);
}

void AnimWin::open(bool undo)
{	
	//animwin_enable_menu(menu);

	impl->open2(undo);

	//HACK: MainWin::open_animation_window opens up
	//the window manually, so it can operate in the 
	//background to support the sidebar and toolbar.
	//It's like this because animwin.cc has so much
	//code.
	//show(); 
}
void AnimWin::Impl::open2(bool undo)
{	
	bool swapping = false; //YUCK

	if(model!=win.model)
	{
		swapping = true; //NEW

		model = win.model;
	}
	else if(mode&&!undo) //Sidebar?
	{			
		anim_selected(sidebar.animation);
		return;
	}

	if(undo&&!swapping)
	{
		mode = model->getAnimationMode();
		anim = model->getCurrentAnimation();
		frame = model->getCurrentAnimationFrame();
	}
	else
	{
		int id = sidebar.animation;		 
		if(id==new_animation)
		{
			//RECURSIVE
			return anim_selected(id);
		}
		mode = item_mode(id);
		anim = item_anim(id);
		frame = 0;

		model->setCurrentAnimation(mode,anim);
		model->setCurrentAnimationFrame(frame);
	}

	if(mode) animwin_enable_menu(win.menu);

	shelf1.animation.select_id(anim_item());
	sidebar.animation.select_id(anim_item());
	refresh_item();

	if(!undo&&mode) 
	model->operationComplete(::tr("Start animation mode","operation complete"));

	if(!undo){ autoplay = true; play(); } //NEW
}

void AnimWin::Impl::anim_deleted(int item)
{
	if(id_ok!=WarningBox
	(::tr("Delete Animation?","window title"),
	 ::tr("Are you sure you want to delete this animation?"),id_ok|id_cancel))
	return;

	if(playing) pause(); //NEW
		
	//FIX ME
	//NOTE: removeSkelAnim/deleteAnimation select the
	//previous animation always. Seems like bad policy.
	//item = std::min(0,item-1);
	if(item&&item>=new_animation-1)
	{
		item--;
	}

	auto swap = mode;
	auto swap2 = anim;
	mode = item_mode(item);
	anim = item_anim(item);
	model->deleteAnimation(swap,swap2);
	model->setCurrentAnimation(mode,anim);
	open2(true);

	model->operationComplete(::tr("Delete Animation","Delete animation,operation complete"));
}
void AnimWin::Impl::anim_selected(int item, bool local)
{
	//log_debug("anim name selected: %d\n",item); //???

	int was = shelf1.animation;

	if(item==was)
	{
		if(!local)
		{
			int cmp = sidebar.frame;
			if(cmp-1!=shelf2.timeline.int_val()) 
			set_frame(cmp-1);
			return;
		}		
	}
	else shelf1.animation.select_id(item);
	
	if(item!=sidebar.animation.int_val())
	{
		if(item<new_animation)
		sidebar.animation.select_id(item);
	}

	if(item<0)
	{
		assert(item==-1);

		if(mode) close();

		return;
	}
	else if(was<0&&item>=0)
	{
		animwin_enable_menu(win.menu);
	}

	if(item>=new_animation)
	{
		std::string name;
		int type = config.get("ui_new_anim_type",0);
		if(!event.wheel_event
		&&id_ok==NewAnim(&name,&type).return_on_close())
		{
			pause();

			config.set("ui_new_anim_type",type);

			mode = type?Model::ANIMMODE_FRAME:Model::ANIMMODE_SKELETAL;

			anim = model->addAnimation(mode,name.c_str());
			model->setCurrentAnimation(mode,anim);

			set_frame(0);			
			model->operationComplete(::tr("New Animation","operation complete"));			
			open2(false);
		}
		else
		{
			shelf1.animation.select_id(anim_item());
			sidebar.animation.select_id(anim_item());
		}
	}
	else
	{
		mode = item_mode(item);
		anim = item_anim(item);
	
		model->setCurrentAnimation(mode,anim);

		//FIX ME
		//NEW: Note setCurrentAnimation seems to ignore this unless the animation
		//mode was previously ANIMMODE_NONE?!
		//if(!undo) 
		model->operationComplete(::tr("Set current animation","operation complete"));
	
		frame = 0; refresh_item();

		if(0) //Play the animation one time.
		{
			// Re-initialize time interval based on new anim's FPS.
			if(playing) play();
		}
		else //if(!undo)
		{
			autoplay = !playing; //NEW

			play(); //2019
		}
	}	
}

void AnimWin::Impl::frames_edited(int n)
{	
	assert(n==shelf1.frames.int_val());

	int nn = std::max(0,n-1);
	shelf2.timeline.set_range(0,nn);
	win.model.views.timeline.set_range(0,nn);
	sidebar.frame.limit(n?1:0,n);

	bool op = model->setAnimFrameCount(mode,anim,n);

	set_frame(model->getCurrentAnimationFrame());

	if(op) model->operationComplete(::tr("Change Frame Count","operation complete"));

	//https://github.com/zturtleman/mm3d/issues/90
	//DecalManager::getInstance()->modelAnimate(model); //???
	win.model.views.update3dView();
}

void AnimWin::Impl::set_frame(int i)
{	
	if(!model->setCurrentAnimationFrame(i))
	{
		i = 0; shelf2.timeline.name(::tr("Frame: \tn/a"));
	}
	else shelf2.timeline.name().format("%s\t%03d",::tr("Frame: "),i+1);

	sidebar.frame.set_int_val(i+1);
	win.model.views.timeline.set_int_val(i);
	
	//shelf2.timeline.name().push_back('\t');

	frame = i; shelf2.timeline.set_int_val(i); //NEW!

	//https://github.com/zturtleman/mm3d/issues/90
	//DecalManager::getInstance()->modelUpdated(model); //???
	model->updateObservers();
}

bool AnimWin::Impl::copy(bool selected)
{
	copy1.clear(); copy2.clear(); copy3.clear();

	if(mode==Model::ANIMMODE_SKELETAL)
	{	
		KeyframeCopy cp;

		size_t numJoints = model->getBoneJointCount();
		copy1.reserve(numJoints);

		for(int pass=0;pass<=1;pass++) 
		{
			//??? What's with this filter?
			bool rot = pass&1; 

			for(size_t j=0;j<numJoints;j++)
			if(!selected||model->isBoneJointSelected(j))			
			if(model->getSkelAnimKeyframe(anim,frame,j,rot,cp.x,cp.y,cp.z))
			{
				cp.joint = j;
				cp.isRotation = rot; copy1.push_back(cp);
			}			
		}
	}
	else if(mode==Model::ANIMMODE_FRAME)
	{
		FrameCopy cp;

		size_t numVertices = model->getVertexCount();
		copy2.reserve(numVertices);

		for(size_t v=0;v<numVertices;v++)		
		if(!selected||model->isVertexSelected(v))		
		if(model->getFrameAnimVertexCoords(anim,frame,v,cp.x,cp.y,cp.z))
		{
			cp.vertex = v; copy2.push_back(cp);
		}		

		FramePointCopy cpt;

		size_t numPoints = model->getPointCount();
		copy3.reserve(numPoints);

		for(size_t v=0;v<numPoints;v++)
		if(!selected||model->isPointSelected(v))		
		if(model->getFrameAnimPointCoords(anim,frame,v,cpt.x,cpt.y,cpt.z)
		 &&model->getFrameAnimPointRotation(anim,frame,v,cpt.rx,cpt.ry,cpt.rz))
		{
			cpt.point = v; copy3.push_back(cpt);
		}
	}

	return !copy1.empty()||!copy2.empty()||!copy3.empty();
}
void AnimWin::Impl::paste()
{
	if(mode==Model::ANIMMODE_FRAME)
	{
		if(copy2.empty()&&copy3.empty())
		{
			return msg_error(::tr("No frame animation data to paste"));			
		}

		for(FrameCopy*p=copy2.data(),*d=p+copy2.size();p<d;p++)
		{
			model->setFrameAnimVertexCoords(anim,frame,p->vertex,p->x,p->y,p->z);
		}

		for(FramePointCopy*p=copy3.data(),*d=p+copy3.size();p<d;p++)
		{
			model->setFrameAnimPointCoords(anim,frame,p->point,p->x,p->y,p->z);
			model->setFrameAnimPointRotation(anim,frame,p->point,p->rx,p->ry,p->rz);
		}

		model->operationComplete(::tr("Paste frame","paste frame animation position,operation complete"));	
	}
	else if(mode==Model::ANIMMODE_SKELETAL)
	{
		if(copy1.empty())
		{
			return msg_error(::tr("No skeletal animation data to paste"));
		}

		for(KeyframeCopy*p=copy1.data(),*d=p+copy1.size();p<d;p++)
		{
			model->setSkelAnimKeyframe(anim,frame,p->joint,p->isRotation,p->x,p->y,p->z);
		}
		model->operationComplete(::tr("Paste keyframe","Paste keyframe animation data complete"));
	}
	else return;
		
	//REMOVE ME
	// Force refresh of joints
	model->setCurrentAnimationFrame(frame);
	//https://github.com/zturtleman/mm3d/issues/90
	//DecalManager::getInstance()->modelUpdated(model); //???
	model->updateObservers();
}

static void animwin_step(int id) //TEMPORARY
{
	auto w = (AnimWin*)Widgets95::e::find_ui_by_window_id(id);
	if(w&&w->impl->step())
	glutTimerFunc(w->impl->step_ms,animwin_step,id);
}
void AnimWin::Impl::play(int id)
{
	//TODO: Beep?
	//Leaving buttons clickable.
	if(!mode) return;

	bool stop = id==id_animate_pause;

	//shelf2.play.enable(stop); 
	//shelf2.stop.enable(!stop);
	shelf1.frames.enable(stop);
	shelf2.timeline.enable(stop);

	if(playing=!stop)
	{
		//TODO? Add 1/2 speed, etc. buttons.
		double step;
		step = 1/model->getAnimFPS(mode,anim);
		/*Min or max?
		const double shortInterval = 1.0 / 20.0;
	    if ( m_timeInterval > shortInterval )
		 m_timeInterval = shortInterval;
		*/
		step = std::max(1/60.0,step); //min?

		step_t0 = glutGet(GLUT_ELAPSED_TIME);
		step_ms = (int)(step*1000);	

		//TEMPORARY
		glutTimerFunc(step_ms,animwin_step,win.glut_window_id());

		log_debug("starting %s animation,update every %.03f seconds\n",
		(mode==Model::ANIMMODE_SKELETAL?"skeletal":"frame"),step);
	}
	else
	{
		autoplay = false;

		model->setCurrentAnimationFrame((int)shelf2.timeline);

		//https://github.com/zturtleman/mm3d/issues/90
		//DecalManager::getInstance()->modelUpdated(model); //???
		model->updateObservers();
	}
}
bool AnimWin::Impl::step()
{
	if(!playing) return false;

	int t = glutGet(GLUT_ELAPSED_TIME)-step_t0;

	if(!model->setCurrentAnimationTime(t/1000.0,autoplay?0:-1))
	{		
		pause();
	}

	//https://github.com/zturtleman/mm3d/issues/90
	//DecalManager::getInstance()->modelAnimate(model); //???
	win.model.views.update3dView();

	return playing;
}

void AnimWin::Impl::refresh_item()
{
	log_debug("refresh anim window page\n"); //???

	bool loop = false;
	double fps = 0;
	int frames = 0;

	if(!new_animation
	||-1==shelf1.animation.int_val()) //NEW
	{	
		win.disable();
		shelf1.animation.enable();		

		//Leave pressable.
		shelf2.play.enable();
		shelf2.stop.enable();

		assert(!frame);
		frame = 0;

		shelf1.fps.limit();
		shelf1.frames.limit();
	}
	else
	{
		shelf1.fps.limit(1,120); //???
		shelf1.frames.limit(1,INT_MAX);

		shelf1.del.enable();
		shelf1.fps.enable();
		shelf1.loop.enable();
		shelf1.frames.enable(!playing);
		//shelf2.play.enable(!playing);
		//shelf2.stop.enable(playing);
		shelf2.timeline.enable(!playing);

		fps = model->getAnimFPS(mode,anim);
		loop = model->getAnimLooping(mode,anim);
		frames = model->getAnimFrameCount(mode,anim);
	}

	shelf1.fps.set_float_val(fps);
	shelf1.loop.set(loop);
	shelf1.frames.set_int_val(frames);	
	int nn = std::max(0,frames-1);
	shelf2.timeline.set_range(0,nn);
	sidebar.frame.limit(frames?1:0,frames);
	win.model.views.timeline.set_range(0,nn);
	//IMPLEMENT ME? (QSlider API)
	//shelf2.timeline.setTickInterval(animwin_tick_interval(frames));

	set_frame(frame);
}

void AnimWin::Impl::refresh_undo()
{
	if(!model->getAnimationMode()) //???
	{
		return close();
	}
	
	if(Model::ANIMMODE_FRAME==model->getAnimationMode())
	{
		mode = Model::ANIMMODE_FRAME;
	}
	else mode = Model::ANIMMODE_SKELETAL; 

	anim = model->getCurrentAnimation();
	frame = model->getCurrentAnimationFrame();

	shelf1.animation.select_id(anim_item());
	sidebar.animation.select_id(anim_item());

	//UNDOCUMENTED ISN'T THIS REDUNDANT???
	model->setCurrentAnimation(mode,anim);
	model->setCurrentAnimationFrame(frame);

	refresh_item();
}

void AnimWin::Impl::close()
{
	if(model->getAnimationMode())
	{
		model->setNoAnimation();
		model->operationComplete(::tr("End animation mode","operation complete"));
	}
		
	//NEW: Keeping open.
	//NOTE: If modelChanged is implemented this is uncalled for.
	mode = Model::ANIMMODE_NONE;
	anim = 0;
	frame = 0;
	shelf1.animation.select_id(-1);
	sidebar.animation.select_id(-1);
	refresh_item();
		
	//emit animWindowClosed();
	//emit(this,animWindowClosed);
	{	
		//model->setNoAnimation(); //Done above.

//		win.hide(); //Or close? Should remember animation/frame.
		
		//views.modelUpdatedEvent(); //Probably unnecessary.

		//editEnableEvent(); //UNUSED?

		animwin_enable_menu(-win.menu);	
	}
}

AnimWin::~AnimWin()
{
	log_debug("Destroying AnimWin()\n"); //???

	delete impl;	
}
void AnimWin::submit(int id)
{
	switch(id)
	{
	case id_init:
		
		log_debug("AnimWidget constructor\n"); //???
		
		shelf1.fps.edit(0.0);
		shelf1.frames.edit(0);
		//TODO: CONFIRM LARGE VALUES
		shelf1.frames.limit(0,INT_MAX).compact();
		shelf2.play.span(60).picture(pics[pic_play]);
		shelf2.stop.span(60).picture(pics[pic_stop]);
		//IMPLEMENT ME? (QSlider API)
		shelf2.timeline.style(bar::sunken|bar::tickmark|behind);			
		
		//Line up Delete button with scrollbar.
		shelf2.nav.pack();
		shelf1.animation.lock(shelf2.timeline.active_area<0>()-12,false);

		//Make space equal to that above media buttons.
		shelf2.timeline.space<top>(3).drop()+=2;		

		assert(!impl);
		impl = new Impl(*this);
		open(false);
		
		break;

	case id_animate_copy:
	case id_animate_copy_selection: 

		glutSetMenu(menu);
		{
			void *l = 0; 
			if(impl->copy(id==id_animate_copy_selection))		
			l = glutext::GLUT_MENU_ENABLE;
			glutext::glutMenuEnable(id_animate_paste,l);
			//glutext::glutMenuEnable(id_animate_paste_selection,0);
		}
		break;

	case id_animate_paste:
	//case id_animate_paste_selection: // Same logic for both (???)

		impl->paste();
		break;

	case id_animate_clear: //clearFrame
		
		model->clearAnimFrame(impl->mode,impl->anim,impl->frame);
		model->operationComplete(::tr("Clear frame","Remove animation data from frame,operation complete"));
		break;	

	case id_edit_undo: //REMOVE ME
		
		log_debug("anim undo request\n"); //???
		model->undo(); impl->refresh_undo();
		break;

	case id_edit_redo: //REMOVE ME
		
		log_debug("anim redo request\n"); //???
		model->redo(); impl->refresh_undo();
		break;
			
	case id_item:

		impl->anim_selected(shelf1.animation,true);
		break;

	case id_delete:

		impl->anim_deleted(shelf1.del);
		break;		
		
	case id_anim_fps:

		log_debug("changing FPS\n"); //???
		model->setAnimFPS(impl->mode,impl->anim,shelf1.fps);
		model->operationComplete(::tr("Set FPS","Frames per second,operation complete"));
		break;

	case id_anim_loop:

		log_debug("toggling loop\n"); //???
		model->setAnimLooping(impl->mode,impl->anim,shelf1.loop);
		model->operationComplete(::tr("Set Looping","Change whether animation loops operation complete"));
		//WHAT'S THIS DOING HERE??? (DISABLING)
		//model->setCurrentAnimationFrame((int)shelf2.timeline);		
		break;
	
	case id_animate_play: case id_animate_pause:

		impl->play(id);
		break;		

	case id_anim_frames:

		impl->frames_edited(shelf1.frames);
		break;

	case id_bar:

		impl->set_frame(shelf2.timeline);
		break;

	case id_ok: case id_close:

		event.close_ui_by_create_id(); //Help?

		hide(); return;
	}
}
