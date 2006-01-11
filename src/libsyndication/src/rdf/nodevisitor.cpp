#include "literal.h"
#include "node.h"
#include "nodevisitor.h"
#include "property.h"
#include "resource.h"
#include "sequence.h"

namespace LibSyndication {
namespace RDF {

NodeVisitor::~NodeVisitor() {}

void NodeVisitor::visit(NodePtr node)
{
    node->accept(this, node);
}
    
bool NodeVisitor::visitLiteral(LiteralPtr)
{
    return false;
}
        
bool NodeVisitor::visitNode(NodePtr)
{
    return false;
}

bool NodeVisitor::visitProperty(PropertyPtr)
{
    return false;
}
        
bool NodeVisitor::visitResource(ResourcePtr)
{
    return false;
}
        
bool NodeVisitor::visitSequence(SequencePtr)
{
    return false;
}
} // namespace RDF
} // namespace LibSyndication
