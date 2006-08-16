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

#ifndef AKREGATOR_OPENURLREQUEST_H
#define AKREGATOR_OPENURLREQUEST_H

#include <kparts/browserextension.h>

class KUrl;

namespace Akregator
{

class Frame;
    
class OpenURLRequest
{
    public:
        
        /**
         * Akregator-specific options specifying how a link should be handled.
         * TODO: check what can be done by overriding KURLArgs flags.
         */
        enum Options
        {
            None, /**< no explicit options, use default */
            NewTab, /**< open in new tab */
            ExternalBrowser /**< open in external browser */
        };
        
        OpenURLRequest();

        /**
         * the Id of the frame that sent the request */  
        int frameId() const;
        void setFrameId(int frameId);
        
        const KUrl& url() const;
        void setUrl(const KUrl& url);
         
        const KParts::URLArgs& args() const;
        void setArgs(const KParts::URLArgs& args);
        
        Options options() const;
        void setOptions(Options options);
        
        QString mimetype() const;
        void setMimetype(const QString& mimetype);
        
     private:

        int m_frameId;
        KUrl m_url;
        KParts::URLArgs m_args;
        QString m_mimetype;
        Options m_options;
};

} // namespace Akregator

#endif // AKREGATOR_OPENURLREQUEST_H
