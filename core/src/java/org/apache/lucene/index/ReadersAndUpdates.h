#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <algorithm>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/SegmentCommitInfo.h"

#include  "core/src/java/org/apache/lucene/index/SegmentReader.h"
#include  "core/src/java/org/apache/lucene/index/PendingDeletes.h"
#include  "core/src/java/org/apache/lucene/index/DocValuesFieldUpdates.h"
#include  "core/src/java/org/apache/lucene/index/Sorter.h"
#include  "core/src/java/org/apache/lucene/index/DocMap.h"
#include  "core/src/java/org/apache/lucene/store/IOContext.h"
#include  "core/src/java/org/apache/lucene/index/MergePolicy.h"
#include  "core/src/java/org/apache/lucene/index/CodecReader.h"
#include  "core/src/java/org/apache/lucene/util/Bits.h"
#include  "core/src/java/org/apache/lucene/store/Directory.h"
#include  "core/src/java/org/apache/lucene/codecs/DocValuesFormat.h"
#include  "core/src/java/org/apache/lucene/index/FieldInfos.h"
#include  "core/src/java/org/apache/lucene/util/InfoStream.h"
#include  "core/src/java/org/apache/lucene/index/FieldInfo.h"
#include  "core/src/java/org/apache/lucene/index/Iterator.h"
#include  "core/src/java/org/apache/lucene/index/BinaryDocValues.h"
namespace org::apache::lucene::index
{
template <typename DocValuesInstance>
class MergedDocValues;
}
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"
#include  "core/src/java/org/apache/lucene/index/NumericDocValues.h"
#include  "core/src/java/org/apache/lucene/codecs/FieldInfosFormat.h"
#include  "core/src/java/org/apache/lucene/index/FieldNumbers.h"

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

using DocValuesFormat = org::apache::lucene::codecs::DocValuesFormat;
using FieldInfosFormat = org::apache::lucene::codecs::FieldInfosFormat;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using Bits = org::apache::lucene::util::Bits;
using InfoStream = org::apache::lucene::util::InfoStream;

