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


#include "config.h"
#include "pluginmgr.h"
#include "viewwin.h"
#include "viewpanel.h"
#include "model.h"
#include "tool.h"
#include "toolbox.h"
#include "cmdmgr.h"
#include "viewportsettings.h"
#include "groupwin.h"
#include "texwin.h"
#include "texturecoord.h"
#include "painttexturewin.h"
#include "log.h"
#include "decalmgr.h"
#include "msg.h"
#include "statusbar.h"
#include "modelstatus.h"
#include "filtermgr.h"
#include "misc.h"
#include "helpwin.h"
#include "licensewin.h"
#include "aboutwin.h"
#include "animsetwin.h"
#include "animexportwin.h"
#include "animwin.h"
#include "animwidget.h"
#include "metawin.h"
#include "3dmprefs.h"
#include "stdcmds.h"
#include "pluginwin.h"
#include "backgroundwin.h"
#include "mergewin.h"
#include "misc.h"
#include "version.h"
#include "texmgr.h"
#include "sysconf.h"
#include "contextpanel.h"
#include "boolpanel.h"
#include "projectionwin.h"
#include "transformwin.h"

#include "keycfg.h"

#include "qtmain.h"

#include <QtCore/QDataStream>
#include <QtCore/QFile>
#include <QtCore/QTimer>
#include <QtGui/QActionGroup>
#include <QtGui/QApplication>
#include <QtGui/QCloseEvent>
#include <QtGui/QContextMenuEvent>
#include <QtGui/QDockWidget>
#include <QtGui/QFileDialog>
#include <QtGui/QIcon>
#include <QtGui/QLayout>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QMessageBox>
#include <QtGui/QPixmap>
#include <QtGui/QResizeEvent>
#include <QtGui/QShortcut>
#include <QtGui/QToolBar>
#include <QtGui/QToolButton>
#include <QtGui/QToolTip>
#include <QtGui/QVBoxLayout>

#include "errorobj.h"

#include "luascript.h"
#include "luaif.h"

#include "pixmap/mm3dlogo-32x32.xpm"


#include <stdio.h>
#include <list>
#include <string>

using std::list;
using std::string;

const char DOCK_FILENAME[] = "dock.dat";
const int DOCK_VERSION = 1;

//using namespace QIconSet;

typedef QAction * QActionPtr;
typedef ::Tool * ToolPtr;

typedef std::list< ViewWindow *> ViewWindowList;
static ViewWindowList _winList;

static bool _shuttingDown = false;

bool ViewWindow::closeAllWindows()
{
   bool noPrompt = true;
   bool doSave   = false;

   std::list<ViewWindow *>::iterator it;

   for ( it = _winList.begin(); noPrompt == true && it != _winList.end(); it++ )
   {
      noPrompt = (*it)->getSaved();
   }

#ifndef CODE_DEBUG
   if ( noPrompt == false )
   {
       char response = msg_warning_prompt( (const char *) tr("Some models are unsaved.  Save before exiting?").toUtf8() );

       if ( response == 'C' )
       {
          return false;
       }

       if ( response == 'Y' )
       {
          doSave = true;
       }
   }
#endif // CODE_DEBUG

   bool abortQuit = false;

   for ( it = _winList.begin(); !abortQuit && it != _winList.end(); it++ )
   {
      ViewWindow * win = (*it);

      if ( doSave && win->getSaved() == false )
      {
         win->raise();
         win->setAbortQuit( false );
         win->saveModelEvent();
         abortQuit = win->getAbortQuit();
      }
   }

   if ( !abortQuit )
   {
      while ( !_winList.empty() )
      {
         ViewWindow * win = _winList.front();

         win->hide();
         win->deleteLater();

         _winList.pop_front();
      }
   }

   if ( abortQuit )
   {
      return false;
   }
   else
   {
      return true;
   }
}

class MainWidget : public QWidget
{
   public:
      MainWidget( QWidget * parent = NULL );
      virtual ~MainWidget() {};

      void addWidgetToLayout( QWidget * w );

   protected:

      QVBoxLayout * m_layout;
};

MainWidget::MainWidget( QWidget * parent )
   : QWidget( parent )
{
   m_layout = new QVBoxLayout( this );
   m_layout->setMargin(2);
   m_layout->setSpacing(2);
}

void MainWidget::addWidgetToLayout( QWidget * w )
{
   m_layout->addWidget( w );
}

