#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <iostream>
#include <memory>
#include <mutex>
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
namespace org::apache::lucene::store
{

/**
 * Simple standalone server that must be running when you
 * use {@link VerifyingLockFactory}.  This server simply
 * verifies at most one process holds the lock at a time.
 * Run without any args to see usage.
 *
 * @see VerifyingLockFactory
 * @see LockStressTest
 */

class LockVerifyServer : public std::enable_shared_from_this<LockVerifyServer>
{
  GET_CLASS_NAME(LockVerifyServer)

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressForbidden(reason = "System.out required: command
  // line tool") public static void main(std::wstring[] args) throws Exception
  static void main(std::deque<std::wstring> &args) ;

private:
  class ThreadAnonymousInnerClass : public Thread
  {
    GET_CLASS_NAME(ThreadAnonymousInnerClass)
  private:
    std::any localLock;
    std::deque<int> lockedID;
    std::shared_ptr<CountDownLatch> startingGun;
    std::shared_ptr<Socket> cs;

  public:
    ThreadAnonymousInnerClass(std::any localLock, std::deque<int> &lockedID,
                              std::shared_ptr<CountDownLatch> startingGun,
                              std::shared_ptr<Socket> cs);

    void run() override;

  protected:
    std::shared_ptr<ThreadAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ThreadAnonymousInnerClass>(
          Thread::shared_from_this());
    }
  };
};

} // namespace org::apache::lucene::store
