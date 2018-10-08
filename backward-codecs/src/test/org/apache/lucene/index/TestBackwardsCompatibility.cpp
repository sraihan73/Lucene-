using namespace std;

#include "TestBackwardsCompatibility.h"
#include "../../../../../../../core/src/java/org/apache/lucene/codecs/Codec.h"
#include "../../../../../../../core/src/java/org/apache/lucene/document/BinaryDocValuesField.h"
#include "../../../../../../../core/src/java/org/apache/lucene/document/BinaryPoint.h"
#include "../../../../../../../core/src/java/org/apache/lucene/document/Document.h"
#include "../../../../../../../core/src/java/org/apache/lucene/document/DoubleDocValuesField.h"
#include "../../../../../../../core/src/java/org/apache/lucene/document/DoublePoint.h"
#include "../../../../../../../core/src/java/org/apache/lucene/document/Field.h"
#include "../../../../../../../core/src/java/org/apache/lucene/document/FieldType.h"
#include "../../../../../../../core/src/java/org/apache/lucene/document/FloatDocValuesField.h"
#include "../../../../../../../core/src/java/org/apache/lucene/document/FloatPoint.h"
#include "../../../../../../../core/src/java/org/apache/lucene/document/IntPoint.h"
#include "../../../../../../../core/src/java/org/apache/lucene/document/LongPoint.h"
#include "../../../../../../../core/src/java/org/apache/lucene/document/NumericDocValuesField.h"
#include "../../../../../../../core/src/java/org/apache/lucene/document/SortedDocValuesField.h"
#include "../../../../../../../core/src/java/org/apache/lucene/document/SortedNumericDocValuesField.h"
#include "../../../../../../../core/src/java/org/apache/lucene/document/SortedSetDocValuesField.h"
#include "../../../../../../../core/src/java/org/apache/lucene/document/StringField.h"
#include "../../../../../../../core/src/java/org/apache/lucene/document/TextField.h"
#include "../../../../../../../core/src/java/org/apache/lucene/index/BinaryDocValues.h"
#include "../../../../../../../core/src/java/org/apache/lucene/index/CheckIndex.h"
#include "../../../../../../../core/src/java/org/apache/lucene/index/DirectoryReader.h"
#include "../../../../../../../core/src/java/org/apache/lucene/index/Fields.h"
#include "../../../../../../../core/src/java/org/apache/lucene/index/IndexFormatTooOldException.h"
#include "../../../../../../../core/src/java/org/apache/lucene/index/IndexOptions.h"
#include "../../../../../../../core/src/java/org/apache/lucene/index/IndexReader.h"
#include "../../../../../../../core/src/java/org/apache/lucene/index/IndexUpgrader.h"
#include "../../../../../../../core/src/java/org/apache/lucene/index/IndexWriter.h"
#include "../../../../../../../core/src/java/org/apache/lucene/index/IndexWriterConfig.h"
#include "../../../../../../../core/src/java/org/apache/lucene/index/IndexableField.h"
#include "../../../../../../../core/src/java/org/apache/lucene/index/LeafReader.h"
#include "../../../../../../../core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include "../../../../../../../core/src/java/org/apache/lucene/index/LogByteSizeMergePolicy.h"
#include "../../../../../../../core/src/java/org/apache/lucene/index/MergePolicy.h"
#include "../../../../../../../core/src/java/org/apache/lucene/index/MultiDocValues.h"
#include "../../../../../../../core/src/java/org/apache/lucene/index/MultiFields.h"
#include "../../../../../../../core/src/java/org/apache/lucene/index/NoMergePolicy.h"
#include "../../../../../../../core/src/java/org/apache/lucene/index/NumericDocValues.h"
#include "../../../../../../../core/src/java/org/apache/lucene/index/PostingsEnum.h"
#include "../../../../../../../core/src/java/org/apache/lucene/index/SegmentInfos.h"
#include "../../../../../../../core/src/java/org/apache/lucene/index/SegmentReader.h"
#include "../../../../../../../core/src/java/org/apache/lucene/index/SortedDocValues.h"
#include "../../../../../../../core/src/java/org/apache/lucene/index/SortedNumericDocValues.h"
#include "../../../../../../../core/src/java/org/apache/lucene/index/SortedSetDocValues.h"
#include "../../../../../../../core/src/java/org/apache/lucene/index/Term.h"
#include "../../../../../../../core/src/java/org/apache/lucene/index/Terms.h"
#include "../../../../../../../core/src/java/org/apache/lucene/index/TermsEnum.h"
#include "../../../../../../../core/src/java/org/apache/lucene/search/DocIdSetIterator.h"
#include "../../../../../../../core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include "../../../../../../../core/src/java/org/apache/lucene/search/ScoreDoc.h"
#include "../../../../../../../core/src/java/org/apache/lucene/search/Sort.h"
#include "../../../../../../../core/src/java/org/apache/lucene/search/SortField.h"
#include "../../../../../../../core/src/java/org/apache/lucene/search/TermQuery.h"
#include "../../../../../../../core/src/java/org/apache/lucene/store/Directory.h"
#include "../../../../../../../core/src/java/org/apache/lucene/store/NIOFSDirectory.h"
#include "../../../../../../../core/src/java/org/apache/lucene/store/RAMDirectory.h"
#include "../../../../../../../core/src/java/org/apache/lucene/store/SimpleFSDirectory.h"
#include "../../../../../../../core/src/java/org/apache/lucene/util/Bits.h"
#include "../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "../../../../../../../core/src/java/org/apache/lucene/util/IOUtils.h"
#include "../../../../../../../core/src/java/org/apache/lucene/util/InfoStream.h"
#include "../../../../../../../core/src/java/org/apache/lucene/util/Version.h"
#include "../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockAnalyzer.h"
#include "../../../../../../../test-framework/src/java/org/apache/lucene/index/RandomIndexWriter.h"
#include "../../../../../../../test-framework/src/java/org/apache/lucene/store/BaseDirectoryWrapper.h"
#include "../../../../../../../test-framework/src/java/org/apache/lucene/util/LineFileDocs.h"
#include "../../../../../../../test-framework/src/java/org/apache/lucene/util/TestUtil.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Codec = org::apache::lucene::codecs::Codec;
using BinaryDocValuesField =
    org::apache::lucene::document::BinaryDocValuesField;
using BinaryPoint = org::apache::lucene::document::BinaryPoint;
using Document = org::apache::lucene::document::Document;
using DoubleDocValuesField =
    org::apache::lucene::document::DoubleDocValuesField;
using DoublePoint = org::apache::lucene::document::DoublePoint;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using FloatDocValuesField = org::apache::lucene::document::FloatDocValuesField;
using FloatPoint = org::apache::lucene::document::FloatPoint;
using IntPoint = org::apache::lucene::document::IntPoint;
using LongPoint = org::apache::lucene::document::LongPoint;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using SortedDocValuesField =
    org::apache::lucene::document::SortedDocValuesField;
using SortedNumericDocValuesField =
    org::apache::lucene::document::SortedNumericDocValuesField;
using SortedSetDocValuesField =
    org::apache::lucene::document::SortedSetDocValuesField;
using StringField = org::apache::lucene::document::StringField;
using TextField = org::apache::lucene::document::TextField;
using OpenMode = org::apache::lucene::index::IndexWriterConfig::OpenMode;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using ScoreDoc = org::apache::lucene::search::ScoreDoc;
using Sort = org::apache::lucene::search::Sort;
using SortField = org::apache::lucene::search::SortField;
using TermQuery = org::apache::lucene::search::TermQuery;
using BaseDirectoryWrapper = org::apache::lucene::store::BaseDirectoryWrapper;
using Directory = org::apache::lucene::store::Directory;
using FSDirectory = org::apache::lucene::store::FSDirectory;
using NIOFSDirectory = org::apache::lucene::store::NIOFSDirectory;
using RAMDirectory = org::apache::lucene::store::RAMDirectory;
using SimpleFSDirectory = org::apache::lucene::store::SimpleFSDirectory;
using Bits = org::apache::lucene::util::Bits;
using BytesRef = org::apache::lucene::util::BytesRef;
using IOUtils = org::apache::lucene::util::IOUtils;
using InfoStream = org::apache::lucene::util::InfoStream;
using LineFileDocs = org::apache::lucene::util::LineFileDocs;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;
using Version = org::apache::lucene::util::Version;
using org::junit::AfterClass;
using org::junit::BeforeClass;

void TestBackwardsCompatibility::testCreateCFS() 
{
  createIndex(L"index.cfs", true, false);
}

void TestBackwardsCompatibility::testCreateNoCFS() 
{
  createIndex(L"index.nocfs", false, false);
}

void TestBackwardsCompatibility::testCreateSingleSegmentCFS() 
{
  createIndex(L"index.singlesegment-cfs", true, true);
}

void TestBackwardsCompatibility::testCreateSingleSegmentNoCFS() throw(
    IOException)
{
  createIndex(L"index.singlesegment-nocfs", false, true);
}

shared_ptr<Path> TestBackwardsCompatibility::getIndexDir()
{
  wstring path = System::getProperty(L"tests.bwcdir");
  assumeTrue(L"backcompat creation tests must be run with "
             L"-Dtests.bwcdir=/path/to/write/indexes",
             path != L"");
  return Paths->get(path);
}

