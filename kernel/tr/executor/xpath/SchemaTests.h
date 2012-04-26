#ifndef _SCHEMA_TEST_H_
#define _SCHEMA_TEST_H_

#include "structures/schema.h"

struct SchemaTestData {
    t_item m_type;
    const char * m_uri;
    const char * m_local;

    SchemaTestData(t_item type, const char * uri, const char * local) : m_type(type), m_uri(uri), m_local(local) {};
};

struct SchemaTestOperatorType {
    inline static bool test(schema_node_cptr node, const SchemaTestData * data) { return (node->type & data->m_type) > 0; };
    inline static bool testref(const sc_ref &ref, const SchemaTestData * data) { return (ref.type & data->m_type) > 0; };
};

struct SchemaTestOperatorQNameType {
    inline static bool test(schema_node_cptr node, const SchemaTestData * data) { return node->matches(data->m_uri, data->m_local, data->m_type); };
    inline static bool testref(const sc_ref &ref, const SchemaTestData * data) { return ref.matches(data->m_uri, data->m_local, data->m_type); };
};

struct SchemaTestOperatorLocalType {
    inline static bool test(schema_node_cptr node, const SchemaTestData * data) { return SchemaTestOperatorType::test(node, data) && strcmpex(node->get_name(), data->m_local) == 0; };
    inline static bool testref(const sc_ref &ref, const SchemaTestData * data) { return SchemaTestOperatorType::testref(ref, data) && strcmpex(ref.name, data->m_local) == 0; };
};

struct SchemaTestOperatorUriType {
    inline static bool test(schema_node_cptr node, const SchemaTestData * data) { return SchemaTestOperatorType::test(node, data) && same_xmlns_uri(node->get_xmlns(), data->m_uri); };
    inline static bool testref(const sc_ref &ref, const SchemaTestData * data) { return SchemaTestOperatorType::testref(ref, data) && same_xmlns_uri(ref.get_xmlns(), data->m_uri); };
};


#endif /* _SCHEMA_TEST_H_ */