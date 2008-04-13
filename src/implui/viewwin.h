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


#ifndef __VIEWWIN_H
#define __VIEWWIN_H

#include "config.h"

#include "mq3macro.h"
#include "mq3compat.h"

#include "contextpanelobserver.h"

#include <list>


using std::list;

class QVBoxLayout;
class QMenuBar;
class QPopupMenu;
#ifdef HAVE_QT4
class Q3ToolBar;
#else 
class QToolBar;
#endif 

class QTimer;
class ViewPanel;
class ContextPanel;
class BoolPanel;
class ProjectionWin;
class TextureCoord;
class TransformWindow;
class StatusBar;
class Model;

class Command;
class Tool;
class Script;

class AnimWidget;
class QToolButton;

class Toolbox;
class CommandManager;
class QAccel;

class ViewWindow : public QMainWindow, public ContextPanelObserver
{
   Q_OBJECT

   public:
      ViewWindow( Model * model, QWidget * parent = NULL, const char * name = "" );
      virtual ~ViewWindow();

      static bool closeAllWindows();

      static bool openModel( const char * filename );
      static bool openModelDialog( const char * openDirectory = NULL );

      static void invalidateModelTextures();

      void setModel( Model * model );

      bool openModelInWindow( const char * filename );
      bool openModelDialogInWindow( const char * openDirectory = NULL );

      bool getSaved();
      bool getAbortQuit() { return m_abortQuit; };
      void setAbortQuit( bool o ) { m_abortQuit = o; };
      
      Model *getModel() { return m_model; };

      // ContextPanelObserver methods
      void showProjectionEvent();

   signals:
      void modelChanged( Model * m );

   public slots:
      void helpNowEvent( int );

      void saveModelEvent();
      void saveModelAsEvent();
      void exportModelEvent();
      void exportSelectedEvent();

      void mergeModelsEvent();
      void mergeAnimationsEvent();
      void scriptEvent();
      void runScript( const char * filename );

      void frameAllEvent();
      void frameSelectedEvent();

      void showContextEvent();

      void renderBadEvent();
      void noRenderBadEvent();
      void renderSelectionEvent();
      void noRenderSelectionEvent();
      void renderBackface();
      void noRenderBackface();
      void renderProjections();
      void noRenderProjections();
      void boneJointHide();
      void boneJointLines();
      void boneJointBones();

      void viewportSettingsEvent();

      void groupWindowEvent();
      void textureWindowEvent();
      void textureCoordEvent();
      void paintTextureEvent();
      void projectionWindowEvent();
      void transformWindowEvent();
      void metaWindowEvent();
      void boolWindowEvent();
      void reloadTexturesEvent();

      void buttonToggled( bool on );

      void toolActivated( int id );
      void primitiveCommandActivated( int id );
      void scriptActivated( int id );

      void animSetWindowEvent();
      void animExportWindowEvent();
      void animSetRotEvent();
      void animSetTransEvent();

      void animCopyFrameEvent();
      void animPasteFrameEvent();
      void animClearFrameEvent();
      void animCopySelectedEvent();
      void animPasteSelectedEvent();

      void startAnimationMode();
      void stopAnimationMode();
      void animationModeDone();

      void contextPanelHidden();

      void editDisableEvent();
      void editEnableEvent();

      void undoRequest();
      void redoRequest();

      void snapToSelectedEvent( int snapTo );

      void fillMruMenu();
      void openMru( int id );

      void fillScriptMruMenu();
      void openScriptMru( int id );

      void openModelEvent();
      void newModelEvent();
      void quitEvent();

      void pluginWindowEvent();
      void backgroundWindowEvent();
      void helpWindowEvent();
      void aboutWindowEvent();
      void licenseWindowEvent();

      void savedTimeoutCheck();

      // influences slots
      void jointWinEvent();
      void jointAssignSelectedToJoint();
      void jointAutoAssignSelected();
      void jointRemoveInfluencesFromSelected();
      void jointRemoveInfluenceJoint();
      void jointMakeSingleInfluence();
      void jointSelectInfluenceJoints();
      void jointSelectInfluencedVertices();
      void jointSelectInfluencedPoints();
      void jointSelectUnassignedVertices();
      void jointSelectUnassignedPoints();

   protected:
      void saveModelInternal( Model * model, bool exportModel = false );

      void contextMenuEvent( QContextMenuEvent * e );

