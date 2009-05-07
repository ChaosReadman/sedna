#include "tr/xqp/visitor/ASTVisitor.h"
#include "ASTSchemaAttrTest.h"

ASTSchemaAttrTest::~ASTSchemaAttrTest()
{
    delete npref;
    delete nloc;
}

void ASTSchemaAttrTest::accept(ASTVisitor &v)
{
    v.visit(*this);
}

ASTNode *ASTSchemaAttrTest::dup()
{
    return new ASTSchemaAttrTest(loc,
                             (npref) ? new std::string(*npref) : NULL,
                             (nloc) ? new std::string(*nloc) : NULL
                            );
}