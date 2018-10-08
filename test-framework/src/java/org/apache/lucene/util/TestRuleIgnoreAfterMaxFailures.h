#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>

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
 * This rule keeps a count of failed tests (suites) and will result in an
 * {@link AssumptionViolatedException} after a given number of failures for all
 * tests following this condition.
 *
 * <p>
 * Aborting quickly on failed tests can be useful when used in combination with
 * test repeats (via the {@link Repeat} annotation or system property).
 */
class TestRuleIgnoreAfterMaxFailures final
    : public std::enable_shared_from_this<TestRuleIgnoreAfterMaxFailures>,
      public TestRule
{
  GET_CLASS_NAME(TestRuleIgnoreAfterMaxFailures)
  /**
   * Maximum failures. Package scope for tests.
   */
public:
  int maxFailures = 0;

  /**
   * @param maxFailures
   *          The number of failures after which all tests are ignored. Must be
   *          greater or equal 1.
   */
  TestRuleIgnoreAfterMaxFailures(int maxFailures);

  std::shared_ptr<Statement> apply(std::shared_ptr<Statement> s,
                                   std::shared_ptr<Description> d) override;

private:
  class StatementAnonymousInnerClass : public Statement
  {
    GET_CLASS_NAME(StatementAnonymousInnerClass)
  private:
    std::shared_ptr<TestRuleIgnoreAfterMaxFailures> outerInstance;

    std::shared_ptr<Statement> s;

  public:
    StatementAnonymousInnerClass(
        std::shared_ptr<TestRuleIgnoreAfterMaxFailures> outerInstance,
        std::shared_ptr<Statement> s);

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