      void saveDockPositions();
      void loadDockPositions();
      void updateCaption();

      void initializeToolbox();
      void initializeCommands();

      void closeEvent( QCloseEvent * e );
      void resizeEvent( QResizeEvent * );

      // returns id in menu
      int insertMenuItem( QPopupMenu * parentMenu,
            const QString & path, const QString & name, QPopupMenu * subMenu );

      struct _ToolMenuItem_t
      {
         int id;
         ::Tool * tool;
         int arg;
      };
      typedef struct _ToolMenuItem_t ToolMenuItemT;

      typedef list< ToolMenuItemT * > ToolMenuItemList;

      typedef struct _CommandMenuItem_t
      {
         int id;
         Command * command;
         int arg;
      } CommandMenuItemT;

      typedef list< CommandMenuItemT * > CommandMenuItemList;

      typedef struct _MenuItem_t
      {
         QString      text;
         QPopupMenu * menu;
      } MenuItemT;

      typedef list< MenuItemT > MenuItemList;

      QAccel      * m_accel;
      QMenuBar    * m_menuBar;
      QPopupMenu  * m_fileMenu;
      QPopupMenu  * m_viewMenu;
#ifdef HAVE_QT4
      QMenu       * m_renderMenu;
#else
      QPopupMenu  * m_renderMenu;
#endif // HAVE_QT4
      QPopupMenu  * m_toolMenu;
      QPopupMenu  * m_modelMenu;
      QPopupMenu  * m_geometryMenu;
      QPopupMenu  * m_materialsMenu;
      QPopupMenu  * m_jointsMenu;
      QPopupMenu  * m_animMenu;
      QPopupMenu  * m_scriptMenu;
      QPopupMenu  * m_helpMenu;
      QPopupMenu  * m_mruMenu;
      QPopupMenu  * m_scriptMruMenu;
      QPopupMenu  * m_snapMenu;
#ifdef HAVE_QT4
      Q3ToolBar    * m_toolBar;
#else
      QToolBar   * m_toolBar;
#endif
      ViewPanel    * m_viewPanel;
      ContextPanel * m_contextPanel;
      BoolPanel    * m_boolPanel;
      ProjectionWin * m_projectionWin;
      TextureCoord * m_textureCoordWin;
      TransformWindow * m_transformWin;
      StatusBar   * m_statusBar;
      Model       * m_model;
      QDockWindow * m_animWin;
      AnimWidget  * m_animWidget;

      int           m_animSetsItem;
      int           m_animExportItem;
      int           m_animSetRotItem;
      int           m_animSetTransItem;
      int           m_animCopyFrame;
      int           m_animPasteFrame;
      int           m_animCopySelected;
      int           m_animPasteSelected;
      int           m_animClearFrame;
      int           m_startAnimItem;
      int           m_stopAnimItem;
      int           m_showContext;

#ifdef HAVE_QT4
      QAction *     m_renderBadItem;
      QAction *     m_noRenderBadItem;
      QAction *     m_renderSelectionItem;
      QAction *     m_noRenderSelectionItem;
      QAction *     m_hideJointsItem;
      QAction *     m_drawJointLinesItem;
      QAction *     m_drawJointBonesItem;
      QAction *     m_renderBackface;
      QAction *     m_noRenderBackface;
      QAction *     m_renderProjections;
      QAction *     m_noRenderProjections;
#else // Qt 3.x
      int           m_renderBadItem;
      int           m_noRenderBadItem;
      int           m_renderSelectionItem;
      int           m_noRenderSelectionItem;
      int           m_hideJointsItem;
      int           m_drawJointLinesItem;
      int           m_drawJointBonesItem;
      int           m_renderBackface;
      int           m_noRenderBackface;
      int           m_renderProjections;
      int           m_noRenderProjections;
#endif // HAVE_QT4

      bool          m_abortQuit;

      CommandMenuItemList m_primitiveCommands;
      CommandMenuItemList m_groupCommands;
      ToolMenuItemList m_tools;

      MenuItemList m_menuItems;

      CommandManager * m_cmdMgr;

      // Moved from toolwidget
      int m_toolCount;
      Toolbox     * m_toolbox;
      ::Tool        ** m_toolList;
      QToolButton ** m_toolButtons;
      QToolButton * m_last;
      ::Tool        * m_currentTool;
      bool m_canEdit;

      QTimer      * m_savedTimer;
};


#endif // __VIEWWIN_H
