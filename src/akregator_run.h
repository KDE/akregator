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

#ifndef AKREGATOR_RUN_H 
#define AKREGATOR_RUN_H

#include <kparts/browserrun.h>

namespace Akregator
{

    class Viewer;
    
    class BrowserRun : public KParts::BrowserRun
{
	Q_OBJECT
    public:
        BrowserRun(Viewer *, QWidget *, KParts::ReadOnlyPart *, const KURL & , const KParts::URLArgs &);
        virtual ~BrowserRun();
    
    protected:
	    virtual void foundMimeType( const QString & _type );

    private slots:
        void killMyself();

    private:
        Viewer *m_viewer;
};

}

#endif

// vim: set et ts=4 sts=4 sw=4:
