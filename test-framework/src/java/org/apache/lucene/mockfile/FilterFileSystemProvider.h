#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <unordered_map>
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
 * A {@code FilterFileSystemProvider} contains another
 * {@code FileSystemProvider}, which it uses as its basic
 * source of data, possibly transforming the data along the
 * way or providing additional functionality.
 */
class FilterFileSystemProvider : public FileSystemProvider
{
  GET_CLASS_NAME(FilterFileSystemProvider)

  /**
   * The underlying {@code FileSystemProvider}.
   */
protected:
  const std::shared_ptr<FileSystemProvider> delegate_;
  /**
   * The underlying {@code FileSystem} instance.
   */
  std::shared_ptr<FileSystem> fileSystem;
  /**
   * The URI scheme for this provider.
   */
  const std::wstring scheme;

  /**
   * Construct a {@code FilterFileSystemProvider} indicated by
   * the specified {@code scheme} and wrapping functionality of the
   * provider of the specified base filesystem.
   * @param scheme URI scheme
   * @param delegateInstance specified base filesystem.
   */
public:
  FilterFileSystemProvider(const std::wstring &scheme,
                           std::shared_ptr<FileSystem> delegateInstance);

  /**
   * Construct a {@code FilterFileSystemProvider} indicated by
   * the specified {@code scheme} and wrapping functionality of the
   * provider. You must set the singleton {@code filesystem} yourself.
   * @param scheme URI scheme
   * @param delegate specified base provider.
   */
  FilterFileSystemProvider(const std::wstring &scheme,
                           std::shared_ptr<FileSystemProvider> delegate_);

  std::wstring getScheme() override;

  template <typename std::wstring, typename T1>
  std::shared_ptr<FileSystem>
  newFileSystem(std::shared_ptr<URI> uri,
                std::unordered_map<T1> env) ;

  template <typename std::wstring, typename T1>
  std::shared_ptr<FileSystem>
  newFileSystem(std::shared_ptr<Path> path,
                std::unordered_map<T1> env) ;

  std::shared_ptr<FileSystem> getFileSystem(std::shared_ptr<URI> uri) override;

