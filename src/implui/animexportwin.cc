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

#include "viewwin.h"
#include "model.h"
#include "mview.h"
#include "misc.h"
#include "msg.h"
#include "filedatadest.h"

/*NOT SURE WHAT THIS IS FOR?
"The Export Animation Window allows you to save an animation as a series of jpeg or png files."
Note: wxWidgets has an animated GIF feature that might make this more useful for sharing work.
*/

struct AnimExportWin : Win
{
	void submit(int);

	AnimExportWin(Model *model, ViewPanel *vp)
		:
	Win("Export Animation"),
	model(model),vp(vp),
	nav(main),source(nav),duration(nav.inl),
	output(main),
	f1_ok_cancel(main)
	{
		duration.nav.expand(bottom);

		active_callback = &AnimExportWin::submit;

		submit(id_init);
	}

	Model *model; ViewPanel *vp;

	struct source_group
	{	
		source_group(node *frame)
			:
		nav(frame,"Source"),
		animation(nav,"Animation"),
		viewport(nav,"Viewport\t")
		{
			animation.sspace<left>({viewport});
			animation.expand();
			viewport.expand();
		}

		panel nav; 
		dropdown animation,viewport;
	};
	struct duration_group
	{
		duration_group(node *frame)
			:
		nav(frame,"Duration"),mult(nav)
		{
			mult.space(1);
			mult.add_item(0,"Seconds",seconds);
			mult.add_item(1,"Iterations",iterations);
			seconds.prev()->span({iterations.prev()});
		}

		panel nav; 
		multiple mult; textbox seconds,iterations;
	};
	struct output_group
	{
		output_group(node *frame)
			:
		nav(frame,"Output"),
		format_nav(nav),
		framerate(format_nav,"Frame Rate"),
		format(format_nav,"Format"),			
		browse_nav(nav),
		directory(browse_nav,"Directory"),
		browse(browse_nav,"...",id_browse)
		{
			format_nav.expand();
			format.ralign();
			browse_nav.expand().space(0);
			directory.expand();
			browse.drop(directory.drop()).span(0).ralign();
		}

