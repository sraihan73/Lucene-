#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <algorithm>
#include <any>
#include <iostream>
#include <limits>
#include <deque>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/Directory.h"

#include  "core/src/java/org/apache/lucene/store/Lock.h"
#include  "core/src/java/org/apache/lucene/index/SegmentInfoStatus.h"
#include  "core/src/java/org/apache/lucene/index/SegmentInfos.h"
#include  "core/src/java/org/apache/lucene/codecs/Codec.h"
#include  "core/src/java/org/apache/lucene/index/LiveDocStatus.h"
#include  "core/src/java/org/apache/lucene/index/FieldInfoStatus.h"
#include  "core/src/java/org/apache/lucene/index/FieldNormStatus.h"
#include  "core/src/java/org/apache/lucene/index/TermIndexStatus.h"
#include  "core/src/java/org/apache/lucene/index/StoredFieldStatus.h"
#include  "core/src/java/org/apache/lucene/index/TermVectorStatus.h"
#include  "core/src/java/org/apache/lucene/index/DocValuesStatus.h"
#include  "core/src/java/org/apache/lucene/index/PointsStatus.h"
#include  "core/src/java/org/apache/lucene/index/IndexSortStatus.h"
#include  "core/src/java/org/apache/lucene/index/CodecReader.h"
#include  "core/src/java/org/apache/lucene/search/Sort.h"
#include  "core/src/java/org/apache/lucene/index/TermsEnum.h"
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"
#include  "core/src/java/org/apache/lucene/util/FixedBitSet.h"
#include  "core/src/java/org/apache/lucene/index/Terms.h"
#include  "core/src/java/org/apache/lucene/index/FieldInfos.h"
#include  "core/src/java/org/apache/lucene/index/Fields.h"
#include  "core/src/java/org/apache/lucene/util/Bits.h"
#include  "core/src/java/org/apache/lucene/util/Version.h"
#include  "core/src/java/org/apache/lucene/index/PointValues.h"
#include  "core/src/java/org/apache/lucene/index/DocValuesIterator.h"
#include  "core/src/java/org/apache/lucene/index/FieldInfo.h"
#include  "core/src/java/org/apache/lucene/index/BinaryDocValues.h"
#include  "core/src/java/org/apache/lucene/index/SortedDocValues.h"
#include  "core/src/java/org/apache/lucene/index/SortedSetDocValues.h"
#include  "core/src/java/org/apache/lucene/index/SortedNumericDocValues.h"
#include  "core/src/java/org/apache/lucene/index/NumericDocValues.h"
#include  "core/src/java/org/apache/lucene/codecs/DocValuesProducer.h"
#include  "core/src/java/org/apache/lucene/index/SegmentCommitInfo.h"
#include  "core/src/java/org/apache/lucene/index/SegmentReader.h"

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

using Codec = org::apache::lucene::codecs::Codec;
using DocValuesProducer = org::apache::lucene::codecs::DocValuesProducer;
using DocValuesStatus =
    org::apache::lucene::index::CheckIndex::Status::DocValuesStatus;
using IntersectVisitor =
    org::apache::lucene::index::PointValues::IntersectVisitor;
using Relation = org::apache::lucene::index::PointValues::Relation;
using Sort = org::apache::lucene::search::Sort;
using Directory = org::apache::lucene::store::Directory;
using Lock = org::apache::lucene::store::Lock;
using Bits = org::apache::lucene::util::Bits;
using BytesRef = org::apache::lucene::util::BytesRef;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;
using Version = org::apache::lucene::util::Version;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.search.DocIdSetIterator.NO_MORE_DOCS;

/**
 * Basic tool and API to check the health of an index and
 * write a new segments file that removes reference to
 * problematic segments.
 *
 * <p>As this tool checks every byte in the index, on a large
 * index it can take quite a long time to run.
 *
 * @lucene.experimental Please make a complete backup of your
 * index before using this to exorcise corrupted documents from your index!
 */
class CheckIndex final : public std::enable_shared_from_this<CheckIndex>
{
  GET_CLASS_NAME(CheckIndex)

private:
  std::shared_ptr<PrintStream> infoStream;
  std::shared_ptr<Directory> dir;
  std::shared_ptr<Lock> writeLock;
  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: private volatile bool closed;
  bool closed = false;

