/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2007 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/
#ifndef AKREGATOR_TYPES_H
#define AKREGATOR_TYPES_H

namespace Akregator {
/** (un)read status of the article */
enum ArticleStatus {
    Unread = 0, /**< article wasn't read yet by the user */
    Read, /**< article is read */
    New /**< article was fetched in the last fetch of it's feed and not read yet. Note that, semantically, new implies unread */
};
}

#endif // AKREGATOR_TYPES_H
