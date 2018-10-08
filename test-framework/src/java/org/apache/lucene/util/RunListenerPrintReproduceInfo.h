#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <deque>

/*
 * Licensed to the Syed Mamun Raihan (sraihan.com) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * sraihan.com licenses this file to You under GPLv3 License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
namespace org::apache::lucene::util
{

using namespace org::apache::lucene::util;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.util.LuceneTestCase.*;

using org::junit::runner::Description;
using org::junit::runner::Result;
using org::junit::runner::notification::Failure;
using org::junit::runner::notification::RunListener;

using com::carrotsearch::randomizedtesting::LifecycleScope;

/**
 * A suite listener printing a "reproduce string". This ensures test result
 * events are always captured properly even if exceptions happen at
 * initialization or suite/ hooks level.
 */
class RunListenerPrintReproduceInfo final : public RunListener
{
  GET_CLASS_NAME(RunListenerPrintReproduceInfo)
  /**
   * A deque of all test suite classes executed so far in this JVM (ehm,
   * under this class's classloader).
   */
private:
  static std::deque<std::wstring> testClassesRun;

  /**
   * The currently executing scope.
   */
  std::shared_ptr<LifecycleScope> scope;

  /** Current test failed. */
  bool testFailed = false;

  /** Suite-level code (initialization, rule, hook) failed. */
  bool suiteFailed = false;

  /** A marker to print full env. diagnostics after the suite. */
  bool printDiagnosticsAfterClass = false;

  /** true if we should skip the reproduce string (diagnostics are independent)
   */
  bool suppressReproduceLine = false;

public:
  void testRunStarted(std::shared_ptr<Description> description) throw(
      std::runtime_error) override;

  void testStarted(std::shared_ptr<Description> description) throw(
      std::runtime_error) override;

  void testFailure(std::shared_ptr<Failure> failure) throw(
      std::runtime_error) override;

  void testFinished(std::shared_ptr<Description> description) throw(
      std::runtime_error) override;

  /**
   * The {@link Description} object in JUnit does not expose the actual test
   * method, instead it has the concept of a unique "name" of a test. To run the
   * same method (tests) repeatedly, randomizedtesting must make those "names"
   * unique: it appends the current iteration and seeds to the test method's
   * name. We strip this information here.
   */
private:
  std::wstring stripTestNameAugmentations(const std::wstring &methodName);

public:
  void testRunFinished(std::shared_ptr<Result> result) throw(
      std::runtime_error) override;

  /** print some useful debugging information about the environment */
private:
  static void printDebuggingInformation();

  void reportAdditionalFailureInfo(const std::wstring &testName);

  /**
   * Append a VM option (-Dkey=value) to a {@link StringBuilder}. Add quotes if
   * spaces or other funky characters are detected.
   */
public:
  static void addVmOpt(std::shared_ptr<StringBuilder> b,
                       const std::wstring &key, std::any value);

protected:
  std::shared_ptr<RunListenerPrintReproduceInfo> shared_from_this()
  {
    return std::static_pointer_cast<RunListenerPrintReproduceInfo>(
        org.junit.runner.notification.RunListener::shared_from_this());
  }
};

} // namespace org::apache::lucene::util
