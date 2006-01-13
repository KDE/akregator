#ifndef LIBSYNDICATION_ATOM_DOCUMENT_H
#define LIBSYNDICATION_ATOM_DOCUMENT_H

#include "../abstractdocument.h"
#include "../elementwrapper.h"

#include <ctime>

class QDomDocument;
template <class T> class QList;

namespace LibSyndication {

class DocumentVisitor;

namespace Atom {

class Category;
class Entry;
class Link;
class Person;

class FeedDocument : public LibSyndication::AbstractDocument, public ElementWrapper
{
    public:
    
        FeedDocument();
        FeedDocument(const QDomElement& element);
    
        bool accept(DocumentVisitor* visitor);
    
        /* 1..* v each entry has >=1 author element */
        
        QList<Person> authors() const;
        
        /* 0..* */
        QList<Person> contributors() const;
        
        /* 0..* */
        QList<Category> categories() const;
        
        /* 0..1 */
        QString icon() const;
            
        /* 0..1 */
        QString logo() const;
        
        QString id() const;
            
        /* 0..1 */
        QString rights() const;
        
        /* 1 */
        QString title() const;
            
        /* 0..1 */
        QString subtitle() const;
        
        time_t updated() const;
            
        QList<Link> links() const;
            
        QList<Entry> entries() const;
    
        QString debugInfo() const;
};

class EntryDocument : public LibSyndication::AbstractDocument, public ElementWrapper
{
    public:

        EntryDocument();
        EntryDocument(const QDomElement& element);
    
        bool accept(DocumentVisitor* visitor);
    
        Entry entry() const;
    
        QString debugInfo() const;
};

} // namespace Atom
} // namespace LibSyndication

#endif // LIBSYNDICATION_ATOM_DOCUMENT_H
