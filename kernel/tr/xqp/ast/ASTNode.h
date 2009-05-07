#ifndef _AST_NODE_H_
#define _AST_NODE_H_

//#include "tr/xqp/XQueryParser.hpp"
#include "tr/xqp/location.hh"

class ASTVisitor;
typedef sedna::location ASTLocation;

class ASTNode
{
public:
    ASTLocation loc;

public:
    ASTNode(ASTLocation  &l) : loc(l) {}

    // destructor should deal with children
    virtual ~ASTNode() {}

    // accept visitor
    // children are traversed by visitor
    virtual void accept(ASTVisitor &v) = 0;

    // returns first line of a definition
    int getFirstLine() const
    {
        return loc.begin.line;
    }

    // deep copy of the corresponding subtree
    virtual ASTNode *dup() = 0;
};

#endif