#ifndef TEST_COMMON_H__
#define TEST_COMMON_H__

#include <QtTest/QtTest>

QString CompareValToString( int val ) 
{
   return QString::number( val );
}

QString CompareValToString( const std::string & val ) 
{
   return QString( val.c_str() );
}

template<typename T>
void CompareValLessThan(const T & lhs, const T & rhs,
      const char * lhs_text, const char * rhs_text,
      const char * file, int line )
{
   if ( !(lhs < rhs) )
   {
      QString msg = QString("'") + QString(lhs_text) + " < " + QString(rhs_text);
      msg += "' eval: ";
      msg += CompareValToString( lhs ) + QString(" < ") + CompareValToString( rhs );
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
      msg += CompareValToString( lhs ) + QString(" > ") + CompareValToString( rhs );
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
      msg += CompareValToString( lhs ) + QString(" <= ") + CompareValToString( rhs );
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
      msg += CompareValToString( lhs ) + QString(" >= ") + CompareValToString( rhs );
      QTest::qFail( msg.toUtf8(), file, line );
   }
}

template<typename T>
void CompareValEqual(const T & lhs, const T & rhs,
      const char * lhs_text, const char * rhs_text,
      const char * file, int line )
{
   if ( !(lhs == rhs) )
   {
      QString msg = QString("'") + QString(lhs_text) + " == " + QString(rhs_text);
      msg += "' eval: ";
      msg += CompareValToString( lhs ) + QString(" == ") + CompareValToString( rhs );
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
      msg += CompareValToString( lhs ) + QString(" != ") + CompareValToString( rhs );
      QTest::qFail( msg.toUtf8(), file, line );
   }
}

void CompareValTrue(bool cond, const char * cond_text, const char * file, int line )
{
   if ( !cond )
   {
      QString msg = QString("'") + QString(cond_text);
      msg += "' returned FALSE (expected TRUE)";
      QTest::qFail( msg.toUtf8(), file, line );
   }
}

void CompareValFalse(bool cond, const char * cond_text, const char * file, int line )
{
   if ( !(!cond) )
   {
      QString msg = QString("'") + QString(cond_text);
      msg += "' returned TRUE (expected FALSE)";
      QTest::qFail( msg.toUtf8(), file, line );
   }
}

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

#endif // TEST_COMMON_H__
