/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2005 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/
#ifndef AKREGATOR_TREENODEVISITOR_H
#define AKREGATOR_TREENODEVISITOR_H

#include "akregator_export.h"

namespace Akregator
{
class TreeNode;
class Folder;
class Feed;

class AKREGATOR_EXPORT TreeNodeVisitor
{
public:
    virtual ~TreeNodeVisitor()
    {
    }

    virtual bool visit(TreeNode *node);
    virtual bool visitTreeNode(TreeNode * /*node*/)
    {
        return false;
    }

    virtual bool visitFolder(Folder * /*node*/)
    {
        return false;
    }

    virtual bool visitFeed(Feed * /*node*/)
    {
        return false;
    }
};
} // namespace Akregator

#endif // AKREGATOR_TREENODEVISITOR_H