ViewWindow::ViewWindow( Model * model, QWidget * parent )
   : QMainWindow( parent ),
     m_model( model ),
     m_animWin( NULL ),
     m_toolbox( NULL ),
     m_toolList( NULL ),
     m_toolButtons( NULL ),
     m_last( NULL ),
     m_currentTool( NULL )
{
   m_model->setUndoEnabled( false );
   setAttribute( Qt::WA_DeleteOnClose );
   _winList.push_back( this );


   setWindowIcon( QPixmap((const char **) mm3dlogo_32x32_xpm) );
   setWindowIconText( QString("Misfit Model 3D") );

   QShortcut * help = new QShortcut( QKeySequence( tr("F1", "Help Shortcut")), this );
   connect( help, SIGNAL(activated()), this, SLOT(helpNowEvent()) );

   updateCaption();

   m_toolbox = new Toolbox();

   MainWidget * mainWidget = new MainWidget( this );

   m_viewPanel = new ViewPanel( m_toolbox, mainWidget );
   mainWidget->addWidgetToLayout( m_viewPanel );

   m_statusBar = new StatusBar( m_model, mainWidget );
   mainWidget->addWidgetToLayout( m_statusBar );
   m_statusBar->setText( tr( "Press F1 for help using any window" ).toUtf8() );
   m_statusBar->setMaximumHeight( 30 );

   m_animWin = new AnimWindow( m_model, false, this );
   m_animWin->setObjectName( "mainwin_animwin" );
   m_animWidget = m_animWin->getAnimWidget();
   addDockWidget( Qt::BottomDockWidgetArea, m_animWin );

   connect( m_animWin, SIGNAL(animWindowClosed()), this, SLOT(animationModeOff()) );
   connect( m_animWidget, SIGNAL(animWindowClosed()), this, SLOT(animationModeOff()) );

   m_contextPanel = new ContextPanel( this, m_viewPanel, this );
   m_contextPanel->setObjectName( "mainwin_properties" );
   m_contextPanel->setWindowTitle( tr( "Properties" ) );

   connect( this, SIGNAL(modelChanged(Model*)), m_contextPanel, SLOT(setModel(Model*)));
   addDockWidget( Qt::RightDockWidgetArea, m_contextPanel );

   m_boolPanel = new BoolPanel( m_model, this, m_viewPanel );
   m_boolPanel->setObjectName( "mainwin_boolwin" );
   connect( this, SIGNAL(modelChanged(Model*)), m_boolPanel, SLOT(setModel(Model*)));

   m_projectionWin = new ProjectionWin( m_model, this, m_viewPanel );
   connect( this, SIGNAL(modelChanged(Model*)), m_projectionWin, SLOT(setModel(Model*)));

   m_textureCoordWin = new TextureCoord( m_model );
   connect( this, SIGNAL(modelChanged(Model*)), m_textureCoordWin, SLOT(setModel(Model*)));

   m_transformWin = new TransformWindow( m_model, this );
   connect( this, SIGNAL(modelChanged(Model*)), m_transformWin, SLOT(setModel(Model*)));

   addDockWidget( Qt::RightDockWidgetArea, m_boolPanel );
   setModel( m_model );

   tabifyDockWidget( m_contextPanel, m_boolPanel );

   setCentralWidget( mainWidget );

   m_mruMenu = new QMenu( this );
   m_scriptMruMenu = new QMenu( this );
   connect( m_mruMenu, SIGNAL(aboutToShow()), this, SLOT(fillMruMenu()) );
   connect( m_mruMenu, SIGNAL(triggered(QAction*)), this, SLOT(openMru(QAction*)) );
   connect( m_scriptMruMenu, SIGNAL(aboutToShow()), this, SLOT(fillScriptMruMenu()) );
   connect( m_scriptMruMenu, SIGNAL(triggered(QAction*)), this, SLOT(openScriptMru(QAction*)) );

   m_fileMenu = new QMenu( this );
   m_fileMenu->addAction( tr("New", "File|New"), this, SLOT(newModelEvent()), g_keyConfig.getKey( "viewwin_file_new" ) );
   m_fileMenu->addAction( tr("Open...", "File|Open"), this, SLOT(openModelEvent()), g_keyConfig.getKey( "viewwin_file_open" ) );
   m_fileMenu->addAction( tr("Save", "File|Save"), this, SLOT(saveModelEvent()), g_keyConfig.getKey( "viewwin_file_save" ) );
   m_fileMenu->addAction( tr("Save As...", "File|Save As"), this, SLOT(saveModelAsEvent()), g_keyConfig.getKey( "viewwin_file_save_as" ) );
   m_fileMenu->addAction( tr("Export...", "File|Export"), this, SLOT(exportModelEvent()), g_keyConfig.getKey( "viewwin_file_export" ) );
   m_fileMenu->addAction( tr("Export Selected...", "File|Export Selected"), this, SLOT(exportSelectedEvent()), g_keyConfig.getKey( "viewwin_file_export_selected" ) );
#ifdef HAVE_LUALIB
   m_fileMenu->addAction( tr("Run Script...", "File|Run Script"), this, SLOT(scriptEvent()), g_keyConfig.getKey( "viewwin_file_run_script" ) );
   m_scriptMruMenu->setTitle( tr("Recent Scripts", "File|Recent Script") );
   m_fileMenu->addMenu( m_scriptMruMenu );
#endif // HAVE_LUALIB
   m_fileMenu->addSeparator();
   m_mruMenu->setTitle( tr("Recent Models", "File|Recent Models") );
   m_fileMenu->addMenu( m_mruMenu );
   m_fileMenu->addSeparator();
   m_fileMenu->addAction( tr("Plugins...", "File|Plugins"), this, SLOT(pluginWindowEvent()), g_keyConfig.getKey( "viewwin_file_plugins" ) );
   m_fileMenu->addSeparator();
   m_fileMenu->addAction( tr("Close", "File|Close"), this, SLOT(close()), g_keyConfig.getKey( "viewwin_file_close" ) );
   m_fileMenu->addAction( tr("Quit", "File|Quit"), this, SLOT(quitEvent()), g_keyConfig.getKey( "viewwin_file_quit" ) );
   
   m_renderMenu = new QMenu( this );
   QActionGroup * group;

   // Bones group
   group = new QActionGroup( this );
   m_hideJointsItem     = m_renderMenu->addAction( tr("Hide Joints", "View|Hide Joints"),      this, SLOT(boneJointHide()), g_keyConfig.getKey( "viewwin_view_render_joints_hide" ) );
   m_drawJointLinesItem = m_renderMenu->addAction( tr("Draw Joint Lines", "View|Draw Joint Lines"), this, SLOT(boneJointLines()), g_keyConfig.getKey( "viewwin_view_render_joints_lines" ) );
   m_drawJointBonesItem = m_renderMenu->addAction( tr("Draw Joint Bones", "View|Draw Joint Bones"), this, SLOT(boneJointBones()), g_keyConfig.getKey( "viewwin_view_render_joints_bones" ) );

   m_hideJointsItem->setCheckable( true );
   m_drawJointLinesItem->setCheckable( true );
   m_drawJointBonesItem->setCheckable( true );
   group->addAction( m_hideJointsItem );
   group->addAction( m_drawJointLinesItem );
   group->addAction( m_drawJointBonesItem );

   g_prefs.setDefault( "ui_draw_joints", (int) Model::JOINTMODE_BONES );
   if ( g_prefs( "ui_draw_joints" ).intValue() == (int) Model::JOINTMODE_BONES ) 
      m_drawJointBonesItem->setChecked( true );
   else
      m_drawJointBonesItem->setChecked( true );

   m_renderMenu->addSeparator();

   // Projection group
   group = new QActionGroup( this );
   m_renderProjections   = m_renderMenu->addAction( tr("Draw Texture Projections", "View|Draw Texture Projections"),   this, SLOT(renderProjections()), g_keyConfig.getKey( "viewwin_view_render_projections_show" ) );
   m_noRenderProjections = m_renderMenu->addAction( tr("Hide Texture Projections", "View|Hide Texture Projections"),   this, SLOT(noRenderProjections()), g_keyConfig.getKey( "viewwin_view_render_projections_hide" ) );

   m_renderProjections->setCheckable( true );
   m_noRenderProjections->setCheckable( true );
   group->addAction( m_renderProjections );
   group->addAction( m_noRenderProjections );

   m_renderProjections->setChecked( true );

   m_renderMenu->addSeparator();

   // Bad texture group
   group = new QActionGroup( this );
   m_renderBadItem   = m_renderMenu->addAction( tr("Use Red Error Texture", "View|Use Red Error Texture"),     this, SLOT(renderBadEvent()), g_keyConfig.getKey( "viewwin_view_render_badtex_red" ) );
   m_noRenderBadItem = m_renderMenu->addAction( tr("Use Blank Error Texture", "View|Use Blank Error Texture"),   this, SLOT(noRenderBadEvent()), g_keyConfig.getKey( "viewwin_view_render_badtex_blank" ) );

   m_renderBadItem->setCheckable( true );
   m_noRenderBadItem->setCheckable( true );
   group->addAction( m_renderBadItem );
   group->addAction( m_noRenderBadItem );

   g_prefs.setDefault( "ui_render_bad_textures", 1 );
   if ( g_prefs( "ui_render_bad_textures" ).intValue() != 0 )
      m_renderBadItem->setChecked( true );
   else
      m_noRenderBadItem->setChecked( true );

   m_renderMenu->addSeparator();

   // 3D Lines group
   group = new QActionGroup( this );
   m_renderSelectionItem   = m_renderMenu->addAction( tr("Render 3D Lines", "View|Render 3D Lines"),   this, SLOT(renderSelectionEvent()), g_keyConfig.getKey( "viewwin_view_render_3d_lines_show" ) );
   m_noRenderSelectionItem = m_renderMenu->addAction( tr("Hide 3D Lines", "View|Hide 3D Lines"),   this, SLOT(noRenderSelectionEvent()), g_keyConfig.getKey( "viewwin_view_render_3d_lines_hide" ) );

   m_renderSelectionItem->setCheckable( true );
   m_noRenderSelectionItem->setCheckable( true );
   group->addAction( m_renderSelectionItem );
   group->addAction( m_noRenderSelectionItem );

   g_prefs.setDefault( "ui_render_3d_selections", 0 );
   if ( g_prefs( "ui_render_3d_selections" ).intValue() != 0 )
      m_renderSelectionItem->setChecked( true );
   else
      m_noRenderSelectionItem->setChecked( true );

   m_renderMenu->addSeparator();

   // Backfacing poly group
   group = new QActionGroup( this );
   m_renderBackface   = m_renderMenu->addAction( tr("Draw Back-facing Triangles", "View|Draw Back-facing Triangles"),   this, SLOT(renderBackface()), g_keyConfig.getKey( "viewwin_view_render_backface_show" ) );
   m_noRenderBackface = m_renderMenu->addAction( tr("Hide Back-facing Triangles", "View|Hide Back-facing Triangles"),   this, SLOT(noRenderBackface()), g_keyConfig.getKey( "viewwin_view_render_backface_hide" ) );

   m_renderBackface->setCheckable( true );
   m_noRenderBackface->setCheckable( true );
   group->addAction( m_renderBackface );
   group->addAction( m_noRenderBackface );

   g_prefs.setDefault( "ui_render_backface_cull", 0 );
   if ( g_prefs( "ui_render_backface_cull" ).intValue() == 0 )
      m_renderBackface->setChecked( true );
   else
      m_noRenderBackface->setChecked( true );

   m_viewMenu = new QMenu( this );
   m_viewMenu->addAction( tr( "Frame All", "View|Frame"), this, SLOT(frameAllEvent()), g_keyConfig.getKey( "viewwin_view_frame_all" ) );
   m_viewMenu->addAction( tr( "Frame Selected", "View|Frame"), this, SLOT(frameSelectedEvent()), g_keyConfig.getKey( "viewwin_view_frame_selected" ) );
   m_viewMenu->addSeparator();
   m_showContext = m_viewMenu->addAction( tr("Show Properties", "View|Show Properties"), this, SLOT(showContextEvent()), g_keyConfig.getKey( "viewwin_view_show_properties" ) );
   connect( m_contextPanel, SIGNAL(panelHidden()), this, SLOT(contextPanelHidden()) );
   m_renderMenu->setTitle( tr( "Render Options", "View|Render Options") );
   m_viewMenu->addMenu( m_renderMenu );
   m_viewMenu->addSeparator();

   // 3D View group
   group = new QActionGroup(this);
   m_3dWire = group->addAction( m_viewMenu->addAction( tr( "3D Wireframe", "View|3D"),   m_viewPanel, SLOT(wireframeEvent()), g_keyConfig.getKey( "viewwin_view_3d_wireframe" ) ) );
   m_3dFlat = group->addAction( m_viewMenu->addAction( tr( "3D Flat", "View|3D"),        m_viewPanel, SLOT(flatEvent()), g_keyConfig.getKey( "viewwin_view_3d_flat" ) ) );
   m_3dSmooth = group->addAction( m_viewMenu->addAction( tr( "3D Smooth", "View|3D"),      m_viewPanel, SLOT(smoothEvent()), g_keyConfig.getKey( "viewwin_view_3d_smooth" ) ) );
   m_3dTexture = group->addAction( m_viewMenu->addAction( tr( "3D Texture", "View|3D"),     m_viewPanel, SLOT(textureEvent()), g_keyConfig.getKey( "viewwin_view_3d_textured" ) ) );
   m_3dAlpha = group->addAction( m_viewMenu->addAction( tr( "3D Alpha Blend", "View|3D"), m_viewPanel, SLOT(alphaEvent()), g_keyConfig.getKey( "viewwin_view_3d_alpha" ) ) );
   m_viewMenu->addSeparator();

   // Canvas Group
   group = new QActionGroup(this);
   m_canvasWire = group->addAction( m_viewMenu->addAction( tr( "Canvas Wireframe", "View|Canvas"),   m_viewPanel, SLOT(canvasWireframeEvent()), g_keyConfig.getKey( "viewwin_view_ortho_wireframe" ) ) );
   m_canvasFlat = group->addAction( m_viewMenu->addAction( tr( "Canvas Flat", "View|Canvas"),        m_viewPanel, SLOT(canvasFlatEvent()), g_keyConfig.getKey( "viewwin_view_ortho_flat" ) ) );
   m_canvasSmooth = group->addAction( m_viewMenu->addAction( tr( "Canvas Smooth", "View|Canvas"),      m_viewPanel, SLOT(canvasSmoothEvent()), g_keyConfig.getKey( "viewwin_view_ortho_smooth" ) ) );
   m_canvasTexture = group->addAction( m_viewMenu->addAction( tr( "Canvas Texture", "View|Canvas"),     m_viewPanel, SLOT(canvasTextureEvent()), g_keyConfig.getKey( "viewwin_view_ortho_textured" ) ) );
   m_canvasAlpha = group->addAction( m_viewMenu->addAction( tr( "Canvas Alpha Blend", "View|Canvas"), m_viewPanel, SLOT(canvasAlphaEvent()), g_keyConfig.getKey( "viewwin_view_ortho_alpha" ) ) );
   m_viewMenu->addSeparator();

   // Num viewports group
   group = new QActionGroup(this);
   m_view1 = group->addAction( m_viewMenu->addAction( tr( "1 View", "View|Viewports"),   m_viewPanel, SLOT(view1()), g_keyConfig.getKey( "viewwin_view_1" )   ) );
   m_view1x2 = group->addAction( m_viewMenu->addAction( tr( "1x2 View", "View|Viewports"), m_viewPanel, SLOT(view1x2()), g_keyConfig.getKey( "viewwin_view_1x2" ) ) );
   m_view2x1 = group->addAction( m_viewMenu->addAction( tr( "2x1 View", "View|Viewports"), m_viewPanel, SLOT(view2x1()), g_keyConfig.getKey( "viewwin_view_2x1" ) ) );
   m_view2x2 = group->addAction( m_viewMenu->addAction( tr( "2x2 View", "View|Viewports"), m_viewPanel, SLOT(view2x2()), g_keyConfig.getKey( "viewwin_view_2x2" ) ) );
   m_view2x3 = group->addAction( m_viewMenu->addAction( tr( "2x3 View", "View|Viewports"), m_viewPanel, SLOT(view2x3()), g_keyConfig.getKey( "viewwin_view_2x3" ) ) );
   m_view3x2 = group->addAction( m_viewMenu->addAction( tr( "3x2 View", "View|Viewports"), m_viewPanel, SLOT(view3x2()), g_keyConfig.getKey( "viewwin_view_3x2" ) ) );
   m_view3x3 = group->addAction( m_viewMenu->addAction( tr( "3x3 View", "View|Viewports"), m_viewPanel, SLOT(view3x3()), g_keyConfig.getKey( "viewwin_view_3x3" ) ) );

   m_3dWire->setCheckable( true );
   m_3dFlat->setCheckable( true );
   m_3dSmooth->setCheckable( true );
   m_3dTexture->setCheckable( true );
   m_3dAlpha->setCheckable( true );

   m_3dTexture->setChecked( true );

   m_canvasWire->setCheckable( true );
   m_canvasFlat->setCheckable( true );
   m_canvasSmooth->setCheckable( true );
   m_canvasTexture->setCheckable( true );
   m_canvasAlpha->setCheckable( true );

   m_canvasWire->setChecked( true );

   m_view1->setCheckable( true );
   m_view1x2->setCheckable( true );
   m_view2x1->setCheckable( true );
   m_view2x2->setCheckable( true );
   m_view2x3->setCheckable( true );
   m_view3x2->setCheckable( true );
   m_view3x3->setCheckable( true );

   int count = 4;
   bool tall = false;

   if ( g_prefs.exists( "ui_viewport_count" ) )
      count = g_prefs( "ui_viewport_count" ).intValue();
   if ( g_prefs.exists( "ui_viewport_tall" ) )
      tall = (g_prefs( "ui_viewport_tall" ).intValue() != 0);

   switch ( count )
   {
      case 1:
         m_view1->setChecked( true );
         break;

      case 2:
         if ( tall )
            m_view1x2->setChecked( true );
         else
            m_view2x1->setChecked( true );
         break;

      default:
      case 4:
         m_view2x2->setChecked( true );
         break;

      case 6:
         if ( tall )
            m_view2x3->setChecked( true );
         else
            m_view3x2->setChecked( true );
         break;

      case 9:
         m_view3x3->setChecked( true );
         break;
   }

   
   m_viewMenu->addSeparator();
   m_viewMenu->addAction( tr( "Viewport Settings...", "View|Viewport Settings" ), this, SLOT(viewportSettingsEvent()), g_keyConfig.getKey( "viewwin_view_viewport_settings" ) );

   m_snapMenu = new QMenu( this );
   connect( m_snapMenu, SIGNAL(triggered(QAction*)), this, SLOT(snapToSelectedEvent(QAction*)) );
   m_snapToGrid = m_snapMenu->addAction( tr("Grid", "Tools|Snap to Grid") );
   m_snapToGrid->setShortcut( g_keyConfig.getKey( "tool_snap_to_grid" ) );
   m_snapToGrid->setCheckable( true );
   m_snapToVertex = m_snapMenu->addAction( tr("Vertex", "Tools|Snap to Vertex") );
   m_snapToVertex->setShortcut( g_keyConfig.getKey( "tool_snap_to_vertex" ) );
   m_snapToVertex->setCheckable( true );

   if ( g_prefs.exists( "ui_snap_grid" ) 
         &&  g_prefs( "ui_snap_grid" ).intValue() != 0 )
   {
      m_snapToGrid->setChecked( true );
   }
   if ( g_prefs.exists( "ui_snap_vertex" ) 
         &&  g_prefs( "ui_snap_vertex" ).intValue() != 0 )
   {
      m_snapToVertex->setChecked( true );
   }

   m_toolMenu = new QMenu( this );
   m_snapMenu->setTitle(  tr("Snap To") );
   m_toolMenu->addMenu( m_snapMenu );
   m_toolMenu->addSeparator();

   m_toolBar = new QToolBar( this );
   m_toolBar->setObjectName( "mainwin_toolbar" );
   addToolBar( m_toolBar );

   m_toolBar->setWindowTitle( tr( "Tools" ) );
   //m_toolBar->setHorizontallyStretchable( true );
   //m_toolBar->setVerticallyStretchable( true );

   initializeToolbox();

   m_modelMenu = new QMenu( this );
   m_modelMenu->addAction( tr("Undo"), this, SLOT(undoRequest()), QKeySequence( tr("Ctrl+Z", "Undo shortcut" ) ) );
   m_modelMenu->addAction( tr("Redo"), this, SLOT(redoRequest()), QKeySequence( tr("Ctrl+Y", "Redo shortcut" ) ) );
   m_modelMenu->addSeparator();
   m_modelMenu->addAction( tr("Edit Model Meta Data...", "Model|Edit Model Meta Data"), this, SLOT(metaWindowEvent()), g_keyConfig.getKey( "viewwin_model_edit_meta_data" ) );
   m_modelMenu->addAction( tr("Transform Model...", "Model|Transform Model"), this, SLOT(transformWindowEvent()), g_keyConfig.getKey( "viewwin_model_transform" ) );
   m_modelMenu->addAction( tr("Boolean Operation...", "Model|Boolean Operation"), this, SLOT(boolWindowEvent()), g_keyConfig.getKey( "viewwin_model_boolean_operation" ) );
   m_modelMenu->addSeparator();
   m_modelMenu->addAction( tr("Set Background Image...", "Model|Set Background Image"), this, SLOT(backgroundWindowEvent()), g_keyConfig.getKey( "viewwin_model_set_background_image" ) );
   m_modelMenu->addAction( tr("Merge...", "Model|Merge"), this, SLOT(mergeModelsEvent()), g_keyConfig.getKey( "viewwin_model_merge" ) );
   m_modelMenu->addAction( tr("Import Animations...", "Model|Import Animations"), this, SLOT(mergeAnimationsEvent()), g_keyConfig.getKey( "viewwin_model_import_animations" ) );

   m_geometryMenu = new QMenu( this );
   m_materialsMenu  = new QMenu( this );

   m_materialsMenu->addAction( tr("Edit Groups...", "Groups|Edit Groups"), this, SLOT(groupWindowEvent()), g_keyConfig.getKey( "viewwin_groups_edit_groups" ) );
   m_materialsMenu->addAction( tr("Edit Materials...", "Groups|Edit Materials"), this, SLOT(textureWindowEvent()), g_keyConfig.getKey( "viewwin_groups_edit_materials" ) );
   m_materialsMenu->addAction( tr("Reload Textures", "Groups|Reload Textures"), this, SLOT(reloadTexturesEvent()), g_keyConfig.getKey( "viewwin_groups_reload_textures" ) );
   m_materialsMenu->addSeparator();
   m_materialsMenu->addAction( tr("Edit Projection...", "Groups|Edit Projection"), this, SLOT(projectionWindowEvent()), g_keyConfig.getKey( "viewwin_groups_edit_projection" ) );
   m_materialsMenu->addAction( tr("Edit Texture Coordinates...", "Groups|Edit Texture Coordinates"), this, SLOT(textureCoordEvent()), g_keyConfig.getKey( "viewwin_groups_edit_texture_coordinates" ) );
   m_materialsMenu->addAction( tr("Paint Texture...", "Groups|Paint Texture"), this, SLOT(paintTextureEvent()), g_keyConfig.getKey( "viewwin_groups_paint_texture" ) );

   m_jointsMenu     = new QMenu( this );

   m_jointsMenu->addAction( tr( "Edit Joints...", "Joints|Edit Joints"), this, SLOT(jointWinEvent()), g_keyConfig.getKey( "viewwin_joints_edit_joints" ) );
   m_jointsMenu->addAction( tr( "Assign Selected to Joint", "Joints|Assign Selected to Joint"), this, SLOT(jointAssignSelectedToJoint()), g_keyConfig.getKey( "viewwin_joints_assign_selected" ) );
   m_jointsMenu->addAction( tr( "Auto-Assign Selected...", "Joints|Auto-Assign Selected"), this, SLOT(jointAutoAssignSelected()), g_keyConfig.getKey( "viewwin_joints_auto_assign_selected" ) );
   m_jointsMenu->addAction( tr( "Remove All Influences from Selected", "Joints|Remove All Influences from Selected"), this, SLOT(jointRemoveInfluencesFromSelected()), g_keyConfig.getKey( "viewwin_joints_remove_influences" ) );
   m_jointsMenu->addAction( tr( "Remove Selected Joint from Influencing", "Joints|Remove Selected Joint from Influencing"), this, SLOT(jointRemoveInfluenceJoint()), g_keyConfig.getKey( "viewwin_joints_remove_joint" ) );
   m_jointsMenu->addSeparator();
   m_jointsMenu->addAction( tr( "Convert Multiple Influences to Single", "Joints|Convert Multiple Influences to Single"), this, SLOT(jointMakeSingleInfluence()), g_keyConfig.getKey( "viewwin_joints_make_single_influence" ) );
   m_jointsMenu->addSeparator();
   m_jointsMenu->addAction( tr( "Select Joint Influences", "Joints|Select Joint Influences"), this, SLOT(jointSelectInfluenceJoints()), g_keyConfig.getKey( "viewwin_joints_select_joint_influences" ) );
   m_jointsMenu->addAction( tr( "Select Influenced Vertices", "Joints|Select Influenced Vertices"), this, SLOT(jointSelectInfluencedVertices()), g_keyConfig.getKey( "viewwin_joints_select_influenced_vertices" ) );
   m_jointsMenu->addAction( tr( "Select Influenced Points", "Joints|Select Influenced Points"), this, SLOT(jointSelectInfluencedPoints()), g_keyConfig.getKey( "viewwin_joints_select_influenced_points" ) );
   m_jointsMenu->addAction( tr( "Select Unassigned Vertices", "Joints|Select Unassigned Vertices"), this, SLOT(jointSelectUnassignedVertices()), g_keyConfig.getKey( "viewwin_joints_select_unassigned_vertices" ) );
   m_jointsMenu->addAction( tr( "Select Unassigned Points", "Joints|Select Unassigned Points"), this, SLOT(jointSelectUnassignedPoints()), g_keyConfig.getKey( "viewwin_joints_select_unassigned_points" ) );

   initializeCommands();

   connect( m_geometryMenu, SIGNAL(triggered(QAction*)), this, SLOT(primitiveCommandActivated(QAction*)) );

   //m_scriptMenu = new QPopupMenu( this );

   m_animMenu = new QMenu( this );
   m_startAnimItem     = m_animMenu->addAction( tr("Start Animation Mode...", "Animation|Start Animation Mode"), this, SLOT(startAnimationMode()), g_keyConfig.getKey( "viewwin_anim_start_mode" ) );
   m_stopAnimItem      = m_animMenu->addAction( tr("Stop Animation Mode", "Animation|Stop Animation Mode"),     this, SLOT(stopAnimationMode()), g_keyConfig.getKey( "viewwin_anim_stop_mode" ) );
   m_animMenu->addSeparator();
   m_animSetsItem      = m_animMenu->addAction( tr("Animation Sets...", "Animation|Animation Sets"), this, SLOT(animSetWindowEvent()), g_keyConfig.getKey( "viewwin_anim_animation_sets" ) );
   m_animExportItem    = m_animMenu->addAction( tr("Save Animation Images...", "Animation|Save Animation Images"), this, SLOT(animExportWindowEvent()), g_keyConfig.getKey( "viewwin_anim_save_images" ) );
   m_animMenu->addSeparator();
   m_animCopyFrame   = m_animMenu->addAction( tr("Copy Animation Frame", "Animation|Copy Animation Frame"), this, SLOT(animCopyFrameEvent()), g_keyConfig.getKey( "viewwin_anim_frame_copy" ) );
   m_animPasteFrame  = m_animMenu->addAction( tr("Paste Animation Frame", "Animation|Paste Animation Frame"), this, SLOT(animPasteFrameEvent()), g_keyConfig.getKey( "viewwin_anim_frame_paste" ) );
   m_animClearFrame  = m_animMenu->addAction( tr("Clear Animation Frame", "Animation|Clear Animation Frame"), this, SLOT(animClearFrameEvent()), g_keyConfig.getKey( "viewwin_anim_frame_clear" ) );
   m_animMenu->addSeparator();
   m_animCopySelected   = m_animMenu->addAction( tr("Copy Selected Keyframes", "Animation|Copy Animation Frame"), this, SLOT(animCopySelectedEvent()), g_keyConfig.getKey( "viewwin_anim_selected_copy" ) );
   m_animPasteSelected  = m_animMenu->addAction( tr("Paste Selected Keyframes", "Animation|Paste Animation Frame"), this, SLOT(animPasteSelectedEvent()), g_keyConfig.getKey( "viewwin_anim_selected_paste" ) );
   m_animMenu->addSeparator();
   m_animSetRotItem    = m_animMenu->addAction( tr("Set Rotation Keyframe", "Animation|Set Rotation Keyframe"), this, SLOT(animSetRotEvent()), g_keyConfig.getKey( "viewwin_anim_set_rotation" ) );
   m_animSetTransItem  = m_animMenu->addAction( tr("Set Translation Keyframe", "Animation|Set Translation Keyframe"), this, SLOT(animSetTransEvent()), g_keyConfig.getKey( "viewwin_anim_set_translation" ) );

   m_stopAnimItem->setEnabled( false );
   m_animSetRotItem->setEnabled( false );
   m_animSetTransItem->setEnabled( false );

   m_animCopyFrame->setEnabled( false );
   m_animPasteFrame->setEnabled( false );
   m_animClearFrame->setEnabled( false );
   m_animCopySelected->setEnabled( false );
   m_animPasteSelected->setEnabled( false );

   m_helpMenu = new QMenu( this );
   m_helpMenu->addAction( tr("Contents...", "Help|Contents"), this, SLOT(helpWindowEvent()), g_keyConfig.getKey( "viewwin_help_contents" ) );
   m_helpMenu->addAction( tr("License...", "Help|License"),  this, SLOT(licenseWindowEvent()), g_keyConfig.getKey( "viewwin_help_license" ) );
   m_helpMenu->addAction( tr("About...", "Help|About"),    this, SLOT(aboutWindowEvent()), g_keyConfig.getKey( "viewwin_help_about" ) );

   m_menuBar = menuBar();
   m_fileMenu->setTitle( tr("&File", "menu bar") );
   m_menuBar->addMenu( m_fileMenu );
   m_viewMenu->setTitle( tr("&View", "menu bar") );
   m_menuBar->addMenu( m_viewMenu );
   m_toolMenu->setTitle( tr("&Tools", "menu bar") );
   m_menuBar->addMenu( m_toolMenu );
   m_modelMenu->setTitle( tr("&Model", "menu bar") );
   m_menuBar->addMenu( m_modelMenu );
   m_geometryMenu->setTitle( tr("&Geometry", "menu bar") );
   m_menuBar->addMenu( m_geometryMenu );
   m_materialsMenu->setTitle( tr("Mate&rials", "menu bar") );
   m_menuBar->addMenu( m_materialsMenu );
   m_jointsMenu->setTitle( tr("&Influences", "menu bar") );
   m_menuBar->addMenu( m_jointsMenu );
   m_animMenu->setTitle( tr("&Animation", "menu bar") );
   m_menuBar->addMenu( m_animMenu );
   m_helpMenu->setTitle( tr("&Help", "menu bar") );
   m_menuBar->addMenu( m_helpMenu );

   resize( 
         g_prefs.exists( "ui_viewwin_width")  ? g_prefs( "ui_viewwin_width" )  : 900,
         g_prefs.exists( "ui_viewwin_height") ? g_prefs( "ui_viewwin_height" ) : 700 );

   setMinimumSize( 520, 520 );

   frameAllEvent();

   m_viewPanel->modelUpdatedEvent();
   show();

   QTimer * m_savedTimer = new QTimer();
   connect( m_savedTimer, SIGNAL(timeout()), this, SLOT(savedTimeoutCheck()) );
   m_savedTimer->start( 1000 );

   QString windowPos;

   loadDockPositions();
   stopAnimationMode();
   m_model->setUndoEnabled( true );
   m_model->clearUndo();

   // This is a hack to prevent a minimized bool panel from blocking the left
   // side of the menu bar.
   if ( !m_boolPanel->isVisible() )
   {
      m_boolPanel->show();
      m_boolPanel->hide();
   }
}

