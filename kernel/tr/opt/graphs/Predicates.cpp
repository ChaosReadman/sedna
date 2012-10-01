#include "tr/opt/graphs/DataGraphs.h"
#include "tr/opt/graphs/Predicates.h"
#include "tr/opt/phm/PhysicalModel.h"

#include "tr/models/XmlConstructor.h"

#include <algorithm>

using namespace opt;

BinaryPredicate::BinaryPredicate(DataNode* left, DataNode* right)
{
    dataNodeList.push_back(left);
    dataNodeList.push_back(right);
}

StructuralPredicate::StructuralPredicate(DataNode* left, DataNode* right, const pe::Path& _path)
    : BinaryPredicate(left, right), outer(false), path(_path)
{
    
}

ValuePredicate::ValuePredicate(DataNode* left, DataNode* right, const Comparison & _cmp)
    : BinaryPredicate(left, right), cmp(_cmp)
{

}

FnDocPredicate::FnDocPredicate(DataNode* left, DataNode* right)
    : BinaryPredicate(left, right)
{

}


void * StructuralPredicate::compile(PhysicalModel* model)
{
    return model->compile(this);
}

void * ValuePredicate::compile(PhysicalModel* model)
{
    return model->compile(this);
}

void* FnDocPredicate::compile(PhysicalModel* model)
{
    return NULL;
//    return model->c
}


/* Serialization */

/*
std::string DataGraph::toLRString() const
{
    std::stringstream stream;
    stream << "(datagraph ";

    stream << "(";

    FOR_ALL_GRAPH_ELEMENTS(dataNodes, i) {
        stream << dataNodes[i]->toLRString();
    };

    stream << ")";

    stream << "(";

    FOR_ALL_GRAPH_ELEMENTS(predicates, i) {
        stream << predicates[i]->toLRString();
    };
    stream << ")";

    stream << ")";
    return stream.str();
}

std::string StructuralPredicate::toLRString() const
{
    std::stringstream stream;
    stream << "(sj " << left()->getName() << " " << right()->getName() << " " << path.toLRString() << ")";
    return stream.str();
}

std::string FPredicate::toLRString() const
{
  return "";
}

std::string ValuePredicate::toLRString() const
{
    std::stringstream stream;
    stream << "(vj " << left()->getName() << " " << right()->getName() << " " << cmp.toLRString() << ")";
    return stream.str();
}
*/

XmlConstructor & StructuralPredicate::toXML(XmlConstructor & element) const
{
    element.openElement(SE_EL_NAME("StructurePredicate"));
  
    element.addAttributeValue(SE_EL_NAME("id"), tuple_cell::atomic_int(index));
    element.addElementValue(SE_EL_NAME("path"), path.toXPathString());
    element.addElementValue(SE_EL_NAME("left"), tuple_cell::atomic_int(left()->index));
    element.addElementValue(SE_EL_NAME("right"), tuple_cell::atomic_int(right()->index));
    
    element.closeElement();

    return element;
}

XmlConstructor & ValuePredicate::toXML(XmlConstructor & element) const
{
    element.openElement(SE_EL_NAME("ValuePredicate"));

    element.addAttributeValue(SE_EL_NAME("id"), tuple_cell::atomic_int(index));
    element.addElementValue(SE_EL_NAME("cmp"), cmp.toLRString());
    element.addElementValue(SE_EL_NAME("left"), tuple_cell::atomic_int(left()->index));
    element.addElementValue(SE_EL_NAME("right"), tuple_cell::atomic_int(right()->index));

    element.closeElement();

    return element;
}

XmlConstructor& FnDocPredicate::toXML(XmlConstructor& element) const
{
    element.openElement(SE_EL_NAME("FnDocPredicate"));

    element.addAttributeValue(SE_EL_NAME("id"), tuple_cell::atomic_int(index));
    element.addElementValue(SE_EL_NAME("left"), tuple_cell::atomic_int(left()->index));
    element.addElementValue(SE_EL_NAME("right"), tuple_cell::atomic_int(right()->index));

    element.closeElement();

    return element;
}
