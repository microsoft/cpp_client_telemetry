#ifndef GMOCK_HELPER_2_H
#define GMOCK_HELPER_2_H

#include "gmock/gmock.h"

#define DEFINE_CLASS( ThisClassName, InterfaceClassName ) \
	typedef ThisClassName ThisClass; \
	typedef InterfaceClassName InterfaceClass;

#define DEFINE_CLASS2( ThisClassName, InterfaceClassName, InterfaceClassName2 ) \
	typedef ThisClassName ThisClass; \
	typedef InterfaceClassName InterfaceClass; \
	typedef InterfaceClassName2 InterfaceClass2;

#define DEFINE_CLASS3( ThisClassName, InterfaceClassName, InterfaceClassName2, InterfaceClassName3 ) \
	typedef ThisClassName ThisClass; \
	typedef InterfaceClassName InterfaceClass; \
	typedef InterfaceClassName2 InterfaceClass2; \
	typedef InterfaceClassName3 InterfaceClass3;

//
//  macros common to declarations and definitions
//
#define SKY_MOCKER_(Method, NNN, constness) GTEST_CONCAT_TOKEN_(gmock_obj_##Method##_##NNN##_##constness, __LINE__)

//
//  mock declaration helper macros
//

#define SKY_DECL_MOCK_METHOD0_(tn, constness, ct, Method, ...) \
  GMOCK_RESULT_(tn, __VA_ARGS__) ct Method() constness; \
  ::testing::MockSpec<__VA_ARGS__>& \
      gmock_##Method() constness; \
  mutable ::testing::FunctionMocker<__VA_ARGS__> SKY_MOCKER_(Method, 0, constness)



#define SKY_DECL_MOCK_METHOD1_(tn, constness, ct, Method, ...) \
  GMOCK_RESULT_(tn, __VA_ARGS__) ct Method(GMOCK_ARG_(tn, 1, __VA_ARGS__) gmock_a1) constness; \
  ::testing::MockSpec<__VA_ARGS__>& \
      gmock_##Method(GMOCK_MATCHER_(tn, 1, __VA_ARGS__) gmock_a1) constness; \
  mutable ::testing::FunctionMocker<__VA_ARGS__> SKY_MOCKER_(Method, 1, constness)



#define SKY_DECL_MOCK_METHOD2_(tn, constness, ct, Method, ...) \
  GMOCK_RESULT_(tn, __VA_ARGS__) ct Method(GMOCK_ARG_(tn, 1, __VA_ARGS__) gmock_a1, \
                                 GMOCK_ARG_(tn, 2, __VA_ARGS__) gmock_a2) constness; \
  ::testing::MockSpec<__VA_ARGS__>& \
      gmock_##Method(GMOCK_MATCHER_(tn, 1, __VA_ARGS__) gmock_a1, \
                     GMOCK_MATCHER_(tn, 2, __VA_ARGS__) gmock_a2) constness; \
  mutable ::testing::FunctionMocker<__VA_ARGS__> SKY_MOCKER_(Method, 2, constness)



#define SKY_DECL_MOCK_METHOD3_(tn, constness, ct, Method, ...) \
  GMOCK_RESULT_(tn, __VA_ARGS__) ct Method(GMOCK_ARG_(tn, 1, __VA_ARGS__) gmock_a1, \
                                 GMOCK_ARG_(tn, 2, __VA_ARGS__) gmock_a2, \
                                 GMOCK_ARG_(tn, 3, __VA_ARGS__) gmock_a3) constness; \
  ::testing::MockSpec<__VA_ARGS__>& \
      gmock_##Method(GMOCK_MATCHER_(tn, 1, __VA_ARGS__) gmock_a1, \
                     GMOCK_MATCHER_(tn, 2, __VA_ARGS__) gmock_a2, \
                     GMOCK_MATCHER_(tn, 3, __VA_ARGS__) gmock_a3) constness; \
  mutable ::testing::FunctionMocker<__VA_ARGS__> SKY_MOCKER_(Method, 3, constness)



#define SKY_DECL_MOCK_METHOD4_(tn, constness, ct, Method, ...) \
  GMOCK_RESULT_(tn, __VA_ARGS__) ct Method(GMOCK_ARG_(tn, 1, __VA_ARGS__) gmock_a1, \
                                 GMOCK_ARG_(tn, 2, __VA_ARGS__) gmock_a2, \
                                 GMOCK_ARG_(tn, 3, __VA_ARGS__) gmock_a3, \
                                 GMOCK_ARG_(tn, 4, __VA_ARGS__) gmock_a4) constness; \
  ::testing::MockSpec<__VA_ARGS__>& \
      gmock_##Method(GMOCK_MATCHER_(tn, 1, __VA_ARGS__) gmock_a1, \
                     GMOCK_MATCHER_(tn, 2, __VA_ARGS__) gmock_a2, \
                     GMOCK_MATCHER_(tn, 3, __VA_ARGS__) gmock_a3, \
                     GMOCK_MATCHER_(tn, 4, __VA_ARGS__) gmock_a4) constness; \
  mutable ::testing::FunctionMocker<__VA_ARGS__> SKY_MOCKER_(Method, 4, constness)



#define SKY_DECL_MOCK_METHOD5_(tn, constness, ct, Method, ...) \
  GMOCK_RESULT_(tn, __VA_ARGS__) ct Method(GMOCK_ARG_(tn, 1, __VA_ARGS__) gmock_a1, \
                                 GMOCK_ARG_(tn, 2, __VA_ARGS__) gmock_a2, \
                                 GMOCK_ARG_(tn, 3, __VA_ARGS__) gmock_a3, \
                                 GMOCK_ARG_(tn, 4, __VA_ARGS__) gmock_a4, \
                                 GMOCK_ARG_(tn, 5, __VA_ARGS__) gmock_a5) constness; \
  ::testing::MockSpec<__VA_ARGS__>& \
      gmock_##Method(GMOCK_MATCHER_(tn, 1, __VA_ARGS__) gmock_a1, \
                     GMOCK_MATCHER_(tn, 2, __VA_ARGS__) gmock_a2, \
                     GMOCK_MATCHER_(tn, 3, __VA_ARGS__) gmock_a3, \
                     GMOCK_MATCHER_(tn, 4, __VA_ARGS__) gmock_a4, \
                     GMOCK_MATCHER_(tn, 5, __VA_ARGS__) gmock_a5) constness; \
  mutable ::testing::FunctionMocker<__VA_ARGS__> SKY_MOCKER_(Method, 5, constness)



#define SKY_DECL_MOCK_METHOD6_(tn, constness, ct, Method, ...) \
  GMOCK_RESULT_(tn, __VA_ARGS__) ct Method(GMOCK_ARG_(tn, 1, __VA_ARGS__) gmock_a1, \
                                 GMOCK_ARG_(tn, 2, __VA_ARGS__) gmock_a2, \
                                 GMOCK_ARG_(tn, 3, __VA_ARGS__) gmock_a3, \
                                 GMOCK_ARG_(tn, 4, __VA_ARGS__) gmock_a4, \
                                 GMOCK_ARG_(tn, 5, __VA_ARGS__) gmock_a5, \
                                 GMOCK_ARG_(tn, 6, __VA_ARGS__) gmock_a6) constness; \
  ::testing::MockSpec<__VA_ARGS__>& \
      gmock_##Method(GMOCK_MATCHER_(tn, 1, __VA_ARGS__) gmock_a1, \
                     GMOCK_MATCHER_(tn, 2, __VA_ARGS__) gmock_a2, \
                     GMOCK_MATCHER_(tn, 3, __VA_ARGS__) gmock_a3, \
                     GMOCK_MATCHER_(tn, 4, __VA_ARGS__) gmock_a4, \
                     GMOCK_MATCHER_(tn, 5, __VA_ARGS__) gmock_a5, \
                     GMOCK_MATCHER_(tn, 6, __VA_ARGS__) gmock_a6) constness; \
  mutable ::testing::FunctionMocker<__VA_ARGS__> SKY_MOCKER_(Method, 6, constness)



