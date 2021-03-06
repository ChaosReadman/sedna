/*
 * File:  ASTTextConst.h
 * Copyright (C) 2009 The Institute for System Programming of the Russian Academy of Sciences (ISP RAS)
 */

#ifndef _AST_TEXT_CONST_H_
#define _AST_TEXT_CONST_H_

#include "ASTNode.h"
class ASTVisitor;

class ASTTextConst : public ASTNode
{
public:
    ASTNode *expr; // computed construction expression

    bool deep_copy; // text-node will be attached to virtual_root and copied on demand

public:
    ASTTextConst(const ASTNodeCommonData &loc, ASTNode *expr_) : ASTNode(loc), expr(expr_) {}

    ~ASTTextConst();

    void accept(ASTVisitor &v);
    ASTNode *dup();
    void modifyChild(const ASTNode *oldc, ASTNode *newc);

    static ASTNode *createNode(scheme_list &sl);
};

#endif
