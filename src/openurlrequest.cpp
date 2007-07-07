/*
    This file is part of Akregator.

    Copyright (C) 2006 Frank Osterfeld <frank.osterfeld@kdemail.net>
    
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

#include "openurlrequest.h"


namespace Akregator {

OpenUrlRequest::OpenUrlRequest(const KUrl& url) : m_frameId(-1), m_url(url),
     m_options(None), m_part(0L), m_inBackground(false)
{
}

int OpenUrlRequest::frameId() const
{
    return m_frameId;
}

void OpenUrlRequest::setFrameId(int frameId)
{
    m_frameId = frameId;
}

KUrl OpenUrlRequest::url() const
{
    return m_url;
}

void OpenUrlRequest::setUrl(const KUrl& url)
{
    m_url = url;
}
bool OpenUrlRequest::openInBackground() const
{
    return m_inBackground;
}

void OpenUrlRequest::setOpenInBackground(bool background)
{
    m_inBackground = background;
}
          
KParts::URLArgs OpenUrlRequest::args() const
{
    return m_args;
}

void OpenUrlRequest::setArgs(const KParts::URLArgs& args)
{
    //m_hasArgs = true;
    m_args = args;
}

OpenUrlRequest::Options OpenUrlRequest::options() const
{
    return m_options;
}

void OpenUrlRequest::setOptions(OpenUrlRequest::Options options)
{
    m_options = options;
}

void OpenUrlRequest::setPart(KParts::ReadOnlyPart* part)
{
    m_part = part;
}

KParts::ReadOnlyPart* OpenUrlRequest::part() const
{
    return m_part;
}

QString OpenUrlRequest::debugInfo() const
{
    return  "url=" + m_url.url() 
            + " serviceType=" + m_args.serviceType  
            + " newTab=" + m_args.newTab() 
            + " forcesNewWindow=" + m_args.forcesNewWindow()
            + " options="+ m_options; 
}

} // namespace Akregator