#define SKY_DECL_MOCK_METHOD7_(tn, constness, ct, Method, ...) \
  GMOCK_RESULT_(tn, __VA_ARGS__) ct Method(GMOCK_ARG_(tn, 1, __VA_ARGS__) gmock_a1, \
                                 GMOCK_ARG_(tn, 2, __VA_ARGS__) gmock_a2, \
                                 GMOCK_ARG_(tn, 3, __VA_ARGS__) gmock_a3, \
                                 GMOCK_ARG_(tn, 4, __VA_ARGS__) gmock_a4, \
                                 GMOCK_ARG_(tn, 5, __VA_ARGS__) gmock_a5, \
                                 GMOCK_ARG_(tn, 6, __VA_ARGS__) gmock_a6, \
                                 GMOCK_ARG_(tn, 7, __VA_ARGS__) gmock_a7) constness; \
  ::testing::MockSpec<__VA_ARGS__>& \
      gmock_##Method(GMOCK_MATCHER_(tn, 1, __VA_ARGS__) gmock_a1, \
                     GMOCK_MATCHER_(tn, 2, __VA_ARGS__) gmock_a2, \
                     GMOCK_MATCHER_(tn, 3, __VA_ARGS__) gmock_a3, \
                     GMOCK_MATCHER_(tn, 4, __VA_ARGS__) gmock_a4, \
                     GMOCK_MATCHER_(tn, 5, __VA_ARGS__) gmock_a5, \
                     GMOCK_MATCHER_(tn, 6, __VA_ARGS__) gmock_a6, \
                     GMOCK_MATCHER_(tn, 7, __VA_ARGS__) gmock_a7) constness; \
  mutable ::testing::FunctionMocker<__VA_ARGS__> SKY_MOCKER_(Method, 7, constness)



#define SKY_DECL_MOCK_METHOD8_(tn, constness, ct, Method, ...) \
  GMOCK_RESULT_(tn, __VA_ARGS__) ct Method(GMOCK_ARG_(tn, 1, __VA_ARGS__) gmock_a1, \
                                 GMOCK_ARG_(tn, 2, __VA_ARGS__) gmock_a2, \
                                 GMOCK_ARG_(tn, 3, __VA_ARGS__) gmock_a3, \
                                 GMOCK_ARG_(tn, 4, __VA_ARGS__) gmock_a4, \
                                 GMOCK_ARG_(tn, 5, __VA_ARGS__) gmock_a5, \
                                 GMOCK_ARG_(tn, 6, __VA_ARGS__) gmock_a6, \
                                 GMOCK_ARG_(tn, 7, __VA_ARGS__) gmock_a7, \
                                 GMOCK_ARG_(tn, 8, __VA_ARGS__) gmock_a8) constness; \
  ::testing::MockSpec<__VA_ARGS__>& \
      gmock_##Method(GMOCK_MATCHER_(tn, 1, __VA_ARGS__) gmock_a1, \
                     GMOCK_MATCHER_(tn, 2, __VA_ARGS__) gmock_a2, \
                     GMOCK_MATCHER_(tn, 3, __VA_ARGS__) gmock_a3, \
                     GMOCK_MATCHER_(tn, 4, __VA_ARGS__) gmock_a4, \
                     GMOCK_MATCHER_(tn, 5, __VA_ARGS__) gmock_a5, \
                     GMOCK_MATCHER_(tn, 6, __VA_ARGS__) gmock_a6, \
                     GMOCK_MATCHER_(tn, 7, __VA_ARGS__) gmock_a7, \
                     GMOCK_MATCHER_(tn, 8, __VA_ARGS__) gmock_a8) constness; \
  mutable ::testing::FunctionMocker<__VA_ARGS__> SKY_MOCKER_(Method, 8, constness)



#define SKY_DECL_MOCK_METHOD9_(tn, constness, ct, Method, ...) \
  GMOCK_RESULT_(tn, __VA_ARGS__) ct Method(GMOCK_ARG_(tn, 1, __VA_ARGS__) gmock_a1, \
                                 GMOCK_ARG_(tn, 2, __VA_ARGS__) gmock_a2, \
                                 GMOCK_ARG_(tn, 3, __VA_ARGS__) gmock_a3, \
                                 GMOCK_ARG_(tn, 4, __VA_ARGS__) gmock_a4, \
                                 GMOCK_ARG_(tn, 5, __VA_ARGS__) gmock_a5, \
                                 GMOCK_ARG_(tn, 6, __VA_ARGS__) gmock_a6, \
                                 GMOCK_ARG_(tn, 7, __VA_ARGS__) gmock_a7, \
                                 GMOCK_ARG_(tn, 8, __VA_ARGS__) gmock_a8, \
                                 GMOCK_ARG_(tn, 9, __VA_ARGS__) gmock_a9) constness; \
  ::testing::MockSpec<__VA_ARGS__>& \
      gmock_##Method(GMOCK_MATCHER_(tn, 1, __VA_ARGS__) gmock_a1, \
                     GMOCK_MATCHER_(tn, 2, __VA_ARGS__) gmock_a2, \
                     GMOCK_MATCHER_(tn, 3, __VA_ARGS__) gmock_a3, \
                     GMOCK_MATCHER_(tn, 4, __VA_ARGS__) gmock_a4, \
                     GMOCK_MATCHER_(tn, 5, __VA_ARGS__) gmock_a5, \
                     GMOCK_MATCHER_(tn, 6, __VA_ARGS__) gmock_a6, \
                     GMOCK_MATCHER_(tn, 7, __VA_ARGS__) gmock_a7, \
                     GMOCK_MATCHER_(tn, 8, __VA_ARGS__) gmock_a8, \
                     GMOCK_MATCHER_(tn, 9, __VA_ARGS__) gmock_a9) constness; \
  mutable ::testing::FunctionMocker<__VA_ARGS__> SKY_MOCKER_(Method, 9, constness)



#define SKY_DECL_MOCK_METHOD10_(tn, constness, ct, Method, ...) \
  GMOCK_RESULT_(tn, __VA_ARGS__) ct Method(GMOCK_ARG_(tn, 1, __VA_ARGS__) gmock_a1, \
                                 GMOCK_ARG_(tn, 2, __VA_ARGS__) gmock_a2, \
                                 GMOCK_ARG_(tn, 3, __VA_ARGS__) gmock_a3, \
                                 GMOCK_ARG_(tn, 4, __VA_ARGS__) gmock_a4, \
                                 GMOCK_ARG_(tn, 5, __VA_ARGS__) gmock_a5, \
                                 GMOCK_ARG_(tn, 6, __VA_ARGS__) gmock_a6, \
                                 GMOCK_ARG_(tn, 7, __VA_ARGS__) gmock_a7, \
                                 GMOCK_ARG_(tn, 8, __VA_ARGS__) gmock_a8, \
                                 GMOCK_ARG_(tn, 9, __VA_ARGS__) gmock_a9, \
                                 GMOCK_ARG_(tn, 10, __VA_ARGS__) gmock_a10) constness; \
  ::testing::MockSpec<__VA_ARGS__>& \
      gmock_##Method(GMOCK_MATCHER_(tn, 1, __VA_ARGS__) gmock_a1, \
                     GMOCK_MATCHER_(tn, 2, __VA_ARGS__) gmock_a2, \
                     GMOCK_MATCHER_(tn, 3, __VA_ARGS__) gmock_a3, \
                     GMOCK_MATCHER_(tn, 4, __VA_ARGS__) gmock_a4, \
                     GMOCK_MATCHER_(tn, 5, __VA_ARGS__) gmock_a5, \
                     GMOCK_MATCHER_(tn, 6, __VA_ARGS__) gmock_a6, \
                     GMOCK_MATCHER_(tn, 7, __VA_ARGS__) gmock_a7, \
                     GMOCK_MATCHER_(tn, 8, __VA_ARGS__) gmock_a8, \
                     GMOCK_MATCHER_(tn, 9, __VA_ARGS__) gmock_a9, \
                     GMOCK_MATCHER_(tn, 10, __VA_ARGS__) gmock_a10) constness; \
  mutable ::testing::FunctionMocker<__VA_ARGS__> SKY_MOCKER_(Method, 10, constness)




//
//  mock definition helper macros
//


