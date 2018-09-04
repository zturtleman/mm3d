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


#ifndef __CONTEXTINFLUENCES_H
#define __CONTEXTINFLUENCES_H

#include "contextinfluences.base.h"

#include <QtWidgets/QWidget>

#include "contextwidget.h"

#include "model.h"

class ContextInfluences : public QWidget, public Ui::ContextInfluencesBase, public ContextWidget
{
   Q_OBJECT
   public:
      ContextInfluences( QWidget * parent );
      virtual ~ContextInfluences();

      // ContextWidget methods
      void setModel( Model * );
      void modelChanged( int changeBits );
      bool isUpdating() { return m_update; };

   signals:
      void panelChange();

   public slots:
      // Influence slots
      void joint1Changed();
      void joint2Changed();
      void joint3Changed();
      void joint4Changed();
      void type1Changed();
      void type2Changed();
      void type3Changed();
      void type4Changed();
      void weight1Changed( const QString & );
      void weight2Changed( const QString & );
      void weight3Changed( const QString & );
      void weight4Changed( const QString & );

   protected:
      class JointCount
      {
         public:
            JointCount();
            bool inList;
            int count;
            int typeCount[ Model::IT_MAX ];
            int typeIndex;
            int weight;
      };
      typedef std::vector< JointCount > JointCountList;

      void jointChanged(  int index, int oldJoint, int newJoint );
      void weightChanged( int index, double weight );
      void typeChanged( int index );

      void updateRemainders();
      void updateWeightField( int index, bool enabled, int type, int weight );
      int getRemainderWeight( int joint );

      Model * m_model;
      bool    m_change;
      bool    m_update;

      int     m_joints[4];
      JointCountList m_jclist;
};

#endif // __CONTEXTINFLUENCES_H