ViewWindow::~ViewWindow()
{
   log_debug( "deleting view window\n" );
   _winList.remove( this );

   log_debug( "deleting view window %08X, model %08X\n", this, m_model );
   DecalManager::getInstance()->unregisterModel( m_model );
   m_viewPanel->freeTextures();
   delete m_model;
   m_viewPanel->setModel( NULL );

   model_show_alloc_stats();

   // QToolBar actually deletes buttons, we just need to delete array
   delete[] m_toolButtons;

   if ( !_shuttingDown )
   {
      if ( _winList.empty() )
      {
         ui_exit();
      }
   }

   delete m_toolbox;
}

void ViewWindow::setModel( Model * model )
{
   m_viewPanel->setModel( model );
   m_statusBar->setModel( model );
   m_transformWin->setModel( model );
   m_model = model;

   if ( m_model )
   {
      Model::DrawJointModeE jointMode = 
         static_cast<Model::DrawJointModeE>( g_prefs( "ui_draw_joints" ).intValue() );

      if ( jointMode != Model::JOINTMODE_LINES )
      {
         jointMode = Model::JOINTMODE_BONES;
      }
      m_model->setDrawJoints( jointMode );

      g_prefs( "ui_draw_joints" ) = (int) jointMode;
   }

   updateCaption();

   //m_contextPanel->setModel( m_model );
   if ( m_animWin->isVisible() )
   {
      m_animWidget->initialize( m_model, true ); // Treat it like an undo
   }
   m_viewPanel->modelUpdatedEvent();

   emit modelChanged( m_model );

   while ( m_model->hasErrors() )
   {
      std::string str = m_model->popError();
      model_status( m_model, StatusError, STATUSTIME_LONG, "%s", str.c_str() );
   }

   if ( m_currentTool )
   {
      m_currentTool->setModel( m_model );
   }
}

