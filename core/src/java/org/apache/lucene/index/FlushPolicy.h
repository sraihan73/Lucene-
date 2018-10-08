#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/LiveIndexWriterConfig.h"

#include  "core/src/java/org/apache/lucene/util/InfoStream.h"
#include  "core/src/java/org/apache/lucene/index/DocumentsWriterFlushControl.h"
#include  "core/src/java/org/apache/lucene/index/DocumentsWriterPerThreadPool.h"
#include  "core/src/java/org/apache/lucene/index/ThreadState.h"

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
namespace org::apache::lucene::index
{

using ThreadState =
    org::apache::lucene::index::DocumentsWriterPerThreadPool::ThreadState;
using InfoStream = org::apache::lucene::util::InfoStream;

/**
 * {@link FlushPolicy} controls when segments are flushed from a RAM resident
 * internal data-structure to the {@link IndexWriter}s {@link Directory}.
 * <p>
 * Segments are traditionally flushed by:
 * <ul>
 * <li>RAM consumption - configured via
 * {@link IndexWriterConfig#setRAMBufferSizeMB(double)}</li>
 * <li>Number of RAM resident documents - configured via
 * {@link IndexWriterConfig#setMaxBufferedDocs(int)}</li>
 * </ul>
 * <p>
 * {@link IndexWriter} consults the provided {@link FlushPolicy} to control the
 * flushing process. The policy is informed for each added or updated document
 * as well as for each delete term. Based on the {@link FlushPolicy}, the
 * information provided via {@link ThreadState} and
 * {@link DocumentsWriterFlushControl}, the {@link FlushPolicy} decides if a
 * {@link DocumentsWriterPerThread} needs flushing and mark it as flush-pending
 * via {@link DocumentsWriterFlushControl#setFlushPending}, or if deletes need
 * to be applied.
 *
 * @see ThreadState
 * @see DocumentsWriterFlushControl
 * @see DocumentsWriterPerThread
 * @see IndexWriterConfig#setFlushPolicy(FlushPolicy)
 */
class FlushPolicy : public std::enable_shared_from_this<FlushPolicy>
{
  GET_CLASS_NAME(FlushPolicy)
protected:
  std::shared_ptr<LiveIndexWriterConfig> indexWriterConfig;
  std::shared_ptr<InfoStream> infoStream;

  /**
   * Called for each delete term. If this is a delete triggered due to an update
   * the given {@link ThreadState} is non-null.
   * <p>
   * Note: This method is called synchronized on the given
   * {@link DocumentsWriterFlushControl} and it is guaranteed that the calling
   * thread holds the lock on the given {@link ThreadState}
   */
public:
  virtual void onDelete(std::shared_ptr<DocumentsWriterFlushControl> control,
                        std::shared_ptr<ThreadState> state) = 0;

  /**
   * Called for each document update on the given {@link ThreadState}'s
   * {@link DocumentsWriterPerThread}.
   * <p>
   * Note: This method is called  synchronized on the given
   * {@link DocumentsWriterFlushControl} and it is guaranteed that the calling
   * thread holds the lock on the given {@link ThreadState}
   */
  virtual void onUpdate(std::shared_ptr<DocumentsWriterFlushControl> control,
                        std::shared_ptr<ThreadState> state);

  /**
   * Called for each document addition on the given {@link ThreadState}s
   * {@link DocumentsWriterPerThread}.
   * <p>
   * Note: This method is synchronized by the given
   * {@link DocumentsWriterFlushControl} and it is guaranteed that the calling
   * thread holds the lock on the given {@link ThreadState}
   */
  virtual void onInsert(std::shared_ptr<DocumentsWriterFlushControl> control,
                        std::shared_ptr<ThreadState> state) = 0;

  /**
   * Called by DocumentsWriter to initialize the FlushPolicy
   */
protected:
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual void init(std::shared_ptr<LiveIndexWriterConfig> indexWriterConfig);

  /**
   * Returns the current most RAM consuming non-pending {@link ThreadState} with
   * at least one indexed document.
   * <p>
   * This method will never return <code>null</code>
   */
  virtual std::shared_ptr<ThreadState> findLargestNonPendingWriter(
      std::shared_ptr<DocumentsWriterFlushControl> control,
      std::shared_ptr<ThreadState> perThreadState);

private:
  bool assertMessage(const std::wstring &s);
};

} // #include  "core/src/java/org/apache/lucene/index/
