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

/**
 * Test reproduce message is right.
 */
class TestReproduceMessage : public WithNestedTests
{
  GET_CLASS_NAME(TestReproduceMessage)
public:
  static SorePoint where;
  static SoreType type;

public:
  class Nested : public AbstractNestedTest
  {
    GET_CLASS_NAME(Nested)
  public:
    // C++ TODO: Most Java annotations will not have direct C++ equivalents:
    // ORIGINAL LINE: @BeforeClass public static void beforeClass()
    static void beforeClass();

    // C++ TODO: Most Java annotations will not have direct C++ equivalents:
    // ORIGINAL LINE: @Rule public org.junit.rules.TestRule rule = new
    // org.junit.rules.TestRule()
    std::shared_ptr<TestRule> rule =
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

    /** Class initializer block/ default constructor. */
  public:
    Nested();

    // C++ TODO: Most Java annotations will not have direct C++ equivalents:
    // ORIGINAL LINE: @Before public void before()
    virtual void before();

    // C++ TODO: Most Java annotations will not have direct C++ equivalents:
    // ORIGINAL LINE: @Test public void test()
    virtual void test();

    // C++ TODO: Most Java annotations will not have direct C++ equivalents:
    // ORIGINAL LINE: @After public void after()
    virtual void after();

    // C++ TODO: Most Java annotations will not have direct C++ equivalents:
    // ORIGINAL LINE: @AfterClass public static void afterClass()
    static void afterClass();

    /** */
  private:
    static void triggerOn(SorePoint pt);

  protected:
    std::shared_ptr<Nested> shared_from_this()
    {
      return std::static_pointer_cast<Nested>(
          AbstractNestedTest::shared_from_this());
    }
  };

  /*
   * ASSUMPTIONS.
   */

public:
  TestReproduceMessage();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testAssumeBeforeClass() throws Exception
  virtual void testAssumeBeforeClass() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testAssumeInitializer() throws Exception
  virtual void testAssumeInitializer() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testAssumeRule() throws Exception
  virtual void testAssumeRule() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testAssumeBefore() throws Exception
  virtual void testAssumeBefore() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testAssumeTest() throws Exception
  virtual void testAssumeTest() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testAssumeAfter() throws Exception
  virtual void testAssumeAfter() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testAssumeAfterClass() throws Exception
  virtual void testAssumeAfterClass() ;

  /*
   * FAILURES
   */

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testFailureBeforeClass() throws Exception
  virtual void testFailureBeforeClass() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testFailureInitializer() throws Exception
  virtual void testFailureInitializer() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testFailureRule() throws Exception
  virtual void testFailureRule() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testFailureBefore() throws Exception
  virtual void testFailureBefore() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testFailureTest() throws Exception
  virtual void testFailureTest() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testFailureAfter() throws Exception
  virtual void testFailureAfter() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testFailureAfterClass() throws Exception
  virtual void testFailureAfterClass() ;

  /*
   * ERRORS
   */

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testErrorBeforeClass() throws Exception
  virtual void testErrorBeforeClass() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testErrorInitializer() throws Exception
  virtual void testErrorInitializer() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testErrorRule() throws Exception
  virtual void testErrorRule() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testErrorBefore() throws Exception
  virtual void testErrorBefore() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testErrorTest() throws Exception
  virtual void testErrorTest() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testErrorAfter() throws Exception
  virtual void testErrorAfter() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testErrorAfterClass() throws Exception
  virtual void testErrorAfterClass() ;

private:
  std::wstring runAndReturnSyserr();

protected:
  std::shared_ptr<TestReproduceMessage> shared_from_this()
  {
    return std::static_pointer_cast<TestReproduceMessage>(
        WithNestedTests::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/util/
