/*  Misfit Model 3D
 * 
 *  Copyright (c) 2008 Kevin Worcester
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


#ifndef TEST_COMMON_H__
#define TEST_COMMON_H__

#include "modelstatus.h"

#include <vector>
#include <QtTest/QtTest>

/*
QString ConvertValToString( float val );
QString ConvertValToString( double val );
*/

QString ConvertValToString( int val );
QString ConvertValToString( const std::string & val );

template<typename T>
void CompareValLessThan(const T & lhs, const T & rhs,
      const char * lhs_text, const char * rhs_text,
      const char * file, int line )
{
   if ( !(lhs < rhs) )
   {
      QString msg = QString("'") + QString(lhs_text) + " < " + QString(rhs_text);
      msg += "' eval: ";
      msg += ConvertValToString( lhs ) + QString(" < ") + ConvertValToString( rhs );
      QTest::qFail( msg.toUtf8(), file, line );
   }
}

template<typename T>
void CompareValGreaterThan(const T & lhs, const T & rhs,
      const char * lhs_text, const char * rhs_text,
      const char * file, int line )
{
   if ( !(lhs > rhs) )
   {
      QString msg = QString("'") + QString(lhs_text) + " > " + QString(rhs_text);
      msg += "' eval: ";
      msg += ConvertValToString( lhs ) + QString(" > ") + ConvertValToString( rhs );
      QTest::qFail( msg.toUtf8(), file, line );
   }
}

template<typename T>
void CompareValLessEqual(const T & lhs, const T & rhs,
      const char * lhs_text, const char * rhs_text,
      const char * file, int line )
{
   if ( !(lhs <= rhs) )
   {
      QString msg = QString("'") + QString(lhs_text) + " <= " + QString(rhs_text);
      msg += "' eval: ";
      msg += ConvertValToString( lhs ) + QString(" <= ") + ConvertValToString( rhs );
      QTest::qFail( msg.toUtf8(), file, line );
   }
}

template<typename T>
void CompareValGreaterEqual(const T & lhs, const T & rhs,
      const char * lhs_text, const char * rhs_text,
      const char * file, int line )
{
   if ( !(lhs >= rhs) )
   {
      QString msg = QString("'") + QString(lhs_text) + " >= " + QString(rhs_text);
      msg += "' eval: ";
      msg += ConvertValToString( lhs ) + QString(" >= ") + ConvertValToString( rhs );
      QTest::qFail( msg.toUtf8(), file, line );
   }
}

/*
// FIXME look up template specialization
void CompareValEqual(const double & lhs, const double & rhs,
      const char * lhs_text, const char * rhs_text,
      const char * file, int line )
{
   printf( "double compare!\n" );
   if ( fabs(lhs - rhs) > 0.00001 )
   {
      QString msg = QString("'") + QString(lhs_text) + " == " + QString(rhs_text);
      msg += "' eval: ";
      msg += ConvertValToString( lhs ) + QString(" == ") + ConvertValToString( rhs );
      QTest::qFail( msg.toUtf8(), file, line );
   }
}

void CompareValEqual(const float & lhs, const float & rhs,
      const char * lhs_text, const char * rhs_text,
      const char * file, int line )
{
   printf( "float compare!\n" );
   if ( fabs(lhs - rhs) > 0.00001 )
   {
      QString msg = QString("'") + QString(lhs_text) + " == " + QString(rhs_text);
      msg += "' eval: ";
      msg += ConvertValToString( lhs ) + QString(" == ") + ConvertValToString( rhs );
      QTest::qFail( msg.toUtf8(), file, line );
   }
}
*/

template<typename T>
void CompareValEqual(const T & lhs, const T & rhs,
      const char * lhs_text, const char * rhs_text,
      const char * file, int line )
{
   if ( !(lhs == rhs) )
   {
      QString msg = QString("'") + QString(lhs_text) + " == " + QString(rhs_text);
      msg += "' eval: ";
      msg += ConvertValToString( lhs ) + QString(" == ") + ConvertValToString( rhs );
      QTest::qFail( msg.toUtf8(), file, line );
   }
}