void TestBackwardsCompatibility::testCreateMoreTermsIndex() 
{

  shared_ptr<Path> indexDir = getIndexDir()->resolve(L"moreterms");
  Files::deleteIfExists(indexDir);
  shared_ptr<Directory> dir = newFSDirectory(indexDir);

  shared_ptr<LogByteSizeMergePolicy> mp = make_shared<LogByteSizeMergePolicy>();
  mp->setNoCFSRatio(1.0);
  mp->setMaxCFSSegmentSizeMB(numeric_limits<double>::infinity());
  shared_ptr<MockAnalyzer> analyzer = make_shared<MockAnalyzer>(random());
  analyzer->setMaxTokenLength(
      TestUtil::nextInt(random(), 1, IndexWriter::MAX_TERM_LENGTH));

  shared_ptr<IndexWriterConfig> conf =
      (make_shared<IndexWriterConfig>(analyzer))
          ->setMergePolicy(mp)
          ->setUseCompoundFile(false);
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);
  shared_ptr<LineFileDocs> docs =
      make_shared<LineFileDocs>(make_shared<Random>(0));
  for (int i = 0; i < 50; i++) {
    writer->addDocument(docs->nextDoc());
  }
  delete docs;
  delete writer;
  delete dir;

  // Gives you time to copy the index out!: (there is also
  // a test option to not remove temp dir...):
  delay(100000);
}

void TestBackwardsCompatibility::testCreateSortedIndex() 
{

  shared_ptr<Path> indexDir = getIndexDir()->resolve(L"sorted");
  Files::deleteIfExists(indexDir);
  shared_ptr<Directory> dir = newFSDirectory(indexDir);

  shared_ptr<LogByteSizeMergePolicy> mp = make_shared<LogByteSizeMergePolicy>();
  mp->setNoCFSRatio(1.0);
  mp->setMaxCFSSegmentSizeMB(numeric_limits<double>::infinity());
  shared_ptr<MockAnalyzer> analyzer = make_shared<MockAnalyzer>(random());
  analyzer->setMaxTokenLength(
      TestUtil::nextInt(random(), 1, IndexWriter::MAX_TERM_LENGTH));

  // TODO: remove randomness
  shared_ptr<IndexWriterConfig> conf = make_shared<IndexWriterConfig>(analyzer);
  conf->setMergePolicy(mp);
  conf->setUseCompoundFile(false);
  conf->setIndexSort(make_shared<Sort>(
      make_shared<SortField>(L"dateDV", SortField::Type::LONG, true)));
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);
  shared_ptr<LineFileDocs> docs = make_shared<LineFileDocs>(random());
  shared_ptr<SimpleDateFormat> parser =
      make_shared<SimpleDateFormat>(L"yyyy-MM-dd", Locale::ROOT);
  parser->setTimeZone(TimeZone::getTimeZone(L"UTC"));
  shared_ptr<ParsePosition> position = make_shared<ParsePosition>(0);
  shared_ptr<Field> dateDVField = nullptr;
  for (int i = 0; i < 50; i++) {
    shared_ptr<Document> doc = docs->nextDoc();
    wstring dateString = doc[L"date"];

    position->setIndex(0);
    Date date = parser->parse(dateString, position);
    if (position->getErrorIndex() != -1) {
      throw make_shared<AssertionError>(L"failed to parse \"" + dateString +
                                        L"\" as date");
    }
    if (position->getIndex() != dateString.length()) {
      throw make_shared<AssertionError>(L"failed to parse \"" + dateString +
                                        L"\" as date");
    }
    if (dateDVField == nullptr) {
      dateDVField = make_shared<NumericDocValuesField>(L"dateDV", 0LL);
      doc->push_back(dateDVField);
    }
    dateDVField->setLongValue(date.getTime());
    if (i == 250) {
      writer->commit();
    }
    writer->addDocument(doc);
  }
  writer->forceMerge(1);
  delete writer;
  delete dir;
}

void TestBackwardsCompatibility::updateNumeric(
    shared_ptr<IndexWriter> writer, const wstring &id, const wstring &f,
    const wstring &cf, int64_t value) 
{
  writer->updateNumericDocValue(make_shared<Term>(L"id", id), f, value);
  writer->updateNumericDocValue(make_shared<Term>(L"id", id), cf, value * 2);
}

void TestBackwardsCompatibility::updateBinary(
    shared_ptr<IndexWriter> writer, const wstring &id, const wstring &f,
    const wstring &cf, int64_t value) 
{
  writer->updateBinaryDocValue(make_shared<Term>(L"id", id), f, toBytes(value));
  writer->updateBinaryDocValue(make_shared<Term>(L"id", id), cf,
                               toBytes(value * 2));
}

void TestBackwardsCompatibility::testCreateIndexWithDocValuesUpdates() throw(
    runtime_error)
{
  shared_ptr<Path> indexDir = getIndexDir()->resolve(L"dvupdates");
  Files::deleteIfExists(indexDir);
  shared_ptr<Directory> dir = newFSDirectory(indexDir);

  shared_ptr<IndexWriterConfig> conf =
      (make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random())))
          ->setUseCompoundFile(false)
          ->setMergePolicy(NoMergePolicy::INSTANCE);
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);
  // create an index w/ few doc-values fields, some with updates and some
  // without
  for (int i = 0; i < 30; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(
        make_shared<StringField>(L"id", L"" + to_wstring(i), Field::Store::NO));
    doc->push_back(make_shared<NumericDocValuesField>(L"ndv1", i));
    doc->push_back(make_shared<NumericDocValuesField>(L"ndv1_c", i * 2));
    doc->push_back(make_shared<NumericDocValuesField>(L"ndv2", i * 3));
    doc->push_back(make_shared<NumericDocValuesField>(L"ndv2_c", i * 6));
    doc->push_back(make_shared<BinaryDocValuesField>(L"bdv1", toBytes(i)));
    doc->push_back(
        make_shared<BinaryDocValuesField>(L"bdv1_c", toBytes(i * 2)));
    doc->push_back(make_shared<BinaryDocValuesField>(L"bdv2", toBytes(i * 3)));
    doc->push_back(
        make_shared<BinaryDocValuesField>(L"bdv2_c", toBytes(i * 6)));
    writer->addDocument(doc);
    if ((i + 1) % 10 == 0) {
      writer->commit(); // flush every 10 docs
    }
  }

  // first segment: no updates

  // second segment: update two fields, same gen
  updateNumeric(writer, L"10", L"ndv1", L"ndv1_c", 100LL);
  updateBinary(writer, L"11", L"bdv1", L"bdv1_c", 100LL);
  writer->commit();

  // third segment: update few fields, different gens, few docs
  updateNumeric(writer, L"20", L"ndv1", L"ndv1_c", 100LL);
  updateBinary(writer, L"21", L"bdv1", L"bdv1_c", 100LL);
  writer->commit();
  updateNumeric(writer, L"22", L"ndv1", L"ndv1_c",
                200LL); // update the field again
  writer->commit();

  delete writer;
  delete dir;
}

void TestBackwardsCompatibility::testCreateEmptyIndex() 
{
  shared_ptr<Path> indexDir = getIndexDir()->resolve(L"emptyIndex");
  Files::deleteIfExists(indexDir);
  shared_ptr<IndexWriterConfig> conf =
      (make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random())))
          ->setUseCompoundFile(false)
          ->setMergePolicy(NoMergePolicy::INSTANCE);
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.Directory dir =
  // newFSDirectory(indexDir); IndexWriter writer = new IndexWriter(dir, conf))
  {
    org::apache::lucene::store::Directory dir = newFSDirectory(indexDir);
    IndexWriter writer = IndexWriter(dir, conf);
    writer->flush();
  }
}

std::deque<wstring> const TestBackwardsCompatibility::oldNames = {
    L"6.0.0-cfs", L"6.0.0-nocfs", L"6.0.1-cfs", L"6.0.1-nocfs",
    L"6.1.0-cfs", L"6.1.0-nocfs", L"6.2.0-cfs", L"6.2.0-nocfs",
    L"6.2.1-cfs", L"6.2.1-nocfs", L"6.3.0-cfs", L"6.3.0-nocfs",
    L"6.4.0-cfs", L"6.4.0-nocfs", L"6.4.1-cfs", L"6.4.1-nocfs",
    L"6.4.2-cfs", L"6.4.2-nocfs", L"6.5.0-cfs", L"6.5.0-nocfs",
    L"6.5.1-cfs", L"6.5.1-nocfs", L"6.6.0-cfs", L"6.6.0-nocfs",
    L"6.6.1-cfs", L"6.6.1-nocfs", L"6.6.2-cfs", L"6.6.2-nocfs",
    L"6.6.3-cfs", L"6.6.3-nocfs", L"6.6.4-cfs", L"6.6.4-nocfs",
    L"7.0.0-cfs", L"7.0.0-nocfs", L"7.0.1-cfs", L"7.0.1-nocfs",
    L"7.1.0-cfs", L"7.1.0-nocfs", L"7.2.0-cfs", L"7.2.0-nocfs",
    L"7.2.1-cfs", L"7.2.1-nocfs", L"7.3.0-cfs", L"7.3.0-nocfs",
    L"7.3.1-cfs", L"7.3.1-nocfs"};

std::deque<wstring> TestBackwardsCompatibility::getOldNames()
{
  return oldNames;
}

std::deque<wstring> const TestBackwardsCompatibility::oldSortedNames = {
    L"sorted.6.2.0", L"sorted.6.2.1", L"sorted.6.3.0", L"sorted.6.4.0",
    L"sorted.6.4.1", L"sorted.6.4.2", L"sorted.6.5.0", L"sorted.6.5.1",
    L"sorted.6.6.0", L"sorted.6.6.1", L"sorted.6.6.2", L"sorted.6.6.3",
    L"sorted.6.6.4", L"sorted.7.0.0", L"sorted.7.0.1", L"sorted.7.1.0",
    L"sorted.7.2.0", L"sorted.7.2.1", L"sorted.7.3.0", L"sorted.7.3.1"};

std::deque<wstring> TestBackwardsCompatibility::getOldSortedNames()
{
  return oldSortedNames;
}

std::deque<wstring> const TestBackwardsCompatibility::oldSingleSegmentNames =
    {};

std::deque<wstring> TestBackwardsCompatibility::getOldSingleSegmentNames()
{
  return oldSingleSegmentNames;
}

unordered_map<wstring, std::shared_ptr<org::apache::lucene::store::Directory>>
    TestBackwardsCompatibility::oldIndexDirs;

