#pragma once
#include "stringhelper.h"
#include <memory>
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
namespace org::apache::lucene::store
{
class FSDirectory;
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
 * Base class for file system based locking implementation.
 * This class is explicitly checking that the passed {@link Directory}
GET_CLASS_NAME(for)
 * is an {@link FSDirectory}.
 */
class FSLockFactory : public LockFactory
{
  GET_CLASS_NAME(FSLockFactory)

  /** Returns the default locking implementation for this platform.
   * This method currently returns always {@link NativeFSLockFactory}.
   */
public:
  static std::shared_ptr<FSLockFactory> getDefault();

  std::shared_ptr<Lock>
  obtainLock(std::shared_ptr<Directory> dir,
             const std::wstring &lockName)  override final;

  /**
   * Implement this method to obtain a lock for a FSDirectory instance.
   * @throws IOException if the lock could not be obtained.
   */
protected:
  virtual std::shared_ptr<Lock> obtainFSLock(std::shared_ptr<FSDirectory> dir,
                                             const std::wstring &lockName) = 0;

protected:
  std::shared_ptr<FSLockFactory> shared_from_this()
  {
    return std::static_pointer_cast<FSLockFactory>(
        LockFactory::shared_from_this());
  }
};

} // namespace org::apache::lucene::store