template<typename T>
void CompareValNotEqual(const T & lhs, const T & rhs,
      const char * lhs_text, const char * rhs_text,
      const char * file, int line )
{
   if ( !(lhs != rhs) )
   {
      QString msg = QString("'") + QString(lhs_text) + " != " + QString(rhs_text);
      msg += "' eval: ";
      msg += ConvertValToString( lhs ) + QString(" != ") + ConvertValToString( rhs );
      QTest::qFail( msg.toUtf8(), file, line );
   }
}

void CompareValTrue(bool cond, const char * cond_text, const char * file, int line );
void CompareValFalse(bool cond, const char * cond_text, const char * file, int line );

template<typename T>
void ComparePred(bool cond, const T & pred, const char * file, int line )
{
   if ( !cond )
   {
      QTest::qFail( pred.toString().toUtf8(), file, line );
   }
}

#define QVERIFY_LT( lhs, rhs ) { CompareValLessThan( (lhs), (rhs), #lhs, #rhs, __FILE__, __LINE__ ); }
#define QVERIFY_GT( lhs, rhs ) { CompareValGreaterThan( (lhs), (rhs), #lhs, #rhs, __FILE__, __LINE__ ); }
#define QVERIFY_LE( lhs, rhs ) { CompareValLessEqual( (lhs), (rhs), #lhs, #rhs, __FILE__, __LINE__ ); }
#define QVERIFY_GE( lhs, rhs ) { CompareValGreaterEqual( (lhs), (rhs), #lhs, #rhs, __FILE__, __LINE__ ); }
#define QVERIFY_EQ( lhs, rhs ) { CompareValEqual( (lhs), (rhs), #lhs, #rhs, __FILE__, __LINE__ ); }
#define QVERIFY_NE( lhs, rhs ) { CompareValNotEqual( (lhs), (rhs), #lhs, #rhs, __FILE__, __LINE__ ); }
#define QVERIFY_TRUE( cond ) { CompareValTrue( (cond), #cond, __FILE__, __LINE__ ); }
#define QVERIFY_FALSE( cond ) { CompareValFalse( (cond), #cond, __FILE__, __LINE__ ); }
#define QVERIFY_PRED( pred, expect, actual ) { pred pred_val; ComparePred( pred_val.compare(expect, actual), pred_val, __FILE__, __LINE__ ); }

// Common predicates for QVERIFY_PRED

class IsSubstring
{
   public:
      bool compare( const QString & expect, const QString & actual )
      {
         m_msg =  expect + QString(" is not a substring of ") + actual;
         return actual.contains(expect);
      }
      QString toString() const
      {
         return m_msg;
      }

   private:
      QString m_msg;
};

class StartsWith
{
   public:
      bool compare( const QString & expect, const QString & actual )
      {
         m_msg =  actual + QString(" does not begin with ") + expect;
         return actual.startsWith(expect);
      }
      QString toString() const
      {
         return m_msg;
      }

   private:
      QString m_msg;
};

class EndsWith
{
   public:
      bool compare( const QString & expect, const QString & actual )
      {
         m_msg =  actual + QString(" does not end with ") + expect;
         return actual.endsWith(expect);
      }
      QString toString() const
      {
         return m_msg;
      }

   private:
      QString m_msg;
};

class MatchesRegExp
{
   public:
      bool compare( const QString & expect, const QString & actual )
      {
         return compare( QRegExp(expect), actual );
      }
      bool compare( const QRegExp & expect, const QString & actual )
      {
         m_msg =  actual + QString(" does not match ") + expect.pattern();
         return expect.indexIn( actual ) >= 0;
      }
      QString toString() const
      {
         return m_msg;
      }

   private:
      QString m_msg;
};

void model_status( Model * model, StatusTypeE type, unsigned ms, const char * fmt, ... );

Model * loadModelOrDie( const char * filename );
Model * newTestModel();

typedef std::vector<Model *> ModelList;
void checkUndoRedo( int operations, Model * lhs, const ModelList & rhs_list );

#endif // TEST_COMMON_H__
