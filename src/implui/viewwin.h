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

#include "contextpanelobserver.h"

#include <list>

using std::list;

class QVBoxLayout;
class QMenuBar;
class QMenu;
class QToolBar;

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
class AnimWindow;
class QToolButton;

class Toolbox;
class CommandManager;

class QContextMenuEvent;
class QCloseEvent;
class QResizeEvent;

#include <QtWidgets/QMainWindow>
#include <QtCore/QObject>

class CommandWidget : public QObject
{
   Q_OBJECT

   public:
      CommandWidget( QObject * parent, Model * model, bool * canEdit,
            Command * cmd, int index );
      virtual ~CommandWidget();

   public slots:
      void setModel( Model * m );
      void activateCommand( bool );

   private:
      Model * m_model;
      bool * m_canEdit;
      Command * m_cmd;
      int m_index;
};

class ViewWindow : public QMainWindow, public ContextPanelObserver
{
   Q_OBJECT

   public:
      ViewWindow( Model * model, QWidget * parent = NULL );
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
      void helpNowEvent();

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
      void groupCleanWindowEvent();
      void textureCoordEvent();
      void paintTextureEvent();
      void projectionWindowEvent();
      void transformWindowEvent();
      void metaWindowEvent();
      void boolWindowEvent();
      void reloadTexturesEvent();

      void buttonToggled( bool on );

      void toolActivated( QAction * id );
      void scriptActivated( QAction * id );

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

      void animationModeOn();
      void animationModeOff();

      void contextPanelHidden();

      void editDisableEvent();
      void editEnableEvent();

      void undoRequest();
      void redoRequest();

      void snapToSelectedEvent( QAction * snapTo );

      void fillMruMenu();
      void openMru( QAction * id );

      void fillScriptMruMenu();
      void openScriptMru( QAction * id );

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
      QAction * insertMenuItem( QMenu * parentMenu, bool isTool,
            const QString & path, const QString & name, QMenu * subMenu );

      struct _ToolMenuItem_t
      {
         QAction * id;
         ::Tool * tool;
         int arg;
      };
      typedef struct _ToolMenuItem_t ToolMenuItemT;

      typedef list< ToolMenuItemT * > ToolMenuItemList;

      typedef struct _CommandMenuItem_t
      {
         QAction * id;
         Command * command;
         CommandWidget * widget;
         int arg;
      } CommandMenuItemT;

      typedef list< CommandMenuItemT * > CommandMenuItemList;

      typedef struct _MenuItem_t
      {
         QString      text;
         QMenu * menu;
      } MenuItemT;

      typedef list< MenuItemT > MenuItemList;

      QMenuBar    * m_menuBar;
      QMenu  * m_fileMenu;
      QMenu  * m_viewMenu;
      QMenu  * m_renderMenu;
      QMenu  * m_toolMenu;
      QMenu  * m_modelMenu;
      QMenu  * m_geometryMenu;
      QMenu  * m_materialsMenu;
      QMenu  * m_jointsMenu;
      QMenu  * m_animMenu;
      QMenu  * m_scriptMenu;
      QMenu  * m_helpMenu;
      QMenu  * m_mruMenu;
      QMenu  * m_scriptMruMenu;
      QMenu  * m_snapMenu;

      QToolBar    * m_toolBar;

      ViewPanel    * m_viewPanel;
      ContextPanel * m_contextPanel;
      BoolPanel    * m_boolPanel;
      ProjectionWin * m_projectionWin;
      TextureCoord * m_textureCoordWin;
      TransformWindow * m_transformWin;
      StatusBar   * m_statusBar;
      Model       * m_model;
      AnimWindow  * m_animWin;
      AnimWidget  * m_animWidget;

      QAction *     m_snapToGrid;
      QAction *     m_snapToVertex;

      QAction *     m_animSetsItem;
      QAction *     m_animExportItem;
      QAction *     m_animSetRotItem;
      QAction *     m_animSetTransItem;
      QAction *     m_animCopyFrame;
      QAction *     m_animPasteFrame;
      QAction *     m_animCopySelected;
      QAction *     m_animPasteSelected;
      QAction *     m_animClearFrame;
      QAction *     m_startAnimItem;
      QAction *     m_stopAnimItem;
      QAction *     m_showContext;

      QAction *     m_3dWire;
      QAction *     m_3dFlat;
      QAction *     m_3dSmooth;
      QAction *     m_3dTexture;
      QAction *     m_3dAlpha;

      QAction *     m_canvasWire;
      QAction *     m_canvasFlat;
      QAction *     m_canvasSmooth;
      QAction *     m_canvasTexture;
      QAction *     m_canvasAlpha;

      QAction *     m_view1;
      QAction *     m_view1x2;
      QAction *     m_view2x1;
      QAction *     m_view2x2;
      QAction *     m_view2x3;
      QAction *     m_view3x2;
      QAction *     m_view3x3;

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
      QAction ** m_toolButtons;
      QAction * m_last;
      ::Tool        * m_currentTool;
      bool m_canEdit;

      QTimer      * m_savedTimer;
};


#endif // __VIEWWIN_H
