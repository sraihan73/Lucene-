using namespace std;

#include "TestBlockPostingsFormat3.h"

namespace org::apache::lucene::codecs::lucene50
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using MockFixedLengthPayloadFilter =
    org::apache::lucene::analysis::MockFixedLengthPayloadFilter;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using MockVariableLengthPayloadFilter =
    org::apache::lucene::analysis::MockVariableLengthPayloadFilter;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using TextField = org::apache::lucene::document::TextField;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using IndexOptions = org::apache::lucene::index::IndexOptions;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using OpenMode = org::apache::lucene::index::IndexWriterConfig::OpenMode;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using LeafReader = org::apache::lucene::index::LeafReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Terms = org::apache::lucene::index::Terms;
using SeekStatus = org::apache::lucene::index::TermsEnum::SeekStatus;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using Directory = org::apache::lucene::store::Directory;
using Bits = org::apache::lucene::util::Bits;
using BytesRef = org::apache::lucene::util::BytesRef;
using English = org::apache::lucene::util::English;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;
using AutomatonTestUtil =
    org::apache::lucene::util::automaton::AutomatonTestUtil;
using CompiledAutomaton =
    org::apache::lucene::util::automaton::CompiledAutomaton;
using RegExp = org::apache::lucene::util::automaton::RegExp;

void TestBlockPostingsFormat3::test() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Analyzer> analyzer = make_shared<AnalyzerAnonymousInnerClass>(
      shared_from_this(), Analyzer::PER_FIELD_REUSE_STRATEGY);
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig(analyzer);
  iwc->setCodec(
      TestUtil::alwaysPostingsFormat(make_shared<Lucene50PostingsFormat>()));
  // TODO we could actually add more fields implemented with different PFs
  // or, just put this test into the usual rotation?
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<FieldType> docsOnlyType =
      make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  // turn this on for a cross-check
  docsOnlyType->setStoreTermVectors(true);
  docsOnlyType->setIndexOptions(IndexOptions::DOCS);

  shared_ptr<FieldType> docsAndFreqsType =
      make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  // turn this on for a cross-check
  docsAndFreqsType->setStoreTermVectors(true);
  docsAndFreqsType->setIndexOptions(IndexOptions::DOCS_AND_FREQS);

  shared_ptr<FieldType> positionsType =
      make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  // turn these on for a cross-check
  positionsType->setStoreTermVectors(true);
  positionsType->setStoreTermVectorPositions(true);
  positionsType->setStoreTermVectorOffsets(true);
  positionsType->setStoreTermVectorPayloads(true);
  shared_ptr<FieldType> offsetsType = make_shared<FieldType>(positionsType);
  offsetsType->setIndexOptions(
      IndexOptions::DOCS_AND_FREQS_AND_POSITIONS_AND_OFFSETS);
  shared_ptr<Field> field1 =
      make_shared<Field>(L"field1docs", L"", docsOnlyType);
  shared_ptr<Field> field2 =
      make_shared<Field>(L"field2freqs", L"", docsAndFreqsType);
  shared_ptr<Field> field3 =
      make_shared<Field>(L"field3positions", L"", positionsType);
  shared_ptr<Field> field4 =
      make_shared<Field>(L"field4offsets", L"", offsetsType);
  shared_ptr<Field> field5 =
      make_shared<Field>(L"field5payloadsFixed", L"", positionsType);
  shared_ptr<Field> field6 =
      make_shared<Field>(L"field6payloadsVariable", L"", positionsType);
  shared_ptr<Field> field7 =
      make_shared<Field>(L"field7payloadsFixedOffsets", L"", offsetsType);
  shared_ptr<Field> field8 =
      make_shared<Field>(L"field8payloadsVariableOffsets", L"", offsetsType);
  doc->push_back(field1);
  doc->push_back(field2);
  doc->push_back(field3);
  doc->push_back(field4);
  doc->push_back(field5);
  doc->push_back(field6);
  doc->push_back(field7);
  doc->push_back(field8);
  for (int i = 0; i < MAXDOC; i++) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    wstring stringValue =
        Integer::toString(i) + L" verycommon " +
        StringHelper::replace(English::intToEnglish(i), L'-', L' ') + L" " +
        TestUtil::randomSimpleString(random());
    field1->setStringValue(stringValue);
    field2->setStringValue(stringValue);
    field3->setStringValue(stringValue);
    field4->setStringValue(stringValue);
    field5->setStringValue(stringValue);
    field6->setStringValue(stringValue);
    field7->setStringValue(stringValue);
    field8->setStringValue(stringValue);
    iw->addDocument(doc);
  }
  delete iw;
  verify(dir);
  TestUtil::checkIndex(
      dir); // for some extra coverage, checkIndex before we forceMerge
  iwc = newIndexWriterConfig(analyzer);
  iwc->setCodec(
      TestUtil::alwaysPostingsFormat(make_shared<Lucene50PostingsFormat>()));
  iwc->setOpenMode(OpenMode::APPEND);
  shared_ptr<IndexWriter> iw2 = make_shared<IndexWriter>(dir, iwc);
  iw2->forceMerge(1);
  delete iw2;
  verify(dir);
  delete dir;
}