		panel nav;
		row format_nav;
		textbox framerate;
		dropdown format;
		row browse_nav;
		textbox directory; button browse;
	};
	panel nav;
	source_group source;
	duration_group duration;
	output_group output;	
	f1_ok_cancel_panel f1_ok_cancel;
};
void AnimExportWin::submit(int id)
{
	//TODO: Try animated GIF path.
	static const utf8 formats[] =
	{
		"anim_0001.png","anim_1.png",
		"anim_0001.jpg","anim_1.jpg",
	};
	enum{ formatsN = sizeof(formats)/sizeof(*formats) };

	switch(id)
	{
	case id_init:
	{
		assert(model&&vp);

		for(int i=0;i<formatsN;i++) 
		output.format.add_item(i,formats[i]);

		int fmt = config.get("ui_animexport_format",0);
		if(fmt>0&&fmt<formatsN) output.format.select_id(fmt);

		double min = 0.0001;
		double fps = config.get("ui_animexport_framerate",25.0);
		output.framerate.edit(min,fps>=min?fps:25.0,DBL_MAX);

		double sec = config.get("ui_animexport_seconds",15.0);
		duration.seconds.edit(min,sec>=min?sec:15.0,DBL_MAX);

		double i = config.get("ui_animexport_iterations",1.0);
		duration.iterations.edit(min,i>=min?i:1.0,DBL_MAX);

		//i = config.get("ui_animexport_?",1);
		duration.mult.select_id(1);
	
		size_t scount = model->getAnimCount(Model::ANIMMODE_SKELETAL);
		size_t fcount = model->getAnimCount(Model::ANIMMODE_FRAME);
		bool labelAnims = scount&&fcount;

		std::string name;
		for(size_t i=0;i<scount;i++,name.clear())
		{
			if(labelAnims) 
			name = ::tr("Skeletal-","Skeletal Animation prefix");
			name+=model->getAnimName(Model::ANIMMODE_SKELETAL,i);
			source.animation.add_item((int)i,name);
		}
		for(size_t i=0;i<fcount;i++,name.clear())
		{
			if(labelAnims) 
			name = ::tr("Frame-","Frame Animation prefix");
			name+=model->getAnimName(Model::ANIMMODE_FRAME,i);
			source.animation.add_item((int)(scount+i),name);
		}

		const char *filename = model->getFilename();
				
		output.directory.set_text(config.get("ui_animexport_dir",""));
		
		int persp = -1;
		for(int i=0;i<vp->viewsN;i++)
		{
			ViewBar::ModelView *mv = vp->getModelView(i);
			
			//name = ::tr("Viewport %1-").arg(i+1);
			name = "Viewport 1"; 
			name.back()+=0xff&i;
			name.push_back('-');
			name+=mv->view.selection()->text();

			if(-1!=persp&&!mv->view)
			{
				persp = (int)i;
			}

			source.viewport.add_item((int)i,name);
		}

		if(-1!=persp) source.viewport.select_id(persp);

		break;
	}	
	case id_browse:
	{
		auto &d = output.directory;
		d.set_text(d.text().locate("Index","Select an output directory"));
		break;
	}
	case id_ok:
	{	
		if(!is_directory(output.directory))
		{
			return msg_warning(::tr("Output directory does not exist."));
		}

		auto swap = model->getAnimationMode();
		int swap2 = model->getCurrentAnimation();

		//https://gcc.gnu.org/bugzilla/show_bug.cgi?id=92338
		ModelViewport &mvp = vp->ports[(int)source.viewport];
						 
		auto mode = Model::ANIMMODE_SKELETAL;
		int a = source.animation;
		if((size_t)a>=model->getAnimCount(mode))
		{
			a-=(int)model->getAnimCount(mode);
			mode = Model::ANIMMODE_FRAME;
		}

		double fps = model->getAnimFPS(mode,a);
		double spf = 1/fps;
		double tm, dur = 0;
		double outfps = output.framerate;
		double interval = 1/outfps;

		if(!duration.mult)
		{
			dur = duration.seconds;
		}
		else //???
		{
			dur = duration.iterations;		
			dur*=spf*model->getAnimFrameCount(mode,a);
		}

		FileBox file;
		int frameN = dur/interval; if(frameN>60) //NEW
		{
			file.format("Save %d images?",frameN);
			if(id_yes!=WarningBox("Confirmation",file.c_str(),id_yes|id_no,id_no))
			return;
		}
			
			////POINT-OF-NO-RETURN///
			////POINT-OF-NO-RETURN///
			////POINT-OF-NO-RETURN///


		config.set("ui_animexport_dir",(utf8)output.directory);
		config.set("ui_animexport_format",(int)output.format);
		config.set("ui_animexport_framerate",(double)output.framerate);
		config.set("ui_animexport_seconds",(double)duration.seconds);
		config.set("ui_animexport_iterations",(double)duration.iterations);

		bool enable = 
		model->setUndoEnabled(false);
		model->setCurrentAnimation(mode,a);
				
		file = output.directory.text();
		size_t saveFile = file.size();
		//https://gcc.gnu.org/bugzilla/show_bug.cgi?id=92338
		const char *saveFormat = strchr(formats[(int)output.format],'.');

		int x,y,w,h; mvp.getGeometry(x,y,w,h); 
		int il = glutext::glutCreateImageList();
		int*pb =*glutext::glutLoadImageList(il,w,h,false);

		//Select main window's OpenGL context.
		glutSetWindow(vp->model.glut_window_id);
		glPixelStorei(GL_PACK_ALIGNMENT,1); //glReadPixels

		int gif = !file.render_gif; //IMPLEMENT ME
		
		int frameNum = 0; for(tm=0;tm<=dur;tm+=interval)
		{
			frameNum++;
			
			model->setCurrentAnimationTime(tm);			

			//TODO: Try animated GIF (single file) path.
			file.erase(saveFile);
			file.append("/anim_");
			char num[33];
			sprintf(num,saveFormat[-2]=='0'?"%04d":"%d",frameNum);
			file.append(num);
			file.append(saveFormat);
			file.push_back('\0');
			size_t buf = file.size();
			
			mvp.render();
			if(!pb) assert(pb);
			else glReadPixels(x,y,pb[0],pb[1],pb[2],pb[3],(void*&)pb[4]);

			if(!file.render(il,saveFormat+1,gif)
			||!FileDataDest(file.c_str()).writeBytes(&file[buf],file.size()-buf))
			{
				msg_error("%s\n%s",::tr("Could not write file: "),file.c_str());
				break;
			}
		}		

		glutext::glutDestroyImageList(il);

		model->setCurrentAnimation(swap,swap2); //model->setNoAnimation();
		model->setUndoEnabled(enable);

		if(tm<=dur) return; break;
	}}

	basic_submit(id);
}

extern void animexportwin(Model *model, ViewPanel *vp)
{
	if(!model->getAnimCount(Model::ANIMMODE_SKELETAL)
	 &&!model->getAnimCount(Model::ANIMMODE_FRAME)) //FIX ME
	{
		msg_error(::tr("This model does not have any animations"));
	}
	else AnimExportWin(model,vp).return_on_close();
}
