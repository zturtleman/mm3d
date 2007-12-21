#ifndef __LOCAL_ARRAY_H
#define __LOCAL_ARRAY_H

#include <stdlib.h>

template<typename T> class local_array
{
   public:
      local_array( T * pval = NULL )
         : m_pval( pval ) {}
      virtual ~local_array() { free_ptr(); }

      T * get() { return m_pval; }
      T * reset(T * newPval) { free_ptr(); return m_pval = newPval; }
      const T * get() const { return m_pval; }
      const T * reset(T * newPval) const { free_ptr(); return m_pval = newPval; }

      T & operator*() { return *m_pval; }
      T * operator->() { return m_pval; }
      const T & operator*() const { return *m_pval; }
      const T * operator->() const { return m_pval; }

      T * operator=(T* newPval) { return reset(newPval); }
      const T * operator=(T* newPval) const { return reset(newPval); }

      bool operator!() const { return m_pval == NULL; }
      bool isnull() const { return m_pval == NULL; }

   protected:
      void free_ptr() const { delete[] m_pval; m_pval = NULL; }

      mutable T * m_pval;
};

#endif // __LOCAL_ARRAY_H
