#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class DocConsumer;
}

namespace org::apache::lucene::analysis
{
class Analyzer;
}
namespace org::apache::lucene::util
{
class InfoStream;
}
namespace org::apache::lucene::search::similarities
{
class Similarity;
}
namespace org::apache::lucene::index
{
class IndexableField;
}
namespace org::apache::lucene::index
{
class SegmentCommitInfo;
}
namespace org::apache::lucene::index
{
class FieldInfos;
}
namespace org::apache::lucene::index
{
class FrozenBufferedUpdates;
}
namespace org::apache::lucene::util
{
class FixedBitSet;
}
namespace org::apache::lucene::index
{
class Sorter;
}
namespace org::apache::lucene::index
{
class DocMap;
}
namespace org::apache::lucene::index
{
class BufferedUpdates;
}
namespace org::apache::lucene::codecs
{
class Codec;
}
namespace org::apache::lucene::store
{
class TrackingDirectoryWrapper;
}
namespace org::apache::lucene::store
{
class Directory;
}
namespace org::apache::lucene::util
{
class Counter;
}
namespace org::apache::lucene::index
{
class SegmentInfo;
}
namespace org::apache::lucene::index
{
class Builder;
}
namespace org::apache::lucene::index
{
class DocumentsWriterDeleteQueue;
}
namespace org::apache::lucene::index
{
class DeleteSlice;
}
namespace org::apache::lucene::util
{
class ByteBlockPool;
}
namespace org::apache::lucene::util
{
class Allocator;
}
namespace org::apache::lucene::util
{
class IntBlockPool;
}
namespace org::apache::lucene::index
{
class LiveIndexWriterConfig;
}
namespace org::apache::lucene::index
{
class DocumentsWriter;
}
namespace org::apache::lucene::index
{
class FlushNotifications;
}
namespace org::apache::lucene::index
{
template <typename T>
class Node;
}
namespace org::apache::lucene::util
{
class Bits;
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

using Analyzer = org::apache::lucene::analysis::Analyzer;
using Codec = org::apache::lucene::codecs::Codec;
using DeleteSlice =
    org::apache::lucene::index::DocumentsWriterDeleteQueue::DeleteSlice;
using Similarity = org::apache::lucene::search::similarities::Similarity;
using Directory = org::apache::lucene::store::Directory;
using TrackingDirectoryWrapper =
    org::apache::lucene::store::TrackingDirectoryWrapper;
using Bits = org::apache::lucene::util::Bits;
using Allocator = org::apache::lucene::util::ByteBlockPool::Allocator;
using Counter = org::apache::lucene::util::Counter;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;
using InfoStream = org::apache::lucene::util::InfoStream;
using IntBlockPool = org::apache::lucene::util::IntBlockPool;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.util.ByteBlockPool.BYTE_BLOCK_MASK;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.util.ByteBlockPool.BYTE_BLOCK_SIZE;

class DocumentsWriterPerThread final
    : public std::enable_shared_from_this<DocumentsWriterPerThread>
{
  GET_CLASS_NAME(DocumentsWriterPerThread)

  /**
   * The IndexingChain must define the {@link
   * #getChain(DocumentsWriterPerThread)} method which returns the DocConsumer
   * that the DocumentsWriter calls to process the documents.
   */
public:
  class IndexingChain : public std::enable_shared_from_this<IndexingChain>
  {
    GET_CLASS_NAME(IndexingChain)
  public:
    virtual std::shared_ptr<DocConsumer> getChain(
        std::shared_ptr<DocumentsWriterPerThread> documentsWriterPerThread) = 0;
  };

private:
  std::runtime_error abortingException;

public:
  void onAbortingException(std::runtime_error throwable);

  bool hasHitAbortingException();

  bool isAborted();

  static const std::shared_ptr<IndexingChain> defaultIndexingChain;

private:
  class IndexingChainAnonymousInnerClass : public IndexingChain
  {
    GET_CLASS_NAME(IndexingChainAnonymousInnerClass)
  public:
    IndexingChainAnonymousInnerClass();

    std::shared_ptr<DocConsumer>
    getChain(std::shared_ptr<DocumentsWriterPerThread>
                 documentsWriterPerThread)  override;

  protected:
    std::shared_ptr<IndexingChainAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<IndexingChainAnonymousInnerClass>(
          IndexingChain::shared_from_this());
    }
  };

public:
  class DocState : public std::enable_shared_from_this<DocState>
  {
    GET_CLASS_NAME(DocState)
  public:
    const std::shared_ptr<DocumentsWriterPerThread> docWriter;
    std::shared_ptr<Analyzer> analyzer;
    std::shared_ptr<InfoStream> infoStream;
    std::shared_ptr<Similarity> similarity;
    int docID = 0;
    // C++ TODO: Java wildcard generics are not converted to C++:
    // ORIGINAL LINE: Iterable<? extends IndexableField> doc;
    std::deque < ? extends IndexableField > doc;