#define SKY_DEF_MOCK_METHOD0_(tn, constness, ct, Method, ...) \
  GMOCK_RESULT_(tn, __VA_ARGS__) ct ClassName::Method() constness { \
    GTEST_COMPILE_ASSERT_((::std::tr1::tuple_size< \
        tn ::testing::internal::Function<__VA_ARGS__>::ArgumentTuple>::value == 0), \
        this_method_does_not_take_0_arguments); \
    SKY_MOCKER_(Method, 0, constness).SetOwnerAndName(this, #Method); \
    return SKY_MOCKER_(Method, 0, constness).Invoke(); \
  } \
  ::testing::MockSpec<__VA_ARGS__>& \
      ClassName::gmock_##Method() constness { \
    SKY_MOCKER_(Method, 0, constness).RegisterOwner(this);\
    return SKY_MOCKER_(Method, 0, constness).With(); \
  } \
  


#define SKY_DEF_MOCK_METHOD1_(tn, constness, ct, Method, ...) \
  GMOCK_RESULT_(tn, __VA_ARGS__) ct ClassName::Method(GMOCK_ARG_(tn, 1, __VA_ARGS__) gmock_a1) constness { \
    GTEST_COMPILE_ASSERT_((::std::tr1::tuple_size< \
        tn ::testing::internal::Function<__VA_ARGS__>::ArgumentTuple>::value == 1), \
        this_method_does_not_take_1_argument); \
    SKY_MOCKER_(Method, 1, constness).SetOwnerAndName(this, #Method); \
    return SKY_MOCKER_(Method, 1, constness).Invoke(gmock_a1); \
  } \
  ::testing::MockSpec<__VA_ARGS__>& \
      ClassName::gmock_##Method(GMOCK_MATCHER_(tn, 1, __VA_ARGS__) gmock_a1) constness { \
    SKY_MOCKER_(Method, 1, constness).RegisterOwner(this);\
    return SKY_MOCKER_(Method, 1, constness).With(gmock_a1); \
  } \
  


#define SKY_DEF_MOCK_METHOD2_(tn, constness, ct, Method, ...) \
  GMOCK_RESULT_(tn, __VA_ARGS__) ct ClassName::Method(GMOCK_ARG_(tn, 1, __VA_ARGS__) gmock_a1, \
                                 GMOCK_ARG_(tn, 2, __VA_ARGS__) gmock_a2) constness { \
    GTEST_COMPILE_ASSERT_((::std::tr1::tuple_size< \
        tn ::testing::internal::Function<__VA_ARGS__>::ArgumentTuple>::value == 2), \
        this_method_does_not_take_2_arguments); \
    SKY_MOCKER_(Method, 2, constness).SetOwnerAndName(this, #Method); \
    return SKY_MOCKER_(Method, 2, constness).Invoke(gmock_a1, gmock_a2); \
  } \
  ::testing::MockSpec<__VA_ARGS__>& \
      ClassName::gmock_##Method(GMOCK_MATCHER_(tn, 1, __VA_ARGS__) gmock_a1, \
                     GMOCK_MATCHER_(tn, 2, __VA_ARGS__) gmock_a2) constness { \
    SKY_MOCKER_(Method, 2, constness).RegisterOwner(this);\
    return SKY_MOCKER_(Method, 2, constness).With(gmock_a1, gmock_a2); \
  } \
  


#define SKY_DEF_MOCK_METHOD3_(tn, constness, ct, Method, ...) \
  GMOCK_RESULT_(tn, __VA_ARGS__) ct ClassName::Method(GMOCK_ARG_(tn, 1, __VA_ARGS__) gmock_a1, \
                                 GMOCK_ARG_(tn, 2, __VA_ARGS__) gmock_a2, \
                                 GMOCK_ARG_(tn, 3, __VA_ARGS__) gmock_a3) constness { \
    GTEST_COMPILE_ASSERT_((::std::tr1::tuple_size< \
        tn ::testing::internal::Function<__VA_ARGS__>::ArgumentTuple>::value == 3), \
        this_method_does_not_take_3_arguments); \
    SKY_MOCKER_(Method, 3, constness).SetOwnerAndName(this, #Method); \
    return SKY_MOCKER_(Method, 3, constness).Invoke(gmock_a1, gmock_a2, gmock_a3); \
  } \
  ::testing::MockSpec<__VA_ARGS__>& \
      ClassName::gmock_##Method(GMOCK_MATCHER_(tn, 1, __VA_ARGS__) gmock_a1, \
                     GMOCK_MATCHER_(tn, 2, __VA_ARGS__) gmock_a2, \
                     GMOCK_MATCHER_(tn, 3, __VA_ARGS__) gmock_a3) constness { \
    SKY_MOCKER_(Method, 3, constness).RegisterOwner(this); \
    return SKY_MOCKER_(Method, 3, constness).With(gmock_a1, gmock_a2, \
        gmock_a3); \
  } \
  


#define SKY_DEF_MOCK_METHOD4_(tn, constness, ct, Method, ...) \
  GMOCK_RESULT_(tn, __VA_ARGS__) ct ClassName::Method(GMOCK_ARG_(tn, 1, __VA_ARGS__) gmock_a1, \
                                 GMOCK_ARG_(tn, 2, __VA_ARGS__) gmock_a2, \
                                 GMOCK_ARG_(tn, 3, __VA_ARGS__) gmock_a3, \
                                 GMOCK_ARG_(tn, 4, __VA_ARGS__) gmock_a4) constness { \
    GTEST_COMPILE_ASSERT_((::std::tr1::tuple_size< \
        tn ::testing::internal::Function<__VA_ARGS__>::ArgumentTuple>::value == 4), \
        this_method_does_not_take_4_arguments); \
    SKY_MOCKER_(Method, 4, constness).SetOwnerAndName(this, #Method); \
    return SKY_MOCKER_(Method, 4, constness).Invoke(gmock_a1, gmock_a2, gmock_a3, \
        gmock_a4); \
  } \
  ::testing::MockSpec<__VA_ARGS__>& \
      ClassName::gmock_##Method(GMOCK_MATCHER_(tn, 1, __VA_ARGS__) gmock_a1, \
                     GMOCK_MATCHER_(tn, 2, __VA_ARGS__) gmock_a2, \
                     GMOCK_MATCHER_(tn, 3, __VA_ARGS__) gmock_a3, \
                     GMOCK_MATCHER_(tn, 4, __VA_ARGS__) gmock_a4) constness { \
    SKY_MOCKER_(Method, 4, constness).RegisterOwner(this); \
    return SKY_MOCKER_(Method, 4, constness).With(gmock_a1, gmock_a2, \
        gmock_a3, gmock_a4); \
  } \
  

#define SKY_DEF_MOCK_METHOD5_(tn, constness, ct, Method, ...) \
  GMOCK_RESULT_(tn, __VA_ARGS__) ct ClassName::Method(GMOCK_ARG_(tn, 1, __VA_ARGS__) gmock_a1, \
                                 GMOCK_ARG_(tn, 2, __VA_ARGS__) gmock_a2, \
                                 GMOCK_ARG_(tn, 3, __VA_ARGS__) gmock_a3, \
                                 GMOCK_ARG_(tn, 4, __VA_ARGS__) gmock_a4, \
                                 GMOCK_ARG_(tn, 5, __VA_ARGS__) gmock_a5) constness { \
    GTEST_COMPILE_ASSERT_((::std::tr1::tuple_size< \
        tn ::testing::internal::Function<__VA_ARGS__>::ArgumentTuple>::value == 5), \
        this_method_does_not_take_5_arguments); \
    SKY_MOCKER_(Method, 5, constness).SetOwnerAndName(this, #Method); \
    return SKY_MOCKER_(Method, 5, constness).Invoke(gmock_a1, gmock_a2, gmock_a3, \
        gmock_a4, gmock_a5); \
  } \
  ::testing::MockSpec<__VA_ARGS__>& \
      ClassName::gmock_##Method(GMOCK_MATCHER_(tn, 1, __VA_ARGS__) gmock_a1, \
                     GMOCK_MATCHER_(tn, 2, __VA_ARGS__) gmock_a2, \
                     GMOCK_MATCHER_(tn, 3, __VA_ARGS__) gmock_a3, \
                     GMOCK_MATCHER_(tn, 4, __VA_ARGS__) gmock_a4, \
                     GMOCK_MATCHER_(tn, 5, __VA_ARGS__) gmock_a5) constness { \
    SKY_MOCKER_(Method, 5, constness).RegisterOwner(this); \
    return SKY_MOCKER_(Method, 5, constness).With(gmock_a1, gmock_a2, \
        gmock_a3, gmock_a4, gmock_a5); \
  } \
  


