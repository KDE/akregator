#ifndef LIBSYNDICATION_MAPPER_RSS2MAPPER_H
#define LIBSYNDICATION_MAPPER_RSS2MAPPER_H

#include "feedrss2impl.h"

#include "../rss2/document.h"
#include "../abstractdocument.h"
#include "../feed.h"
#include "../mapper.h"



namespace LibSyndication {

/** @internal */
class RSS2Mapper : public Mapper<Feed>
{
    KSharedPtr<Feed> map(AbstractDocumentPtr doc) const
    {
        return KSharedPtr<Feed>(new FeedRSS2Impl(LibSyndication::RSS2::DocumentPtr::staticCast(doc)));
    }
};

} // namespace LibSyndication

#endif // LIBSYNDICATION_MAPPER_RSS2MAPPER_H
