#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <iostream>
#include <limits>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/IndexWriter.h"

#include  "core/src/java/org/apache/lucene/index/ReaderManager.h"
#include  "core/src/java/org/apache/lucene/store/Directory.h"
#include  "core/src/java/org/apache/lucene/index/SegmentIDAndGen.h"
#include  "core/src/java/org/apache/lucene/index/LeafReader.h"
#include  "core/src/java/org/apache/lucene/index/IndexWriterConfig.h"
#include  "core/src/java/org/apache/lucene/util/Bits.h"
#include  "core/src/java/org/apache/lucene/index/DirectoryReader.h"
#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/index/CacheKey.h"
#include  "core/src/java/org/apache/lucene/index/ParallelLeafReader.h"
#include  "core/src/java/org/apache/lucene/index/SegmentCommitInfo.h"
#include  "core/src/java/org/apache/lucene/index/CodecReader.h"
#include  "core/src/java/org/apache/lucene/index/MergePolicy.h"
#include  "core/src/java/org/apache/lucene/index/SegmentInfos.h"
#include  "core/src/java/org/apache/lucene/index/SegmentInfo.h"
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include  "core/src/java/org/apache/lucene/search/ScoreDoc.h"

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

using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

// TODO:
//   - old parallel indices are only pruned on commit/close; can we do it on
//   refresh?

/** Simple example showing how to use ParallelLeafReader to index new
 *  stuff (postings, DVs, etc.) from previously stored fields, on the
 *  fly (during NRT reader reopen), after the  initial indexing.  The
 *  test indexes just a single stored field with text "content X" (X is
 *  a number embedded in the text).
 *
 *  Then, on reopen, for any newly created segments (flush or merge), it
 *  builds a new parallel segment by loading all stored docs, parsing
 *  out that X, and adding it as DV and numeric indexed (trie) field.
 *
 *  Finally, for searching, it builds a top-level MultiReader, with
 *  ParallelLeafReader for each segment, and then tests that random
 *  numeric range queries, and sorting by the new DV field, work
 *  correctly.
 *
 *  Each per-segment index lives in a private directory next to the main
 *  index, and they are deleted once their segments are removed from the
 *  index.  They are "volatile", meaning if e.g. the index is replicated to
 *  another machine, it's OK to not copy parallel segments indices,
 *  since they will just be regnerated (at a cost though). */

// @SuppressSysoutChecks(bugUrl="we print stuff")
// See: https://issues.apache.org/jira/browse/SOLR-12028 Tests cannot remove
// files on Windows machines occasionally
class TestDemoParallelLeafReader : public LuceneTestCase
{
  GET_CLASS_NAME(TestDemoParallelLeafReader)

public:
  static constexpr bool DEBUG = false;

public:
  class ReindexingReader : public std::enable_shared_from_this<ReindexingReader>
  {
    GET_CLASS_NAME(ReindexingReader)

    /** Key used to store the current schema gen in the SegmentInfo diagnostics
     */
  public:
    static const std::wstring SCHEMA_GEN_KEY;

    const std::shared_ptr<IndexWriter> w;
    const std::shared_ptr<ReaderManager> mgr;

  private:
    const std::shared_ptr<Directory> indexDir;
    const std::shared_ptr<Path> root;
    const std::shared_ptr<Path> segsPath;

    /** Which segments have been closed, but their parallel index is not yet not
     * removed. */
    const std::shared_ptr<Set<std::shared_ptr<SegmentIDAndGen>>>
        closedSegments = Collections::newSetFromMap(
            std::make_shared<
                ConcurrentHashMap<std::shared_ptr<SegmentIDAndGen>, bool>>());

    /** Holds currently open parallel readers for each segment. */
    const std::unordered_map<std::shared_ptr<SegmentIDAndGen>,
                             std::shared_ptr<LeafReader>>
        parallelReaders =
            std::make_shared<ConcurrentHashMap<std::shared_ptr<SegmentIDAndGen>,
                                               std::shared_ptr<LeafReader>>>();

  public:
    virtual void printRefCounts();

    ReindexingReader(std::shared_ptr<Path> root) ;

