#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class Directory;
}

namespace org::apache::lucene::store
{
class Lock;
}

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
 * Directory that wraps another, and that sleeps and retries
 * if obtaining the lock fails.
 * <p>
 * This is not a good idea.
 */
class SleepingLockWrapper final : public FilterDirectory
{
  GET_CLASS_NAME(SleepingLockWrapper)

  /**
   * Pass this lockWaitTimeout to try forever to obtain the lock.
   */
public:
  static constexpr int64_t LOCK_OBTAIN_WAIT_FOREVER = -1;

  /**
   * How long {@link #obtainLock} waits, in milliseconds,
   * in between attempts to acquire the lock.
   */
  static int64_t DEFAULT_POLL_INTERVAL;

private:
  const int64_t lockWaitTimeout;
  const int64_t pollInterval;

  /**
   * Create a new SleepingLockFactory
   * @param delegate        underlying directory to wrap
   * @param lockWaitTimeout length of time to wait in milliseconds
   *                        or {@link #LOCK_OBTAIN_WAIT_FOREVER} to retry
   * forever.
   */
public:
  SleepingLockWrapper(std::shared_ptr<Directory> delegate_,
                      int64_t lockWaitTimeout);

  /**
   * Create a new SleepingLockFactory
   * @param delegate        underlying directory to wrap
   * @param lockWaitTimeout length of time to wait in milliseconds
   *                        or {@link #LOCK_OBTAIN_WAIT_FOREVER} to retry
   * forever.
   * @param pollInterval    poll once per this interval in milliseconds until
   *                        {@code lockWaitTimeout} is exceeded.
   */
  SleepingLockWrapper(std::shared_ptr<Directory> delegate_,
                      int64_t lockWaitTimeout, int64_t pollInterval);

  std::shared_ptr<Lock>
  obtainLock(const std::wstring &lockName)  override;

  virtual std::wstring toString();

protected:
  std::shared_ptr<SleepingLockWrapper> shared_from_this()
  {
    return std::static_pointer_cast<SleepingLockWrapper>(
        FilterDirectory::shared_from_this());
  }
};

} // namespace org::apache::lucene::store