  /**
   * Returned from {@link #checkIndex()} detailing the health and status of the
   *index.
   *
   * @lucene.experimental
   **/

public:
  class Status : public std::enable_shared_from_this<Status>
  {
    GET_CLASS_NAME(Status)

  public:
    Status();

    /** True if no problems were found with the index. */
    bool clean = false;

    /** True if we were unable to locate and load the segments_N file. */
    bool missingSegments = false;

    /** True if we were unable to open the segments_N file. */
    bool cantOpenSegments = false;

    /** True if we were unable to read the version number from segments_N file.
     */
    bool missingSegmentVersion = false;

    /** Name of latest segments_N file in the index. */
    std::wstring segmentsFileName;

    /** Number of segments in the index. */
    int numSegments = 0;

    /** Empty unless you passed specific segments deque to check as optional 3rd
     * argument.
     *  @see CheckIndex#checkIndex(List) */
    std::deque<std::wstring> segmentsChecked = std::deque<std::wstring>();

    /** True if the index was created with a newer version of Lucene than the
     * CheckIndex tool. */
    bool toolOutOfDate = false;

    /** List of {@link SegmentInfoStatus} instances, detailing status of each
     * segment. */
    std::deque<std::shared_ptr<SegmentInfoStatus>> segmentInfos =
        std::deque<std::shared_ptr<SegmentInfoStatus>>();

    /** Directory index is in. */
    std::shared_ptr<Directory> dir;

    /**
     * SegmentInfos instance containing only segments that
     * had no problems (this is used with the {@link CheckIndex#exorciseIndex}
     * method to repair the index.
     */
    std::shared_ptr<SegmentInfos> newSegments;

    /** How many documents will be lost to bad segments. */
    int totLoseDocCount = 0;

    /** How many bad segments were found. */
    int numBadSegments = 0;

    /** True if we checked only specific segments ({@link
     * #checkIndex(List)}) was called with non-null
     * argument). */
    bool partial_ = false;

    /** The greatest segment name. */
    int64_t maxSegmentName = 0;

    /** Whether the SegmentInfos.counter is greater than any of the segments'
     * names. */
    bool validCounter = false;

    /** Holds the userData of the last commit in the index */
    std::unordered_map<std::wstring, std::wstring> userData;

    /** Holds the status of each segment in the index.
     *  See {@link #segmentInfos}.
     *
     * @lucene.experimental
     */
  public:
    class SegmentInfoStatus
        : public std::enable_shared_from_this<SegmentInfoStatus>
    {
      GET_CLASS_NAME(SegmentInfoStatus)

    public:
      SegmentInfoStatus();

      /** Name of the segment. */
      std::wstring name;

      /** Codec used to read this segment. */
      std::shared_ptr<Codec> codec;

      /** Document count (does not take deletions into account). */
      int maxDoc = 0;

      /** True if segment is compound file format. */
      bool compound = false;

      /** Number of files referenced by this segment. */
      int numFiles = 0;

      /** Net size (MB) of the files referenced by this
       *  segment. */
      double sizeMB = 0;

      /** True if this segment has pending deletions. */
      bool hasDeletions = false;

      /** Current deletions generation. */
      int64_t deletionsGen = 0;

      /** True if we were able to open a CodecReader on this
       *  segment. */
      bool openReaderPassed = false;

      /** Map that includes certain
       *  debugging details that IndexWriter records into
       *  each segment it creates */
      std::unordered_map<std::wstring, std::wstring> diagnostics;

      /** Status for testing of livedocs */
      std::shared_ptr<LiveDocStatus> liveDocStatus;

      /** Status for testing of field infos */
      std::shared_ptr<FieldInfoStatus> fieldInfoStatus;

      /** Status for testing of field norms (null if field norms could not be
       * tested). */
      std::shared_ptr<FieldNormStatus> fieldNormStatus;

      /** Status for testing of indexed terms (null if indexed terms could not
       * be tested). */
      std::shared_ptr<TermIndexStatus> termIndexStatus;

      /** Status for testing of stored fields (null if stored fields could not
       * be tested). */
      std::shared_ptr<StoredFieldStatus> storedFieldStatus;

      /** Status for testing of term vectors (null if term vectors could not be
       * tested). */
      std::shared_ptr<TermVectorStatus> termVectorStatus;

      /** Status for testing of DocValues (null if DocValues could not be
       * tested). */
      std::shared_ptr<DocValuesStatus> docValuesStatus;

      /** Status for testing of PointValues (null if PointValues could not be
       * tested). */
      std::shared_ptr<PointsStatus> pointsStatus;

      /** Status of index sort */
      std::shared_ptr<IndexSortStatus> indexSortStatus;
    };

