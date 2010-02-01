/*
 * File:  PPExplainVisitor.cpp
 * Copyright (C) 2009 The Institute for System Programming of the Russian Academy of Sciences (ISP RAS)
 */

#include <string>

#include "common/sedna.h"
#include "common/base.h"
#include "common/u/uutils.h"

#include "tr/executor/base/visitor/PPExplainVisitor.h"
#include "tr/executor/base/tuple.h"
#include "tr/executor/fo/casting_operations.h"
#include "tr/cat/catptr.h"
#include "tr/mo/mo.h"

using namespace std;

static xmlns_ptr explain_ns = NULL_XMLNS;

static inline string 
arr_of_var_dsc2string(const arr_of_var_dsc& var_dscs)
{
    string variables;
    for (unsigned int i = 0; i < var_dscs.size(); i++)
    {
        variables += int2string(var_dscs.at(i));
        if(i != var_dscs.size() -1) variables += ", ";
    }
    return variables;
}

static inline string
bool2string(bool b)
{
    return b ? string("true") : string("false");
}

static inline 
xptr insertAttributeHelper(const char* name,
                           const xptr& left,
                           const xptr& parent,
                           const string& value)
{
    return insert_attribute_i(left,XNULL,parent,name,xs_untypedAtomic,value.c_str(),value.length(),NULL_XMLNS);
}
                             

static inline 
xptr insertElementHelper(const char* name,
                         const xptr& left,
                         const xptr& parent)
{
    return insert_element_i(left,XNULL,parent,name,xs_untyped,explain_ns);
}

static inline 
xptr insertVariableHelper(const char* name,
                          const xptr& left,
                          const xptr& parent,
                          int dsc)
{
    xptr var = left;
    if(dsc >= 0)
    {
        var = insert_element_i(left,XNULL,parent,"variable",xs_untyped,explain_ns);
        xptr attr_left = insertAttributeHelper("descriptor", XNULL, var, int2string(dsc));
        if(name != NULL)
        {
            insertAttributeHelper("name", attr_left, var, string(name));
        }
    }
    return var;
}


PPExplainVisitor::PPExplainVisitor(dynamic_context* _cxt_,
                                   xptr _root_) : cxt(_cxt_), 
                                                  parent(_root_),
                                                  left(XNULL),
                                                  left_inside(XNULL)
{
    explain_ns = cxt->st_cxt->get_default_namespace();
}

PPExplainVisitor::~PPExplainVisitor() 
{
}

void PPExplainVisitor::push()
{
    U_ASSERT(left != XNULL);
    
    pointers.push(xptr_pair(parent, left));
    parent = left;
    left = left_inside;
    left_inside = XNULL;
}

void PPExplainVisitor::pop()
{
    U_ASSERT(!pointers.empty());    

    xptr_pair ptrs = pointers.top();
    pointers.pop();
    parent = ptrs.first;
    left   = ptrs.second;
    left_inside = XNULL;
}
   

/* Helper to insert operation nodes */
void PPExplainVisitor::insertOperationElement(const char* name, xptr& left, const xptr& parent, PPIterator* op = NULL)
{
    U_ASSERT(parent != XNULL);
    elog(EL_DBG, ("[EXPLAIN] Going to insert element '%s', parent (0x%x, 0x%x), left (0x%x, 0x%x)", name, 
                                                           parent.layer, parent.addr, 
                                                           left.layer, left.addr));
    left = insert_element_i(left,XNULL,parent,"operation",xs_untyped,explain_ns);
    xptr attr_left = insert_attribute_i(XNULL,XNULL,left,"name",xs_untypedAtomic, name, strlen(name), NULL_XMLNS);

    if(NULL != op) 
    {
        char buf[20];
        if(op->get_operation_info().query_line != 0)
        {
            u_itoa(op->get_operation_info().query_line,buf,10);
            attr_left = insert_attribute_i(attr_left,XNULL,left,"line",xs_untypedAtomic,buf,strlen(buf),NULL_XMLNS);
        }
        if(op->get_operation_info().query_col != 0)
        {
            u_itoa(op->get_operation_info().query_col,buf,10);
            attr_left = insert_attribute_i(attr_left,XNULL,left,"column",xs_untypedAtomic,buf,strlen(buf),NULL_XMLNS);
        }
    }
}


void PPExplainVisitor::visit(PPDmStringValue* op)
{
    insertOperationElement("PPDmStringValue", left, parent, op);
}

void PPExplainVisitor::visit(PPDmTypedValue* op)
{
    insertOperationElement("PPDmTypedValue", left, parent, op);
}

void PPExplainVisitor::visit(PPDmNodeKind* op)
{
    insertOperationElement("PPDmNodeKind", left, parent, op);
}

void PPExplainVisitor::visit(PPFnNodeName* op)
{
    insertOperationElement("PPFnNodeName", left, parent, op);
}

void PPExplainVisitor::visit(PPFnNilled* op)
{
    insertOperationElement("PPFnNilled", left, parent, op);
}

void PPExplainVisitor::visit(PPFnString* op)
{
    insertOperationElement("PPFnString", left, parent, op);
}

void PPExplainVisitor::visit(PPFnData* op)
{
    insertOperationElement("PPFnData", left, parent, op);
}

void PPExplainVisitor::visit(PPFnBaseURI* op)
{
    insertOperationElement("PPFnBaseURI", left, parent, op);
}

void PPExplainVisitor::visit(PPFnDocumentURI* op)
{
    insertOperationElement("PPFnDocumentURI", left, parent, op);
}

void PPExplainVisitor::visit(PPFnStaticBaseUri* op)
{
    insertOperationElement("PPFnStaticBaseUri", left, parent, op);
}

