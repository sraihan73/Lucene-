#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <unordered_map>
#include <unordered_set>
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
namespace org::apache::lucene::util
{

using FilterFileSystemProvider =
    org::apache::lucene::mockfile::FilterFileSystemProvider;
using FilterPath = org::apache::lucene::mockfile::FilterPath;

/** Simple test methods for IOUtils */
class TestIOUtils : public LuceneTestCase
{
  GET_CLASS_NAME(TestIOUtils)

public:
  virtual void testDeleteFileIgnoringExceptions() ;

  virtual void testDontDeleteFileIgnoringExceptions() ;

  virtual void testDeleteTwoFilesIgnoringExceptions() ;

  virtual void testDeleteFileIfExists() ;

  virtual void testDontDeleteDoesntExist() ;

  virtual void testDeleteTwoFilesIfExist() ;

  virtual void testSpinsBasics() ;

  // fake up a filestore to test some underlying methods
public:
  class MockFileStore : public FileStore
  {
    GET_CLASS_NAME(MockFileStore)
  public:
    const std::wstring description;
    // C++ NOTE: Fields cannot have the same name as methods:
    const std::wstring type_;
    // C++ NOTE: Fields cannot have the same name as methods:
    const std::wstring name_;

    MockFileStore(const std::wstring &description, const std::wstring &type,
                  const std::wstring &name);

    std::wstring type() override;

    std::wstring name() override;

    virtual std::wstring toString();

    // TODO: we can enable mocking of these when we need them later:

    bool isReadOnly() override;

    int64_t getTotalSpace()  override;

    int64_t getUsableSpace()  override;

    int64_t getUnallocatedSpace()  override;

    bool supportsFileAttributeView(std::type_info type) override;

    bool supportsFileAttributeView(const std::wstring &name) override;

    template <typename V>
    V getFileStoreAttributeView(std::type_info<V> &type);

    std::any
    getAttribute(const std::wstring &attribute)  override;

  protected:
    std::shared_ptr<MockFileStore> shared_from_this()
    {
      return std::static_pointer_cast<MockFileStore>(
          java.nio.file.FileStore::shared_from_this());
    }
  };

public:
  virtual void testGetMountPoint() ;

  /** mock linux that takes mappings of test files, to their associated
   * filesystems. it will chroot /dev and /sys requests to root, so you can mock
   * those too. <p> It is hacky by definition, so don't try putting it around a
   * complex chain or anything.
   */
public:
  class MockLinuxFileSystemProvider : public FilterFileSystemProvider
  {
    GET_CLASS_NAME(MockLinuxFileSystemProvider)
  public:
    const std::unordered_map<std::wstring, std::shared_ptr<FileStore>>
        filesToStore;
    const std::shared_ptr<Path> root;

    MockLinuxFileSystemProvider(
        std::shared_ptr<FileSystem> delegateInstance,
        std::unordered_map<std::wstring, std::shared_ptr<FileStore>>
            &filesToStore,
        std::shared_ptr<Path> root);

  private:
    class FilterFileSystemAnonymousInnerClass : public FilterFileSystem
    {
      GET_CLASS_NAME(FilterFileSystemAnonymousInnerClass)
    private:
      std::shared_ptr<MockLinuxFileSystemProvider> outerInstance;

      std::shared_ptr<FileSystem> delegateInstance;
      std::shared_ptr<std::deque<std::shared_ptr<FileStore>>> allStores;

    public:
      FilterFileSystemAnonymousInnerClass(
          std::shared_ptr<MockLinuxFileSystemProvider> outerInstance,
          std::shared_ptr<FileSystem> delegateInstance,
          std::shared_ptr<std::deque<std::shared_ptr<FileStore>>> allStores);

      std::deque<std::shared_ptr<FileStore>> getFileStores() override;

      std::shared_ptr<Path> getPath(const std::wstring &first,
                                    std::deque<std::wstring> &more) override;

    protected:
      std::shared_ptr<FilterFileSystemAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<FilterFileSystemAnonymousInnerClass>(
            org.apache.lucene.mockfile.FilterFileSystem::shared_from_this());
      }
    };

  public:
    std::shared_ptr<FileStore>
    getFileStore(std::shared_ptr<Path> path)  override;

    virtual std::shared_ptr<Path> maybeChroot(std::shared_ptr<Path> path);

  protected:
    std::shared_ptr<Path> toDelegate(std::shared_ptr<Path> path) override;

  public:
    class MockLinuxPath : public FilterPath
    {
      GET_CLASS_NAME(MockLinuxPath)
    private:
      std::shared_ptr<TestIOUtils::MockLinuxFileSystemProvider> outerInstance;

    public:
      MockLinuxPath(std::shared_ptr<TestIOUtils::MockLinuxFileSystemProvider>
                        outerInstance,
                    std::shared_ptr<Path> delegate_,
                    std::shared_ptr<FileSystem> fileSystem);

      std::shared_ptr<Path>
      toRealPath(std::deque<LinkOption> &options)  override;

    protected:
      std::shared_ptr<Path> wrap(std::shared_ptr<Path> other) override;

    protected:
      std::shared_ptr<MockLinuxPath> shared_from_this()
      {
        return std::static_pointer_cast<MockLinuxPath>(
            org.apache.lucene.mockfile.FilterPath::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<MockLinuxFileSystemProvider> shared_from_this()
    {
      return std::static_pointer_cast<MockLinuxFileSystemProvider>(
          org.apache.lucene.mockfile
              .FilterFileSystemProvider::shared_from_this());
    }
  };

public:
  virtual void testGetFileStore() ;

  virtual void testTmpfsDoesntSpin() ;

  virtual void testNfsSpins() ;

  virtual void testSSD() ;

  virtual void testNVME() ;

  virtual void testRotatingPlatters() ;

  virtual void testManyPartitions() ;

  virtual void testSymlinkSSD() ;

  virtual void testFsyncDirectory() ;

  virtual void testFsyncFile() ;

  virtual void testApplyToAll();

protected:
  std::shared_ptr<TestIOUtils> shared_from_this()
  {
    return std::static_pointer_cast<TestIOUtils>(
        LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/util/