shared_ptr<IndexUpgrader>
TestBackwardsCompatibility::newIndexUpgrader(shared_ptr<Directory> dir)
{
  constexpr bool streamType = random()->nextBoolean();
  constexpr int choice = TestUtil::nextInt(random(), 0, 2);
  switch (choice) {
  case 0:
    return make_shared<IndexUpgrader>(dir);
  case 1:
    return make_shared<IndexUpgrader>(
        dir, streamType ? nullptr : InfoStream::NO_OUTPUT, false);
  case 2:
    return make_shared<IndexUpgrader>(dir, newIndexWriterConfig(nullptr),
                                      false);
  default:
    fail(L"case statement didn't get updated when random bounds changed");
  }
  return nullptr; // never get here
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void beforeClass() throws Exception
void TestBackwardsCompatibility::beforeClass() 
{
  deque<wstring> names =
      deque<wstring>(oldNames.size() + oldSingleSegmentNames.size());
  names.addAll(Arrays::asList(oldNames));
  names.addAll(Arrays::asList(oldSingleSegmentNames));
  oldIndexDirs = unordered_map<>();
  for (auto name : names) {
    shared_ptr<Path> dir = createTempDir(name);
    shared_ptr<InputStream> resource =
        TestBackwardsCompatibility::typeid->getResourceAsStream(L"index." +
                                                                name + L".zip");
    assertNotNull(L"Index name " + name + L" not found", resource);
    TestUtil::unzip(resource, dir);
    oldIndexDirs.emplace(name, newFSDirectory(dir));
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @AfterClass public static void afterClass() throws Exception
void TestBackwardsCompatibility::afterClass() 
{
  for (auto d : oldIndexDirs) {
    d->second->close();
  }
  oldIndexDirs.clear();
}

void TestBackwardsCompatibility::testAllVersionHaveCfsAndNocfs()
{
  // ensure all tested versions with cfs also have nocfs
  std::deque<wstring> files(oldNames.size());
  System::arraycopy(oldNames, 0, files, 0, oldNames.size());
  Arrays::sort(files);
  wstring prevFile = L"";
  for (auto file : files) {
    if (StringHelper::endsWith(prevFile, L"-cfs")) {
      wstring prefix = StringHelper::replace(prevFile, L"-cfs", L"");
      assertEquals(L"Missing -nocfs for backcompat index " + prefix,
                   prefix + L"-nocfs", file);
    }
  }
}

void TestBackwardsCompatibility::testAllVersionsTested() 
{
  shared_ptr<Pattern> constantPattern =
      Pattern::compile(L"LUCENE_(\\d+)_(\\d+)_(\\d+)(_ALPHA|_BETA)?");
  // find the unique versions according to Version.java
  deque<wstring> expectedVersions = deque<wstring>();
  for (shared_ptr<java::lang::reflect::Field> field :
       Version::typeid->getDeclaredFields()) {
    if (Modifier::isStatic(field->getModifiers()) &&
        field->getType() == Version::typeid) {
      shared_ptr<Version> v =
          std::static_pointer_cast<Version>(field->get(Version::typeid));
      if (v->equals(Version::LATEST)) {
        continue;
      }

      shared_ptr<Matcher> constant = constantPattern->matcher(field->getName());
      if (constant->matches() == false) {
        continue;
      }

      // C++ TODO: There is no native C++ equivalent to 'toString':
      expectedVersions.push_back(v->toString() + L"-cfs");
    }
  }

  sort(expectedVersions.begin(), expectedVersions.end());

  // find what versions we are testing
  deque<wstring> testedVersions = deque<wstring>();
  for (auto testedVersion : oldNames) {
    if (testedVersion.endsWith(L"-cfs") == false) {
      continue;
    }
    testedVersions.push_back(testedVersion);
  }
  sort(testedVersions.begin(), testedVersions.end());

  int i = 0;
  int j = 0;
  deque<wstring> missingFiles = deque<wstring>();
  deque<wstring> extraFiles = deque<wstring>();
  while (i < expectedVersions.size() && j < testedVersions.size()) {
    wstring expectedVersion = expectedVersions[i];
    wstring testedVersion = testedVersions[j];
    int compare = expectedVersion.compare(testedVersion);
    if (compare == 0) { // equal, we can move on
      ++i;
      ++j;
    } else if (compare < 0) { // didn't find test for version constant
      missingFiles.push_back(expectedVersion);
      ++i;
    } else { // extra test file
      extraFiles.push_back(testedVersion);
      ++j;
    }
  }
  while (i < expectedVersions.size()) {
    missingFiles.push_back(expectedVersions[i]);
    ++i;
  }
  while (j < testedVersions.size()) {
    missingFiles.push_back(testedVersions[j]);
    ++j;
  }

  // we could be missing up to 1 file, which may be due to a release that is in
  // progress
  if (missingFiles.size() <= 1 && extraFiles.empty()) {
    // success
    return;
  }

  shared_ptr<StringBuilder> msg = make_shared<StringBuilder>();
  if (missingFiles.size() > 1) {
    msg->append(L"Missing backcompat test files:\n");
    for (auto missingFile : missingFiles) {
      msg->append(L"  " + missingFile + L"\n");
    }
  }
  if (extraFiles.empty() == false) {
    msg->append(L"Extra backcompat test files:\n");
    for (auto extraFile : extraFiles) {
      msg->append(L"  " + extraFile + L"\n");
    }
  }
  fail(msg->toString());
}

void TestBackwardsCompatibility::testUnsupportedOldIndexes() throw(
    runtime_error)
{
  for (int i = 0; i < unsupportedNames.size(); i++) {
    if (VERBOSE) {
      wcout << L"TEST: index " << unsupportedNames[i] << endl;
    }
    shared_ptr<Path> oldIndexDir = createTempDir(unsupportedNames[i]);
    TestUtil::unzip(
        getDataInputStream(L"unsupported." + unsupportedNames[i] + L".zip"),
        oldIndexDir);
    shared_ptr<BaseDirectoryWrapper> dir = newFSDirectory(oldIndexDir);
    // don't checkindex, these are intentionally not supported
    dir->setCheckIndexOnClose(false);

    shared_ptr<IndexReader> reader = nullptr;
    shared_ptr<IndexWriter> writer = nullptr;
    try {
      reader = DirectoryReader::open(dir);
      fail(L"DirectoryReader.open should not pass for " + unsupportedNames[i]);
    } catch (const IndexFormatTooOldException &e) {
      if (e->getReason() != L"") {
        assertNull(e->getVersion());
        assertNull(e->getMinVersion());
        assertNull(e->getMaxVersion());
        TestUtil::assertEquals(
            e->getMessage(), (make_shared<IndexFormatTooOldException>(
                                  e->getResourceDescription(), e->getReason()))
                                 ->getMessage());
      } else {
        assertNotNull(e->getVersion());
        assertNotNull(e->getMinVersion());
        assertNotNull(e->getMaxVersion());
        assertTrue(e->getMessage(), e->getMaxVersion() >= e->getMinVersion());
        assertTrue(e->getMessage(), e->getMaxVersion() < e->getVersion() ||
                                        e->getVersion() < e->getMinVersion());
        TestUtil::assertEquals(
            e->getMessage(), (make_shared<IndexFormatTooOldException>(
                                  e->getResourceDescription(), e->getVersion(),
                                  e->getMinVersion(), e->getMaxVersion()))
                                 ->getMessage());
      }
      // pass
      if (VERBOSE) {
        wcout << L"TEST: got expected exc:" << endl;
        e->printStackTrace(System::out);
      }
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      if (reader != nullptr) {
        delete reader;
      }
      reader.reset();
    }

    try {
      writer = make_shared<IndexWriter>(
          dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
                   ->setCommitOnClose(false));
      fail(L"IndexWriter creation should not pass for " + unsupportedNames[i]);
    } catch (const IndexFormatTooOldException &e) {
      if (e->getReason() != L"") {
        assertNull(e->getVersion());
        assertNull(e->getMinVersion());
        assertNull(e->getMaxVersion());
        TestUtil::assertEquals(
            e->getMessage(), (make_shared<IndexFormatTooOldException>(
                                  e->getResourceDescription(), e->getReason()))
                                 ->getMessage());
      } else {
        assertNotNull(e->getVersion());
        assertNotNull(e->getMinVersion());
        assertNotNull(e->getMaxVersion());
        assertTrue(e->getMessage(), e->getMaxVersion() >= e->getMinVersion());
        assertTrue(e->getMessage(), e->getMaxVersion() < e->getVersion() ||
                                        e->getVersion() < e->getMinVersion());
        TestUtil::assertEquals(
            e->getMessage(), (make_shared<IndexFormatTooOldException>(
                                  e->getResourceDescription(), e->getVersion(),
                                  e->getMinVersion(), e->getMaxVersion()))
                                 ->getMessage());
      }
      // pass
      if (VERBOSE) {
        wcout << L"TEST: got expected exc:" << endl;
        e->printStackTrace(System::out);
      }
      // Make sure exc message includes a path=
      assertTrue(L"got exc message: " + e->getMessage(),
                 e->getMessage()->find(L"path=\"") != -1);
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      // we should fail to open IW, and so it should be null when we get here.
      // However, if the test fails (i.e., IW did not fail on open), we need
      // to close IW. However, if merges are run, IW may throw
      // IndexFormatTooOldException, and we don't want to mask the fail()
      // above, so close without waiting for merges.
      if (writer != nullptr) {
        try {
          writer->commit();
        }
        // C++ TODO: There is no native C++ equivalent to the exception
        // 'finally' clause:
        finally {
          delete writer;
        }
      }
      writer.reset();
    }

    shared_ptr<ByteArrayOutputStream> bos =
        make_shared<ByteArrayOutputStream>(1024);
    shared_ptr<CheckIndex> checker = make_shared<CheckIndex>(dir);
    checker->setInfoStream(
        make_shared<PrintStream>(bos, false, IOUtils::UTF_8));
    shared_ptr<CheckIndex::Status> indexStatus = checker->checkIndex();
    assertFalse(indexStatus->clean);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    assertTrue(bos->toString(IOUtils::UTF_8)
                   ->contains(IndexFormatTooOldException::typeid->getName()));
    delete checker;

    delete dir;
  }
}

void TestBackwardsCompatibility::testFullyMergeOldIndex() 
{
  for (auto name : oldNames) {
    if (VERBOSE) {
      wcout << L"\nTEST: index=" << name << endl;
    }
    shared_ptr<Directory> dir = newDirectory(oldIndexDirs[name]);

    shared_ptr<SegmentInfos> *const oldSegInfos =
        SegmentInfos::readLatestCommit(dir);

    shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
        dir,
        make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random())));
    w->forceMerge(1);
    delete w;

    shared_ptr<SegmentInfos> *const segInfos =
        SegmentInfos::readLatestCommit(dir);
    TestUtil::assertEquals(oldSegInfos->getIndexCreatedVersionMajor(),
                           segInfos->getIndexCreatedVersionMajor());
    TestUtil::assertEquals(Version::LATEST,
                           segInfos->asList()[0]->info.getVersion());
    TestUtil::assertEquals(oldSegInfos->asList()[0]->info.getMinVersion(),
                           segInfos->asList()[0]->info.getMinVersion());

    delete dir;
  }
}

