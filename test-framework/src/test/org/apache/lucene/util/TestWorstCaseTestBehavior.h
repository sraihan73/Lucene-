#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <iostream>
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

using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestWorstCaseTestBehavior : public LuceneTestCase
{
  GET_CLASS_NAME(TestWorstCaseTestBehavior)
public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Ignore public void testThreadLeak()
  virtual void testThreadLeak();

private:
  class ThreadAnonymousInnerClass : public Thread
  {
    GET_CLASS_NAME(ThreadAnonymousInnerClass)
  private:
    std::shared_ptr<TestWorstCaseTestBehavior> outerInstance;

  public:
    ThreadAnonymousInnerClass(
        std::shared_ptr<TestWorstCaseTestBehavior> outerInstance);

    void run() override;

  protected:
    std::shared_ptr<ThreadAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ThreadAnonymousInnerClass>(
          Thread::shared_from_this());
    }
  };

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Ignore public void testLaaaaaargeOutput() throws Exception
  virtual void testLaaaaaargeOutput() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Ignore public void testProgressiveOutput() throws Exception
  virtual void testProgressiveOutput() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Ignore public void testUncaughtException() throws Exception
  virtual void testUncaughtException() ;

private:
  class ThreadAnonymousInnerClass2 : public Thread
  {
    GET_CLASS_NAME(ThreadAnonymousInnerClass2)
  private:
    std::shared_ptr<TestWorstCaseTestBehavior> outerInstance;

  public:
    ThreadAnonymousInnerClass2(
        std::shared_ptr<TestWorstCaseTestBehavior> outerInstance);

    void run() override;

  protected:
    std::shared_ptr<ThreadAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<ThreadAnonymousInnerClass2>(
          Thread::shared_from_this());
    }
  };

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Ignore @Timeout(millis = 500) public void testTimeout()
  // throws Exception
  virtual void testTimeout() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Ignore @Timeout(millis = 1000) public void testZombie()
  // throws Exception
  virtual void testZombie() ;

protected:
  std::shared_ptr<TestWorstCaseTestBehavior> shared_from_this()
  {
    return std::static_pointer_cast<TestWorstCaseTestBehavior>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::util
