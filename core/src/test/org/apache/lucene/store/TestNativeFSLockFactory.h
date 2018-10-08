#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class Directory;
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

using FilterFileSystemProvider =
    org::apache::lucene::mockfile::FilterFileSystemProvider;

/** Simple tests for NativeFSLockFactory */
class TestNativeFSLockFactory : public BaseLockFactoryTestCase
{
  GET_CLASS_NAME(TestNativeFSLockFactory)

protected:
  std::shared_ptr<Directory>
  getDirectory(std::shared_ptr<Path> path)  override;

  /** Verify NativeFSLockFactory works correctly if the lock file exists */
public:
  virtual void testLockFileExists() ;

  /** release the lock and test ensureValid fails */
  virtual void testInvalidateLock() ;

  /** close the channel and test ensureValid fails */
  virtual void testInvalidateChannel() ;

  /** delete the lockfile and test ensureValid fails */
  virtual void testDeleteLockFile() ;

  /** MockFileSystem that throws AccessDeniedException on creating test.lock */
public:
  class MockBadPermissionsFileSystem : public FilterFileSystemProvider
  {
    GET_CLASS_NAME(MockBadPermissionsFileSystem)
  public:
    MockBadPermissionsFileSystem(std::shared_ptr<FileSystem> delegateInstance);

        template<typename T1>
//C++ TODO: There is no native C++ template equivalent to this generic constraint:
//ORIGINAL LINE: @Override public java.nio.channels.SeekableByteChannel newByteChannel(java.nio.file.Path path, std::unordered_set<? extends java.nio.file.OpenOption> options, java.nio.file.attribute.FileAttribute<?>... attrs) throws java.io.IOException
//C++ TODO: Java wildcard generics are not converted to C++:
        std::shared_ptr<SeekableByteChannel> newByteChannel(std::shared_ptr<Path> path, std::shared_ptr<Set<T1>> options, std::deque<FileAttribute<?>> &attrs) ;

      protected:
        std::shared_ptr<MockBadPermissionsFileSystem> shared_from_this()
        {
          return std::static_pointer_cast<MockBadPermissionsFileSystem>(
              org.apache.lucene.mockfile
                  .FilterFileSystemProvider::shared_from_this());
        }
  };

public:
  virtual void testBadPermissions() ;

protected:
  std::shared_ptr<TestNativeFSLockFactory> shared_from_this()
  {
    return std::static_pointer_cast<TestNativeFSLockFactory>(
        BaseLockFactoryTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::store