void TestBackwardsCompatibility::testAddOldIndexes() 
{
  for (auto name : oldNames) {
    if (VERBOSE) {
      wcout << L"\nTEST: old index " << name << endl;
    }
    shared_ptr<Directory> oldDir = oldIndexDirs[name];
    shared_ptr<SegmentInfos> infos = SegmentInfos::readLatestCommit(oldDir);

    shared_ptr<Directory> targetDir = newDirectory();
    if (infos->getCommitLuceneVersion()->major != Version::LATEST->major) {
      // both indexes are not compatible
      shared_ptr<Directory> targetDir2 = newDirectory();
      shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
          targetDir2,
          newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
      invalid_argument e = expectThrows(invalid_argument::typeid,
                                        [&]() { w->addIndexes({oldDir}); });
      assertTrue(e.what(),
                 e.what()->startsWith(
                     L"Cannot use addIndexes(Directory) with indexes that have "
                     L"been created by a different Lucene version."));
      delete w;
      delete targetDir2;

      // for the next test, we simulate writing to an index that was created on
      // the same major version
      (make_shared<SegmentInfos>(infos->getIndexCreatedVersionMajor()))
          ->commit(targetDir);
    }

    shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
        targetDir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
    w->addIndexes({oldDir});
    delete w;
    delete targetDir;

    if (VERBOSE) {
      wcout << L"\nTEST: done adding indices; now close" << endl;
    }

    delete targetDir;
  }
}

void TestBackwardsCompatibility::testAddOldIndexesReader() 
{
  for (auto name : oldNames) {
    shared_ptr<Directory> oldDir = oldIndexDirs[name];
    shared_ptr<SegmentInfos> infos = SegmentInfos::readLatestCommit(oldDir);
    shared_ptr<DirectoryReader> reader = DirectoryReader::open(oldDir);

    shared_ptr<Directory> targetDir = newDirectory();
    if (infos->getCommitLuceneVersion()->major != Version::LATEST->major) {
      shared_ptr<Directory> targetDir2 = newDirectory();
      shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
          targetDir2,
          newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
      invalid_argument e = expectThrows(invalid_argument::typeid, [&]() {
        org::apache::lucene::util::TestUtil::addIndexesSlowly(w, {reader});
      });
      TestUtil::assertEquals(
          e.what(),
          L"Cannot merge a segment that has been created with major version 6 "
          L"into this index which has been created by major version 7");
      delete w;
      delete targetDir2;

      // for the next test, we simulate writing to an index that was created on
      // the same major version
      (make_shared<SegmentInfos>(infos->getIndexCreatedVersionMajor()))
          ->commit(targetDir);
    }
    shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
        targetDir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
    TestUtil::addIndexesSlowly(w, {reader});
    delete w;
    reader->close();

    delete targetDir;
  }
}

void TestBackwardsCompatibility::testSearchOldIndex() 
{
  for (auto name : oldNames) {
    searchIndex(oldIndexDirs[name], name);
  }
}

void TestBackwardsCompatibility::testIndexOldIndexNoAdds() 
{
  for (auto name : oldNames) {
    shared_ptr<Directory> dir = newDirectory(oldIndexDirs[name]);
    changeIndexNoAdds(random(), dir);
    delete dir;
  }
}

void TestBackwardsCompatibility::testIndexOldIndex() 
{
  for (auto name : oldNames) {
    if (VERBOSE) {
      wcout << L"TEST: oldName=" << name << endl;
    }
    shared_ptr<Directory> dir = newDirectory(oldIndexDirs[name]);
    shared_ptr<Version> v = Version::parse(name.substr(0, name.find(L'-')));
    changeIndexWithAdds(random(), dir, v);
    delete dir;
  }
}

void TestBackwardsCompatibility::doTestHits(
    std::deque<std::shared_ptr<ScoreDoc>> &hits, int expectedCount,
    shared_ptr<IndexReader> reader) 
{
  constexpr int hitCount = hits.size();
  assertEquals(L"wrong number of hits", expectedCount, hitCount);
  for (int i = 0; i < hitCount; i++) {
    reader->document(hits[i]->doc);
    reader->getTermVectors(hits[i]->doc);
  }
}

