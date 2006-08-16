/*
    This file is part of Akregator.

    Copyright (C) 2004 Sashmit Bhaduri <smt@vfemail.net>
                  2005 Frank Osterfeld <frank.osterfeld@kdemail.net>
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

#ifndef AKREGATOR_FRAME_H
#define AKREGATOR_FRAME_H

#include <QSet>
#include <QString>
#include <QWidget>

#include <kparts/browserextension.h>

class KUrl;

namespace KParts
{
    class ReadOnlyPart;
}

namespace KPIM
{
    class ProgressItem;
}

namespace Akregator
{

class OpenURLRequest;
    
class Frame : public QWidget
{
    Q_OBJECT

    public:
        Frame(QWidget* parent=0);
        virtual ~Frame();

        enum {Idle, Started, Completed, Canceled};

        /** options for open URL requests sent from the frame to the outside. TODO: check if KParts::URLArgs covers our needs */
        
        virtual KParts::ReadOnlyPart* part() const = 0;

        /**
        * Returns whether the frame can be removed from
        * Akregator (via detach or close tab etc.)  Usually
        * all tabs but the main tab can be removed.
        * Default is @c true
        */
        bool isRemovable() const;

        /**
         * returns the URL of the embedded part
         */
        virtual KUrl url() const = 0;

        virtual const QString& title() const;
        virtual const QString& caption() const;
        virtual int state() const;
        virtual int progress() const;
        virtual const QString& statusText() const;
        
	virtual int id() const;
	
        /**
         * returns whether it is possible to go forward in the history
         */
        virtual bool canGoForward() const { return false; }

        /**
         * returns whether it is possible to go back in the history
         */
        virtual bool canGoBack() const { return false; }

        /**
         * returns whether the shown content can be reloaded */
        virtual bool isReloadable() const { return false; }

        /**
         * returns whether the embedded part is loading a website. If so, it can be stopped using slotStop() */
        virtual bool isLoading() const { return false; }

        virtual bool openURL(const KUrl& url, const QString& mimetype="text/html") = 0;

    public slots:

        /** goes a step forward in the history, if possible. See also canGoForward(). */
        virtual void slotHistoryForward() {}

        /** goes a step backwards in the history, if possible. See also canGoBack(). */
        virtual void slotHistoryBack() {}

        /** reloads the current content, if possible. See also isReloadable(). */
        virtual void slotReload() {}

        virtual void slotStop() {}

        virtual void slotSetStarted();
        virtual void slotSetCanceled(const QString&);
        virtual void slotSetCompleted();
        virtual void slotSetState(int);
        virtual void slotSetProgress(int);
        virtual void slotSetCaption(const QString&);
        virtual void slotSetTitle(const QString&);
        virtual void slotSetStatusText(const QString&);

    signals:
        void signalCaptionChanged(Frame*, const QString&);
        void signalTitleChanged(Frame*, const QString&);
        void signalStarted(Frame*);
        void signalCanceled(Frame*, const QString&);
        void signalCompleted(Frame*);
        void signalLoadingProgress(Frame*, int);
        void signalStatusText(Frame*, const QString&);

        void signalOpenURLRequest(const OpenURLRequest& request);

        void signalCanGoBackToggled(Frame*, bool);
        void signalCanGoForwardToggled(Frame*, bool);
        void signalIsReloadableToggled(Frame*, bool);
        void signalIsLoadingToggled(Frame*, bool);


    protected:
        void setRemovable(bool removable);

    protected:
        QString m_title;
        QString m_caption;
        int m_state;
        int m_progress;
        QString m_statusText;
        QString m_progressId;
        KPIM::ProgressItem* m_progressItem;
        bool m_isRemovable;
	int m_id;
	static int m_idCounter;
};

class MainFrame : public Frame
{
    Q_OBJECT

    public:

        MainFrame(QWidget* /*parent*/, KParts::ReadOnlyPart* /*part*/, QWidget* /*widget*/, const QString& /*title*/);
        virtual ~MainFrame();

        virtual KUrl url() const;
        virtual bool openURL(const KUrl& /*url*/, const QString& /*mimetype="text/html"*/) { return false; }

        virtual KParts::ReadOnlyPart* part() const { return m_part; }

   private:
        KParts::ReadOnlyPart* m_part;
};

} // namespace Akregator

#endif // AKREGATOR_FRAME_H
