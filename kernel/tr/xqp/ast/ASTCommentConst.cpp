/*
 * File:  ASTCommentConst.cpp
 * Copyright (C) 2009 The Institute for System Programming of the Russian Academy of Sciences (ISP RAS)
 */

#include "tr/xqp/serial/deser.h"

#include "tr/xqp/visitor/ASTVisitor.h"
#include "ASTCommentConst.h"

ASTCommentConst::~ASTCommentConst()
{
    delete expr;
}

void ASTCommentConst::accept(ASTVisitor &v)
{
    v.addToPath(this);
    v.visit(*this);
    v.removeFromPath(this);
}

ASTNode *ASTCommentConst::dup()
{
    return new ASTCommentConst(cd, expr->dup());
}

ASTNode *ASTCommentConst::createNode(scheme_list &sl)
{
    ASTNodeCommonData cd;
    ASTNode *expr = NULL;
    ASTCommentConst *res;

    U_ASSERT(sl[1].type == SCM_LIST && sl[2].type == SCM_LIST && sl[3].type == SCM_BOOL);

    cd = dsGetASTCommonFromSList(*sl[1].internal.list);
    expr = dsGetASTFromSchemeList(*sl[2].internal.list);

    res = new ASTCommentConst(cd, expr);

    res->deep_copy = sl[3].internal.b;

    return res;
}

void ASTCommentConst::modifyChild(const ASTNode *oldc, ASTNode *newc)
{
    if (expr == oldc)
    {
        expr = newc;
        return;
    }
}