void TestBackwardsCompatibility::searchIndex(
    shared_ptr<Directory> dir, const wstring &oldName) 
{
  // QueryParser parser = new QueryParser("contents", new MockAnalyzer(random));
  // Query query = parser.parse("handle:1");

  shared_ptr<IndexReader> reader = DirectoryReader::open(dir);
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);

  TestUtil::checkIndex(dir);

  // true if this is a 4.0+ index
  constexpr bool is40Index =
      MultiFields::getMergedFieldInfos(reader)->fieldInfo(L"content5") !=
      nullptr;
  // true if this is a 4.2+ index
  constexpr bool is42Index =
      MultiFields::getMergedFieldInfos(reader)->fieldInfo(L"dvSortedSet") !=
      nullptr;
  // true if this is a 4.9+ index
  constexpr bool is49Index =
      MultiFields::getMergedFieldInfos(reader)->fieldInfo(L"dvSortedNumeric") !=
      nullptr;
  // true if this index has points (>= 6.0)
  constexpr bool hasPoints =
      MultiFields::getMergedFieldInfos(reader)->fieldInfo(L"intPoint1d") !=
      nullptr;

  assert(is40Index);

  shared_ptr<Bits> *const liveDocs = MultiFields::getLiveDocs(reader);

  for (int i = 0; i < 35; i++) {
    if (liveDocs->get(i)) {
      shared_ptr<Document> d = reader->document(i);
      deque<std::shared_ptr<IndexableField>> fields = d->getFields();
      bool isProxDoc = d->getField(L"content3") == nullptr;
      if (isProxDoc) {
        constexpr int numFields = is40Index ? 7 : 5;
        TestUtil::assertEquals(numFields, fields.size());
        shared_ptr<IndexableField> f = d->getField(L"id");
        TestUtil::assertEquals(L"" + to_wstring(i), f->stringValue());

        f = d->getField(L"utf8");
        TestUtil::assertEquals(
            L"Lu\uD834\uDD1Ece\uD834\uDD60ne \u0000 \u2620 ab\ud917\udc17cd",
            f->stringValue());

        f = d->getField(L"autf8");
        TestUtil::assertEquals(
            L"Lu\uD834\uDD1Ece\uD834\uDD60ne \u0000 \u2620 ab\ud917\udc17cd",
            f->stringValue());

        f = d->getField(L"content2");
        TestUtil::assertEquals(L"here is more content with aaa aaa aaa",
                               f->stringValue());

        f = d->getField(L"fie\u2C77ld");
        TestUtil::assertEquals(L"field with non-ascii name", f->stringValue());
      }

      shared_ptr<Fields> tfvFields = reader->getTermVectors(i);
      assertNotNull(L"i=" + to_wstring(i), tfvFields);
      shared_ptr<Terms> tfv = tfvFields->terms(L"utf8");
      assertNotNull(L"docID=" + to_wstring(i) + L" index=" + oldName, tfv);
    } else {
      // Only ID 7 is deleted
      TestUtil::assertEquals(7, i);
    }
  }

  if (is40Index) {
    // check docvalues fields
    shared_ptr<NumericDocValues> dvByte =
        MultiDocValues::getNumericValues(reader, L"dvByte");
    shared_ptr<BinaryDocValues> dvBytesDerefFixed =
        MultiDocValues::getBinaryValues(reader, L"dvBytesDerefFixed");
    shared_ptr<BinaryDocValues> dvBytesDerefVar =
        MultiDocValues::getBinaryValues(reader, L"dvBytesDerefVar");
    shared_ptr<SortedDocValues> dvBytesSortedFixed =
        MultiDocValues::getSortedValues(reader, L"dvBytesSortedFixed");
    shared_ptr<SortedDocValues> dvBytesSortedVar =
        MultiDocValues::getSortedValues(reader, L"dvBytesSortedVar");
    shared_ptr<BinaryDocValues> dvBytesStraightFixed =
        MultiDocValues::getBinaryValues(reader, L"dvBytesStraightFixed");
    shared_ptr<BinaryDocValues> dvBytesStraightVar =
        MultiDocValues::getBinaryValues(reader, L"dvBytesStraightVar");
    shared_ptr<NumericDocValues> dvDouble =
        MultiDocValues::getNumericValues(reader, L"dvDouble");
    shared_ptr<NumericDocValues> dvFloat =
        MultiDocValues::getNumericValues(reader, L"dvFloat");
    shared_ptr<NumericDocValues> dvInt =
        MultiDocValues::getNumericValues(reader, L"dvInt");
    shared_ptr<NumericDocValues> dvLong =
        MultiDocValues::getNumericValues(reader, L"dvLong");
    shared_ptr<NumericDocValues> dvPacked =
        MultiDocValues::getNumericValues(reader, L"dvPacked");
    shared_ptr<NumericDocValues> dvShort =
        MultiDocValues::getNumericValues(reader, L"dvShort");
    shared_ptr<SortedSetDocValues> dvSortedSet = nullptr;
    if (is42Index) {
      dvSortedSet = MultiDocValues::getSortedSetValues(reader, L"dvSortedSet");
    }
    shared_ptr<SortedNumericDocValues> dvSortedNumeric = nullptr;
    if (is49Index) {
      dvSortedNumeric =
          MultiDocValues::getSortedNumericValues(reader, L"dvSortedNumeric");
    }

    for (int i = 0; i < 35; i++) {
      int id = static_cast<Integer>(reader->document(i)[L"id"]);
      TestUtil::assertEquals(i, dvByte->nextDoc());
      TestUtil::assertEquals(id, dvByte->longValue());

      std::deque<char> bytes = {static_cast<char>(static_cast<int>(
                                     static_cast<unsigned int>(id) >> 24)),
                                 static_cast<char>(static_cast<int>(
                                     static_cast<unsigned int>(id) >> 16)),
                                 static_cast<char>(static_cast<int>(
                                     static_cast<unsigned int>(id) >> 8)),
                                 static_cast<char>(id)};
      shared_ptr<BytesRef> expectedRef = make_shared<BytesRef>(bytes);

      TestUtil::assertEquals(i, dvBytesDerefFixed->nextDoc());
      shared_ptr<BytesRef> term = dvBytesDerefFixed->binaryValue();
      TestUtil::assertEquals(expectedRef, term);
      TestUtil::assertEquals(i, dvBytesDerefVar->nextDoc());
      term = dvBytesDerefVar->binaryValue();
      TestUtil::assertEquals(expectedRef, term);
      TestUtil::assertEquals(i, dvBytesSortedFixed->nextDoc());
      term = dvBytesSortedFixed->binaryValue();
      TestUtil::assertEquals(expectedRef, term);
      TestUtil::assertEquals(i, dvBytesSortedVar->nextDoc());
      term = dvBytesSortedVar->binaryValue();
      TestUtil::assertEquals(expectedRef, term);
      TestUtil::assertEquals(i, dvBytesStraightFixed->nextDoc());
      term = dvBytesStraightFixed->binaryValue();
      TestUtil::assertEquals(expectedRef, term);
      TestUtil::assertEquals(i, dvBytesStraightVar->nextDoc());
      term = dvBytesStraightVar->binaryValue();
      TestUtil::assertEquals(expectedRef, term);

      TestUtil::assertEquals(i, dvDouble->nextDoc());
      assertEquals(static_cast<double>(id),
                   Double::longBitsToDouble(dvDouble->longValue()), 0);
      TestUtil::assertEquals(i, dvFloat->nextDoc());
      assertEquals(
          static_cast<float>(id),
          Float::intBitsToFloat(static_cast<int>(dvFloat->longValue())), 0.0F);
      TestUtil::assertEquals(i, dvInt->nextDoc());
      TestUtil::assertEquals(id, dvInt->longValue());
      TestUtil::assertEquals(i, dvLong->nextDoc());
      TestUtil::assertEquals(id, dvLong->longValue());
      TestUtil::assertEquals(i, dvPacked->nextDoc());
      TestUtil::assertEquals(id, dvPacked->longValue());
      TestUtil::assertEquals(i, dvShort->nextDoc());
      TestUtil::assertEquals(id, dvShort->longValue());
      if (is42Index) {
        TestUtil::assertEquals(i, dvSortedSet->nextDoc());
        int64_t ord = dvSortedSet->nextOrd();
        TestUtil::assertEquals(SortedSetDocValues::NO_MORE_ORDS,
                               dvSortedSet->nextOrd());
        term = dvSortedSet->lookupOrd(ord);
        TestUtil::assertEquals(expectedRef, term);
      }
      if (is49Index) {
        TestUtil::assertEquals(i, dvSortedNumeric->nextDoc());
        TestUtil::assertEquals(1, dvSortedNumeric->docValueCount());
        TestUtil::assertEquals(id, dvSortedNumeric->nextValue());
      }
    }
  }

  std::deque<std::shared_ptr<ScoreDoc>> hits =
      searcher
          ->search(make_shared<TermQuery>(
                       make_shared<Term>(wstring(L"content"), L"aaa")),
                   1000)
          ->scoreDocs;

  // First document should be #0
  shared_ptr<Document> d = searcher->getIndexReader()->document(hits[0]->doc);
  assertEquals(L"didn't get the right document first", L"0", d[L"id"]);

  doTestHits(hits, 34, searcher->getIndexReader());

  if (is40Index) {
    hits = searcher
               ->search(make_shared<TermQuery>(
                            make_shared<Term>(wstring(L"content5"), L"aaa")),
                        1000)
               ->scoreDocs;

    doTestHits(hits, 34, searcher->getIndexReader());

    hits = searcher
               ->search(make_shared<TermQuery>(
                            make_shared<Term>(wstring(L"content6"), L"aaa")),
                        1000)
               ->scoreDocs;

    doTestHits(hits, 34, searcher->getIndexReader());
  }

  hits = searcher
             ->search(
                 make_shared<TermQuery>(make_shared<Term>(L"utf8", L"\u0000")),
                 1000)
             ->scoreDocs;
  TestUtil::assertEquals(34, hits.size());
  hits = searcher
             ->search(make_shared<TermQuery>(make_shared<Term>(
                          wstring(L"utf8"), L"lu\uD834\uDD1Ece\uD834\uDD60ne")),
                      1000)
             ->scoreDocs;
  TestUtil::assertEquals(34, hits.size());
  hits = searcher
             ->search(make_shared<TermQuery>(
                          make_shared<Term>(L"utf8", L"ab\ud917\udc17cd")),
                      1000)
             ->scoreDocs;
  TestUtil::assertEquals(34, hits.size());

  if (hasPoints) {
    doTestHits(
        searcher->search(IntPoint::newRangeQuery(L"intPoint1d", 0, 34), 1000)
            ->scoreDocs,
        34, searcher->getIndexReader());
    doTestHits(searcher
                   ->search(IntPoint::newRangeQuery(L"intPoint2d",
                                                    std::deque<int>{0, 0},
                                                    std::deque<int>{34, 68}),
                            1000)
                   ->scoreDocs,
               34, searcher->getIndexReader());
    doTestHits(
        searcher
            ->search(FloatPoint::newRangeQuery(L"floatPoint1d", 0.0f, 34.0f),
                     1000)
            ->scoreDocs,
        34, searcher->getIndexReader());
    doTestHits(searcher
                   ->search(FloatPoint::newRangeQuery(
                                L"floatPoint2d", std::deque<float>{0.0f, 0.0f},
                                std::deque<float>{34.0f, 68.0f}),
                            1000)
                   ->scoreDocs,
               34, searcher->getIndexReader());
    doTestHits(
        searcher->search(LongPoint::newRangeQuery(L"longPoint1d", 0, 34), 1000)
            ->scoreDocs,
        34, searcher->getIndexReader());
    doTestHits(searcher
                   ->search(LongPoint::newRangeQuery(
                                L"longPoint2d", std::deque<int64_t>{0, 0},
                                std::deque<int64_t>{34, 68}),
                            1000)
                   ->scoreDocs,
               34, searcher->getIndexReader());
    doTestHits(
        searcher
            ->search(DoublePoint::newRangeQuery(L"doublePoint1d", 0.0, 34.0),
                     1000)
            ->scoreDocs,
        34, searcher->getIndexReader());
    doTestHits(searcher
                   ->search(DoublePoint::newRangeQuery(
                                L"doublePoint2d", std::deque<double>{0.0, 0.0},
                                std::deque<double>{34.0, 68.0}),
                            1000)
                   ->scoreDocs,
               34, searcher->getIndexReader());

    std::deque<char> bytes1(4);
    std::deque<char> bytes2 = {0, 0, 0, static_cast<char>(34)};
    doTestHits(searcher
                   ->search(BinaryPoint::newRangeQuery(L"binaryPoint1d", bytes1,
                                                       bytes2),
                            1000)
                   ->scoreDocs,
               34, searcher->getIndexReader());
    std::deque<char> bytes3 = {0, 0, 0, static_cast<char>(68)};
    doTestHits(searcher
                   ->search(BinaryPoint::newRangeQuery(
                                L"binaryPoint2d",
                                std::deque<std::deque<char>>{bytes1, bytes1},
                                std::deque<std::deque<char>>{bytes2, bytes3}),
                            1000)
                   ->scoreDocs,
               34, searcher->getIndexReader());
  }

  delete reader;
}

void TestBackwardsCompatibility::changeIndexWithAdds(
    shared_ptr<Random> random, shared_ptr<Directory> dir,
    shared_ptr<Version> nameVersion) 
{
  shared_ptr<SegmentInfos> infos = SegmentInfos::readLatestCommit(dir);
  TestUtil::assertEquals(nameVersion, infos->getCommitLuceneVersion());
  TestUtil::assertEquals(nameVersion, infos->getMinSegmentLuceneVersion());

  // open writer
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random))
               ->setOpenMode(OpenMode::APPEND)
               ->setMergePolicy(newLogMergePolicy()));
  // add 10 docs
  for (int i = 0; i < 10; i++) {
    addDoc(writer, 35 + i);
  }

  // make sure writer sees right total -- writer seems not to know about deletes
  // in .del?
  constexpr int expected = 45;
  assertEquals(L"wrong doc count", expected, writer->numDocs());
  delete writer;

  // make sure searching sees right # hits
  shared_ptr<IndexReader> reader = DirectoryReader::open(dir);
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);
  std::deque<std::shared_ptr<ScoreDoc>> hits =
      searcher
          ->search(
              make_shared<TermQuery>(make_shared<Term>(L"content", L"aaa")),
              1000)
          ->scoreDocs;
  shared_ptr<Document> d = searcher->getIndexReader()->document(hits[0]->doc);
  assertEquals(L"wrong first document", L"0", d[L"id"]);
  doTestHits(hits, 44, searcher->getIndexReader());
  delete reader;

  // fully merge
  writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random))
               ->setOpenMode(OpenMode::APPEND)
               ->setMergePolicy(newLogMergePolicy()));
  writer->forceMerge(1);
  delete writer;

  reader = DirectoryReader::open(dir);
  searcher = newSearcher(reader);
  hits = searcher
             ->search(
                 make_shared<TermQuery>(make_shared<Term>(L"content", L"aaa")),
                 1000)
             ->scoreDocs;
  assertEquals(L"wrong number of hits", 44, hits.size());
  d = searcher->doc(hits[0]->doc);
  doTestHits(hits, 44, searcher->getIndexReader());
  assertEquals(L"wrong first document", L"0", d[L"id"]);
  delete reader;
}

