/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2005 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "treenodevisitor.h"
#include "treenode.h"

using namespace Akregator;
bool TreeNodeVisitor::visit(TreeNode *node)
{
    return node->accept(this);
}