void PPExplainVisitor::visit(PPFnDefaultCollation* op)
{
    insertOperationElement("PPFnDefaultCollation", left, parent, op);
}

void PPExplainVisitor::visit(PPFnCount* op)
{
    insertOperationElement("PPFnCount", left, parent, op);
}

void PPExplainVisitor::visit(PPFnMaxMin* op)
{
    insertOperationElement("PPFnMaxMin", left, parent, op);
    const char* function = op->get_function_name();
    insert_attribute_i(XNULL,XNULL,left,"function",xs_untypedAtomic, function, strlen(function),NULL_XMLNS);
}

void PPExplainVisitor::visit(PPFnSumAvg* op)
{
    insertOperationElement("PPFnSumAvg", left, parent, op);
    const char* function = op->get_function_name();
    insert_attribute_i(XNULL,XNULL,left,"function",xs_untypedAtomic, function, strlen(function),NULL_XMLNS);
}

void PPExplainVisitor::visit(PPAbsPath* op)
{
    insertOperationElement("PPAbsPath", left, parent, op);
    string path_expr = op->getPathExpr()->to_string();
    counted_ptr<db_entity> dbe = op->getDocColl();
    string root;
    switch(dbe->type)
    {
        case dbe_document: root+= "document("; break;
        case dbe_collection: root += "collection("; break;
        case dbe_module: root += "module("; break;
    }
    if(NULL != dbe->name) root += dbe->name;
    root += ")";
    xptr attr_left = insertAttributeHelper("root", XNULL, left, root);
    if(path_expr.length() != 0)
    {
        insertAttributeHelper("path",attr_left, left, path_expr);
    }
}

void PPExplainVisitor::visit(PPAxisAncestor* op)
{
    insertOperationElement("PPAxisAncestor", left, parent, op);
    string node_test = "ancestor::" + NodeTest::to_string(op->get_node_test_type(),
                                                          op->get_node_test_data());
    insert_attribute_i(XNULL,XNULL,left,"step",xs_untypedAtomic, node_test.c_str(), node_test.length(),NULL_XMLNS);
}

void PPExplainVisitor::visit(PPAxisAncestorOrSelf* op)
{
    insertOperationElement("PPAxisAncestorOrSelf", left, parent, op);
    string node_test = "ancestor-or-self::" + NodeTest::to_string(op->get_node_test_type(),
                                                                  op->get_node_test_data());
    insert_attribute_i(XNULL,XNULL,left,"step",xs_untypedAtomic, node_test.c_str(), node_test.length(), NULL_XMLNS);
}

void PPExplainVisitor::visit(PPAxisAttribute* op)
{
    insertOperationElement("PPAxisAttribute", left, parent, op);
    string node_test = "attribute::" + NodeTest::to_string(op->get_node_test_type(),
                                                           op->get_node_test_data());
    insert_attribute_i(XNULL,XNULL,left,"step",xs_untypedAtomic, node_test.c_str(), node_test.length(), NULL_XMLNS);
}

void PPExplainVisitor::visit(PPAxisChild* op)
{
    insertOperationElement("PPAxisChild", left, parent, op);
    string node_test = "child::" + NodeTest::to_string(op->get_node_test_type(),
                                                       op->get_node_test_data());
    insert_attribute_i(XNULL,XNULL,left,"step",xs_untypedAtomic, node_test.c_str(), node_test.length(), NULL_XMLNS);
}

void PPExplainVisitor::visit(PPAxisDescendant* op)
{
    insertOperationElement("PPAxisDescendant", left, parent, op);
    string node_test = "descendant::" + NodeTest::to_string(op->get_node_test_type(),
                                                            op->get_node_test_data());
    insert_attribute_i(XNULL,XNULL,left,"step",xs_untypedAtomic, node_test.c_str(), node_test.length(), NULL_XMLNS);
}

void PPExplainVisitor::visit(PPAxisDescendantOrSelf* op)
{
    insertOperationElement("PPAxisDescendantOrSelf", left, parent, op);
    string node_test = "descendant-or-self::" + NodeTest::to_string(op->get_node_test_type(),
                                                                    op->get_node_test_data());
    insert_attribute_i(XNULL,XNULL,left,"step",xs_untypedAtomic, node_test.c_str(), node_test.length(), NULL_XMLNS);
}

void PPExplainVisitor::visit(PPAxisDescendantAttr* op)
{
    insertOperationElement("PPAxisDescendantAttr", left, parent, op);
    string node_test = "@descendant::" + NodeTest::to_string(op->get_node_test_type(),
                                                             op->get_node_test_data());
    insert_attribute_i(XNULL,XNULL,left,"step",xs_untypedAtomic, node_test.c_str(), node_test.length(), NULL_XMLNS);
}

void PPExplainVisitor::visit(PPAxisFP* op)
{
    insertOperationElement("PPAxisFP", left, parent, op);
    string node_test = (op->is_following() ? "following::" : "preceding::") + NodeTest::to_string(op->get_node_test_type(),
                                                                  op->get_node_test_data());
    insert_attribute_i(XNULL,XNULL,left,"step",xs_untypedAtomic, node_test.c_str(), node_test.length(), NULL_XMLNS);
}

void PPExplainVisitor::visit(PPAxisParent* op)
{
    insertOperationElement("PPAxisParent", left, parent, op);
    string node_test = "parent::" + NodeTest::to_string(op->get_node_test_type(),
                                                        op->get_node_test_data());
    insert_attribute_i(XNULL,XNULL,left,"step",xs_untypedAtomic, node_test.c_str(), node_test.length(), NULL_XMLNS);
}