void TestBackwardsCompatibility::changeIndexNoAdds(
    shared_ptr<Random> random, shared_ptr<Directory> dir) 
{
  // make sure searching sees right # hits
  shared_ptr<DirectoryReader> reader = DirectoryReader::open(dir);
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);
  std::deque<std::shared_ptr<ScoreDoc>> hits =
      searcher
          ->search(
              make_shared<TermQuery>(make_shared<Term>(L"content", L"aaa")),
              1000)
          ->scoreDocs;
  assertEquals(L"wrong number of hits", 34, hits.size());
  shared_ptr<Document> d = searcher->doc(hits[0]->doc);
  assertEquals(L"wrong first document", L"0", d[L"id"]);
  reader->close();

  // fully merge
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random))
               ->setOpenMode(OpenMode::APPEND));
  writer->forceMerge(1);
  delete writer;

  reader = DirectoryReader::open(dir);
  searcher = newSearcher(reader);
  hits = searcher
             ->search(
                 make_shared<TermQuery>(make_shared<Term>(L"content", L"aaa")),
                 1000)
             ->scoreDocs;
  assertEquals(L"wrong number of hits", 34, hits.size());
  doTestHits(hits, 34, searcher->getIndexReader());
  reader->close();
}

void TestBackwardsCompatibility::createIndex(
    const wstring &dirName, bool doCFS, bool fullyMerged) 
{
  shared_ptr<Path> indexDir = getIndexDir()->resolve(dirName);
  Files::deleteIfExists(indexDir);
  shared_ptr<Directory> dir = newFSDirectory(indexDir);
  shared_ptr<LogByteSizeMergePolicy> mp = make_shared<LogByteSizeMergePolicy>();
  mp->setNoCFSRatio(doCFS ? 1.0 : 0.0);
  mp->setMaxCFSSegmentSizeMB(numeric_limits<double>::infinity());
  // TODO: remove randomness
  shared_ptr<IndexWriterConfig> conf =
      (make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random())))
          ->setMaxBufferedDocs(10)
          ->setMergePolicy(NoMergePolicy::INSTANCE);
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);

  for (int i = 0; i < 35; i++) {
    addDoc(writer, i);
  }
  assertEquals(L"wrong doc count", 35, writer->maxDoc());
  if (fullyMerged) {
    writer->forceMerge(1);
  }
  delete writer;

  if (!fullyMerged) {
    // open fresh writer so we get no prx file in the added segment
    mp = make_shared<LogByteSizeMergePolicy>();
    mp->setNoCFSRatio(doCFS ? 1.0 : 0.0);
    // TODO: remove randomness
    conf = (make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random())))
               ->setMaxBufferedDocs(10)
               ->setMergePolicy(NoMergePolicy::INSTANCE);
    writer = make_shared<IndexWriter>(dir, conf);
    addNoProxDoc(writer);
    delete writer;

    conf = (make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random())))
               ->setMaxBufferedDocs(10)
               ->setMergePolicy(NoMergePolicy::INSTANCE);
    writer = make_shared<IndexWriter>(dir, conf);
    shared_ptr<Term> searchTerm = make_shared<Term>(L"id", L"7");
    writer->deleteDocuments({searchTerm});
    delete writer;
  }

  delete dir;
}

void TestBackwardsCompatibility::addDoc(shared_ptr<IndexWriter> writer,
                                        int id) 
{
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<TextField>(L"content", L"aaa", Field::Store::NO));
  // C++ TODO: There is no native C++ equivalent to 'toString':
  doc->push_back(make_shared<StringField>(L"id", Integer::toString(id),
                                          Field::Store::YES));
  shared_ptr<FieldType> customType2 =
      make_shared<FieldType>(TextField::TYPE_STORED);
  customType2->setStoreTermVectors(true);
  customType2->setStoreTermVectorPositions(true);
  customType2->setStoreTermVectorOffsets(true);
  doc->push_back(make_shared<Field>(
      L"autf8",
      L"Lu\uD834\uDD1Ece\uD834\uDD60ne \u0000 \u2620 ab\ud917\udc17cd",
      customType2));
  doc->push_back(make_shared<Field>(
      L"utf8", L"Lu\uD834\uDD1Ece\uD834\uDD60ne \u0000 \u2620 ab\ud917\udc17cd",
      customType2));
  doc->push_back(make_shared<Field>(
      L"content2", L"here is more content with aaa aaa aaa", customType2));
  doc->push_back(make_shared<Field>(L"fie\u2C77ld",
                                    L"field with non-ascii name", customType2));

  // add docvalues fields
  doc->push_back(
      make_shared<NumericDocValuesField>(L"dvByte", static_cast<char>(id)));
  std::deque<char> bytes = {
      static_cast<char>(static_cast<int>(static_cast<unsigned int>(id) >> 24)),
      static_cast<char>(static_cast<int>(static_cast<unsigned int>(id) >> 16)),
      static_cast<char>(static_cast<int>(static_cast<unsigned int>(id) >> 8)),
      static_cast<char>(id)};
  shared_ptr<BytesRef> ref = make_shared<BytesRef>(bytes);
  doc->push_back(make_shared<BinaryDocValuesField>(L"dvBytesDerefFixed", ref));
  doc->push_back(make_shared<BinaryDocValuesField>(L"dvBytesDerefVar", ref));
  doc->push_back(make_shared<SortedDocValuesField>(L"dvBytesSortedFixed", ref));
  doc->push_back(make_shared<SortedDocValuesField>(L"dvBytesSortedVar", ref));
  doc->push_back(
      make_shared<BinaryDocValuesField>(L"dvBytesStraightFixed", ref));
  doc->push_back(make_shared<BinaryDocValuesField>(L"dvBytesStraightVar", ref));
  doc->push_back(
      make_shared<DoubleDocValuesField>(L"dvDouble", static_cast<double>(id)));
  doc->push_back(
      make_shared<FloatDocValuesField>(L"dvFloat", static_cast<float>(id)));
  doc->push_back(make_shared<NumericDocValuesField>(L"dvInt", id));
  doc->push_back(make_shared<NumericDocValuesField>(L"dvLong", id));
  doc->push_back(make_shared<NumericDocValuesField>(L"dvPacked", id));
  doc->push_back(
      make_shared<NumericDocValuesField>(L"dvShort", static_cast<short>(id)));
  doc->push_back(make_shared<SortedSetDocValuesField>(L"dvSortedSet", ref));
  doc->push_back(
      make_shared<SortedNumericDocValuesField>(L"dvSortedNumeric", id));

  doc->push_back(make_shared<IntPoint>(L"intPoint1d", id));
  doc->push_back(make_shared<IntPoint>(L"intPoint2d", id, 2 * id));
  doc->push_back(
      make_shared<FloatPoint>(L"floatPoint1d", static_cast<float>(id)));
  doc->push_back(make_shared<FloatPoint>(
      L"floatPoint2d", static_cast<float>(id), static_cast<float>(2) * id));
  doc->push_back(make_shared<LongPoint>(L"longPoint1d", id));
  doc->push_back(make_shared<LongPoint>(L"longPoint2d", id, 2 * id));
  doc->push_back(
      make_shared<DoublePoint>(L"doublePoint1d", static_cast<double>(id)));
  doc->push_back(make_shared<DoublePoint>(
      L"doublePoint2d", static_cast<double>(id), static_cast<double>(2) * id));
  doc->push_back(make_shared<BinaryPoint>(L"binaryPoint1d", bytes));
  doc->push_back(make_shared<BinaryPoint>(L"binaryPoint2d", bytes, bytes));

  // a field with both offsets and term vectors for a cross-check
  shared_ptr<FieldType> customType3 =
      make_shared<FieldType>(TextField::TYPE_STORED);
  customType3->setStoreTermVectors(true);
  customType3->setStoreTermVectorPositions(true);
  customType3->setStoreTermVectorOffsets(true);
  customType3->setIndexOptions(
      IndexOptions::DOCS_AND_FREQS_AND_POSITIONS_AND_OFFSETS);
  doc->push_back(make_shared<Field>(
      L"content5", L"here is more content with aaa aaa aaa", customType3));
  // a field that omits only positions
  shared_ptr<FieldType> customType4 =
      make_shared<FieldType>(TextField::TYPE_STORED);
  customType4->setStoreTermVectors(true);
  customType4->setStoreTermVectorPositions(false);
  customType4->setStoreTermVectorOffsets(true);
  customType4->setIndexOptions(IndexOptions::DOCS_AND_FREQS);
  doc->push_back(make_shared<Field>(
      L"content6", L"here is more content with aaa aaa aaa", customType4));
  // TODO:
  //   index different norms types via similarity (we use a random one
  //   currently?!) remove any analyzer randomness, explicitly add payloads for
  //   certain fields.
  writer->addDocument(doc);
}