  protected:
    virtual std::shared_ptr<IndexWriterConfig> getIndexWriterConfig() = 0;

    /** Optional method to validate that the provided parallell reader in fact
     * reflects the changes in schemaGen. */
    virtual void checkParallelReader(std::shared_ptr<LeafReader> reader,
                                     std::shared_ptr<LeafReader> parallelReader,
                                     int64_t schemaGen) ;

    /** Override to customize Directory impl. */
    virtual std::shared_ptr<Directory>
    openDirectory(std::shared_ptr<Path> path) ;

  public:
    virtual void commit() ;

    virtual std::shared_ptr<LeafReader>
    getCurrentReader(std::shared_ptr<LeafReader> reader,
                     int64_t schemaGen) ;

  private:
    class ParallelLeafReaderAnonymousInnerClass : public ParallelLeafReader
    {
      GET_CLASS_NAME(ParallelLeafReaderAnonymousInnerClass)
    private:
      std::shared_ptr<ReindexingReader> outerInstance;

    public:
      ParallelLeafReaderAnonymousInnerClass(
          std::shared_ptr<ReindexingReader> outerInstance,
          std::shared_ptr<org::apache::lucene::index::LeafReader>
              parallelReader,
          std::shared_ptr<org::apache::lucene::index::LeafReader> reader);

      std::shared_ptr<Bits> getLiveDocs() override;
      int numDocs() override;

    protected:
      std::shared_ptr<ParallelLeafReaderAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<ParallelLeafReaderAnonymousInnerClass>(
            ParallelLeafReader::shared_from_this());
      }
    };

  private:
    class ParallelLeafDirectoryReader : public FilterDirectoryReader
    {
      GET_CLASS_NAME(ParallelLeafDirectoryReader)
    private:
      std::shared_ptr<TestDemoParallelLeafReader::ReindexingReader>
          outerInstance;

    public:
      ParallelLeafDirectoryReader(
          std::shared_ptr<TestDemoParallelLeafReader::ReindexingReader>
              outerInstance,
          std::shared_ptr<DirectoryReader> in_) ;

    private:
      class SubReaderWrapperAnonymousInnerClass
          : public FilterDirectoryReader::SubReaderWrapper
      {
        GET_CLASS_NAME(SubReaderWrapperAnonymousInnerClass)
      public:
        SubReaderWrapperAnonymousInnerClass();

        const int64_t currentSchemaGen =
            outerInstance->outerInstance->getCurrentSchemaGen();
        std::shared_ptr<LeafReader>
        wrap(std::shared_ptr<LeafReader> reader) override;

      protected:
        std::shared_ptr<SubReaderWrapperAnonymousInnerClass> shared_from_this()
        {
          return std::static_pointer_cast<SubReaderWrapperAnonymousInnerClass>(
              FilterDirectoryReader.SubReaderWrapper::shared_from_this());
        }
      };

    protected:
      std::shared_ptr<DirectoryReader> doWrapDirectoryReader(
          std::shared_ptr<DirectoryReader> in_)  override;

      void doClose()  override;

    public:
      std::shared_ptr<CacheHelper> getReaderCacheHelper() override;

    protected:
      std::shared_ptr<ParallelLeafDirectoryReader> shared_from_this()
      {
        return std::static_pointer_cast<ParallelLeafDirectoryReader>(
            FilterDirectoryReader::shared_from_this());
      }
    };

  public:
    virtual ~ReindexingReader();

    // Make sure we deleted all parallel indices for segments that are no longer
    // in the main index:
  private:
    void assertNoExtraSegments() ;

  private:
    class SegmentIDAndGen : public std::enable_shared_from_this<SegmentIDAndGen>
    {
      GET_CLASS_NAME(SegmentIDAndGen)
    public:
      const std::wstring segID;
      const int64_t schemaGen;

      SegmentIDAndGen(const std::wstring &segID, int64_t schemaGen);

      SegmentIDAndGen(const std::wstring &s);

      virtual int hashCode();

      bool equals(std::any _other) override;

      virtual std::wstring toString();
    };

