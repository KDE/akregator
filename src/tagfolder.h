#ifndef AKREGATOR_TAGFOLDER_H
#define AKREGATOR_TAGFOLDER_H

#include "folder.h"

class QDomDocument;
class QDomElement;

namespace Akregator {

class TreeNodeVisitor;

class TagFolder : public Folder
{
    public:

        TagFolder(const QString& title = QString::null);

        virtual ~TagFolder();
        virtual bool accept(TreeNodeVisitor* visitor);

        virtual QDomElement toOPML( QDomElement parent, QDomDocument document ) const;

    private:
        class TagFolderPrivate;
        TagFolderPrivate* d;
};

} // namespace Akregator

#endif // AKREGATOR_TAGFOLDER_H
