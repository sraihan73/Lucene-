#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/LockFactory.h"

#include  "core/src/java/org/apache/lucene/store/IndexInput.h"
#include  "core/src/java/org/apache/lucene/store/IOContext.h"

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

/** A straightforward implementation of {@link FSDirectory}
 *  using {@link Files#newByteChannel(Path, java.nio.file.OpenOption...)}.
 *  However, this class has
 *  poor concurrent performance (multiple threads will
 *  bottleneck) as it synchronizes when multiple threads
 *  read from the same file.  It's usually better to use
 *  {@link NIOFSDirectory} or {@link MMapDirectory} instead.
 * <p>
 * <b>NOTE:</b> Accessing this class either directly or
 * indirectly from a thread while it's interrupted can close the
 * underlying file descriptor immediately if at the same time the thread is
 * blocked on IO. The file descriptor will remain closed and subsequent access
 * to {@link SimpleFSDirectory} will throw a {@link ClosedChannelException}. If
 * your application uses either {@link Thread#interrupt()} or
 * {@link Future#cancel(bool)} you should use the legacy {@code RAFDirectory}
 * from the Lucene {@code misc} module in favor of {@link SimpleFSDirectory}.
 * </p>
 */
class SimpleFSDirectory : public FSDirectory
{
  GET_CLASS_NAME(SimpleFSDirectory)

  /** Create a new SimpleFSDirectory for the named location.
   *  The directory is created at the named location if it does not yet exist.
   *
   * @param path the path of the directory
   * @param lockFactory the lock factory to use
   * @throws IOException if there is a low-level I/O error
   */
public:
  SimpleFSDirectory(
      std::shared_ptr<Path> path,
      std::shared_ptr<LockFactory> lockFactory) ;

  /** Create a new SimpleFSDirectory for the named location and {@link
   * FSLockFactory#getDefault()}. The directory is created at the named location
   * if it does not yet exist.
   *
   * @param path the path of the directory
   * @throws IOException if there is a low-level I/O error
   */
  SimpleFSDirectory(std::shared_ptr<Path> path) ;

  /** Creates an IndexInput for the file with the given name. */
  std::shared_ptr<IndexInput>
  openInput(const std::wstring &name,
            std::shared_ptr<IOContext> context)  override;

  /**
   * Reads bytes with {@link SeekableByteChannel#read(ByteBuffer)}
   */
public:
  class SimpleFSIndexInput final : public BufferedIndexInput
  {
    GET_CLASS_NAME(SimpleFSIndexInput)
    /**
     * The maximum chunk size for reads of 16384 bytes.
     */
  private:
    static constexpr int CHUNK_SIZE = 16384;

    /** the channel we will read from */
  protected:
    const std::shared_ptr<SeekableByteChannel> channel;
    /** is this instance a clone and hence does not own the file to close it */
  public:
    bool isClone = false;
    /** start offset: non-zero in the slice case */
  protected:
    const int64_t off;
    /** end offset (start+length) */
    const int64_t end;

  private:
    std::shared_ptr<ByteBuffer> byteBuf; // wraps the buffer for NIO

  public:
    SimpleFSIndexInput(const std::wstring &resourceDesc,
                       std::shared_ptr<SeekableByteChannel> channel,
                       std::shared_ptr<IOContext> context) ;

    SimpleFSIndexInput(const std::wstring &resourceDesc,
                       std::shared_ptr<SeekableByteChannel> channel,
                       int64_t off, int64_t length, int bufferSize);

    virtual ~SimpleFSIndexInput();

    std::shared_ptr<SimpleFSIndexInput> clone() override;

    std::shared_ptr<IndexInput>
    slice(const std::wstring &sliceDescription, int64_t offset,
          int64_t length)  override;

    int64_t length() override final;

  protected:
    void newBuffer(std::deque<char> &newBuffer) override;

    void readInternal(std::deque<char> &b, int offset,
                      int len)  override;

    void seekInternal(int64_t pos)  override;

  protected:
    std::shared_ptr<SimpleFSIndexInput> shared_from_this()
    {
      return std::static_pointer_cast<SimpleFSIndexInput>(
          BufferedIndexInput::shared_from_this());
    }
  };

protected:
  std::shared_ptr<SimpleFSDirectory> shared_from_this()
  {
    return std::static_pointer_cast<SimpleFSDirectory>(
        FSDirectory::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/store/
