#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class FSDirectory;
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
 * <p>Implements {@link LockFactory} using {@link
 * Files#createFile}.</p>
 *
 * <p>The main downside with using this API for locking is
 * that the Lucene write lock may not be released when
 * the JVM exits abnormally.</p>
 *
 * <p>When this happens, an {@link LockObtainFailedException}
 * is hit when trying to create a writer, in which case you may
 * need to explicitly clear the lock file first by
 * manually removing the file.  But, first be certain that
 * no writer is in fact writing to the index otherwise you
 * can easily corrupt your index.</p>
 *
 * <p>Special care needs to be taken if you change the locking
 * implementation: First be certain that no writer is in fact
 * writing to the index otherwise you can easily corrupt
 * your index. Be sure to do the LockFactory change all Lucene
 * instances and clean up all leftover lock files before starting
 * the new configuration for the first time. Different implementations
 * can not work together!</p>
 *
 * <p>If you suspect that this or any other LockFactory is
 * not working properly in your environment, you can easily
 * test it by using {@link VerifyingLockFactory}, {@link
 * LockVerifyServer} and {@link LockStressTest}.</p>
 *
 * <p>This is a singleton, you have to use {@link #INSTANCE}.
 *
 * @see LockFactory
 */

class SimpleFSLockFactory final : public FSLockFactory
{
  GET_CLASS_NAME(SimpleFSLockFactory)

  /**
   * Singleton instance
   */
public:
  static const std::shared_ptr<SimpleFSLockFactory> INSTANCE;

private:
  SimpleFSLockFactory();

protected:
  std::shared_ptr<Lock>
  obtainFSLock(std::shared_ptr<FSDirectory> dir,
               const std::wstring &lockName)  override;

public:
  class SimpleFSLock final : public Lock
  {
    GET_CLASS_NAME(SimpleFSLock)
  private:
    const std::shared_ptr<Path> path;
    const std::shared_ptr<FileTime> creationTime;
    // C++ TODO: 'volatile' has a different meaning in C++:
    // ORIGINAL LINE: private volatile bool closed;
    bool closed = false;

  public:
    SimpleFSLock(std::shared_ptr<Path> path,
                 std::shared_ptr<FileTime> creationTime) ;

    void ensureValid()  override;

    // C++ WARNING: The following method was originally marked 'synchronized':
    virtual ~SimpleFSLock();

    virtual std::wstring toString();

  protected:
    std::shared_ptr<SimpleFSLock> shared_from_this()
    {
      return std::static_pointer_cast<SimpleFSLock>(Lock::shared_from_this());
    }
  };

protected:
  std::shared_ptr<SimpleFSLockFactory> shared_from_this()
  {
    return std::static_pointer_cast<SimpleFSLockFactory>(
        FSLockFactory::shared_from_this());
  }
};

} // namespace org::apache::lucene::store