  private:
    class ParallelReaderClosed
        : public std::enable_shared_from_this<ParallelReaderClosed>,
          public IndexReader::ClosedListener
    {
      GET_CLASS_NAME(ParallelReaderClosed)
    private:
      std::shared_ptr<TestDemoParallelLeafReader::ReindexingReader>
          outerInstance;

      const std::shared_ptr<SegmentIDAndGen> segIDGen;
      const std::shared_ptr<Directory> dir;

    public:
      ParallelReaderClosed(
          std::shared_ptr<TestDemoParallelLeafReader::ReindexingReader>
              outerInstance,
          std::shared_ptr<SegmentIDAndGen> segIDGen,
          std::shared_ptr<Directory> dir);

      void onClose(std::shared_ptr<IndexReader::CacheKey> ignored) override;
    };

    // Returns a ref
  public:
    virtual std::shared_ptr<LeafReader>
    getParallelLeafReader(std::shared_ptr<LeafReader> leaf, bool doCache,
                          int64_t schemaGen) ;

    // TODO: we could pass a writer already opened...?
  protected:
    virtual void reindex(int64_t oldSchemaGen, int64_t newSchemaGen,
                         std::shared_ptr<LeafReader> reader,
                         std::shared_ptr<Directory> parallelDir) = 0;

    /** Returns the gen for the current schema. */
    virtual int64_t getCurrentSchemaGen() = 0;

    /** Returns the gen that should be merged, meaning those changes will be
     * folded back into the main index. */
    virtual int64_t getMergingSchemaGen();

    /** Removes the parallel index that are no longer in the last commit point.
     * We can't remove this when the parallel reader is closed because it may
     * still be referenced by the last commit. */
  private:
    void pruneOldSegments(bool removeOldGens) ;

    /** Just replaces the sub-readers with parallel readers, so reindexed fields
     * are merged into new segments. */
  private:
    class ReindexingMergePolicy : public FilterMergePolicy
    {
      GET_CLASS_NAME(ReindexingMergePolicy)
    private:
      std::shared_ptr<TestDemoParallelLeafReader::ReindexingReader>
          outerInstance;

    public:
      class ReindexingOneMerge : public OneMerge
      {
        GET_CLASS_NAME(ReindexingOneMerge)
      private:
        std::shared_ptr<
            TestDemoParallelLeafReader::ReindexingReader::ReindexingMergePolicy>
            outerInstance;

      public:
        const std::deque<std::shared_ptr<ParallelLeafReader>> parallelReaders =
            std::deque<std::shared_ptr<ParallelLeafReader>>();
        const int64_t schemaGen;

        ReindexingOneMerge(
            std::shared_ptr<TestDemoParallelLeafReader::ReindexingReader::
                                ReindexingMergePolicy>
                outerInstance,
            std::deque<std::shared_ptr<SegmentCommitInfo>> &segments);

        std::shared_ptr<CodecReader> wrapForMerge(
            std::shared_ptr<CodecReader> reader)  override;

        void mergeFinished()  override;

        void setMergeInfo(std::shared_ptr<SegmentCommitInfo> info) override;

      protected:
        std::shared_ptr<ReindexingOneMerge> shared_from_this()
        {
          return std::static_pointer_cast<ReindexingOneMerge>(
              OneMerge::shared_from_this());
        }
      };

    public:
      class ReindexingMergeSpecification : public MergeSpecification
      {
        GET_CLASS_NAME(ReindexingMergeSpecification)
      private:
        std::shared_ptr<
            TestDemoParallelLeafReader::ReindexingReader::ReindexingMergePolicy>
            outerInstance;

      public:
        ReindexingMergeSpecification(
            std::shared_ptr<TestDemoParallelLeafReader::ReindexingReader::
                                ReindexingMergePolicy>
                outerInstance);

        void add(std::shared_ptr<OneMerge> merge) override;

        std::wstring segString(std::shared_ptr<Directory> dir) override;

      protected:
        std::shared_ptr<ReindexingMergeSpecification> shared_from_this()
        {
          return std::static_pointer_cast<ReindexingMergeSpecification>(
              MergeSpecification::shared_from_this());
        }
      };

    public:
      virtual std::shared_ptr<MergeSpecification>
      wrap(std::shared_ptr<MergeSpecification> spec);

