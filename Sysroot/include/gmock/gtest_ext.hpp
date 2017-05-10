#ifndef __gtest_ext_hpp__
#define __gtest_ext_hpp__

#include <gtest/gtest.h>

/* TPS Code Start
File-Path: utils/common-unittest/gtest_ext.hpp
Allowed-Usage: INTERNAL
ID: TPS-2541
Comment: Reviewed by LCA in June 2013 and per license is acceptable for Microsoft internal use
*/
#define GTEST_TEST_BOOLEAN_DYN_(expression, text, actual, expected, fail, doAsert) \
  GTEST_AMBIGUOUS_ELSE_BLOCKER_ \
  if (const ::testing::AssertionResult gtest_ar_ = \
      ::testing::AssertionResult(expression)) \
    ; \
  else \
  if( !doAsert ) return; \
  else \
    fail(::testing::internal::GetBoolAssertionFailureMessage(\
        gtest_ar_, text, #actual, #expected).c_str() )


#define ASSERT_FALSE_DYN(condition, doAssert) \
  GTEST_TEST_BOOLEAN_DYN_(!(condition), #condition, true, false, \
                      GTEST_FATAL_FAILURE_, doAssert)

#define ASSERT_TRUE_DYN(condition, doAssert) \
  GTEST_TEST_BOOLEAN_DYN_(condition, #condition, false, true, \
                      GTEST_FATAL_FAILURE_, doAssert)

#define GTEST_ASSERT_DYN(expression, on_failure, doAssert) \
  GTEST_AMBIGUOUS_ELSE_BLOCKER_ \
  if (const ::testing::AssertionResult gtest_ar = (expression)) \
    ; \
  else \
  if( !doAssert ) return; \
  else \
    on_failure(gtest_ar.failure_message())

#define GTEST_PRED_FORMAT2_DYN_(pred_format, v1, v2, on_failure, doAssert)\
  GTEST_ASSERT_DYN(pred_format(#v1, #v2, v1, v2),\
                on_failure, doAssert)

#define GTEST_PRED2_DYN_(pred, v1, v2, on_failure, doAssert)\
  GTEST_ASSERT_DYN_(::testing::AssertPred2Helper(#pred, \
                                             #v1, \
                                             #v2, \
                                             pred, \
                                             v1, \
                                             v2), on_failure, doAssert)
											 
/* TPS Code End
*/

#define ASSERT_PRED_FORMAT2_DYN(pred_format, v1, v2, doAssert) \
  GTEST_PRED_FORMAT2_DYN_(pred_format, v1, v2, GTEST_FATAL_FAILURE_, doAssert)
#define ASSERT_PRED2_DYN(pred, v1, v2, doAssert) \
  GTEST_PRED2_DYN_(pred, v1, v2, GTEST_FATAL_FAILURE_, doAssert)


#define ASSERT_EQ_DYN(expected, actual, doAssert) \
  ASSERT_PRED_FORMAT2_DYN(::testing::internal::EqHelper<GTEST_IS_NULL_LITERAL_(expected)>::Compare, \
                      expected, actual, doAssert)

#define ASSERT_NE_DYN(val1, val2, doAssert) \
  ASSERT_PRED_FORMAT2_DYN(::testing::internal::CmpHelperNE, val1, val2, doAssert)
#define ASSERT_LE_DYN(val1, val2, doAssert) \
  ASSERT_PRED_FORMAT2_DYN(::testing::internal::CmpHelperLE, val1, val2, doAssert)
#define ASSERT_LT_DYN(val1, val2, doAssert) \
  ASSERT_PRED_FORMAT2_DYN(::testing::internal::CmpHelperLT, val1, val2, doAssert)
#define ASSERT_GE_DYN(val1, val2, doAssert) \
  ASSERT_PRED_FORMAT2_DYN(::testing::internal::CmpHelperGE, val1, val2, doAssert)
#define ASSERT_GT_DYN(val1, val2, doAssert) \
  ASSERT_PRED_FORMAT2_DYN(::testing::internal::CmpHelperGT, val1, val2, doAssert)

#endif