void PPExplainVisitor::visit(PPAxisSelf* op)
{
    insertOperationElement("PPAxisSelf", left, parent, op);
    string node_test = "self::" + NodeTest::to_string(op->get_node_test_type(),
                                                      op->get_node_test_data());
    insert_attribute_i(XNULL,XNULL,left,"step",xs_untypedAtomic, node_test.c_str(), node_test.length(), NULL_XMLNS);
}

void PPExplainVisitor::visit(PPAxisSibling* op)
{
    insertOperationElement("PPAxisSibling", left, parent, op);
    string node_test = (op->is_following() ? "following-sibling::" : "preceding-sibling::") + NodeTest::to_string(op->get_node_test_type(),
                                                                  op->get_node_test_data());
    insert_attribute_i(XNULL,XNULL,left,"step",xs_untypedAtomic, node_test.c_str(), node_test.length(), NULL_XMLNS);
}

void PPExplainVisitor::visit(PPPred1* op)
{
    insertOperationElement("PPPred1", left, parent, op);
    insertAttributeHelper("once", XNULL, left, bool2string(op->is_once()));

    /* Insert conjuncts details */
    unsigned int conj_num = op->get_conjuncts_number();
    left_inside = insertElementHelper("conjuncts", XNULL, left);
    xptr conj_left = XNULL;
    for (unsigned int i = 0; i < conj_num; i++) {
        conj_left = insertElementHelper("conjunct", conj_left, left_inside);
        insertAttributeHelper("comparison", XNULL, conj_left, operation_compare_condition2string(op->get_conjunt_comparison_type(i)));
    }

    /* Insert variables details */    
    left_inside = insertElementHelper("produces", left_inside, left);
    const arr_of_var_dsc& var_dscs = op->get_variable_descriptors();
    xptr var_left = XNULL;
    for (unsigned int i = 0; i < var_dscs.size(); i++)
    {
        var_left = insertVariableHelper(NULL, var_left, left_inside, var_dscs.at(i));
    }
    insertVariableHelper("position", var_left, left_inside, op->get_position_var_dsc());
}

void PPExplainVisitor::visit(PPPred2* op)
{
    insertOperationElement("PPPred2", left, parent, op);
    insertAttributeHelper("once", XNULL, left, bool2string(op->is_once()));

    /* Insert conjuncts details */
    unsigned int conj_num = op->get_conjuncts_number();
    left_inside = insertElementHelper("conjuncts", XNULL, left);
    xptr conj_left = XNULL;
    for (unsigned int i = 0; i < conj_num; i++) {
        conj_left = insertElementHelper("conjunct", conj_left, left_inside);
        insertAttributeHelper("comparison", XNULL, conj_left, operation_compare_condition2string(op->get_conjunt_comparison_type(i)));
    }

    /* Insert variables details */    
    left_inside = insertElementHelper("produces", left_inside, left);
    const arr_of_var_dsc& var_dscs = op->get_variable_descriptors();
    xptr var_left = XNULL;
    for (unsigned int i = 0; i < var_dscs.size(); i++)
    {
        var_left = insertVariableHelper(NULL, var_left, left_inside, var_dscs.at(i));
    }
    insertVariableHelper("position", var_left, left_inside, op->get_position_var_dsc());
    insertVariableHelper("last", var_left, left_inside, op->get_last_var_dsc());
}

void PPExplainVisitor::visit(PPFnTrue* op)
{
    insertOperationElement("PPFnTrue", left, parent, op);
}

void PPExplainVisitor::visit(PPFnFalse* op)
{
    insertOperationElement("PPFnFalse", left, parent, op);
}

void PPExplainVisitor::visit(PPFnNot* op)
{
    insertOperationElement("PPFnNot", left, parent, op);
}

void PPExplainVisitor::visit(PPFnBoolean* op)
{
    insertOperationElement("PPFnBoolean", left, parent, op);
}

void PPExplainVisitor::visit(PPCalculate* op)
{
    insertOperationElement("PPCalculate", left, parent, op);
}

void PPExplainVisitor::visit(UnaryOp* op)
{
    insertOperationElement("UnaryOp", left, parent);
    const char* operation =  xq_unary_op_type2string(op->get_operation_type());
    insert_attribute_i(XNULL,XNULL,left,"operation",xs_untypedAtomic, operation, strlen(operation), NULL_XMLNS);
}

void PPExplainVisitor::visit(BinaryOp* op)
{
    insertOperationElement("BinaryOp", left, parent);
    const char* operation =  xq_binary_op_type2string(op->get_operation_type());
    insert_attribute_i(XNULL,XNULL,left,"operation",xs_untypedAtomic, operation, strlen(operation), NULL_XMLNS);
}

void PPExplainVisitor::visit(BinaryOpCollation* op)
{
    insertOperationElement("BinaryOpCollation", left, parent);
    const char* operation =  xq_binary_op_type2string(op->get_operation_type());
    insert_attribute_i(XNULL,XNULL,left,"operation",xs_untypedAtomic, operation, strlen(operation), NULL_XMLNS);
}

void PPExplainVisitor::visit(BinaryOpAnd* op)
{
    insertOperationElement("BinaryOpAnd", left, parent);
}

void PPExplainVisitor::visit(BinaryOpOr* op)
{
    insertOperationElement("BinaryOpOr", left, parent);
}

void PPExplainVisitor::visit(LeafAtomOp* op)
{
    insertOperationElement("LeafAtomOp", left, parent);
}

void PPExplainVisitor::visit(LeafEffectBoolOp* op)
{
    insertOperationElement("LeafEffectBoolOp", left, parent);
}

