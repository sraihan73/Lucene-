#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>
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
namespace org::apache::lucene::mockfile
{

/**
 * A {@code FilterPath} contains another
 * {@code Path}, which it uses as its basic
 * source of data, possibly transforming the data along the
 * way or providing additional functionality.
 */
class FilterPath : public std::enable_shared_from_this<FilterPath>, public Path
{
  GET_CLASS_NAME(FilterPath)

  /**
   * The underlying {@code Path} instance.
   */
protected:
  const std::shared_ptr<Path> delegate_;

  /**
   * The parent {@code FileSystem} for this path.
   */
  const std::shared_ptr<FileSystem> fileSystem;

  /**
   * Construct a {@code FilterPath} with parent
   * {@code fileSystem}, based on the specified base path.
   * @param delegate specified base path.
   * @param fileSystem parent fileSystem.
   */
public:
  FilterPath(std::shared_ptr<Path> delegate_,
             std::shared_ptr<FileSystem> fileSystem);

  /**
   * Get the underlying wrapped path.
   * @return wrapped path.
   */
  virtual std::shared_ptr<Path> getDelegate();

  std::shared_ptr<FileSystem> getFileSystem() override;

  bool isAbsolute() override;

  std::shared_ptr<Path> getRoot() override;

  std::shared_ptr<Path> getFileName() override;

  std::shared_ptr<Path> getParent() override;

  int getNameCount() override;

  std::shared_ptr<Path> getName(int index) override;

  std::shared_ptr<Path> subpath(int beginIndex, int endIndex) override;

  bool startsWith(std::shared_ptr<Path> other) override;

  bool startsWith(const std::wstring &other) override;

  bool endsWith(std::shared_ptr<Path> other) override;

  bool endsWith(const std::wstring &other) override;

  std::shared_ptr<Path> normalize() override;

  std::shared_ptr<Path> resolve(std::shared_ptr<Path> other) override;

  std::shared_ptr<Path> resolve(const std::wstring &other) override;

  std::shared_ptr<Path> resolveSibling(std::shared_ptr<Path> other) override;

  std::shared_ptr<Path> resolveSibling(const std::wstring &other) override;

  std::shared_ptr<Path> relativize(std::shared_ptr<Path> other) override;

  // TODO: should these methods not expose delegate result directly?
  // it could allow code to "escape" the sandbox...

  std::shared_ptr<URI> toUri() override;

  virtual std::wstring toString();

  std::shared_ptr<Path> toAbsolutePath() override;

  std::shared_ptr<Path>
  toRealPath(std::deque<LinkOption> &options)  override;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Override @SuppressForbidden(reason = "Abstract API requires
  // to use java.io.File") public java.io.File toFile()
  std::shared_ptr<File> toFile() override;

  template <typename T1>
  std::shared_ptr<WatchKey>
  register_(std::shared_ptr<WatchService> watcher,
            std::deque<Kind<T1>> &events,
            std::deque<Modifier> &modifiers) ;

  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: @Override public java.nio.file.WatchKey
  // register(java.nio.file.WatchService watcher,
  // java.nio.file.WatchEvent.Kind<?>... events) throws java.io.IOException
      std::shared_ptr<WatchKey> register_(std::shared_ptr<WatchService> watcher, std::deque<Kind<?>> &events)  override;

      std::shared_ptr<Iterator<std::shared_ptr<Path>>> iterator() override;

    private:
      class IteratorAnonymousInnerClass
          : public std::enable_shared_from_this<IteratorAnonymousInnerClass>,
            public Iterator<std::shared_ptr<Path>>
      {
        GET_CLASS_NAME(IteratorAnonymousInnerClass)
      private:
        std::shared_ptr<FilterPath> outerInstance;

        std::shared_ptr<Path::const_iterator> iterator;

      public:
        IteratorAnonymousInnerClass(
            std::shared_ptr<FilterPath> outerInstance,
            std::shared_ptr<Path::const_iterator> iterator);

        bool hasNext();

        std::shared_ptr<Path> next();

        void remove();
      };

    public:
      int compareTo(std::shared_ptr<Path> other) override;

      virtual int hashCode();

      bool equals(std::any obj) override;

      /**
       * Unwraps all {@code FilterPath}s, returning
       * the innermost {@code Path}.
       * <p>
       * WARNING: this is exposed for testing only!
       * @param path specified path.
       * @return innermost Path instance
       */
      static std::shared_ptr<Path> unwrap(std::shared_ptr<Path> path);

      /** Override this to customize the return wrapped
       *  path from various operations */
    protected:
      virtual std::shared_ptr<Path> wrap(std::shared_ptr<Path> other);

      /** Override this to customize the unboxing of Path
       *  from various operations
       */
      virtual std::shared_ptr<Path> toDelegate(std::shared_ptr<Path> path);
};

} // #include  "core/src/java/org/apache/lucene/mockfile/
