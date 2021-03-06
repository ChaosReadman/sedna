/*
 * File:  ASTDocTest.h
 * Copyright (C) 2009 The Institute for System Programming of the Russian Academy of Sciences (ISP RAS)
 */

#ifndef _AST_DOC_TEST_H_
#define _AST_DOC_TEST_H_

#include "ASTNode.h"
class ASTVisitor;

class ASTDocTest : public ASTNode
{
public:
    ASTNode *elem_test; // element test; may be NULL

public:
    ASTDocTest(const ASTNodeCommonData &loc, ASTNode *elem = NULL) : ASTNode(loc), elem_test(elem) {}

    ~ASTDocTest();

    void accept(ASTVisitor &v);
    ASTNode *dup();
    void modifyChild(const ASTNode *oldc, ASTNode *newc);

    static ASTNode *createNode(scheme_list &sl);
};

#endif
