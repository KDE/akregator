#ifndef AKREGATOR_DRAGOBJECTS_H
#define AKREGATOR_DRAGOBJECTS_H

#include "article.h"

#include <kurl.h>
#include <kurldrag.h>

#include <qstring.h>
#include <qvaluelist.h>

typedef class QMemArray<char> QByteArray;

namespace Akregator {

class Article;

struct ArticleDragItem
{
    QString feedURL;
    QString guid;
};

class ArticleDrag : public KURLDrag
{

public:

    ArticleDrag(const QValueList<Article>& articles, QWidget* dragSource=0, const char* name=0);

    static bool canDecode(const QMimeSource* e);
    static bool decode(const QMimeSource* e, QValueList<ArticleDragItem>& articles);

protected:

    virtual QByteArray encodedData(const char* mime) const;
    virtual const char* format(int i) const;

private:

    static QValueList<ArticleDragItem> articlesToDragItems(const QValueList<Article>& articles);
    static KURL::List articleURLs(const QValueList<Article>& articles);
    QValueList<ArticleDragItem> m_items;
};

} // namespace Akregator

#endif // AKREGATOR_DRAGOBJECTS_H
