//
// Created by Martin Harriman on 8/12/20.
//

#ifndef EVENTSTEST_RUNTESTS_HPP
#define EVENTSTEST_RUNTESTS_HPP
#include "jni.h"
#include "../include/public/Version.hpp"

namespace MAT_NS_BEGIN
{

class RunTests
{
 public:
  static int run_all_tests(JNIEnv* env, jobject logger);
  static JavaVM * javaVm;
};
} MAT_NS_END
#endif //EVENTSTEST_RUNTESTS_HPP
