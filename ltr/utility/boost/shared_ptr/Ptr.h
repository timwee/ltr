////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001, Brad Cupit -- see the bottom of this file for the BSD
// license under which this source code is being released
////////////////////////////////////////////////////////////////////////////////
//  File Name: shared_ptr.h
//
//  Author: Brad Cupit
//  Project: C++ Smart Pointers
//  Platforms: pure
//
//  Description:
//    Defines the template class shared_ptr, which is the main Smart Pointer
//    implementation intended for general use (i.e. non-circular reference).
//    Also defines the exception that the shared_ptr class might throw.
//
////////////////////////////////////////////////////////////////////////////////
//  History:
//  09/28/01 file created
//
////////////////////////////////////////////////////////////////////////////////
#ifndef LTR_UTILITY_BOOST_SHARED_PTR_PTR_H_
#define LTR_UTILITY_BOOST_SHARED_PTR_PTR_H_

#include <algorithm>

using namespace ReferenceCountedSmartPointer; // NOLINT


namespace PtrExceptions {
  class DereferencedNULL { };
}

namespace ltr {
namespace utility {

using PtrExceptions::DereferencedNULL;
using std::swap;

// class declaration

////////////////////////////////////////////////////////////////////////////////
//  shared_ptr: Smart pointer definition. The pointers are reference counted,
//    when the last one goes out of scope it will delete the object being
//    pointed to. Sometimes referred to as "Active Pointers". Guaranteed to
//    either be NULL, or point to a valid object.
//    IMPORTANT: The raw memory passed in through the constructor or
//    assignment operator must be memory allocated on the heap!
////////////////////////////////////////////////////////////////////////////////
template<class T>
class shared_ptr {
// Exceptions thrown by this class:
//    PtrExceptions::DereferencedNULL

 public:
  // template copy constructor that takes type U which can be
  // implicitly cast to type T
  template<class U>
  shared_ptr(const shared_ptr<U>& pSmartPtr) : // NOLINT
    m_pRawPtr(pSmartPtr.GetRawPtr()),
    m_pRefCount(pSmartPtr.GetRefObject()) {
    AdjustRefCount();
  }

  // template assignment operator that takes type U which can be
  // implicitly cast to type T
  template<class U>
  shared_ptr& operator=(const shared_ptr<U>& pRightSide) {
    ReassignSelf(pRightSide.GetRawPtr(), pRightSide.GetRefObject());
    return(*this);
  }

  // constructor that takes an Inactive Pointer of a type U which can be
  // implicitly cast to type T
  template<class U>
  shared_ptr(const InactivePtr<U>& pInactivePtr) // NOLINT
  :  m_pRawPtr(pInactivePtr.GetRawPtr()),
    m_pRefCount(pInactivePtr.GetRefObject()) {
    AdjustRefCount();
  }

  // assignment operator that takes an Inactive Pointer of a type U which can
  // be implicitly cast to type T
  template<class U>
  shared_ptr& operator=(const InactivePtr<U>& pRightSide) {
    ReassignSelf(pRightSide.GetRawPtr(), pRightSide.GetRefObject());
    return(*this);
  }

  // compares a type U to a compatible type T
  template<class U>
  inline bool operator==(const shared_ptr<U>& pRightSide) const {
    return(m_pRawPtr == pRightSide.GetRawPtr());
  }

  // compares a type U to a compatible type T
  template<class U>
  inline bool operator!=(const shared_ptr<U>&pRightSide) const {
    return(m_pRawPtr != pRightSide.GetRawPtr());
  }


  shared_ptr(T* pRawPtr = NULL); // NOLINT
    // Used only by casting operators
  shared_ptr(T* pRawPtr, PtrRefCounter* pRefCount);
  shared_ptr(const shared_ptr& pSmartPtr);
  shared_ptr& operator=(const shared_ptr& pRightSide);
  shared_ptr& operator=(T* pRawPtr);
  ~shared_ptr(void);

  inline  T& operator*(void) const;
  inline  T* operator->(void) const;
  inline  bool operator==(int iShouldBeNull) const;
  inline  bool operator!=(int iShouldBeNull) const;
  inline  operator bool() const {
    return (m_pRawPtr != NULL);
  }

  inline  void ResetPtr(void);
  inline  void reset(void) {
    ResetPtr();
  }
  inline  void reset(T* pRawPtr) {
    m_pRawPtr = pRawPtr;
    m_pRefCount = NULL;
    if (m_pRawPtr != NULL) {
      m_pRefCount = new PtrRefCounter();
    }
  }

  inline  T* GetRawPtr(void) const;
  inline  T* get(void) const {return GetRawPtr();}
  inline  PtrRefCounter* GetRefObject(void) const;
  inline  void swap(shared_ptr<T>& ptr) {
    shared_ptr<T> temp_ptr(ptr);
    ptr.ReassignSelf(m_pRawPtr, m_pRefCount);
    ReassignSelf(temp_ptr.m_pRawPtr, temp_ptr.m_pRefCount);
  }