      /** Create a new {@code MergePolicy} that sorts documents with the given
       * {@code sort}. */
      ReindexingMergePolicy(
          std::shared_ptr<TestDemoParallelLeafReader::ReindexingReader>
              outerInstance,
          std::shared_ptr<MergePolicy> in_);

      std::shared_ptr<MergeSpecification>
      findMerges(MergeTrigger mergeTrigger,
                 std::shared_ptr<SegmentInfos> segmentInfos,
                 std::shared_ptr<MergeContext> mergeContext) 
          override;

      std::shared_ptr<MergeSpecification> findForcedMerges(
          std::shared_ptr<SegmentInfos> segmentInfos, int maxSegmentCount,
          std::unordered_map<std::shared_ptr<SegmentCommitInfo>, bool>
              &segmentsToMerge,
          std::shared_ptr<MergeContext> mergeContext) 
          override;

      std::shared_ptr<MergeSpecification>
      findForcedDeletesMerges(std::shared_ptr<SegmentInfos> segmentInfos,
                              std::shared_ptr<MergeContext>
                                  mergeContext)  override;

      bool useCompoundFile(std::shared_ptr<SegmentInfos> segments,
                           std::shared_ptr<SegmentCommitInfo> newSegment,
                           std::shared_ptr<MergeContext>
                               mergeContext)  override;

      virtual std::wstring toString();

    protected:
      std::shared_ptr<ReindexingMergePolicy> shared_from_this()
      {
        return std::static_pointer_cast<ReindexingMergePolicy>(
            FilterMergePolicy::shared_from_this());
      }
    };

  public:
    static int64_t getSchemaGen(std::shared_ptr<SegmentInfo> info);
  };

private:
  std::shared_ptr<ReindexingReader>
  getReindexer(std::shared_ptr<Path> root) ;

private:
  class ReindexingReaderAnonymousInnerClass : public ReindexingReader
  {
    GET_CLASS_NAME(ReindexingReaderAnonymousInnerClass)
  private:
    std::shared_ptr<TestDemoParallelLeafReader> outerInstance;

  public:
    ReindexingReaderAnonymousInnerClass(
        std::shared_ptr<TestDemoParallelLeafReader> outerInstance,
        std::shared_ptr<Path> root);

  protected:
    std::shared_ptr<IndexWriterConfig>
    getIndexWriterConfig()  override;

    std::shared_ptr<Directory>
    openDirectory(std::shared_ptr<Path> path)  override;

    void
    reindex(int64_t oldSchemaGen, int64_t newSchemaGen,
            std::shared_ptr<LeafReader> reader,
            std::shared_ptr<Directory> parallelDir)  override;

    int64_t getCurrentSchemaGen() override;

  protected:
    std::shared_ptr<ReindexingReaderAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ReindexingReaderAnonymousInnerClass>(
          ReindexingReader::shared_from_this());
    }
  };

  /** Schema change by adding a new number_<schemaGen> DV field each time. */
private:
  std::shared_ptr<ReindexingReader> getReindexerNewDVFields(
      std::shared_ptr<Path> root,
      std::shared_ptr<AtomicLong> currentSchemaGen) ;

private:
  class ReindexingReaderAnonymousInnerClass2 : public ReindexingReader
  {
    GET_CLASS_NAME(ReindexingReaderAnonymousInnerClass2)
  private:
    std::shared_ptr<TestDemoParallelLeafReader> outerInstance;

    std::shared_ptr<AtomicLong> currentSchemaGen;

  public:
    ReindexingReaderAnonymousInnerClass2(
        std::shared_ptr<TestDemoParallelLeafReader> outerInstance,
        std::shared_ptr<Path> root,
        std::shared_ptr<AtomicLong> currentSchemaGen);

  protected:
    std::shared_ptr<IndexWriterConfig>
    getIndexWriterConfig()  override;

    std::shared_ptr<Directory>
    openDirectory(std::shared_ptr<Path> path)  override;

    void
    reindex(int64_t oldSchemaGen, int64_t newSchemaGen,
            std::shared_ptr<LeafReader> reader,
            std::shared_ptr<Directory> parallelDir)  override;

    int64_t getCurrentSchemaGen() override;

    void checkParallelReader(std::shared_ptr<LeafReader> r,
                             std::shared_ptr<LeafReader> parR,
                             int64_t schemaGen)  override;

  protected:
    std::shared_ptr<ReindexingReaderAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<ReindexingReaderAnonymousInnerClass2>(
          ReindexingReader::shared_from_this());
    }
  };

  /** Schema change by adding changing how the same "number" DV field is
   * indexed. */