    /**
     * Status from testing livedocs
     */
  public:
    class LiveDocStatus final
        : public std::enable_shared_from_this<LiveDocStatus>
    {
      GET_CLASS_NAME(LiveDocStatus)
    private:
      LiveDocStatus();

      /** Number of deleted documents. */
    public:
      int numDeleted = 0;

      /** Exception thrown during term index test (null on success) */
      std::runtime_error error = nullptr;
    };

    /**
     * Status from testing field infos.
     */
  public:
    class FieldInfoStatus final
        : public std::enable_shared_from_this<FieldInfoStatus>
    {
      GET_CLASS_NAME(FieldInfoStatus)
    private:
      FieldInfoStatus();

      /** Number of fields successfully tested */
    public:
      int64_t totFields = 0LL;

      /** Exception thrown during term index test (null on success) */
      std::runtime_error error = nullptr;
    };

    /**
     * Status from testing field norms.
     */
  public:
    class FieldNormStatus final
        : public std::enable_shared_from_this<FieldNormStatus>
    {
      GET_CLASS_NAME(FieldNormStatus)
    private:
      FieldNormStatus();

      /** Number of fields successfully tested */
    public:
      int64_t totFields = 0LL;

      /** Exception thrown during term index test (null on success) */
      std::runtime_error error = nullptr;
    };

    /**
     * Status from testing term index.
     */
  public:
    class TermIndexStatus final
        : public std::enable_shared_from_this<TermIndexStatus>
    {
      GET_CLASS_NAME(TermIndexStatus)

    public:
      TermIndexStatus();

      /** Number of terms with at least one live doc. */
      int64_t termCount = 0LL;

      /** Number of terms with zero live docs docs. */
      int64_t delTermCount = 0LL;

      /** Total frequency across all terms. */
      int64_t totFreq = 0LL;

      /** Total number of positions. */
      int64_t totPos = 0LL;

      /** Exception thrown during term index test (null on success) */
      std::runtime_error error = nullptr;

      /** Holds details of block allocations in the block
       *  tree terms dictionary (this is only set if the
       *  {@link PostingsFormat} for this segment uses block
       *  tree. */
      std::unordered_map<std::wstring, std::any> blockTreeStats;
    };

    /**
     * Status from testing stored fields.
     */
  public:
    class StoredFieldStatus final
        : public std::enable_shared_from_this<StoredFieldStatus>
    {
      GET_CLASS_NAME(StoredFieldStatus)

    public:
      StoredFieldStatus();

      /** Number of documents tested. */
      int docCount = 0;

      /** Total number of stored fields tested. */
      int64_t totFields = 0;

      /** Exception thrown during stored fields test (null on success) */
      std::runtime_error error = nullptr;
    };

    /**
     * Status from testing stored fields.
     */
  public:
    class TermVectorStatus final
        : public std::enable_shared_from_this<TermVectorStatus>
    {
      GET_CLASS_NAME(TermVectorStatus)

    public:
      TermVectorStatus();

      /** Number of documents tested. */
      int docCount = 0;

      /** Total number of term vectors tested. */
      int64_t totVectors = 0;

      /** Exception thrown during term deque test (null on success) */
      std::runtime_error error = nullptr;
    };

    /**
     * Status from testing DocValues
     */
  public:
    class DocValuesStatus final
        : public std::enable_shared_from_this<DocValuesStatus>
    {
      GET_CLASS_NAME(DocValuesStatus)

    public:
      std::shared_ptr<internal> DocValuesStatus();

      /** Total number of docValues tested. */
      int64_t totalValueFields = 0;

      /** Total number of numeric fields */
      int64_t totalNumericFields = 0;

      /** Total number of binary fields */
      int64_t totalBinaryFields = 0;

      /** Total number of sorted fields */
      int64_t totalSortedFields = 0;

      /** Total number of sortednumeric fields */
      int64_t totalSortedNumericFields = 0;

      /** Total number of sortedset fields */
      int64_t totalSortedSetFields = 0;

      /** Exception thrown during doc values test (null on success) */
      std::runtime_error error = nullptr;
    };

