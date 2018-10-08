#pragma once
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
 * Base class for tracking file handles.
 * <p>
 * This class adds tracking to all streams/channels and
 * provides two hooks to handle file management:
 * <ul>
 *   <li>{@link #onOpen(Path, Object)}
 *   <li>{@link #onClose(Path, Object)}
 * </ul>
 */
class HandleTrackingFS : public FilterFileSystemProvider
{
  GET_CLASS_NAME(HandleTrackingFS)

  /**
   * Create a new instance, identified by {@code scheme} and passing
   * through operations to {@code delegate}.
   * @param scheme URI scheme for this provider
   * @param delegate delegate filesystem to wrap.
   */
public:
  HandleTrackingFS(const std::wstring &scheme,
                   std::shared_ptr<FileSystem> delegate_);

  /**
   * Called when {@code path} is opened via {@code stream}.
   * @param path Path that was opened
   * @param stream Stream or Channel opened against the path.
   * @throws IOException if an I/O error occurs.
   */
protected:
  virtual void onOpen(std::shared_ptr<Path> path, std::any stream) = 0;

  /**
   * Called when {@code path} is closed via {@code stream}.
   * @param path Path that was closed
   * @param stream Stream or Channel closed against the path.
   * @throws IOException if an I/O error occurs.
   */
  virtual void onClose(std::shared_ptr<Path> path, std::any stream) = 0;

  /**
   * Helper method, to deal with onOpen() throwing exception
   */
public:
  void callOpenHook(std::shared_ptr<Path> path,
                    std::shared_ptr<Closeable> stream) ;

  std::shared_ptr<InputStream>
  newInputStream(std::shared_ptr<Path> path,
                 std::deque<OpenOption> &options)  override;

private:
  class FilterInputStream2AnonymousInnerClass : public FilterInputStream2
  {
    GET_CLASS_NAME(FilterInputStream2AnonymousInnerClass)
  private:
    std::shared_ptr<HandleTrackingFS> outerInstance;

    std::shared_ptr<Path> path;

  public:
    FilterInputStream2AnonymousInnerClass(
        std::shared_ptr<HandleTrackingFS> outerInstance,
        std::shared_ptr<InputStream> newInputStream,
        std::shared_ptr<Path> path);

    bool closed = false;

    virtual ~FilterInputStream2AnonymousInnerClass();

    virtual std::wstring toString();

    virtual int hashCode();

    bool equals(std::any obj) override;

  protected:
    std::shared_ptr<FilterInputStream2AnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<FilterInputStream2AnonymousInnerClass>(
          FilterInputStream2::shared_from_this());
    }
  };

public:
  std::shared_ptr<OutputStream>
  newOutputStream(std::shared_ptr<Path> path,
                  std::deque<OpenOption> &options)  override;

private:
  class FilterOutputStream2AnonymousInnerClass : public FilterOutputStream2
  {
    GET_CLASS_NAME(FilterOutputStream2AnonymousInnerClass)
  private:
    std::shared_ptr<HandleTrackingFS> outerInstance;

    std::shared_ptr<Path> path;

  public:
    FilterOutputStream2AnonymousInnerClass(
        std::shared_ptr<HandleTrackingFS> outerInstance,
        std::shared_ptr<UnknownType> newOutputStream,
        std::shared_ptr<Path> path);

    bool closed = false;

    virtual ~FilterOutputStream2AnonymousInnerClass();

    virtual std::wstring toString();

    virtual int hashCode();

    bool equals(std::any obj) override;

  protected:
    std::shared_ptr<FilterOutputStream2AnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<FilterOutputStream2AnonymousInnerClass>(
          FilterOutputStream2::shared_from_this());
    }
  };

public:
      template<typename T1>
//C++ TODO: There is no native C++ template equivalent to this generic constraint:
//ORIGINAL LINE: @Override public java.nio.channels.FileChannel newFileChannel(java.nio.file.Path path, std::unordered_set<? extends java.nio.file.OpenOption> options, java.nio.file.attribute.FileAttribute<?>... attrs) throws java.io.IOException
//C++ TODO: Java wildcard generics are not converted to C++:
      std::shared_ptr<FileChannel> newFileChannel(std::shared_ptr<Path> path, std::shared_ptr<Set<T1>> options, std::deque<FileAttribute<?>> &attrs) ;

    private:
      class FilterFileChannelAnonymousInnerClass : public FilterFileChannel
      {
        GET_CLASS_NAME(FilterFileChannelAnonymousInnerClass)
      private:
        std::shared_ptr<HandleTrackingFS> outerInstance;

        std::shared_ptr<Path> path;

      public:
        FilterFileChannelAnonymousInnerClass(
            std::shared_ptr<HandleTrackingFS> outerInstance,
            std::shared_ptr<UnknownType> newFileChannel,
            std::shared_ptr<Path> path);

        bool closed = false;

      protected:
        void implCloseChannel()  override;

      public:
        virtual std::wstring toString();

        virtual int hashCode();

        bool equals(std::any obj) override;

      protected:
        std::shared_ptr<FilterFileChannelAnonymousInnerClass> shared_from_this()
        {
          return std::static_pointer_cast<FilterFileChannelAnonymousInnerClass>(
              FilterFileChannel::shared_from_this());
        }
      };

    public:
      template<typename T1>
//C++ TODO: There is no native C++ template equivalent to this generic constraint:
//ORIGINAL LINE: @Override public java.nio.channels.AsynchronousFileChannel newAsynchronousFileChannel(java.nio.file.Path path, std::unordered_set<? extends java.nio.file.OpenOption> options, java.util.concurrent.ExecutorService executor, java.nio.file.attribute.FileAttribute<?>... attrs) throws java.io.IOException
//C++ TODO: Java wildcard generics are not converted to C++:
      std::shared_ptr<AsynchronousFileChannel> newAsynchronousFileChannel(std::shared_ptr<Path> path, std::shared_ptr<Set<T1>> options, std::shared_ptr<ExecutorService> executor, std::deque<FileAttribute<?>> &attrs) ;

    private:
      class FilterAsynchronousFileChannelAnonymousInnerClass
          : public FilterAsynchronousFileChannel
      {
        GET_CLASS_NAME(FilterAsynchronousFileChannelAnonymousInnerClass)
      private:
        std::shared_ptr<HandleTrackingFS> outerInstance;

        std::shared_ptr<Path> path;

      public:
        FilterAsynchronousFileChannelAnonymousInnerClass(
            std::shared_ptr<HandleTrackingFS> outerInstance,
            std::shared_ptr<AsynchronousFileChannel> newAsynchronousFileChannel,
            std::shared_ptr<Path> path);

        bool closed = false;

        virtual ~FilterAsynchronousFileChannelAnonymousInnerClass();

        virtual std::wstring toString();

        virtual int hashCode();

        bool equals(std::any obj) override;

      protected:
        std::shared_ptr<FilterAsynchronousFileChannelAnonymousInnerClass>
        shared_from_this()
        {
          return std::static_pointer_cast<
              FilterAsynchronousFileChannelAnonymousInnerClass>(
              FilterAsynchronousFileChannel::shared_from_this());
        }
      };

    public:
      template<typename T1>
//C++ TODO: There is no native C++ template equivalent to this generic constraint:
//ORIGINAL LINE: @Override public java.nio.channels.SeekableByteChannel newByteChannel(java.nio.file.Path path, std::unordered_set<? extends java.nio.file.OpenOption> options, java.nio.file.attribute.FileAttribute<?>... attrs) throws java.io.IOException
//C++ TODO: Java wildcard generics are not converted to C++:
      std::shared_ptr<SeekableByteChannel> newByteChannel(std::shared_ptr<Path> path, std::shared_ptr<Set<T1>> options, std::deque<FileAttribute<?>> &attrs) ;

    private:
      class FilterSeekableByteChannelAnonymousInnerClass
          : public FilterSeekableByteChannel
      {
        GET_CLASS_NAME(FilterSeekableByteChannelAnonymousInnerClass)
      private:
        std::shared_ptr<HandleTrackingFS> outerInstance;

        std::shared_ptr<Path> path;

      public:
        FilterSeekableByteChannelAnonymousInnerClass(
            std::shared_ptr<HandleTrackingFS> outerInstance,
            std::shared_ptr<SeekableByteChannel> newByteChannel,
            std::shared_ptr<Path> path);

        bool closed = false;

        virtual ~FilterSeekableByteChannelAnonymousInnerClass();

        virtual std::wstring toString();

        virtual int hashCode();

        bool equals(std::any obj) override;

      protected:
        std::shared_ptr<FilterSeekableByteChannelAnonymousInnerClass>
        shared_from_this()
        {
          return std::static_pointer_cast<
              FilterSeekableByteChannelAnonymousInnerClass>(
              FilterSeekableByteChannel::shared_from_this());
        }
      };

    public:
      template <typename T1>
      // C++ TODO: There is no C++ equivalent to the Java 'super' constraint:
      // ORIGINAL LINE: @Override public
      // java.nio.file.DirectoryStream<java.nio.file.Path>
      // newDirectoryStream(java.nio.file.Path dir,
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
        std::shared_ptr<HandleTrackingFS> outerInstance;

        // C++ TODO: Java wildcard generics are not converted to C++:
        // ORIGINAL LINE: private java.nio.file.DirectoryStream.Filter<? super
        // java.nio.file.Path> filter;
        std::shared_ptr < DirectoryStream::Filter <
            ? DirectoryStream::Filter<Path> Path >> filter;

      public:
        template <typename T1>
        // C++ TODO: There is no C++ equivalent to the Java 'super' constraint:
        // ORIGINAL LINE: public FilterAnonymousInnerClass(HandleTrackingFS
        // outerInstance, java.nio.file.DirectoryStream.Filter<? super
        // java.nio.file.Path> filter)
        FilterAnonymousInnerClass(
            std::shared_ptr<HandleTrackingFS> outerInstance,
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

    private:
      class FilterDirectoryStreamAnonymousInnerClass
          : public FilterDirectoryStream
      {
        GET_CLASS_NAME(FilterDirectoryStreamAnonymousInnerClass)
      private:
        std::shared_ptr<HandleTrackingFS> outerInstance;

        std::shared_ptr<Path> dir;

      public:
        FilterDirectoryStreamAnonymousInnerClass(
            std::shared_ptr<HandleTrackingFS> outerInstance,
            std::shared_ptr<DirectoryStream<std::shared_ptr<Path>>> stream,
            std::shared_ptr<FileSystem> fileSystem, std::shared_ptr<Path> dir);

        bool closed = false;

        virtual ~FilterDirectoryStreamAnonymousInnerClass();

        virtual std::wstring toString();

        virtual int hashCode();

        bool equals(std::any obj) override;

      protected:
        std::shared_ptr<FilterDirectoryStreamAnonymousInnerClass>
        shared_from_this()
        {
          return std::static_pointer_cast<
              FilterDirectoryStreamAnonymousInnerClass>(
              FilterDirectoryStream::shared_from_this());
        }
      };

    protected:
      std::shared_ptr<HandleTrackingFS> shared_from_this()
      {
        return std::static_pointer_cast<HandleTrackingFS>(
            FilterFileSystemProvider::shared_from_this());
      }
};

} // namespace org::apache::lucene::mockfile
