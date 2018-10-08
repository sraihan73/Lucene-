#pragma once
#include "stringhelper.h"
#include <algorithm>
#include <memory>
#include <stdexcept>
#include <string>
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

class TestExceptionInBeforeClassHooks : public WithNestedTests
{
  GET_CLASS_NAME(TestExceptionInBeforeClassHooks)
public:
  TestExceptionInBeforeClassHooks();

public:
  class Nested1 : public WithNestedTests::AbstractNestedTest
  {
    GET_CLASS_NAME(Nested1)
  public:
    // C++ TODO: Most Java annotations will not have direct C++ equivalents:
    // ORIGINAL LINE: @BeforeClass public static void beforeClass() throws
    // Exception
    static void beforeClass() ;

  private:
    class ThreadAnonymousInnerClass : public Thread
    {
      GET_CLASS_NAME(ThreadAnonymousInnerClass)
    public:
      ThreadAnonymousInnerClass();

      void run() override;

    protected:
      std::shared_ptr<ThreadAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<ThreadAnonymousInnerClass>(
            Thread::shared_from_this());
      }
    };

  public:
    virtual void test();

  protected:
    std::shared_ptr<Nested1> shared_from_this()
    {
      return std::static_pointer_cast<Nested1>(
          WithNestedTests.AbstractNestedTest::shared_from_this());
    }
  };

public:
  class Nested2 : public WithNestedTests::AbstractNestedTest
  {
    GET_CLASS_NAME(Nested2)
  public:
    virtual void test1() ;

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

  public:
    virtual void test2() ;

  private:
    class ThreadAnonymousInnerClass2 : public Thread
    {
      GET_CLASS_NAME(ThreadAnonymousInnerClass2)
    private:
      std::shared_ptr<Nested2> outerInstance;

    public:
      ThreadAnonymousInnerClass2(std::shared_ptr<Nested2> outerInstance);

      void run() override;

    protected:
      std::shared_ptr<ThreadAnonymousInnerClass2> shared_from_this()
      {
        return std::static_pointer_cast<ThreadAnonymousInnerClass2>(
            Thread::shared_from_this());
      }
    };

  public:
    virtual void test3() ;

  private:
    class ThreadAnonymousInnerClass3 : public Thread
    {
      GET_CLASS_NAME(ThreadAnonymousInnerClass3)
    private:
      std::shared_ptr<Nested2> outerInstance;

    public:
      ThreadAnonymousInnerClass3(std::shared_ptr<Nested2> outerInstance);

      void run() override;

    protected:
      std::shared_ptr<ThreadAnonymousInnerClass3> shared_from_this()
      {
        return std::static_pointer_cast<ThreadAnonymousInnerClass3>(
            Thread::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<Nested2> shared_from_this()
    {
      return std::static_pointer_cast<Nested2>(
          WithNestedTests.AbstractNestedTest::shared_from_this());
    }
  };

public:
  class Nested3 : public WithNestedTests::AbstractNestedTest
  {
    GET_CLASS_NAME(Nested3)
  public:
    // C++ TODO: Most Java annotations will not have direct C++ equivalents:
    // ORIGINAL LINE: @Before public void runBeforeTest() throws Exception
    virtual void runBeforeTest() ;

  private:
    class ThreadAnonymousInnerClass : public Thread
    {
      GET_CLASS_NAME(ThreadAnonymousInnerClass)
    private:
      std::shared_ptr<Nested3> outerInstance;

    public:
      ThreadAnonymousInnerClass(std::shared_ptr<Nested3> outerInstance);

      void run() override;

    protected:
      std::shared_ptr<ThreadAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<ThreadAnonymousInnerClass>(
            Thread::shared_from_this());
      }
    };

  public:
    virtual void test1() ;

  protected:
    std::shared_ptr<Nested3> shared_from_this()
    {
      return std::static_pointer_cast<Nested3>(
          WithNestedTests.AbstractNestedTest::shared_from_this());
    }
  };

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testExceptionInBeforeClassFailsTheTest()
  virtual void testExceptionInBeforeClassFailsTheTest();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testExceptionWithinTestFailsTheTest()
  virtual void testExceptionWithinTestFailsTheTest();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testExceptionWithinBefore()
  virtual void testExceptionWithinBefore();

protected:
  std::shared_ptr<TestExceptionInBeforeClassHooks> shared_from_this()
  {
    return std::static_pointer_cast<TestExceptionInBeforeClassHooks>(
        WithNestedTests::shared_from_this());
  }
};

} // namespace org::apache::lucene::util