    /**
     * Status from testing PointValues
     */
  public:
    class PointsStatus final : public std::enable_shared_from_this<PointsStatus>
    {
      GET_CLASS_NAME(PointsStatus)

    public:
      PointsStatus();

      /** Total number of values points tested. */
      int64_t totalValuePoints = 0;

      /** Total number of fields with points. */
      int totalValueFields = 0;

      /** Exception thrown during doc values test (null on success) */
      std::runtime_error error = nullptr;
    };

    /**
     * Status from testing index sort
     */
  public:
    class IndexSortStatus final
        : public std::enable_shared_from_this<IndexSortStatus>
    {
      GET_CLASS_NAME(IndexSortStatus)
    public:
      IndexSortStatus();

      /** Exception thrown during term index test (null on success) */
      std::runtime_error error = nullptr;
    };
  };

  /** Create a new CheckIndex on the directory. */
public:
  CheckIndex(std::shared_ptr<Directory> dir) ;

  /**
   * Expert: create a directory with the specified lock.
   * This should really not be used except for unit tests!!!!
   * It exists only to support special tests (such as
   * TestIndexWriterExceptions*), that would otherwise be more complicated to
   * debug if they had to close the writer for each check.
   */
  CheckIndex(std::shared_ptr<Directory> dir, std::shared_ptr<Lock> writeLock);

private:
  void ensureOpen();

public:
  virtual ~CheckIndex();

private:
  bool crossCheckTermVectors = false;

  /** If true, term vectors are compared against postings to
   *  make sure they are the same.  This will likely
   *  drastically increase time it takes to run CheckIndex! */
public:
  void setCrossCheckTermVectors(bool v);

  /** See {@link #setCrossCheckTermVectors}. */
  bool getCrossCheckTermVectors();

private:
  bool failFast = false;

  /** If true, just throw the original exception immediately when
   *  corruption is detected, rather than continuing to iterate to other
   *  segments looking for more corruption.  */
public:
  void setFailFast(bool v);

  /** See {@link #setFailFast}. */
  bool getFailFast();

private:
  bool verbose = false;

  /** See {@link #getChecksumsOnly}. */
public:
  bool getChecksumsOnly();

  /**
   * If true, only validate physical integrity for all files.
   * Note that the returned nested status objects (e.g. storedFieldStatus) will
   * be null.  */
  void setChecksumsOnly(bool v);

private:
  bool checksumsOnly = false;

  /** Set infoStream where messages should go.  If null, no
   *  messages are printed.  If verbose is true then more
   *  details are printed. */
public:
  void setInfoStream(std::shared_ptr<PrintStream> out, bool verbose);

  /** Set infoStream where messages should go. See {@link
   * #setInfoStream(PrintStream,bool)}. */
  void setInfoStream(std::shared_ptr<PrintStream> out);

private:
  static void msg(std::shared_ptr<PrintStream> out, const std::wstring &msg);

  /** Returns a {@link Status} instance detailing
   *  the state of the index.
   *
   *  <p>As this method checks every byte in the index, on a large
   *  index it can take quite a long time to run.
   *
   *  <p><b>WARNING</b>: make sure
   *  you only call this when the index is not opened by any
   *  writer. */
public:
  std::shared_ptr<Status> checkIndex() ;

  /** Returns a {@link Status} instance detailing
   *  the state of the index.
   *
   *  @param onlySegments deque of specific segment names to check
   *
   *  <p>As this method checks every byte in the specified
   *  segments, on a large index it can take quite a long
   *  time to run. */
  std::shared_ptr<Status>
  checkIndex(std::deque<std::wstring> &onlySegments) ;

  /**
   * Tests index sort order.
   * @lucene.experimental
   */
  static std::shared_ptr<Status::IndexSortStatus>
  testSort(std::shared_ptr<CodecReader> reader, std::shared_ptr<Sort> sort,
           std::shared_ptr<PrintStream> infoStream,
           bool failFast) ;

  /**
   * Test live docs.
   * @lucene.experimental
   */
  static std::shared_ptr<Status::LiveDocStatus>
  testLiveDocs(std::shared_ptr<CodecReader> reader,
               std::shared_ptr<PrintStream> infoStream,
               bool failFast) ;

