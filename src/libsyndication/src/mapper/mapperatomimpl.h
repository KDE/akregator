#ifndef LIBSYNDICATION_MAPPER_ATOMMAPPER_H
#define LIBSYNDICATION_MAPPER_ATOMMAPPER_H

#include "feedatomimpl.h"

#include "../atom/document.h"
#include "../abstractdocument.h"
#include "../feed.h"
#include "../mapper.h"



namespace LibSyndication {

/** @internal */
class AtomMapper : public Mapper<Feed>
{
    KSharedPtr<Feed> map(AbstractDocumentPtr doc) const
    {
        return KSharedPtr<Feed>(new FeedAtomImpl(LibSyndication::Atom::FeedDocumentPtr::staticCast(doc)));
    }
};

} // namespace LibSyndication

#endif // LIBSYNDICATION_MAPPER_ATOMMAPPER_H
