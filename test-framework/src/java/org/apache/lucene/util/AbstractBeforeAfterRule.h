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
 * A {@link TestRule} that guarantees the execution of {@link #after} even
 * if an exception has been thrown from delegate {@link Statement}. This is much
 * like {@link AfterClass} or {@link After} annotations but can be used with
 * {@link RuleChain} to guarantee the order of execution.
 */
class AbstractBeforeAfterRule
    : public std::enable_shared_from_this<AbstractBeforeAfterRule>,
      public TestRule
{
  GET_CLASS_NAME(AbstractBeforeAfterRule)
public:
  std::shared_ptr<Statement> apply(std::shared_ptr<Statement> s,
                                   std::shared_ptr<Description> d) override;

private:
  class StatementAnonymousInnerClass : public Statement
  {
    GET_CLASS_NAME(StatementAnonymousInnerClass)
  private:
    std::shared_ptr<AbstractBeforeAfterRule> outerInstance;

    std::shared_ptr<Statement> s;

  public:
    StatementAnonymousInnerClass(
        std::shared_ptr<AbstractBeforeAfterRule> outerInstance,
        std::shared_ptr<Statement> s);

    void evaluate()  override;

  protected:
    std::shared_ptr<StatementAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<StatementAnonymousInnerClass>(
          org.junit.runners.model.Statement::shared_from_this());
    }
  };

protected:
  virtual void before() ;
  virtual void after() ;
};

} // namespace org::apache::lucene::util
