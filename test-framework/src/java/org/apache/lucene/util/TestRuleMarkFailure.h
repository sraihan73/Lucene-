#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
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

using org::junit::rules::TestRule;
using org::junit::runner::Description;
using org::junit::runners::model::Statement;

/**
 * A rule for marking failed tests and suites.
 */
class TestRuleMarkFailure final
    : public std::enable_shared_from_this<TestRuleMarkFailure>,
      public TestRule
{
  GET_CLASS_NAME(TestRuleMarkFailure)
private:
  std::deque<std::shared_ptr<TestRuleMarkFailure>> const chained;
  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: private volatile bool failures;
  bool failures = false;

public:
  TestRuleMarkFailure(std::deque<TestRuleMarkFailure> &chained);

  std::shared_ptr<Statement> apply(std::shared_ptr<Statement> s,
                                   std::shared_ptr<Description> d) override;

private:
  class StatementAnonymousInnerClass : public Statement
  {
    GET_CLASS_NAME(StatementAnonymousInnerClass)
  private:
    std::shared_ptr<TestRuleMarkFailure> outerInstance;

    std::shared_ptr<Statement> s;

  public:
    StatementAnonymousInnerClass(
        std::shared_ptr<TestRuleMarkFailure> outerInstance,
        std::shared_ptr<Statement> s);

    void evaluate()  override;

  protected:
    std::shared_ptr<StatementAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<StatementAnonymousInnerClass>(
          org.junit.runners.model.Statement::shared_from_this());
    }
  };

  /**
   * Is a given exception (or a MultipleFailureException) an
   * {@link AssumptionViolatedException}?
   */
public:
  static bool isAssumption(std::runtime_error t);

  /**
   * Expand from multi-exception wrappers.
   */
private:
  static std::deque<std::runtime_error>
  expandFromMultiple(std::runtime_error t);

  /** Internal recursive routine. */
  static std::deque<std::runtime_error>
  expandFromMultiple(std::runtime_error t,
                     std::deque<std::runtime_error> &deque);

  /**
   * Taints this object and any chained as having failures.
   */
public:
  void markFailed();

  /**
   * Check if this object had any marked failures.
   */
  bool hadFailures();

  /**
   * Check if this object was successful (the opposite of {@link
   * #hadFailures()}).
   */
  bool wasSuccessful();
};

} // namespace org::apache::lucene::util
