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
#include <QWidget>

class KURL;

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

class Frame : public QWidget
{
    Q_OBJECT

    public:
        Frame(QWidget* parent, KParts::ReadOnlyPart *part, QWidget *w, const QString& tit, bool watchSignals=true);
        virtual ~Frame();

        enum {Idle, Started, Completed, Canceled};

        KParts::ReadOnlyPart *part() const;

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
        virtual KURL url() const = 0;

        virtual const QString& title() const;
        virtual const QString& caption() const;
        virtual int state() const;
        virtual int progress() const;
        virtual const QString& statusText() const;

    public slots:
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
    
    protected:
        void setRemovable(bool removable);

    private:
        QString m_title;
        QString m_caption;
        int m_state;
        int m_progress;
        QString m_statusText;
        KParts::ReadOnlyPart* m_part;
        QWidget* m_widget;
        QString m_progressId;
        KPIM::ProgressItem* m_progressItem;
        bool m_isRemovable;
};

class MainFrame : public Frame
{
    Q_OBJECT

    public:

        MainFrame(QWidget* parent, KParts::ReadOnlyPart* part, QWidget* widget, const QString& title);
        virtual ~MainFrame();

        virtual KURL url() const;
};

class BrowserFrame : public Frame
{
    Q_OBJECT

    public:

        BrowserFrame(QWidget* parent, KParts::ReadOnlyPart* part, QWidget* widget, const QString& title);
        virtual ~BrowserFrame();

        virtual KURL url() const;
};

class FrameManager : public QObject
{
    Q_OBJECT

    public:

        FrameManager(QObject* parent=0);
        virtual ~FrameManager();

        Frame* currentFrame() const;

        void addFrame(Frame* frame);
        void removeFrame(Frame* frame);
       
    public slots:

        void slotChangeFrame(Frame* frame);

    signals:

        void signalCurrentFrameChanged(Frame*);

        void signalStarted();
        void signalCanceled(const QString&);
        void signalCompleted();
        void signalCaptionChanged(const QString&);
        void signalTitleChanged(const QString&);
        void signalLoadingProgress(int);
        void signalStatusText(const QString&);

    protected slots:

        virtual void slotSetStarted(Frame* frame);
        virtual void slotSetCanceled(Frame* frame, const QString& reason);
        virtual void slotSetCompleted(Frame* frame);
        virtual void slotSetProgress(Frame* frame, int progress);
        virtual void slotSetCaption(Frame* frame, const QString& caption);
        virtual void slotSetTitle(Frame* frame, const QString& title);
        virtual void slotSetStatusText(Frame* frame, const QString& statusText);

    private:

        Frame* m_currentFrame;
        QSet<Frame*> m_frames;
};

} // namespace Akregator

#endif
