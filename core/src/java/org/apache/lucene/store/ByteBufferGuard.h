#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
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
namespace org::apache::lucene::store
{

/**
 * A guard that is created for every {@link ByteBufferIndexInput} that tries on
 * best effort to reject any access to the {@link ByteBuffer} behind, once it is
 * unmapped. A single instance of this is used for the original and all clones,
 * so once the original is closed and unmapped all clones also throw {@link
 * AlreadyClosedException}, triggered by a {@link NullPointerException}. <p>
 * This code tries to hopefully flush any CPU caches using a store-store
 * barrier. It also yields the current thread to give other threads a chance to
 * finish in-flight requests...
 */
class ByteBufferGuard final
    : public std::enable_shared_from_this<ByteBufferGuard>
{
  GET_CLASS_NAME(ByteBufferGuard)

  /**
   * Pass in an implementation of this interface to cleanup ByteBuffers.
   * MMapDirectory implements this to allow unmapping of bytebuffers with
   * private Java APIs.
   */
  using BufferCleaner =
      std::function<void(std::wstring resourceDescription, ByteBuffer b)>;

private:
  const std::wstring resourceDescription;
  const BufferCleaner cleaner;

  /** Not volatile; see comments on visibility below! */
  bool invalidated = false;

  /** Used as a store-store barrier; see comments below! */
  const std::shared_ptr<AtomicInteger> barrier =
      std::make_shared<AtomicInteger>();

  /**
   * Creates an instance to be used for a single {@link ByteBufferIndexInput}
   * which must be shared by all of its clones.
   */
public:
  ByteBufferGuard(const std::wstring &resourceDescription,
                  BufferCleaner cleaner);

  /**
   * Invalidates this guard and unmaps (if supported).
   */
  void invalidateAndUnmap(std::deque<ByteBuffer> &bufs) ;

private:
  void ensureValid();

public:
  void getBytes(std::shared_ptr<ByteBuffer> receiver, std::deque<char> &dst,
                int offset, int length);

  char getByte(std::shared_ptr<ByteBuffer> receiver);

  short getShort(std::shared_ptr<ByteBuffer> receiver);

  int getInt(std::shared_ptr<ByteBuffer> receiver);

  int64_t getLong(std::shared_ptr<ByteBuffer> receiver);

  char getByte(std::shared_ptr<ByteBuffer> receiver, int pos);

  short getShort(std::shared_ptr<ByteBuffer> receiver, int pos);

  int getInt(std::shared_ptr<ByteBuffer> receiver, int pos);

  int64_t getLong(std::shared_ptr<ByteBuffer> receiver, int pos);
};

} // #include  "core/src/java/org/apache/lucene/store/
