#pragma once
#include "../../../../../../../test-framework/src/java/org/apache/lucene/util/LuceneTestCase.h"
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <algorithm>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/IndexWriter.h"

#include  "core/src/java/org/apache/lucene/store/Directory.h"
#include  "core/src/java/org/apache/lucene/index/IndexUpgrader.h"
#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/search/ScoreDoc.h"
#include  "core/src/java/org/apache/lucene/util/Version.h"
#include  "core/src/java/org/apache/lucene/index/PostingsEnum.h"
#include  "core/src/java/org/apache/lucene/index/LeafReader.h"
#include  "core/src/java/org/apache/lucene/index/BinaryDocValues.h"
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"

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

using ScoreDoc = org::apache::lucene::search::ScoreDoc;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using Version = org::apache::lucene::util::Version;

/*
  Verify we can read previous versions' indexes, do searches
  against them, and add documents to them.
*/
// See: https://issues.apache.org/jira/browse/SOLR-12028 Tests cannot remove
// files on Windows machines occasionally
class TestBackwardsCompatibility : public LuceneTestCase
{
  GET_CLASS_NAME(TestBackwardsCompatibility)

  // Backcompat index generation, described below, is mostly automated in:
  //
  //    dev-tools/scripts/addBackcompatIndexes.py
  //
  // For usage information, see:
  //
  //    http://wiki.apache.org/lucene-java/ReleaseTodo#Generate_Backcompat_Indexes
  //
  // -----
  //
  // To generate backcompat indexes with the current default codec, run the
  // following ant command:
  //  ant test -Dtestcase=TestBackwardsCompatibility
  //  -Dtests.bwcdir=/path/to/store/indexes
  //           -Dtests.codec=default -Dtests.useSecurityManager=false
  // Also add testmethod with one of the index creation methods below, for
  // example:
  //    -Dtestmethod=testCreateCFS
  //
  // Zip up the generated indexes:
  //
  //    cd /path/to/store/indexes/index.cfs   ; zip index.<VERSION>-cfs.zip *
  //    cd /path/to/store/indexes/index.nocfs ; zip index.<VERSION>-nocfs.zip *
  //
  // Then move those 2 zip files to your trunk checkout and add them
  // to the oldNames array.

public:
  virtual void testCreateCFS() ;

  virtual void testCreateNoCFS() ;

  // These are only needed for the special upgrade test to verify
  // that also single-segment indexes are correctly upgraded by IndexUpgrader.
  // You don't need them to be build for non-4.0 (the test is happy with just
  // one "old" segment format, version is unimportant:

  virtual void testCreateSingleSegmentCFS() ;

  virtual void testCreateSingleSegmentNoCFS() ;

private:
  std::shared_ptr<Path> getIndexDir();

public:
  virtual void testCreateMoreTermsIndex() ;

  // ant test -Dtestcase=TestBackwardsCompatibility
  // -Dtestmethod=testCreateSortedIndex -Dtests.codec=default
  // -Dtests.useSecurityManager=false -Dtests.bwcdir=/tmp/sorted
  virtual void testCreateSortedIndex() ;

private:
  void updateNumeric(std::shared_ptr<IndexWriter> writer,
                     const std::wstring &id, const std::wstring &f,
                     const std::wstring &cf,
                     int64_t value) ;

  void updateBinary(std::shared_ptr<IndexWriter> writer, const std::wstring &id,
                    const std::wstring &f, const std::wstring &cf,
                    int64_t value) ;

  // Creates an index with DocValues updates
public:
  virtual void testCreateIndexWithDocValuesUpdates() ;

  virtual void testCreateEmptyIndex() ;

  static std::deque<std::wstring> const oldNames;

  static std::deque<std::wstring> getOldNames();

  static std::deque<std::wstring> const oldSortedNames;

  static std::deque<std::wstring> getOldSortedNames();