void TestBackwardsCompatibility::addNoProxDoc(
    shared_ptr<IndexWriter> writer) 
{
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<FieldType> customType =
      make_shared<FieldType>(TextField::TYPE_STORED);
  customType->setIndexOptions(IndexOptions::DOCS);
  shared_ptr<Field> f = make_shared<Field>(L"content3", L"aaa", customType);
  doc->push_back(f);
  shared_ptr<FieldType> customType2 = make_shared<FieldType>();
  customType2->setStored(true);
  customType2->setIndexOptions(IndexOptions::DOCS);
  f = make_shared<Field>(L"content4", L"aaa", customType2);
  doc->push_back(f);
  writer->addDocument(doc);
}

int TestBackwardsCompatibility::countDocs(shared_ptr<PostingsEnum> docs) throw(
    IOException)
{
  int count = 0;
  while ((docs->nextDoc()) != DocIdSetIterator::NO_MORE_DOCS) {
    count++;
  }
  return count;
}

void TestBackwardsCompatibility::testNextIntoWrongField() 
{
  for (auto name : oldNames) {
    shared_ptr<Directory> dir = oldIndexDirs[name];
    shared_ptr<IndexReader> r = DirectoryReader::open(dir);
    shared_ptr<TermsEnum> terms = MultiFields::getTerms(r, L"content")->begin();
    shared_ptr<BytesRef> t = terms->next();
    assertNotNull(t);

    // content field only has term aaa:
    TestUtil::assertEquals(L"aaa", t->utf8ToString());
    assertNull(terms->next());

    shared_ptr<BytesRef> aaaTerm = make_shared<BytesRef>(L"aaa");

    // should be found exactly
    TestUtil::assertEquals(TermsEnum::SeekStatus::FOUND,
                           terms->seekCeil(aaaTerm));
    TestUtil::assertEquals(35,
                           countDocs(TestUtil::docs(random(), terms, nullptr,
                                                    PostingsEnum::NONE)));
    assertNull(terms->next());

    // should hit end of field
    TestUtil::assertEquals(TermsEnum::SeekStatus::END,
                           terms->seekCeil(make_shared<BytesRef>(L"bbb")));
    assertNull(terms->next());

    // should seek to aaa
    TestUtil::assertEquals(TermsEnum::SeekStatus::NOT_FOUND,
                           terms->seekCeil(make_shared<BytesRef>(L"a")));
    assertTrue(terms->term()->bytesEquals(aaaTerm));
    TestUtil::assertEquals(35,
                           countDocs(TestUtil::docs(random(), terms, nullptr,
                                                    PostingsEnum::NONE)));
    assertNull(terms->next());

    TestUtil::assertEquals(TermsEnum::SeekStatus::FOUND,
                           terms->seekCeil(aaaTerm));
    TestUtil::assertEquals(35,
                           countDocs(TestUtil::docs(random(), terms, nullptr,
                                                    PostingsEnum::NONE)));
    assertNull(terms->next());

    delete r;
  }
}

void TestBackwardsCompatibility::testOldVersions() 
{
  // first create a little index with the current code and get the version
  shared_ptr<Directory> currentDir = newDirectory();
  shared_ptr<RandomIndexWriter> riw =
      make_shared<RandomIndexWriter>(random(), currentDir);
  riw->addDocument(make_shared<Document>());
  delete riw;
  shared_ptr<DirectoryReader> ir = DirectoryReader::open(currentDir);
  shared_ptr<SegmentReader> air =
      std::static_pointer_cast<SegmentReader>(ir->leaves()->get(0).reader());
  shared_ptr<Version> currentVersion =
      air->getSegmentInfo()->info->getVersion();
  assertNotNull(currentVersion); // only 3.0 segments can have a null version
  ir->close();
  delete currentDir;

  // now check all the old indexes, their version should be < the current
  // version
  for (auto name : oldNames) {
    shared_ptr<Directory> dir = oldIndexDirs[name];
    shared_ptr<DirectoryReader> r = DirectoryReader::open(dir);
    for (shared_ptr<LeafReaderContext> context : r->leaves()) {
      air = std::static_pointer_cast<SegmentReader>(context->reader());
      shared_ptr<Version> oldVersion =
          air->getSegmentInfo()->info->getVersion();
      assertNotNull(oldVersion); // only 3.0 segments can have a null version
      assertTrue(L"current Version.LATEST is <= an old index: did you forget "
                 L"to bump it?!",
                 currentVersion->onOrAfter(oldVersion));
    }
    r->close();
  }
}

void TestBackwardsCompatibility::testIndexCreatedVersion() 
{
  for (auto name : oldNames) {
    shared_ptr<Directory> dir = oldIndexDirs[name];
    shared_ptr<SegmentInfos> infos = SegmentInfos::readLatestCommit(dir);
    // those indexes are created by a single version so we can
    // compare the commit version with the created version
    TestUtil::assertEquals(infos->getCommitLuceneVersion()->major,
                           infos->getIndexCreatedVersionMajor());
  }
}

void TestBackwardsCompatibility::verifyUsesDefaultCodec(
    shared_ptr<Directory> dir, const wstring &name) 
{
  shared_ptr<DirectoryReader> r = DirectoryReader::open(dir);
  for (shared_ptr<LeafReaderContext> context : r->leaves()) {
    shared_ptr<SegmentReader> air =
        std::static_pointer_cast<SegmentReader>(context->reader());
    shared_ptr<Codec> codec = air->getSegmentInfo()->info->getCodec();
    assertTrue(L"codec used in " + name + L" (" + codec->getName() +
                   L") is not a default codec (does not begin with Lucene)",
               StringHelper::startsWith(codec->getName(), L"Lucene"));
  }
  r->close();
}

void TestBackwardsCompatibility::testAllIndexesUseDefaultCodec() throw(
    runtime_error)
{
  for (auto name : oldNames) {
    shared_ptr<Directory> dir = oldIndexDirs[name];
    verifyUsesDefaultCodec(dir, name);
  }
}

int TestBackwardsCompatibility::checkAllSegmentsUpgraded(
    shared_ptr<Directory> dir, int indexCreatedVersion) 
{
  shared_ptr<SegmentInfos> *const infos = SegmentInfos::readLatestCommit(dir);
  if (VERBOSE) {
    wcout << L"checkAllSegmentsUpgraded: " << infos << endl;
  }
  for (auto si : infos) {
    TestUtil::assertEquals(Version::LATEST, si->info->getVersion());
  }
  TestUtil::assertEquals(Version::LATEST, infos->getCommitLuceneVersion());
  TestUtil::assertEquals(indexCreatedVersion,
                         infos->getIndexCreatedVersionMajor());
  return infos->size();
}

int TestBackwardsCompatibility::getNumberOfSegments(
    shared_ptr<Directory> dir) 
{
  shared_ptr<SegmentInfos> *const infos = SegmentInfos::readLatestCommit(dir);
  return infos->size();
}

void TestBackwardsCompatibility::testUpgradeOldIndex() 
{
  deque<wstring> names =
      deque<wstring>(oldNames.size() + oldSingleSegmentNames.size());
  names.addAll(Arrays::asList(oldNames));
  names.addAll(Arrays::asList(oldSingleSegmentNames));
  for (auto name : names) {
    if (VERBOSE) {
      wcout << L"testUpgradeOldIndex: index=" << name << endl;
    }
    shared_ptr<Directory> dir = newDirectory(oldIndexDirs[name]);
    int indexCreatedVersion =
        SegmentInfos::readLatestCommit(dir)->getIndexCreatedVersionMajor();

    newIndexUpgrader(dir)->upgrade();

    checkAllSegmentsUpgraded(dir, indexCreatedVersion);

    delete dir;
  }
}

void TestBackwardsCompatibility::testCommandLineArgs() 
{

  shared_ptr<PrintStream> savedSystemOut = System::out;
  System::setOut(make_shared<PrintStream>(make_shared<ByteArrayOutputStream>(),
                                          false, L"UTF-8"));
  try {
    for (auto entry : oldIndexDirs) {
      wstring name = entry.first;
      int indexCreatedVersion = SegmentInfos::readLatestCommit(entry.second)
                                    ->getIndexCreatedVersionMajor();
      shared_ptr<Path> dir = createTempDir(name);
      TestUtil::unzip(getDataInputStream(L"index." + name + L".zip"), dir);

      // C++ TODO: There is no native C++ equivalent to 'toString':
      wstring path = dir->toAbsolutePath()->toString();

      deque<wstring> args = deque<wstring>();
      if (random()->nextBoolean()) {
        args.push_back(L"-verbose");
      }
      if (random()->nextBoolean()) {
        args.push_back(L"-delete-prior-commits");
      }
      if (random()->nextBoolean()) {
        // TODO: need to better randomize this, but ...
        //  - LuceneTestCase.FS_DIRECTORIES is private
        //  - newFSDirectory returns BaseDirectoryWrapper
        //  - BaseDirectoryWrapper doesn't expose delegate
        type_info dirImpl = random()->nextBoolean() ? SimpleFSDirectory::typeid
                                                    : NIOFSDirectory::typeid;

        args.push_back(L"-dir-impl");
        args.push_back(dirImpl.getName());
      }
      args.push_back(path);

      shared_ptr<IndexUpgrader> upgrader = nullptr;
      try {
        upgrader =
            IndexUpgrader::parseArgs(args.toArray(std::deque<wstring>(0)));
      } catch (const runtime_error &e) {
        throw make_shared<AssertionError>(L"unable to parse args: " + args, e);
      }
      upgrader->upgrade();

      shared_ptr<Directory> upgradedDir = newFSDirectory(dir);
      try {
        checkAllSegmentsUpgraded(upgradedDir, indexCreatedVersion);
      }
      // C++ TODO: There is no native C++ equivalent to the exception 'finally'
      // clause:
      finally {
        delete upgradedDir;
      }
    }
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    System::setOut(savedSystemOut);
  }
}