// Used by IndexWriter to hold open SegmentReaders (for
// searching or merging), plus pending deletes and updates,
// for a given segment
class ReadersAndUpdates final
    : public std::enable_shared_from_this<ReadersAndUpdates>
{
  GET_CLASS_NAME(ReadersAndUpdates)
  // Not final because we replace (clone) when we need to
  // change it and it's been shared:
public:
  const std::shared_ptr<SegmentCommitInfo> info;

  // Tracks how many consumers are using this instance:
private:
  // C++ NOTE: Fields cannot have the same name as methods:
  const std::shared_ptr<AtomicInteger> refCount_ =
      std::make_shared<AtomicInteger>(1);

  // Set once (null, and then maybe set, and never set again):
  std::shared_ptr<SegmentReader> reader;

  // How many further deletions we've done against
  // liveDocs vs when we loaded it or last wrote it:
  const std::shared_ptr<PendingDeletes> pendingDeletes;

  // the major version this index was created with
  const int indexCreatedVersionMajor;

  // Indicates whether this segment is currently being merged. While a segment
  // is merging, all field updates are also registered in the
  // mergingNumericUpdates map_obj. Also, calls to writeFieldUpdates merge the
  // updates with mergingNumericUpdates.
  // That way, when the segment is done merging, IndexWriter can apply the
  // updates on the merged segment too.
  // C++ NOTE: Fields cannot have the same name as methods:
  bool isMerging_ = false;

  // Holds resolved (to docIDs) doc values updates that have not yet been
  // written to the index
  const std::unordered_map<std::wstring,
                           std::deque<std::shared_ptr<DocValuesFieldUpdates>>>
      pendingDVUpdates = std::unordered_map<
          std::wstring, std::deque<std::shared_ptr<DocValuesFieldUpdates>>>();

  // Holds resolved (to docIDs) doc values updates that were resolved while
  // this segment was being merged; at the end of the merge we carry over
  // these updates (remapping their docIDs) to the newly merged segment
  const std::unordered_map<std::wstring,
                           std::deque<std::shared_ptr<DocValuesFieldUpdates>>>
      mergingDVUpdates = std::unordered_map<
          std::wstring, std::deque<std::shared_ptr<DocValuesFieldUpdates>>>();

  // Only set if there are doc values updates against this segment, and the
  // index is sorted:
public:
  std::shared_ptr<Sorter::DocMap> sortMap;

  const std::shared_ptr<AtomicLong> ramBytesUsed =
      std::make_shared<AtomicLong>();

  ReadersAndUpdates(int indexCreatedVersionMajor,
                    std::shared_ptr<SegmentCommitInfo> info,
                    std::shared_ptr<PendingDeletes> pendingDeletes);

  /** Init from a previously opened SegmentReader.
   *
   * <p>NOTE: steals incoming ref from reader. */
  ReadersAndUpdates(
      int indexCreatedVersionMajor, std::shared_ptr<SegmentReader> reader,
      std::shared_ptr<PendingDeletes> pendingDeletes) ;

  void incRef();

  void decRef();

  int refCount();

  // C++ WARNING: The following method was originally marked 'synchronized':
  int getDelCount();

private:
  // C++ WARNING: The following method was originally marked 'synchronized':
  bool assertNoDupGen(
      std::deque<std::shared_ptr<DocValuesFieldUpdates>> &fieldUpdates,
      std::shared_ptr<DocValuesFieldUpdates> update);

  /** Adds a new resolved (meaning it maps docIDs to new values) doc values
   * packet.  We buffer these in RAM and write to disk when too much
   *  RAM is used or when a merge needs to kick off, or a commit/refresh. */
public:
  // C++ WARNING: The following method was originally marked 'synchronized':
  void
  addDVUpdate(std::shared_ptr<DocValuesFieldUpdates> update) ;

  // C++ WARNING: The following method was originally marked 'synchronized':
  int64_t getNumDVUpdates();

  /** Returns a {@link SegmentReader}. */
  // C++ WARNING: The following method was originally marked 'synchronized':
  std::shared_ptr<SegmentReader>
  getReader(std::shared_ptr<IOContext> context) ;

  // C++ WARNING: The following method was originally marked 'synchronized':
  void release(std::shared_ptr<SegmentReader> sr) ;

  // C++ WARNING: The following method was originally marked 'synchronized':
  bool delete_(int docID) ;

  // NOTE: removes callers ref
  // C++ WARNING: The following method was originally marked 'synchronized':
  void dropReaders() ;

  /**
   * Returns a ref to a clone. NOTE: you should decRef() the reader when you're
   * done (ie do not call close()).
   */
  // C++ WARNING: The following method was originally marked 'synchronized':
  std::shared_ptr<SegmentReader>
  getReadOnlyClone(std::shared_ptr<IOContext> context) ;

  // C++ WARNING: The following method was originally marked 'synchronized':
  int numDeletesToMerge(std::shared_ptr<MergePolicy> policy) ;

private:
  // C++ WARNING: The following method was originally marked 'synchronized':
  std::shared_ptr<CodecReader> getLatestReader() ;

  /**
   * Returns a snapshot of the live docs.
   */
public:
  // C++ WARNING: The following method was originally marked 'synchronized':
  std::shared_ptr<Bits> getLiveDocs();

  /**
   * Returns the live-docs bits excluding documents that are not live due to
   * soft-deletes
   */
  // C++ WARNING: The following method was originally marked 'synchronized':
  std::shared_ptr<Bits> getHardLiveDocs();

  // C++ WARNING: The following method was originally marked 'synchronized':
  void dropChanges();

  // Commit live docs (writes new _X_N.del files) and field updates (writes new
  // _X_N updates files) to the directory; returns true if it wrote any file
  // and false if there were no new deletes or updates to write:
  // C++ WARNING: The following method was originally marked 'synchronized':
  bool writeLiveDocs(std::shared_ptr<Directory> dir) ;

private:
  // C++ WARNING: The following method was originally marked 'synchronized':
  void
  handleDVUpdates(std::shared_ptr<FieldInfos> infos,
                  std::shared_ptr<Directory> dir,
                  std::shared_ptr<DocValuesFormat> dvFormat,
                  std::shared_ptr<SegmentReader> reader,
                  std::unordered_map<int, Set<std::wstring>> &fieldFiles,
                  int64_t maxDelGen,
                  std::shared_ptr<InfoStream> infoStream) ;

private:
  class EmptyDocValuesProducerAnonymousInnerClass
      : public EmptyDocValuesProducer
  {
    GET_CLASS_NAME(EmptyDocValuesProducerAnonymousInnerClass)
  private:
    std::shared_ptr<ReadersAndUpdates> outerInstance;

    std::shared_ptr<org::apache::lucene::index::SegmentReader> reader;
    std::wstring field;
    std::shared_ptr<org::apache::lucene::index::FieldInfo> fieldInfo;
    std::function<DocValuesFieldUpdates::Iterator *(FieldInfo *)>
        updateSupplier;

  public:
    EmptyDocValuesProducerAnonymousInnerClass(
        std::shared_ptr<ReadersAndUpdates> outerInstance,
        std::shared_ptr<org::apache::lucene::index::SegmentReader> reader,
        const std::wstring &field,
        std::shared_ptr<org::apache::lucene::index::FieldInfo> fieldInfo,
        std::function<DocValuesFieldUpdates::Iterator *(FieldInfo *)>
            &updateSupplier);

    std::shared_ptr<BinaryDocValues> getBinary(
        std::shared_ptr<FieldInfo> fieldInfoIn)  override;

  private:
    class BinaryDocValuesAnonymousInnerClass : public BinaryDocValues
    {
      GET_CLASS_NAME(BinaryDocValuesAnonymousInnerClass)
    private:
      std::shared_ptr<EmptyDocValuesProducerAnonymousInnerClass> outerInstance;

      std::shared_ptr<org::apache::lucene::index::ReadersAndUpdates::
                          MergedDocValues<std::shared_ptr<BinaryDocValues>>>
          mergedDocValues;

    public:
      BinaryDocValuesAnonymousInnerClass(
          std::shared_ptr<EmptyDocValuesProducerAnonymousInnerClass>
              outerInstance,
          std::shared_ptr<org::apache::lucene::index::ReadersAndUpdates::
                              MergedDocValues<std::shared_ptr<BinaryDocValues>>>
              mergedDocValues);

      std::shared_ptr<BytesRef> binaryValue()  override;
      bool advanceExact(int target) override;
      int docID() override;
      int nextDoc()  override;
      int advance(int target) override;
      int64_t cost() override;

    protected:
      std::shared_ptr<BinaryDocValuesAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<BinaryDocValuesAnonymousInnerClass>(
            BinaryDocValues::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<EmptyDocValuesProducerAnonymousInnerClass>
    shared_from_this()
    {
      return std::static_pointer_cast<
          EmptyDocValuesProducerAnonymousInnerClass>(
          EmptyDocValuesProducer::shared_from_this());
    }
  };

private:
  class EmptyDocValuesProducerAnonymousInnerClass2
      : public EmptyDocValuesProducer
  {
    GET_CLASS_NAME(EmptyDocValuesProducerAnonymousInnerClass2)
  private:
    std::shared_ptr<ReadersAndUpdates> outerInstance;

    std::shared_ptr<org::apache::lucene::index::SegmentReader> reader;
    std::wstring field;
    std::shared_ptr<org::apache::lucene::index::FieldInfo> fieldInfo;
    std::function<DocValuesFieldUpdates::Iterator *(FieldInfo *)>
        updateSupplier;

  public:
    EmptyDocValuesProducerAnonymousInnerClass2(
        std::shared_ptr<ReadersAndUpdates> outerInstance,
        std::shared_ptr<org::apache::lucene::index::SegmentReader> reader,
        const std::wstring &field,
        std::shared_ptr<org::apache::lucene::index::FieldInfo> fieldInfo,
        std::function<DocValuesFieldUpdates::Iterator *(FieldInfo *)>
            &updateSupplier);

    std::shared_ptr<NumericDocValues> getNumeric(
        std::shared_ptr<FieldInfo> fieldInfoIn)  override;

  private:
    class NumericDocValuesAnonymousInnerClass : public NumericDocValues
    {
      GET_CLASS_NAME(NumericDocValuesAnonymousInnerClass)
    private:
      std::shared_ptr<EmptyDocValuesProducerAnonymousInnerClass2> outerInstance;

      std::shared_ptr<org::apache::lucene::index::ReadersAndUpdates::
                          MergedDocValues<std::shared_ptr<NumericDocValues>>>
          mergedDocValues;

    public:
      NumericDocValuesAnonymousInnerClass(
          std::shared_ptr<EmptyDocValuesProducerAnonymousInnerClass2>
              outerInstance,
          std::shared_ptr<
              org::apache::lucene::index::ReadersAndUpdates::MergedDocValues<
                  std::shared_ptr<NumericDocValues>>>
              mergedDocValues);

      int64_t longValue()  override;
      bool advanceExact(int target) override;
      int docID() override;
      int nextDoc()  override;
      int advance(int target) override;
      int64_t cost() override;

    protected:
      std::shared_ptr<NumericDocValuesAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<NumericDocValuesAnonymousInnerClass>(
            NumericDocValues::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<EmptyDocValuesProducerAnonymousInnerClass2>
    shared_from_this()
    {
      return std::static_pointer_cast<
          EmptyDocValuesProducerAnonymousInnerClass2>(
          EmptyDocValuesProducer::shared_from_this());
    }
  };

  /**
   * This class merges the current on-disk DV with an incoming update DV
   * instance and merges the two instances giving the incoming update precedence
   * in terms of values, in other words the values of the update always wins
   * over the on-disk version.
   */
public:
  template <typename DocValuesInstance>
  class MergedDocValues final : public DocValuesIterator
  {
    GET_CLASS_NAME(MergedDocValues)
    static_assert(std::is_base_of<DocValuesIterator, DocValuesInstance>::value,
                  L"DocValuesInstance must inherit from DocValuesIterator");

  private:
    const std::shared_ptr<DocValuesFieldUpdates::Iterator> updateIterator;
    // merged docID
    int docIDOut = -1;
    // docID from our original doc values
    int docIDOnDisk = -1;
    // docID from our updates
    int updateDocID = -1;

    const DocValuesInstance onDiskDocValues;
    const DocValuesInstance updateDocValues;

  public:
    DocValuesInstance currentValuesSupplier;

  protected:
    MergedDocValues(
        DocValuesInstance onDiskDocValues, DocValuesInstance updateDocValues,
        std::shared_ptr<DocValuesFieldUpdates::Iterator> updateIterator)
        : updateIterator(updateIterator), onDiskDocValues(onDiskDocValues),
          updateDocValues(updateDocValues)
    {
    }

  public:
    int docID() override { return docIDOut; }

    int advance(int target) override
    {
      throw std::make_shared<UnsupportedOperationException>();
    }

    bool advanceExact(int target) override
    {
      throw std::make_shared<UnsupportedOperationException>();
    }

    int64_t cost() override { return onDiskDocValues->cost(); }

    int nextDoc()  override
    {
      bool hasValue = false;
      do {
        if (docIDOnDisk == docIDOut) {
          if (onDiskDocValues == nullptr) {
            docIDOnDisk = NO_MORE_DOCS;
          } else {
            docIDOnDisk = onDiskDocValues->nextDoc();
          }
        }
        if (updateDocID == docIDOut) {
          updateDocID = updateDocValues->nextDoc();
        }
        if (docIDOnDisk < updateDocID) {
          // no update to this doc - we use the on-disk values
          docIDOut = docIDOnDisk;
          currentValuesSupplier = onDiskDocValues;
          hasValue = true;
        } else {
          docIDOut = updateDocID;
          if (docIDOut != NO_MORE_DOCS) {
            currentValuesSupplier = updateDocValues;
            hasValue = updateIterator->hasValue();
          } else {
            hasValue = true;
          }
        }
      } while (hasValue == false);
      return docIDOut;
    }

  protected:
    std::shared_ptr<MergedDocValues> shared_from_this()
    {
      return std::static_pointer_cast<MergedDocValues>(
          DocValuesIterator::shared_from_this());
    }
  };

private:
  // C++ WARNING: The following method was originally marked 'synchronized':
  std::shared_ptr<Set<std::wstring>> writeFieldInfosGen(
      std::shared_ptr<FieldInfos> fieldInfos, std::shared_ptr<Directory> dir,
      std::shared_ptr<FieldInfosFormat> infosFormat) ;

public:
  // C++ WARNING: The following method was originally marked 'synchronized':
  bool
  writeFieldUpdates(std::shared_ptr<Directory> dir,
                    std::shared_ptr<FieldInfos::FieldNumbers> fieldNumbers,
                    int64_t maxDelGen,
                    std::shared_ptr<InfoStream> infoStream) ;

private:
  std::shared_ptr<SegmentReader> createNewReaderWithLatestLiveDocs(
      std::shared_ptr<SegmentReader> reader) ;

  void swapNewReaderWithLatestLiveDocs() ;

public:
  // C++ WARNING: The following method was originally marked 'synchronized':
  void setIsMerging();

  // C++ WARNING: The following method was originally marked 'synchronized':
  bool isMerging();

public:
  class MergeReader final : public std::enable_shared_from_this<MergeReader>
  {
    GET_CLASS_NAME(MergeReader)
  public:
    const std::shared_ptr<SegmentReader> reader;
    const std::shared_ptr<Bits> hardLiveDocs;

    MergeReader(std::shared_ptr<SegmentReader> reader,
                std::shared_ptr<Bits> hardLiveDocs);
  };

  /** Returns a reader for merge, with the latest doc values updates and
   * deletions. */
public:
  // C++ WARNING: The following method was originally marked 'synchronized':
  std::shared_ptr<MergeReader>
  getReaderForMerge(std::shared_ptr<IOContext> context) ;

  /**
   * Drops all merging updates. Called from IndexWriter after this segment
   * finished merging (whether successfully or not).
   */
  // C++ WARNING: The following method was originally marked 'synchronized':
  void dropMergingUpdates();

  // C++ WARNING: The following method was originally marked 'synchronized':
  std::unordered_map<std::wstring,
                     std::deque<std::shared_ptr<DocValuesFieldUpdates>>>
  getMergingDVUpdates();

  virtual std::wstring toString();

  // C++ WARNING: The following method was originally marked 'synchronized':
  bool isFullyDeleted() ;

  bool keepFullyDeletedSegment(std::shared_ptr<MergePolicy> mergePolicy) throw(
      IOException);
};

} // #include  "core/src/java/org/apache/lucene/index/