    DocState(std::shared_ptr<DocumentsWriterPerThread> docWriter,
             std::shared_ptr<InfoStream> infoStream);

    virtual void clear();
  };

public:
  class FlushedSegment final
      : public std::enable_shared_from_this<FlushedSegment>
  {
    GET_CLASS_NAME(FlushedSegment)
  public:
    const std::shared_ptr<SegmentCommitInfo> segmentInfo;
    const std::shared_ptr<FieldInfos> fieldInfos;
    const std::shared_ptr<FrozenBufferedUpdates> segmentUpdates;
    const std::shared_ptr<FixedBitSet> liveDocs;
    const std::shared_ptr<Sorter::DocMap> sortMap;
    const int delCount;

  private:
    FlushedSegment(std::shared_ptr<InfoStream> infoStream,
                   std::shared_ptr<SegmentCommitInfo> segmentInfo,
                   std::shared_ptr<FieldInfos> fieldInfos,
                   std::shared_ptr<BufferedUpdates> segmentUpdates,
                   std::shared_ptr<FixedBitSet> liveDocs, int delCount,
                   std::shared_ptr<Sorter::DocMap> sortMap) ;
  };

  /** Called if we hit an exception at a bad time (when
   *  updating the index files) and must discard all
   *  currently buffered docs.  This resets our state,
   *  discarding any docs added since last flush. */
public:
  void abort() ;

private:
  static constexpr bool INFO_VERBOSE = false;

public:
  const std::shared_ptr<Codec> codec;
  const std::shared_ptr<TrackingDirectoryWrapper> directory;
  const std::shared_ptr<Directory> directoryOrig;
  const std::shared_ptr<DocState> docState;
  const std::shared_ptr<DocConsumer> consumer;
  // C++ NOTE: Fields cannot have the same name as methods:
  const std::shared_ptr<Counter> bytesUsed_;

  // Updates for our still-in-RAM (to be flushed next) segment
  const std::shared_ptr<BufferedUpdates> pendingUpdates;
  const std::shared_ptr<SegmentInfo>
      segmentInfo; // Current segment we are working on
private:
  bool aborted = false; // True if we aborted

  const std::shared_ptr<FieldInfos::Builder> fieldInfos;
  const std::shared_ptr<InfoStream> infoStream;
  int numDocsInRAM = 0;

public:
  const std::shared_ptr<DocumentsWriterDeleteQueue> deleteQueue;

private:
  const std::shared_ptr<DeleteSlice> deleteSlice;
  const std::shared_ptr<NumberFormat> nf =
      NumberFormat::getInstance(Locale::ROOT);

public:
  const std::shared_ptr<Allocator> byteBlockAllocator;
  const std::shared_ptr<IntBlockPool::Allocator> intBlockAllocator;

private:
  const std::shared_ptr<AtomicLong> pendingNumDocs;
  const std::shared_ptr<LiveIndexWriterConfig> indexWriterConfig;
  const bool enableTestPoints;
  const int indexVersionCreated;

public:
  DocumentsWriterPerThread(
      int indexVersionCreated, const std::wstring &segmentName,
      std::shared_ptr<Directory> directoryOrig,
      std::shared_ptr<Directory> directory,
      std::shared_ptr<LiveIndexWriterConfig> indexWriterConfig,
      std::shared_ptr<InfoStream> infoStream,
      std::shared_ptr<DocumentsWriterDeleteQueue> deleteQueue,
      std::shared_ptr<FieldInfos::Builder> fieldInfos,
      std::shared_ptr<AtomicLong> pendingNumDocs,
      bool enableTestPoints) ;

  std::shared_ptr<FieldInfos::Builder> getFieldInfosBuilder();

  int getIndexCreatedVersionMajor();

  void testPoint(const std::wstring &message);