  std::deque<std::wstring> const unsupportedNames = {
      L"1.9.0-cfs",
      L"1.9.0-nocfs",
      L"2.0.0-cfs",
      L"2.0.0-nocfs",
      L"2.1.0-cfs",
      L"2.1.0-nocfs",
      L"2.2.0-cfs",
      L"2.2.0-nocfs",
      L"2.3.0-cfs",
      L"2.3.0-nocfs",
      L"2.4.0-cfs",
      L"2.4.0-nocfs",
      L"2.4.1-cfs",
      L"2.4.1-nocfs",
      L"2.9.0-cfs",
      L"2.9.0-nocfs",
      L"2.9.1-cfs",
      L"2.9.1-nocfs",
      L"2.9.2-cfs",
      L"2.9.2-nocfs",
      L"2.9.3-cfs",
      L"2.9.3-nocfs",
      L"2.9.4-cfs",
      L"2.9.4-nocfs",
      L"3.0.0-cfs",
      L"3.0.0-nocfs",
      L"3.0.1-cfs",
      L"3.0.1-nocfs",
      L"3.0.2-cfs",
      L"3.0.2-nocfs",
      L"3.0.3-cfs",
      L"3.0.3-nocfs",
      L"3.1.0-cfs",
      L"3.1.0-nocfs",
      L"3.2.0-cfs",
      L"3.2.0-nocfs",
      L"3.3.0-cfs",
      L"3.3.0-nocfs",
      L"3.4.0-cfs",
      L"3.4.0-nocfs",
      L"3.5.0-cfs",
      L"3.5.0-nocfs",
      L"3.6.0-cfs",
      L"3.6.0-nocfs",
      L"3.6.1-cfs",
      L"3.6.1-nocfs",
      L"3.6.2-cfs",
      L"3.6.2-nocfs",
      L"4.0.0-cfs",
      L"4.0.0-nocfs",
      L"4.0.0.1-cfs",
      L"4.0.0.1-nocfs",
      L"4.0.0.2-cfs",
      L"4.0.0.2-nocfs",
      L"4.1.0-cfs",
      L"4.1.0-nocfs",
      L"4.2.0-cfs",
      L"4.2.0-nocfs",
      L"4.2.1-cfs",
      L"4.2.1-nocfs",
      L"4.3.0-cfs",
      L"4.3.0-nocfs",
      L"4.3.1-cfs",
      L"4.3.1-nocfs",
      L"4.4.0-cfs",
      L"4.4.0-nocfs",
      L"4.5.0-cfs",
      L"4.5.0-nocfs",
      L"4.5.1-cfs",
      L"4.5.1-nocfs",
      L"4.6.0-cfs",
      L"4.6.0-nocfs",
      L"4.6.1-cfs",
      L"4.6.1-nocfs",
      L"4.7.0-cfs",
      L"4.7.0-nocfs",
      L"4.7.1-cfs",
      L"4.7.1-nocfs",
      L"4.7.2-cfs",
      L"4.7.2-nocfs",
      L"4.8.0-cfs",
      L"4.8.0-nocfs",
      L"4.8.1-cfs",
      L"4.8.1-nocfs",
      L"4.9.0-cfs",
      L"4.9.0-nocfs",
      L"4.9.1-cfs",
      L"4.9.1-nocfs",
      L"4.10.0-cfs",
      L"4.10.0-nocfs",
      L"4.10.1-cfs",
      L"4.10.1-nocfs",
      L"4.10.2-cfs",
      L"4.10.2-nocfs",
      L"4.10.3-cfs",
      L"4.10.3-nocfs",
      L"4.10.4-cfs",
      L"4.10.4-nocfs",
      L"5x-with-4x-segments-cfs",
      L"5x-with-4x-segments-nocfs",
      L"5.0.0.singlesegment-cfs",
      L"5.0.0.singlesegment-nocfs",
      L"5.0.0-cfs",
      L"5.0.0-nocfs",
      L"5.1.0-cfs",
      L"5.1.0-nocfs",
      L"5.2.0-cfs",
      L"5.2.0-nocfs",
      L"5.2.1-cfs",
      L"5.2.1-nocfs",
      L"5.3.0-cfs",
      L"5.3.0-nocfs",
      L"5.3.1-cfs",
      L"5.3.1-nocfs",
      L"5.3.2-cfs",
      L"5.3.2-nocfs",
      L"5.4.0-cfs",
      L"5.4.0-nocfs",
      L"5.4.1-cfs",
      L"5.4.1-nocfs",
      L"5.5.0-cfs",
      L"5.5.0-nocfs",
      L"5.5.1-cfs",
      L"5.5.1-nocfs",
      L"5.5.2-cfs",
      L"5.5.2-nocfs",
      L"5.5.3-cfs",
      L"5.5.3-nocfs",
      L"5.5.4-cfs",
      L"5.5.4-nocfs",
      L"5.5.5-cfs",
      L"5.5.5-nocfs"};

  // TODO: on 6.0.0 release, gen the single segment indices and add here:
  static std::deque<std::wstring> const oldSingleSegmentNames;

