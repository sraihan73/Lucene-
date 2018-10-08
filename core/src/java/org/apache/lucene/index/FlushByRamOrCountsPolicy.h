#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class DocumentsWriterFlushControl;
}

namespace org::apache::lucene::index
{
class DocumentsWriterPerThreadPool;
}
namespace org::apache::lucene::index
{
class ThreadState;
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
namespace org::apache::lucene::index
{

using ThreadState =
    org::apache::lucene::index::DocumentsWriterPerThreadPool::ThreadState;

/**
 * Default {@link FlushPolicy} implementation that flushes new segments based on
 * RAM used and document count depending on the IndexWriter's
 * {@link IndexWriterConfig}. It also applies pending deletes based on the
 * number of buffered delete terms.
 *
 * <ul>
 * <li>
 * {@link #onDelete(DocumentsWriterFlushControl,
 * DocumentsWriterPerThreadPool.ThreadState)}
 * - applies pending delete operations based on the global number of buffered
 * delete terms if the consumed memory is greater than {@link
 * IndexWriterConfig#getRAMBufferSizeMB()}</li>. <li>
 * {@link #onInsert(DocumentsWriterFlushControl,
 * DocumentsWriterPerThreadPool.ThreadState)}
 * - flushes either on the number of documents per
 * {@link DocumentsWriterPerThread} (
 * {@link DocumentsWriterPerThread#getNumDocsInRAM()}) or on the global active
 * memory consumption in the current indexing session iff
 * {@link IndexWriterConfig#getMaxBufferedDocs()} or
 * {@link IndexWriterConfig#getRAMBufferSizeMB()} is enabled respectively</li>
 * <li>
 * {@link #onUpdate(DocumentsWriterFlushControl,
 * DocumentsWriterPerThreadPool.ThreadState)}
 * - calls
 * {@link #onInsert(DocumentsWriterFlushControl,
 * DocumentsWriterPerThreadPool.ThreadState)} and
 * {@link #onDelete(DocumentsWriterFlushControl,
 * DocumentsWriterPerThreadPool.ThreadState)} in order</li>
 * </ul>
 * All {@link IndexWriterConfig} settings are used to mark
 * {@link DocumentsWriterPerThread} as flush pending during indexing with
 * respect to their live updates.
 * <p>
 * If {@link IndexWriterConfig#setRAMBufferSizeMB(double)} is enabled, the
 * largest ram consuming {@link DocumentsWriterPerThread} will be marked as
 * pending iff the global active RAM consumption is {@code >=} the configured
 * max RAM buffer.
 */
class FlushByRamOrCountsPolicy : public FlushPolicy
{
  GET_CLASS_NAME(FlushByRamOrCountsPolicy)

public:
  void onDelete(std::shared_ptr<DocumentsWriterFlushControl> control,
                std::shared_ptr<ThreadState> state) override;

  void onInsert(std::shared_ptr<DocumentsWriterFlushControl> control,
                std::shared_ptr<ThreadState> state) override;

  /**
   * Marks the most ram consuming active {@link DocumentsWriterPerThread} flush
   * pending
   */
protected:
  virtual void
  markLargestWriterPending(std::shared_ptr<DocumentsWriterFlushControl> control,
                           std::shared_ptr<ThreadState> perThreadState,
                           int64_t const currentBytesPerThread);

  /**
   * Returns <code>true</code> if this {@link FlushPolicy} flushes on
   * {@link IndexWriterConfig#getMaxBufferedDocs()}, otherwise
   * <code>false</code>.
   */
  virtual bool flushOnDocCount();

  /**
   * Returns <code>true</code> if this {@link FlushPolicy} flushes on
   * {@link IndexWriterConfig#getRAMBufferSizeMB()}, otherwise
   * <code>false</code>.
   */
  virtual bool flushOnRAM();

protected:
  std::shared_ptr<FlushByRamOrCountsPolicy> shared_from_this()
  {
    return std::static_pointer_cast<FlushByRamOrCountsPolicy>(
        FlushPolicy::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