  /**
   * Test field infos.
   * @lucene.experimental
   */
  static std::shared_ptr<Status::FieldInfoStatus>
  testFieldInfos(std::shared_ptr<CodecReader> reader,
                 std::shared_ptr<PrintStream> infoStream,
                 bool failFast) ;

  /**
   * Test field norms.
   * @lucene.experimental
   */
  static std::shared_ptr<Status::FieldNormStatus>
  testFieldNorms(std::shared_ptr<CodecReader> reader,
                 std::shared_ptr<PrintStream> infoStream,
                 bool failFast) ;

  /** Visits all terms in the range minTerm (inclusive) to maxTerm (exclusive),
   * marking all doc IDs encountered into allDocsSeen, and returning the total
   * number of terms visited. */
private:
  static int64_t getDocsFromTermRange(const std::wstring &field, int maxDoc,
                                        std::shared_ptr<TermsEnum> termsEnum,
                                        std::shared_ptr<FixedBitSet> docsSeen,
                                        std::shared_ptr<BytesRef> minTerm,
                                        std::shared_ptr<BytesRef> maxTerm,
                                        bool isIntersect) ;

  /** Test Terms.intersect on this range, and validates that it returns the same
   * doc ids as using non-intersect TermsEnum.  Returns true if any fake terms
   * were seen. */
  static bool checkSingleTermRange(
      const std::wstring &field, int maxDoc, std::shared_ptr<Terms> terms,
      std::shared_ptr<BytesRef> minTerm, std::shared_ptr<BytesRef> maxTerm,
      std::shared_ptr<FixedBitSet> normalDocs,
      std::shared_ptr<FixedBitSet> intersectDocs) ;

  /** Make an effort to visit "fake" (e.g. auto-prefix) terms.  We do this by
   * running term range intersections across an initially wide interval of
   * terms, at different boundaries, and then gradually decrease the interval.
   * This is not guaranteed to hit all non-real terms (doing that in general is
   * non-trivial), but it should hit many of them, and validate their postings
   * against the postings for the real terms. */
  static void checkTermRanges(const std::wstring &field, int maxDoc,
                              std::shared_ptr<Terms> terms,
                              int64_t numTerms) ;

  /**
   * checks Fields api is consistent with itself.
   * searcher is optional, to verify with queries. Can be null.
   */
  static std::shared_ptr<Status::TermIndexStatus>
  checkFields(std::shared_ptr<Fields> fields, std::shared_ptr<Bits> liveDocs,
              int maxDoc, std::shared_ptr<FieldInfos> fieldInfos, bool doPrint,
              bool isVectors, std::shared_ptr<PrintStream> infoStream,
              bool verbose,
              std::shared_ptr<Version> version) ;

  /**
   * Test the term index.
   * @lucene.experimental
   */
public:
  static std::shared_ptr<Status::TermIndexStatus>
  testPostings(std::shared_ptr<CodecReader> reader,
               std::shared_ptr<PrintStream> infoStream,
               std::shared_ptr<Version> version) ;

  /**
   * Test the term index.
   * @lucene.experimental
   */
  static std::shared_ptr<Status::TermIndexStatus>
  testPostings(std::shared_ptr<CodecReader> reader,
               std::shared_ptr<PrintStream> infoStream, bool verbose,
               bool failFast,
               std::shared_ptr<Version> version) ;

  /**
   * Test the points index
   * @lucene.experimental
   */
  static std::shared_ptr<Status::PointsStatus>
  testPoints(std::shared_ptr<CodecReader> reader,
             std::shared_ptr<PrintStream> infoStream,
             bool failFast) ;