#define SKY_DEF_MOCK_METHOD6_(tn, constness, ct, Method, ...) \
  GMOCK_RESULT_(tn, __VA_ARGS__) ct ClassName::Method(GMOCK_ARG_(tn, 1, __VA_ARGS__) gmock_a1, \
                                 GMOCK_ARG_(tn, 2, __VA_ARGS__) gmock_a2, \
                                 GMOCK_ARG_(tn, 3, __VA_ARGS__) gmock_a3, \
                                 GMOCK_ARG_(tn, 4, __VA_ARGS__) gmock_a4, \
                                 GMOCK_ARG_(tn, 5, __VA_ARGS__) gmock_a5, \
                                 GMOCK_ARG_(tn, 6, __VA_ARGS__) gmock_a6) constness { \
    GTEST_COMPILE_ASSERT_((::std::tr1::tuple_size< \
        tn ::testing::internal::Function<__VA_ARGS__>::ArgumentTuple>::value == 6), \
        this_method_does_not_take_6_arguments); \
    SKY_MOCKER_(Method, 6, constness).SetOwnerAndName(this, #Method); \
    return SKY_MOCKER_(Method, 6, constness).Invoke(gmock_a1, gmock_a2, gmock_a3, \
        gmock_a4, gmock_a5, gmock_a6); \
  } \
  ::testing::MockSpec<__VA_ARGS__>& \
      ClassName::gmock_##Method(GMOCK_MATCHER_(tn, 1, __VA_ARGS__) gmock_a1, \
                     GMOCK_MATCHER_(tn, 2, __VA_ARGS__) gmock_a2, \
                     GMOCK_MATCHER_(tn, 3, __VA_ARGS__) gmock_a3, \
                     GMOCK_MATCHER_(tn, 4, __VA_ARGS__) gmock_a4, \
                     GMOCK_MATCHER_(tn, 5, __VA_ARGS__) gmock_a5, \
                     GMOCK_MATCHER_(tn, 6, __VA_ARGS__) gmock_a6) constness { \
    SKY_MOCKER_(Method, 6, constness).RegisterOwner(this); \
    return SKY_MOCKER_(Method, 6, constness).With(gmock_a1, gmock_a2, \
        gmock_a3, gmock_a4, gmock_a5, gmock_a6); \
  } \
  


#define SKY_DEF_MOCK_METHOD7_(tn, constness, ct, Method, ...) \
  GMOCK_RESULT_(tn, __VA_ARGS__) ct ClassName::Method(GMOCK_ARG_(tn, 1, __VA_ARGS__) gmock_a1, \
                                 GMOCK_ARG_(tn, 2, __VA_ARGS__) gmock_a2, \
                                 GMOCK_ARG_(tn, 3, __VA_ARGS__) gmock_a3, \
                                 GMOCK_ARG_(tn, 4, __VA_ARGS__) gmock_a4, \
                                 GMOCK_ARG_(tn, 5, __VA_ARGS__) gmock_a5, \
                                 GMOCK_ARG_(tn, 6, __VA_ARGS__) gmock_a6, \
                                 GMOCK_ARG_(tn, 7, __VA_ARGS__) gmock_a7) constness { \
    GTEST_COMPILE_ASSERT_((::std::tr1::tuple_size< \
        tn ::testing::internal::Function<__VA_ARGS__>::ArgumentTuple>::value == 7), \
        this_method_does_not_take_7_arguments); \
    SKY_MOCKER_(Method, 7, constness).SetOwnerAndName(this, #Method); \
    return SKY_MOCKER_(Method, 7, constness).Invoke(gmock_a1, gmock_a2, gmock_a3, \
        gmock_a4, gmock_a5, gmock_a6, gmock_a7); \
  } \
  ::testing::MockSpec<__VA_ARGS__>& \
      ClassName::gmock_##Method(GMOCK_MATCHER_(tn, 1, __VA_ARGS__) gmock_a1, \
                     GMOCK_MATCHER_(tn, 2, __VA_ARGS__) gmock_a2, \
                     GMOCK_MATCHER_(tn, 3, __VA_ARGS__) gmock_a3, \
                     GMOCK_MATCHER_(tn, 4, __VA_ARGS__) gmock_a4, \
                     GMOCK_MATCHER_(tn, 5, __VA_ARGS__) gmock_a5, \
                     GMOCK_MATCHER_(tn, 6, __VA_ARGS__) gmock_a6, \
                     GMOCK_MATCHER_(tn, 7, __VA_ARGS__) gmock_a7) constness { \
    SKY_MOCKER_(Method, 7, constness).RegisterOwner(this); \
    return SKY_MOCKER_(Method, 7, constness).With(gmock_a1, gmock_a2, \
        gmock_a3, gmock_a4, gmock_a5, gmock_a6, gmock_a7); \
  } \
  


#define SKY_DEF_MOCK_METHOD8_(tn, constness, ct, Method, ...) \
  GMOCK_RESULT_(tn, __VA_ARGS__) ct ClassName::Method(GMOCK_ARG_(tn, 1, __VA_ARGS__) gmock_a1, \
                                 GMOCK_ARG_(tn, 2, __VA_ARGS__) gmock_a2, \
                                 GMOCK_ARG_(tn, 3, __VA_ARGS__) gmock_a3, \
                                 GMOCK_ARG_(tn, 4, __VA_ARGS__) gmock_a4, \
                                 GMOCK_ARG_(tn, 5, __VA_ARGS__) gmock_a5, \
                                 GMOCK_ARG_(tn, 6, __VA_ARGS__) gmock_a6, \
                                 GMOCK_ARG_(tn, 7, __VA_ARGS__) gmock_a7, \
                                 GMOCK_ARG_(tn, 8, __VA_ARGS__) gmock_a8) constness { \
    GTEST_COMPILE_ASSERT_((::std::tr1::tuple_size< \
        tn ::testing::internal::Function<__VA_ARGS__>::ArgumentTuple>::value == 8), \
        this_method_does_not_take_8_arguments); \
    SKY_MOCKER_(Method, 8, constness).SetOwnerAndName(this, #Method); \
    return SKY_MOCKER_(Method, 8, constness).Invoke(gmock_a1, gmock_a2, gmock_a3, \
        gmock_a4, gmock_a5, gmock_a6, gmock_a7, gmock_a8); \
  } \
  ::testing::MockSpec<__VA_ARGS__>& \
      ClassName::gmock_##Method(GMOCK_MATCHER_(tn, 1, __VA_ARGS__) gmock_a1, \
                     GMOCK_MATCHER_(tn, 2, __VA_ARGS__) gmock_a2, \
                     GMOCK_MATCHER_(tn, 3, __VA_ARGS__) gmock_a3, \
                     GMOCK_MATCHER_(tn, 4, __VA_ARGS__) gmock_a4, \
                     GMOCK_MATCHER_(tn, 5, __VA_ARGS__) gmock_a5, \
                     GMOCK_MATCHER_(tn, 6, __VA_ARGS__) gmock_a6, \
                     GMOCK_MATCHER_(tn, 7, __VA_ARGS__) gmock_a7, \
                     GMOCK_MATCHER_(tn, 8, __VA_ARGS__) gmock_a8) constness { \
    SKY_MOCKER_(Method, 8, constness).RegisterOwner(this); \
    return SKY_MOCKER_(Method, 8, constness).With(gmock_a1, gmock_a2, \
        gmock_a3, gmock_a4, gmock_a5, gmock_a6, gmock_a7, gmock_a8); \
  } \
  


