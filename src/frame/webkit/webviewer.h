/*
  Copyright (c) 2016 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef WEBVIEWER_H
#define WEBVIEWER_H

#include <articleviewer-ng/articleviewerng.h>

namespace Akregator
{
class WebViewer : public ArticleViewerNg
{
    Q_OBJECT
public:
    explicit WebViewer(KActionCollection *ac, QWidget *parent = Q_NULLPTR);
    ~WebViewer();

protected:
    void displayContextMenu(const QPoint &pos) Q_DECL_OVERRIDE;

private slots:
    void slotOpenBlockableItemsDialog();
};
}
#endif // WEBVIEWER_H