private:
  std::shared_ptr<ReindexingReader> getReindexerSameDVField(
      std::shared_ptr<Path> root, std::shared_ptr<AtomicLong> currentSchemaGen,
      std::shared_ptr<AtomicLong> mergingSchemaGen) ;

private:
  class ReindexingReaderAnonymousInnerClass3 : public ReindexingReader
  {
    GET_CLASS_NAME(ReindexingReaderAnonymousInnerClass3)
  private:
    std::shared_ptr<TestDemoParallelLeafReader> outerInstance;

    std::shared_ptr<AtomicLong> currentSchemaGen;
    std::shared_ptr<AtomicLong> mergingSchemaGen;

  public:
    ReindexingReaderAnonymousInnerClass3(
        std::shared_ptr<TestDemoParallelLeafReader> outerInstance,
        std::shared_ptr<Path> root,
        std::shared_ptr<AtomicLong> currentSchemaGen,
        std::shared_ptr<AtomicLong> mergingSchemaGen);

  protected:
    std::shared_ptr<IndexWriterConfig>
    getIndexWriterConfig()  override;

    std::shared_ptr<Directory>
    openDirectory(std::shared_ptr<Path> path)  override;

    void
    reindex(int64_t oldSchemaGen, int64_t newSchemaGen,
            std::shared_ptr<LeafReader> reader,
            std::shared_ptr<Directory> parallelDir)  override;

    int64_t getCurrentSchemaGen() override;

    int64_t getMergingSchemaGen() override;

    void checkParallelReader(std::shared_ptr<LeafReader> r,
                             std::shared_ptr<LeafReader> parR,
                             int64_t schemaGen)  override;

  protected:
    std::shared_ptr<ReindexingReaderAnonymousInnerClass3> shared_from_this()
    {
      return std::static_pointer_cast<ReindexingReaderAnonymousInnerClass3>(
          ReindexingReader::shared_from_this());
    }
  };

public:
  virtual void testBasicMultipleSchemaGens() ;

  virtual void testRandomMultipleSchemaGens() ;

  /** First schema change creates a new "number" DV field off the stored field;
   * subsequent changes just change the value of that number
   *  field for all docs. */
  virtual void
  testRandomMultipleSchemaGensSameField() ;

  virtual void testBasic() ;

  virtual void testRandom() ;

private:
  static void
  checkAllNumberDVs(std::shared_ptr<IndexReader> r) ;

  static void checkAllNumberDVs(std::shared_ptr<IndexReader> r,
                                const std::wstring &fieldName, bool doThrow,
                                int multiplier) ;

  static void
  testNumericDVSort(std::shared_ptr<IndexSearcher> s) ;

  static void
  testPointRangeQuery(std::shared_ptr<IndexSearcher> s) ;

private:
  class ComparatorAnonymousInnerClass
      : public std::enable_shared_from_this<ComparatorAnonymousInnerClass>,
        public Comparator<std::shared_ptr<ScoreDoc>>
  {
    GET_CLASS_NAME(ComparatorAnonymousInnerClass)
  public:
    ComparatorAnonymousInnerClass();

    int compare(std::shared_ptr<ScoreDoc> a, std::shared_ptr<ScoreDoc> b);
  };

  // TODO: maybe the leading id could be further restricted?  It's from
  // StringHelper.idToString:
public:
  static const std::shared_ptr<Pattern> SEG_GEN_SUB_DIR_PATTERN;

private:
  static std::deque<std::shared_ptr<Path>>
  segSubDirs(std::shared_ptr<Path> segsPath) ;

  // TODO: test exceptions

protected:
  std::shared_ptr<TestDemoParallelLeafReader> shared_from_this()
  {
    return std::static_pointer_cast<TestDemoParallelLeafReader>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
