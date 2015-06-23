/*
    This file is part of Akregator.

    Copyright (C) 2005 Frank Osterfeld <osterfeld@kde.org>

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

#ifndef AKREGATOR_BROWSERFRAME_H
#define AKREGATOR_BROWSERFRAME_H

#include <kconfiggroup.h>

#include "frame.h"
#include "feediconmanager.h"

#include "akregator_export.h"

class QPoint;
class QString;
class QWidget;

class QUrl;

namespace KParts
{
class OpenUrlArguments;
class ReadOnlyPart;
}

namespace Akregator
{

class AKREGATOR_EXPORT BrowserFrame : public Frame, public virtual FaviconListener
{
    Q_OBJECT

public:

    explicit BrowserFrame(QWidget *parent = Q_NULLPTR);
    ~BrowserFrame();

    QUrl url() const Q_DECL_OVERRIDE;

    KParts::ReadOnlyPart *part() const Q_DECL_OVERRIDE;

    bool canGoForward() const Q_DECL_OVERRIDE;
    bool canGoBack() const Q_DECL_OVERRIDE;
    bool isReloadable() const Q_DECL_OVERRIDE;
    bool isLoading() const Q_DECL_OVERRIDE;

    bool openUrl(const OpenUrlRequest &request) Q_DECL_OVERRIDE;

    void setFavicon(const QIcon &icon) Q_DECL_OVERRIDE;

    void loadConfig(const KConfigGroup &config, const QString &prefix) Q_DECL_OVERRIDE;
    void saveConfig(KConfigGroup &config, const QString &prefix) Q_DECL_OVERRIDE;

    bool hasZoom() const;
    int  getZoomFactor() const;
    void setZoomFactor(int);
public Q_SLOTS:

    void slotHistoryForward() Q_DECL_OVERRIDE;
    void slotHistoryBack() Q_DECL_OVERRIDE;
    void slotReload() Q_DECL_OVERRIDE;
    void slotStop() Q_DECL_OVERRIDE;
    void slotHistoryBackAboutToShow() Q_DECL_OVERRIDE;
    void slotHistoryForwardAboutToShow() Q_DECL_OVERRIDE;

    void slotPaletteOrFontChanged();
    void slotOpenLinkInBrowser();
    void slotOpenLinkInNewTab();
    void slotZoomIn(int);
    void slotZoomOut(int);

private Q_SLOTS:

    void slotOpenUrlRequestDelayed(const QUrl &, const KParts::OpenUrlArguments &, const KParts::BrowserArguments &);
    void slotCreateNewWindow(const QUrl &url,
                             const KParts::OpenUrlArguments &args,
                             const KParts::BrowserArguments &browserArgs,
                             const KParts::WindowArgs &windowArgs,
                             KParts::ReadOnlyPart **part);
    void slotOpenUrlNotify();
    void slotSetLocationBarUrl(const QString &url);
    void slotSetIconUrl(const QUrl &url);
    void slotSpeedProgress(int);

    void slotPopupMenu(const QPoint &global,
                       const QUrl &url,
                       mode_t mode,
                       const KParts::OpenUrlArguments &args,
                       const KParts::BrowserArguments &browserArgs,
                       KParts::BrowserExtension::PopupFlags flags,
                       const KParts::BrowserExtension::ActionGroupMap &actionGroups);

Q_SIGNALS:
    void signalPartDestroyed(int id);

private:
    class Private;
    Private *const d;
};

} // namespace Akregator

#endif // AKREGATOR_BROWSERFRAME_H
