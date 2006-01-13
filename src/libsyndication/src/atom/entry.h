#ifndef LIBSYNDICATION_ATOM_ENTRY_H
#define LIBSYNDICATION_ATOM_ENTRY_H

#include "../elementwrapper.h"

#include <ctime>

class QDomElement;
class QString;
template <class T> class QList;

namespace LibSyndication {
namespace Atom {

class Category;
class Content;
class Link;
class Person;
class Source;

class Entry : public ElementWrapper
{
    public:
    
        Entry();
        Entry(const QDomElement& element);
    
        /* 1..0, or atom:source with an author in it, or a feed with author */
        QList<Person> authors() const;
        
        /* 0..* */
        QList<Category> categories() const;
    
        /* 0..* */
        QList<Person> contributors() const;
    
        /* 1 */
        QString id() const;
    
        /* at least one link with rel=alternate */
        QList<Link> links() const;
    
        /* 0..1*/
        QString rights() const;
    
        /* 0..1*/
        Source source() const;
    
        /* 0..1*/
        time_t published() const;
    
        /* 1 */
        time_t updated() const;
    
        /* 0..1
        mandatory when 
            - atom:content with src attr. (thus empty)
            - atom:content is not XML but Base64 encoded      
        */
        QString summary() const;
    
        /* 1 */
        QString title() const;
    
        /* 0..1 */
        Content content() const;
    
        QString debugInfo() const;
};

} // namespace Atom
} // namespace LibSyndication

#endif // LIBSYNDICATION_ATOM_ENTRY_H