bool ViewWindow::getSaved()
{
   if ( m_model )
   {
      return m_model->getSaved();
   }
   return true;
}

void ViewWindow::resizeEvent ( QResizeEvent * e )
{
   int w = e->size().width();
   int h = e->size().height();

   /*
   m_menuBar->move( 0, 0 );
   m_menuBar->resize( w, m_menuBar->height() );

   m_viewPanel->move( 0, m_menuBar->height() );
   m_viewPanel->resize( w, h - m_menuBar->height() - m_statusBar->height() );

   m_statusBar->move( 0, h - m_statusBar->height() );
   m_statusBar->resize( w, m_statusBar->height() );
   */

   g_prefs( "ui_viewwin_width" )  = w;
   g_prefs( "ui_viewwin_height" ) = h;

   QMainWindow::resizeEvent(e);
}

/*
void ViewWindow::resizeEvent ( QResizeEvent * e )
{
   int w = e->size().width();
   int h = e->size().height();

   m_menuBar->move( 0, 0 );
   m_menuBar->resize( w, m_menuBar->height() );

   m_viewPanel->move( 0, m_menuBar->height() );
   m_viewPanel->resize( w, h - m_menuBar->height() - m_statusBar->height() );

   m_statusBar->move( 0, h - m_statusBar->height() );
   m_statusBar->resize( w, m_statusBar->height() );

   g_prefs( "ui_viewwin_width" )  = w;
   g_prefs( "ui_viewwin_height" ) = h;
}
*/

void ViewWindow::helpNowEvent()
{
   HelpWin * win = new HelpWin( "olh_mainwin.html", false );
   win->show();
}

void ViewWindow::contextMenuEvent( QContextMenuEvent * e )
{
   e->ignore();
}

void ViewWindow::saveModelEvent()
{
   const char * filename = m_model->getFilename();
   if ( filename && filename[0] )
   {
      Model::ModelErrorE err 
         = FilterManager::getInstance()->writeFile( m_model, filename, false );

      if ( err == Model::ERROR_NONE )
      {
         m_model->setSaved( true );
         prefs_recent_model( filename );
      }
      else
      {
         m_abortQuit = true;
         if ( Model::operationFailed( err ) )
         {
            QString reason = modelErrStr( err, m_model );
            reason = QString(filename) + QString(":\n") + reason;
            msg_error( (const char *) reason.toUtf8() );
         }
      }
   }
   else
   {
      saveModelAsEvent();
   }
}

void ViewWindow::saveModelAsEvent()
{
   saveModelInternal( m_model, false );
}

void ViewWindow::exportModelEvent()
{
   saveModelInternal( m_model, true );
}

void ViewWindow::exportSelectedEvent()
{
   if ( m_model->getSelectedTriangleCount() == 0
         && m_model->getSelectedPointCount() == 0
         && m_model->getSelectedProjectionCount() == 0 )
   {
      model_status( m_model, StatusError, STATUSTIME_LONG, tr( "You must have at least 1 face, joint, or point selected to Export Selected" ).toUtf8() );
      return;
   }

   Model * m = m_model->copySelected();
   
   if ( !m )
      return;

   saveModelInternal( m, true );

   delete m;
}

