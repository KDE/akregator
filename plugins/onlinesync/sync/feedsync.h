#ifndef FEEDSYNC_H
#define FEEDSYNC_H

#include <QObject>

class SubscriptionList;
class KConfigGroup;

namespace feedsync {

class Aggregator;

class FeedSync : public QObject
{
    Q_OBJECT

 public:
    FeedSync();
    ~FeedSync();
    enum SyncType { Get, Send };

 protected Q_SLOTS:
    void sync();
    void slotLoadDone();
    void slotAddDone();
    void slotRemoveDone();

 private:
    Aggregator * _aggrSend;
    Aggregator * _aggrGet;
    Aggregator * createAggregatorFactory(KConfigGroup configgroup);
    void error();
    int _syncstep;
    int _loadedAggrCount;
    SubscriptionList * tmp_removelist;
    SubscriptionList * tmp_addlist;
};

}

#endif
