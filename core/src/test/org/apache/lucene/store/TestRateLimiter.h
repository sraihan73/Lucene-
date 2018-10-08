#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/RateLimiter.h"

#include  "core/src/java/org/apache/lucene/store/SimpleRateLimiter.h"

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
namespace org::apache::lucene::store
{

using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/**
 * Simple testcase for RateLimiter.SimpleRateLimiter
 */
class TestRateLimiter final : public LuceneTestCase
{
  GET_CLASS_NAME(TestRateLimiter)

  // LUCENE-6075
public:
  void testOverflowInt() ;

private:
  class ThreadAnonymousInnerClass : public Thread
  {
    GET_CLASS_NAME(ThreadAnonymousInnerClass)
  private:
    std::shared_ptr<TestRateLimiter> outerInstance;

  public:
    ThreadAnonymousInnerClass(std::shared_ptr<TestRateLimiter> outerInstance);

    void run() override;

  protected:
    std::shared_ptr<ThreadAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ThreadAnonymousInnerClass>(
          Thread::shared_from_this());
    }
  };

public:
  void testThreads() ;

private:
  class ThreadAnonymousInnerClass2 : public Thread
  {
    GET_CLASS_NAME(ThreadAnonymousInnerClass2)
  private:
    std::shared_ptr<TestRateLimiter> outerInstance;

    std::shared_ptr<SimpleRateLimiter> limiter;
    std::shared_ptr<CountDownLatch> startingGun;
    std::shared_ptr<AtomicLong> totBytes;
    int i = 0;

  public:
    ThreadAnonymousInnerClass2(std::shared_ptr<TestRateLimiter> outerInstance,
                               std::shared_ptr<SimpleRateLimiter> limiter,
                               std::shared_ptr<CountDownLatch> startingGun,
                               std::shared_ptr<AtomicLong> totBytes, int i);

    void run() override;

  protected:
    std::shared_ptr<ThreadAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<ThreadAnonymousInnerClass2>(
          Thread::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestRateLimiter> shared_from_this()
  {
    return std::static_pointer_cast<TestRateLimiter>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/store/