TestBlockPostingsFormat3::AnalyzerAnonymousInnerClass::
    AnalyzerAnonymousInnerClass(
        shared_ptr<TestBlockPostingsFormat3> outerInstance,
        shared_ptr<UnknownType> PER_FIELD_REUSE_STRATEGY)
    : org::apache::lucene::analysis::Analyzer(PER_FIELD_REUSE_STRATEGY)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestBlockPostingsFormat3::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<MockTokenizer>();
  if (fieldName.find(L"payloadsFixed") != wstring::npos) {
    shared_ptr<TokenFilter> filter = make_shared<MockFixedLengthPayloadFilter>(
        make_shared<Random>(0), tokenizer, 1);
    return make_shared<Analyzer::TokenStreamComponents>(tokenizer, filter);
  } else if (fieldName.find(L"payloadsVariable") != wstring::npos) {
    shared_ptr<TokenFilter> filter =
        make_shared<MockVariableLengthPayloadFilter>(make_shared<Random>(0),
                                                     tokenizer);
    return make_shared<Analyzer::TokenStreamComponents>(tokenizer, filter);
  } else {
    return make_shared<Analyzer::TokenStreamComponents>(tokenizer);
  }
}

void TestBlockPostingsFormat3::verify(shared_ptr<Directory> dir) throw(
    runtime_error)
{
  shared_ptr<DirectoryReader> ir = DirectoryReader::open(dir);
  for (shared_ptr<LeafReaderContext> leaf : ir->leaves()) {
    shared_ptr<LeafReader> leafReader = leaf->reader();
    assertTerms(leafReader->terms(L"field1docs"),
                leafReader->terms(L"field2freqs"), true);
    assertTerms(leafReader->terms(L"field3positions"),
                leafReader->terms(L"field4offsets"), true);
    assertTerms(leafReader->terms(L"field4offsets"),
                leafReader->terms(L"field5payloadsFixed"), true);
    assertTerms(leafReader->terms(L"field5payloadsFixed"),
                leafReader->terms(L"field6payloadsVariable"), true);
    assertTerms(leafReader->terms(L"field6payloadsVariable"),
                leafReader->terms(L"field7payloadsFixedOffsets"), true);
    assertTerms(leafReader->terms(L"field7payloadsFixedOffsets"),
                leafReader->terms(L"field8payloadsVariableOffsets"), true);
  }
  ir->close();
}

void TestBlockPostingsFormat3::assertTerms(shared_ptr<Terms> leftTerms,
                                           shared_ptr<Terms> rightTerms,
                                           bool deep) 
{
  if (leftTerms == nullptr || rightTerms == nullptr) {
    assertNull(leftTerms);
    assertNull(rightTerms);
    return;
  }
  assertTermsStatistics(leftTerms, rightTerms);

  // NOTE: we don't assert hasOffsets/hasPositions/hasPayloads because they are
  // allowed to be different

  bool bothHavePositions =
      leftTerms->hasPositions() && rightTerms->hasPositions();
  shared_ptr<TermsEnum> leftTermsEnum = leftTerms->begin();
  shared_ptr<TermsEnum> rightTermsEnum = rightTerms->begin();
  assertTermsEnum(leftTermsEnum, rightTermsEnum, true, bothHavePositions);

  assertTermsSeeking(leftTerms, rightTerms);

  if (deep) {
    int numIntersections = atLeast(3);
    for (int i = 0; i < numIntersections; i++) {
      wstring re = AutomatonTestUtil::randomRegexp(random());
      shared_ptr<CompiledAutomaton> automaton = make_shared<CompiledAutomaton>(
          (make_shared<RegExp>(re, RegExp::NONE))->toAutomaton());
      if (automaton->type == CompiledAutomaton::AUTOMATON_TYPE::NORMAL) {
        // TODO: test start term too
        shared_ptr<TermsEnum> leftIntersection =
            leftTerms->intersect(automaton, nullptr);
        shared_ptr<TermsEnum> rightIntersection =
            rightTerms->intersect(automaton, nullptr);
        assertTermsEnum(leftIntersection, rightIntersection, rarely(),
                        bothHavePositions);
      }
    }
  }
}

