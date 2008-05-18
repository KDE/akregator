#ifndef AKREGATOR_H
#define AKREGATOR_H

#include <KApplication>

#include "aggregator.h"

class SubscriptionList;

namespace feedsync
{

class Akregator: public Aggregator
{
    Q_OBJECT

 public:

    Akregator();
    ~Akregator();
    SubscriptionList * getSubscriptionList() const;
    void load();
    void add(SubscriptionList * list);
    void update(SubscriptionList * list);
    void remove(SubscriptionList * list);

 private:
    SubscriptionList * _subscriptionList;

 protected Q_SLOTS:
    void sendSignalLoadDone();

 signals:
    void loadDone();
    void addDone();
    void updateDone();
    void removeDone();
};

}

#endif
