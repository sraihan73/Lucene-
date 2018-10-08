#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/mockfile/FilterFileSystemProvider.h"

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
namespace org::apache::lucene::mockfile
{

/**
 * A {@code FilterFileSystem} contains another
 * {@code FileSystem}, which it uses as its basic
 * source of data, possibly transforming the data along the
 * way or providing additional functionality.
 */
class FilterFileSystem : public FileSystem
{
  GET_CLASS_NAME(FilterFileSystem)

  /**
   * FileSystemProvider that created this FilterFileSystem
   */
protected:
  const std::shared_ptr<FilterFileSystemProvider> parent;

  /**
   * The underlying {@code FileSystem} instance.
   */
  const std::shared_ptr<FileSystem> delegate_;

  /**
   * Construct a {@code FilterFileSystem} based on
   * the specified base filesystem.
   * <p>
   * Note that base filesystem is closed if this filesystem is closed,
   * however the default filesystem provider will never be closed, it doesn't
   * support that.
   * @param delegate specified base channel.
   */
public:
  FilterFileSystem(std::shared_ptr<FilterFileSystemProvider> parent,
                   std::shared_ptr<FileSystem> delegate_);

  std::shared_ptr<FileSystemProvider> provider() override;

  virtual ~FilterFileSystem();

  bool isOpen() override;

  bool isReadOnly() override;

  std::wstring getSeparator() override;

  std::deque<std::shared_ptr<Path>> getRootDirectories() override;

private:
  class IteratorAnonymousInnerClass
      : public std::enable_shared_from_this<IteratorAnonymousInnerClass>,
        public Iterator<std::shared_ptr<Path>>
  {
    GET_CLASS_NAME(IteratorAnonymousInnerClass)
  private:
    std::shared_ptr<FilterFileSystem> outerInstance;

    std::deque<std::shared_ptr<Path>>::const_iterator iterator;

  public:
    IteratorAnonymousInnerClass(std::shared_ptr<FilterFileSystem> outerInstance,
                                std::deque<Path>::const_iterator iterator);

    bool hasNext();

    std::shared_ptr<Path> next();

    void remove();
  };

public:
  std::deque<std::shared_ptr<FileStore>> getFileStores() override;

private:
  class IteratorAnonymousInnerClass2
      : public std::enable_shared_from_this<IteratorAnonymousInnerClass2>,
        public Iterator<std::shared_ptr<FileStore>>
  {
    GET_CLASS_NAME(IteratorAnonymousInnerClass2)
  private:
    std::shared_ptr<FilterFileSystem> outerInstance;

    std::deque<std::shared_ptr<FileStore>>::const_iterator iterator;

  public:
    IteratorAnonymousInnerClass2(
        std::shared_ptr<FilterFileSystem> outerInstance,
        std::deque<FileStore>::const_iterator iterator);

    bool hasNext();

    std::shared_ptr<FileStore> next();

  private:
    class FilterFileStoreAnonymousInnerClass : public FilterFileStore
    {
      GET_CLASS_NAME(FilterFileStoreAnonymousInnerClass)
    private:
      std::shared_ptr<IteratorAnonymousInnerClass2> outerInstance;

    public:
      FilterFileStoreAnonymousInnerClass(
          std::shared_ptr<IteratorAnonymousInnerClass2> outerInstance,
          std::shared_ptr<UnknownType> next, const std::wstring &getScheme);

    protected:
      std::shared_ptr<FilterFileStoreAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<FilterFileStoreAnonymousInnerClass>(
            FilterFileStore::shared_from_this());
      }
    };

  public:
    void remove();
  };

public:
  std::shared_ptr<Set<std::wstring>> supportedFileAttributeViews() override;

  std::shared_ptr<Path> getPath(const std::wstring &first,
                                std::deque<std::wstring> &more) override;

  std::shared_ptr<PathMatcher>
  getPathMatcher(const std::wstring &syntaxAndPattern) override;

  std::shared_ptr<UserPrincipalLookupService>
  getUserPrincipalLookupService() override;

  std::shared_ptr<WatchService> newWatchService()  override;

  /** Returns the {@code FileSystem} we wrap. */
  virtual std::shared_ptr<FileSystem> getDelegate();

  /** Returns the {@code FilterFileSystemProvider} sent to this on init. */
  virtual std::shared_ptr<FileSystemProvider> getParent();

protected:
  std::shared_ptr<FilterFileSystem> shared_from_this()
  {
    return std::static_pointer_cast<FilterFileSystem>(
        java.nio.file.FileSystem::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/mockfile/
