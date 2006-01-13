#include "../documentsource.h"

#include "constants.h"
#include "document.h"
#include "parser.h"

#include <QDomDocument>
#include <QDomElement>
#include <QString>

namespace LibSyndication {
namespace Atom {

bool Parser::accept(const LibSyndication::DocumentSource& source) const
{
    QDomDocument doc = source.asDomDocument();
    if (doc.isNull())
        return false;
    
    QDomElement feed = doc.namedItem(QString::fromLatin1("feed")).toElement();
    bool feedValid = !feed.isNull() && feed.namespaceURI() == Constants::atom1NameSpace();

    if (feedValid)
        return true;

    QDomElement entry = doc.namedItem(QString::fromLatin1("entry")).toElement();
    bool entryValid = !entry.isNull() && entry.namespaceURI() == Constants::atom1NameSpace();

    return entryValid;
}

LibSyndication::AbstractDocument* Parser::parse(const LibSyndication::DocumentSource& source) const
{
    QDomDocument doc = source.asDomDocument();

    if (doc.isNull())
        return 0;

    
    QDomElement feed = doc.namedItem(QString::fromLatin1("feed")).toElement();
    bool feedValid = !feed.isNull() && feed.namespaceURI() == Constants::atom1NameSpace();

    if (feedValid)
    {
        return new FeedDocument(feed);
    }

    QDomElement entry = doc.namedItem(QString::fromLatin1("entry")).toElement();
    bool entryValid = !entry.isNull() && entry.namespaceURI() == Constants::atom1NameSpace();

    if (entryValid)
    {
        return new EntryDocument(entry);
    }

    return 0;
}

QString Parser::format() const
{
    return QString::fromLatin1("atom");
}

} // namespace Atom
} // namespace LibSyndication
