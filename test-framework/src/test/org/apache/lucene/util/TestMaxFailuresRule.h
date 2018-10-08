#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <iostream>
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

using WithNestedTests = org::apache::lucene::util::WithNestedTests;

/**
 * @see TestRuleIgnoreAfterMaxFailures
 */
class TestMaxFailuresRule : public WithNestedTests
{
  GET_CLASS_NAME(TestMaxFailuresRule)
public:
  TestMaxFailuresRule();

public:
  class Nested : public WithNestedTests::AbstractNestedTest
  {
    GET_CLASS_NAME(Nested)
  public:
    static constexpr int TOTAL_ITERS = 500;
    static const int DESIRED_FAILURES = TOTAL_ITERS / 10;

  private:
    int numFails = 0;
    int numIters = 0;

  public:
    // C++ TODO: Most Java annotations will not have direct C++ equivalents:
    // ORIGINAL LINE: @Repeat(iterations = TOTAL_ITERS) public void
    // testFailSometimes()
    virtual void testFailSometimes();

  protected:
    std::shared_ptr<Nested> shared_from_this()
    {
      return std::static_pointer_cast<Nested>(
          org.apache.lucene.util.WithNestedTests
              .AbstractNestedTest::shared_from_this());
    }
  };

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testMaxFailures()
  virtual void testMaxFailures();

private:
  class RunListenerAnonymousInnerClass : public RunListener
  {
    GET_CLASS_NAME(RunListenerAnonymousInnerClass)
  private:
    std::shared_ptr<TestMaxFailuresRule> outerInstance;

    std::shared_ptr<StringBuilder> results;

  public:
    RunListenerAnonymousInnerClass(
        std::shared_ptr<TestMaxFailuresRule> outerInstance,
        std::shared_ptr<StringBuilder> results);

    wchar_t lastTest = L'\0';

    void testStarted(std::shared_ptr<Description> description) throw(
        std::runtime_error) override;

    void testAssumptionFailure(std::shared_ptr<Failure> failure) override;

    void testFailure(std::shared_ptr<Failure> failure) throw(
        std::runtime_error) override;

    void testFinished(std::shared_ptr<Description> description) throw(
        std::runtime_error) override;

  protected:
    std::shared_ptr<RunListenerAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<RunListenerAnonymousInnerClass>(
          org.junit.runner.notification.RunListener::shared_from_this());
    }
  };

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @ThreadLeakZombies(Consequence.IGNORE_REMAINING_TESTS)
  // @ThreadLeakAction({ThreadLeakAction.Action.WARN})
  // @ThreadLeakScope(Scope.TEST) @ThreadLeakLingering(linger = 500) public
  // static class Nested2 extends
  // org.apache.lucene.util.WithNestedTests.AbstractNestedTest
  class Nested2 : public WithNestedTests::AbstractNestedTest
  {
  public:
    static constexpr int TOTAL_ITERS = 10;
    static std::shared_ptr<CountDownLatch> die;
    static std::shared_ptr<Thread> zombie;
    static int testNum;

    // C++ TODO: Most Java annotations will not have direct C++ equivalents:
    // ORIGINAL LINE: @BeforeClass public static void setup()
    static void setup();

    // C++ TODO: Most Java annotations will not have direct C++ equivalents:
    // ORIGINAL LINE: @Repeat(iterations = TOTAL_ITERS) public void
    // testLeaveZombie()
    virtual void testLeaveZombie();

  private:
    class ThreadAnonymousInnerClass : public Thread
    {
      GET_CLASS_NAME(ThreadAnonymousInnerClass)
    private:
      std::shared_ptr<Nested2> outerInstance;

    public:
      ThreadAnonymousInnerClass(std::shared_ptr<Nested2> outerInstance);

      void run() override;

    protected:
      std::shared_ptr<ThreadAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<ThreadAnonymousInnerClass>(
            Thread::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<Nested2> shared_from_this()
    {
      return std::static_pointer_cast<Nested2>(
          org.apache.lucene.util.WithNestedTests
              .AbstractNestedTest::shared_from_this());
    }
  };

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testZombieThreadFailures() throws
  // Exception
  virtual void testZombieThreadFailures() ;

private:
  class RunListenerAnonymousInnerClass : public RunListener
  {
    GET_CLASS_NAME(RunListenerAnonymousInnerClass)
  private:
    std::shared_ptr<TestMaxFailuresRule> outerInstance;

    std::shared_ptr<StringBuilder> results;

  public:
    RunListenerAnonymousInnerClass(
        std::shared_ptr<TestMaxFailuresRule> outerInstance,
        std::shared_ptr<StringBuilder> results);

    wchar_t lastTest = L'\0';

    void testStarted(std::shared_ptr<Description> description) throw(
        std::runtime_error) override;

    void testAssumptionFailure(std::shared_ptr<Failure> failure) override;

    void testFailure(std::shared_ptr<Failure> failure) throw(
        std::runtime_error) override;

    void testFinished(std::shared_ptr<Description> description) throw(
        std::runtime_error) override;

  protected:
    std::shared_ptr<RunListenerAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<RunListenerAnonymousInnerClass>(
          org.junit.runner.notification.RunListener::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestMaxFailuresRule> shared_from_this()
  {
    return std::static_pointer_cast<TestMaxFailuresRule>(
        org.apache.lucene.util.WithNestedTests::shared_from_this());
  }
};

} // namespace org::apache::lucene::util