void PPExplainVisitor::visit(PPElementConstructor* op)
{
    insertOperationElement("PPElementConstructor", left, parent, op);
    xptr attr_left = XNULL;
    const char* name = op->get_name();
    if(NULL != name)
    {
        attr_left = insert_attribute_i(XNULL,XNULL,left,"element-name",xs_untypedAtomic, name, strlen(name), NULL_XMLNS);
    }
    const char* deep_copy = op->is_deep_copy() ? "true" : "false";
    const char* ns_inside = op->is_ns_inside() ? "true" : "false";
    attr_left = insert_attribute_i(attr_left,XNULL,left,"deep-copy",xs_untypedAtomic, deep_copy, strlen(deep_copy), NULL_XMLNS);
    insert_attribute_i(attr_left,XNULL,left,"namespace-iside",xs_untypedAtomic, ns_inside, strlen(ns_inside), NULL_XMLNS);
}

void PPExplainVisitor::visit(PPAttributeConstructor* op)
{
    insertOperationElement("PPAttributeConstructor", left, parent, op);
    xptr attr_left = XNULL;
    const char* name = op->get_name();
    if(NULL != name)
    {
        attr_left = insert_attribute_i(XNULL,XNULL,left,"attribute-name",xs_untypedAtomic, name, strlen(name), NULL_XMLNS);
    }
    const char* value = op->get_value();
    if(NULL != value)
    {
        attr_left = insert_attribute_i(attr_left,XNULL,left,"attribute-value",xs_untypedAtomic, value, strlen(value), NULL_XMLNS);
    }
    const char* deep_copy = op->is_deep_copy() ? "true" : "false";
    insert_attribute_i(attr_left,XNULL,left,"deep-copy",xs_untypedAtomic, deep_copy, strlen(deep_copy), NULL_XMLNS);
}

void PPExplainVisitor::visit(PPNamespaceConstructor* op)
{
    insertOperationElement("PPNamespaceConstructor", left, parent, op);
    xptr attr_left = XNULL;
    const char* name = op->get_name();
    if(NULL != name)
    {
        attr_left = insert_attribute_i(XNULL,XNULL,left,"namespace-name",xs_untypedAtomic, name, strlen(name), NULL_XMLNS);
    }
    const char* value = op->get_value();
    if(NULL != value)
    {
        insert_attribute_i(attr_left,XNULL,left,"namespace-value",xs_untypedAtomic, value, strlen(value), NULL_XMLNS);
    }
}

void PPExplainVisitor::visit(PPCommentConstructor* op)
{
    insertOperationElement("PPCommentConstructor", left, parent, op);
    xptr attr_left = XNULL;
    const char* value = op->get_value();
    if(NULL != value)
    {
        attr_left = insert_attribute_i(XNULL,XNULL,left,"comment-value",xs_untypedAtomic, value, strlen(value), NULL_XMLNS);
    }
    const char* deep_copy = op->is_deep_copy() ? "true" : "false";
    insert_attribute_i(attr_left,XNULL,left,"deep-copy",xs_untypedAtomic, deep_copy, strlen(deep_copy), NULL_XMLNS);
}

void PPExplainVisitor::visit(PPTextConstructor* op)
{
    insertOperationElement("PPTextConstructor", left, parent, op);
    xptr attr_left = XNULL;
    const char* value = op->get_value();
    if(NULL != value)
    {
        attr_left = insert_attribute_i(XNULL,XNULL,left,"text-value",xs_untypedAtomic, value, strlen(value), NULL_XMLNS);
    }
    const char* deep_copy = op->is_deep_copy() ? "true" : "false";
    insert_attribute_i(attr_left,XNULL,left,"deep-copy",xs_untypedAtomic, deep_copy, strlen(deep_copy), NULL_XMLNS);
}

void PPExplainVisitor::visit(PPDocumentConstructor* op)
{
    insertOperationElement("PPDocumentConstructor", left, parent, op);
}

void PPExplainVisitor::visit(PPPIConstructor* op)
{
    insertOperationElement("PPPIConstructor", left, parent, op);
    xptr attr_left = XNULL;
    const char* name = op->get_name();
    if(NULL != name)
    {
        attr_left = insert_attribute_i(XNULL,XNULL,left,"pi-name",xs_untypedAtomic, name, strlen(name), NULL_XMLNS);
    }
    const char* value = op->get_value();
    if(NULL != value)
    {
        attr_left = insert_attribute_i(attr_left,XNULL,left,"pi-value",xs_untypedAtomic, value, strlen(value), NULL_XMLNS);
    }
    const char* deep_copy = op->is_deep_copy() ? "true" : "false";
    insert_attribute_i(attr_left,XNULL,left,"deep-copy",xs_untypedAtomic, deep_copy, strlen(deep_copy), NULL_XMLNS);
}

void PPExplainVisitor::visit(PPFnError* op)
{
    insertOperationElement("PPFnError", left, parent, op);
}

void PPExplainVisitor::visit(PPFnTrace* op)
{
    insertOperationElement("PPFnTrace", left, parent, op);
}

void PPExplainVisitor::visit(PPDebug* op)
{
    insertOperationElement("PPDebug", left, parent, op);
}

void PPExplainVisitor::visit(PPExcept* op)
{
    insertOperationElement("PPExcept", left, parent, op);
    const char* doc_order = op->is_document_order() ? "true" : "false";
    insert_attribute_i(XNULL,XNULL,left,"doc-order",xs_untypedAtomic,doc_order,strlen(doc_order),NULL_XMLNS);
}