void ViewWindow::saveModelInternal( Model * model, bool exportModel )
{
   list<string> formats = FilterManager::getInstance()->getAllWriteTypes( exportModel );

   QString formatsStr = 
      ((exportModel) ? tr( "All Exportable Formats" ) : tr( "All Writable Formats" ))
                                                      + QString(" (" );

   list<string>::iterator it = formats.begin();
   while(  it != formats.end() )
   {
      formatsStr += QString( (*it).c_str() );

      it++;

      if ( it != formats.end() )
      {
         formatsStr += QString( " " );
      }
   }

   formatsStr += QString( ")" );

   const char * modelFile = model->getFilename();
   QString dir = QString::fromUtf8( g_prefs( "ui_model_dir" ).stringValue().c_str() );
   if ( exportModel )
   {
      dir = QString::fromUtf8( g_prefs( "ui_export_dir" ).stringValue().c_str() );
   }
   else
   {
      if ( modelFile && modelFile[0] != '\0' )
      {
         std::string fullname;
         std::string fullpath;
         std::string basename;

         normalizePath( modelFile, fullname, fullpath, basename );
         dir = tr( fullpath.c_str() );
      }
   }

   if ( dir.isEmpty() )
   {
      dir = QString( "." );
   }

   QFileDialog d(NULL, QString(""), dir, formatsStr + QString(";; ") + tr( "All Files (*)" ) );
   d.setAcceptMode( QFileDialog::AcceptSave );
   if ( exportModel )
   {
      d.selectFile( QString( model->getExportFile() ) );
   }

   d.setWindowTitle( tr( "Save model file as" ) );
   if ( exportModel )
   {
      d.setWindowTitle( tr( "Export model" ) );
   }
   d.selectFilter( formatsStr );
   d.setFileMode( QFileDialog::AnyFile );

   m_abortQuit = true;

   bool again = true;
   while ( again )
   {
      again = false;
      if ( QDialog::Accepted == d.exec() )
      {
         bool save = true;

         QStringList files = d.selectedFiles();

         if ( save && !files.empty() )
         {
            std::string filename = (const char *) files[0].toUtf8();
            if ( !strchr(filename.c_str(), '.' ) )
            {
               filename += ".mm3d";
            }

            Model::ModelErrorE err 
               = FilterManager::getInstance()->writeFile( model, filename.c_str(), exportModel );
            if ( err == Model::ERROR_NONE )
            {
               m_abortQuit = false;
               if ( exportModel )
               {
                  g_prefs( "ui_export_dir" ) = (const char *) d.directory().absolutePath().toUtf8();
                  model->setExportFile( filename.c_str() );
               }
               else
               {
                  model->setSaved( true );
                  model->setFilename( filename.c_str() );
                  g_prefs( "ui_model_dir" ) = (const char *) d.directory().absolutePath().toUtf8();
               }
               prefs_recent_model( (const char *) filename.c_str() );

               updateCaption();
            }
            else
            {
               if ( Model::operationFailed( err ) )
               {
                  msg_error( modelErrStr( err, model ).toUtf8() );
               }
            }
         }
      }
   }
}

void ViewWindow::mergeModelsEvent()
{
   list<string> formats = FilterManager::getInstance()->getAllReadTypes();

   QString formatsStr = tr( "All Supported Formats", "model formats" ) +  QString( " (" );

   list<string>::iterator it = formats.begin();
   while(  it != formats.end() )
   {
      formatsStr += QString( (*it).c_str() );

      it++;

      if ( it != formats.end() )
      {
         formatsStr += QString( " " );
      }
   }

   formatsStr += QString(")");

   QString dir = QString::fromUtf8( g_prefs( "ui_model_dir" ).stringValue().c_str() );
   if ( dir.isEmpty() )
   {
      dir = ".";
   }

   QFileDialog d(NULL, QString(""), dir, formatsStr + QString(";; ") + tr( "All Files (*)" ) );

   d.setWindowTitle( tr( "Open model file" ) );
   d.selectFilter( formatsStr );

   if ( QDialog::Accepted == d.exec() )
   {
      QStringList files = d.selectedFiles();

      if ( files.empty() )
         return;

      std::string filename = (const char *) files[0].toUtf8();

      Model::ModelErrorE err;
      Model * model = new Model();
      if ( (err = FilterManager::getInstance()->readFile( model, filename.c_str() )) == Model::ERROR_NONE)
      {
         model_show_alloc_stats();

         MergeWindow mw( model, this );

         if ( mw.exec() )
         {
            Model::AnimationMergeE mode = Model::AM_NONE;

            if ( mw.getIncludeAnimation() )
            {
               mode = Model::AM_ADD;

               if ( mw.getAnimationMerge() )
               {
                  mode = Model::AM_MERGE;
               }
            }
            double rot[3];
            double trans[3];
            mw.getRotation( rot );
            mw.getTranslation( trans );
            m_model->mergeModels( model, mw.getIncludeTexture(), mode, true, trans, rot );
            m_model->operationComplete( tr("Merge models").toUtf8() );
            g_prefs( "ui_model_dir" ) = (const char *) d.directory().absolutePath().toUtf8();

            m_viewPanel->modelUpdatedEvent();
         }

         prefs_recent_model( filename.c_str() );
         delete model;
      }
      else
      {
         if ( Model::operationFailed( err ) )
         {
            QString reason = modelErrStr( err, model );
            reason = tr(filename.c_str()) + tr(":\n") + reason;
            msg_error( (const char *) reason.toUtf8() );
         }
         delete model;
      }
   }
}

void ViewWindow::mergeAnimationsEvent()
{
   list<string> formats = FilterManager::getInstance()->getAllReadTypes();

   QString formatsStr = tr( "All Supported Formats", "model formats") + QString( " (" );

   list<string>::iterator it = formats.begin();
   while(  it != formats.end() )
   {
      formatsStr += QString( (*it).c_str() );

      it++;

      if ( it != formats.end() )
      {
         formatsStr += QString( " " );
      }
   }

   formatsStr += QString( ")" );

   QString dir = QString::fromUtf8( g_prefs( "ui_model_dir" ).stringValue().c_str() );
   if ( dir.isEmpty() )
   {
      dir = ".";
   }

   QFileDialog d(NULL, QString(""), dir, formatsStr + QString(";; ") + tr( "All Files (*)" ) );

   d.setWindowTitle( tr( "Open model file" ) );
   d.selectFilter( formatsStr );

   if ( QDialog::Accepted == d.exec() )
   {
      QStringList files = d.selectedFiles();

      if ( files.empty() )
         return;

      std::string filename = (const char *) files[0].toUtf8();

      Model::ModelErrorE err;
      Model * model = new Model();
      if ( (err = FilterManager::getInstance()->readFile( model, filename.c_str() )) == Model::ERROR_NONE)
      {
         model_show_alloc_stats();

         m_model->mergeAnimations( model );

         prefs_recent_model( filename.c_str() );
         delete model;
      }
      else
      {
         if ( Model::operationFailed( err ) )
         {
            QString reason = modelErrStr( err, model );
            reason = tr(filename.c_str()) + tr(":\n") + reason;
            msg_error( (const char *) reason.toUtf8() );
         }
         delete model;
      }
   }
}

void ViewWindow::scriptEvent()
{
#ifdef HAVE_LUALIB
   QString dir = QString::fromUtf8( g_prefs( "ui_script_dir" ).stringValue().c_str() );
   if ( dir.isEmpty() )
   {
      dir = QString(".");
   }

   QString formatsStr = QString( "Lua scripts (*.lua)" );
   QFileDialog d(NULL, QString(""), dir, formatsStr + QString(";; ") + tr( "All Files (*)" ) );

   d.setWindowTitle( tr( "Open model file" ) );
   d.selectFilter( formatsStr );

   if ( QDialog::Accepted == d.exec() )
   {
      QStringList files = d.selectedFiles();

      if ( files.empty() )
         return;

      g_prefs( "ui_script_dir" ) = (const char *) d.directory().absolutePath().toUtf8();

      std::string filename = (const char *) files[0].toUtf8();
      runScript( filename.c_str() );
   }
#endif // HAVE_LUALIB
}

void ViewWindow::runScript( const char * filename )
{
#ifdef HAVE_LUALIB
   if ( filename )
   {
      LuaScript lua;
      LuaContext lc( m_model );
      luaif_registerfunctions( &lua, &lc );

      std::string scriptfile = filename;

      std::string fullname;
      std::string fullpath;
      std::string basename;

      normalizePath( scriptfile.c_str(), fullname, fullpath, basename );

      log_debug( "running script %s\n", basename.c_str() );
      int rval = lua.runFile( scriptfile.c_str() );

      prefs_recent_script( filename );

      if ( rval == 0 )
      {
         log_debug( "script complete, exited normally\n" );
         QString str = tr("Script %1 complete").arg(basename.c_str());
         model_status( m_model, StatusNormal, STATUSTIME_SHORT, "%s", (const char *) str.toUtf8() );
      }
      else
      {
         log_error( "script complete, exited with error code %d\n", rval );
         QString str = tr("Script %1 error %2")
            .arg(basename.c_str())
            .arg(lua.error());
         model_status( m_model, StatusError, STATUSTIME_LONG, "%s", (const char *) str.toUtf8() );
      }

      m_model->setNoAnimation();
      m_model->operationComplete( basename.c_str() );

      m_viewPanel->modelUpdatedEvent();
   }
#endif // HAVE_LUALIB
}

void ViewWindow::closeEvent( QCloseEvent * e )
{
   saveDockPositions();
   
#ifdef CODE_DEBUG
   e->accept();
#else // CODE_DEBUG
   if ( ! m_model->getSaved() )
   {
      int val = QMessageBox::warning( this, tr("Save first?"), tr("Model has been modified\nDo you want to save before closing?"), QMessageBox::Yes, QMessageBox::No, QMessageBox::Cancel );
      switch ( val )
      {
         case QMessageBox::Yes:
            m_abortQuit = false;
            saveModelEvent();
            if ( ! m_abortQuit )
            {
               e->accept();
            }
            break;
         case QMessageBox::No:
            e->accept();
            break;
         case QMessageBox::Cancel:
            e->ignore();
            break;
         default:
            {
               QString str = tr( "Unknown response: %1, Canceling close request" )
                  .arg( val );
               msg_error( (const char *) str.toUtf8() );
            }
            e->ignore();
            break;
      }
   }
   else
   {
      e->accept();
   }
#endif // CODE_DEBUG
}

QAction * ViewWindow::insertMenuItem( QMenu * parentMenu, bool isTool,
      const QString & path, const QString & name, QMenu * subMenu )
{
   QMenu * addMenu = parentMenu;

   if ( path.length() != 0 )
   {
      bool found = false;

      MenuItemList::iterator it;
      for ( it = m_menuItems.begin(); it != m_menuItems.end(); it++ )
      {
         if ( it->text == path )
         {
            addMenu = it->menu;
            found = true;
         }
      }

      if ( !found )
      {
         // TODO deal with multi-level paths
         addMenu = new QMenu( this );

         QString module;
         if ( isTool )
         {
            module = "Tool";
            connect( addMenu, SIGNAL(triggered(QAction*)), this, SLOT(toolActivated(QAction*)));
         }
         else
         {
            module = "Command";
            connect( addMenu, SIGNAL(triggered(QAction*)), this, SLOT(primitiveCommandActivated(QAction*)) );
         }
         addMenu->setTitle( qApp->translate( module.toUtf8(), path.toUtf8() ) );
         parentMenu->addMenu( addMenu );

         MenuItemT mi;
         mi.text = path;
         mi.menu = addMenu;

         m_menuItems.push_back( mi );
      }
   }

   QAction * id;
   if ( subMenu )
   {
      subMenu->setTitle( name );
      id = addMenu->addMenu( subMenu );
   }
   else
   {
      id = addMenu->addAction( name );
   }
   log_debug( "added %s as id %d\n", (const char *) name.toUtf8(), id );
   return id;
}

