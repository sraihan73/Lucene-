#pragma once
#include "stringhelper.h"
#include <memory>
#include <stack>
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

/**
 * This verifies that JUnit {@link Rule}s are invoked before
 * {@link Before} and {@link  After} hooks. This should be the
 * case from JUnit 4.10 on.
 */
class TestJUnitRuleOrder : public WithNestedTests
{
  GET_CLASS_NAME(TestJUnitRuleOrder)
public:
  static std::stack<std::wstring> stack;

  TestJUnitRuleOrder();

public:
  class Nested : public WithNestedTests::AbstractNestedTest
  {
    GET_CLASS_NAME(Nested)
  public:
    // C++ TODO: Most Java annotations will not have direct C++ equivalents:
    // ORIGINAL LINE: @Before public void before()
    virtual void before();

    // C++ TODO: Most Java annotations will not have direct C++ equivalents:
    // ORIGINAL LINE: @After public void after()
    virtual void after();

    // C++ TODO: Most Java annotations will not have direct C++ equivalents:
    // ORIGINAL LINE: @Rule public org.junit.rules.TestRule testRule = new
    // org.junit.rules.TestRule()
    std::shared_ptr<TestRule> testRule =
        std::make_shared<TestRuleAnonymousInnerClass>();

  private:
    class TestRuleAnonymousInnerClass : public TestRule
    {
      GET_CLASS_NAME(TestRuleAnonymousInnerClass)
    public:
      TestRuleAnonymousInnerClass();

      std::shared_ptr<Statement>
      apply(std::shared_ptr<Statement> base,
            std::shared_ptr<Description> description) override;

    private:
      class StatementAnonymousInnerClass : public Statement
      {
        GET_CLASS_NAME(StatementAnonymousInnerClass)
      private:
        std::shared_ptr<TestRuleAnonymousInnerClass> outerInstance;

        std::shared_ptr<Statement> base;

      public:
        StatementAnonymousInnerClass(
            std::shared_ptr<TestRuleAnonymousInnerClass> outerInstance,
            std::shared_ptr<Statement> base);

        void evaluate()  override;

      protected:
        std::shared_ptr<StatementAnonymousInnerClass> shared_from_this()
        {
          return std::static_pointer_cast<StatementAnonymousInnerClass>(
              org.junit.runners.model.Statement::shared_from_this());
        }
      };

    protected:
      std::shared_ptr<TestRuleAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<TestRuleAnonymousInnerClass>(
            org.junit.rules.TestRule::shared_from_this());
      }
    };

  public:
    // C++ TODO: Most Java annotations will not have direct C++ equivalents:
    // ORIGINAL LINE: @Test public void test()
    virtual void test();
    // C++ TODO: Most Java annotations will not have direct C++ equivalents:
    // ORIGINAL LINE: @BeforeClass public static void beforeClassCleanup()
    static void beforeClassCleanup();

    // C++ TODO: Most Java annotations will not have direct C++ equivalents:
    // ORIGINAL LINE: @AfterClass public static void afterClassCheck()
    static void afterClassCheck();

  protected:
    std::shared_ptr<Nested> shared_from_this()
    {
      return std::static_pointer_cast<Nested>(
          WithNestedTests.AbstractNestedTest::shared_from_this());
    }
  };

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testRuleOrder()
  virtual void testRuleOrder();

protected:
  std::shared_ptr<TestJUnitRuleOrder> shared_from_this()
  {
    return std::static_pointer_cast<TestJUnitRuleOrder>(
        WithNestedTests::shared_from_this());
  }
};

} // namespace org::apache::lucene::util
