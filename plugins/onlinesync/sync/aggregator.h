#ifndef AGGREGATOR_H
#define AGGREGATOR_H

#include <QObject>

namespace feedsync
{
class SubscriptionList;

class Aggregator: public QObject
{
 public:
    explicit Aggregator( QObject* parent=0 ) : QObject( parent ) {}
    
    virtual ~Aggregator() {};
    virtual SubscriptionList * getSubscriptionList() const = 0;
    virtual void load() = 0;
    virtual void add(SubscriptionList * list) = 0;
    virtual void update(SubscriptionList * list) = 0;
    virtual void remove(SubscriptionList * list) = 0;

 signals:
    void loadDone();
    void addDone();
    void updateDone();
    void removeDone();
    void error(const QString& msg);
};

}

#endif
