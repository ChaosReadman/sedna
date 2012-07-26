#ifndef _ELEMENTARY_OPERATIONS_H_
#define _ELEMENTARY_OPERATIONS_H_

#include "IndependentPlan.h"

namespace rqp {

struct EmptySequenceConst { explicit EmptySequenceConst(int x = 0) {}; };

class Const : public ConstantOperation {
    RTTI_DECL(plan_operation_Const, ConstantOperation)
protected:
    virtual XmlConstructor& __toXML ( XmlConstructor& constructor ) const;
private:
    opt::MemoryTupleSequencePtr sequence;
public:
    explicit Const(opt::MemoryTupleSequencePtr _sequence)
       : ConstantOperation(SELF_RTTI_REF), sequence(_sequence)
    {};

    explicit Const(tuple_cell _value)
      : ConstantOperation(SELF_RTTI_REF), sequence(new opt::MemoryTupleSequence())
    { sequence->push_back(_value); };

    explicit Const(const EmptySequenceConst &)
      : ConstantOperation(SELF_RTTI_REF), sequence(new opt::MemoryTupleSequence())
    { };

    PROPERTY_RO(Sequence, opt::MemoryTupleSequencePtr, sequence)
};

class VarIn : public ConstantOperation {
    RTTI_DECL(plan_operation_VarIn, ConstantOperation)
protected:
    virtual XmlConstructor& __toXML ( XmlConstructor& constructor ) const;
private:
    opt::TupleId tid;

    void setDataNode(opt::TupleId _tid);
public:
    /* Phantom datanode */
    opt::DataNode * dnode;

    explicit VarIn(opt::TupleId _tid)
      : ConstantOperation(SELF_RTTI_REF), tid(_tid)
    {
        setDataNode(_tid);
    };

    PROPERTY_RO(Tuple, opt::TupleId, tid)
};

class Exists : public ListOperation {
    RTTI_DECL(plan_operation_Exists, ListOperation)
protected:
    virtual XmlConstructor& __toXML ( XmlConstructor& constructor ) const;
public:
    explicit Exists(RPBase * list)
      : ListOperation(SELF_RTTI_REF, list) {};
};

class Sequence : public ManyChildren {
    RTTI_DECL(plan_operation_Sequence, ManyChildren)
protected:
    virtual XmlConstructor& __toXML ( XmlConstructor& constructor ) const;
public:
    enum space_t {
        none,
        atomic_spaces,
        all_spaces
    };

private:
    space_t spaces;
public:
    explicit Sequence(const OperationList & _oplist)
      : ManyChildren(SELF_RTTI_REF, _oplist), spaces(none) { };

    explicit Sequence(RPBase* _in)
      : ManyChildren(SELF_RTTI_REF, _in), spaces(none) { };

    PROPERTY(Spaces, space_t, spaces)
};

}

#endif /* _ELEMENTARY_OPERATIONS_H_ */
