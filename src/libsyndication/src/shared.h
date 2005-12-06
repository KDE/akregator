/*
 * tools_p.h
 *
 * Copyright (c) 2001, 2002, 2003 Frerich Raabe <raabe@kde.org>
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. For licensing and distribution details, check the
 * accompanying file 'COPYING'.
 */
#ifndef LIBSYNDICATION_SHARED_H
#define LIBSYNDICATION_SHARED_H

#include <time.h>

namespace LibSyndication
{
	class Shared
	{
		public:
        Shared(const Shared& other) : count(1) { *this = other; }
		Shared() : count(1) { }
		void ref() { count++; }
		bool deref() { return !--count; }
		unsigned int count;
	};

}

#endif // LIBSYNDICATION_SHARED_H
