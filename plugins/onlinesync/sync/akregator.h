#ifndef AKREGATOR_H
#define AKREGATOR_H

#include <KApplication>

#include "aggregator.h"
#include "subscriptionlist.h"

namespace feedsync
{

class Akregator: public Aggregator
{
    Q_OBJECT

 public:

    explicit Akregator( QObject* parent=0 );
    ~Akregator();
    SubscriptionList getSubscriptionList() const;
    void load();
    void add(const SubscriptionList & list);
    void update(const SubscriptionList & list);
    void remove(const SubscriptionList & list);

 private:
    SubscriptionList _subscriptionList;

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
