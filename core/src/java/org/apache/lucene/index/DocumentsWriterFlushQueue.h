#pragma once
#include "stringhelper.h"
#include <deque>
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class FlushTicket;
}

namespace org::apache::lucene::index
{
class DocumentsWriterDeleteQueue;
}
namespace org::apache::lucene::index
{
class DocumentsWriterPerThread;
}
namespace org::apache::lucene::index
{
class FlushedSegment;
}
namespace org::apache::lucene::util
{
class IOUtils;
}
namespace org::apache::lucene::util
{
template <typename T>
class IOConsumer;
}
namespace org::apache::lucene::index
{
class FrozenBufferedUpdates;
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

using FlushedSegment =
    org::apache::lucene::index::DocumentsWriterPerThread::FlushedSegment;
using IOUtils = org::apache::lucene::util::IOUtils;

/**
 * @lucene.internal
 */
class DocumentsWriterFlushQueue final
    : public std::enable_shared_from_this<DocumentsWriterFlushQueue>
{
  GET_CLASS_NAME(DocumentsWriterFlushQueue)
private:
  const std::deque<std::shared_ptr<FlushTicket>> queue =
      std::deque<std::shared_ptr<FlushTicket>>();
  // we track tickets separately since count must be present even before the
  // ticket is constructed ie. queue.size would not reflect it.
  const std::shared_ptr<AtomicInteger> ticketCount =
      std::make_shared<AtomicInteger>();
  const std::shared_ptr<ReentrantLock> purgeLock =
      std::make_shared<ReentrantLock>();

public:
  // C++ WARNING: The following method was originally marked 'synchronized':
  void
  addDeletes(std::shared_ptr<DocumentsWriterDeleteQueue> deleteQueue) throw(
      IOException);

private:
  void incTickets();

  void decTickets();

public:
  // C++ WARNING: The following method was originally marked 'synchronized':
  std::shared_ptr<FlushTicket> addFlushTicket(
      std::shared_ptr<DocumentsWriterPerThread> dwpt) ;

  // C++ WARNING: The following method was originally marked 'synchronized':
  void addSegment(std::shared_ptr<FlushTicket> ticket,
                  std::shared_ptr<FlushedSegment> segment);

  // C++ WARNING: The following method was originally marked 'synchronized':
  void markTicketFailed(std::shared_ptr<FlushTicket> ticket);

  bool hasTickets();

private:
  void
  innerPurge(IOUtils::IOConsumer<std::shared_ptr<FlushTicket>> consumer) throw(
      IOException);

public:
  void
  forcePurge(IOUtils::IOConsumer<std::shared_ptr<FlushTicket>> consumer) throw(
      IOException);

  void
  tryPurge(IOUtils::IOConsumer<std::shared_ptr<FlushTicket>> consumer) throw(
      IOException);

  int getTicketCount();

public:
  class FlushTicket final : public std::enable_shared_from_this<FlushTicket>
  {
    GET_CLASS_NAME(FlushTicket)
  private:
    const std::shared_ptr<FrozenBufferedUpdates> frozenUpdates;
    const bool hasSegment;
    std::shared_ptr<FlushedSegment> segment;
    bool failed = false;
    bool published = false;

  public:
    FlushTicket(std::shared_ptr<FrozenBufferedUpdates> frozenUpdates,
                bool hasSegment);

    bool canPublish();

    // C++ WARNING: The following method was originally marked 'synchronized':
    void markPublished();

  private:
    void setSegment(std::shared_ptr<FlushedSegment> segment);

    void setFailed();

    /**
     * Returns the flushed segment or <code>null</code> if this flush ticket
     * doesn't have a segment. This can be the case if this ticket represents a
     * flushed global frozen updates package.
     */
  public:
    std::shared_ptr<FlushedSegment> getFlushedSegment();

    /**
     * Returns a frozen global deletes package.
     */
    std::shared_ptr<FrozenBufferedUpdates> getFrozenUpdates();
  };
};

} // namespace org::apache::lucene::index
