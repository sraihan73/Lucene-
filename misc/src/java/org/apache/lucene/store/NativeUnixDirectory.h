#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/Directory.h"

#include  "core/src/java/org/apache/lucene/store/LockFactory.h"
#include  "core/src/java/org/apache/lucene/store/IndexInput.h"
#include  "core/src/java/org/apache/lucene/store/IOContext.h"
#include  "core/src/java/org/apache/lucene/store/IndexOutput.h"

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

using Directory = org::apache::lucene::store::Directory; // javadoc

// TODO
//   - newer Linux kernel versions (after 2.6.29) have
//     improved MADV_SEQUENTIAL (and hopefully also
//     FADV_SEQUENTIAL) interaction with the buffer
//     cache; we should explore using that instead of direct
//     IO when context is merge

/**
 * A {@link Directory} implementation for all Unixes that uses
 * DIRECT I/O to bypass OS level IO caching during
 * merging.  For all other cases (searching, writing) we delegate
 * to the provided Directory instance.
 *
 * <p>See <a
 * href="{@docRoot}/overview-summary.html#NativeUnixDirectory">Overview</a>
 * for more details.
 *
 * <p>To use this you must compile
 * NativePosixUtil.cpp (exposes Linux-specific APIs through
 * JNI) for your platform, by running <code>ant
 * build-native-unix</code>, and then putting the resulting
 * <code>libNativePosixUtil.so</code> (from
 * <code>lucene/build/native</code>) onto your dynamic
 * linker search path.
 *
 * <p><b>WARNING</b>: this code is very new and quite easily
 * could contain horrible bugs.  For example, here's one
 * known issue: if you use seek in <code>IndexOutput</code>, and then
 * write more than one buffer's worth of bytes, then the
 * file will be wrong.  Lucene does not do this today (only writes
 * small number of bytes after seek), but that may change.
 *
 * <p>This directory passes Solr and Lucene tests on Linux
 * and OS X; other Unixes should work but have not been
 * tested!  Use at your own risk.
 *
 * @lucene.experimental
 */
class NativeUnixDirectory : public FSDirectory
{
  GET_CLASS_NAME(NativeUnixDirectory)

  // TODO: this is OS dependent, but likely 512 is the LCD
private:
  static constexpr int64_t ALIGN = 512;
  static const int64_t ALIGN_NOT_MASK = ~(ALIGN - 1);

  /** Default buffer size before writing to disk (256 KB);
   *  larger means less IO load but more RAM and direct
   *  buffer storage space consumed during merging. */

public:
  static constexpr int DEFAULT_MERGE_BUFFER_SIZE = 262144;

  /** Default min expected merge size before direct IO is
   *  used (10 MB): */
  static constexpr int64_t DEFAULT_MIN_BYTES_DIRECT = 10 * 1024 * 1024;

private:
  const int mergeBufferSize;
  const int64_t minBytesDirect;
  const std::shared_ptr<Directory> delegate_;

  /** Create a new NIOFSDirectory for the named location.
   *
   * @param path the path of the directory
   * @param lockFactory to use
   * @param mergeBufferSize Size of buffer to use for
   *    merging.  See {@link #DEFAULT_MERGE_BUFFER_SIZE}.
   * @param minBytesDirect Merges, or files to be opened for
   *   reading, smaller than this will
   *   not use direct IO.  See {@link
   *   #DEFAULT_MIN_BYTES_DIRECT}
   * @param delegate fallback Directory for non-merges
   * @throws IOException If there is a low-level I/O error
   */
public:
  NativeUnixDirectory(std::shared_ptr<Path> path, int mergeBufferSize,
                      int64_t minBytesDirect,
                      std::shared_ptr<LockFactory> lockFactory,
                      std::shared_ptr<Directory> delegate_) ;