void TestBlockPostingsFormat3::assertTermsSeeking(
    shared_ptr<Terms> leftTerms,
    shared_ptr<Terms> rightTerms) 
{
  shared_ptr<TermsEnum> leftEnum = nullptr;
  shared_ptr<TermsEnum> rightEnum = nullptr;

  // just an upper bound
  int numTests = atLeast(20);
  shared_ptr<Random> random = TestBlockPostingsFormat3::random();

  // collect this number of terms from the left side
  unordered_set<std::shared_ptr<BytesRef>> tests =
      unordered_set<std::shared_ptr<BytesRef>>();
  int numPasses = 0;
  while (numPasses < 10 && tests.size() < numTests) {
    leftEnum = leftTerms->begin();
    shared_ptr<BytesRef> term = nullptr;
    while ((term = leftEnum->next()) != nullptr) {
      int code = random->nextInt(10);
      if (code == 0) {
        // the term
        tests.insert(BytesRef::deepCopyOf(term));
      } else if (code == 1) {
        // truncated subsequence of term
        term = BytesRef::deepCopyOf(term);
        if (term->length > 0) {
          // truncate it
          term->length = random->nextInt(term->length);
        }
      } else if (code == 2) {
        // term, but ensure a non-zero offset
        std::deque<char> newbytes(term->length + 5);
        System::arraycopy(term->bytes, term->offset, newbytes, 5, term->length);
        tests.insert(make_shared<BytesRef>(newbytes, 5, term->length));
      }
    }
    numPasses++;
  }

  deque<std::shared_ptr<BytesRef>> shuffledTests =
      deque<std::shared_ptr<BytesRef>>(tests);
  Collections::shuffle(shuffledTests, random);

  for (auto b : shuffledTests) {
    leftEnum = leftTerms->begin();
    rightEnum = rightTerms->begin();

    TestUtil::assertEquals(leftEnum->seekExact(b), rightEnum->seekExact(b));
    TestUtil::assertEquals(leftEnum->seekExact(b), rightEnum->seekExact(b));

    SeekStatus leftStatus;
    SeekStatus rightStatus;

    leftStatus = leftEnum->seekCeil(b);
    rightStatus = rightEnum->seekCeil(b);
    TestUtil::assertEquals(leftStatus, rightStatus);
    if (leftStatus != SeekStatus::END) {
      TestUtil::assertEquals(leftEnum->term(), rightEnum->term());
    }

    leftStatus = leftEnum->seekCeil(b);
    rightStatus = rightEnum->seekCeil(b);
    TestUtil::assertEquals(leftStatus, rightStatus);
    if (leftStatus != SeekStatus::END) {
      TestUtil::assertEquals(leftEnum->term(), rightEnum->term());
    }
  }
}

void TestBlockPostingsFormat3::assertTermsStatistics(
    shared_ptr<Terms> leftTerms,
    shared_ptr<Terms> rightTerms) 
{
  if (leftTerms->getDocCount() != -1 && rightTerms->getDocCount() != -1) {
    TestUtil::assertEquals(leftTerms->getDocCount(), rightTerms->getDocCount());
  }
  if (leftTerms->getSumDocFreq() != -1 && rightTerms->getSumDocFreq() != -1) {
    TestUtil::assertEquals(leftTerms->getSumDocFreq(),
                           rightTerms->getSumDocFreq());
  }
  if (leftTerms->getSumTotalTermFreq() != -1 &&
      rightTerms->getSumTotalTermFreq() != -1) {
    TestUtil::assertEquals(leftTerms->getSumTotalTermFreq(),
                           rightTerms->getSumTotalTermFreq());
  }
  if (leftTerms->size() != -1 && rightTerms->size() != -1) {
    TestUtil::assertEquals(leftTerms->size(), rightTerms->size());
  }
}

