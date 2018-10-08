using namespace std;

#include "TestSegmentMerger.h"

namespace org::apache::lucene::index
{
using Codec = org::apache::lucene::codecs::Codec;
using Document = org::apache::lucene::document::Document;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using MergeInfo = org::apache::lucene::store::MergeInfo;
using BytesRef = org::apache::lucene::util::BytesRef;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;
using InfoStream = org::apache::lucene::util::InfoStream;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using StringHelper = org::apache::lucene::util::StringHelper;
using TestUtil = org::apache::lucene::util::TestUtil;
using Version = org::apache::lucene::util::Version;
using PackedLongValues = org::apache::lucene::util::packed::PackedLongValues;

void TestSegmentMerger::setUp() 
{
  LuceneTestCase::setUp();
  mergedDir = newDirectory();
  merge1Dir = newDirectory();
  merge2Dir = newDirectory();
  DocHelper::setupDoc(doc1);
  shared_ptr<SegmentCommitInfo> info1 =
      DocHelper::writeDoc(random(), merge1Dir, doc1);
  DocHelper::setupDoc(doc2);
  shared_ptr<SegmentCommitInfo> info2 =
      DocHelper::writeDoc(random(), merge2Dir, doc2);
  reader1 = make_shared<SegmentReader>(info1, Version::LATEST->major,
                                       newIOContext(random()));
  reader2 = make_shared<SegmentReader>(info2, Version::LATEST->major,
                                       newIOContext(random()));
}

void TestSegmentMerger::tearDown() 
{
  delete reader1;
  delete reader2;
  delete mergedDir;
  delete merge1Dir;
  delete merge2Dir;
  LuceneTestCase::tearDown();
}

void TestSegmentMerger::test()
{
  assertTrue(mergedDir != nullptr);
  assertTrue(merge1Dir != nullptr);
  assertTrue(merge2Dir != nullptr);
  assertTrue(reader1 != nullptr);
  assertTrue(reader2 != nullptr);
}

void TestSegmentMerger::testMerge() 
{
  shared_ptr<Codec> *const codec = Codec::getDefault();
  shared_ptr<SegmentInfo> *const si = make_shared<SegmentInfo>(
      mergedDir, Version::LATEST, nullptr, mergedSegment, -1, false, codec,
      Collections::emptyMap(), StringHelper::randomId(), unordered_map<>(),
      nullptr);

  shared_ptr<SegmentMerger> merger = make_shared<SegmentMerger>(
      Arrays::asList<std::shared_ptr<CodecReader>>(reader1, reader2), si,
      InfoStream::getDefault(), mergedDir,
      make_shared<FieldInfos::FieldNumbers>(nullptr),
      newIOContext(random(), make_shared<IOContext>(
                                 make_shared<MergeInfo>(-1, -1, false, -1))));
  shared_ptr<MergeState> mergeState = merger->merge();
  int docsMerged = mergeState->segmentInfo->maxDoc();
  assertTrue(docsMerged == 2);
  // Should be able to open a new SegmentReader against the new directory
  shared_ptr<SegmentReader> mergedReader = make_shared<SegmentReader>(
      make_shared<SegmentCommitInfo>(mergeState->segmentInfo, 0, 0, -1LL, -1LL,
                                     -1LL),
      Version::LATEST->major, newIOContext(random()));
  assertTrue(mergedReader != nullptr);
  assertTrue(mergedReader->numDocs() == 2);
  shared_ptr<Document> newDoc1 = mergedReader->document(0);
  assertTrue(newDoc1->size() > 0);
  // There are 2 unstored fields on the document
  assertTrue(DocHelper::numFields(newDoc1) ==
             DocHelper::numFields(doc1) - DocHelper::unstored.size());
  shared_ptr<Document> newDoc2 = mergedReader->document(1);
  assertTrue(newDoc2->size() > 0);
  assertTrue(DocHelper::numFields(newDoc2) ==
             DocHelper::numFields(doc2) - DocHelper::unstored.size());

  shared_ptr<PostingsEnum> termDocs =
      TestUtil::docs(random(), mergedReader, DocHelper::TEXT_FIELD_2_KEY,
                     make_shared<BytesRef>(L"field"), nullptr, 0);
  assertTrue(termDocs != nullptr);
  assertTrue(termDocs->nextDoc() != DocIdSetIterator::NO_MORE_DOCS);

  int tvCount = 0;
  for (auto fieldInfo : mergedReader->getFieldInfos()) {
    if (fieldInfo->hasVectors()) {
      tvCount++;
    }
  }

  // System.out.println("stored size: " + stored.size());
  assertEquals(L"We do not have 3 fields that were indexed with term deque", 3,
               tvCount);

  shared_ptr<Terms> deque =
      mergedReader->getTermVectors(0)->terms(DocHelper::TEXT_FIELD_2_KEY);
  assertNotNull(deque);
  TestUtil::assertEquals(3, deque->size());
  shared_ptr<TermsEnum> termsEnum = deque->begin();

  int i = 0;
  while (termsEnum->next() != nullptr) {
    wstring term = termsEnum->term()->utf8ToString();
    int freq = static_cast<int>(termsEnum->totalTermFreq());
    // System.out.println("Term: " + term + " Freq: " + freq);
    assertTrue(DocHelper::FIELD_2_TEXT.find(term) != wstring::npos);
    assertTrue(DocHelper::FIELD_2_FREQS[i] == freq);
    i++;
  }

  TestSegmentReader::checkNorms(mergedReader);
  delete mergedReader;
}

void TestSegmentMerger::testBuildDocMap()
{
  constexpr int maxDoc = TestUtil::nextInt(random(), 1, 128);
  constexpr int numDocs = TestUtil::nextInt(random(), 0, maxDoc);
  shared_ptr<FixedBitSet> *const liveDocs = make_shared<FixedBitSet>(maxDoc);
  for (int i = 0; i < numDocs; ++i) {
    while (true) {
      constexpr int docID = random()->nextInt(maxDoc);
      if (!liveDocs->get(docID)) {
        liveDocs->set(docID);
        break;
      }
    }
  }

  shared_ptr<PackedLongValues> *const docMap =
      MergeState::removeDeletes(maxDoc, liveDocs);

  // assert the mapping is compact
  for (int i = 0, del = 0; i < maxDoc; ++i) {
    if (liveDocs->get(i) == false) {
      ++del;
    } else {
      TestUtil::assertEquals(i - del, docMap->get(i));
    }
  }
}
} // namespace org::apache::lucene::index