void ViewWindow::frameAllEvent()
{
   double x1, y1, z1, x2, y2, z2;
   if ( m_model->getBoundingRegion( &x1, &y1, &z1, &x2, &y2, &z2 ) )
   {
      m_viewPanel->frameArea( x1, y1, z1, x2, y2, z2 );
   }
}

void ViewWindow::frameSelectedEvent()
{
   double x1, y1, z1, x2, y2, z2;
   if ( m_model->getSelectedBoundingRegion( &x1, &y1, &z1, &x2, &y2, &z2 ) )
   {
      m_viewPanel->frameArea( x1, y1, z1, x2, y2, z2 );
   }
}

void ViewWindow::showContextEvent()
{
   m_viewPanel->modelUpdatedEvent();

   // Set model so that it is properly initialized before display
   m_contextPanel->setModel( m_model );
   m_contextPanel->show();
   m_contextPanel->raise();

   // Set model again so that it actually displays properties
   m_contextPanel->setModel( m_model );
}

void ViewWindow::renderBadEvent()
{
   g_prefs( "ui_render_bad_textures" ) = 1;
   m_viewPanel->modelUpdatedEvent();
}

void ViewWindow::noRenderBadEvent()
{
   g_prefs( "ui_render_bad_textures" ) = 0;
   m_viewPanel->modelUpdatedEvent();
}

void ViewWindow::renderBackface()
{
   g_prefs( "ui_render_backface_cull" ) = 0;
   m_viewPanel->modelUpdatedEvent();
}

void ViewWindow::noRenderBackface()
{
   g_prefs( "ui_render_backface_cull" ) = 1;
   m_viewPanel->modelUpdatedEvent();
}

void ViewWindow::renderProjections()
{
   g_prefs( "ui_render_projections" ) = 0;
   m_model->setDrawProjections( true );
   m_viewPanel->modelUpdatedEvent();
}

void ViewWindow::noRenderProjections()
{
   bool doHide = true;

   if ( m_model->getSelectedProjectionCount() > 0 )
   {
      doHide = false;

      char ch = msg_info_prompt( (const char *) tr("Cannot hide with selected projections.  Unselect projections now?").toUtf8(), "yN" );

      if ( toupper(ch) == 'Y' )
      {
         doHide = true;
         m_model->unselectAll();
         m_model->operationComplete( tr("Hide projections").toUtf8() );
      }
   }

   if ( doHide )
   {
      g_prefs( "ui_render_projections" ) = 1;
      m_model->setDrawProjections( false );
      m_viewPanel->modelUpdatedEvent();
   }
}

void ViewWindow::renderSelectionEvent()
{
   g_prefs( "ui_render_3d_selections" ) = 1;
   m_viewPanel->modelUpdatedEvent();
}

void ViewWindow::noRenderSelectionEvent()
{
   g_prefs( "ui_render_3d_selections" ) = 0;
   m_viewPanel->modelUpdatedEvent();
}

void ViewWindow::boneJointHide()
{
   bool doHide = true;

   if ( m_model->getSelectedBoneJointCount() > 0 )
   {
      doHide = false;

      char ch = msg_info_prompt( (const char *) tr("Cannot hide with selected joints.  Unselect joints now?").toUtf8(), "yN" );

      if ( toupper(ch) == 'Y' )
      {
         doHide = true;
         m_model->unselectAll();
         m_model->operationComplete( tr("Hide bone joints").toUtf8() );
      }
   }

   if ( doHide )
   {
      m_model->setDrawJoints( Model::JOINTMODE_NONE );
      m_viewPanel->modelUpdatedEvent();
   }
}

void ViewWindow::boneJointLines()
{
   Model::DrawJointModeE m = Model::JOINTMODE_LINES;
   g_prefs( "ui_draw_joints" ) = (int) m;
   m_model->setDrawJoints( m );
   m_viewPanel->modelUpdatedEvent();
}

void ViewWindow::boneJointBones()
{
   Model::DrawJointModeE m = Model::JOINTMODE_BONES;
   g_prefs( "ui_draw_joints" ) = (int) m;
   m_model->setDrawJoints( m );
   m_viewPanel->modelUpdatedEvent();
}

void ViewWindow::viewportSettingsEvent()
{
   ViewportSettings * win = new ViewportSettings();
   win->show();
}

void ViewWindow::toolActivated( QAction * id )
{
   log_debug( "toolActivated(%p)\n", id );
   for ( ToolMenuItemList::iterator it = m_tools.begin();
         it != m_tools.end(); ++it )
   {
      if ( (*it)->id == id )
      {
         ::Tool * tool = (*it)->tool;
         for ( int t = 0; t < m_toolCount; t++ )
         {
            if ( m_toolList[t] == tool )
            {
               if ( !m_toolButtons[t]->isChecked() )
               {
                  m_toolButtons[t]->setChecked( true );
               }
               m_currentTool = m_toolList[ t ];
            }
         }
         return;
      }
   }
}

void ViewWindow::primitiveCommandActivated( QAction * id )
{
   CommandMenuItemList::iterator it;
   it = m_primitiveCommands.begin();
   while ( it != m_primitiveCommands.end() )
   {
      if ( (*it)->id == id )
      {
         if ( ((*it)->command)->activated( (*it)->arg, m_model ) )
         {
            m_model->operationComplete( qApp->translate( "Command", ((*it)->command)->getName( (*it)->arg ) ).toUtf8() );
            m_viewPanel->modelUpdatedEvent();
         }
         break;
      }
      it++;
   }
}

void ViewWindow::scriptActivated( QAction * id )
{
}

void ViewWindow::groupWindowEvent()
{
   GroupWindow * win = new GroupWindow( m_model );
   win->show();
}

void ViewWindow::textureWindowEvent()
{
   TextureWindow * win = new TextureWindow( m_model );
   win->show();
}

void ViewWindow::textureCoordEvent()
{
   if ( m_model->getSelectedTriangleCount() > 0 )
   {
      m_textureCoordWin->show();
      m_textureCoordWin->raise();
   }
   else
   {
      msg_info( (const char *) tr("You must select faces first.\nUse the 'Select Faces' tool.", "Notice that user must have faces selected to open 'edit texture coordinates' window" ).toUtf8());
   }
}

void ViewWindow::paintTextureEvent()
{
   if ( m_model->getSelectedTriangleCount() > 0 )
   {
      PaintTextureWin * win = new PaintTextureWin( m_model );
      win->show();
   }
   else
   {
      msg_info( (const char *) tr("You must select faces first.\nUse the 'Select Faces' tool.", "Notice that user must have faces selected to open 'paint texture' window").toUtf8() );
   }
}

// ContextPanelObserver method
void ViewWindow::showProjectionEvent()
{
   projectionWindowEvent();
}

void ViewWindow::projectionWindowEvent()
{
   m_projectionWin->show();
   m_projectionWin->raise();
}

void ViewWindow::transformWindowEvent()
{
   m_transformWin->show();
   m_transformWin->raise();
}

void ViewWindow::metaWindowEvent()
{
   MetaWindow * win = new MetaWindow( m_model );
   win->show();
}

void ViewWindow::boolWindowEvent()
{
   m_boolPanel->show();
   m_boolPanel->raise();
}

void ViewWindow::reloadTexturesEvent()
{
   if( TextureManager::getInstance()->reloadTextures() )
   {
      invalidateModelTextures();
   }
}


void ViewWindow::undoRequest()
{
   log_debug( "undo request\n" );

   if ( m_model->canUndo() )
   {
      const char * opname = m_model->getUndoOpName();

      if ( m_animWin->isVisible() )
      {
         m_animWidget->undoRequest();
      }
      else
      {
         m_model->undo();

         if ( m_model->getAnimationMode() )
         {
            m_animWidget->initialize( m_model, true );
            animationModeOn();
            m_animWin->show();
         }
         else
         {
            m_viewPanel->modelUpdatedEvent();
         }
      }
      
      QString str = tr( "Undo %1" ).arg( (opname && opname[0]) ? opname : "" );
      model_status ( m_model, StatusNormal, STATUSTIME_SHORT, "%s", 
            (const char *) str.toUtf8() );

      if ( m_model->getSelectedBoneJointCount() > 0 )
      {
         m_model->setDrawJoints( 
               (Model::DrawJointModeE) g_prefs( "ui_draw_joints" ).intValue() );
         m_viewPanel->modelUpdatedEvent();
      }
   }
   else
   {
      model_status( m_model, StatusNormal, STATUSTIME_SHORT, tr("Nothing to undo").toUtf8() );
   }
}

void ViewWindow::redoRequest()
{
   log_debug( "redo request\n" );

   if ( m_model->canRedo() )
   {
      const char * opname = m_model->getRedoOpName();

      if ( m_animWin->isVisible() )
      {
         m_animWidget->redoRequest();
      }
      else
      {
         m_model->redo();

         if ( m_model->getAnimationMode() )
         {
            m_animWidget->initialize( m_model, true );
            animationModeOn();
            m_animWin->show();
         }
         else
         {
            m_viewPanel->modelUpdatedEvent();
         }
      }

      if ( m_model->getSelectedBoneJointCount() > 0 )
      {
         m_model->setDrawJoints( 
               (Model::DrawJointModeE) g_prefs( "ui_draw_joints" ).intValue() );
         m_viewPanel->modelUpdatedEvent();
      }
      
      QString str = tr( "Redo %1" ).arg( (opname && opname[0]) ? opname : "" );
      model_status ( m_model, StatusNormal, STATUSTIME_SHORT, "%s", 
            (const char *) str.toUtf8() );
   }
   else
   {
      model_status( m_model, StatusNormal, STATUSTIME_SHORT, tr("Nothing to redo").toUtf8() );
   }
}

void ViewWindow::snapToSelectedEvent( QAction * snapTo )
{
   log_debug( "snapToSelectedEvent( %d )\n", snapTo );
   g_prefs( "ui_snap_grid" )   = ( m_snapToGrid->isChecked() ) ? 1 : 0;
   g_prefs( "ui_snap_vertex" ) = ( m_snapToVertex->isChecked() ) ? 1 : 0;
}

void ViewWindow::helpWindowEvent()
{
   HelpWin * win = new HelpWin();
   win->show();
}

void ViewWindow::aboutWindowEvent()
{
   AboutWin * win = new AboutWin();
   win->show();
}

