/*  Maverick Model 3D
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

#include <cstdio>

#include "keycfg.h"

#include "log.h"
#include "qtmain.h"
#include "misc.h"
#include "mm3dconfig.h"
#include "filedatadest.h"
#include "filedatasource.h"

#include <QtWidgets/QApplication>

static void _writeDefaultFile( const char * filename )
{
   FileDataDest dst( filename );
   if ( !dst.errorOccurred() )
   {
      const char *msg =
         "; This file is used to change key bindings for Maverick Model 3D." FILE_NEWLINE
         ";" FILE_NEWLINE
         "; To change keyboard shortcuts, add keyboard shortcuts assignments at the bottom " FILE_NEWLINE
         "; of this file. The format for keyboard shortcuts is:" FILE_NEWLINE
         ";" FILE_NEWLINE
         ";    mm3d_command_name  MODIFIER+Key" FILE_NEWLINE
         ";" FILE_NEWLINE
         "; For a full list of MM3D command names, see keycfg.out. The keycfg.out file" FILE_NEWLINE
         "; is written at the end of every MM3D session and includes all the currently" FILE_NEWLINE
         "; assigned shortcuts (including default values and user-specified values)." FILE_NEWLINE
         ";" FILE_NEWLINE
         "; There can be as many spaces between the command name and the keyboard shortcut" FILE_NEWLINE
         "; as you want, but there cannot be any spaces in the keyboard shortcut sequence." FILE_NEWLINE
         ";" FILE_NEWLINE
         "; Valid modifiers are:" FILE_NEWLINE
         ";" FILE_NEWLINE
         ";   Ctrl" FILE_NEWLINE
         ";   Alt" FILE_NEWLINE
         ";   Shift" FILE_NEWLINE
         ";   Meta" FILE_NEWLINE
         ";" FILE_NEWLINE
         "; Modifiers are not case-sensitive. You can have as many modifiers as you want " FILE_NEWLINE
         "; (none, all, or any combination)." FILE_NEWLINE
         ";" FILE_NEWLINE
         "; Most keys can be entered by typing the actual key (letters, numbers, arithmetic" FILE_NEWLINE
         "; operators, etc)." FILE_NEWLINE
         ";" FILE_NEWLINE
         "; Some special keys:" FILE_NEWLINE
         ";" FILE_NEWLINE
         ";   Del" FILE_NEWLINE
         ";   PgDown" FILE_NEWLINE
         ";   PgUp" FILE_NEWLINE
         ";   Space" FILE_NEWLINE
         ";   Print   (Print Screen key)" FILE_NEWLINE
         ";   Up" FILE_NEWLINE
         ";   Down" FILE_NEWLINE
         ";   Left" FILE_NEWLINE
         ";   Right" FILE_NEWLINE
         ";   Esc" FILE_NEWLINE
         ";   Tab" FILE_NEWLINE
         ";   Enter" FILE_NEWLINE
         ";   Return" FILE_NEWLINE
         ";   Backspace" FILE_NEWLINE
         ";   Insert" FILE_NEWLINE
         ";   Ins" FILE_NEWLINE
         ";   Delete" FILE_NEWLINE
         ";   Del" FILE_NEWLINE
         ";   Home" FILE_NEWLINE
         ";   End" FILE_NEWLINE
         ";   F1 through F12" FILE_NEWLINE
         ";" FILE_NEWLINE
         FILE_NEWLINE
         "; Uncomment the following line to assign Shift+F1 to the Help->Contents menu item" FILE_NEWLINE
         ";viewwin_help_contents    Shift+F1" FILE_NEWLINE
         FILE_NEWLINE
         "; Uncomment the following line to remove the keyboard shortcut for File->Save" FILE_NEWLINE
         ";viewwin_file_save " FILE_NEWLINE;

      dst.writeString( msg );
   }
   else
   {
      log_error( "Couldn't write to key config file: %s\n", filename );
   }
}

KeyConfig g_keyConfig;

KeyConfig::KeyConfig()
{
}

KeyConfig::~KeyConfig()
{
}

QKeySequence KeyConfig::getKey( const char * operation )
{
   KeyDataList::iterator it;

   for ( it = m_list.begin(); it != m_list.end(); it++ )
   {
      if ( strcasecmp( operation, (*it).operation.c_str() ) == 0 )
      {
         return (*it).key;
      }
   }

   KeyDataT kd;

   kd.operation = operation;
   m_list.push_back( kd );

   return m_list.back().key;
}

void KeyConfig::setKey( const char * operation, const QKeySequence & key )
{
   KeyDataList::iterator it;

   // Clear any operation key that uses this key (to prevent duplicates)
   if ( !key.isEmpty() )
   {
      for ( it = m_list.begin(); it != m_list.end(); it++ )
      {
         if ( (*it).key == key )
         {
            (*it).key = 0;
         }
      }
   }

   for ( it = m_list.begin(); it != m_list.end(); it++ )
   {
      if ( strcasecmp( operation, (*it).operation.c_str() ) == 0 )
      {
         (*it).key = key;
         return;
      }
   }

   KeyDataT kd;
   kd.operation = operation;
   kd.key = key;

   m_list.push_back( kd );
}

void KeyConfig::setDefaultKey( const char * operation, const QKeySequence & key )
{
   KeyDataList::iterator it;

   // See if this key is already in use (and ignore this request)
   for ( it = m_list.begin(); it != m_list.end(); it++ )
   {
      if ( (*it).key == key )
      {
         return;
      }
   }

   // See if this action is already defined (and ignore this request)
   for ( it = m_list.begin(); it != m_list.end(); it++ )
   {
      if ( strcasecmp( operation, (*it).operation.c_str() ) == 0 )
      {
         return;
      }
   }

   // Key combo not in use, action not already defined, save default
   KeyDataT kd;
   kd.operation = operation;
   kd.key = key;

   m_list.push_back( kd );
}

bool KeyConfig::saveFile( const char * filename )
{
   FileDataDest dst( filename );

   if ( !dst.errorOccurred() )
   {
      KeyDataList::iterator it;

      for ( it = m_list.begin(); it != m_list.end(); it++ )
      {
         QString keyStr = (*it).key.toString();
         if ( keyStr.isNull() )
         {
            dst.writePrintf( "%s " FILE_NEWLINE, (*it).operation.c_str() );
         }
         else
         {
            dst.writePrintf( "%s %s" FILE_NEWLINE, (*it).operation.c_str(), (const char *) keyStr.toUtf8() );
         }
      }

      return true;
   }
   else
   {
      log_error( "Couldn't write to key config file: %s\n", filename );
   }

   return false;
}

bool KeyConfig::loadFile( const char * filename )
{
   log_debug( "loading keyboard shortcut config file:\n" );
   log_debug( "   %s\n", filename );
   FileDataSource src( filename );

   if ( !src.errorOccurred() )
   {
      log_debug( "reading file...\n" );
      m_list.clear();

      char line[1024];
      while ( src.readLine( line, sizeof(line) ) )
      {
         chomp( line );

         if ( isspace(line[0]) 
               || line[0] == ';' 
               || line[0] == '#' 
               || strncmp(line, "//", 2 ) == 0 )
         {
            // comment, ignore it
         }
         else if ( line[0] )
         {
            // not blank or comment line, parse it
            char * space = strchr( line, ' ' );
            if ( space )
            {
               std::string op;
               op.assign( line, space - line );

               char * str = space;
               QKeySequence key;

               if ( op.size() > 0 )
               {
                  while ( isspace( str[0] ) )
                  {
                     str++;
                  }

                  // empty definitions are valid
                  QString s( str );
                  QKeySequence k( s );
                  setKey( op.c_str(), k );
               }
            }
            else
            {
               std::string op = line;
               if ( op.size() > 0 )
               {
                  QKeySequence k;
                  setKey( op.c_str(), k );
               }
            }
         }
      }

      return true;
   }
   else
   {
      log_debug( "keyboard config file does not exist, creating...\n" );
      _writeDefaultFile( filename );
   }
   return false;
}

/*
int KeyConfig::getSpecialKey( const char * keyName )
{
   for ( unsigned int k = 0; k < KEY_NAMES; k++ )
   {
      if ( strcasecmp( keyName, _special[k].operation.c_str() ) == 0 )
      {
         return _special[k].key;
      }
   }

   return 0;
}
*/

