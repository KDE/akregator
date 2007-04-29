#ifndef AKREGATOR_TYPES_H
#define AKREGATOR_TYPES_H

namespace Akregator {

/** (un)read status of the article */
enum ArticleStatus {
    Unread=0, /**< article wasn't read yet by the user */
    Read, /**< article is read */
    New /**< article was fetched in the last fetch of it's feed and not read yet. Note that, semantically, new implies unread */
};

}

#endif // AKREGATOR_TYPES_H


