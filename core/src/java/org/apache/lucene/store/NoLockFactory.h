#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class NoLock;
}

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
 * Use this {@link LockFactory} to disable locking entirely.
 * This is a singleton, you have to use {@link #INSTANCE}.
 *
 * @see LockFactory
 */

class NoLockFactory final : public LockFactory
{
  GET_CLASS_NAME(NoLockFactory)

  /** The singleton */
public:
  static const std::shared_ptr<NoLockFactory> INSTANCE;

  // visible for AssertingLock!
  static const std::shared_ptr<NoLock> SINGLETON_LOCK;

private:
  NoLockFactory();

public:
  std::shared_ptr<Lock> obtainLock(std::shared_ptr<Directory> dir,
                                   const std::wstring &lockName) override;

private:
  class NoLock : public Lock
  {
    GET_CLASS_NAME(NoLock)
  public:
    virtual ~NoLock();

    void ensureValid()  override;

    virtual std::wstring toString();

  protected:
    std::shared_ptr<NoLock> shared_from_this()
    {
      return std::static_pointer_cast<NoLock>(Lock::shared_from_this());
    }
  };

protected:
  std::shared_ptr<NoLockFactory> shared_from_this()
  {
    return std::static_pointer_cast<NoLockFactory>(
        LockFactory::shared_from_this());
  }
};

} // namespace org::apache::lucene::store
