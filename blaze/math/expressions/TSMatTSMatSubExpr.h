//=================================================================================================
/*!
//  \file blaze/math/expressions/TSMatTSMatSubExpr.h
//  \brief Header file for the transpose sparse matrix/transpose sparse matrix subtraction expression
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

#ifndef _BLAZE_MATH_EXPRESSIONS_TSMATTSMATSUBEXPR_H_
#define _BLAZE_MATH_EXPRESSIONS_TSMATTSMATSUBEXPR_H_


//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <stdexcept>
#include <blaze/math/constraints/MatMatSubExpr.h>
#include <blaze/math/constraints/SparseMatrix.h>
#include <blaze/math/constraints/StorageOrder.h>
#include <blaze/math/constraints/Symmetric.h>
#include <blaze/math/expressions/Computation.h>
#include <blaze/math/expressions/Forward.h>
#include <blaze/math/expressions/MatMatSubExpr.h>
#include <blaze/math/expressions/SparseMatrix.h>
#include <blaze/math/shims/IsDefault.h>
#include <blaze/math/shims/Serial.h>
#include <blaze/math/traits/ColumnExprTrait.h>
#include <blaze/math/traits/RowExprTrait.h>
#include <blaze/math/traits/SubExprTrait.h>
#include <blaze/math/traits/SubmatrixExprTrait.h>
#include <blaze/math/traits/SubTrait.h>
#include <blaze/math/typetraits/Columns.h>
#include <blaze/math/typetraits/IsComputation.h>
#include <blaze/math/typetraits/IsExpression.h>
#include <blaze/math/typetraits/IsLower.h>
#include <blaze/math/typetraits/IsResizable.h>
#include <blaze/math/typetraits/IsStrictlyLower.h>
#include <blaze/math/typetraits/IsStrictlyUpper.h>
#include <blaze/math/typetraits/IsSymmetric.h>
#include <blaze/math/typetraits/IsTemporary.h>
#include <blaze/math/typetraits/IsUniLower.h>
#include <blaze/math/typetraits/IsUniUpper.h>
#include <blaze/math/typetraits/IsUpper.h>
#include <blaze/math/typetraits/RequiresEvaluation.h>
#include <blaze/math/typetraits/Rows.h>
#include <blaze/util/Assert.h>
#include <blaze/util/DisableIf.h>
#include <blaze/util/EnableIf.h>
#include <blaze/util/logging/FunctionTrace.h>
#include <blaze/util/mpl/And.h>
#include <blaze/util/mpl/Max.h>
#include <blaze/util/SelectType.h>
#include <blaze/util/Types.h>
#include <blaze/util/typetraits/RemoveReference.h>
#include <blaze/util/valuetraits/IsTrue.h>


namespace blaze {

//=================================================================================================
//
//  CLASS TSMATTSMATSUBEXPR
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Expression object for transpose sparse matrix-transpose sparse matrix subtractions.
// \ingroup sparse_matrix_expression
//
// The TSMatTSMatSubExpr class represents the compile time expression for subtractions between
// two column-major sparse matrices.
*/
template< typename MT1    // Type of the left-hand side sparse matrix
        , typename MT2 >  // Type of the right-hand side sparse matrix