  /** Walks the entire N-dimensional points space, verifying that all points
   * fall within the last cell's boundaries.
   *
   * @lucene.internal */
public:
  class VerifyPointsVisitor
      : public std::enable_shared_from_this<VerifyPointsVisitor>,
        public PointValues::IntersectVisitor
  {
    GET_CLASS_NAME(VerifyPointsVisitor)
  private:
    int64_t pointCountSeen = 0;
    int lastDocID = -1;
    const int maxDoc;
    const std::shared_ptr<FixedBitSet> docsSeen;
    std::deque<char> const lastMinPackedValue;
    std::deque<char> const lastMaxPackedValue;
    std::deque<char> const lastPackedValue;
    std::deque<char> const globalMinPackedValue;
    std::deque<char> const globalMaxPackedValue;
    const int packedBytesCount;
    const int numDims;
    const int bytesPerDim;
    const std::wstring fieldName;

    /** Sole constructor */
  public:
    VerifyPointsVisitor(const std::wstring &fieldName, int maxDoc,
                        std::shared_ptr<PointValues> values) ;

    /** Returns total number of points in this BKD tree */
    virtual int64_t getPointCountSeen();

    /** Returns total number of unique docIDs in this BKD tree */
    virtual int64_t getDocCountSeen();

    void visit(int docID) override;

    void visit(int docID, std::deque<char> &packedValue) override;

    PointValues::Relation compare(std::deque<char> &minPackedValue,
                                  std::deque<char> &maxPackedValue) override;

  private:
    void checkPackedValue(const std::wstring &desc,
                          std::deque<char> &packedValue, int docID);
  };

private:
  class ConstantRelationIntersectVisitor
      : public std::enable_shared_from_this<ConstantRelationIntersectVisitor>,
        public IntersectVisitor
  {
    GET_CLASS_NAME(ConstantRelationIntersectVisitor)
  private:
    const Relation relation;

  public:
    ConstantRelationIntersectVisitor(Relation relation);

    void visit(int docID)  override;

    void visit(int docID,
               std::deque<char> &packedValue)  override;

    Relation compare(std::deque<char> &minPackedValue,
                     std::deque<char> &maxPackedValue) override;
  };

  /**
   * Test stored fields.
   * @lucene.experimental
   */
public:
  static std::shared_ptr<Status::StoredFieldStatus>
  testStoredFields(std::shared_ptr<CodecReader> reader,
                   std::shared_ptr<PrintStream> infoStream,
                   bool failFast) ;

  /**
   * Test docvalues.
   * @lucene.experimental
   */
  static std::shared_ptr<Status::DocValuesStatus>
  testDocValues(std::shared_ptr<CodecReader> reader,
                std::shared_ptr<PrintStream> infoStream,
                bool failFast) ;

  using DocValuesIteratorSupplier =
      std::function<org::apache::lucene::index::DocValuesIterator(
          FieldInfo fi)>;

private:
  static void
  checkDVIterator(std::shared_ptr<FieldInfo> fi, int maxDoc,
                  DocValuesIteratorSupplier producer) ;

  static void
  checkBinaryDocValues(const std::wstring &fieldName, int maxDoc,
                       std::shared_ptr<BinaryDocValues> bdv) ;

  static void
  checkSortedDocValues(const std::wstring &fieldName, int maxDoc,
                       std::shared_ptr<SortedDocValues> dv) ;

  static void checkSortedSetDocValues(
      const std::wstring &fieldName, int maxDoc,
      std::shared_ptr<SortedSetDocValues> dv) ;

  static void checkSortedNumericDocValues(
      const std::wstring &fieldName, int maxDoc,
      std::shared_ptr<SortedNumericDocValues> ndv) ;

  static void checkNumericDocValues(
      const std::wstring &fieldName,
      std::shared_ptr<NumericDocValues> ndv) ;

  static void checkDocValues(
      std::shared_ptr<FieldInfo> fi,
      std::shared_ptr<DocValuesProducer> dvReader, int maxDoc,
      std::shared_ptr<PrintStream> infoStream,
      std::shared_ptr<DocValuesStatus> status) ;

  /**
   * Test term vectors.
   * @lucene.experimental
   */
public:
  static std::shared_ptr<Status::TermVectorStatus>
  testTermVectors(std::shared_ptr<CodecReader> reader,
                  std::shared_ptr<PrintStream> infoStream,
                  std::shared_ptr<Version> version) ;

  /**
   * Test term vectors.
   * @lucene.experimental
   */
  static std::shared_ptr<Status::TermVectorStatus>
  testTermVectors(std::shared_ptr<CodecReader> reader,
                  std::shared_ptr<PrintStream> infoStream, bool verbose,
                  bool crossCheckTermVectors, bool failFast,
                  std::shared_ptr<Version> version) ;

