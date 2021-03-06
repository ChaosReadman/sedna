/*
 * File:  PPDeleteUndeep.cpp
 * Copyright (C) 2004 The Institute for System Programming of the Russian Academy of Sciences (ISP RAS)
 */

#include "common/sedna.h"

#include "tr/executor/root/PPDeleteUndeep.h"
#include "tr/executor/base/visitor/PPVisitor.h"
#include "tr/updates/updates.h"
#include "tr/locks/locks.h"

PPDeleteUndeep::PPDeleteUndeep(PPOpIn _child_, 
                               dynamic_context *_cxt_) : PPUpdate("PPDeleteUndeep"),
                                                         child(_child_),
                                                         cxt(_cxt_)
{
}

PPDeleteUndeep::~PPDeleteUndeep()
{
    delete child.op;
    child.op = NULL;
    delete cxt;
    cxt = NULL;
}

void PPDeleteUndeep::do_open()
{
    local_lock_mrg->lock(lm_x);
    cxt->global_variables_open();
    child.op->open();
}

void PPDeleteUndeep::do_close()
{
    child.op->close();
    cxt->global_variables_close();
}

void PPDeleteUndeep::do_accept(PPVisitor &v)
{
    v.visit (this);
    v.push  (this);
    child.op->accept(v);    
    v.pop();
}

void PPDeleteUndeep::do_execute()
{
    delete_undeep(child);
}
