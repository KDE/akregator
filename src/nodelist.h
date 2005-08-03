#ifndef AKREGATOR_NODELIST_H
#define AKREGATOR_NODELIST_H

#include <qobject.h>

class QDomDocument;
class QDomNode;
class QString;

template <class K,class T> class QMap;
template <class T> class QValueList;

namespace Akregator {

class Folder;
class TreeNode;

class NodeList : public QObject
{

Q_OBJECT

public:
    NodeList(QObject *parent=0, const char *name=0);
    virtual ~NodeList();

    virtual Folder* rootNode() const;

    virtual bool readFromXML(const QDomDocument& doc) = 0;

    virtual QDomDocument toXML() const = 0;

    virtual bool isEmpty() const;

    TreeNode* findByID(int id) const;

    /** returns the title of the feed list (as used in the OPML document) */
    const QString& title() const;

    /** sets the title of the feed list */
    void setTitle(const QString& name);

    /** returns a flat list containing all nodes in the tree */
    const QValueList<TreeNode*>& asFlatList() const;

signals:
    void signalDestroyed(NodeList*);
    /** emitted when a node was added to the list */
    void signalNodeAdded(TreeNode*);
    /** emitted when a node was removed from the list */
    void signalNodeRemoved(TreeNode*);

protected:

    QValueList<TreeNode*>* flatList() const;
    QMap<int, TreeNode*>* idMap() const;
    
    int generateID();
    void setRootNode(Folder* folder);
    virtual void addNode(TreeNode* node, bool preserveID);
    virtual void removeNode(TreeNode* node);
  
protected slots:

    virtual void slotNodeDestroyed(TreeNode* node);
    virtual void slotNodeAdded(TreeNode* node);
    virtual void slotNodeRemoved(Folder* parent, TreeNode* node);

private:
    NodeList(const NodeList&) : QObject() {}
    NodeList& operator=(const NodeList&) { return *this; }

    friend class AddNodeVisitor;
    class AddNodeVisitor;

    friend class RemoveNodeVisitor;
    class RemoveNodeVisitor;

    class NodeListPrivate;
    NodeListPrivate* d;
    
};

} // namespace Akregator

#endif // AKREGATOR_NODELIST_H
