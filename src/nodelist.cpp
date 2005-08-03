#include "folder.h"
#include "nodelist.h"
#include "treenode.h"
#include "treenodevisitor.h"

#include <kapplication.h>

#include <qmap.h>
#include <qstring.h>
#include <qvaluelist.h>

namespace Akregator {

class NodeList::NodeListPrivate
{
    public:
    QValueList<TreeNode*> flatList;
    Folder* rootNode;
    QString title;
    QMap<int, TreeNode*> idMap;
    AddNodeVisitor* addNodeVisitor;
    RemoveNodeVisitor* removeNodeVisitor;
};


class NodeList::AddNodeVisitor : public TreeNodeVisitor
{
    public:
        AddNodeVisitor(NodeList* list) : m_list(list) {}


        virtual bool visitTreeNode(TreeNode* node)
        {
            if (!m_preserveID)
            	node->setId(m_list->generateID());
            m_list->d->idMap[node->id()] = node;
            m_list->d->flatList.append(node);
            
            connect(node, SIGNAL(signalDestroyed(TreeNode*)), m_list, SLOT(slotNodeDestroyed(TreeNode*) ));
            m_list->signalNodeAdded(node); // emit

            return true;
        }
        virtual bool visitFolder(Folder* node)
        {
            connect(node, SIGNAL(signalChildAdded(TreeNode*)), m_list, SLOT(slotNodeAdded(TreeNode*) ));
            connect(node, SIGNAL(signalChildRemoved(Folder*, TreeNode*)), m_list, SLOT(slotNodeRemoved(Folder*, TreeNode*) ));

            visitTreeNode(node);

            for (TreeNode* i = node->firstChild(); i && i != node; i = i->next() )
                m_list->slotNodeAdded(i);

            return true;
        }

        virtual void visit(TreeNode* node, bool preserveID)
        {
            m_preserveID = preserveID;
            TreeNodeVisitor::visit(node);
        }

    private:
        NodeList* m_list;
        bool m_preserveID;
};

class NodeList::RemoveNodeVisitor : public TreeNodeVisitor
{
    public:
        RemoveNodeVisitor(NodeList* list) : m_list(list) {}

        virtual bool visitTreeNode(TreeNode* node)
        {
            m_list->d->idMap.remove(node->id());
            m_list->d->flatList.remove(node);

            disconnect(node, SIGNAL(signalDestroyed(TreeNode*)), m_list, SLOT(slotNodeDestroyed(TreeNode*) ));
            m_list->signalNodeRemoved(node); // emit signal
            
            return true;
        }

        virtual bool visitFolder(Folder* node)
        {
            
            disconnect(node, SIGNAL(signalChildAdded(TreeNode*)), m_list, SLOT(slotNodeAdded(TreeNode*) ));
            disconnect(node, SIGNAL(signalChildRemoved(Folder*, TreeNode*)), m_list, SLOT(slotNodeRemoved(Folder*, TreeNode*) ));
            visitTreeNode(node);
          
            return true;
        }
    private:
        NodeList* m_list;
};

NodeList::NodeList(QObject *parent, const char *name) : d(new NodeListPrivate)
{
    d->addNodeVisitor = new AddNodeVisitor(this);
    d->removeNodeVisitor = new RemoveNodeVisitor(this);
}

const QString& NodeList::title() const
{
    return d->title;
}

TreeNode* NodeList::findByID(int id) const
{
    return d->idMap[id];
}

void NodeList::setTitle(const QString& title)
{
    d->title = title;
}

Folder* NodeList::rootNode() const
{
    return d->rootNode;
}

const QValueList<TreeNode*>& NodeList::asFlatList() const
{
    return d->flatList;
}

bool NodeList::isEmpty() const
{
    return d->rootNode->firstChild() == 0;
}

QValueList<TreeNode*>* NodeList::flatList() const
{
    return &(d->flatList);
}

QMap<int, TreeNode*>* NodeList::idMap() const
{
    return &(d->idMap);
}

void NodeList::setRootNode(Folder* folder)
{
    d->rootNode = folder;
}

void NodeList::addNode(TreeNode* node, bool preserveID)
{
    d->addNodeVisitor->visit(node, preserveID);
}

void NodeList::removeNode(TreeNode* node)
{
   d->removeNodeVisitor->visit(node);
}

NodeList::~NodeList()
{
    emit signalDestroyed(this);
    delete d->addNodeVisitor;
    delete d->removeNodeVisitor;
    delete d;
    d = 0;
}

int NodeList::generateID()
{
    return KApplication::random();
}

void NodeList::slotNodeAdded(TreeNode* node)
{
    Folder* parent = node->parent();
    if ( !node || !d->flatList.contains(parent) || d->flatList.contains(node) )
        return;

    addNode(node, false);
}

void NodeList::slotNodeDestroyed(TreeNode* node)
{
    if ( !node || !d->flatList.contains(node) )
        return;

    removeNode(node);
}

void NodeList::slotNodeRemoved(Folder* /*parent*/, TreeNode* node)
{
    if ( !node || !d->flatList.contains(node) )
        return;

    removeNode(node);
}

}

#include "nodelist.moc"
