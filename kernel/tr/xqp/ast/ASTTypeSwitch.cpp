#include "tr/xqp/visitor/ASTVisitor.h"
#include "ASTTypeSwitch.h"

ASTTypeSwitch::~ASTTypeSwitch()
{
    delete expr;
    destroyASTNodesVector(cases);
    delete def_case;
}

void ASTTypeSwitch::accept(ASTVisitor &v)
{
    v.visit(*this);
}

ASTNode *ASTTypeSwitch::dup()
{
    return new ASTTypeSwitch(loc, expr->dup(), duplicateASTNodes(cases), static_cast<ASTCase *>(def_case->dup()));
}