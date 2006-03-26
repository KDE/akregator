/*
 * This file is part of the KDE libraries.
 *
 * Copyright 2005 Frerich Raabe <raabe@kde.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef LIBSYNDICATION_SHAREDPTR_H
#define LIBSYNDICATION_SHAREDPTR_H

#include <QAtomic>

namespace LibSyndication {

template <class T>
class SharedPtr;

template <class T>
bool operator==( const SharedPtr<T> &lhs, const SharedPtr<T> &rhs )
{
	return lhs.d == rhs.d;
}

template <class T>
bool operator!=( const SharedPtr<T> &lhs, const SharedPtr<T> &rhs )
{
	return !operator==( lhs, rhs );
}

/**
 * A reference-counting pointer.
 *
 * This class serves as a replacement for ordinary C++ pointers, allowing
 * you to handle objects as if they are explicitely shared, handling
 * reference counting behind the scenes.
 *
 * An explicitely shared object provides very cheap copying since only a
 * pointer is copied and a reference count is updated. When an explicitely
 * shared object goes out of scope, the internal reference count gets
 * decreased. The data which multiple explicitely shared objects share is
 * only deleted when the last object which references it goes out of scope.
 *
 * Explicitely shared objects do not provide copy-on-write semantics; this
 * means that changing one object will also change all other objects which
 * share the data. Consider this:
 * \code
 * SharedPtr<QString> w = new QString( "Hello" );
 * SharedPtr<QString> v = w; // Now both v and w say 'Hello'
 * v->clear();                // Clears both v and w!
 * \endcode
 * The last line of code will clear both v and w, since they share the same
 * QString internally. To acquire a copy of the contained object which is
 * independant of all other copies, use the copy() function, as in:
 * \code
 * SharedPtr<QString> w = new QString( "Hello" );
 * SharedPtr<QString> v = w.copy(); // Make v a detached copy of w
 * v->clear();                       // v is cleared, w still says 'Hello'
 * \endcode
 *
 * @author Frerich Raabe <raabe@kde.org>
 * @short A reference counting pointer.
 */
template <class T>
class SharedPtr
{
	template <class U> friend class SharedPtr;
	friend bool operator==<>( const SharedPtr<T> &lhs, const SharedPtr<T> &rhs );
	friend bool operator!=<>( const SharedPtr<T> &lhs, const SharedPtr<T> &rhs );
	public:
		/**
		 * Constructs a shared pointer from a dumb pointer. Ownership
		 * of the referenced object is transferred to this shared
		 * pointer.
		 * @param obj A dumb pointer to mimic. May be a null pointer.
		 */
		SharedPtr( T *obj = 0 )
		{
			d = new KSharedData;
			d->ref = 1;
			obj = qAtomicSetPtr( &d->obj, obj );
		}

		/**
		 * Constructs a copy of another shared pointer, doing a
		 * shallow copy of the contained object.
		 * @param other Another shared pointer to copy.
		 */
		SharedPtr( const SharedPtr<T> &other )
		{
			acquire( other.d );
		}

		/**
		 * Constructs a copy of another shared pointer, doing a
		 * shallow copy of the contained object.
		 * @param other Another shared pointer to copy.
		 */
		template <class U>
		SharedPtr( const SharedPtr<U> &other )
		{
			acquire<U>( other.d );
		}

		/**
		 * Destructs this shared pointer. The contained object will
		 * only be deleted if no other pointers are pointing to it.
		 */
		~SharedPtr()
		{
			deref();
		}

		/**
		 * Makes this shared pointer refer to the pointee of the given
		 * dumb pointer. Ownership of the pointee is transferred to
		 * this shared pointer.
		 * @param rhs A dumb pointer whose pointee shall be referenced.
		 * @return A reference to this shared pointer.
		 */
		SharedPtr<T> &operator=( T *rhs )
		{
			deref();
			d = new KSharedData;
			rhs = qAtomicSetPtr( &d->obj, rhs );
			d->ref = 1;
			return *this;
		}

		/**
		 * Makes this shared pointer refer to the pointee of the
		 * given shared pointer.
		 * @param rhs A shared pointer whose pointee shall be referenced.
		 * @return A reference to this shared pointer.
		 */
		SharedPtr<T> &operator=( const SharedPtr<T> &rhs )
		{
			if ( this != &rhs ) {
				deref();
				acquire( rhs.d );
			}
			return *this;
		}

		/**
		 * Makes this shared pointer refer to the pointee of the
		 * given shared pointer.
		 * @param rhs A shared pointer whose pointee shall be referenced.
		 * @return A reference to this shared pointer.
		 */
		template <class U>
		SharedPtr<T> &operator=( const SharedPtr<U> &rhs )
		{
			if ( d->obj != rhs.d->obj ) {
				deref();
				acquire<U>( rhs.d );
			}
			return *this;
		}

		const T &operator*() const { return *d->obj; }
		T &operator*() { return *d->obj; }
		const T *operator->() const { return d->obj; }
		T *operator->() { return d->obj; }
		operator bool() const { return d->obj != 0; }

		/**
		 * @return A pointer which points to a detached copy of the
		 * referenced object. Modifications done via the returned
		 * pointer will not affect any other objects.
		 */
		SharedPtr<T> copy()
		{
			if ( !d->obj ) {
				return SharedPtr<T>();
			}
			return SharedPtr<T>( new T( *d->obj ) );
		}

		/**
		 * Detach this pointer from other shared pointers pointing
		 * to the same object. Future modifications done through this
		 * pointer will no longer affect other pointers.
		 */
		void detach()
		{
			*this = copy();
		}

		/**
		 * @return A dumb pointer to the contained object. This is
		 * rarely needed, and very dangerous since it completely
		 * circumvents the reference counting. Use with care!
		 */
		T *get() const
		{
			return d->obj;
		}

		/**
		 * @return Whether this is the only shared pointer pointing to
		 * to the pointee, or whether it's shared among multiple
		 * shared pointers.
		 */
		bool isUnique() const
		{
			return d->ref == 1;
		}

	private:
		struct KSharedData
		{
			QAtomic ref;
			T *obj;
		};

		void deref()
		{
			if ( !d->ref.deref() ) {
				delete d->obj;
				delete d;
				d = 0;
			}
		}

		void acquire( KSharedData *ptr )
		{
			ptr = qAtomicSetPtr( &d, ptr );
			d->ref.ref();
		}

		template <class U>
		void acquire( typename SharedPtr<U>::KSharedData *ptr )
		{
			/* Unfortunately both arguments to qAtomicSetPtr have
			 * to have the same type. */
			qAtomicSetPtr( reinterpret_cast<void **>( &d ), static_cast<void *>( ptr ) );
			d->ref.ref();
		}

		KSharedData *d;
};

} // namespace LibSyndication

#endif // LIBSYNDICATION_SHAREDPTR_H
