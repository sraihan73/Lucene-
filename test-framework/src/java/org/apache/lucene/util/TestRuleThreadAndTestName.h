#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>

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

using org::junit::rules::TestRule;
using org::junit::runner::Description;
using org::junit::runners::model::Statement;

/**
 * Saves the executing thread and method name of the test case.
 */
class TestRuleThreadAndTestName final
    : public std::enable_shared_from_this<TestRuleThreadAndTestName>,
      public TestRule
{
  GET_CLASS_NAME(TestRuleThreadAndTestName)
  /**
   * The thread executing the current test case.
   * @see LuceneTestCase#isTestThread()
   */
public:
  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: public volatile Thread testCaseThread;
  std::shared_ptr<Thread> testCaseThread;

  /**
   * Test method name.
   */
  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: public volatile std::wstring testMethodName = "<unknown>";
  std::wstring testMethodName = L"<unknown>";

  std::shared_ptr<Statement>
  apply(std::shared_ptr<Statement> base,
        std::shared_ptr<Description> description) override;

private:
  class StatementAnonymousInnerClass : public Statement
  {
    GET_CLASS_NAME(StatementAnonymousInnerClass)
  private:
    std::shared_ptr<TestRuleThreadAndTestName> outerInstance;

    std::shared_ptr<Statement> base;
    std::shared_ptr<Description> description;

  public:
    StatementAnonymousInnerClass(
        std::shared_ptr<TestRuleThreadAndTestName> outerInstance,
        std::shared_ptr<Statement> base,
        std::shared_ptr<Description> description);

    void evaluate()  override;

  protected:
    std::shared_ptr<StatementAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<StatementAnonymousInnerClass>(
          org.junit.runners.model.Statement::shared_from_this());
    }
  };
};
} // namespace org::apache::lucene::util