void TestBlockPostingsFormat3::assertTermsEnum(
    shared_ptr<TermsEnum> leftTermsEnum, shared_ptr<TermsEnum> rightTermsEnum,
    bool deep, bool hasPositions) 
{
  shared_ptr<BytesRef> term;
  shared_ptr<PostingsEnum> leftPositions = nullptr;
  shared_ptr<PostingsEnum> rightPositions = nullptr;
  shared_ptr<PostingsEnum> leftDocs = nullptr;
  shared_ptr<PostingsEnum> rightDocs = nullptr;

  while ((term = leftTermsEnum->next()) != nullptr) {
    TestUtil::assertEquals(term, rightTermsEnum->next());
    assertTermStats(leftTermsEnum, rightTermsEnum);
    if (deep) {
      if (hasPositions) {
        // with payloads + off
        assertDocsAndPositionsEnum(leftPositions = leftTermsEnum->postings(
                                       leftPositions, PostingsEnum::ALL),
                                   rightPositions = rightTermsEnum->postings(
                                       rightPositions, PostingsEnum::ALL));

        assertPositionsSkipping(leftTermsEnum->docFreq(),
                                leftPositions = leftTermsEnum->postings(
                                    leftPositions, PostingsEnum::ALL),
                                rightPositions = rightTermsEnum->postings(
                                    rightPositions, PostingsEnum::ALL));
        // with payloads only
        assertDocsAndPositionsEnum(leftPositions = leftTermsEnum->postings(
                                       leftPositions, PostingsEnum::PAYLOADS),
                                   rightPositions = rightTermsEnum->postings(
                                       rightPositions, PostingsEnum::PAYLOADS));

        assertPositionsSkipping(leftTermsEnum->docFreq(),
                                leftPositions = leftTermsEnum->postings(
                                    leftPositions, PostingsEnum::PAYLOADS),
                                rightPositions = rightTermsEnum->postings(
                                    rightPositions, PostingsEnum::PAYLOADS));

        // with offsets only
        assertDocsAndPositionsEnum(leftPositions = leftTermsEnum->postings(
                                       leftPositions, PostingsEnum::OFFSETS),
                                   rightPositions = rightTermsEnum->postings(
                                       rightPositions, PostingsEnum::OFFSETS));

        assertPositionsSkipping(leftTermsEnum->docFreq(),
                                leftPositions = leftTermsEnum->postings(
                                    leftPositions, PostingsEnum::OFFSETS),
                                rightPositions = rightTermsEnum->postings(
                                    rightPositions, PostingsEnum::OFFSETS));

        // with positions only
        assertDocsAndPositionsEnum(
            leftPositions =
                leftTermsEnum->postings(leftPositions, PostingsEnum::POSITIONS),
            rightPositions = rightTermsEnum->postings(rightPositions,
                                                      PostingsEnum::POSITIONS));

        assertPositionsSkipping(leftTermsEnum->docFreq(),
                                leftPositions = leftTermsEnum->postings(
                                    leftPositions, PostingsEnum::POSITIONS),
                                rightPositions = rightTermsEnum->postings(
                                    rightPositions, PostingsEnum::POSITIONS));
      }

      // with freqs:
      assertDocsEnum(leftDocs = leftTermsEnum->postings(leftDocs),
                     rightDocs = rightTermsEnum->postings(rightDocs));

      // w/o freqs:
      assertDocsEnum(
          leftDocs = leftTermsEnum->postings(leftDocs, PostingsEnum::NONE),
          rightDocs = rightTermsEnum->postings(rightDocs, PostingsEnum::NONE));

      // with freqs:
      assertDocsSkipping(leftTermsEnum->docFreq(),
                         leftDocs = leftTermsEnum->postings(leftDocs),
                         rightDocs = rightTermsEnum->postings(rightDocs));

      // w/o freqs:
      assertDocsSkipping(
          leftTermsEnum->docFreq(),
          leftDocs = leftTermsEnum->postings(leftDocs, PostingsEnum::NONE),
          rightDocs = rightTermsEnum->postings(rightDocs, PostingsEnum::NONE));
    }
  }
  assertNull(rightTermsEnum->next());
}

void TestBlockPostingsFormat3::assertTermStats(
    shared_ptr<TermsEnum> leftTermsEnum,
    shared_ptr<TermsEnum> rightTermsEnum) 
{
  TestUtil::assertEquals(leftTermsEnum->docFreq(), rightTermsEnum->docFreq());
  if (leftTermsEnum->totalTermFreq() != -1 &&
      rightTermsEnum->totalTermFreq() != -1) {
    TestUtil::assertEquals(leftTermsEnum->totalTermFreq(),
                           rightTermsEnum->totalTermFreq());
  }
}

