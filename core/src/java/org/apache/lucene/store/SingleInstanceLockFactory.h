#pragma once
#include "stringhelper.h"
#include <algorithm>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_set>

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
 * Implements {@link LockFactory} for a single in-process instance,
 * meaning all locking will take place through this one instance.
 * Only use this {@link LockFactory} when you are certain all
 * IndexWriters for a given index are running
 * against a single shared in-process Directory instance.  This is
 * currently the default locking for RAMDirectory.
 *
 * @see LockFactory
 */

class SingleInstanceLockFactory final : public LockFactory
{
  GET_CLASS_NAME(SingleInstanceLockFactory)

public:
  const std::unordered_set<std::wstring> locks =
      std::unordered_set<std::wstring>();

  std::shared_ptr<Lock>
  obtainLock(std::shared_ptr<Directory> dir,
             const std::wstring &lockName)  override;

private:
  class SingleInstanceLock : public Lock
  {
    GET_CLASS_NAME(SingleInstanceLock)
  private:
    std::shared_ptr<SingleInstanceLockFactory> outerInstance;

    const std::wstring lockName;
    // C++ TODO: 'volatile' has a different meaning in C++:
    // ORIGINAL LINE: private volatile bool closed;
    bool closed = false;

  public:
    SingleInstanceLock(std::shared_ptr<SingleInstanceLockFactory> outerInstance,
                       const std::wstring &lockName);

    void ensureValid()  override;

    // C++ WARNING: The following method was originally marked 'synchronized':
    virtual ~SingleInstanceLock();

    virtual std::wstring toString();

  protected:
    std::shared_ptr<SingleInstanceLock> shared_from_this()
    {
      return std::static_pointer_cast<SingleInstanceLock>(
          Lock::shared_from_this());
    }
  };

protected:
  std::shared_ptr<SingleInstanceLockFactory> shared_from_this()
  {
    return std::static_pointer_cast<SingleInstanceLockFactory>(
        LockFactory::shared_from_this());
  }
};

} // namespace org::apache::lucene::store
