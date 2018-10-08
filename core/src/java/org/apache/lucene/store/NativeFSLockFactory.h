#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
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
 * <p>Implements {@link LockFactory} using native OS file
 * locks.  Note that because this LockFactory relies on
 * java.nio.* APIs for locking, any problems with those APIs
 * will cause locking to fail.  Specifically, on certain NFS
 * environments the java.nio.* locks will fail (the lock can
 * incorrectly be double acquired) whereas {@link
 * SimpleFSLockFactory} worked perfectly in those same
 * environments.  For NFS based access to an index, it's
 * recommended that you try {@link SimpleFSLockFactory}
 * first and work around the one limitation that a lock file
 * could be left when the JVM exits abnormally.</p>
 *
 * <p>The primary benefit of {@link NativeFSLockFactory} is
 * that locks (not the lock file itsself) will be properly
 * removed (by the OS) if the JVM has an abnormal exit.</p>
 *
 * <p>Note that, unlike {@link SimpleFSLockFactory}, the existence of
 * leftover lock files in the filesystem is fine because the OS
 * will free the locks held against these files even though the
 * files still remain. Lucene will never actively remove the lock
 * files, so although you see them, the index may not be locked.</p>
 *
 * <p>Special care needs to be taken if you change the locking
 * implementation: First be certain that no writer is in fact
 * writing to the index otherwise you can easily corrupt
 * your index. Be sure to do the LockFactory change on all Lucene
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

class NativeFSLockFactory final : public FSLockFactory
{
  GET_CLASS_NAME(NativeFSLockFactory)

  /**
   * Singleton instance
   */
public:
  static const std::shared_ptr<NativeFSLockFactory> INSTANCE;

private:
  static const std::shared_ptr<Set<std::wstring>> LOCK_HELD;

  NativeFSLockFactory();

protected:
  std::shared_ptr<Lock>
  obtainFSLock(std::shared_ptr<FSDirectory> dir,
               const std::wstring &lockName)  override;

private:
  static void clearLockHeld(std::shared_ptr<Path> path) ;

  // TODO: kind of bogus we even pass channel:
  // FileLock has an accessor, but mockfs doesnt yet mock the locks, too scary
  // atm.

public:
  class NativeFSLock final : public Lock
  {
    GET_CLASS_NAME(NativeFSLock)
  public:
    const std::shared_ptr<FileLock> lock;
    const std::shared_ptr<FileChannel> channel;
    const std::shared_ptr<Path> path;
    const std::shared_ptr<FileTime> creationTime;
    // C++ TODO: 'volatile' has a different meaning in C++:
    // ORIGINAL LINE: volatile bool closed;
    bool closed = false;

    NativeFSLock(std::shared_ptr<FileLock> lock,
                 std::shared_ptr<FileChannel> channel,
                 std::shared_ptr<Path> path,
                 std::shared_ptr<FileTime> creationTime);

    void ensureValid()  override;

    // C++ WARNING: The following method was originally marked 'synchronized':
    virtual ~NativeFSLock();

    virtual std::wstring toString();

  protected:
    std::shared_ptr<NativeFSLock> shared_from_this()
    {
      return std::static_pointer_cast<NativeFSLock>(Lock::shared_from_this());
    }
  };

protected:
  std::shared_ptr<NativeFSLockFactory> shared_from_this()
  {
    return std::static_pointer_cast<NativeFSLockFactory>(
        FSLockFactory::shared_from_this());
  }
};

} // namespace org::apache::lucene::store