#define SKY_DEF_MOCK_METHOD9_(tn, constness, ct, Method, ...) \
  GMOCK_RESULT_(tn, __VA_ARGS__) ct ClassName::Method(GMOCK_ARG_(tn, 1, __VA_ARGS__) gmock_a1, \
                                 GMOCK_ARG_(tn, 2, __VA_ARGS__) gmock_a2, \
                                 GMOCK_ARG_(tn, 3, __VA_ARGS__) gmock_a3, \
                                 GMOCK_ARG_(tn, 4, __VA_ARGS__) gmock_a4, \
                                 GMOCK_ARG_(tn, 5, __VA_ARGS__) gmock_a5, \
                                 GMOCK_ARG_(tn, 6, __VA_ARGS__) gmock_a6, \
                                 GMOCK_ARG_(tn, 7, __VA_ARGS__) gmock_a7, \
                                 GMOCK_ARG_(tn, 8, __VA_ARGS__) gmock_a8, \
                                 GMOCK_ARG_(tn, 9, __VA_ARGS__) gmock_a9) constness { \
    GTEST_COMPILE_ASSERT_((::std::tr1::tuple_size< \
        tn ::testing::internal::Function<__VA_ARGS__>::ArgumentTuple>::value == 9), \
        this_method_does_not_take_9_arguments); \
    SKY_MOCKER_(Method, 9, constness).SetOwnerAndName(this, #Method); \
    return SKY_MOCKER_(Method, 9, constness).Invoke(gmock_a1, gmock_a2, gmock_a3, \
        gmock_a4, gmock_a5, gmock_a6, gmock_a7, gmock_a8, gmock_a9); \
  } \
  ::testing::MockSpec<__VA_ARGS__>& \
      ClassName::gmock_##Method(GMOCK_MATCHER_(tn, 1, __VA_ARGS__) gmock_a1, \
                     GMOCK_MATCHER_(tn, 2, __VA_ARGS__) gmock_a2, \
                     GMOCK_MATCHER_(tn, 3, __VA_ARGS__) gmock_a3, \
                     GMOCK_MATCHER_(tn, 4, __VA_ARGS__) gmock_a4, \
                     GMOCK_MATCHER_(tn, 5, __VA_ARGS__) gmock_a5, \
                     GMOCK_MATCHER_(tn, 6, __VA_ARGS__) gmock_a6, \
                     GMOCK_MATCHER_(tn, 7, __VA_ARGS__) gmock_a7, \
                     GMOCK_MATCHER_(tn, 8, __VA_ARGS__) gmock_a8, \
                     GMOCK_MATCHER_(tn, 9, __VA_ARGS__) gmock_a9) constness { \
    SKY_MOCKER_(Method, 9, constness).RegisterOwner(this); \
    return SKY_MOCKER_(Method, 9, constness).With(gmock_a1, gmock_a2, \
        gmock_a3, gmock_a4, gmock_a5, gmock_a6, gmock_a7, gmock_a8, \
        gmock_a9); \
  } \
  


#define SKY_DEF_MOCK_METHOD10_(tn, constness, ct, Method, ...) \
  GMOCK_RESULT_(tn, __VA_ARGS__) ct ClassName::Method(GMOCK_ARG_(tn, 1, __VA_ARGS__) gmock_a1, \
                                 GMOCK_ARG_(tn, 2, __VA_ARGS__) gmock_a2, \
                                 GMOCK_ARG_(tn, 3, __VA_ARGS__) gmock_a3, \
                                 GMOCK_ARG_(tn, 4, __VA_ARGS__) gmock_a4, \
                                 GMOCK_ARG_(tn, 5, __VA_ARGS__) gmock_a5, \
                                 GMOCK_ARG_(tn, 6, __VA_ARGS__) gmock_a6, \
                                 GMOCK_ARG_(tn, 7, __VA_ARGS__) gmock_a7, \
                                 GMOCK_ARG_(tn, 8, __VA_ARGS__) gmock_a8, \
                                 GMOCK_ARG_(tn, 9, __VA_ARGS__) gmock_a9, \
                                 GMOCK_ARG_(tn, 10, __VA_ARGS__) gmock_a10) constness { \
    GTEST_COMPILE_ASSERT_((::std::tr1::tuple_size< \
        tn ::testing::internal::Function<__VA_ARGS__>::ArgumentTuple>::value == 10), \
        this_method_does_not_take_10_arguments); \
    SKY_MOCKER_(Method, 10, constness).SetOwnerAndName(this, #Method); \
    return SKY_MOCKER_(Method, 10, constness).Invoke(gmock_a1, gmock_a2, gmock_a3, \
        gmock_a4, gmock_a5, gmock_a6, gmock_a7, gmock_a8, gmock_a9, \
        gmock_a10); \
  } \
  ::testing::MockSpec<__VA_ARGS__>& \
      ClassName::gmock_##Method(GMOCK_MATCHER_(tn, 1, __VA_ARGS__) gmock_a1, \
                     GMOCK_MATCHER_(tn, 2, __VA_ARGS__) gmock_a2, \
                     GMOCK_MATCHER_(tn, 3, __VA_ARGS__) gmock_a3, \
                     GMOCK_MATCHER_(tn, 4, __VA_ARGS__) gmock_a4, \
                     GMOCK_MATCHER_(tn, 5, __VA_ARGS__) gmock_a5, \
                     GMOCK_MATCHER_(tn, 6, __VA_ARGS__) gmock_a6, \
                     GMOCK_MATCHER_(tn, 7, __VA_ARGS__) gmock_a7, \
                     GMOCK_MATCHER_(tn, 8, __VA_ARGS__) gmock_a8, \
                     GMOCK_MATCHER_(tn, 9, __VA_ARGS__) gmock_a9, \
                     GMOCK_MATCHER_(tn, 10, __VA_ARGS__) gmock_a10) constness { \
    SKY_MOCKER_(Method, 10, constness).RegisterOwner(this); \
    return SKY_MOCKER_(Method, 10, constness).With(gmock_a1, gmock_a2, \
        gmock_a3, gmock_a4, gmock_a5, gmock_a6, gmock_a7, gmock_a8, gmock_a9, \
        gmock_a10); \
  } \



// mock declaration macros
#define DECL_MOCK_METHOD0(m, ...) SKY_DECL_MOCK_METHOD0_(, , , m, __VA_ARGS__)
#define DECL_MOCK_METHOD1(m, ...) SKY_DECL_MOCK_METHOD1_(, , , m, __VA_ARGS__)
#define DECL_MOCK_METHOD2(m, ...) SKY_DECL_MOCK_METHOD2_(, , , m, __VA_ARGS__)
#define DECL_MOCK_METHOD3(m, ...) SKY_DECL_MOCK_METHOD3_(, , , m, __VA_ARGS__)
#define DECL_MOCK_METHOD4(m, ...) SKY_DECL_MOCK_METHOD4_(, , , m, __VA_ARGS__)
#define DECL_MOCK_METHOD5(m, ...) SKY_DECL_MOCK_METHOD5_(, , , m, __VA_ARGS__)
#define DECL_MOCK_METHOD6(m, ...) SKY_DECL_MOCK_METHOD6_(, , , m, __VA_ARGS__)
#define DECL_MOCK_METHOD7(m, ...) SKY_DECL_MOCK_METHOD7_(, , , m, __VA_ARGS__)
#define DECL_MOCK_METHOD8(m, ...) SKY_DECL_MOCK_METHOD8_(, , , m, __VA_ARGS__)
#define DECL_MOCK_METHOD9(m, ...) SKY_DECL_MOCK_METHOD9_(, , , m, __VA_ARGS__)
#define DECL_MOCK_METHOD10(m, ...) SKY_DECL_MOCK_METHOD10_(, , , m, __VA_ARGS__)

#define DECL_MOCK_CONST_METHOD0(m, ...) SKY_DECL_MOCK_METHOD0_(, const, , m, __VA_ARGS__)
#define DECL_MOCK_CONST_METHOD1(m, ...) SKY_DECL_MOCK_METHOD1_(, const, , m, __VA_ARGS__)
#define DECL_MOCK_CONST_METHOD2(m, ...) SKY_DECL_MOCK_METHOD2_(, const, , m, __VA_ARGS__)
#define DECL_MOCK_CONST_METHOD3(m, ...) SKY_DECL_MOCK_METHOD3_(, const, , m, __VA_ARGS__)
#define DECL_MOCK_CONST_METHOD4(m, ...) SKY_DECL_MOCK_METHOD4_(, const, , m, __VA_ARGS__)
#define DECL_MOCK_CONST_METHOD5(m, ...) SKY_DECL_MOCK_METHOD5_(, const, , m, __VA_ARGS__)
#define DECL_MOCK_CONST_METHOD6(m, ...) SKY_DECL_MOCK_METHOD6_(, const, , m, __VA_ARGS__)
#define DECL_MOCK_CONST_METHOD7(m, ...) SKY_DECL_MOCK_METHOD7_(, const, , m, __VA_ARGS__)
#define DECL_MOCK_CONST_METHOD8(m, ...) SKY_DECL_MOCK_METHOD8_(, const, , m, __VA_ARGS__)
#define DECL_MOCK_CONST_METHOD9(m, ...) SKY_DECL_MOCK_METHOD9_(, const, , m, __VA_ARGS__)
#define DECL_MOCK_CONST_METHOD10(m, ...) SKY_DECL_MOCK_METHOD10_(, const, , m, __VA_ARGS__)



