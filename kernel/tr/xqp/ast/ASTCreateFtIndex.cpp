/*
 * File:  ASTCreateFtIndex.cpp
 * Copyright (C) 2009 The Institute for System Programming of the Russian Academy of Sciences (ISP RAS)
 */

#include "tr/xqp/serial/deser.h"

#include "tr/xqp/visitor/ASTVisitor.h"
#include "ASTCreateFtIndex.h"

ASTCreateFtIndex::~ASTCreateFtIndex()
{
    delete name;
    delete path;
    delete type;
    delete cust_expr;
	if (options)
		delete options;
}

void ASTCreateFtIndex::accept(ASTVisitor &v)
{
    v.addToPath(this);
    v.visit(*this);
    v.removeFromPath(this);
}

ASTNode *ASTCreateFtIndex::dup()
{
	return new ASTCreateFtIndex(cd, name->dup(), path->dup(), new std::string(*type), (cust_expr) ? cust_expr->dup() : NULL, (options) ? options->dup() : NULL);
}

ASTNode *ASTCreateFtIndex::createNode(scheme_list &sl)
{
    ASTNodeCommonData cd;
    std::string *type;
    ASTNode *name = NULL, *path = NULL, *cust = NULL, *opts = NULL;

    U_ASSERT(sl[1].type == SCM_LIST && sl[2].type == SCM_LIST && sl[3].type == SCM_LIST && sl[4].type == SCM_STRING && sl[5].type == SCM_LIST && sl[6].type == SCM_LIST);

    cd = dsGetASTCommonFromSList(*sl[1].internal.list);
    name = dsGetASTFromSchemeList(*sl[2].internal.list);
    path = dsGetASTFromSchemeList(*sl[3].internal.list);
    type = new std::string(sl[4].internal.str);
    cust = dsGetASTFromSchemeList(*sl[5].internal.list);
	opts = dsGetASTFromSchemeList(*sl[6].internal.list);

    return new ASTCreateFtIndex(cd, name, path, type, cust, opts);
}

void ASTCreateFtIndex::modifyChild(const ASTNode *oldc, ASTNode *newc)
{
    if (name == oldc)
    {
        name = newc;
        return;
    }
    if (path == oldc)
    {
        path = newc;
        return;
    }
    if (cust_expr == oldc)
    {
        cust_expr = newc;
        return;
    }
	if (options == oldc)
	{
		options = newc;
		return;
	}
}