void PPExplainVisitor::visit(PPUnion* op)
{
    insertOperationElement("PPUnion", left, parent, op);
    const char* doc_order = op->is_document_order() ? "true" : "false";
    insert_attribute_i(XNULL,XNULL,left,"doc-order",xs_untypedAtomic,doc_order,strlen(doc_order),NULL_XMLNS);
}

void PPExplainVisitor::visit(PPIntersect* op)
{
    insertOperationElement("PPIntersect", left, parent, op);
    const char* doc_order = op->is_document_order() ? "true" : "false";
    insert_attribute_i(XNULL,XNULL,left,"doc-order",xs_untypedAtomic,doc_order,strlen(doc_order),NULL_XMLNS);
}

void PPExplainVisitor::visit(PPFnDeepEqual* op)
{
    insertOperationElement("PPFnDeepEqual", left, parent, op);
}

void PPExplainVisitor::visit(PPFnDocAvailable* op)
{
    insertOperationElement("PPFnDocAvailable", left, parent, op);
}

void PPExplainVisitor::visit(PPRange* op)
{
    insertOperationElement("PPRange", left, parent, op);
}

void PPExplainVisitor::visit(PPSequence* op)
{
    insertOperationElement("PPSequence", left, parent, op);
}

void PPExplainVisitor::visit(PPSpaceSequence* op)
{
    insertOperationElement("PPSpaceSequence", left, parent, op);
    const char* atomized = op->is_atomized() ? "true" : "false";
    insert_attribute_i(XNULL,XNULL,left,"doc-order",xs_untypedAtomic,atomized,strlen(atomized),NULL_XMLNS);
}

void PPExplainVisitor::visit(PPFnEmpty* op)
{
    insertOperationElement("PPFnEmpty", left, parent, op);
}

void PPExplainVisitor::visit(PPFnExists* op)
{
    insertOperationElement("PPFnExists", left, parent, op);
}

void PPExplainVisitor::visit(PPFnItemAt* op)
{
    insertOperationElement("PPFnItemAt", left, parent, op);
}

void PPExplainVisitor::visit(PPFnDistinctValues* op)
{
    insertOperationElement("PPFnDistinctValues", left, parent, op);
}

void PPExplainVisitor::visit(PPFnIndexOf* op)
{
    insertOperationElement("PPFnIndexOf", left, parent, op);
}

void PPExplainVisitor::visit(PPFnReverse* op)
{
    insertOperationElement("PPFnReverse", left, parent, op);
}

void PPExplainVisitor::visit(PPFnSubsequence* op)
{
    insertOperationElement("PPFnSubsequence", left, parent, op);
}

void PPExplainVisitor::visit(PPFnRemove* op)
{
    insertOperationElement("PPFnRemove", left, parent, op);
}

void PPExplainVisitor::visit(PPFnInsertBefore* op)
{
    insertOperationElement("PPFnInsertBefore", left, parent, op);
}

void PPExplainVisitor::visit(PPFnZeroOrOne* op)
{
    insertOperationElement("PPFnZeroOrOne", left, parent, op);
}

void PPExplainVisitor::visit(PPFnOneOrMore* op)
{
    insertOperationElement("PPFnOneOrMore", left, parent, op);
}

void PPExplainVisitor::visit(PPFnExactlyOne* op)
{
    insertOperationElement("PPFnExactlyOne", left, parent, op);
}

void PPExplainVisitor::visit(PPFnDateTimeFuncNoParam* op)
{
    insertOperationElement("PPFnDateTimeFuncNoParam", left, parent, op);
    const char* function = PPFnDateTimeFuncNoParam::dateTimeFunc2string(op->get_function_type());
    insert_attribute_i(XNULL,XNULL,left,"function",xs_untypedAtomic, function, strlen(function), NULL_XMLNS);
}

void PPExplainVisitor::visit(PPFnDateTimeFunc* op)
{
    insertOperationElement("PPFnDateTimeFunc", left, parent, op);
    const char* function = PPFnDateTimeFunc::dateTimeFunc2string(op->get_function_type());
    insert_attribute_i(XNULL,XNULL,left,"function",xs_untypedAtomic, function, strlen(function), NULL_XMLNS);
}

void PPExplainVisitor::visit(PPFnDateTimeFunc2Params* op)
{
    insertOperationElement("PPFnDateTimeFunc2Params", left, parent, op);
    const char* function = PPFnDateTimeFunc2Params::dateTimeFunc2string(op->get_function_type());
    insert_attribute_i(XNULL,XNULL,left,"function",xs_untypedAtomic, function, strlen(function), NULL_XMLNS);
}

void PPExplainVisitor::visit(PPFilterEL* op)
{
    insertOperationElement("PPFilterEL", left, parent, op);
}

void PPExplainVisitor::visit(PPCheckpoint* op)
{
    insertOperationElement("PPCheckpoint", left, parent, op);
}

void PPExplainVisitor::visit(PPTest* op)
{
    insertOperationElement("PPTest", left, parent, op);
}

void PPExplainVisitor::visit(PPConst* op)
{
    insertOperationElement("PPConst", left, parent, op);
    tuple_cell tc = op->get_tuple_cell();
    string type = tc.type2string();
    xptr attr_left = insert_attribute_i(XNULL,XNULL,left,"type",xs_untypedAtomic, type.c_str(), type.size(), NULL_XMLNS);
    if(is_castable(tc, xs_string))
    {
    	tc = tuple_cell::make_sure_light_atomic(cast(tc, xs_string));
	    insert_attribute_i(attr_left,XNULL,left,"value",xs_untypedAtomic, tc.get_str_mem(), tc.get_strlen_mem(), NULL_XMLNS);
    }
}

