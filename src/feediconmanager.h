/*
    This file is part of Akregator.

    Copyright (C) 2004 Sashmit Bhaduri <smt@vfemail.net>

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef FEEDICONMGR_H
#define FEEDICONMGR_H

#include <dcopobject.h>

#include <qobject.h>

class QPixmap;

class DCOPClient;
class KURL;


namespace Akregator
{
    class FeedIconManager:public QObject, public DCOPObject
    {
        Q_OBJECT
        K_DCOP
        
        public:
            static FeedIconManager *self();
            void loadIcon(const QString &);
            QString iconLocation(const KURL &) const;
            
        k_dcop:
            void slotIconChanged(bool, const QString&, const QString&);

        signals:
            void iconChanged(const QString &, const QPixmap &);

        protected:
            FeedIconManager(QObject * = 0L, const char * = 0L);
            ~FeedIconManager();

      private:
            static FeedIconManager *m_instance;
    };
}

#endif
