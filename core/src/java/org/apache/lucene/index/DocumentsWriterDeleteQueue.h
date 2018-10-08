#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/DeleteSlice.h"

#include  "core/src/java/org/apache/lucene/index/BufferedUpdates.h"
#include  "core/src/java/org/apache/lucene/util/InfoStream.h"
#include  "core/src/java/org/apache/lucene/search/Query.h"
#include  "core/src/java/org/apache/lucene/index/Term.h"
#include  "core/src/java/org/apache/lucene/index/DocValuesUpdate.h"
namespace org::apache::lucene::index
{
template <typename T>
class Node;
}
#include  "core/src/java/org/apache/lucene/index/FrozenBufferedUpdates.h"

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

using Query = org::apache::lucene::search::Query;
using Accountable = org::apache::lucene::util::Accountable;
using InfoStream = org::apache::lucene::util::InfoStream;

/**
 * {@link DocumentsWriterDeleteQueue} is a non-blocking linked pending deletes
 * queue. In contrast to other queue implementation we only maintain the
 * tail of the queue. A delete queue is always used in a context of a set of
 * DWPTs and a global delete pool. Each of the DWPT and the global pool need to
 * maintain their 'own' head of the queue (as a DeleteSlice instance per
 * {@link DocumentsWriterPerThread}).
 * The difference between the DWPT and the global pool is that the DWPT starts
 * maintaining a head once it has added its first document since for its
 * segments private deletes only the deletes after that document are relevant.
 * The global pool instead starts maintaining the head once this instance is
 * created by taking the sentinel instance as its initial head. <p> Since each
 * {@link DeleteSlice} maintains its own head and the deque is only single linked
 * the garbage collector takes care of pruning the deque for us. All nodes in the
 * deque that are still relevant should be either directly or indirectly
 * referenced by one of the DWPT's private {@link DeleteSlice} or by the global
 * {@link BufferedUpdates} slice. <p> Each DWPT as well as the global delete
 * pool maintain their private DeleteSlice instance. In the DWPT case updating a
 * slice is equivalent to atomically finishing the document. The slice update
 * guarantees a "happens before" relationship to all other updates in the same
 * indexing session. When a DWPT updates a document it:
 *
 * <ol>
 * <li>consumes a document and finishes its processing</li>
 * <li>updates its private {@link DeleteSlice} either by calling
 * {@link #updateSlice(DeleteSlice)} or {@link #add(Node, DeleteSlice)} (if the
 * document has a delTerm)</li>
 * <li>applies all deletes in the slice to its private {@link BufferedUpdates}
 * and resets it</li>
 * <li>increments its internal document id</li>
 * </ol>
 *
 * The DWPT also doesn't apply its current documents delete term until it has
 * updated its delete slice which ensures the consistency of the update. If the
 * update fails before the DeleteSlice could have been updated the deleteTerm
 * will also not be added to its private deletes neither to the global deletes.
 *
 */
