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
 * Check that uncaught exceptions result in seed info being dumped to
 * console.
 */
class TestSeedFromUncaught : public WithNestedTests
{
  GET_CLASS_NAME(TestSeedFromUncaught)
public:
  class ThrowInUncaught : public AbstractNestedTest
  {
    GET_CLASS_NAME(ThrowInUncaught)
  public:
    // C++ TODO: Most Java annotations will not have direct C++ equivalents:
    // ORIGINAL LINE: @Test public void testFoo() throws Exception
    virtual void testFoo() ;

  private:
    class ThreadAnonymousInnerClass : public Thread
    {
      GET_CLASS_NAME(ThreadAnonymousInnerClass)
    private:
      std::shared_ptr<ThrowInUncaught> outerInstance;

    public:
      ThreadAnonymousInnerClass(std::shared_ptr<ThrowInUncaught> outerInstance);

      void run() override;

    protected:
      std::shared_ptr<ThreadAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<ThreadAnonymousInnerClass>(
            Thread::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<ThrowInUncaught> shared_from_this()
    {
      return std::static_pointer_cast<ThrowInUncaught>(
          AbstractNestedTest::shared_from_this());
    }
  };

public:
  TestSeedFromUncaught();

  /**
   * Verify super method calls on {@link LuceneTestCase#setUp()}.
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testUncaughtDumpsSeed()
  virtual void testUncaughtDumpsSeed();

protected:
  std::shared_ptr<TestSeedFromUncaught> shared_from_this()
  {
    return std::static_pointer_cast<TestSeedFromUncaught>(
        WithNestedTests::shared_from_this());
  }
};

} // namespace org::apache::lucene::util