/*
std::string KeyConfig::getSpecialKeyName( int key )
{
   for ( unsigned int k = 0; k < KEY_NAMES; k++ )
   {
      if ( key == _special[k].key )
      {
         return _special[k].operation;
      }
   }

   return "";
}
*/

bool keycfg_load_file( const char * filename )
{
   return g_keyConfig.loadFile( filename );
}

bool keycfg_save_file( const char * filename )
{
   return g_keyConfig.saveFile( filename );
}

void keycfg_set_defaults()
{
   QApplication * a = ui_getapp();

   // Tools
   g_keyConfig.setDefaultKey( "tool_snap_to_grid", 0 );
   g_keyConfig.setDefaultKey( "tool_snap_to_vertex", 0 );
   g_keyConfig.setDefaultKey( "tool_select_vertices", QKeySequence( a->translate( "KeyConfig", "V", "Select Vertices Tool Shortcut" )) );
   g_keyConfig.setDefaultKey( "tool_select_faces", QKeySequence( a->translate( "KeyConfig", "F", "Select Faces Tool Shortcut")) );
   g_keyConfig.setDefaultKey( "tool_select_connected_mesh", QKeySequence( a->translate( "KeyConfig", "C", "Select Connected Mesh Tool Shortcut")) );
   g_keyConfig.setDefaultKey( "tool_select_groups", QKeySequence( a->translate( "KeyConfig", "G", "Select Groups Tool Shortcut")) );
   g_keyConfig.setDefaultKey( "tool_select_bone_joints", QKeySequence( a->translate( "KeyConfig", "B", "Select Bone Joints Tool Shortcut")) );
   g_keyConfig.setDefaultKey( "tool_select_points", QKeySequence( a->translate( "KeyConfig", "T", "Select Points Tool Shortcut")) );
   g_keyConfig.setDefaultKey( "tool_select_projections", 0 );
   g_keyConfig.setDefaultKey( "tool_move", QKeySequence( a->translate( "KeyConfig", "M", "Move Tool Shortcut")) );
   g_keyConfig.setDefaultKey( "tool_rotate", QKeySequence( a->translate( "KeyConfig", "R", "Rotate Tool Shortcut")) );
   g_keyConfig.setDefaultKey( "tool_scale", 0 );
   g_keyConfig.setDefaultKey( "tool_shear", 0 );
   g_keyConfig.setDefaultKey( "tool_extrude", 0 );
   g_keyConfig.setDefaultKey( "tool_drag_vertex_on_edge", 0 );
   g_keyConfig.setDefaultKey( "tool_attract_near", 0 );
   g_keyConfig.setDefaultKey( "tool_attract_far", 0 );
   g_keyConfig.setDefaultKey( "tool_move_background_image", 0 );
   g_keyConfig.setDefaultKey( "tool_scale_background_image", 0 );
   g_keyConfig.setDefaultKey( "tool_create_vertex", 0 );
   g_keyConfig.setDefaultKey( "tool_create_rectangle", 0 );
   g_keyConfig.setDefaultKey( "tool_create_cube", 0 );
   g_keyConfig.setDefaultKey( "tool_create_ellipsoid", 0 );
   g_keyConfig.setDefaultKey( "tool_create_cylinder", 0 );
   g_keyConfig.setDefaultKey( "tool_create_torus", 0 );
   g_keyConfig.setDefaultKey( "tool_create_polygon", 0 );
   g_keyConfig.setDefaultKey( "tool_create_bone_joint", 0 );
   g_keyConfig.setDefaultKey( "tool_create_point", 0 );
   g_keyConfig.setDefaultKey( "tool_create_projection", 0 );

   // Commands
   g_keyConfig.setDefaultKey( "cmd_invert_selection", 0 );
   g_keyConfig.setDefaultKey( "cmd_select_free_vertices", 0 );
   g_keyConfig.setDefaultKey( "cmd_hide_hide_unselected", QKeySequence( a->translate( "KeyConfig", "H", "Hide Unselected Command Shortcut")) );
   g_keyConfig.setDefaultKey( "cmd_hide_hide_selected", QKeySequence( a->translate( "KeyConfig", "Shift+H", "Hide Selected Command Shortcut")) );
   g_keyConfig.setDefaultKey( "cmd_hide_unhide_all", QKeySequence( a->translate( "KeyConfig", "Shift+U", "Unhide All Command Shortcut")) );
   g_keyConfig.setDefaultKey( "cmd_hide", 0 );
   g_keyConfig.setDefaultKey( "cmd_delete", QKeySequence( a->translate( "KeyConfig", "Delete", "Delete Command Shortcut")) );
   g_keyConfig.setDefaultKey( "cmd_duplicate", QKeySequence( a->translate( "KeyConfig", "Ctrl+D", "Duplicate Command Shortcut")) );
   g_keyConfig.setDefaultKey( "cmd_copy_selected_to_clipboard", QKeySequence( a->translate( "KeyConfig", "Ctrl+C", "Copy to Clipboard Command Shortcut")) );
   g_keyConfig.setDefaultKey( "cmd_paste_from_clipboard", QKeySequence( a->translate( "KeyConfig", "Ctrl+V", "Paste from Clipboard Command Shortcut")) );
   g_keyConfig.setDefaultKey( "cmd_flip_flip_x", 0 );
   g_keyConfig.setDefaultKey( "cmd_flip_flip_y", 0 );
   g_keyConfig.setDefaultKey( "cmd_flip_flip_z", 0 );
   g_keyConfig.setDefaultKey( "cmd_flip", 0 );
   g_keyConfig.setDefaultKey( "cmd_flatten_flatten_x", 0 );
   g_keyConfig.setDefaultKey( "cmd_flatten_flatten_y", 0 );
   g_keyConfig.setDefaultKey( "cmd_flatten_flatten_z", 0 );
   g_keyConfig.setDefaultKey( "cmd_flatten", 0 );
   g_keyConfig.setDefaultKey( "cmd_extrude", QKeySequence( a->translate( "KeyConfig", "Insert", "Extrude Command Shortcut")) );
   g_keyConfig.setDefaultKey( "cmd_invert_normals", 0 );
   g_keyConfig.setDefaultKey( "cmd_weld_vertices", QKeySequence( a->translate( "KeyConfig", "Ctrl+W", "Weld Command Shortcut")) );
   g_keyConfig.setDefaultKey( "cmd_unweld_vertices", 0 );
   g_keyConfig.setDefaultKey( "cmd_snap_vertices_together_snap_all_selected", 0 );
   g_keyConfig.setDefaultKey( "cmd_snap_vertices_together_snap_nearest_selected", 0 );
   g_keyConfig.setDefaultKey( "cmd_snap_vertices_together_snap_all_and_weld", 0 );
   g_keyConfig.setDefaultKey( "cmd_snap_vertices_together_snap_nearest_and_weld", 0 );
   g_keyConfig.setDefaultKey( "cmd_snap_vertices_together", 0 );
   g_keyConfig.setDefaultKey( "cmd_edge_turn", 0 );
   g_keyConfig.setDefaultKey( "cmd_edge_divide", 0 );
   g_keyConfig.setDefaultKey( "cmd_subdivide_faces", 0 );
   g_keyConfig.setDefaultKey( "cmd_make_face_from_vertices", 0 );
   g_keyConfig.setDefaultKey( "cmd_cap_holes", 0 );
   g_keyConfig.setDefaultKey( "cmd_rotate_texture_coordinates_face", 0 );
   g_keyConfig.setDefaultKey( "cmd_rotate_texture_coordinates_group", 0 );
   g_keyConfig.setDefaultKey( "cmd_rotate_texture_coordinates", 0 );
   g_keyConfig.setDefaultKey( "cmd_align_selected", 0 );
   g_keyConfig.setDefaultKey( "cmd_spherify", 0 );
   g_keyConfig.setDefaultKey( "cmd_simplify_mesh", 0 );

   // View Window
   g_keyConfig.setDefaultKey( "viewwin_file_new", QKeySequence( a->translate( "KeyConfig", "Ctrl+N", "File | New Window Shortcut")) );
   g_keyConfig.setDefaultKey( "viewwin_file_open", QKeySequence( a->translate( "KeyConfig", "Ctrl+O", "File | Open Shortcut")) );
   g_keyConfig.setDefaultKey( "viewwin_file_save", QKeySequence( a->translate( "KeyConfig", "Ctrl+S", "File | Save Shortcut")) );
   g_keyConfig.setDefaultKey( "viewwin_file_save_as", 0 );
   g_keyConfig.setDefaultKey( "viewwin_file_export", 0 );
   g_keyConfig.setDefaultKey( "viewwin_file_export_selected", 0 );
   g_keyConfig.setDefaultKey( "viewwin_file_run_script", 0 );
   g_keyConfig.setDefaultKey( "viewwin_file_plugins", 0 );
   g_keyConfig.setDefaultKey( "viewwin_file_close", 0 );
   g_keyConfig.setDefaultKey( "viewwin_file_quit", QKeySequence( a->translate( "KeyConfig", "Ctrl+Q", "File | Quit Shortcut")) );
   g_keyConfig.setDefaultKey( "viewwin_view_render_joints_hide", 0 );
   g_keyConfig.setDefaultKey( "viewwin_view_render_joints_lines", 0 );
   g_keyConfig.setDefaultKey( "viewwin_view_render_joints_bones", 0 );
   g_keyConfig.setDefaultKey( "viewwin_view_render_projections_show", 0 );
   g_keyConfig.setDefaultKey( "viewwin_view_render_projections_hide", 0 );
   g_keyConfig.setDefaultKey( "viewwin_view_render_badtex_red", 0 );
   g_keyConfig.setDefaultKey( "viewwin_view_render_badtex_blank", 0 );
   g_keyConfig.setDefaultKey( "viewwin_view_render_3d_lines_show", 0 );
   g_keyConfig.setDefaultKey( "viewwin_view_render_3d_lines_hide", 0 );
   g_keyConfig.setDefaultKey( "viewwin_view_render_backface_show", 0 );
   g_keyConfig.setDefaultKey( "viewwin_view_render_backface_hide", 0 );
   g_keyConfig.setDefaultKey( "viewwin_view_frame_all", QKeySequence( a->translate( "KeyConfig", "Home", "View | Frame All Shortcut")) );
   g_keyConfig.setDefaultKey( "viewwin_view_frame_selected", QKeySequence( a->translate( "KeyConfig", "Shift+Home", "View | Frame Selected Shortcut")) );
   g_keyConfig.setDefaultKey( "viewwin_view_show_properties", 0 );
   g_keyConfig.setDefaultKey( "viewwin_view_3d_wireframe", 0 );
   g_keyConfig.setDefaultKey( "viewwin_view_3d_flat", 0 );
   g_keyConfig.setDefaultKey( "viewwin_view_3d_smooth", 0 );
   g_keyConfig.setDefaultKey( "viewwin_view_3d_textured", 0 );
   g_keyConfig.setDefaultKey( "viewwin_view_3d_alpha", 0 );
   g_keyConfig.setDefaultKey( "viewwin_view_ortho_wireframe", 0 );
   g_keyConfig.setDefaultKey( "viewwin_view_ortho_flat", 0 );
   g_keyConfig.setDefaultKey( "viewwin_view_ortho_smooth", 0 );
   g_keyConfig.setDefaultKey( "viewwin_view_ortho_textured", 0 );
   g_keyConfig.setDefaultKey( "viewwin_view_ortho_alpha", 0 );
   g_keyConfig.setDefaultKey( "viewwin_view_1", 0 );
   g_keyConfig.setDefaultKey( "viewwin_view_1x2", 0 );
   g_keyConfig.setDefaultKey( "viewwin_view_2x1", 0 );
   g_keyConfig.setDefaultKey( "viewwin_view_2x2", 0 );
   g_keyConfig.setDefaultKey( "viewwin_view_2x3", 0 );
   g_keyConfig.setDefaultKey( "viewwin_view_3x2", 0 );
   g_keyConfig.setDefaultKey( "viewwin_view_3x3", 0 );
   g_keyConfig.setDefaultKey( "viewwin_view_viewport_settings", 0 );
   g_keyConfig.setDefaultKey( "viewwin_model_edit_meta_data", 0 );
   g_keyConfig.setDefaultKey( "viewwin_model_transform", 0 );
   g_keyConfig.setDefaultKey( "viewwin_model_boolean_operation", 0 );
   g_keyConfig.setDefaultKey( "viewwin_model_set_background_image", 0 );
   g_keyConfig.setDefaultKey( "viewwin_model_merge", 0 );
   g_keyConfig.setDefaultKey( "viewwin_model_import_animations", 0 );
   g_keyConfig.setDefaultKey( "viewwin_groups_edit_groups", QKeySequence( a->translate( "KeyConfig", "Ctrl+G", "Groups | Edit Groups Shortcut")) );
   g_keyConfig.setDefaultKey( "viewwin_groups_edit_materials", QKeySequence( a->translate( "KeyConfig", "Ctrl+M", "Groups | Edit Materials Shortcut")) );
   g_keyConfig.setDefaultKey( "viewwin_groups_cleanup", 0 );
   g_keyConfig.setDefaultKey( "viewwin_groups_reload_textures", 0 );
   g_keyConfig.setDefaultKey( "viewwin_groups_edit_projection", 0 );
   g_keyConfig.setDefaultKey( "viewwin_groups_edit_texture_coordinates", QKeySequence( a->translate( "KeyConfig", "Ctrl+E", "Groups | Edit Texture Coordinates Shortcut")) );
   g_keyConfig.setDefaultKey( "viewwin_groups_paint_texture", 0 );
   g_keyConfig.setDefaultKey( "viewwin_joints_edit_joints", 0 );
   g_keyConfig.setDefaultKey( "viewwin_joints_assign_selected", QKeySequence( a->translate( "KeyConfig", "Ctrl+B", "Joints | Assign Selected Shortcut")) );
   g_keyConfig.setDefaultKey( "viewwin_joints_auto_assign_selected", 0 );
   g_keyConfig.setDefaultKey( "viewwin_joints_remove_influences", 0 );
   g_keyConfig.setDefaultKey( "viewwin_joints_remove_joint", 0 );
   g_keyConfig.setDefaultKey( "viewwin_joints_make_single_influence", 0 );
   g_keyConfig.setDefaultKey( "viewwin_joints_select_joint_influences", 0 );
   g_keyConfig.setDefaultKey( "viewwin_joints_select_influenced_vertices", 0 );
   g_keyConfig.setDefaultKey( "viewwin_joints_select_influenced_points", 0 );
   g_keyConfig.setDefaultKey( "viewwin_joints_select_unassigned_vertices", 0 );
   g_keyConfig.setDefaultKey( "viewwin_joints_select_unassigned_points", 0 );
   g_keyConfig.setDefaultKey( "viewwin_anim_start_mode", 0 );
   g_keyConfig.setDefaultKey( "viewwin_anim_stop_mode", 0 );
   g_keyConfig.setDefaultKey( "viewwin_anim_animation_sets", 0 );
   g_keyConfig.setDefaultKey( "viewwin_anim_save_images", 0 );
   g_keyConfig.setDefaultKey( "viewwin_anim_frame_copy", 0 );
   g_keyConfig.setDefaultKey( "viewwin_anim_frame_paste", 0 );
   g_keyConfig.setDefaultKey( "viewwin_anim_frame_clear", 0 );
   g_keyConfig.setDefaultKey( "viewwin_anim_selected_copy", 0 );
   g_keyConfig.setDefaultKey( "viewwin_anim_selected_paste", 0 );
   g_keyConfig.setDefaultKey( "viewwin_anim_set_rotation", 0 );
   g_keyConfig.setDefaultKey( "viewwin_anim_set_translation", 0 );
   g_keyConfig.setDefaultKey( "viewwin_anim_clear_rotation", 0 );
   g_keyConfig.setDefaultKey( "viewwin_anim_clear_translation", 0 );
   g_keyConfig.setDefaultKey( "viewwin_help_contents", 0 );
   g_keyConfig.setDefaultKey( "viewwin_help_license", 0 );
   g_keyConfig.setDefaultKey( "viewwin_help_about", 0 );
}