void TestBackwardsCompatibility::
    testUpgradeOldSingleSegmentIndexWithAdditions() 
{
  for (auto name : oldSingleSegmentNames) {
    if (VERBOSE) {
      wcout << L"testUpgradeOldSingleSegmentIndexWithAdditions: index=" << name
            << endl;
    }
    shared_ptr<Directory> dir = newDirectory(oldIndexDirs[name]);
    assertEquals(L"Original index must be single segment", 1,
                 getNumberOfSegments(dir));
    int indexCreatedVersion =
        SegmentInfos::readLatestCommit(dir)->getIndexCreatedVersionMajor();

    // create a bunch of dummy segments
    int id = 40;
    shared_ptr<RAMDirectory> ramDir = make_shared<RAMDirectory>();
    for (int i = 0; i < 3; i++) {
      // only use Log- or TieredMergePolicy, to make document addition
      // predictable and not suddenly merge:
      shared_ptr<MergePolicy> mp = random()->nextBoolean()
                                       ? newLogMergePolicy()
                                       : newTieredMergePolicy();
      shared_ptr<IndexWriterConfig> iwc =
          (make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random())))
              ->setMergePolicy(mp);
      shared_ptr<IndexWriter> w = make_shared<IndexWriter>(ramDir, iwc);
      // add few more docs:
      for (int j = 0; j < RANDOM_MULTIPLIER * random()->nextInt(30); j++) {
        addDoc(w, id++);
      }
      try {
        w->commit();
      }
      // C++ TODO: There is no native C++ equivalent to the exception 'finally'
      // clause:
      finally {
        delete w;
      }
    }

    // add dummy segments (which are all in current
    // version) to single segment index
    shared_ptr<MergePolicy> mp =
        random()->nextBoolean() ? newLogMergePolicy() : newTieredMergePolicy();
    shared_ptr<IndexWriterConfig> iwc =
        (make_shared<IndexWriterConfig>(nullptr))->setMergePolicy(mp);
    shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);
    w->addIndexes({ramDir});
    try {
      w->commit();
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      delete w;
    }

    // determine count of segments in modified index
    constexpr int origSegCount = getNumberOfSegments(dir);

    // ensure there is only one commit
    TestUtil::assertEquals(1, DirectoryReader::listCommits(dir).size());
    newIndexUpgrader(dir)->upgrade();

    constexpr int segCount = checkAllSegmentsUpgraded(dir, indexCreatedVersion);
    assertEquals(L"Index must still contain the same number of segments, as "
                 L"only one segment was upgraded and nothing else merged",
                 origSegCount, segCount);

    delete dir;
  }
}

const wstring TestBackwardsCompatibility::emptyIndex = L"empty.6.0.0.zip";

void TestBackwardsCompatibility::testUpgradeEmptyOldIndex() 
{
  shared_ptr<Path> oldIndexDir = createTempDir(L"emptyIndex");
  TestUtil::unzip(getDataInputStream(emptyIndex), oldIndexDir);
  shared_ptr<Directory> dir = newFSDirectory(oldIndexDir);

  newIndexUpgrader(dir)->upgrade();

  checkAllSegmentsUpgraded(dir, 6);

  delete dir;
}

const wstring TestBackwardsCompatibility::moreTermsIndex =
    L"moreterms.6.0.0.zip";

void TestBackwardsCompatibility::testMoreTerms() 
{
  shared_ptr<Path> oldIndexDir = createTempDir(L"moreterms");
  TestUtil::unzip(getDataInputStream(moreTermsIndex), oldIndexDir);
  shared_ptr<Directory> dir = newFSDirectory(oldIndexDir);
  verifyUsesDefaultCodec(dir, moreTermsIndex);
  // TODO: more tests
  TestUtil::checkIndex(dir);
  delete dir;
}

const wstring TestBackwardsCompatibility::dvUpdatesIndex =
    L"dvupdates.6.0.0.zip";

void TestBackwardsCompatibility::assertNumericDocValues(
    shared_ptr<LeafReader> r, const wstring &f,
    const wstring &cf) 
{
  shared_ptr<NumericDocValues> ndvf = r->getNumericDocValues(f);
  shared_ptr<NumericDocValues> ndvcf = r->getNumericDocValues(cf);
  for (int i = 0; i < r->maxDoc(); i++) {
    TestUtil::assertEquals(i, ndvcf->nextDoc());
    TestUtil::assertEquals(i, ndvf->nextDoc());
    TestUtil::assertEquals(ndvcf->longValue(), ndvf->longValue() * 2);
  }
}

void TestBackwardsCompatibility::assertBinaryDocValues(
    shared_ptr<LeafReader> r, const wstring &f,
    const wstring &cf) 
{
  shared_ptr<BinaryDocValues> bdvf = r->getBinaryDocValues(f);
  shared_ptr<BinaryDocValues> bdvcf = r->getBinaryDocValues(cf);
  for (int i = 0; i < r->maxDoc(); i++) {
    TestUtil::assertEquals(i, bdvf->nextDoc());
    TestUtil::assertEquals(i, bdvcf->nextDoc());
    TestUtil::assertEquals(getValue(bdvcf), getValue(bdvf) * 2);
  }
}

void TestBackwardsCompatibility::verifyDocValues(
    shared_ptr<Directory> dir) 
{
  shared_ptr<DirectoryReader> reader = DirectoryReader::open(dir);
  for (shared_ptr<LeafReaderContext> context : reader->leaves()) {
    shared_ptr<LeafReader> r = context->reader();
    assertNumericDocValues(r, L"ndv1", L"ndv1_c");
    assertNumericDocValues(r, L"ndv2", L"ndv2_c");
    assertBinaryDocValues(r, L"bdv1", L"bdv1_c");
    assertBinaryDocValues(r, L"bdv2", L"bdv2_c");
  }
  reader->close();
}

void TestBackwardsCompatibility::testDocValuesUpdates() 
{
  shared_ptr<Path> oldIndexDir = createTempDir(L"dvupdates");
  TestUtil::unzip(getDataInputStream(dvUpdatesIndex), oldIndexDir);
  shared_ptr<Directory> dir = newFSDirectory(oldIndexDir);
  verifyUsesDefaultCodec(dir, dvUpdatesIndex);

  verifyDocValues(dir);

  // update fields and verify index
  shared_ptr<IndexWriterConfig> conf =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);
  updateNumeric(writer, L"1", L"ndv1", L"ndv1_c", 300LL);
  updateNumeric(writer, L"1", L"ndv2", L"ndv2_c", 300LL);
  updateBinary(writer, L"1", L"bdv1", L"bdv1_c", 300LL);
  updateBinary(writer, L"1", L"bdv2", L"bdv2_c", 300LL);
  writer->commit();
  verifyDocValues(dir);

  // merge all segments
  writer->forceMerge(1);
  writer->commit();
  verifyDocValues(dir);

  delete writer;
  delete dir;
}

void TestBackwardsCompatibility::testUpgradeWithNRTReader() 
{
  for (auto name : oldNames) {
    shared_ptr<Directory> dir = newDirectory(oldIndexDirs[name]);

    shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
        dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
                 ->setOpenMode(OpenMode::APPEND));
    writer->addDocument(make_shared<Document>());
    shared_ptr<DirectoryReader> r = DirectoryReader::open(writer);
    writer->commit();
    r->close();
    writer->forceMerge(1);
    writer->commit();
    writer->rollback();
    SegmentInfos::readLatestCommit(dir);
    delete dir;
  }
}

void TestBackwardsCompatibility::testUpgradeThenMultipleCommits() throw(
    runtime_error)
{
  for (auto name : oldNames) {
    shared_ptr<Directory> dir = newDirectory(oldIndexDirs[name]);

    shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
        dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
                 ->setOpenMode(OpenMode::APPEND));
    writer->addDocument(make_shared<Document>());
    writer->commit();
    writer->addDocument(make_shared<Document>());
    writer->commit();
    delete writer;
    delete dir;
  }
}

void TestBackwardsCompatibility::testSortedIndex() 
{
  for (auto name : oldSortedNames) {
    shared_ptr<Path> path = createTempDir(L"sorted");
    shared_ptr<InputStream> resource =
        TestBackwardsCompatibility::typeid->getResourceAsStream(name + L".zip");
    assertNotNull(L"Sorted index index " + name + L" not found", resource);
    TestUtil::unzip(resource, path);

    // TODO: more tests
    shared_ptr<Directory> dir = newFSDirectory(path);

    shared_ptr<DirectoryReader> reader = DirectoryReader::open(dir);
    TestUtil::assertEquals(1, reader->leaves()->size());
    shared_ptr<Sort> sort =
        reader->leaves()->get(0).reader().getMetaData().getSort();
    assertNotNull(sort);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    TestUtil::assertEquals(L"<long: \"dateDV\">!", sort->toString());
    reader->close();

    // this will confirm the docs really are sorted:
    TestUtil::checkIndex(dir);
    delete dir;
  }
}

int64_t TestBackwardsCompatibility::getValue(
    shared_ptr<BinaryDocValues> bdv) 
{
  shared_ptr<BytesRef> term = bdv->binaryValue();
  int idx = term->offset;
  char b = term->bytes[idx++];
  int64_t value = b & 0x7FLL;
  for (int shift = 7; (b & 0x80LL) != 0; shift += 7) {
    b = term->bytes[idx++];
    value |= (b & 0x7FLL) << shift;
  }
  return value;
}

shared_ptr<BytesRef> TestBackwardsCompatibility::toBytes(int64_t value)
{
  shared_ptr<BytesRef> bytes =
      make_shared<BytesRef>(10); // negative longs may take 10 bytes
  while ((value & ~0x7FLL) != 0LL) {
    bytes->bytes[bytes->length++] =
        static_cast<char>((value & 0x7FLL) | 0x80LL);
    value = static_cast<int64_t>(static_cast<uint64_t>(value) >> 7);
  }
  bytes->bytes[bytes->length++] = static_cast<char>(value);
  return bytes;
}
} // namespace org::apache::lucene::index