class TSMatTSMatSubExpr : public SparseMatrix< TSMatTSMatSubExpr<MT1,MT2>, true >
                        , private MatMatSubExpr
                        , private Computation
{
 private:
   //**Type definitions****************************************************************************
   typedef typename MT1::ResultType     RT1;  //!< Result type of the left-hand side sparse matrix expression.
   typedef typename MT2::ResultType     RT2;  //!< Result type of the right-hand side sparse matrix expression.
   typedef typename MT1::ReturnType     RN1;  //!< Return type of the left-hand side sparse matrix expression.
   typedef typename MT2::ReturnType     RN2;  //!< Return type of the right-hand side sparse matrix expression.
   typedef typename MT1::CompositeType  CT1;  //!< Composite type of the left-hand side sparse matrix expression.
   typedef typename MT2::CompositeType  CT2;  //!< Composite type of the right-hand side sparse matrix expression.
   //**********************************************************************************************

   //**Return type evaluation**********************************************************************
   //! Compilation switch for the selection of the subscript operator return type.
   /*! The \a returnExpr compile time constant expression is a compilation switch for the
       selection of the \a ReturnType. If either matrix operand returns a temporary vector
       or matrix, \a returnExpr will be set to \a false and the subscript operator will
       return it's result by value. Otherwise \a returnExpr will be set to \a true and
       the subscript operator may return it's result as an expression. */
   enum { returnExpr = !IsTemporary<RN1>::value && !IsTemporary<RN2>::value };

   //! Expression return type for the subscript operator.
   typedef typename SubExprTrait<RN1,RN2>::Type  ExprReturnType;
   //**********************************************************************************************

   //**Serial evaluation strategy******************************************************************
   /*! \cond BLAZE_INTERNAL */
   //! Helper structure for the explicit application of the SFINAE principle.
   /*! The UseSymmetricKernel struct is a helper struct for the selection of the serial
       evaluation strategy. In case the target matrix is column-major and both matrix
       operands are symmetric, \a value is set to 1 and an optimized evaluation strategy
       is selected. Otherwise \a value is set to 0 and the default strategy is chosen. */
   template< typename T1, typename T2, typename T3 >
   struct UseSymmetricKernel {
      BLAZE_CONSTRAINT_MUST_BE_ROW_MAJOR_MATRIX_TYPE( T1 );
      enum { value = IsSymmetric<T2>::value && IsSymmetric<T3>::value };
   };
   /*! \endcond */
   //**********************************************************************************************

   //**Parallel evaluation strategy****************************************************************
   /*! \cond BLAZE_INTERNAL */
   //! Helper structure for the explicit application of the SFINAE principle.
   /*! The UseSMPAssign struct is a helper struct for the selection of the parallel evaluation
       strategy. In case the target matrix is SMP assignable, \a value is set to 1 and the
       expression specific evaluation strategy is selected. Otherwise \a value is set to 0
       and the default strategy is chosen. */
   template< typename MT >
   struct UseSMPAssign {
      enum { value = MT::smpAssignable };
   };
   /*! \endcond */
   //**********************************************************************************************

 public:
   //**Type definitions****************************************************************************
   typedef TSMatTSMatSubExpr<MT1,MT2>          This;           //!< Type of this TSMatTSMatSubExpr instance.
   typedef typename SubTrait<RT1,RT2>::Type    ResultType;     //!< Result type for expression template evaluations.
   typedef typename ResultType::OppositeType   OppositeType;   //!< Result type with opposite storage order for expression template evaluations.
   typedef typename ResultType::TransposeType  TransposeType;  //!< Transpose type for expression template evaluations.
   typedef typename ResultType::ElementType    ElementType;    //!< Resulting element type.

   //! Return type for expression template evaluations.
   typedef const typename SelectType< returnExpr, ExprReturnType, ElementType >::Type  ReturnType;

   //! Data type for composite expression templates.
   typedef const ResultType  CompositeType;

   //! Composite type of the left-hand side sparse matrix expression.
   typedef typename SelectType< IsExpression<MT1>::value, const MT1, const MT1& >::Type  LeftOperand;

   //! Composite type of the right-hand side sparse matrix expression.
   typedef typename SelectType< IsExpression<MT2>::value, const MT2, const MT2& >::Type  RightOperand;
   //**********************************************************************************************

   //**Compilation flags***************************************************************************
   //! Compilation switch for the expression template assignment strategy.
   enum { smpAssignable = 0 };
   //**********************************************************************************************

   //**Constructor*********************************************************************************
   /*!\brief Constructor for the TSMatTSMatSubExpr class.
   //
   // \param lhs The left-hand side sparse matrix operand of the subtraction expression.
   // \param rhs The right-hand side sparse matrix operand of the subtraction expression.
   */
   explicit inline TSMatTSMatSubExpr( const MT1& lhs, const MT2& rhs )
      : lhs_( lhs )  // Left-hand side sparse matrix of the subtraction expression
      , rhs_( rhs )  // Right-hand side sparse matrix of the subtraction expression
   {
      BLAZE_INTERNAL_ASSERT( lhs.rows()    == rhs.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( lhs.columns() == rhs.columns(), "Invalid number of columns" );
   }
   //**********************************************************************************************

   //**Access operator*****************************************************************************
   /*!\brief 2D-access to the matrix elements.
   //
   // \param i Access index for the row. The index has to be in the range \f$[0..M-1]\f$.
   // \param j Access index for the column. The index has to be in the range \f$[0..N-1]\f$.
   // \return The resulting value.
   */
   inline ReturnType operator()( size_t i, size_t j ) const {
      BLAZE_INTERNAL_ASSERT( i < lhs_.rows()   , "Invalid row access index"    );
      BLAZE_INTERNAL_ASSERT( j < lhs_.columns(), "Invalid column access index" );
      return lhs_(i,j) - rhs_(i,j);
   }
   //**********************************************************************************************

   //**Rows function*******************************************************************************
   /*!\brief Returns the current number of rows of the matrix.
   //
   // \return The number of rows of the matrix.
   */
   inline size_t rows() const {
      return lhs_.rows();
   }
   //**********************************************************************************************

   //**Columns function****************************************************************************
   /*!\brief Returns the current number of columns of the matrix.
   //
   // \return The number of columns of the matrix.
   */
   inline size_t columns() const {
      return lhs_.columns();
   }
   //**********************************************************************************************

   //**NonZeros function***************************************************************************
   /*!\brief Returns the number of non-zero elements in the sparse matrix.
   //
   // \return The number of non-zero elements in the sparse matrix.
   */
   inline size_t nonZeros() const {
      return lhs_.nonZeros() + rhs_.nonZeros();
   }
   //**********************************************************************************************

   //**NonZeros function***************************************************************************
   /*!\brief Returns the number of non-zero elements in the specified row.
   //
   // \param i The index of the row.
   // \return The number of non-zero elements of row \a i.
   */
   inline size_t nonZeros( size_t i ) const {
      return lhs_.nonZeros(i) + rhs_.nonZeros(i);
   }
   //**********************************************************************************************

   //**Left operand access*************************************************************************
   /*!\brief Returns the left-hand side transpose sparse matrix operand.
   //
   // \return The left-hand side transpose sparse matrix operand.
   */
   inline LeftOperand leftOperand() const {
      return lhs_;
   }
   //**********************************************************************************************

   //**Right operand access************************************************************************
   /*!\brief Returns the right-hand side transpose sparse matrix operand.
   //
   // \return The right-hand side transpose sparse matrix operand.
   */
   inline RightOperand rightOperand() const {
      return rhs_;
   }
   //**********************************************************************************************

   //**********************************************************************************************
   /*!\brief Returns whether the expression can alias with the given address \a alias.
   //
   // \param alias The alias to be checked.
   // \return \a true in case the expression can alias, \a false otherwise.
   */
   template< typename T >
   inline bool canAlias( const T* alias ) const {
      return ( lhs_.canAlias( alias ) || rhs_.canAlias( alias ) );
   }
   //**********************************************************************************************

   //**********************************************************************************************
   /*!\brief Returns whether the expression is aliased with the given address \a alias.
   //
   // \param alias The alias to be checked.
   // \return \a true in case an alias effect is detected, \a false otherwise.
   */
   template< typename T >
   inline bool isAliased( const T* alias ) const {
      return ( lhs_.isAliased( alias ) || rhs_.isAliased( alias ) );
   }
   //**********************************************************************************************

 private:
   //**Member variables****************************************************************************
   LeftOperand  lhs_;  //!< Left-hand side sparse matrix of the subtraction expression.
   RightOperand rhs_;  //!< Right-hand side sparse matrix of the subtraction expression.
   //**********************************************************************************************

   //**Assignment to dense matrices****************************************************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Assignment of a transpose sparse matrix-transpose sparse matrix subtraction to a
   //        dense matrix.
   // \ingroup sparse_matrix
   //
   // \param lhs The target left-hand side dense matrix.
   // \param rhs The right-hand side subtraction expression to be assigned.
   // \return void
   //
   // This function implements the performance optimized assignment of a transpsoe sparse matrix-
   // transpose sparse matrix subtraction expression to a dense matrix.
   */
   template< typename MT  // Type of the target dense matrix
           , bool SO >    // Storage order of the target dense matrix
   friend inline void assign( DenseMatrix<MT,SO>& lhs, const TSMatTSMatSubExpr& rhs )
   {
      BLAZE_FUNCTION_TRACE;

      BLAZE_INTERNAL_ASSERT( (~lhs).rows()    == rhs.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( (~lhs).columns() == rhs.columns(), "Invalid number of columns" );

      typedef typename RemoveReference<CT2>::Type::ConstIterator  RightIterator;

      assign( ~lhs, rhs.lhs_ );

      if( !IsResizable<typename MT::ElementType>::value ) {
         subAssign( ~lhs, rhs.rhs_ );
      }
      else
      {
         CT2 B( serial( rhs.rhs_ ) );  // Evaluation of the right-hand side sparse matrix operand

         BLAZE_INTERNAL_ASSERT( B.rows()    == rhs.rhs_.rows()   , "Invalid number of rows"    );
         BLAZE_INTERNAL_ASSERT( B.columns() == rhs.rhs_.columns(), "Invalid number of columns" );
         BLAZE_INTERNAL_ASSERT( B.rows()    == (~lhs).rows()     , "Invalid number of rows"    );
         BLAZE_INTERNAL_ASSERT( B.columns() == (~lhs).columns()  , "Invalid number of columns" );

         for( size_t j=0UL; j<(~lhs).columns(); ++j ) {
            const RightIterator end( B.end(j) );
            for( RightIterator element=B.begin(j); element!=end; ++element ) {
               if( isDefault( (~lhs)(element->index(),j) ) )
                  (~lhs)(element->index(),j) = -element->value();
               else
                  (~lhs)(element->index(),j) -= element->value();
            }
         }
      }
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Assignment to row-major sparse matrices*****************************************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Assignment of a transpose sparse matrix-transpose sparse matrix subtraction to a
   //        row-major sparse matrix.
   // \ingroup sparse_matrix
   //
   // \param lhs The target left-hand side sparse matrix.
   // \param rhs The right-hand side subtraction expression to be assigned.
   // \return void
   //
   // This function implements the default assignment of a transpose sparse matrix-transpose
   // sparse matrix subtraction expression to a row-major sparse matrix.
   */
   template< typename MT >  // Type of the target sparse matrix
   friend inline typename DisableIf< UseSymmetricKernel<MT,MT1,MT2> >::Type
      assign( SparseMatrix<MT,false>& lhs, const TSMatTSMatSubExpr& rhs )
   {
      BLAZE_FUNCTION_TRACE;

      BLAZE_CONSTRAINT_MUST_NOT_BE_SYMMETRIC_MATRIX_TYPE( MT );

      BLAZE_INTERNAL_ASSERT( (~lhs).rows()    == rhs.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( (~lhs).columns() == rhs.columns(), "Invalid number of columns" );

      typedef typename RemoveReference<CT1>::Type::ConstIterator  LeftIterator;
      typedef typename RemoveReference<CT2>::Type::ConstIterator  RightIterator;

      CT1 A( serial( rhs.lhs_ ) );  // Evaluation of the left-hand side sparse matrix operand
      CT2 B( serial( rhs.rhs_ ) );  // Evaluation of the right-hand side sparse matrix operand

      BLAZE_INTERNAL_ASSERT( A.rows()    == rhs.lhs_.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( A.columns() == rhs.lhs_.columns(), "Invalid number of columns" );
      BLAZE_INTERNAL_ASSERT( B.rows()    == rhs.rhs_.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( B.columns() == rhs.rhs_.columns(), "Invalid number of columns" );
      BLAZE_INTERNAL_ASSERT( A.rows()    == (~lhs).rows()     , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( A.columns() == (~lhs).columns()  , "Invalid number of columns" );

      const size_t m( rhs.rows()    );
      const size_t n( rhs.columns() );

      // Counting the number of elements per column
      std::vector<size_t> nonzeros( m, 0UL );
      for( size_t j=0UL; j<n; ++j )
      {
         const LeftIterator  lend( A.end(j) );
         const RightIterator rend( B.end(j) );

         LeftIterator  l( A.begin(j) );
         RightIterator r( B.begin(j) );

         while( l != lend && r != rend )
         {
            if( l->index() < r->index() ) {
               ++nonzeros[l->index()];
               ++l;
            }
            else if( l->index() > r->index() ) {
               ++nonzeros[r->index()];
               ++r;
            }
            else {
               ++nonzeros[l->index()];
               ++l;
               ++r;
            }
         }

         while( l != lend ) {
            ++nonzeros[l->index()];
            ++l;
         }

         while( r != rend ) {
            ++nonzeros[r->index()];
            ++r;
         }
      }

      // Resizing the left-hand side sparse matrix
      for( size_t i=0UL; i<m; ++i ) {
         (~lhs).reserve( i, nonzeros[i] );
      }

      // Performing the matrix subtraction
      for( size_t j=0UL; j<n; ++j )
      {
         const LeftIterator  lend( A.end(j) );
         const RightIterator rend( B.end(j) );

         LeftIterator  l( A.begin(j) );
         RightIterator r( B.begin(j) );

         while( l != lend && r != rend )
         {
            if( l->index() < r->index() ) {
               (~lhs).append( l->index(), j, l->value() );
               ++l;
            }
            else if( l->index() > r->index() ) {
               (~lhs).append( r->index(), j, -r->value() );
               ++r;
            }
            else {
               (~lhs).append( l->index(), j, l->value()-r->value() );
               ++l;
               ++r;
            }
         }

         while( l != lend ) {
            (~lhs).append( l->index(), j, l->value() );
            ++l;
         }

         while( r != rend ) {
            (~lhs).append( r->index(), j, -r->value() );
            ++r;
         }
      }
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Assignment to row-major sparse matrices*****************************************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Assignment of a transpose sparse matrix-transpose sparse matrix subtraction to a
   //        row-major sparse matrix.
   // \ingroup sparse_matrix
   //
   // \param lhs The target left-hand side sparse matrix.
   // \param rhs The right-hand side subtraction expression to be assigned.
   // \return void
   //
   // This function implements the performance optimized assignment of a transpose sparse matrix-
   // transpose sparse matrix subtraction expression to a row-major sparse matrix.
   */
   template< typename MT >  // Type of the target sparse matrix
   friend inline typename EnableIf< UseSymmetricKernel<MT,MT1,MT2> >::Type
      assign( SparseMatrix<MT,false>& lhs, const TSMatTSMatSubExpr& rhs )
   {
      BLAZE_FUNCTION_TRACE;

      BLAZE_CONSTRAINT_MUST_NOT_BE_SYMMETRIC_MATRIX_TYPE( MT );

      BLAZE_INTERNAL_ASSERT( (~lhs).rows()    == rhs.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( (~lhs).columns() == rhs.columns(), "Invalid number of columns" );

      assign( ~lhs, trans( rhs.lhs_ ) - trans( rhs.rhs_ ) );
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Assignment to column-major sparse matrices**************************************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Assignment of a transpose sparse matrix-transpose sparse matrix subtraction to a
   //        column-major sparse matrix.
   // \ingroup sparse_matrix
   //
   // \param lhs The target left-hand side sparse matrix.
   // \param rhs The right-hand side subtraction expression to be assigned.
   // \return void
   //
   // This function implements the performance optimized assignment of a transpose sparse matrix-
   // transpose sparse matrix subtraction expression to a column-major sparse matrix.
   */
   template< typename MT >  // Type of the target sparse matrix
   friend inline void assign( SparseMatrix<MT,true>& lhs, const TSMatTSMatSubExpr& rhs )
   {
      BLAZE_FUNCTION_TRACE;

      BLAZE_INTERNAL_ASSERT( (~lhs).rows()    == rhs.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( (~lhs).columns() == rhs.columns(), "Invalid number of columns" );

      typedef typename RemoveReference<CT1>::Type::ConstIterator  LeftIterator;
      typedef typename RemoveReference<CT2>::Type::ConstIterator  RightIterator;

      CT1 A( serial( rhs.lhs_ ) );  // Evaluation of the left-hand side sparse matrix operand
      CT2 B( serial( rhs.rhs_ ) );  // Evaluation of the right-hand side sparse matrix operand

      BLAZE_INTERNAL_ASSERT( A.rows()    == rhs.lhs_.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( A.columns() == rhs.lhs_.columns(), "Invalid number of columns" );
      BLAZE_INTERNAL_ASSERT( B.rows()    == rhs.rhs_.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( B.columns() == rhs.rhs_.columns(), "Invalid number of columns" );
      BLAZE_INTERNAL_ASSERT( A.rows()    == (~lhs).rows()     , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( A.columns() == (~lhs).columns()  , "Invalid number of columns" );

      // Final memory allocation (based on the evaluated operands)
      (~lhs).reserve( A.nonZeros() + B.nonZeros() );

      // Performing the matrix subtraction
      for( size_t j=0UL; j<(~lhs).columns(); ++j )
      {
         const LeftIterator  lend( A.end(j) );
         const RightIterator rend( B.end(j) );

         LeftIterator  l( A.begin(j) );
         RightIterator r( B.begin(j) );

         while( l != lend && r != rend )
         {
            if( l->index() < r->index() ) {
               (~lhs).append( l->index(), j, l->value() );
               ++l;
            }
            else if( l->index() > r->index() ) {
               (~lhs).append( r->index(), j, -r->value() );
               ++r;
            }
            else {
               (~lhs).append( l->index(), j, l->value()-r->value() );
               ++l;
               ++r;
            }
         }

         while( l != lend ) {
            (~lhs).append( l->index(), j, l->value() );
            ++l;
         }

         while( r != rend ) {
            (~lhs).append( r->index(), j, -r->value() );
            ++r;
         }

         (~lhs).finalize( j );
      }
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Addition assignment to dense matrices*******************************************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Addition assignment of a transpose sparse matrix-transpose sparse matrix subtraction
   //        to a dense matrix.
   // \ingroup sparse_matrix
   //
   // \param lhs The target left-hand side dense matrix.
   // \param rhs The right-hand side subtraction expression to be added.
   // \return void
   //
   // This function implements the performance optimized addition assignment of a transpose
   // sparse matrix-transpose sparse matrix subtraction expression to a dense matrix.
   */
   template< typename MT  // Type of the target dense matrix
           , bool SO >    // Storage order of the target dense matrix
   friend inline void addAssign( DenseMatrix<MT,SO>& lhs, const TSMatTSMatSubExpr& rhs )
   {
      BLAZE_FUNCTION_TRACE;

      BLAZE_INTERNAL_ASSERT( (~lhs).rows()    == rhs.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( (~lhs).columns() == rhs.columns(), "Invalid number of columns" );

      addAssign( ~lhs, rhs.lhs_ );
      subAssign( ~lhs, rhs.rhs_ );
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Addition assignment to sparse matrices******************************************************
   // No special implementation for the addition assignment to sparse matrices.
   //**********************************************************************************************

   //**Subtraction assignment to dense matrices****************************************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief Subtraction assignment of a transpose sparse matrix-transpose sparse matrix
   //        subtraction to a dense matrix.
   // \ingroup sparse_matrix
   //
   // \param lhs The target left-hand side dense matrix.
   // \param rhs The right-hand side subtraction expression to be subtracted.
   // \return void
   //
   // This function implements the performance optimized subtraction assignment of a transpose
   // sparse matrix-transpose sparse matrix subtraction expression to a dense matrix.
   */
   template< typename MT  // Type of the target dense matrix
           , bool SO >    // Storage order of the target dense matrix
   friend inline void subAssign( DenseMatrix<MT,SO>& lhs, const TSMatTSMatSubExpr& rhs )
   {
      BLAZE_FUNCTION_TRACE;

      BLAZE_INTERNAL_ASSERT( (~lhs).rows()    == rhs.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( (~lhs).columns() == rhs.columns(), "Invalid number of columns" );

      subAssign( ~lhs, rhs.lhs_ );
      addAssign( ~lhs, rhs.rhs_ );
   }
   /*! \endcond */
   //**********************************************************************************************

   //**Subtraction assignment to sparse matrices***************************************************
   // No special implementation for the subtraction assignment to sparse matrices.
   //**********************************************************************************************

   //**Multiplication assignment to dense matrices*************************************************
   // No special implementation for the multiplication assignment to dense matrices.
   //**********************************************************************************************

   //**Multiplication assignment to sparse matrices************************************************
   // No special implementation for the multiplication assignment to sparse matrices.
   //**********************************************************************************************

   //**SMP assignment to dense matrices************************************************************
   // No special implementation for the SMP assignment to dense matrices.
   //**********************************************************************************************

   //**SMP assignment to sparse matrices***********************************************************
   // No special implementation for the SMP assignment to sparse matrices.
   //**********************************************************************************************

   //**SMP addition assignment to dense matrices***************************************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief SMP addition assignment of a transpose sparse matrix-transpose sparse matrix
   //        subtraction to a dense matrix.
   // \ingroup sparse_matrix
   //
   // \param lhs The target left-hand side dense matrix.
   // \param rhs The right-hand side subtraction expression to be added.
   // \return void
   //
   // This function implements the performance optimized SMP addition assignment of a transpose
   // sparse matrix-transpose sparse matrix subtraction expression to a dense matrix. Due to the
   // explicit application of the SFINAE principle, this function can only be selected by the
   // compiler in case the expression specific parallel evaluation strategy is selected.
   */
   template< typename MT  // Type of the target dense matrix
           , bool SO >    // Storage order of the target dense matrix
   friend inline typename EnableIf< UseSMPAssign<MT> >::Type
      smpAddAssign( DenseMatrix<MT,SO>& lhs, const TSMatTSMatSubExpr& rhs )
   {
      BLAZE_FUNCTION_TRACE;

      BLAZE_INTERNAL_ASSERT( (~lhs).rows()    == rhs.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( (~lhs).columns() == rhs.columns(), "Invalid number of columns" );

      smpAddAssign( ~lhs, rhs.lhs_ );
      smpSubAssign( ~lhs, rhs.rhs_ );
   }
   /*! \endcond */
   //**********************************************************************************************

   //**SMP addition assignment to sparse matrices**************************************************
   // No special implementation for the SMP addition assignment to sparse matrices.
   //**********************************************************************************************

   //**SMP subtraction assignment to dense matrices************************************************
   /*! \cond BLAZE_INTERNAL */
   /*!\brief SMP subtraction assignment of a transpose sparse matrix-transpose sparse matrix
   //        subtraction to a dense matrix.
   // \ingroup sparse_matrix
   //
   // \param lhs The target left-hand side dense matrix.
   // \param rhs The right-hand side subtraction expression to be subtracted.
   // \return void
   //
   // This function implements the performance optimized SMP subtraction assignment of a transpose
   // sparse matrix-transpose sparse matrix subtraction expression to a dense matrix. Due to the
   // explicit application of the SFINAE principle, this function can only be selected by the
   // compiler in case the expression specific parallel evaluation strategy is selected.
   */
   template< typename MT  // Type of the target dense matrix
           , bool SO >    // Storage order of the target dense matrix
   friend inline typename EnableIf< UseSMPAssign<MT> >::Type
      smpSubAssign( DenseMatrix<MT,SO>& lhs, const TSMatTSMatSubExpr& rhs )
   {
      BLAZE_FUNCTION_TRACE;

      BLAZE_INTERNAL_ASSERT( (~lhs).rows()    == rhs.rows()   , "Invalid number of rows"    );
      BLAZE_INTERNAL_ASSERT( (~lhs).columns() == rhs.columns(), "Invalid number of columns" );

      smpSubAssign( ~lhs, rhs.lhs_ );
      smpAddAssign( ~lhs, rhs.rhs_ );
   }
   /*! \endcond */
   //**********************************************************************************************

   //**SMP subtraction assignment to sparse matrices***********************************************
   // No special implementation for the SMP subtraction assignment to sparse matrices.
   //**********************************************************************************************

   //**SMP multiplication assignment to dense matrices*********************************************
   // No special implementation for the SMP multiplication assignment to dense matrices.
   //**********************************************************************************************

   //**SMP multiplication assignment to sparse matrices********************************************
   // No special implementation for the SMP multiplication assignment to sparse matrices.
   //**********************************************************************************************

   //**Compile time checks*************************************************************************
   /*! \cond BLAZE_INTERNAL */
   BLAZE_CONSTRAINT_MUST_BE_SPARSE_MATRIX_TYPE( MT1 );
   BLAZE_CONSTRAINT_MUST_BE_COLUMN_MAJOR_MATRIX_TYPE( MT1 );
   BLAZE_CONSTRAINT_MUST_BE_SPARSE_MATRIX_TYPE( MT2 );
   BLAZE_CONSTRAINT_MUST_BE_COLUMN_MAJOR_MATRIX_TYPE( MT2 );
   BLAZE_CONSTRAINT_MUST_FORM_VALID_MATMATSUBEXPR( MT1, MT2 );
   /*! \endcond */
   //**********************************************************************************************
};
//*************************************************************************************************




//=================================================================================================
//
//  GLOBAL BINARY ARITHMETIC OPERATORS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Subtraction operator for the subtraction of two column-major sparse matrices
//        (\f$ A=B-C \f$).
// \ingroup sparse_matrix
//
// \param lhs The left-hand side sparse matrix for the matrix subtraction.
// \param rhs The right-hand side sparse matrix to be subtracted from the left-hand side matrix.
// \return The sum of the two matrices.
// \exception std::invalid_argument Matrix sizes do not match
//
// This operator represents the subtraction of two column-major sparse matrices:

   \code
   using blaze::columnMajor;

   blaze::CompressedMatrix<double,columnMajor> A, B, C;
   // ... Resizing and initialization
   C = A - B;
   \endcode

// The operator returns an expression representing a sparse matrix of the higher-order element
// type of the two involved matrix element types \a T1::ElementType and \a T2::ElementType.
// Both matrix types \a T1 and \a T2 as well as the two element types \a T1::ElementType and
// \a T2::ElementType have to be supported by the SubTrait class template.\n
// In case the current number of rows and columns of the two given matrices don't match, a
// \a std::invalid_argument is thrown.
*/
template< typename T1    // Type of the left-hand side sparse matrix
        , typename T2 >  // Type of the right-hand side sparse matrix
inline const TSMatTSMatSubExpr<T1,T2>
   operator-( const SparseMatrix<T1,true>& lhs, const SparseMatrix<T2,true>& rhs )
{
   BLAZE_FUNCTION_TRACE;

   if( (~lhs).rows() != (~rhs).rows() || (~lhs).columns() != (~rhs).columns() )
      throw std::invalid_argument( "Matrix sizes do not match" );

   return TSMatTSMatSubExpr<T1,T2>( ~lhs, ~rhs );
}
//*************************************************************************************************




//=================================================================================================
//
//  ROWS SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename MT1, typename MT2 >
struct Rows< TSMatTSMatSubExpr<MT1,MT2> >
   : public Max< Rows<MT1>, Rows<MT2> >::Type
{};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  COLUMNS SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename MT1, typename MT2 >
struct Columns< TSMatTSMatSubExpr<MT1,MT2> >
   : public Max< Columns<MT1>, Columns<MT2> >::Type
{};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  ISSYMMETRIC SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename MT1, typename MT2 >
struct IsSymmetric< TSMatTSMatSubExpr<MT1,MT2> >
   : public IsTrue< IsSymmetric<MT1>::value && IsSymmetric<MT2>::value >
{};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  ISLOWER SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename MT1, typename MT2 >
struct IsLower< TSMatTSMatSubExpr<MT1,MT2> >
   : public IsTrue< And< IsLower<MT1>, IsLower<MT2> >::value >
{};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  ISUNILOWER SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename MT1, typename MT2 >
struct IsUniLower< TSMatTSMatSubExpr<MT1,MT2> >
   : public IsTrue< And< IsUniLower<MT1>, IsStrictlyLower<MT2> >::value >
{};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  ISSTRICTLYLOWER SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename MT1, typename MT2 >
struct IsStrictlyLower< TSMatTSMatSubExpr<MT1,MT2> >
   : public IsTrue< And< IsStrictlyLower<MT1>, IsStrictlyLower<MT2> >::value >
{};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  ISUPPER SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename MT1, typename MT2 >
struct IsUpper< TSMatTSMatSubExpr<MT1,MT2> >
   : public IsTrue< And< IsUpper<MT1>, IsUpper<MT2> >::value >
{};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  ISUNIUPPER SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename MT1, typename MT2 >
struct IsUniUpper< TSMatTSMatSubExpr<MT1,MT2> >
   : public IsTrue< And< IsUniUpper<MT1>, IsStrictlyUpper<MT2> >::value >
{};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  ISSTRICTLYUPPER SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename MT1, typename MT2 >
struct IsStrictlyUpper< TSMatTSMatSubExpr<MT1,MT2> >
   : public IsTrue< And< IsStrictlyUpper<MT1>, IsStrictlyUpper<MT2> >::value >
{};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  EXPRESSION TRAIT SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename MT1, typename MT2, bool AF >
struct SubmatrixExprTrait< TSMatTSMatSubExpr<MT1,MT2>, AF >
{
 public:
   //**********************************************************************************************
   typedef typename SubExprTrait< typename SubmatrixExprTrait<const MT1,AF>::Type
                                , typename SubmatrixExprTrait<const MT2,AF>::Type >::Type  Type;
   //**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename MT1, typename MT2 >
struct RowExprTrait< TSMatTSMatSubExpr<MT1,MT2> >
{
 public:
   //**********************************************************************************************
   typedef typename SubExprTrait< typename RowExprTrait<const MT1>::Type
                                , typename RowExprTrait<const MT2>::Type >::Type  Type;
   //**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename MT1, typename MT2 >
struct ColumnExprTrait< TSMatTSMatSubExpr<MT1,MT2> >
{
 public:
   //**********************************************************************************************
   typedef typename SubExprTrait< typename ColumnExprTrait<const MT1>::Type
                                , typename ColumnExprTrait<const MT2>::Type >::Type  Type;
   //**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

} // namespace blaze

#endif