void ViewWindow::licenseWindowEvent()
{
   LicenseWin * win = new LicenseWin();
   win->show();
}

void ViewWindow::animSetWindowEvent()
{
   if ( m_animWin->isVisible() )
   {
      stopAnimationMode();
   }

   AnimSetWindow asw( m_model, this );
   asw.exec();
}

void ViewWindow::animExportWindowEvent()
{
   if ( m_model->getAnimCount( Model::ANIMMODE_SKELETAL ) > 0 
         || m_model->getAnimCount( Model::ANIMMODE_FRAME ) > 0 )
   {
      AnimExportWindow aew( m_model, m_viewPanel, this );
      aew.exec();
   }
   else
   {
      msg_error( (const char *) tr("This model does not have any animations").toUtf8() );
   }
}

void ViewWindow::animSetRotEvent()
{
   double point[3] = { 0.0, 0.0, 0.0 };
   Matrix m;
   m.loadIdentity();
   m_model->rotateSelected( m, point );
   m_model->operationComplete( tr("Set rotation keframe").toUtf8() );
}

void ViewWindow::animSetTransEvent()
{
   Matrix m;
   m.loadIdentity();
   m_model->translateSelected( m );
   m_model->operationComplete( tr("Set translation keframe").toUtf8() );
}

void ViewWindow::animCopyFrameEvent()
{
   if ( m_animWin->isVisible() )
   {
      m_animPasteFrame->setEnabled( false );
      m_animPasteSelected->setEnabled( false );

      if ( m_animWidget->copyFrame( false ) )
      {
         m_animPasteFrame->setEnabled( true );
      }
   }
}

void ViewWindow::animPasteFrameEvent()
{
   if ( m_animWin->isVisible() )
   {
      m_animWidget->pasteFrame();
   }
}

void ViewWindow::animCopySelectedEvent()
{
   if ( m_animWin->isVisible() )
   {
      m_animPasteFrame->setEnabled( false );
      m_animPasteSelected->setEnabled( false );

      if ( m_animWidget->copyFrame( true ) )
      {
         m_animPasteSelected->setEnabled( true );
      }
   }
}

void ViewWindow::animPasteSelectedEvent()
{
   animPasteFrameEvent(); // Same logic for both
}

void ViewWindow::animClearFrameEvent()
{
   if ( m_animWin->isVisible() )
   {
      m_animWidget->clearFrame();
   }
}

void ViewWindow::startAnimationMode()
{
   m_animWidget->initialize( m_model, false );
   animationModeOn();
   m_animWin->show();
}

void ViewWindow::stopAnimationMode()
{
   m_animWin->close();
   m_animWidget->stopAnimationMode();
   animationModeOff();
}

void ViewWindow::animationModeOn()
{
   m_animExportItem->setEnabled( false );
   m_startAnimItem->setEnabled( false );
   m_stopAnimItem->setEnabled(  true  );
   m_animSetRotItem->setEnabled(  true  );
   m_animSetTransItem->setEnabled(  true  );
   m_animCopyFrame->setEnabled(  true  );
   m_animPasteFrame->setEnabled(  false  ); // Disabled until copy occurs
   m_animClearFrame->setEnabled(  true  );
   m_animCopySelected->setEnabled(  true  );
   m_animPasteSelected->setEnabled(  false  ); // Disabled until copy occurs
}

void ViewWindow::animationModeOff()
{
   m_model->setNoAnimation();
   m_viewPanel->modelUpdatedEvent();
   m_animWin->close();
   m_animExportItem->setEnabled( true );
   m_startAnimItem->setEnabled( true );
   m_stopAnimItem->setEnabled(  false  );
   m_animSetRotItem->setEnabled(  false  );
   m_animSetTransItem->setEnabled(  false  );
   m_animCopyFrame->setEnabled(  false  );
   m_animPasteFrame->setEnabled(  false  );
   m_animClearFrame->setEnabled(  false  );
   m_animCopySelected->setEnabled(  false  );
   m_animPasteSelected->setEnabled(  false  );
}

void ViewWindow::contextPanelHidden()
{
   m_showContext->setText( tr( "Show Properties", "View|Show Properties") );
   //m_viewPanel->modelUpdatedEvent();
}

void ViewWindow::buttonToggled( bool on )
{
   if ( on )
   {
      if ( m_currentTool )
         m_currentTool->deactivated();

      for ( int t = 0; t < m_toolCount; t++ )
      {
         if ( m_toolButtons[t]->isChecked() )
         {
            if ( m_last != m_toolButtons[t] )
            {
               m_currentTool = m_toolList[ t ];
               m_currentTool->activated( 0, m_model, this );
               m_toolbox->setCurrentTool( m_currentTool );
               break;
            }
         }
      }
   }
}

static void _registerKeyBinding( ::Tool * tool, int index,
      QMenu * menu, QAction * id )
{
   QString name = QString( "tool_" ) + QString::fromUtf8( tool->getName( 0 ) );

   if ( index > 0 )
   {
      name += QString( "_" ) + QString::fromUtf8( tool->getName( index ) );
   }

   name = name.replace( QString("."), QString("") );
   name = name.replace( QString(" "), QString("_") );
   name = name.toLower();

   QKeySequence key = g_keyConfig.getKey( (const char *) name.toUtf8() );

   if ( !key.isEmpty() )
   {
      id->setShortcut( key );
   }
}

static QString _makeToolTip( ::Tool * tool, int index )
{
   QString lookupStr = QString( "tool_" ) + QString::fromUtf8( tool->getName( 0 ) );

   if ( index > 0 )
   {
      lookupStr += QString( "_" ) + QString::fromUtf8( tool->getName( index ) );
   }

   lookupStr = lookupStr.replace( QString("."), QString("") );
   lookupStr = lookupStr.replace( QString(" "), QString("_") );
   lookupStr = lookupStr.toLower();

   QKeySequence key = g_keyConfig.getKey( (const char *) lookupStr.toUtf8() );

   QString name = qApp->translate( "Tool", tool->getName( index ) );

   if ( !key.isEmpty() )
   {
      name += QString(" (");
      name += (QString) key;
      name += QString(")");
   }
   return name;
}

void ViewWindow::initializeToolbox()
{
   connect( m_toolMenu, SIGNAL(triggered(QAction*)), this, SLOT(toolActivated(QAction*)));
   m_toolbox->registerAllTools();

   QActionGroup * grp = new QActionGroup( this );

   m_toolButtons = new QActionPtr[ m_toolbox->getToolCount() ];
   m_toolList    = new ToolPtr[ m_toolbox->getToolCount() ];
   m_toolCount = 0;

   ::Tool * tool = m_toolbox->getFirstTool();
   while ( tool )
   {
      if ( !tool->isSeparator() )
      {
         ToolMenuItemT * item;
         QAction * id;
         int count = tool->getToolCount();
         if ( count > 1 )
         {
            QMenu * menu = new QMenu( this );
            connect( menu, SIGNAL(triggered(QAction*)), this, SLOT(toolActivated(QAction*)));
            for ( int t = 1; t < count; t++ )
            {
               const char * name = tool->getName( t );
               id = menu->addAction( qApp->translate( "Tool", name ) );

               _registerKeyBinding( tool, t, menu, id );

               item = new ToolMenuItemT;
               item->id = id;
               item->tool = tool;
               item->arg = t;

               m_tools.push_back( item );

               // Create tool button
               QIcon set;
               set.addPixmap( QPixmap( tool->getPixmap() ) );

               m_toolList[m_toolCount] = tool;
               // Text below
               m_toolButtons[ m_toolCount ] = m_toolBar->addAction( set, "Text" );
               m_toolButtons[ m_toolCount ]->setCheckable( true );
               if ( name && name[0] )
               {
                  m_toolButtons[ m_toolCount ]->setToolTip( _makeToolTip( tool, t ) );
               }

               connect( m_toolButtons[m_toolCount], SIGNAL(toggled(bool)), this, SLOT(buttonToggled(bool)));

               grp->addAction( m_toolButtons[ m_toolCount ] );

               m_toolCount++;
            }

            //id = m_toolMenu->addAction( qApp->translate( "Tool", tool->getName(0)), menu );
            id = insertMenuItem( m_toolMenu, true, tool->getPath(),
                  qApp->translate( "Tool", tool->getName(0)), menu );

            _registerKeyBinding( tool, 0, m_toolMenu, id );

            item = new ToolMenuItemT;
            item->id = id;
            item->tool = tool;
            item->arg = 0;

            m_tools.push_back( item );
         }
         else
         {
            const char * name = tool->getName( 0 );
            //id = m_toolMenu->addAction( qApp->translate( "Tool", name ) );
            id = insertMenuItem( m_toolMenu, true, tool->getPath(),
                  qApp->translate( "Tool", tool->getName(0)), NULL );

            _registerKeyBinding( tool, 0, m_toolMenu, id );

            item = new ToolMenuItemT;
            item->id = id;
            item->tool = tool;
            item->arg = 0;

            m_tools.push_back( item );

            // Create tool button
            QIcon set;
            set.addPixmap( QPixmap( tool->getPixmap() ) );

            m_toolList[m_toolCount] = tool;
            // Text below
            m_toolButtons[ m_toolCount ] = m_toolBar->addAction( set, "Text" );
            m_toolButtons[ m_toolCount ]->setCheckable( true );
            if ( name && name[0] )
            {
               m_toolButtons[ m_toolCount ]->setToolTip( _makeToolTip( tool, 0 ) );
            }

            connect( m_toolButtons[m_toolCount], SIGNAL(toggled(bool)), this, SLOT(buttonToggled(bool)));

            grp->addAction( m_toolButtons[ m_toolCount ] );

            m_toolCount++;
         }
      }
      else
      {
         m_toolMenu->addSeparator();
      }
      tool = m_toolbox->getNextTool();
   }
}

static void _registerKeyBinding( Command * cmd, int index,
      QMenu * menu, QAction * id )
{
   QString name = QString( "cmd_" ) + QString( cmd->getName( 0 ) );

   if ( index > 0 )
   {
      name += QString( "_" ) + QString( cmd->getName( index ) );
   }

   name = name.replace( QString("."), QString("") );
   name = name.replace( QString(" "), QString("_") );
   name = name.toLower();

   QKeySequence key = g_keyConfig.getKey( (const char *) name.toUtf8() );

   if ( !key.isEmpty() )
   {
      id->setShortcut( key );
   }
}