  /** Create a new NIOFSDirectory for the named location.
   *
   * @param path the path of the directory
   * @param lockFactory the lock factory to use
   * @param delegate fallback Directory for non-merges
   * @throws IOException If there is a low-level I/O error
   */
  NativeUnixDirectory(std::shared_ptr<Path> path,
                      std::shared_ptr<LockFactory> lockFactory,
                      std::shared_ptr<Directory> delegate_) ;

  /** Create a new NIOFSDirectory for the named location with {@link
   * FSLockFactory#getDefault()}.
   *
   * @param path the path of the directory
   * @param delegate fallback Directory for non-merges
   * @throws IOException If there is a low-level I/O error
   */
  NativeUnixDirectory(std::shared_ptr<Path> path,
                      std::shared_ptr<Directory> delegate_) ;

  std::shared_ptr<IndexInput>
  openInput(const std::wstring &name,
            std::shared_ptr<IOContext> context)  override;

  std::shared_ptr<IndexOutput>
  createOutput(const std::wstring &name,
               std::shared_ptr<IOContext> context)  override;

private:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressForbidden(reason = "java.io.File: native API
  // requires old-style FileDescriptor") private final static class
  // NativeUnixIndexOutput extends IndexOutput
  class NativeUnixIndexOutput final : public IndexOutput
  {
  private:
    const std::shared_ptr<ByteBuffer> buffer;
    const std::shared_ptr<FileOutputStream> fos;
    const std::shared_ptr<FileChannel> channel;
    const int bufferSize;

    // private final File path;

    int bufferPos = 0;
    int64_t filePos = 0;
    int64_t fileLength = 0;
    bool isOpen = false;

  public:
    NativeUnixIndexOutput(std::shared_ptr<Path> path, const std::wstring &name,
                          int bufferSize) ;

    void writeByte(char b)  override;

    void writeBytes(std::deque<char> &src, int offset,
                    int len)  override;

    //@Override
    // public void setLength() throws IOException {
    //   TODO -- how to impl this?  neither FOS nor
    //   FileChannel provides an API?
    //}

  private:
    void dump() ;

  public:
    int64_t getFilePointer() override;

    int64_t getChecksum()  override;

    virtual ~NativeUnixIndexOutput();

  protected:
    std::shared_ptr<NativeUnixIndexOutput> shared_from_this()
    {
      return std::static_pointer_cast<NativeUnixIndexOutput>(
          IndexOutput::shared_from_this());
    }
  };

private:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressForbidden(reason = "java.io.File: native API
  // requires old-style FileDescriptor") private final static class
  // NativeUnixIndexInput extends IndexInput
  class NativeUnixIndexInput final : public IndexInput
  {
  private:
    const std::shared_ptr<ByteBuffer> buffer;
    const std::shared_ptr<FileInputStream> fis;
    const std::shared_ptr<FileChannel> channel;
    const int bufferSize;

    bool isOpen = false;
    bool isClone = false;
    int64_t filePos = 0;
    int bufferPos = 0;

  public:
    NativeUnixIndexInput(std::shared_ptr<Path> path,
                         int bufferSize) ;

    // for clone
    NativeUnixIndexInput(std::shared_ptr<NativeUnixIndexInput> other) throw(
        IOException);

    virtual ~NativeUnixIndexInput();

    int64_t getFilePointer() override;

    void seek(int64_t pos)  override;

    int64_t length() override;

    char readByte()  override;

  private:
    void refill() ;

  public:
    void readBytes(std::deque<char> &dst, int offset,
                   int len)  override;

    std::shared_ptr<NativeUnixIndexInput> clone() override;

    std::shared_ptr<IndexInput>
    slice(const std::wstring &sliceDescription, int64_t offset,
          int64_t length)  override;

  protected:
    std::shared_ptr<NativeUnixIndexInput> shared_from_this()
    {
      return std::static_pointer_cast<NativeUnixIndexInput>(
          IndexInput::shared_from_this());
    }
  };

protected:
  std::shared_ptr<NativeUnixDirectory> shared_from_this()
  {
    return std::static_pointer_cast<NativeUnixDirectory>(
        FSDirectory::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/store/
