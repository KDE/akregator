/*
    This file is part of Akregator.

    Copyright (C) 2006 Frank Osterfeld <osterfeld@kde.org>

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
#include <kparts/part.h>

#include <kurl.h>

#include <QPointer>

namespace Akregator
{

class Frame;

class OpenUrlRequest
{
    public:

        /**
         * Akregator-specific options specifying how a link should be handled.
         * TODO: check what can be done by overriding KURLArgs flags.
         */
        enum Options
        {
            None=0, /**< no explicit options, use default */
            NewTab, /**< open in new tab */
            ExternalBrowser /**< open in external browser */
        };

        explicit OpenUrlRequest(const KUrl& url=KUrl());

        /**
         * the Id of the frame that sent the request */
        int frameId() const;
        void setFrameId(int frameId);

        KUrl url() const;
        void setUrl(const KUrl& url);

        KParts::OpenUrlArguments args() const;
        void setArgs(const KParts::OpenUrlArguments& args);

        KParts::BrowserArguments browserArgs() const;
        void setBrowserArgs(const KParts::BrowserArguments& args);

        Options options() const;
        void setOptions(Options options);

        bool openInBackground() const;
        void setOpenInBackground(bool background);

        /**
         * The part that was created for a "NewTab" request.
         *
         * It must be set after creating the tab, so that the initiating
         * part can load the URL into the new part. This works only synchronously
         * and requires args().serviceType to be set.
         *
         * @see KParts::BrowserExtension::createNewWindow()
         */
        KParts::ReadOnlyPart* part() const;
        void setPart(KParts::ReadOnlyPart* part);

        QString debugInfo() const;

        bool wasHandled() const;
        void setWasHandled(bool handled);

    private:

        int m_frameId;
        KUrl m_url;
        KParts::OpenUrlArguments m_args;
        KParts::BrowserArguments m_browserArgs;
        Options m_options;
        QPointer<KParts::ReadOnlyPart> m_part;
        bool m_inBackground;
        bool m_wasHandled;
};

} // namespace Akregator

#endif // AKREGATOR_OPENURLREQUEST_H