void PPExplainVisitor::visit(PPDDO* op)
{
    insertOperationElement("PPDDO", left, parent, op);
}

void PPExplainVisitor::visit(PPSXptr* op)
{
    insertOperationElement("PPSXptr", left, parent, op);
}

void PPExplainVisitor::visit(PPDocInCol* op)
{
    insertOperationElement("PPDocInCol", left, parent, op);
}

void PPExplainVisitor::visit(PPExtFunCall* op)
{
    insertOperationElement("PPExtFunCall", left, parent, op);
    insertAttributeHelper("function-name", XNULL, left, op->get_name());
}

void PPExplainVisitor::visit(PPFnGetProperty* op)
{
    insertOperationElement("PPFnGetProperty", left, parent, op);
}

void PPExplainVisitor::visit(PPIndexScan* op)
{
    insertOperationElement("PPIndexScan", left, parent, op);
    insertAttributeHelper("index-scan-condition", 
                             XNULL, 
                             left, 
                             string(index_scan_condition2string(op->get_index_scan_condition())));
}

void PPExplainVisitor::visit(PPLast* op)
{
    insertOperationElement("PPLast", left, parent, op);
    insertAttributeHelper("last-variable", XNULL, left, int2string(op->get_last_var_dsc()));
}

void PPExplainVisitor::visit(PPNil* op)
{
    insertOperationElement("PPNil", left, parent, op);
}

void PPExplainVisitor::visit(PPSelect* op)
{
    insertOperationElement("PPSelect", left, parent, op);
    xptr attr_left = XNULL;
    if(op->is_check_type())
    {
        attr_left = insertAttributeHelper("type", XNULL, left, (op->get_sequence_type()).to_str());
    }
    insertAttributeHelper("variables", attr_left, left, arr_of_var_dsc2string(op->get_variable_descriptors()));
}

void PPExplainVisitor::visit(PPSeqChecker* op)
{
    insertOperationElement("PPSeqChecker", left, parent, op);
    insertAttributeHelper("mode", XNULL, left, PPSeqChecker::CheckMode2string(op->get_check_mode()));
}

void PPExplainVisitor::visit(PPStore* op)
{
    insertOperationElement("PPStore", left, parent, op);
}

void PPExplainVisitor::visit(PPTuple* op)
{
    insertOperationElement("PPTuple", left, parent, op);
}

void PPExplainVisitor::visit(PPVarDecl* op)
{
    insertOperationElement("PPVarDecl", left, parent, op);
    xptr attr_left = XNULL;
    if(op->is_check_type())
    {
        attr_left = insertAttributeHelper("type", XNULL, left, (op->get_type()).to_str());
    }
    insertAttributeHelper("descriptor", attr_left, left, int2string(op->get_variable_descriptor()));
}

void PPExplainVisitor::visit(PPVariable* op)
{
    insertOperationElement("PPVariable", left, parent, op);
    insertAttributeHelper("descriptor", XNULL, left, int2string(op->get_variable_descriptor()));
}

void PPExplainVisitor::visit(PPGlobalVariable* op)
{
    insertOperationElement("PPGlobalVariable", left, parent, op);
    insertAttributeHelper("descriptor", XNULL, left, int2string(op->get_variable_descriptor()));
}

void PPExplainVisitor::visit(PPXptr* op)
{
    insertOperationElement("PPXptr", left, parent, op);
    insertAttributeHelper("type", XNULL, left, string(trigger_parameter_type2c_string(op->get_type())));
}



#ifdef SE_ENABLE_DTSEARCH
void PPExplainVisitor::visit(PPFtHighlight* op)
{
    insertOperationElement("PPFtHighlight", left, parent, op);
}
void PPExplainVisitor::visit(PPFtScan* op)
{
    insertOperationElement("PPFtScan", left, parent, op);
}
#endif /* SE_ENABLE_DTSEARCH */



#ifdef SE_ENABLE_FTSEARCH
void PPExplainVisitor::visit(PPFtIndexScan* op)
{
    insertOperationElement("PPFtIndexScan", left, parent, op);
}
void PPExplainVisitor::visit(PPFtIndexScan2* op)
{
    insertOperationElement("PPFtIndexScan2", left, parent, op);
}
#endif /* SE_ENABLE_FTSEARCH */



void PPExplainVisitor::visit(PPFunCall* op)
{
    insertOperationElement("PPFunCall", left, parent, op);
}

void PPExplainVisitor::visit(PPGeneralComparison* op)
{
    insertOperationElement("PPGeneralComparison", left, parent, op);
}

void PPExplainVisitor::visit(PPLMGeneralComparison* op)
{
    insertOperationElement("PPLMGeneralComparison", left, parent, op);
}

void PPExplainVisitor::visit(PPNEQGeneralComparison* op)
{
    insertOperationElement("PPNEQGeneralComparison", left, parent, op);
}

void PPExplainVisitor::visit(PPEQLGeneralComparison* op)
{
    insertOperationElement("PPEQLGeneralComparison", left, parent, op);
}

void PPExplainVisitor::visit(PPNodeComparison* op)
{
    insertOperationElement("PPNodeComparison", left, parent, op);
}

void PPExplainVisitor::visit(PPIf* op)
{
    insertOperationElement("PPIf", left, parent, op);
}

void PPExplainVisitor::visit(PPLet* op)
{
    insertOperationElement("PPLet", left, parent, op);
}

void PPExplainVisitor::visit(PPOrderBy* op)
{
    insertOperationElement("PPOrderBy", left, parent, op);
}

void PPExplainVisitor::visit(PPSTuple* op)
{
    insertOperationElement("PPSTuple", left, parent, op);
}