// mock definition macros
#define DEF_MOCK_METHOD0(m, ...) SKY_DEF_MOCK_METHOD0_(, , , m, __VA_ARGS__)
#define DEF_MOCK_METHOD1(m, ...) SKY_DEF_MOCK_METHOD1_(, , , m, __VA_ARGS__)
#define DEF_MOCK_METHOD2(m, ...) SKY_DEF_MOCK_METHOD2_(, , , m, __VA_ARGS__)
#define DEF_MOCK_METHOD3(m, ...) SKY_DEF_MOCK_METHOD3_(, , , m, __VA_ARGS__)
#define DEF_MOCK_METHOD4(m, ...) SKY_DEF_MOCK_METHOD4_(, , , m, __VA_ARGS__)
#define DEF_MOCK_METHOD5(m, ...) SKY_DEF_MOCK_METHOD5_(, , , m, __VA_ARGS__)
#define DEF_MOCK_METHOD6(m, ...) SKY_DEF_MOCK_METHOD6_(, , , m, __VA_ARGS__)
#define DEF_MOCK_METHOD7(m, ...) SKY_DEF_MOCK_METHOD7_(, , , m, __VA_ARGS__)
#define DEF_MOCK_METHOD8(m, ...) SKY_DEF_MOCK_METHOD8_(, , , m, __VA_ARGS__)
#define DEF_MOCK_METHOD9(m, ...) SKY_DEF_MOCK_METHOD9_(, , , m, __VA_ARGS__)
#define DEF_MOCK_METHOD10(m, ...) SKY_DEF_MOCK_METHOD10_(, , , m, __VA_ARGS__)

#define DEF_MOCK_CONST_METHOD0(m, ...) SKY_DEF_MOCK_METHOD0_(, const, , m, __VA_ARGS__)
#define DEF_MOCK_CONST_METHOD1(m, ...) SKY_DEF_MOCK_METHOD1_(, const, , m, __VA_ARGS__)
#define DEF_MOCK_CONST_METHOD2(m, ...) SKY_DEF_MOCK_METHOD2_(, const, , m, __VA_ARGS__)
#define DEF_MOCK_CONST_METHOD3(m, ...) SKY_DEF_MOCK_METHOD3_(, const, , m, __VA_ARGS__)
#define DEF_MOCK_CONST_METHOD4(m, ...) SKY_DEF_MOCK_METHOD4_(, const, , m, __VA_ARGS__)
#define DEF_MOCK_CONST_METHOD5(m, ...) SKY_DEF_MOCK_METHOD5_(, const, , m, __VA_ARGS__)
#define DEF_MOCK_CONST_METHOD6(m, ...) SKY_DEF_MOCK_METHOD6_(, const, , m, __VA_ARGS__)
#define DEF_MOCK_CONST_METHOD7(m, ...) SKY_DEF_MOCK_METHOD7_(, const, , m, __VA_ARGS__)
#define DEF_MOCK_CONST_METHOD8(m, ...) SKY_DEF_MOCK_METHOD8_(, const, , m, __VA_ARGS__)
#define DEF_MOCK_CONST_METHOD9(m, ...) SKY_DEF_MOCK_METHOD9_(, const, , m, __VA_ARGS__)
#define DEF_MOCK_CONST_METHOD10(m, ...) SKY_DEF_MOCK_METHOD10_(, const, , m, __VA_ARGS__)







#define DONT_MOCK_METHOD0_(tn, constness, ct, Method, ...) \
  GMOCK_RESULT_(tn, __VA_ARGS__) ct Method() constness { \
	ADD_FAILURE() << "TEST FAILED: execution reached call to " \
        "DONT_MOCK_FUNCTION(" #Method ", " #__VA_ARGS__ ")"; \
    throw "call to fake function!"; \
  }


#define DONT_MOCK_METHOD1_(tn, constness, ct, Method, __VA_ARGS__) \
  GMOCK_RESULT_(tn, __VA_ARGS__) ct Method(GMOCK_ARG_(tn, 1, __VA_ARGS__) gmock_a1) constness { \
	ADD_FAILURE() << "TEST FAILED: execution reached call to " \
        "DONT_MOCK_FUNCTION(" #Method ", " #__VA_ARGS__ ")"; \
    throw "call to fake function!"; \
  }


#define DONT_MOCK_METHOD2_(tn, constness, ct, Method, __VA_ARGS__) \
  GMOCK_RESULT_(tn, __VA_ARGS__) ct Method(GMOCK_ARG_(tn, 1, __VA_ARGS__) gmock_a1, \
                                 GMOCK_ARG_(tn, 2, __VA_ARGS__) gmock_a2) constness { \
	ADD_FAILURE() << "TEST FAILED: execution reached call to " \
        "DONT_MOCK_FUNCTION(" #Method ", " #__VA_ARGS__ ")"; \
    throw "call to fake function!"; \
  }


#define DONT_MOCK_METHOD3_(tn, constness, ct, Method, ...) \
  GMOCK_RESULT_(tn, __VA_ARGS__) ct Method(GMOCK_ARG_(tn, 1, __VA_ARGS__) gmock_a1, \
                                 GMOCK_ARG_(tn, 2, __VA_ARGS__) gmock_a2, \
                                 GMOCK_ARG_(tn, 3, __VA_ARGS__) gmock_a3) constness { \
	ADD_FAILURE() << "TEST FAILED: execution reached call to " \
        "DONT_MOCK_FUNCTION(" #Method ", " #__VA_ARGS__ ")"; \
    throw "call to fake function!"; \
  }


#define DONT_MOCK_METHOD4_(tn, constness, ct, Method, __VA_ARGS__) \
  GMOCK_RESULT_(tn, __VA_ARGS__) ct Method(GMOCK_ARG_(tn, 1, __VA_ARGS__) gmock_a1, \
                                 GMOCK_ARG_(tn, 2, __VA_ARGS__) gmock_a2, \
                                 GMOCK_ARG_(tn, 3, __VA_ARGS__) gmock_a3, \
                                 GMOCK_ARG_(tn, 4, __VA_ARGS__) gmock_a4) constness { \
	ADD_FAILURE() << "TEST FAILED: execution reached call to " \
        "DONT_MOCK_FUNCTION(" #Method ", " #__VA_ARGS__ ")"; \
    throw "call to fake function!"; \
  }


#define DONT_MOCK_METHOD5_(tn, constness, ct, Method, __VA_ARGS__) \
  GMOCK_RESULT_(tn, __VA_ARGS__) ct Method(GMOCK_ARG_(tn, 1, __VA_ARGS__) gmock_a1, \
                                 GMOCK_ARG_(tn, 2, __VA_ARGS__) gmock_a2, \
                                 GMOCK_ARG_(tn, 3, __VA_ARGS__) gmock_a3, \
                                 GMOCK_ARG_(tn, 4, __VA_ARGS__) gmock_a4, \
                                 GMOCK_ARG_(tn, 5, __VA_ARGS__) gmock_a5) constness { \
	ADD_FAILURE() << "TEST FAILED: execution reached call to " \
        "DONT_MOCK_FUNCTION(" #Method ", " #__VA_ARGS__ ")"; \
    throw "call to fake function!"; \
  }


#define DONT_MOCK_METHOD6_(tn, constness, ct, Method, __VA_ARGS__) \
  GMOCK_RESULT_(tn, __VA_ARGS__) ct Method(GMOCK_ARG_(tn, 1, __VA_ARGS__) gmock_a1, \
                                 GMOCK_ARG_(tn, 2, __VA_ARGS__) gmock_a2, \
                                 GMOCK_ARG_(tn, 3, __VA_ARGS__) gmock_a3, \
                                 GMOCK_ARG_(tn, 4, __VA_ARGS__) gmock_a4, \
                                 GMOCK_ARG_(tn, 5, __VA_ARGS__) gmock_a5, \
                                 GMOCK_ARG_(tn, 6, __VA_ARGS__) gmock_a6) constness { \
	ADD_FAILURE() << "TEST FAILED: execution reached call to " \
        "DONT_MOCK_FUNCTION(" #Method ", " #__VA_ARGS__ ")"; \
    throw "call to fake function!"; \
  }


