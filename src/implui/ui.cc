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

#ifdef WIN32
#pragma comment(lib,"shlwapi.lib")
#pragma comment(lib,"opengl32.lib")
#pragma comment(lib,"glu32.lib")  
#endif

#ifdef WIN32
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

#include "win.h"
#include "viewwin.h"
#include "model.h"
#include "sysconf.h"
#include "log.h"
#include "mm3dport.h"
#include "misc.h"
#include "msg.h"
#include "cmdline.h"

#include "texmgr.h"
#include "datasource.h"

static void ui_info(const char *str)
{
	Win::InfoBox("MM3D",str); //"Maverick Model 3D"
}
static void ui_warning(const char *str)
{
	Win::WarningBox("MM3D",str); //"Maverick Model 3D"
}
static void ui_error(const char *str)
{
	Win::ErrorBox("MM3D",str); //"Maverick Model 3D"
}
static char ui_info_common(int type(utf8,utf8,int,int), utf8 str, utf8 opts)
{
	int def = 0, bt[3] = {};
	for(int i=0;i<3&&opts[i];i++)
	{
		int o = toupper(opts[i]);
		if(o==opts[i]) 
		def = bt[i]; switch(o)
		{
		case 'Y': bt[i] = id_yes; break;
		case 'N': bt[i] = id_no; break;
		case 'C': bt[i] = id_cancel; break;
		case 'O': bt[i] = id_ok; break;
		//case 'A': bt[i] = QMessageBox::Abort; break; //UNUSED
		//case 'R': bt[i] = QMessageBox::Retry; break; //UNUSED
		//case 'I': bt[i] = QMessageBox::Ignore; break; //UNUSED
		default: assert(0); //NEW
		}
	}
	switch(type("MM3D",str,bt[0]|bt[1]|bt[2],def)) //"Maverick Model 3D"
	{
	case id_ok: return 'O';
	case id_yes: return 'Y';
	case id_no: return 'N';
	default: 
	case id_cancel: return 'C';
	}
}
static char ui_info_prompt(const char *str, const char *opts)
{
	return ui_info_common(Win::InfoBox,str,opts);
}
static char ui_warning_prompt(const char *str, const char *opts)
{
	return ui_info_common(Win::WarningBox,str,opts);
} 
static char ui_error_prompt(const char *str, const char *opts)
{
	return ui_info_common(Win::ErrorBox,str,opts);
}

const char *ui_translate(const char *ctxt, const char *msg) //transimp.h
{
	return msg;
}

//#include "pixmap/mm3dlogo-32x32.xpm"
#include "pixmap/mm3dlogo-32x32.h"
#include "pixmap/play.xpm"
#include "pixmap/stop.xpm"
#include "pixmap/zoomin.xpm"
#include "pixmap/zoomout.xpm"

int pics[pic_N] = {}; //extern
int ui_prep(int &argc, char *argv[]) //extern
{
	Widgets95::glut::set_wxWidgets_enabled();
	glutInit(&argc,argv);

	#define _(x) pics[pic_##x] = \
	glutext::glutCreateImageList((char**)x##_xpm);
	auto &png = mm3dlogo_32x32_png;
	const void *icon_xpm[] = {"PNG",png,png+sizeof(png)};
	_(icon)_(play)_(stop)_(zoomin)_(zoomout)
	#undef _

	//s_app = new ModelApp(argc,argv);

	/*
	utf8 loc = mlocale_get().c_str();
	if(!*loc)
	{
		loc = QLocale::system().name();
	}

	// General Qt translations
	s_qtXlat = new QTranslator(0);
	QString qtLocale = QString("qt_")+loc;
	ui_load_translation_file(s_qtXlat,qtLocale);
	s_app->installTranslator(s_qtXlat);

	// MM3D translations
	s_mm3dXlat = new QTranslator(0);
	QString mm3dLocale = QString("mm3d_")+loc;
	ui_load_translation_file(s_mm3dXlat,mm3dLocale);
	s_app->installTranslator(s_mm3dXlat);
	*/

	return 0;
}

extern int ui_init(int &argc, char *argv[])
{
	int rval = 0;

	if(cmdline_runcommand)
	{
		rval = cmdline_command();
	}
	if(cmdline_runui)
	{
		/*if(!ui_has_gl_support())
		{
			return -1;
		}*/
		
		//msgqt.cc
		//extern void init_msgqt();
		//init_msgqt();
		{
			msg_register(ui_info,ui_warning,ui_error);
			msg_register_prompt(ui_info_prompt,ui_warning_prompt,ui_error_prompt);
		}

		bool opened = false;
		
		if(int openCount=cmdline_getOpenModelCount())
		{
			for(int i=0;i<openCount;i++)
			{
				new MainWin(cmdline_getOpenModel(i));
				opened = true;
			}

			cmdline_clearOpenModelList();
		}
		else if(argc>1)
		{
			char *pwd = PORT_get_current_dir_name();

			for(int i=1;i<argc;i++)
			if(MainWin::open(normalizePath(argv[i],pwd).c_str()))
			{
				//openCount++;
				opened = true;
			}

			free(pwd); //???
		}

		if(!opened) new MainWin;
		
		//rval = s_app->exec();
		glutMainLoop();
	}

	//delete s_mm3dXlat;
	//delete s_qtXlat;
	//delete s_app;

	return rval;
}

//This doesn't properly belong here, but it had previously included
//a Qt loader. Maybe to ui.cc?
struct StdTexFilter : TextureFilter
{
	virtual const char *getReadTypes()
	{
		//XBM is untested.
		//SVG may be added.
		//Qt also supported ICNS WBMP SVG WEBP in addition to most of these, save IFF and ANI.
		return "ANI BMP CUR GIF ICO IFF JPEG JPG PBM PCX PGM PNG PNM PPM TGA TIF TIFF XBM XPM";
	}

	virtual Texture::ErrorE readData(Texture &t, DataSource &src, const char *format)
	{
		Texture::ErrorE e = Texture::ERROR_UNSUPPORTED_OPERATION;		

		if(!isalpha(*format)) return e; //Must protect XPM mode.

		//TODO: Avoid copy if MemDataSource? 
		//TODO: Unbounded/unrewindable stream?
		size_t sz = src.getFileSize(); 
		char *buf = new char[sz], *p[] = {(char*)format,buf,buf+sz};
		if(!src.readBytes(buf,sz))
		{
			e = Texture::ERROR_FILE_READ;
		}
		else if(int i=glutext::glutCreateImageList(p))
		{
			int *q = *glutext::glutUpdateImageList(i);

			int s = q[2]==0x1908?4:3; //GL_RGBA?
			
			auto pixels = (uint8_t*&)q[4];
			t.m_data.assign(pixels,pixels+q[0]*q[1]*s);
			t.m_width = q[0]; 
			t.m_height = q[1];			
			t.m_format = s==4?t.FORMAT_RGBA:t.FORMAT_RGB;
				
			glutext::glutDestroyImageList(i);

			e = Texture::ERROR_NONE;
		}
		delete[] buf; return e;
	}
};
TextureFilter *ui_texfilter(){ return new StdTexFilter; }