class DocumentsWriterDeleteQueue final
    : public std::enable_shared_from_this<DocumentsWriterDeleteQueue>,
      public Accountable
{
  GET_CLASS_NAME(DocumentsWriterDeleteQueue)

  // the current end (latest delete operation) in the delete queue:
private:
  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: private volatile Node<?> tail;
  // C++ TODO: Java wildcard generics are not converted to C++:
  std::shared_ptr < Node < ? >> tail;

  /** Used to record deletes against all prior (already written to disk)
   * segments.  Whenever any segment flushes, we bundle up this set of deletes
   * and insert into the buffered updates stream before the newly flushed
   * segment(s). */
  const std::shared_ptr<DeleteSlice> globalSlice;
  const std::shared_ptr<BufferedUpdates> globalBufferedUpdates;

  // only acquired to update the global deletes, pkg-private for access by
  // tests:
public:
  const std::shared_ptr<ReentrantLock> globalBufferLock =
      std::make_shared<ReentrantLock>();

  const int64_t generation;

  /** Generates the sequence number that IW returns to callers changing the
   * index, showing the effective serialization of all operations. */
private:
  const std::shared_ptr<AtomicLong> nextSeqNo;

  const std::shared_ptr<InfoStream> infoStream;

  // for asserts
public:
  int64_t maxSeqNo = std::numeric_limits<int64_t>::max();

  DocumentsWriterDeleteQueue(std::shared_ptr<InfoStream> infoStream);

  DocumentsWriterDeleteQueue(std::shared_ptr<InfoStream> infoStream,
                             int64_t generation, int64_t startSeqNo);

  DocumentsWriterDeleteQueue(
      std::shared_ptr<InfoStream> infoStream,
      std::shared_ptr<BufferedUpdates> globalBufferedUpdates,
      int64_t generation, int64_t startSeqNo);

  int64_t addDelete(std::deque<Query> &queries);

  int64_t addDelete(std::deque<Term> &terms);

  int64_t addDocValuesUpdates(std::deque<DocValuesUpdate> &updates);

  static std::shared_ptr<Node<std::shared_ptr<Term>>>
  newNode(std::shared_ptr<Term> term);

  static std::shared_ptr<Node<std::deque<std::shared_ptr<DocValuesUpdate>>>>
  newNode(std::deque<DocValuesUpdate> &updates);

  /**
   * invariant for document update
   */
  template <typename T1>
  int64_t add(std::shared_ptr<Node<T1>> deleteNode,
                std::shared_ptr<DeleteSlice> slice);

  template <typename T1>
  // C++ WARNING: The following method was originally marked 'synchronized':
  int64_t add(std::shared_ptr<Node<T1>> newNode);

  bool anyChanges();

  void tryApplyGlobalSlice();

  std::shared_ptr<FrozenBufferedUpdates> freezeGlobalBuffer(
      std::shared_ptr<DeleteSlice> callerSlice) ;

  std::shared_ptr<DeleteSlice> newSlice();

  /** Negative result means there were new deletes since we last applied */
  // C++ WARNING: The following method was originally marked 'synchronized':
  int64_t updateSlice(std::shared_ptr<DeleteSlice> slice);

  /** Just like updateSlice, but does not assign a sequence number */
  bool updateSliceNoSeqNo(std::shared_ptr<DeleteSlice> slice);

public:
  class DeleteSlice : public std::enable_shared_from_this<DeleteSlice>
  {
    GET_CLASS_NAME(DeleteSlice)
    // No need to be volatile, slices are thread captive (only accessed by one
    // thread)!
  public:
    // C++ TODO: Java wildcard generics are not converted to C++:
    // ORIGINAL LINE: Node<?> sliceHead;
    std::shared_ptr < Node < ? >> sliceHead; // we don't apply this one
    // C++ TODO: Java wildcard generics are not converted to C++:
    // ORIGINAL LINE: Node<?> sliceTail;
    std::shared_ptr < Node < ? >> sliceTail;

    template <typename T1>
    DeleteSlice(std::shared_ptr<Node<T1>> currentTail);

    virtual void apply(std::shared_ptr<BufferedUpdates> del, int docIDUpto);

    virtual void reset();

    /**
     * Returns <code>true</code> iff the given node is identical to the the
     * slices tail, otherwise <code>false</code>.
     */
    template <typename T1>
    bool isTail(std::shared_ptr<Node<T1>> node);

    /**
     * Returns <code>true</code> iff the given item is identical to the item
     * hold by the slices tail, otherwise <code>false</code>.
     */
    virtual bool isTailItem(std::any object);

    virtual bool isEmpty();
  };

public:
  int numGlobalTermDeletes();

  void clear();

public:
  template <typename T>
  class Node : public std::enable_shared_from_this<Node>
  {
    GET_CLASS_NAME(Node)
  public:
    // C++ TODO: 'volatile' has a different meaning in C++:
    // ORIGINAL LINE: volatile Node<?> next;
    // C++ TODO: Java wildcard generics are not converted to C++:
    std::shared_ptr < Node < ? >> next;
    const T item;

    Node(T item) : item(item) {}

    virtual void apply(std::shared_ptr<BufferedUpdates> bufferedDeletes,
                       int docIDUpto)
    {
      throw std::make_shared<IllegalStateException>(
          L"sentinel item must never be applied");
    }

    virtual bool isDelete() { return true; }
  };

private:
  class TermNode final : public Node<std::shared_ptr<Term>>
  {
    GET_CLASS_NAME(TermNode)

  public:
    TermNode(std::shared_ptr<Term> term);

    void apply(std::shared_ptr<BufferedUpdates> bufferedDeletes,
               int docIDUpto) override;

    virtual std::wstring toString();

  protected:
    std::shared_ptr<TermNode> shared_from_this()
    {
      return std::static_pointer_cast<TermNode>(Node<Term>::shared_from_this());
    }
  };

private:
  class QueryArrayNode final : public Node<std::deque<std::shared_ptr<Query>>>
  {
    GET_CLASS_NAME(QueryArrayNode)
  public:
    QueryArrayNode(std::deque<std::shared_ptr<Query>> &query);

    void apply(std::shared_ptr<BufferedUpdates> bufferedUpdates,
               int docIDUpto) override;

  protected:
    std::shared_ptr<QueryArrayNode> shared_from_this()
    {
      return std::static_pointer_cast<QueryArrayNode>(
          Node<org.apache.lucene.search.Query[]>::shared_from_this());
    }
  };

private:
  class TermArrayNode final : public Node<std::deque<std::shared_ptr<Term>>>
  {
    GET_CLASS_NAME(TermArrayNode)
  public:
    TermArrayNode(std::deque<std::shared_ptr<Term>> &term);

    void apply(std::shared_ptr<BufferedUpdates> bufferedUpdates,
               int docIDUpto) override;

    virtual std::wstring toString();

  protected:
    std::shared_ptr<TermArrayNode> shared_from_this()
    {
      return std::static_pointer_cast<TermArrayNode>(
          Node<Term[]>::shared_from_this());
    }
  };

private:
  class DocValuesUpdatesNode final
      : public Node<std::deque<std::shared_ptr<DocValuesUpdate>>>
  {
    GET_CLASS_NAME(DocValuesUpdatesNode)

  public:
    DocValuesUpdatesNode(std::deque<DocValuesUpdate> &updates);

    void apply(std::shared_ptr<BufferedUpdates> bufferedUpdates,
               int docIDUpto) override;

    bool isDelete() override;

    virtual std::wstring toString();

  protected:
    std::shared_ptr<DocValuesUpdatesNode> shared_from_this()
    {
      return std::static_pointer_cast<DocValuesUpdatesNode>(
          Node<DocValuesUpdate[]>::shared_from_this());
    }
  };

private:
  bool forceApplyGlobalSlice();

public:
  int getBufferedUpdatesTermsSize();

  int64_t ramBytesUsed() override;

  virtual std::wstring toString();

  int64_t getNextSequenceNumber();

  int64_t getLastSequenceNumber();

  /** Inserts a gap in the sequence numbers.  This is used by IW during flush or
   * commit to ensure any in-flight threads get sequence numbers
   *  inside the gap */
  void skipSequenceNumbers(int64_t jump);
};

} // #include  "core/src/java/org/apache/lucene/index/