  std::shared_ptr<Path> getPath(std::shared_ptr<URI> uri) override;

  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: @Override public void createDirectory(java.nio.file.Path
  // dir, java.nio.file.attribute.FileAttribute<?>... attrs) throws
  // java.io.IOException
      void createDirectory(std::shared_ptr<Path> dir, std::deque<FileAttribute<?>> &attrs)  override;

      void delete_(std::shared_ptr<Path> path)  override;

      void copy(std::shared_ptr<Path> source, std::shared_ptr<Path> target,
                std::deque<CopyOption> &options)  override;

      void move(std::shared_ptr<Path> source, std::shared_ptr<Path> target,
                std::deque<CopyOption> &options)  override;

      bool isSameFile(std::shared_ptr<Path> path,
                      std::shared_ptr<Path> path2)  override;

      bool isHidden(std::shared_ptr<Path> path)  override;

      std::shared_ptr<FileStore>
      getFileStore(std::shared_ptr<Path> path)  override;

      void
      checkAccess(std::shared_ptr<Path> path,
                  std::deque<AccessMode> &modes)  override;

      template <typename V>
      V getFileAttributeView(std::shared_ptr<Path> path,
                             std::type_info<V> &type,
                             std::deque<LinkOption> &options);

      template <typename A>
      A readAttributes(std::shared_ptr<Path> path, std::type_info<A> &type,
                       std::deque<LinkOption> &options) ;

      std::unordered_map<std::wstring, std::any> readAttributes(
          std::shared_ptr<Path> path, const std::wstring &attributes,
          std::deque<LinkOption> &options)  override;

      void setAttribute(
          std::shared_ptr<Path> path, const std::wstring &attribute,
          std::any value,
          std::deque<LinkOption> &options)  override;

      std::shared_ptr<InputStream> newInputStream(
          std::shared_ptr<Path> path,
          std::deque<OpenOption> &options)  override;

      std::shared_ptr<OutputStream> newOutputStream(
          std::shared_ptr<Path> path,
          std::deque<OpenOption> &options)  override;

      template<typename T1>
//C++ TODO: There is no native C++ template equivalent to this generic constraint:
//ORIGINAL LINE: @Override public java.nio.channels.FileChannel newFileChannel(java.nio.file.Path path, std::unordered_set<? extends java.nio.file.OpenOption> options, java.nio.file.attribute.FileAttribute<?>... attrs) throws java.io.IOException
//C++ TODO: Java wildcard generics are not converted to C++:
      std::shared_ptr<FileChannel> newFileChannel(std::shared_ptr<Path> path, std::shared_ptr<Set<T1>> options, std::deque<FileAttribute<?>> &attrs) ;

      template<typename T1>
//C++ TODO: There is no native C++ template equivalent to this generic constraint:
//ORIGINAL LINE: @Override public java.nio.channels.AsynchronousFileChannel newAsynchronousFileChannel(java.nio.file.Path path, std::unordered_set<? extends java.nio.file.OpenOption> options, java.util.concurrent.ExecutorService executor, java.nio.file.attribute.FileAttribute<?>... attrs) throws java.io.IOException
//C++ TODO: Java wildcard generics are not converted to C++:
      std::shared_ptr<AsynchronousFileChannel> newAsynchronousFileChannel(std::shared_ptr<Path> path, std::shared_ptr<Set<T1>> options, std::shared_ptr<ExecutorService> executor, std::deque<FileAttribute<?>> &attrs) ;

      template<typename T1>
//C++ TODO: There is no native C++ template equivalent to this generic constraint:
//ORIGINAL LINE: @Override public java.nio.channels.SeekableByteChannel newByteChannel(java.nio.file.Path path, std::unordered_set<? extends java.nio.file.OpenOption> options, java.nio.file.attribute.FileAttribute<?>... attrs) throws java.io.IOException
//C++ TODO: Java wildcard generics are not converted to C++:
      std::shared_ptr<SeekableByteChannel> newByteChannel(std::shared_ptr<Path> path, std::shared_ptr<Set<T1>> options, std::deque<FileAttribute<?>> &attrs) ;

      template <typename T1>
      // C++ TODO: There is no C++ equivalent to the Java 'super' constraint:
      // ORIGINAL LINE: @Override public
      // java.nio.file.DirectoryStream<java.nio.file.Path>
      // newDirectoryStream(java.nio.file.Path dir, final
      // java.nio.file.DirectoryStream.Filter<? super java.nio.file.Path> filter)
      // throws java.io.IOException
      std::shared_ptr<DirectoryStream<std::shared_ptr<Path>>>
      newDirectoryStream(std::shared_ptr<Path> dir,
                         std::shared_ptr<DirectoryStream::Filter<T1>>
                             filter) ;

    private:
      class FilterAnonymousInnerClass
          : public DirectoryStream::Filter<std::shared_ptr<Path>>
      {
        GET_CLASS_NAME(FilterAnonymousInnerClass)
      private:
        std::shared_ptr<FilterFileSystemProvider> outerInstance;

        // C++ TODO: Java wildcard generics are not converted to C++:
        // ORIGINAL LINE: private java.nio.file.DirectoryStream.Filter<? super
        // java.nio.file.Path> filter;
        std::shared_ptr < DirectoryStream::Filter <
            ? DirectoryStream::Filter<Path> Path >> filter;

      public:
        template <typename T1>
        // C++ TODO: There is no C++ equivalent to the Java 'super' constraint:
        // ORIGINAL LINE: public
        // FilterAnonymousInnerClass(FilterFileSystemProvider outerInstance,
        // java.nio.file.DirectoryStream.Filter<? super java.nio.file.Path>
        // filter)
        FilterAnonymousInnerClass(
            std::shared_ptr<FilterFileSystemProvider> outerInstance,
            std::shared_ptr<DirectoryStream::Filter<T1>> filter);

        bool accept(std::shared_ptr<Path> entry)  override;

      protected:
        std::shared_ptr<FilterAnonymousInnerClass> shared_from_this()
        {
          return std::static_pointer_cast<FilterAnonymousInnerClass>(
              java.nio.file.DirectoryStream
                  .Filter<java.nio.file.Path>::shared_from_this());
        }
      };

    public:
      // C++ TODO: Java wildcard generics are not converted to C++:
      // ORIGINAL LINE: @Override public void
      // createSymbolicLink(java.nio.file.Path link, java.nio.file.Path target,
      // java.nio.file.attribute.FileAttribute<?>... attrs) throws
      // java.io.IOException
      void createSymbolicLink(std::shared_ptr<Path> link, std::shared_ptr<Path> target, std::deque<FileAttribute<?>> &attrs)  override;

      void
      createLink(std::shared_ptr<Path> link,
                 std::shared_ptr<Path> existing)  override;

      bool
      deleteIfExists(std::shared_ptr<Path> path)  override;

      std::shared_ptr<Path>
      readSymbolicLink(std::shared_ptr<Path> link)  override;

    protected:
      virtual std::shared_ptr<Path> toDelegate(std::shared_ptr<Path> path);

      /**
       * Override to trigger some behavior when the filesystem is closed.
       * <p>
       * This is always called for each FilterFileSystemProvider in the chain.
       */
      virtual void onClose();

    public:
      virtual std::wstring toString();

    protected:
      std::shared_ptr<FilterFileSystemProvider> shared_from_this()
      {
        return std::static_pointer_cast<FilterFileSystemProvider>(
            java.nio.file.spi.FileSystemProvider::shared_from_this());
      }
};

} // namespace org::apache::lucene::mockfile
