using namespace std;

#include "TestDuelingCodecs.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Codec = org::apache::lucene::codecs::Codec;
using Document = org::apache::lucene::document::Document;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using SortedNumericDocValuesField =
    org::apache::lucene::document::SortedNumericDocValuesField;
using SortedSetDocValuesField =
    org::apache::lucene::document::SortedSetDocValuesField;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using IOUtils = org::apache::lucene::util::IOUtils;
using LineFileDocs = org::apache::lucene::util::LineFileDocs;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::apache::lucene::util::LuceneTestCase::Slow;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestDuelingCodecs::setUp() 
{
  LuceneTestCase::setUp();

  // for now it's SimpleText vs Default(random postings format)
  // as this gives the best overall coverage. when we have more
  // codecs we should probably pick 2 from Codec.availableCodecs()

  leftCodec = Codec::forName(L"SimpleText");
  rightCodec = make_shared<RandomCodec>(random());

  leftDir = newFSDirectory(createTempDir(L"leftDir"));
  rightDir = newFSDirectory(createTempDir(L"rightDir"));

  seed = random()->nextLong();

  // must use same seed because of random payloads, etc
  int maxTermLength =
      TestUtil::nextInt(random(), 1, IndexWriter::MAX_TERM_LENGTH);
  shared_ptr<MockAnalyzer> leftAnalyzer =
      make_shared<MockAnalyzer>(make_shared<Random>(seed));
  leftAnalyzer->setMaxTokenLength(maxTermLength);
  shared_ptr<MockAnalyzer> rightAnalyzer =
      make_shared<MockAnalyzer>(make_shared<Random>(seed));
  rightAnalyzer->setMaxTokenLength(maxTermLength);

  // but these can be different
  // TODO: this turns this into a really big test of Multi*, is that what we
  // want?
  shared_ptr<IndexWriterConfig> leftConfig = newIndexWriterConfig(leftAnalyzer);
  leftConfig->setCodec(leftCodec);
  // preserve docids
  leftConfig->setMergePolicy(newLogMergePolicy());

  shared_ptr<IndexWriterConfig> rightConfig =
      newIndexWriterConfig(rightAnalyzer);
  rightConfig->setCodec(rightCodec);
  // preserve docids
  rightConfig->setMergePolicy(newLogMergePolicy());

  // must use same seed because of random docvalues fields, etc
  leftWriter = make_shared<RandomIndexWriter>(make_shared<Random>(seed),
                                              leftDir, leftConfig);
  rightWriter = make_shared<RandomIndexWriter>(make_shared<Random>(seed),
                                               rightDir, rightConfig);

  // C++ TODO: There is no native C++ equivalent to 'toString':
  info = L"left: " + leftCodec->toString() + L" / right: " +
         rightCodec->toString();
}

void TestDuelingCodecs::tearDown() 
{
  IOUtils::close(
      {leftWriter, rightWriter, leftReader, rightReader, leftDir, rightDir});
  LuceneTestCase::tearDown();
}

void TestDuelingCodecs::createRandomIndex(int numdocs,
                                          shared_ptr<RandomIndexWriter> writer,
                                          int64_t seed) 
{
  shared_ptr<Random> random = make_shared<Random>(seed);
  // primary source for our data is from linefiledocs, it's realistic.
  shared_ptr<LineFileDocs> lineFileDocs = make_shared<LineFileDocs>(random);

  // TODO: we should add other fields that use things like docs&freqs but omit
  // positions, because linefiledocs doesn't cover all the possibilities.
  for (int i = 0; i < numdocs; i++) {
    shared_ptr<Document> document = lineFileDocs->nextDoc();
    // grab the title and add some SortedSet instances for fun
    wstring title = document[L"titleTokenized"];
    std::deque<wstring> split = title.split(L"\\s+");
    document->removeFields(L"sortedset");
    for (auto trash : split) {
      document->push_back(make_shared<SortedSetDocValuesField>(
          L"sortedset", make_shared<BytesRef>(trash)));
    }
    // add a numeric dv field sometimes
    document->removeFields(L"sparsenumeric");
    if (random->nextInt(4) == 2) {
      document->push_back(make_shared<NumericDocValuesField>(
          L"sparsenumeric", random->nextInt()));
    }
    // add sortednumeric sometimes
    document->removeFields(L"sparsesortednum");
    if (random->nextInt(5) == 1) {
      document->push_back(make_shared<SortedNumericDocValuesField>(
          L"sparsesortednum", random->nextLong()));
      if (random->nextBoolean()) {
        document->push_back(make_shared<SortedNumericDocValuesField>(
            L"sparsesortednum", random->nextLong()));
      }
    }
    writer->addDocument(document);
  }

  delete lineFileDocs;
}

void TestDuelingCodecs::testEquals() 
{
  int numdocs = atLeast(100);
  createRandomIndex(numdocs, leftWriter, seed);
  createRandomIndex(numdocs, rightWriter, seed);

  leftReader = leftWriter->getReader();
  rightReader = rightWriter->getReader();

  assertReaderEquals(info, leftReader, rightReader);
}

void TestDuelingCodecs::testCrazyReaderEquals() 
{
  int numdocs = atLeast(100);
  createRandomIndex(numdocs, leftWriter, seed);
  createRandomIndex(numdocs, rightWriter, seed);

  leftReader = wrapReader(leftWriter->getReader());
  rightReader = wrapReader(rightWriter->getReader());

  // check that our readers are valid
  TestUtil::checkReader(leftReader);
  TestUtil::checkReader(rightReader);

  assertReaderEquals(info, leftReader, rightReader);
}
} // namespace org::apache::lucene::index