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
 * Make sure {@link LuceneTestCase#setUp()} and {@link
 * LuceneTestCase#tearDown()} were invoked even if they have been overriden. We
 * assume nobody will call these out of non-overriden methods (they have to be
 * public by contract, unfortunately). The top-level methods just set a flag
 * that is checked upon successful execution of each test case.
 */
class TestRuleSetupTeardownChained
    : public std::enable_shared_from_this<TestRuleSetupTeardownChained>,
      public TestRule
{
  GET_CLASS_NAME(TestRuleSetupTeardownChained)
  /**
   * @see TestRuleSetupTeardownChained
   */
public:
  bool setupCalled = false;

  /**
   * @see TestRuleSetupTeardownChained
   */
  bool teardownCalled = false;

  std::shared_ptr<Statement>
  apply(std::shared_ptr<Statement> base,
        std::shared_ptr<Description> description) override;

private:
  class StatementAnonymousInnerClass : public Statement
  {
    GET_CLASS_NAME(StatementAnonymousInnerClass)
  private:
    std::shared_ptr<TestRuleSetupTeardownChained> outerInstance;

    std::shared_ptr<Statement> base;

  public:
    StatementAnonymousInnerClass(
        std::shared_ptr<TestRuleSetupTeardownChained> outerInstance,
        std::shared_ptr<Statement> base);

    void evaluate()  override;

  protected:
    std::shared_ptr<StatementAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<StatementAnonymousInnerClass>(
          org.junit.runners.model.Statement::shared_from_this());
    }
  };
};
} // #include  "core/src/java/org/apache/lucene/util/