  /** Repairs the index using previously returned result
   *  from {@link #checkIndex}.  Note that this does not
   *  remove any of the unreferenced files after it's done;
   *  you must separately open an {@link IndexWriter}, which
   *  deletes unreferenced files when it's created.
   *
   * <p><b>WARNING</b>: this writes a
   *  new segments file into the index, effectively removing
   *  all documents in broken segments from the index.
   *  BE CAREFUL.
   */
  void exorciseIndex(std::shared_ptr<Status> result) ;

private:
  // C++ NOTE: Fields cannot have the same name as methods:
  static bool assertsOn_;

  static bool testAsserts();

  /**
   * Check whether asserts are enabled or not.
   * @return true iff asserts are enabled
   */
public:
  static bool assertsOn();

  /** Command-line interface to check and exorcise corrupt segments from an
    index.

    <p>
    Run it like this:
    <pre>
    java -ea:org.apache.lucene... org.apache.lucene.index.CheckIndex pathToIndex
    [-exorcise] [-verbose] [-segment X] [-segment Y]
    </pre>
    <ul>
    <li><code>-exorcise</code>: actually write a new segments_N file, removing
    any problematic segments. *LOSES DATA*

    <li><code>-segment X</code>: only check the specified
    segment(s).  This can be specified multiple times,
    to check more than one segment, eg <code>-segment _2
    -segment _a</code>.  You can't use this with the -exorcise
    option.
    </ul>

    <p><b>WARNING</b>: <code>-exorcise</code> should only be used on an
    emergency basis as it will cause documents (perhaps many) to be permanently
    removed from the index.  Always make a backup copy of your index before
    running this!  Do not run this tool on an index that is actively being
    written to.  You have been warned!

    <p>                Run without -exorcise, this tool will open the index,
    report version information and report any exceptions it hits and what action
    it would take if -exorcise were specified.  With -exorcise, this tool will
    remove any segments that have issues and write a new segments_N file.  This
    means all documents contained in the affected segments will be removed.

    <p>
                       This tool exits with exit code 1 if the index cannot be
    opened or has any corruption, else 0.
   */
  static void main(std::deque<std::wstring> &args) throw(IOException,
                                                          InterruptedException);

  /**
   * Run-time configuration options for CheckIndex commands.
   */
public:
  class Options : public std::enable_shared_from_this<Options>
  {
    GET_CLASS_NAME(Options)
  public:
    bool doExorcise = false;
    bool doCrossCheckTermVectors = false;
    bool verbose = false;
    bool doChecksumsOnly = false;
    std::deque<std::wstring> onlySegments = std::deque<std::wstring>();
    std::wstring indexPath = L"";
    std::wstring dirImpl = L"";
    std::shared_ptr<PrintStream> out = nullptr;

    /** Sole constructor. */
    Options();

    /**
     * Get the name of the FSDirectory implementation class to use.
     */
    virtual std::wstring getDirImpl();

    /**
     * Get the directory containing the index.
     */
    virtual std::wstring getIndexPath();

    /**
     * Set the PrintStream to use for reporting results.
     */
    virtual void setOut(std::shared_ptr<PrintStream> out);
  };

  // actual main: returns exit code instead of terminating JVM (for easy
  // testing)
private:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressForbidden(reason = "System.out required: command
  // line tool") private static int doMain(std::wstring args[]) throws
  // java.io.IOException, InterruptedException
  static int
  doMain(std::deque<std::wstring> &args) throw(IOException,
                                                InterruptedException);

  /**
   * Parse command line args into fields
   * @param args The command line arguments
   * @return An Options struct
   * @throws IllegalArgumentException if any of the CLI args are invalid
   */
public:
  static std::shared_ptr<Options> parseOptions(std::deque<std::wstring> &args);

  /**
   * Actually perform the index check
   * @param opts The options to use for this check
   * @return 0 iff the index is clean, 1 otherwise
   */
  int doCheck(std::shared_ptr<Options> opts) throw(IOException,
                                                   InterruptedException);

private:
  static void checkSoftDeletes(const std::wstring &softDeletesField,
                               std::shared_ptr<SegmentCommitInfo> info,
                               std::shared_ptr<SegmentReader> reader,
                               std::shared_ptr<PrintStream> infoStream,
                               bool failFast) ;

  static double nsToSec(int64_t ns);
};

} // #include  "core/src/java/org/apache/lucene/index/
