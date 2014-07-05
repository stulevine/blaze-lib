//=================================================================================================
/*!
//  \file blazetest/utiltest/typetraits/OperationTest.h
//  \brief Header file for the type traits operation test
//
//  Copyright (C) 2013 Klaus Iglberger - All Rights Reserved
//
//  This file is part of the Blaze library. You can redistribute it and/or modify it under
//  the terms of the New (Revised) BSD License. Redistribution and use in source and binary
//  forms, with or without modification, are permitted provided that the following conditions
//  are met:
//
//  1. Redistributions of source code must retain the above copyright notice, this list of
//     conditions and the following disclaimer.
//  2. Redistributions in binary form must reproduce the above copyright notice, this list
//     of conditions and the following disclaimer in the documentation and/or other materials
//     provided with the distribution.
//  3. Neither the names of the Blaze development group nor the names of its contributors
//     may be used to endorse or promote products derived from this software without specific
//     prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
//  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
//  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
//  SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
//  TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
//  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
//  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
//  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
//  DAMAGE.
*/
//=================================================================================================

#ifndef _BLAZETEST_UTILTEST_TYPETRAITS_OPERATIONTEST_H_
#define _BLAZETEST_UTILTEST_TYPETRAITS_OPERATIONTEST_H_


//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <blaze/util/typetraits/GetMemberType.h>
#include <blaze/util/typetraits/HasMember.h>


namespace blazetest {

namespace utiltest {

namespace typetraits {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Auxiliary class for all type trait related tests.
//
// This class represents a test suite for the type traits. It performs a series of mainly compile
// time tests to guarantee the correctness of the type traits.
*/
class OperationTest
{
 public:
   //**Constructors********************************************************************************
   /*!\name Constructors */
   //@{
   explicit OperationTest();
   // No explicitly declared copy constructor.
   //@}
   //**********************************************************************************************

   //**Destructor**********************************************************************************
   // No explicitly declared destructor.
   //**********************************************************************************************

 private:
   //**Type trait setup****************************************************************************
   /*! \cond BLAZE_INTERNAL */
   BLAZE_CREATE_HAS_MEMBER_DATA_OR_FUNCTION_TYPE_TRAIT( HasValue, value_ );
   BLAZE_CREATE_HAS_MEMBER_DATA_OR_FUNCTION_TYPE_TRAIT( HasCompute, compute );
   BLAZE_CREATE_HAS_MEMBER_TYPE_TYPE_TRAIT( HasDataType, DataType );
   BLAZE_CREATE_GET_MEMBER_TYPE_TYPE_TRAIT( GetDataType, DataType, int );
   /*! \endcond */
   //**********************************************************************************************

   //**Test functions******************************************************************************
   /*!\name Test functions */
   //@{
   void testHasMember();
   void testGetMember();
   //@}
   //**********************************************************************************************
};
//*************************************************************************************************




//=================================================================================================
//
//  GLOBAL TEST FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Testing the type traits.
//
// \return void
*/
void runTest()
{
   OperationTest();
}
//*************************************************************************************************




//=================================================================================================
//
//  MACRO DEFINITIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Macro for the execution of the type traits operation test.
*/
#define RUN_TYPETRAITS_OPERATION_TEST \
   blazetest::utiltest::typetraits::runTest()
/*! \endcond */
//*************************************************************************************************

} // namespace typetraits

} // namespace utiltest

} // namespace blazetest

#endif