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
 * <p>Base class for Locking implementation.  {@link Directory} uses
 * instances of this class to implement locking.</p>
 *
 * <p>Lucene uses {@link NativeFSLockFactory} by default for
 * {@link FSDirectory}-based index directories.</p>
 *
 * <p>Special care needs to be taken if you change the locking
 * implementation: First be certain that no writer is in fact
 * writing to the index otherwise you can easily corrupt
 * your index. Be sure to do the LockFactory change on all Lucene
 * instances and clean up all leftover lock files before starting
 * the new configuration for the first time. Different implementations
 * can not work together!</p>
 *
 * <p>If you suspect that some LockFactory implementation is
 * not working properly in your environment, you can easily
 * test it by using {@link VerifyingLockFactory}, {@link
 * LockVerifyServer} and {@link LockStressTest}.</p>
 *
 * @see LockVerifyServer
 * @see LockStressTest
 * @see VerifyingLockFactory
 */

class LockFactory : public std::enable_shared_from_this<LockFactory>
{
  GET_CLASS_NAME(LockFactory)

  /**
   * Return a new obtained Lock instance identified by lockName.
   * @param lockName name of the lock to be created.
   * @throws LockObtainFailedException (optional specific exception) if the lock
   * could not be obtained because it is currently held elsewhere.
   * @throws IOException if any i/o error occurs attempting to gain the lock
   */
public:
  virtual std::shared_ptr<Lock> obtainLock(std::shared_ptr<Directory> dir,
                                           const std::wstring &lockName) = 0;
};

} // namespace org::apache::lucene::store
