#include "tagfolder.h"
#include "treenodevisitor.h"

#include <qdom.h>

namespace Akregator {

class TagFolder::TagFolderPrivate
{
};

TagFolder::TagFolder(const QString& title) : Folder(title), d(new TagFolderPrivate)
{
}

TagFolder::~TagFolder()
{
    emitSignalDestroyed();
    delete d;
    d = 0;
}

bool TagFolder::accept(TreeNodeVisitor* visitor)
{
    if (visitor->visitTagFolder(this))
        return true;
    else
        return visitor->visitFolder(this);
}

QDomElement TagFolder::toOPML( QDomElement /*parent*/, QDomDocument /*document*/ ) const
{
    return QDomElement();
}

}