void ViewWindow::initializeCommands()
{
   //m_cmdMgr = new CommandManager();
   //init_std_cmds( m_cmdMgr );
   m_cmdMgr = CommandManager::getInstance();
   Command * cmd = m_cmdMgr->getFirstCommand();
   while ( cmd )
   {
      if ( !cmd->isSeparator() )
      {
         CommandMenuItemT * item;
         QAction * id;
         int count = cmd->getCommandCount();
         if ( count > 1 )
         {
            QMenu * menu = new QMenu( this );
            for ( int t = 1; t < count; t++ )
            {
               id = menu->addAction(  qApp->translate( "Command", cmd->getName(t) ) );

               item = new CommandMenuItemT;
               item->id = id;
               item->command = cmd;
               item->arg = t;

               _registerKeyBinding( cmd, t, menu, id );
               m_primitiveCommands.push_back( item );
            }

            log_debug( "adding command '%s' to menus\n", cmd->getName(0) );
            id = insertMenuItem( m_geometryMenu, false, cmd->getPath(),
                  qApp->translate( "Command", cmd->getName(0) ), menu );
            //id = m_geometryMenu->addAction( qApp->translate( "Command", cmd->getName(0) ), menu );
            _registerKeyBinding( cmd, 0, m_geometryMenu, id );

            item = new CommandMenuItemT;
            item->id = id;
            item->command = cmd;
            item->arg = 0;

            m_primitiveCommands.push_back( item );
            connect( menu, SIGNAL(triggered(QAction*)), this, SLOT(primitiveCommandActivated(QAction*)) );
         }
         else
         {
            QMenu * curMenu = m_geometryMenu;
            id = insertMenuItem( m_geometryMenu, false, cmd->getPath(),
                   qApp->translate( "Command", cmd->getName(0)), NULL );
            //id = curMenu->addAction( qApp->translate( "Command", cmd->getName(0)) );
            item = new CommandMenuItemT;
            item->id = id;
            item->command = cmd;
            item->arg = 0;

            _registerKeyBinding( cmd, 0, curMenu, id );

            log_debug( "adding command '%s' to menus\n", cmd->getName(0) );
            m_primitiveCommands.push_back( item );
         }
      }
      else
      {
         m_geometryMenu->addSeparator();
      }
      cmd = m_cmdMgr->getNextCommand();
   }
}

void ViewWindow::fillMruMenu()
{
   m_mruMenu->clear();
   for ( unsigned i = 0; i < g_prefs("mru").count(); i++ )
   {
      m_mruMenu->addAction( QString::fromUtf8( g_prefs("mru")[i].stringValue().c_str() ) );
   }
}

void ViewWindow::openMru( QAction * id )
{
   openModelInWindow( id->text().toUtf8() );
}

void ViewWindow::fillScriptMruMenu()
{
   m_scriptMruMenu->clear();
   for ( unsigned i = 0; i < g_prefs("script_mru").count(); i++ )
   {
      m_scriptMruMenu->addAction( QString::fromUtf8( g_prefs("script_mru")[i].stringValue().c_str() ) );
   }
}

void ViewWindow::openScriptMru( QAction * id )
{
   runScript( id->text().toUtf8() );
}

void ViewWindow::openModelEvent()
{
   openModelDialogInWindow();
}

bool ViewWindow::openModel( const char * filename )
{
   bool opened = false;

   log_debug( " file: %s\n", filename );

   Model::ModelErrorE err;
   Model * model = new Model();
   if ( (err = FilterManager::getInstance()->readFile( model, filename )) == Model::ERROR_NONE)
   {
      opened = true;

      model_show_alloc_stats();

      model->setSaved( true );
      ViewWindow * win = new ViewWindow( model, NULL );
      win->getSaved(); // Just so I don't have a warning

      prefs_recent_model( filename );
   }
   else
   {
      if ( Model::operationFailed( err ) )
      {
         QString reason = modelErrStr( err, model );
         reason = QString(filename) + QString(":\n") + reason;
         msg_error( (const char *) reason.toUtf8() );
      }
      delete model;
   }

   return opened;
}

bool ViewWindow::openModelDialog( const char * openDirectory )
{
   bool opened = false;

   list<string> formats = FilterManager::getInstance()->getAllReadTypes();

   QString formatsStr = tr( "All Supported Formats" ) + QString( " (" );

   list<string>::iterator it = formats.begin();
   while(  it != formats.end() )
   {
      formatsStr += QString( (*it).c_str() );

      it++;

      if ( it != formats.end() )
      {
         formatsStr += QString(" ");
      }
   }

   formatsStr += QString(")");

   QString dir = QString::fromUtf8( g_prefs( "ui_model_dir" ).stringValue().c_str() );
   if ( dir.isEmpty() )
   {
      dir = QString( "." );
   }

   if ( openDirectory )
   {
      dir = QString::fromUtf8( openDirectory );
   }

   QFileDialog d(NULL, QString(""), dir, formatsStr + QString(";; ") + tr( "All Files (*)" ) );

   d.setWindowTitle( tr( "Open model file" ) );
   d.selectFilter( formatsStr );

   if ( QDialog::Accepted == d.exec() )
   {
      QStringList files = d.selectedFiles();

      if ( files.empty() )
         return false;

      if ( openModel( files[0].toUtf8() ) )
      {
         opened = true;
         g_prefs( "ui_model_dir" ) = (const char *) d.directory().absolutePath().toUtf8();
      }
   }

   return opened;
}

bool ViewWindow::openModelInWindow( const char * filename )
{
   bool opened = false;

   log_debug( " file: %s\n", filename );

#ifndef CODE_DEBUG
   if ( ! m_model->getSaved() )
   {
      int val = QMessageBox::warning( this, tr("Save first?"), tr("Model has been modified\nDo you want to save before closing?"), QMessageBox::Yes, QMessageBox::No, QMessageBox::Cancel );
      switch ( val )
      {
         case QMessageBox::Yes:
            m_abortQuit = false;
            saveModelEvent();
            if ( m_abortQuit )
            {
               return false;
            }
            break;
         case QMessageBox::No:
            break;
         case QMessageBox::Cancel:
            return false;
            break;
         default:
            {
               msg_error( (const char *) tr("Unknown response: Canceling operation").toUtf8() );
            }
            return false;
      }
   }
#endif // CODE_DEBUG

   Model::ModelErrorE err;
   Model * model = new Model();
   if ( (err = FilterManager::getInstance()->readFile( model, filename )) == Model::ERROR_NONE)
   {
      opened = true;

      model_show_alloc_stats();

      Model * oldModel = m_model;
      setModel( model );
      delete oldModel;

      frameAllEvent();

      prefs_recent_model( filename );
   }
   else
   {
      if ( Model::operationFailed( err ) )
      {
         QString reason = modelErrStr( err, model );
         reason = QString(filename) + QString(":\n") + reason;
         msg_error( (const char *) reason.toUtf8() );
      }
      delete model;
   }

   return opened;
}

bool ViewWindow::openModelDialogInWindow( const char * openDirectory )
{
   bool opened = false;

   list<string> formats = FilterManager::getInstance()->getAllReadTypes();

   QString formatsStr = tr( "All Supported Formats" ) + QString( " (" );

   list<string>::iterator it = formats.begin();
   while(  it != formats.end() )
   {
      formatsStr += QString( (*it).c_str() );

      it++;

      if ( it != formats.end() )
      {
         formatsStr += QString(" ");
      }
   }

   formatsStr += QString(")");

   QString dir = QString::fromUtf8( g_prefs( "ui_model_dir" ).stringValue().c_str() );
   if ( dir.isEmpty() )
   {
      dir = ".";
   }

   if ( openDirectory )
   {
      dir = openDirectory;
   }

   QFileDialog d(NULL, QString(""), dir, formatsStr + QString(";; ") + tr( "All Files (*)" ) );

   d.setWindowTitle( tr( "Open model file" ) );
   d.selectFilter( formatsStr );

   if ( QDialog::Accepted == d.exec() )
   {
      QStringList files = d.selectedFiles();

      if ( files.empty() )
         return false;

      if ( openModelInWindow( files[0].toUtf8() ) )
      {
         opened = true;
         g_prefs( "ui_model_dir" ) = (const char *) d.directory().absolutePath().toUtf8();
      }
   }

   return opened;
}

void ViewWindow::invalidateModelTextures()
{
   ViewWindowList::iterator windowIter;
   
   Model * model;
   DecalManager * mgr = DecalManager::getInstance();

   for( windowIter = _winList.begin(); windowIter != _winList.end(); windowIter++ )
   {
      model = (*windowIter)->getModel();
      model->invalidateTextures();
      mgr->modelUpdated( model );
   }
}

void ViewWindow::quitEvent()
{
   saveDockPositions();

   if ( ViewWindow::closeAllWindows() )
   {
      qApp->quit();
   }
}

void ViewWindow::pluginWindowEvent()
{
   // pluginWin will delete itself view WDestructiveClose
   PluginWindow * pluginWin = new PluginWindow();
   pluginWin->show();
}

void ViewWindow::backgroundWindowEvent()
{
   // pluginWin will delete itself view WDestructiveClose
   BackgroundWin * win = new BackgroundWin( m_model );  
   win->show();
}

void ViewWindow::newModelEvent()
{
   ViewWindow * win = new ViewWindow( new Model, NULL );
   win->getSaved(); // Just so I don't have a warning
}

void ViewWindow::savedTimeoutCheck()
{
   updateCaption();
}

void ViewWindow::saveDockPositions()
{
   QString dockFile( getMm3dHomeDirectory().c_str() );
   dockFile += "/";
   dockFile += DOCK_FILENAME;

   QFile fp( dockFile );
   if ( fp.open( QIODevice::WriteOnly ) )
   {
      {
         // Must go out of scope before fp is closed.
         QDataStream stream(&fp);
         stream << this->saveState( DOCK_VERSION );
      }
      fp.close();
   }
}

void ViewWindow::loadDockPositions()
{
   QString dockFile( getMm3dHomeDirectory().c_str() );
   dockFile += "/";
   dockFile += DOCK_FILENAME;

   QFile fp( dockFile );
   if ( fp.open( QIODevice::ReadOnly ) )
   {
      {
         // Must go out of scope before fp is closed.
         QDataStream stream(&fp);
         QByteArray state;
         stream >> state;
         this->restoreState( state, DOCK_VERSION );
      }
      fp.close();
   }
}

void ViewWindow::updateCaption()
{
   QString caption = QString( "Misfit Model 3D: " );
   if ( m_model )
   {
      caption += m_model->getSaved() ? QString("") : QString("* ");

      const char * filename = m_model->getFilename();
      if ( filename && filename[0] )
      {
         std::string fullName;
         std::string fullPath;
         std::string baseName;
         normalizePath( filename, fullName, fullPath, baseName );

         caption += baseName.c_str();
      }
      else
      {
         caption += tr( "[unnamed]", "For filename in title bar (if not set)" );
      }
   }

   setWindowTitle( caption );
}

