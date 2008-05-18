#ifndef SUBSCRIPTIONLIST_H
#define SUBSCRIPTIONLIST_H

#include <KApplication>

namespace feedsync
{

class SubscriptionList: public QObject
{
    Q_OBJECT

 public:
    SubscriptionList();
    ~SubscriptionList();
    void add(const QString& iRss, const QString& iName, const QString& iCat);
    void remove(const QString& iRss, const QString& iName, const QString& iCat);

    enum ComparisonType {
        Added, Removed
    };
    enum RemovePolicy {
        Nothing, Category, Feed
    };
    enum CategoryFormat {
        Simple, Full
    };

    SubscriptionList * compare(SubscriptionList * iOther, ComparisonType diffType, RemovePolicy removePolicy=Nothing) const;
    bool rssExist(const QString& iRss,const QString& iName,const QString& iCat) const;
    QString getRss(int index) const;
    QString getName(int index) const;
    QString getCat(int index, CategoryFormat format=Simple) const;

    int count() const;
    int countRss(const QString& iRss) const;
    int indexOf(const QString& iRss, const QString& iName, const QString& iCat) const;
    int indexOf(const QString& iRss) const;



 private:
    QList<QString> _rssList;
    QList<QString> _nameList;
    QList<QString> _catListFull;
    QList<QString> _catListSimple;
};

}

#endif
