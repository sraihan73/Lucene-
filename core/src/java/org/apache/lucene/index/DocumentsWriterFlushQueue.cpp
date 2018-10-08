using namespace std;

#include "DocumentsWriterFlushQueue.h"

namespace org::apache::lucene::index
{
using FlushedSegment =
    org::apache::lucene::index::DocumentsWriterPerThread::FlushedSegment;
using IOUtils = org::apache::lucene::util::IOUtils;

// C++ WARNING: The following method was originally marked 'synchronized':
void DocumentsWriterFlushQueue::addDeletes(
    shared_ptr<DocumentsWriterDeleteQueue> deleteQueue) 
{
  incTickets(); // first inc the ticket count - freeze opens
                // a window for #anyChanges to fail
  bool success = false;
  try {
    queue.push_back(make_shared<FlushTicket>(
        deleteQueue->freezeGlobalBuffer(nullptr), false));
    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (!success) {
      decTickets();
    }
  }
}

void DocumentsWriterFlushQueue::incTickets()
{
  int numTickets = ticketCount->incrementAndGet();
  assert(numTickets > 0);
}

void DocumentsWriterFlushQueue::decTickets()
{
  int numTickets = ticketCount->decrementAndGet();
  assert(numTickets >= 0);
}

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<FlushTicket> DocumentsWriterFlushQueue::addFlushTicket(
    shared_ptr<DocumentsWriterPerThread> dwpt) 
{
  // Each flush is assigned a ticket in the order they acquire the ticketQueue
  // lock
  incTickets();
  bool success = false;
  try {
    // prepare flush freezes the global deletes - do in synced block!
    shared_ptr<FlushTicket> *const ticket =
        make_shared<FlushTicket>(dwpt->prepareFlush(), true);
    queue.push_back(ticket);
    success = true;
    return ticket;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (!success) {
      decTickets();
    }
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
void DocumentsWriterFlushQueue::addSegment(shared_ptr<FlushTicket> ticket,
                                           shared_ptr<FlushedSegment> segment)
{
  assert(ticket->hasSegment);
  // the actual flush is done asynchronously and once done the FlushedSegment
  // is passed to the flush ticket
  ticket->setSegment(segment);
}

// C++ WARNING: The following method was originally marked 'synchronized':
void DocumentsWriterFlushQueue::markTicketFailed(shared_ptr<FlushTicket> ticket)
{
  assert(ticket->hasSegment);
  // to free the queue we mark tickets as failed just to clean up the queue.
  ticket->setFailed();
}

bool DocumentsWriterFlushQueue::hasTickets()
{
  assert((ticketCount->get() >= 0,
          L"ticketCount should be >= 0 but was: " + ticketCount->get()));
  return ticketCount->get() != 0;
}

void DocumentsWriterFlushQueue::innerPurge(
    IOUtils::IOConsumer<std::shared_ptr<FlushTicket>>
        consumer) 
{
  assert(purgeLock->isHeldByCurrentThread());
  while (true) {
    shared_ptr<FlushTicket> *const head;
    constexpr bool canPublish;
    // C++ TODO: Multithread locking on 'this' is not converted to native C++:
    synchronized(shared_from_this())
    {
      head = queue.front();
      canPublish = head != nullptr && head->canPublish(); // do this synced
    }
    if (canPublish) {
      try {
        /*
         * if we block on publish -> lock IW -> lock BufferedDeletes we don't
         * block concurrent segment flushes just because they want to append to
         * the queue. the downside is that we need to force a purge on fullFlush
         * since there could be a ticket still in the queue.
         */
        consumer(head);

      }
      // C++ TODO: There is no native C++ equivalent to the exception 'finally'
      // clause:
      finally {
        // C++ TODO: Multithread locking on 'this' is not converted to native
        // C++:
        synchronized(shared_from_this())
        {
          // finally remove the published ticket from the queue
          shared_ptr<FlushTicket> *const poll = queue.pop_front();
          decTickets();
          // we hold the purgeLock so no other thread should have polled:
          assert(poll == head);
        }
      }
    } else {
      break;
    }
  }
}

void DocumentsWriterFlushQueue::forcePurge(
    IOUtils::IOConsumer<std::shared_ptr<FlushTicket>>
        consumer) 
{
  assert(!Thread::holdsLock(shared_from_this()));
  purgeLock->lock();
  try {
    innerPurge(consumer);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    purgeLock->unlock();
  }
}

void DocumentsWriterFlushQueue::tryPurge(
    IOUtils::IOConsumer<std::shared_ptr<FlushTicket>>
        consumer) 
{
  assert(!Thread::holdsLock(shared_from_this()));
  if (purgeLock->tryLock()) {
    try {
      innerPurge(consumer);
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      purgeLock->unlock();
    }
  }
}

int DocumentsWriterFlushQueue::getTicketCount() { return ticketCount->get(); }

DocumentsWriterFlushQueue::FlushTicket::FlushTicket(
    shared_ptr<FrozenBufferedUpdates> frozenUpdates, bool hasSegment)
    : frozenUpdates(frozenUpdates), hasSegment(hasSegment)
{
}

bool DocumentsWriterFlushQueue::FlushTicket::canPublish()
{
  return hasSegment == false || segment != nullptr || failed;
}

// C++ WARNING: The following method was originally marked 'synchronized':
void DocumentsWriterFlushQueue::FlushTicket::markPublished()
{
  assert((published == false,
          L"ticket was already published - can not publish twice"));
  published = true;
}

void DocumentsWriterFlushQueue::FlushTicket::setSegment(
    shared_ptr<FlushedSegment> segment)
{
  assert(!failed);
  this->segment = segment;
}

void DocumentsWriterFlushQueue::FlushTicket::setFailed()
{
  assert(segment == nullptr);
  failed = true;
}

shared_ptr<FlushedSegment>
DocumentsWriterFlushQueue::FlushTicket::getFlushedSegment()
{
  return segment;
}

shared_ptr<FrozenBufferedUpdates>
DocumentsWriterFlushQueue::FlushTicket::getFrozenUpdates()
{
  return frozenUpdates;
}
} // namespace org::apache::lucene::index