 private:
  inline  void AdjustRefCount(void);
  inline  void ReassignSelf(T* pRawPtr, PtrRefCounter* pRefCount);

  T*        m_pRawPtr;
  PtrRefCounter*  m_pRefCount;
};  // class shared_ptr



////////////////////////////////////////////////////////////////////////////////
//  inline function implementations
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//  constructor  [public]
//
// description:
//    This is the default constructor.
// arguments:
//    pRawPtr: a raw pointer that should be a value which has been returned
//         from new
template<class T>
shared_ptr<T>::shared_ptr(T* pRawPtr)
:  m_pRawPtr(pRawPtr),
  m_pRefCount(NULL) {
  if (m_pRawPtr != NULL) {
    m_pRefCount = new PtrRefCounter();
  }
}

////////////////////////////////////////////////////////////////////////////////
//  constructor  [public]
//
// description:
//    This constructor is only used by the casting operators. (This is
//    necessary because Microsoft Visual C++ 6.0 does not allow template
//    functions to be friends of template classes.)
// arguments:
//    pRawPtr:   a raw pointer received from some other smart pointer
//    pRefCount: a pointer to a reference counting object, received from
//           some other smart pointer
template<class T>
shared_ptr<T>::shared_ptr(T* pRawPtr, PtrRefCounter* pRefCount)
:  m_pRawPtr(pRawPtr),
  m_pRefCount(pRefCount) {
  // if dynamic_cast failed or a cast on a NULL pointer is being performed
  if (m_pRawPtr == NULL) {
    m_pRefCount = NULL;
  } else if (m_pRefCount != NULL) {
    // both pointers are non-NULL, so add a reference to the count
    AdjustRefCount();
  } else {
    // if pRawPtr is valid and pRefCount is NULL, construct normally
    m_pRefCount = new PtrRefCounter();
  }
}


////////////////////////////////////////////////////////////////////////////////
//  constructor  [public]
//
// description:
//    This is the copy constructor.
// arguments:
//    pSmartPtr: a reference to another Active Pointer
template<class T>
shared_ptr<T>::shared_ptr(const shared_ptr<T>& pSmartPtr)
:  m_pRawPtr(pSmartPtr.GetRawPtr()),
  m_pRefCount(pSmartPtr.GetRefObject()) {
  AdjustRefCount();
}


////////////////////////////////////////////////////////////////////////////////
//  operator=  [public]
//
// description:
//    This is the assignment operator which will reset the current pointer
//    values and reassign them.
// arguments:
//    pRightSide: a reference to another Active Pointer
// return:
//    a reference to the dereferenced 'this' pointer
template<class T>
shared_ptr<T>& shared_ptr<T>::operator=(const shared_ptr<T>& pRightSide) {
  ReassignSelf(pRightSide.GetRawPtr(), pRightSide.GetRefObject());
  return(*this);
}


////////////////////////////////////////////////////////////////////////////////
//  operator=  [public]
//
// description:
//    This is the assignment operator which will reset the current pointer
//    values and reassign them.
// arguments:
//    pRawPtr: a raw pointer that should be a value which has been returned
//         from new
// return:
//    a reference to the dereferenced 'this' pointer
template<class T>
shared_ptr<T>& shared_ptr<T>::operator=(T* pRawPtr) {
  ResetPtr();

  m_pRawPtr = pRawPtr;
  if (m_pRawPtr != NULL) {
    m_pRefCount = new PtrRefCounter();
  } else {
    m_pRefCount = NULL;
  }

  return(*this);
}


////////////////////////////////////////////////////////////////////////////////
//  destructor  [public]
//
// description:
//    This is the destructor. It will remove one reference from the active
//    count of the Reference Counting Object (m_pRefCount). This could
//    cause the raw pointer object (m_pRawPtr) to get deleted
template<class T>
shared_ptr<T>::~shared_ptr(void) {
  ResetPtr();
}


////////////////////////////////////////////////////////////////////////////////
//  operator*  [public]
//
// description:
//    Dereferences and returns the raw pointer.
//    In non-debug mode, an exception of type PtrExceptions::DereferencedNULL
//    will be thrown.
// return:
//    a dereferenced pointer to the raw object being pointed to by this class
template<class T>
inline  T& shared_ptr<T>::operator*(void) const {
  if (m_pRawPtr == NULL) throw DereferencedNULL();

  return(*m_pRawPtr);
}


////////////////////////////////////////////////////////////////////////////////
//  operator->  [public]
//
// description:
//    Returns the raw pointer.
//    In non-debug mode, an exception of type PtrExceptions::DereferencedNULL
//    will be thrown.
// return:
//    a pointer to the raw object being pointed to by this class
template<class T>
inline  T* shared_ptr<T>::operator->(void) const {
  if (m_pRawPtr == NULL) throw DereferencedNULL();

  return(m_pRawPtr);
}


////////////////////////////////////////////////////////////////////////////////
//  operator==  [public]
//
// description:
//    Compares the raw pointer to NULL.
// arguments:
//    iShouldBeNull: must be NULL
// return:
//    true if the raw pointer is NULL, false otherwise
template<class T>
inline bool shared_ptr<T>::operator==(int iShouldBeNull) const {
  // this function should only be used for comparison to NULL
  return(m_pRawPtr == NULL);
}


////////////////////////////////////////////////////////////////////////////////
//  operator!=  [public]
//
// description:
//    Compares the raw pointer to NULL.
// arguments:
//    iShouldBeNull: must be NULL
// return:
//    true if the raw pointer is not NULL, false otherwise
template<class T>
inline bool shared_ptr<T>::operator!=(int iShouldBeNull) const {
  // this function should only be used for comparison to NULL
  return(m_pRawPtr != NULL);
}


////////////////////////////////////////////////////////////////////////////////
//  ResetPtr  [public]
//
// description:
//    Resets the pointer. It will remove one reference from the active
//    count of the Reference Counting Object (m_pRefCount). This could
//    cause the raw pointer object (m_pRawPtr) and/or the Reference Counting
//    Object (m_pRefCount) to get deleted.
//    Also sets both internal pointers to NULL.
template<class T>
inline void shared_ptr<T>::ResetPtr(void) {
  if (m_pRefCount != NULL) {
    int iDeleteObject = m_pRefCount->SubtractActiveRef();
    if (iDeleteObject != 0) {
      delete m_pRawPtr;

      if (iDeleteObject == DELETE_REF_COUNTER) {
        delete m_pRefCount;
      }
    }

    m_pRefCount = NULL;
    m_pRawPtr = NULL;
  }
}


////////////////////////////////////////////////////////////////////////////////
//  GetRawPtr  [public]
//
// description:
//    This function should be used with care. It was only implemented because
//    an API or library call might need a raw pointer. The pointer returned
//    does not come with any of the safety of the smart pointers.
// return:
//    the raw pointer
template<class T>
inline T* shared_ptr<T>::GetRawPtr(void) const {
  return(m_pRawPtr);
}


////////////////////////////////////////////////////////////////////////////////
//  GetRefObject  [public]
//
// description:
//    This function should never be called by a developer! It was only
//    implemented because Microsoft Visual C++ 6.0 does not allow template
//    classes to be friends of other template classes.
// return:
//    the pointer to the Reference Counting Object
template<class T>
inline PtrRefCounter* shared_ptr<T>::GetRefObject(void) const {
  return(m_pRefCount);
}


////////////////////////////////////////////////////////////////////////////////
//  AdjustRefCount  [private]
//
// description:
//    Increments the Reference Count in the Reference Counting Object.
//    If this Active Pointer has been initialized with an Inactive Pointer,
//    this function will handle determining if the values are valid. It is
//    possible that a call to this function could set both the Raw and the
//    Reference Counter to NULL
template<class T>
inline void  shared_ptr<T>::AdjustRefCount(void) {
  if (m_pRefCount != NULL) {
    if (m_pRefCount->AddActiveRef() == false) {
      // if AddActiveRef() was unsucessful, then the Reference Counting
      // Object and the Raw pointer are invalid
      m_pRefCount = NULL;
      m_pRawPtr = NULL;
    }
  }
}


////////////////////////////////////////////////////////////////////////////////
//  ReassignSelf  [private]
//
// description:
//    Reassigns what the smart pointer is pointing to.
// arguments:
//    pRawPtr:   a raw pointer received from some other smart pointer
//    pRefCount: a pointer to a reference counting object, received from
//           some other smart pointer
template<class T>
inline void shared_ptr<T>::ReassignSelf(T* pRawPtr, PtrRefCounter* pRefCount) {
  // check for self assignment or assignment to the same Reference Counter
  // that is already being pointed to
  if (m_pRefCount != pRefCount) {
    ResetPtr();

    m_pRawPtr = pRawPtr;
    m_pRefCount = pRefCount;

    AdjustRefCount();
  }
}
};
};


#endif  // LTR_UTILITY_BOOST_SHARED_PTR_PTR_H_


/*******************************************************************************
Copyright (c) 2001, Brad Cupit
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

  - Redistributions of source code must retain the above copyright notice, this
    list of conditions and the following disclaimer.
  - Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.
  - Neither the name of this project nor the names of its contributors may be
    used to endorse or promote products derived from this software without
    specific prior written permission. 

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/