  /** Anything that will add N docs to the index should reserve first to
   *  make sure it's allowed. */
private:
  void reserveOneDoc();

public:
  template <typename T1, typename T2>
  // C++ TODO: There is no native C++ template equivalent to this generic
  // constraint: ORIGINAL LINE: public long updateDocument(Iterable<? extends
  // IndexableField> doc, org.apache.lucene.analysis.Analyzer analyzer,
  // DocumentsWriterDeleteQueue.Node<?> deleteNode,
  // DocumentsWriter.FlushNotifications flushNotifications) throws
  // java.io.IOException
  int64_t updateDocument(
      std::deque<T1> doc, std::shared_ptr<Analyzer> analyzer,
      std::shared_ptr<DocumentsWriterDeleteQueue::Node<T2>> deleteNode,
      std::shared_ptr<DocumentsWriter::FlushNotifications>
          flushNotifications) ;

  template <typename T1, typename T2>
  // C++ TODO: There is no native C++ template equivalent to this generic
  // constraint: ORIGINAL LINE: public long updateDocuments(Iterable<? extends
  // Iterable<? extends IndexableField>> docs,
  // org.apache.lucene.analysis.Analyzer analyzer,
  // DocumentsWriterDeleteQueue.Node<?> deleteNode,
  // DocumentsWriter.FlushNotifications flushNotifications) throws
  // java.io.IOException
  int64_t updateDocuments(
      std::deque<T1> docs, std::shared_ptr<Analyzer> analyzer,
      std::shared_ptr<DocumentsWriterDeleteQueue::Node<T2>> deleteNode,
      std::shared_ptr<DocumentsWriter::FlushNotifications>
          flushNotifications) ;

private:
  template <typename T1>
  int64_t finishDocument(
      std::shared_ptr<DocumentsWriterDeleteQueue::Node<T1>> deleteNode);

  // Buffer a specific docID for deletion. Currently only
  // used when we hit an exception when adding a document
public:
  void deleteDocID(int docIDUpto);

  /**
   * Returns the number of RAM resident documents in this {@link
   * DocumentsWriterPerThread}
   */
  int getNumDocsInRAM();

  /**
   * Prepares this DWPT for flushing. This method will freeze and return the
   * {@link DocumentsWriterDeleteQueue}s global buffer and apply all pending
   * deletes to this DWPT.
   */
  std::shared_ptr<FrozenBufferedUpdates> prepareFlush() ;

  /** Flush all pending docs to a new segment */
  std::shared_ptr<FlushedSegment>
  flush(std::shared_ptr<DocumentsWriter::FlushNotifications>
            flushNotifications) ;

private:
  void maybeAbort(const std::wstring &location,
                  std::shared_ptr<DocumentsWriter::FlushNotifications>
                      flushNotifications) ;

  const std::shared_ptr<Set<std::wstring>> filesToDelete =
      std::unordered_set<std::wstring>();

public:
  std::shared_ptr<Set<std::wstring>> pendingFilesToDelete();

private:
  std::shared_ptr<FixedBitSet>
  sortLiveDocs(std::shared_ptr<Bits> liveDocs,
               std::shared_ptr<Sorter::DocMap> sortMap) ;

  /**
   * Seals the {@link SegmentInfo} for the new flushed segment and persists
   * the deleted documents {@link FixedBitSet}.
   */
public:
  void sealFlushedSegment(std::shared_ptr<FlushedSegment> flushedSegment,
                          std::shared_ptr<Sorter::DocMap> sortMap,
                          std::shared_ptr<DocumentsWriter::FlushNotifications>
                              flushNotifications) ;

  /** Get current segment info we are writing. */
  std::shared_ptr<SegmentInfo> getSegmentInfo();

  int64_t bytesUsed();

  /* Initial chunks size of the shared byte[] blocks used to
     store postings data */
  static const int BYTE_BLOCK_NOT_MASK = ~BYTE_BLOCK_MASK;

  /* if you increase this, you must fix field cache impl for
   * getTerms/getTermsIndex requires <= 32768 */
  static const int MAX_TERM_LENGTH_UTF8 = BYTE_BLOCK_SIZE - 2;

private:
  class IntBlockAllocator : public IntBlockPool::Allocator
  {
    GET_CLASS_NAME(IntBlockAllocator)
  private:
    const std::shared_ptr<Counter> bytesUsed;

  public:
    IntBlockAllocator(std::shared_ptr<Counter> bytesUsed);

    /* Allocate another int[] from the shared pool */
    std::deque<int> getIntBlock() override;

    void recycleIntBlocks(std::deque<std::deque<int>> &blocks, int offset,
                          int length) override;

  protected:
    std::shared_ptr<IntBlockAllocator> shared_from_this()
    {
      return std::static_pointer_cast<IntBlockAllocator>(
          org.apache.lucene.util.IntBlockPool.Allocator::shared_from_this());
    }
  };

public:
  virtual std::wstring toString();
};

} // namespace org::apache::lucene::index