#define DONT_MOCK_METHOD7_(tn, constness, ct, Method, __VA_ARGS__) \
  GMOCK_RESULT_(tn, __VA_ARGS__) ct Method(GMOCK_ARG_(tn, 1, __VA_ARGS__) gmock_a1, \
                                 GMOCK_ARG_(tn, 2, __VA_ARGS__) gmock_a2, \
                                 GMOCK_ARG_(tn, 3, __VA_ARGS__) gmock_a3, \
                                 GMOCK_ARG_(tn, 4, __VA_ARGS__) gmock_a4, \
                                 GMOCK_ARG_(tn, 5, __VA_ARGS__) gmock_a5, \
                                 GMOCK_ARG_(tn, 6, __VA_ARGS__) gmock_a6, \
                                 GMOCK_ARG_(tn, 7, __VA_ARGS__) gmock_a7) constness { \
	ADD_FAILURE() << "TEST FAILED: execution reached call to " \
        "DONT_MOCK_FUNCTION(" #Method ", " #__VA_ARGS__ ")"; \
    throw "call to fake function!"; \
  }


#define DONT_MOCK_METHOD8_(tn, constness, ct, Method, __VA_ARGS__) \
  GMOCK_RESULT_(tn, __VA_ARGS__) ct Method(GMOCK_ARG_(tn, 1, __VA_ARGS__) gmock_a1, \
                                 GMOCK_ARG_(tn, 2, __VA_ARGS__) gmock_a2, \
                                 GMOCK_ARG_(tn, 3, __VA_ARGS__) gmock_a3, \
                                 GMOCK_ARG_(tn, 4, __VA_ARGS__) gmock_a4, \
                                 GMOCK_ARG_(tn, 5, __VA_ARGS__) gmock_a5, \
                                 GMOCK_ARG_(tn, 6, __VA_ARGS__) gmock_a6, \
                                 GMOCK_ARG_(tn, 7, __VA_ARGS__) gmock_a7, \
                                 GMOCK_ARG_(tn, 8, __VA_ARGS__) gmock_a8) constness { \
	ADD_FAILURE() << "TEST FAILED: execution reached call to " \
        "DONT_MOCK_FUNCTION(" #Method ", " #__VA_ARGS__ ")"; \
    throw "call to fake function!"; \
  }



#define DONT_MOCK_METHOD9_(tn, constness, ct, Method, __VA_ARGS__) \
  GMOCK_RESULT_(tn, __VA_ARGS__) ct Method(GMOCK_ARG_(tn, 1, __VA_ARGS__) gmock_a1, \
                                 GMOCK_ARG_(tn, 2, __VA_ARGS__) gmock_a2, \
                                 GMOCK_ARG_(tn, 3, __VA_ARGS__) gmock_a3, \
                                 GMOCK_ARG_(tn, 4, __VA_ARGS__) gmock_a4, \
                                 GMOCK_ARG_(tn, 5, __VA_ARGS__) gmock_a5, \
                                 GMOCK_ARG_(tn, 6, __VA_ARGS__) gmock_a6, \
                                 GMOCK_ARG_(tn, 7, __VA_ARGS__) gmock_a7, \
                                 GMOCK_ARG_(tn, 8, __VA_ARGS__) gmock_a8, \
                                 GMOCK_ARG_(tn, 9, __VA_ARGS__) gmock_a9) constness { \
	ADD_FAILURE() << "TEST FAILED: execution reached call to " \
        "DONT_MOCK_FUNCTION(" #Method ", " #__VA_ARGS__ ")"; \
    throw "call to fake function!"; \
  }


#define DONT_MOCK_METHOD10_(tn, constness, ct, Method, __VA_ARGS__) \
  GMOCK_RESULT_(tn, __VA_ARGS__) ct Method(GMOCK_ARG_(tn, 1, __VA_ARGS__) gmock_a1, \
                                 GMOCK_ARG_(tn, 2, __VA_ARGS__) gmock_a2, \
                                 GMOCK_ARG_(tn, 3, __VA_ARGS__) gmock_a3, \
                                 GMOCK_ARG_(tn, 4, __VA_ARGS__) gmock_a4, \
                                 GMOCK_ARG_(tn, 5, __VA_ARGS__) gmock_a5, \
                                 GMOCK_ARG_(tn, 6, __VA_ARGS__) gmock_a6, \
                                 GMOCK_ARG_(tn, 7, __VA_ARGS__) gmock_a7, \
                                 GMOCK_ARG_(tn, 8, __VA_ARGS__) gmock_a8, \
                                 GMOCK_ARG_(tn, 9, __VA_ARGS__) gmock_a9, \
                                 GMOCK_ARG_(tn, 10, __VA_ARGS__) gmock_a10) constness { \
	ADD_FAILURE() << "TEST FAILED: execution reached call to " \
        "DONT_MOCK_FUNCTION(" #Method ", " #__VA_ARGS__ ")"; \
    throw "call to fake function!"; \
  }


// mock definition macros
#define DONT_MOCK_METHOD0(m, ...) DONT_MOCK_METHOD0_(, , , m, __VA_ARGS__)
#define DONT_MOCK_METHOD1(m, ...) DONT_MOCK_METHOD1_(, , , m, __VA_ARGS__)
#define DONT_MOCK_METHOD2(m, ...) DONT_MOCK_METHOD2_(, , , m, __VA_ARGS__)
#define DONT_MOCK_METHOD3(m, ...) DONT_MOCK_METHOD3_(, , , m, __VA_ARGS__)
#define DONT_MOCK_METHOD4(m, ...) DONT_MOCK_METHOD4_(, , , m, __VA_ARGS__)
#define DONT_MOCK_METHOD5(m, ...) DONT_MOCK_METHOD5_(, , , m, __VA_ARGS__)
#define DONT_MOCK_METHOD6(m, ...) DONT_MOCK_METHOD6_(, , , m, __VA_ARGS__)
#define DONT_MOCK_METHOD7(m, ...) DONT_MOCK_METHOD7_(, , , m, __VA_ARGS__)
#define DONT_MOCK_METHOD8(m, ...) DONT_MOCK_METHOD8_(, , , m, __VA_ARGS__)
#define DONT_MOCK_METHOD9(m, ...) DONT_MOCK_METHOD9_(, , , m, __VA_ARGS__)
#define DONT_MOCK_METHOD10(m, ...) DONT_MOCK_METHOD10_(, , , m, __VA_ARGS__)

#define DONT_MOCK_CONST_METHOD0(m, ...) DONT_MOCK_METHOD0_(, const, , m, __VA_ARGS__)
#define DONT_MOCK_CONST_METHOD1(m, ...) DONT_MOCK_METHOD1_(, const, , m, __VA_ARGS__)
#define DONT_MOCK_CONST_METHOD2(m, ...) DONT_MOCK_METHOD2_(, const, , m, __VA_ARGS__)
#define DONT_MOCK_CONST_METHOD3(m, ...) DONT_MOCK_METHOD3_(, const, , m, __VA_ARGS__)
#define DONT_MOCK_CONST_METHOD4(m, ...) DONT_MOCK_METHOD4_(, const, , m, __VA_ARGS__)
#define DONT_MOCK_CONST_METHOD5(m, ...) DONT_MOCK_METHOD5_(, const, , m, __VA_ARGS__)
#define DONT_MOCK_CONST_METHOD6(m, ...) DONT_MOCK_METHOD6_(, const, , m, __VA_ARGS__)
#define DONT_MOCK_CONST_METHOD7(m, ...) DONT_MOCK_METHOD7_(, const, , m, __VA_ARGS__)
#define DONT_MOCK_CONST_METHOD8(m, ...) DONT_MOCK_METHOD8_(, const, , m, __VA_ARGS__)
#define DONT_MOCK_CONST_METHOD9(m, ...) DONT_MOCK_METHOD9_(, const, , m, __VA_ARGS__)
#define DONT_MOCK_CONST_METHOD10(m, ...) DONT_MOCK_METHOD10_(, const, , m, __VA_ARGS__)

/*
#define ON_CALL_FORW(tn, constness, ct, Method, F) \
SKY_MOCKER_(Method, 10, constness).Invoke(gmock_a1, gmock_a2, gmock_a3, \
        gmock_a4, gmock_a5, gmock_a6, gmock_a7, gmock_a8, gmock_a9, \
        gmock_a10);
*/