void TestBlockPostingsFormat3::assertDocsAndPositionsEnum(
    shared_ptr<PostingsEnum> leftDocs,
    shared_ptr<PostingsEnum> rightDocs) 
{
  assertNotNull(leftDocs);
  assertNotNull(rightDocs);
  TestUtil::assertEquals(-1, leftDocs->docID());
  TestUtil::assertEquals(-1, rightDocs->docID());
  int docid;
  while ((docid = leftDocs->nextDoc()) != DocIdSetIterator::NO_MORE_DOCS) {
    TestUtil::assertEquals(docid, rightDocs->nextDoc());
    int freq = leftDocs->freq();
    TestUtil::assertEquals(freq, rightDocs->freq());
    for (int i = 0; i < freq; i++) {
      TestUtil::assertEquals(leftDocs->nextPosition(),
                             rightDocs->nextPosition());
      // we don't assert offsets/payloads, they are allowed to be different
    }
  }
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS, rightDocs->nextDoc());
}

void TestBlockPostingsFormat3::assertDocsEnum(
    shared_ptr<PostingsEnum> leftDocs,
    shared_ptr<PostingsEnum> rightDocs) 
{
  if (leftDocs == nullptr) {
    assertNull(rightDocs);
    return;
  }
  TestUtil::assertEquals(-1, leftDocs->docID());
  TestUtil::assertEquals(-1, rightDocs->docID());
  int docid;
  while ((docid = leftDocs->nextDoc()) != DocIdSetIterator::NO_MORE_DOCS) {
    TestUtil::assertEquals(docid, rightDocs->nextDoc());
    // we don't assert freqs, they are allowed to be different
  }
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS, rightDocs->nextDoc());
}

void TestBlockPostingsFormat3::assertDocsSkipping(
    int docFreq, shared_ptr<PostingsEnum> leftDocs,
    shared_ptr<PostingsEnum> rightDocs) 
{
  if (leftDocs == nullptr) {
    assertNull(rightDocs);
    return;
  }
  int docid = -1;
  int averageGap = MAXDOC / (1 + docFreq);
  int skipInterval = 16;

  while (true) {
    if (random()->nextBoolean()) {
      // nextDoc()
      docid = leftDocs->nextDoc();
      TestUtil::assertEquals(docid, rightDocs->nextDoc());
    } else {
      // advance()
      int skip =
          docid + static_cast<int>(ceil(abs(
                      skipInterval + random()->nextGaussian() * averageGap)));
      docid = leftDocs->advance(skip);
      TestUtil::assertEquals(docid, rightDocs->advance(skip));
    }

    if (docid == DocIdSetIterator::NO_MORE_DOCS) {
      return;
    }
    // we don't assert freqs, they are allowed to be different
  }
}

void TestBlockPostingsFormat3::assertPositionsSkipping(
    int docFreq, shared_ptr<PostingsEnum> leftDocs,
    shared_ptr<PostingsEnum> rightDocs) 
{
  if (leftDocs == nullptr || rightDocs == nullptr) {
    assertNull(leftDocs);
    assertNull(rightDocs);
    return;
  }

  int docid = -1;
  int averageGap = MAXDOC / (1 + docFreq);
  int skipInterval = 16;

  while (true) {
    if (random()->nextBoolean()) {
      // nextDoc()
      docid = leftDocs->nextDoc();
      TestUtil::assertEquals(docid, rightDocs->nextDoc());
    } else {
      // advance()
      int skip =
          docid + static_cast<int>(ceil(abs(
                      skipInterval + random()->nextGaussian() * averageGap)));
      docid = leftDocs->advance(skip);
      TestUtil::assertEquals(docid, rightDocs->advance(skip));
    }

    if (docid == DocIdSetIterator::NO_MORE_DOCS) {
      return;
    }
    int freq = leftDocs->freq();
    TestUtil::assertEquals(freq, rightDocs->freq());
    for (int i = 0; i < freq; i++) {
      TestUtil::assertEquals(leftDocs->nextPosition(),
                             rightDocs->nextPosition());
      // we don't compare the payloads, it's allowed that one is empty etc
    }
  }
}

TestBlockPostingsFormat3::RandomBits::RandomBits(int maxDoc, double pctLive,
                                                 shared_ptr<Random> random)
{
  bits = make_shared<FixedBitSet>(maxDoc);
  for (int i = 0; i < maxDoc; i++) {
    if (random->nextDouble() <= pctLive) {
      bits->set(i);
    }
  }
}

bool TestBlockPostingsFormat3::RandomBits::get(int index)
{
  return bits->get(index);
}

int TestBlockPostingsFormat3::RandomBits::length() { return bits->length(); }
} // namespace org::apache::lucene::codecs::lucene50