/***************************************************************************
 *     Copyright (C) 2004 by Sashmit Bhaduri                               *
 *     smt@vfemail.net                                                     *
 *                                                                         *
 *     Licensed under GPL.                                                 *
 ***************************************************************************/

#ifndef FRAME_H
#define FRAME_H

#include <kparts/part.h>

#include <qobject.h>

namespace Akregator
{

    class Frame : public QObject
    {
        Q_OBJECT
	
	    public:
            Frame(QObject *parent, KParts::ReadOnlyPart *part, QWidget *w, const QString& tit, bool watch=true);
            ~Frame();

            enum {Idle, Started, Completed, Canceled};

    	    KParts::ReadOnlyPart *part() const;
            QWidget *widget() const;
            const QString title() const;
            const QString caption() const;
            int state() const;
            int progress() const;
            const QString statusText() const;

        public slots:
            void setStarted();
            void setCanceled(const QString &);
            void setCompleted();
            void setState(int);
            void setProgress(int);
            void setCaption(const QString &); 
            void setTitle(const QString &);
            void setStatusText(const QString &); 

        signals:
            void captionChanged(const QString &);
            void started();
            void canceled(const QString &);
            void completed();
            int loadingProgress(int);
            void statusText(const QString &);

        private:
    	    KParts::ReadOnlyPart *m_part;
            QWidget *m_widget;
            QString m_title;
            QString m_caption;
            int m_state;
            int m_progress;
            QString m_statusText;
    };
}

#endif


// vim: set et ts=4 sts=4 sw=4:
