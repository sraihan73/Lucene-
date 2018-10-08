#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/LockFactory.h"

#include  "core/src/java/org/apache/lucene/store/Lock.h"
#include  "core/src/java/org/apache/lucene/store/Directory.h"

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
 * A {@link LockFactory} that wraps another {@link
 * LockFactory} and verifies that each lock obtain/release
 * is "correct" (never results in two processes holding the
 * lock at the same time).  It does this by contacting an
 * external server ({@link LockVerifyServer}) to assert that
 * at most one process holds the lock at a time.  To use
 * this, you should also run {@link LockVerifyServer} on the
 * host and port matching what you pass to the constructor.
 *
 * @see LockVerifyServer
 * @see LockStressTest
 */

class VerifyingLockFactory final : public LockFactory
{
  GET_CLASS_NAME(VerifyingLockFactory)

public:
  const std::shared_ptr<LockFactory> lf;
  const std::shared_ptr<InputStream> in_;
  const std::shared_ptr<OutputStream> out;

private:
  class CheckedLock : public Lock
  {
    GET_CLASS_NAME(CheckedLock)
  private:
    std::shared_ptr<VerifyingLockFactory> outerInstance;

    const std::shared_ptr<Lock> lock;

  public:
    CheckedLock(std::shared_ptr<VerifyingLockFactory> outerInstance,
                std::shared_ptr<Lock> lock) ;

    void ensureValid()  override;

    virtual ~CheckedLock();

  private:
    void verify(char message) ;

  protected:
    std::shared_ptr<CheckedLock> shared_from_this()
    {
      return std::static_pointer_cast<CheckedLock>(Lock::shared_from_this());
    }
  };

  /**
   * @param lf the LockFactory that we are testing
   * @param in the socket's input to {@link LockVerifyServer}
   * @param out the socket's output to {@link LockVerifyServer}
   */
public:
  VerifyingLockFactory(std::shared_ptr<LockFactory> lf,
                       std::shared_ptr<InputStream> in_,
                       std::shared_ptr<OutputStream> out) ;

  std::shared_ptr<Lock>
  obtainLock(std::shared_ptr<Directory> dir,
             const std::wstring &lockName)  override;

protected:
  std::shared_ptr<VerifyingLockFactory> shared_from_this()
  {
    return std::static_pointer_cast<VerifyingLockFactory>(
        LockFactory::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/store/
