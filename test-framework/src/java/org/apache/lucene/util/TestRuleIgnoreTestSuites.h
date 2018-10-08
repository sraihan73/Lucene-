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
 * This rule will cause the suite to be assumption-ignored if
 * the test class implements a given marker interface and a special
 * property is not set.
 *
 * <p>This is a workaround for problems with certain JUnit containers (IntelliJ)
 * which automatically discover test suites and attempt to run nested classes
 * that we use for testing the test framework itself.
 */
class TestRuleIgnoreTestSuites final
    : public std::enable_shared_from_this<TestRuleIgnoreTestSuites>,
      public TestRule
{
  GET_CLASS_NAME(TestRuleIgnoreTestSuites)
  /**
   * Marker interface for nested suites that should be ignored
   * if executed in stand-alone mode.
   */
public:
  class NestedTestSuite
  {
    GET_CLASS_NAME(NestedTestSuite)
  };

  /**
   * A bool system property indicating nested suites should be executed
   * normally.
   */
public:
  static const std::wstring PROPERTY_RUN_NESTED;

  std::shared_ptr<Statement> apply(std::shared_ptr<Statement> s,
                                   std::shared_ptr<Description> d) override;

private:
  class StatementAnonymousInnerClass : public Statement
  {
    GET_CLASS_NAME(StatementAnonymousInnerClass)
  private:
    std::shared_ptr<TestRuleIgnoreTestSuites> outerInstance;

    std::shared_ptr<Statement> s;
    std::shared_ptr<Description> d;

  public:
    StatementAnonymousInnerClass(
        std::shared_ptr<TestRuleIgnoreTestSuites> outerInstance,
        std::shared_ptr<Statement> s, std::shared_ptr<Description> d);

    void evaluate()  override;

  protected:
    std::shared_ptr<StatementAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<StatementAnonymousInnerClass>(
          org.junit.runners.model.Statement::shared_from_this());
    }
  };

  /**
   * Check if a suite class is running as a nested test.
   */
public:
  static bool isRunningNested();
};

} // namespace org::apache::lucene::util