void PPExplainVisitor::visit(PPSLet* op)
{
    insertOperationElement("PPSLet", left, parent, op);
}

void PPExplainVisitor::visit(PPReturn* op)
{
    insertOperationElement("PPReturn", left, parent, op);
}

void PPExplainVisitor::visit(PPFnName* op)
{
    insertOperationElement("PPFnName", left, parent, op);
}

void PPExplainVisitor::visit(PPFnLocalName* op)
{
    insertOperationElement("PPFnLocalName", left, parent, op);
}

void PPExplainVisitor::visit(PPFnNamespaceUri* op)
{
    insertOperationElement("PPFnNamespaceUri", left, parent, op);
}

void PPExplainVisitor::visit(PPFnNumber* op)
{
    insertOperationElement("PPFnNumber", left, parent, op);
}

void PPExplainVisitor::visit(PPFnRoot* op)
{
    insertOperationElement("PPFnRoot", left, parent, op);
}

void PPExplainVisitor::visit(PPNumericFuncs* op)
{
    insertOperationElement("PPNumericFuncs", left, parent, op);
}

void PPExplainVisitor::visit(PPFnRoundHalfToEven* op)
{
    insertOperationElement("PPFnRoundHalfToEven", left, parent, op);
}

void PPExplainVisitor::visit(PPPatMatch* op)
{
    insertOperationElement("PPPatMatch", left, parent, op);
}

void PPExplainVisitor::visit(PPFnResolveQName* op)
{
    insertOperationElement("PPFnResolveQName", left, parent, op);
}

void PPExplainVisitor::visit(PPFnQName* op)
{
    insertOperationElement("PPFnQName", left, parent, op);
}

void PPExplainVisitor::visit(PPFnPrefixFromQName* op)
{
    insertOperationElement("PPFnPrefixFromQName", left, parent, op);
}

void PPExplainVisitor::visit(PPFnLocalNameFromQName* op)
{
    insertOperationElement("PPFnLocalNameFromQName", left, parent, op);
}

void PPExplainVisitor::visit(PPFnNamespaceUriFromQName* op)
{
    insertOperationElement("PPFnNamespaceUriFromQName", left, parent, op);
}

void PPExplainVisitor::visit(PPFnNamespaceUriForPrefix* op)
{
    insertOperationElement("PPFnNamespaceUriForPrefix", left, parent, op);
}

void PPExplainVisitor::visit(PPFnInScopePrefixes* op)
{
    insertOperationElement("PPFnInScopePrefixes", left, parent, op);
}

void PPExplainVisitor::visit(PPCast* op)
{
    insertOperationElement("PPCast", left, parent, op);
}

void PPExplainVisitor::visit(PPCastable* op)
{
    insertOperationElement("PPCastable", left, parent, op);
}

void PPExplainVisitor::visit(PPTreat* op)
{
    insertOperationElement("PPTreat", left, parent, op);
}

void PPExplainVisitor::visit(PPTypeswitch* op)
{
    insertOperationElement("PPTypeswitch", left, parent, op);
}

void PPExplainVisitor::visit(PPInstanceOf* op)
{
    insertOperationElement("PPInstanceOf", left, parent, op);
}



#ifdef SQL_CONNECTION
void PPExplainVisitor::visit(PPFnSQLConnect* op)
{
    insertOperationElement("PPFnSQLConnect", left, parent, op);
}
void PPExplainVisitor::visit(PPFnSQLExecute* op)
{
    insertOperationElement("PPFnSQLExecute", left, parent, op);
}
void PPExplainVisitor::visit(PPFnSQLPrepare* op)
{
    insertOperationElement("PPFnSQLPrepare", left, parent, op);
}
void PPExplainVisitor::visit(PPFnSQLClose* op)
{
    insertOperationElement("PPFnSQLClose", left, parent, op);
}
void PPExplainVisitor::visit(PPFnSQLCommit* op)
{
    insertOperationElement("PPFnSQLCommit", left, parent, op);
}
void PPExplainVisitor::visit(PPFnSQLRollback* op)
{
    insertOperationElement("PPFnSQLRollback", left, parent, op);
}
#endif /* SQL_CONNECTION */



void PPExplainVisitor::visit(PPFnConcat* op)
{
    insertOperationElement("PPFnConcat", left, parent, op);
}

void PPExplainVisitor::visit(PPFnStringJoin* op)
{
    insertOperationElement("PPFnStringJoin", left, parent, op);
}

void PPExplainVisitor::visit(PPFnStartsEndsWith* op)
{
    insertOperationElement("PPFnStartsEndsWith", left, parent, op);
}

void PPExplainVisitor::visit(PPFnStringLength* op)
{
    insertOperationElement("PPFnStringLength", left, parent, op);
}

void PPExplainVisitor::visit(PPFnNormalizeSpace* op)
{
    insertOperationElement("PPFnNormalizeSpace", left, parent, op);
}

void PPExplainVisitor::visit(PPFnNormalizeUnicode* op)
{
    insertOperationElement("PPFnNormalizeUnicode", left, parent, op);
}

void PPExplainVisitor::visit(PPFnString2CodePoints* op)
{
    insertOperationElement("PPFnString2CodePoints", left, parent, op);
}

void PPExplainVisitor::visit(PPFnCodePoints2String* op)
{
    insertOperationElement("PPFnCodePoints2String", left, parent, op);
}

void PPExplainVisitor::visit(PPFnTranslate* op)
{
    insertOperationElement("PPFnTranslate", left, parent, op);
}

void PPExplainVisitor::visit(PPFnChangeCase* op)
{
    insertOperationElement("PPFnChangeCase", left, parent, op);
}

