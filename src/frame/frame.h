/*
    This file is part of Akregator.

    Copyright (C) 2004 Sashmit Bhaduri <smt@vfemail.net>
                  2005 Frank Osterfeld <osterfeld@kde.org>
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

#include <QWidget>
#include <QIcon>

#include <kparts/browserextension.h>
#include <kconfiggroup.h>

#include "akregator_export.h"

class QUrl;

namespace KPIM {
class ProgressItem;
}

namespace Akregator {
class OpenUrlRequest;

class AKREGATOR_EXPORT Frame : public QWidget
{
    Q_OBJECT

public:
    explicit Frame(QWidget *parent = nullptr);
    ~Frame() override;

    enum State {
        Idle, Started, Completed, Canceled
    };

    virtual qreal zoomFactor() const = 0;
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
    virtual QUrl url() const = 0;

    Q_REQUIRED_RESULT QString title() const;
    Q_REQUIRED_RESULT QString caption() const;
    Q_REQUIRED_RESULT State state() const;
    Q_REQUIRED_RESULT int progress() const;
    Q_REQUIRED_RESULT QString statusText() const;
    Q_REQUIRED_RESULT QIcon icon() const;
    void setIcon(const QIcon &icon);

    Q_REQUIRED_RESULT int id() const;
    /**
     * returns whether the embedded part is loading a website. If so, it can be stopped using slotStop() */
    virtual bool isLoading() const;

    virtual bool openUrl(const OpenUrlRequest &request) = 0;

    /**
     * Load a frame from a config file for session management.
     */
    virtual void loadConfig(const KConfigGroup & /*config*/, const QString & /*prefix*/)
    {
    }

    /**
     * Save a frame to a config file for session management.
     */
    virtual bool saveConfig(KConfigGroup & /*config*/, const QString & /*prefix*/)
    {
        return false;
    }

public Q_SLOTS:

    /** reloads the current content, if possible. See also isReloadable(). */
    virtual void slotReload()
    {
    }

    virtual void slotStop();

    void slotSetStarted();
    void slotSetCanceled(const QString &);
    void slotSetCompleted();
    void slotSetState(State);
    void slotSetProgress(int);
    void slotSetCaption(const QString &);
    void slotSetTitle(const QString &);
    void slotSetStatusText(const QString &);

Q_SIGNALS:
    void signalCaptionChanged(Akregator::Frame *, const QString &);
    void signalTitleChanged(Akregator::Frame *, const QString &);
    void signalStarted(Akregator::Frame *);
    void signalCanceled(Akregator::Frame *, const QString &);
    void signalCompleted(Akregator::Frame *);
    void signalLoadingProgress(Akregator::Frame *, int);
    void signalStatusText(Akregator::Frame *, const QString &);

    void signalOpenUrlRequest(Akregator::OpenUrlRequest &request);
    void showStatusBarMessage(const QString &msg);

protected:
    void setRemovable(bool removable);

protected:
    QIcon m_icon;
    QString m_title;
    QString m_caption;
    State m_state;
    int m_progress;
    QString m_statusText;
    QString m_progressId;
    KPIM::ProgressItem *m_progressItem = nullptr;
    bool m_isRemovable = false;
    bool m_loading = false;
    int m_id;
    static int m_idCounter;
};
} // namespace Akregator

#endif // AKREGATOR_FRAME_H
