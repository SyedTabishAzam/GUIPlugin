// -*- C++ -*-
// $Id$

/**
 * Code generated by the The ACE ORB (TAO) IDL Compiler v2.2a_p14
 * TAO and the TAO IDL Compiler have been developed by:
 *       Center for Distributed Object Computing
 *       Washington University
 *       St. Louis, MO
 *       USA
 *       http://www.cs.wustl.edu/~schmidt/doc-center.html
 * and
 *       Distributed Object Computing Laboratory
 *       University of California at Irvine
 *       Irvine, CA
 *       USA
 * and
 *       Institute for Software Integrated Systems
 *       Vanderbilt University
 *       Nashville, TN
 *       USA
 *       http://www.isis.vanderbilt.edu/
 *
 * Information about TAO is available at:
 *     http://www.cs.wustl.edu/~schmidt/TAO.html
 **/

// TAO_IDL - Generated from
// be\be_codegen.cpp:376


#include "DatatransferC.h"
#include "tao/CDR.h"

#if !defined (__ACE_INLINE__)
#include "DatatransferC.inl"
#endif /* !defined INLINE */

// TAO_IDL - Generated from
// be\be_visitor_structure\cdr_op_cs.cpp:52

TAO_BEGIN_VERSIONED_NAMESPACE_DECL

::CORBA::Boolean operator<< (
    TAO_OutputCDR &strm,
    const Datatransfer::Variables &_tao_aggregate)
{
  return
    (strm << _tao_aggregate.var_id) &&
    (strm << _tao_aggregate.entity.in ()) &&
    (strm << _tao_aggregate.count) &&
    (strm << _tao_aggregate.vars.in ()) &&
    (strm << _tao_aggregate.type.in ());
}

::CORBA::Boolean operator>> (
    TAO_InputCDR &strm,
    Datatransfer::Variables &_tao_aggregate)
{
  return
    (strm >> _tao_aggregate.var_id) &&
    (strm >> _tao_aggregate.entity.out ()) &&
    (strm >> _tao_aggregate.count) &&
    (strm >> _tao_aggregate.vars.out ()) &&
    (strm >> _tao_aggregate.type.out ());
}

TAO_END_VERSIONED_NAMESPACE_DECL



