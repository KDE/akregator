/*
    This file is part of Akregator.

    Copyright (C) 2005 Frank Osterfeld <frank.osterfeld at kdemail.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef AKREGATOR_KERNEL_H
#define AKREGATOR_KERNEL_H

#include "articlefilter.h"

namespace Akregator {

namespace Backend 
{
    class Storage;
}

class FeedList;
class FetchQueue;
class TagSet;

class Kernel
{
     public:

         static Kernel* self();

         Kernel();
         virtual ~Kernel();
         
         Backend::Storage* storage() { return m_storage; }
         void setStorage(Backend::Storage* storage) { m_storage = storage; }

         FeedList* feedList() { return m_feedList; }
         void setFeedList(FeedList* feedList) { m_feedList = feedList; }

         FetchQueue* fetchQueue() { return m_fetchQueue; }

         TagSet* tagSet() { return m_tagSet; }
         
         void setArticleFilterList(const Filters::ArticleFilterList& list)
         {
            m_articleFilterList = list;
         }

         Filters::ArticleFilterList articleFilterList() const { return m_articleFilterList; }
        
     private:

         static Kernel* m_self;

         Backend::Storage* m_storage;
         FeedList* m_feedList;
         FetchQueue* m_fetchQueue;
         TagSet* m_tagSet;
         Filters::ArticleFilterList m_articleFilterList;
};

}

#endif // AKREGATOR_KERNEL_H
