#ifndef AGGREGATOR_H
#define AGGREGATOR_H

#include <kconfiggroup.h>

#include "subscriptionlist.h"

namespace feedsync
{

class Aggregator: public QObject
{
 public:
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