void PPExplainVisitor::visit(PPFnSubsBeforeAfter* op)
{
    insertOperationElement("PPFnSubsBeforeAfter", left, parent, op);
}

void PPExplainVisitor::visit(PPFnSubstring* op)
{
    insertOperationElement("PPFnSubstring", left, parent, op);
}

void PPExplainVisitor::visit(PPFnCompare* op)
{
    insertOperationElement("PPFnCompare", left, parent, op);
}

void PPExplainVisitor::visit(PPSubsMatch* op)
{
    insertOperationElement("PPSubsMatch", left, parent, op);
}

void PPExplainVisitor::visit(PPFnUriEncoding* op)
{
    insertOperationElement("PPFnUriEncoding", left, parent, op);
}

void PPExplainVisitor::visit(PPFnResolveUri* op)
{
    insertOperationElement("PPFnResolveUri", left, parent, op);
}

void PPExplainVisitor::visit(PPQueryRoot* op)
{
    insertOperationElement("PPQueryRoot", left, parent);
}

void PPExplainVisitor::visit(PPBulkLoad* op)
{
    insertOperationElement("PPBulkLoad", left, parent);
}

void PPExplainVisitor::visit(PPCreateIndex* op)
{
    insertOperationElement("PPCreateIndex", left, parent);
}

void PPExplainVisitor::visit(PPCreateDocument* op)
{
    insertOperationElement("PPCreateDocument", left, parent);
}

void PPExplainVisitor::visit(PPCreateCollection* op)
{
    insertOperationElement("PPCreateCollection", left, parent);
}

void PPExplainVisitor::visit(PPCreateDocumentInCollection* op)
{
    insertOperationElement("PPCreateDocumentInCollection", left, parent);
}

#ifdef SE_ENABLE_TRIGGERS
void PPExplainVisitor::visit(PPCreateTrigger* op)
{
    insertOperationElement("PPCreateTrigger", left, parent);
}
void PPExplainVisitor::visit(PPDropTrigger* op)
{
    insertOperationElement("PPDropTrigger", left, parent);
}
#endif


void PPExplainVisitor::visit(PPDeleteDeep* op)
{
    insertOperationElement("PPDeleteDeep", left, parent);
}

void PPExplainVisitor::visit(PPDeleteUndeep* op)
{
    insertOperationElement("PPDeleteUndeep", left, parent);
}


#ifdef SE_ENABLE_FTSEARCH
void PPExplainVisitor::visit(PPCreateFtIndex* op)
{
    insertOperationElement("PPCreateFtIndex", left, parent);
}
void PPExplainVisitor::visit(PPDropFtIndex* op)
{
    insertOperationElement("PPDropFtIndex", left, parent);
}
#endif


void PPExplainVisitor::visit(PPDropIndex* op)
{
    insertOperationElement("PPDropIndex", left, parent);
}

void PPExplainVisitor::visit(PPDropDocument* op)
{
    insertOperationElement("PPDropDocument", left, parent);
}

void PPExplainVisitor::visit(PPDropCollection* op)
{
    insertOperationElement("PPDropCollection", left, parent);
}

void PPExplainVisitor::visit(PPDropDocumentInCollection* op)
{
    insertOperationElement("PPDropDocumentInCollection", left, parent);
}

void PPExplainVisitor::visit(PPLoadModule* op)
{
    insertOperationElement("PPLoadModule", left, parent);
}

void PPExplainVisitor::visit(PPDropModule* op)
{
    insertOperationElement("PPDropModule", left, parent);
}

void PPExplainVisitor::visit(PPInsertTo* op)
{
    insertOperationElement("PPInsertTo", left, parent);
}

void PPExplainVisitor::visit(PPInsertBefore* op)
{
    insertOperationElement("PPInsertBefore", left, parent);
}

void PPExplainVisitor::visit(PPInsertFollowing* op)
{
    insertOperationElement("PPInsertFollowing", left, parent);
}

void PPExplainVisitor::visit(PPRename* op)
{
    insertOperationElement("PPRename", left, parent);
}

void PPExplainVisitor::visit(PPReplace* op)
{
    insertOperationElement("PPReplace", left, parent);
}

void PPExplainVisitor::visit(PPRetrieveDS* op)
{
    insertOperationElement("PPRetrieveDS", left, parent);
}

void PPExplainVisitor::visit(PPRetrieveMetadata* op)
{
    insertOperationElement("PPRetrieveMetadata", left, parent);
}

void PPExplainVisitor::visit(PPCreateUser* op)
{
    insertOperationElement("PPCreateUser", left, parent);
}

void PPExplainVisitor::visit(PPDropUser* op)
{
    insertOperationElement("PPDropUser", left, parent);
}

void PPExplainVisitor::visit(PPAlterUser* op)
{
    insertOperationElement("PPAlterUser", left, parent);
}

void PPExplainVisitor::visit(PPCreateRole* op)
{
    insertOperationElement("PPCreateRole", left, parent);
}

void PPExplainVisitor::visit(PPDropRole* op)
{
    insertOperationElement("PPDropRole", left, parent);
}

void PPExplainVisitor::visit(PPGrantRole* op)
{
    insertOperationElement("PPGrantRole", left, parent);
}

void PPExplainVisitor::visit(PPGrantRevokePriv* op)
{
    insertOperationElement("PPGrantRevokePriv", left, parent);
    insertAttributeHelper("object-type", XNULL, left, string(op->get_object_type()));
}

void PPExplainVisitor::visit(PPRevokeRole* op)
{
    insertOperationElement("PPRevokeRole", left, parent);
}