  static std::deque<std::wstring> getOldSingleSegmentNames();

  static std::unordered_map<std::wstring, std::shared_ptr<Directory>>
      oldIndexDirs;

  /**
   * Randomizes the use of some of hte constructor variations
   */
private:
  static std::shared_ptr<IndexUpgrader>
  newIndexUpgrader(std::shared_ptr<Directory> dir);

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @BeforeClass public static void beforeClass() throws
  // Exception
  static void beforeClass() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @AfterClass public static void afterClass() throws Exception
  static void afterClass() ;

  virtual void testAllVersionHaveCfsAndNocfs();

  virtual void testAllVersionsTested() ;

  /** This test checks that *only* IndexFormatTooOldExceptions are thrown when
   * you open and operate on too old indexes! */
  virtual void testUnsupportedOldIndexes() ;

  virtual void testFullyMergeOldIndex() ;

  virtual void testAddOldIndexes() ;

  virtual void testAddOldIndexesReader() ;

  virtual void testSearchOldIndex() ;

  virtual void testIndexOldIndexNoAdds() ;

  virtual void testIndexOldIndex() ;

private:
  void doTestHits(std::deque<std::shared_ptr<ScoreDoc>> &hits,
                  int expectedCount,
                  std::shared_ptr<IndexReader> reader) ;

public:
  virtual void searchIndex(std::shared_ptr<Directory> dir,
                           const std::wstring &oldName) ;

  virtual void
  changeIndexWithAdds(std::shared_ptr<Random> random,
                      std::shared_ptr<Directory> dir,
                      std::shared_ptr<Version> nameVersion) ;

  virtual void
  changeIndexNoAdds(std::shared_ptr<Random> random,
                    std::shared_ptr<Directory> dir) ;

  virtual void createIndex(const std::wstring &dirName, bool doCFS,
                           bool fullyMerged) ;

private:
  void addDoc(std::shared_ptr<IndexWriter> writer, int id) ;

  void addNoProxDoc(std::shared_ptr<IndexWriter> writer) ;

  int countDocs(std::shared_ptr<PostingsEnum> docs) ;

  // flex: test basics of TermsEnum api on non-flex index
public:
  virtual void testNextIntoWrongField() ;

  /**
   * Test that we didn't forget to bump the current
   * Constants.LUCENE_MAIN_VERSION. This is important so that we can determine
   * which version of lucene wrote the segment.
   */
  virtual void testOldVersions() ;

  virtual void testIndexCreatedVersion() ;

  virtual void
  verifyUsesDefaultCodec(std::shared_ptr<Directory> dir,
                         const std::wstring &name) ;

  virtual void testAllIndexesUseDefaultCodec() ;

private:
  int checkAllSegmentsUpgraded(std::shared_ptr<Directory> dir,
                               int indexCreatedVersion) ;

  int getNumberOfSegments(std::shared_ptr<Directory> dir) ;

public:
  virtual void testUpgradeOldIndex() ;

  virtual void testCommandLineArgs() ;

  virtual void
  testUpgradeOldSingleSegmentIndexWithAdditions() ;

  static const std::wstring emptyIndex;

  virtual void testUpgradeEmptyOldIndex() ;

  static const std::wstring moreTermsIndex;

  virtual void testMoreTerms() ;

  static const std::wstring dvUpdatesIndex;

private:
  void assertNumericDocValues(std::shared_ptr<LeafReader> r,
                              const std::wstring &f,
                              const std::wstring &cf) ;

  void assertBinaryDocValues(std::shared_ptr<LeafReader> r,
                             const std::wstring &f,
                             const std::wstring &cf) ;

  void verifyDocValues(std::shared_ptr<Directory> dir) ;

public:
  virtual void testDocValuesUpdates() ;

  // LUCENE-5907
  virtual void testUpgradeWithNRTReader() ;

  // LUCENE-5907
  virtual void testUpgradeThenMultipleCommits() ;

  virtual void testSortedIndex() ;

  static int64_t
  getValue(std::shared_ptr<BinaryDocValues> bdv) ;

  // encodes a long into a BytesRef as VLong so that we get varying number of
  // bytes when we update
  static std::shared_ptr<BytesRef> toBytes(int64_t value);

protected:
  std::shared_ptr<TestBackwardsCompatibility> shared_from_this()
  {
    return std::static_pointer_cast<TestBackwardsCompatibility>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