#define SKY_FORWARD_MOCK_CONST_METHOD0(m, ...) SKY_FORWARD_MOCK_METHOD0_(,const, , m, __VA_ARGS__)
#define SKY_FORWARD_MOCK_CONST_METHOD1(m, ...) SKY_FORWARD_MOCK_METHOD1_(,const, , m, __VA_ARGS__)
#define SKY_FORWARD_MOCK_CONST_METHOD2(m, ...) SKY_FORWARD_MOCK_METHOD2_(,const, , m, __VA_ARGS__)
#define SKY_FORWARD_MOCK_CONST_METHOD3(m, ...) SKY_FORWARD_MOCK_METHOD3_(,const, , m, __VA_ARGS__)
#define SKY_FORWARD_MOCK_CONST_METHOD4(m, ...) SKY_FORWARD_MOCK_METHOD4_(,const, , m, __VA_ARGS__)
#define SKY_FORWARD_MOCK_CONST_METHOD5(m, ...) SKY_FORWARD_MOCK_METHOD5_(,const, , m, __VA_ARGS__)

#define SKY_FORWARD_MOCK_METHOD0(m, ...) SKY_FORWARD_MOCK_METHOD0_(, , , m, __VA_ARGS__)
#define SKY_FORWARD_MOCK_METHOD1(m, ...) SKY_FORWARD_MOCK_METHOD1_(, , , m, __VA_ARGS__)
#define SKY_FORWARD_MOCK_METHOD2(m, ...) SKY_FORWARD_MOCK_METHOD2_(, , , m, __VA_ARGS__)
#define SKY_FORWARD_MOCK_METHOD3(m, ...) SKY_FORWARD_MOCK_METHOD3_(, , , m, __VA_ARGS__)
#define SKY_FORWARD_MOCK_METHOD4(m, ...) SKY_FORWARD_MOCK_METHOD4_(, , , m, __VA_ARGS__)
#define SKY_FORWARD_MOCK_METHOD5(m, ...) SKY_FORWARD_MOCK_METHOD5_(, , , m, __VA_ARGS__)


#define SKY_FORWARD_MOCK_METHOD0_(tn, constness, ct, Method, ...) \
	(static_cast<constness SourceClassName*>(delegateSource)->gmock_##Method()).InternalDefaultActionSetAt(__FILE__, __LINE__, \
                                                    "A", #Method)\
	.WillByDefault(testing::Invoke<TargetClassName, GMOCK_RESULT_(tn, __VA_ARGS__)(TargetClassName::*)() constness >(&target,&TargetClassName::Method))

#define SKY_FORWARD_MOCK_METHOD1_(tn, constness, ct, Method, ...) \
	(static_cast<constness SourceClassName*>(delegateSource)->gmock_##Method(testing::An<GMOCK_ARG_(tn, 1, __VA_ARGS__)>())).InternalDefaultActionSetAt(__FILE__, __LINE__, \
                                                    "A", #Method)\
	.WillByDefault(testing::Invoke<TargetClassName, GMOCK_RESULT_(tn, __VA_ARGS__)(TargetClassName::*)(GMOCK_ARG_(tn, 1, __VA_ARGS__)) constness>(&target,&TargetClassName::Method))


#define SKY_FORWARD_MOCK_METHOD2_(tn, constness, ct, Method, ...) \
	(static_cast<constness SourceClassName*>(delegateSource)->gmock_##Method(\
														 testing::An<GMOCK_ARG_(tn, 1, __VA_ARGS__)>(),\
														 testing::An<GMOCK_ARG_(tn, 2, __VA_ARGS__)>()\
														 )).InternalDefaultActionSetAt(__FILE__, __LINE__, \
                                                    "A", #Method)\
	.WillByDefault(testing::Invoke<TargetClassName, GMOCK_RESULT_(tn, __VA_ARGS__)(TargetClassName::*)(\
																		GMOCK_ARG_(tn, 1, __VA_ARGS__),\
																		GMOCK_ARG_(tn, 2, __VA_ARGS__)\
																		) constness>(&target,&TargetClassName::Method))

#define SKY_FORWARD_MOCK_METHOD3_(tn, constness, ct, Method, ...) \
	(static_cast<constness SourceClassName*>(delegateSource)->gmock_##Method(\
														testing::An<GMOCK_ARG_(tn, 1, __VA_ARGS__)>(),\
														testing::An<GMOCK_ARG_(tn, 2, __VA_ARGS__)>(),\
														testing::An<GMOCK_ARG_(tn, 3, __VA_ARGS__)>()\
														)).InternalDefaultActionSetAt(__FILE__, __LINE__, \
                                                    "A", #Method)\
	.WillByDefault(testing::Invoke<TargetClassName, GMOCK_RESULT_(tn, __VA_ARGS__)(TargetClassName::*)(\
																		GMOCK_ARG_(tn, 1, __VA_ARGS__),\
																		GMOCK_ARG_(tn, 2, __VA_ARGS__),\
																		GMOCK_ARG_(tn, 3, __VA_ARGS__)\
																		) constness>(&target,&TargetClassName::Method))

#define SKY_FORWARD_MOCK_METHOD4_(tn, constness, ct, Method, ...) \
	(static_cast<constness SourceClassName*>(delegateSource)->gmock_##Method(\
														testing::An<GMOCK_ARG_(tn, 1, __VA_ARGS__)>(),\
														testing::An<GMOCK_ARG_(tn, 2, __VA_ARGS__)>(),\
														testing::An<GMOCK_ARG_(tn, 3, __VA_ARGS__)>(),\
														testing::An<GMOCK_ARG_(tn, 4, __VA_ARGS__)>()\
														)).InternalDefaultActionSetAt(__FILE__, __LINE__, \
                                                    "A", #Method)\
	.WillByDefault(testing::Invoke<TargetClassName, GMOCK_RESULT_(tn, __VA_ARGS__)(TargetClassName::*)(\
																		GMOCK_ARG_(tn, 1, __VA_ARGS__),\
																		GMOCK_ARG_(tn, 2, __VA_ARGS__),\
																		GMOCK_ARG_(tn, 3, __VA_ARGS__),\
																		GMOCK_ARG_(tn, 4, __VA_ARGS__)\
																		) constness>(&target,&TargetClassName::Method))

#define SKY_FORWARD_MOCK_METHOD5_(tn, constness, ct, Method, ...) \
	(static_cast<constness SourceClassName*>(delegateSource)->gmock_##Method(\
														testing::An<GMOCK_ARG_(tn, 1, __VA_ARGS__)>(),\
														testing::An<GMOCK_ARG_(tn, 2, __VA_ARGS__)>(),\
														testing::An<GMOCK_ARG_(tn, 3, __VA_ARGS__)>(),\
														testing::An<GMOCK_ARG_(tn, 4, __VA_ARGS__)>(),\
														testing::An<GMOCK_ARG_(tn, 5, __VA_ARGS__)>()\
														)).InternalDefaultActionSetAt(__FILE__, __LINE__, \
                                                    "A", #Method)\
	.WillByDefault(testing::Invoke<TargetClassName, GMOCK_RESULT_(tn, __VA_ARGS__)(TargetClassName::*)(\
																		GMOCK_ARG_(tn, 1, __VA_ARGS__),\
																		GMOCK_ARG_(tn, 2, __VA_ARGS__),\
																		GMOCK_ARG_(tn, 3, __VA_ARGS__),\
																		GMOCK_ARG_(tn, 4, __VA_ARGS__),\
																		GMOCK_ARG_(tn, 5, __VA_ARGS__)\
																		) constness>(&target,&TargetClassName::Method))


#if 0
#define DONT_MOCK_METHOD2_(tn, constness, ct, Method, ...) \
  GMOCK_RESULT_(tn, __VA_ARGS__) ct Method(GMOCK_ARG_(tn, 1, __VA_ARGS__) gmock_a1, \
                                 GMOCK_ARG_(tn, 2, __VA_ARGS__) gmock_a2) constness { \
	ADD_FAILURE() << "TEST FAILED: execution reached call to " \
        "DONT_MOCK_FUNCTION(" #Method ", " #__VA_ARGS__ ")"; \
    throw "call to fake function!"; \
  }

#define DONT_MOCK_METHOD2(m, ...) DONT_MOCK_METHOD2_(, const, , m, __VA_ARGS__)

#endif


#endif
