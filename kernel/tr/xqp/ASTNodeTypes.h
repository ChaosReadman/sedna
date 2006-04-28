/*
 * File:  ASTNodeTypes.h
 * Copyright (C) 2004 The Institute for System Programming of the Russian Academy of Sciences (ISP RAS)
 */

#ifndef ASTNodeTypes_h
#define ASTNodeTypes_h

#include "sedna.h"      

enum ASTNodeType { 	 AST_QUERY=1,
			 AST_FUNCDEFS,
			 AST_SEQUENCE,
			 AST_OR,
			 AST_AND,
			 AST_FLWR,
			 AST_FORLETS,
			 AST_FOR,
			 AST_LET,
			 AST_WHERE,
			 AST_RETURN,
			 AST_RPE,
			 AST_VAR,
			 AST_TEST,
			 AST_CHILD,
			 AST_CONTEXT,
			 AST_ATTRIBUTE,
			 AST_QNAME,
			 AST_WILDCARD,
			 AST_TEXT,
			 AST_NODE,
			 AST_FCALL,
			 AST_CONST,
			 AST_SOME,
			 AST_EVERY,
			 AST_IF,
			 AST_B_OP,
			 AST_UNARY_OP,
			 AST_PREDICATES,
			 AST_ELEMENT,
			 AST_ELEMENT_NAME,
			 AST_ELEMENT_ATTRIBUTES,
			 AST_CONTENT,
			 AST_ATTRIBUTE_NAME,
			 AST_FUNCTION,
			 AST_FPARAMS,
			 AST_FBODY,
			 AST_UNKNOWN_TYPE,
			 AST_FUNC,
			 AST_EXPR,
			 AST_SLASH,
			 AST_SLASH_SLASH,
			 AST_AXIS_PATH_STEP,
			 AST_FORWARD_STEP,
			 AST_AXIS,                             
			 AST_CHILD_AXIS,
			 AST_ATTRIBUTE_AXIS,
			 AST_DESCENDANT_AXIS,
			 AST_PARENT_AXIS,
			 AST_SELF_AXIS,
			 AST_DESCENDANT_OR_SELF_AXIS,
			 AST_FOLLOWING_SIBLING_AXIS,
			 AST_FOLLOWING_AXIS,
			 AST_ANCESTOR_AXIS,
			 AST_PRECEDING_SIBLING_AXIS,
			 AST_PRECEDING_AXIS,
			 AST_ANCESTOR_OR_SELF_AXIS,
			 AST_REVERSE_STEP,                         
			 AST_FILTER_PATH_STEP,
			 AST_BOUND,
			 AST_CHAR_SEQ,
			 AST_PREFIX,
			 AST_LOCAL_NAME,
			 AST_CONTEXT_ITEM,
			 AST_RELATIVE_PATH, 
			 AST_STEP,
			 AST_TYPE,
			 AST_EMPTY,
			 AST_MULTIPLICITY,
			 AST_ITEM_TEST,
			 AST_ATOMIC,
			 AST_COMMENT_TEST,
			 AST_TEXT_TEST,
			 AST_NODE_TEST,
			 AST_DOCUMENT_TEST,
			 AST_ELEMENT_TEST,
			 AST_NIL,
			 AST_ATTRIBUTE_TEST,
			 AST_TYPE_NAME,
			 AST_SCHEMA_CONTEXT_PATH,
			 AST_GLOBAL_NAME,
			 AST_CONTEXT_NAME,
			 AST_GLOBAL_TYPE,
			 AST_INTEGER_CONST,
			 AST_DECIMAL_CONST,
			 AST_DOUBLE_CONST,
			 AST_STRING_CONST,
			 AST_INSTANCE_OF,           
			 AST_TREAT,
			 AST_CASTABLE,
			 AST_CAST,
			 AST_UPDATE,
			 AST_INTO,
			 AST_PRECEDING,
			 AST_FOLLOWING,
			 AST_INSERT,
			 AST_DELETE,
			 AST_DELETE_UNDEEP,
			 AST_REPLACE,
			 AST_RENAME,
			 AST_MOVE,
			 AST_STABLE_ORDER_BY,
			 AST_ORDER_BY,
			 AST_ORDER_SPEC,
			 AST_ORDER,
			 AST_ASC_DESC,
			 AST_EMPT_GR_LST,
			 AST_COLLATION,
			 AST_ORDER_PROPERTY,                             
			 AST_RETURNED_TYPE,
			 AST_BODY_FUNC,
			 AST_PARAM,
			 AST_CREATE_DOCUMENT,
			 AST_DROP_DOCUMENT,
			 AST_CREATE_COLLECTION,
			 AST_DROP_COLLECTION,
			 AST_EMPTY_SEQUENCE,
			 AST_LOAD_FILE,
			 AST_LOAD_FILE_EX,
			 AST_RETRIEVE_METADATA_DOCS,
			 AST_RETRIEVE_METADATA_COLLS,
			 AST_DESCR_SCHEMA,
			 AST_DESCR_SCHEMA_FOR_COL,
			 AST_EMPTY_ELEMENT_CONTENT,
			 AST_EMPTY_ATTRIBUTE_CONTENT,
			 AST_DECL_NSP,
			 AST_DECL_DEF_ELEM_NSP,
			 AST_DECL_DEF_FUNC_NSP,          
			 AST_NAMESPACE,
			 AST_NAMESPACE_NAME,
			 AST_PROLOG,
			 AST_SCRIPT,
			 AST_CREATE_ROLE,
			 AST_DROP_ROLE,
			 AST_GRANT_ROLE,
			 AST_REVOKE_PRIV,
			 AST_REVOKE_PRIV_FR_DOC,
			 AST_REVOKE_PRIV_FR_COL,
			 AST_REVOKE_ROLE,
			 AST_COMMIT,
			 AST_ROLLBACK,
			 AST_CREATE_USER,
			 AST_DROP_USER,
			 AST_ALTER_USER,
			 AST_VAR_DECL,
			 AST_STATISTICS,
			 AST_CREATE_INDEX,
			 AST_DROP_INDEX,
			 AST_GRANT_PRIV,
			 AST_GRANT_PRIV_ON_DOC,
			 AST_GRANT_PRIV_ON_COL,
			 AST_PRIV_LIST,
			 AST_PRIV_ALL,
			 AST_USER_LIST,
			 AST_USER_PUBLIC,
			 AST_CREATE,
			 AST_METADATA,
			 AST_BSPACE_P,
			 AST_BSPACE_S,
			 AST_DECLARE_OPT,
			 AST_CREATE_FULLTEXT_INDEX,
			 AST_DROP_FULLTEXT_INDEX
 };

#endif


