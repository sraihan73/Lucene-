using namespace std;

#include "TestPostingsOffsets.h"

namespace org::apache::lucene::index
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using CannedTokenStream = org::apache::lucene::analysis::CannedTokenStream;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using MockPayloadAnalyzer = org::apache::lucene::analysis::MockPayloadAnalyzer;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using Token = org::apache::lucene::analysis::Token;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using StringField = org::apache::lucene::document::StringField;
using TextField = org::apache::lucene::document::TextField;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using English = org::apache::lucene::util::English;
using IOUtils = org::apache::lucene::util::IOUtils;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestPostingsOffsets::setUp() 
{
  LuceneTestCase::setUp();
  iwc = newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
}

void TestPostingsOffsets::testBasic() 
{
  shared_ptr<Directory> dir = newDirectory();

  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();

  shared_ptr<FieldType> ft = make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  ft->setIndexOptions(IndexOptions::DOCS_AND_FREQS_AND_POSITIONS_AND_OFFSETS);
  if (random()->nextBoolean()) {
    ft->setStoreTermVectors(true);
    ft->setStoreTermVectorPositions(random()->nextBoolean());
    ft->setStoreTermVectorOffsets(random()->nextBoolean());
  }
  std::deque<std::shared_ptr<Token>> tokens = {
      makeToken(L"a", 1, 0, 6), makeToken(L"b", 1, 8, 9),
      makeToken(L"a", 1, 9, 17), makeToken(L"c", 1, 19, 50)};
  doc->push_back(make_shared<Field>(
      L"content", make_shared<CannedTokenStream>(tokens), ft));

  w->addDocument(doc);
  shared_ptr<IndexReader> r = w->getReader();
  delete w;

  shared_ptr<PostingsEnum> dp = MultiFields::getTermPositionsEnum(
      r, L"content", make_shared<BytesRef>(L"a"));
  assertNotNull(dp);
  TestUtil::assertEquals(0, dp->nextDoc());
  TestUtil::assertEquals(2, dp->freq());
  TestUtil::assertEquals(0, dp->nextPosition());
  TestUtil::assertEquals(0, dp->startOffset());
  TestUtil::assertEquals(6, dp->endOffset());
  TestUtil::assertEquals(2, dp->nextPosition());
  TestUtil::assertEquals(9, dp->startOffset());
  TestUtil::assertEquals(17, dp->endOffset());
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS, dp->nextDoc());

  dp = MultiFields::getTermPositionsEnum(r, L"content",
                                         make_shared<BytesRef>(L"b"));
  assertNotNull(dp);
  TestUtil::assertEquals(0, dp->nextDoc());
  TestUtil::assertEquals(1, dp->freq());
  TestUtil::assertEquals(1, dp->nextPosition());
  TestUtil::assertEquals(8, dp->startOffset());
  TestUtil::assertEquals(9, dp->endOffset());
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS, dp->nextDoc());

  dp = MultiFields::getTermPositionsEnum(r, L"content",
                                         make_shared<BytesRef>(L"c"));
  assertNotNull(dp);
  TestUtil::assertEquals(0, dp->nextDoc());
  TestUtil::assertEquals(1, dp->freq());
  TestUtil::assertEquals(3, dp->nextPosition());
  TestUtil::assertEquals(19, dp->startOffset());
  TestUtil::assertEquals(50, dp->endOffset());
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS, dp->nextDoc());

  delete r;
  delete dir;
}

void TestPostingsOffsets::testSkipping() 
{
  doTestNumbers(false);
}

void TestPostingsOffsets::testPayloads() 
{
  doTestNumbers(true);
}

void TestPostingsOffsets::doTestNumbers(bool withPayloads) 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Analyzer> analyzer = withPayloads
                                      ? make_shared<MockPayloadAnalyzer>()
                                      : make_shared<MockAnalyzer>(random());
  iwc = newIndexWriterConfig(analyzer);
  iwc->setMergePolicy(
      newLogMergePolicy()); // will rely on docids a bit for skipping
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir, iwc);

  shared_ptr<FieldType> ft = make_shared<FieldType>(TextField::TYPE_STORED);
  ft->setIndexOptions(IndexOptions::DOCS_AND_FREQS_AND_POSITIONS_AND_OFFSETS);
  if (random()->nextBoolean()) {
    ft->setStoreTermVectors(true);
    ft->setStoreTermVectorOffsets(random()->nextBoolean());
    ft->setStoreTermVectorPositions(random()->nextBoolean());
  }

  int numDocs = atLeast(500);
  for (int i = 0; i < numDocs; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(
        make_shared<Field>(L"numbers", English::intToEnglish(i), ft));
    doc->push_back(
        make_shared<Field>(L"oddeven", (i % 2) == 0 ? L"even" : L"odd", ft));
    doc->push_back(
        make_shared<StringField>(L"id", L"" + to_wstring(i), Field::Store::NO));
    w->addDocument(doc);
  }

  shared_ptr<IndexReader> reader = w->getReader();
  delete w;

  std::deque<wstring> terms = {L"one",  L"two", L"three",  L"four",
                                L"five", L"six", L"seven",  L"eight",
                                L"nine", L"ten", L"hundred"};

  for (auto term : terms) {
    shared_ptr<PostingsEnum> dp = MultiFields::getTermPositionsEnum(
        reader, L"numbers", make_shared<BytesRef>(term));
    int doc;
    while ((doc = dp->nextDoc()) != DocIdSetIterator::NO_MORE_DOCS) {
      wstring storedNumbers = reader->document(doc)[L"numbers"];
      int freq = dp->freq();
      for (int i = 0; i < freq; i++) {
        dp->nextPosition();
        int start = dp->startOffset();
        assert(start >= 0);
        int end = dp->endOffset();
        assert(end >= 0 && end >= start);
        // check that the offsets correspond to the term in the src text
        assertTrue(storedNumbers.substr(start, end - start)->equals(term));
        if (withPayloads) {
          // check that we have a payload and it starts with "pos"
          assertNotNull(dp->getPayload());
          shared_ptr<BytesRef> payload = dp->getPayload();
          assertTrue(
              StringHelper::startsWith(payload->utf8ToString(), L"pos:"));
        } // note: withPayloads=false doesnt necessarily mean we dont have them
          // from MockAnalyzer!
      }
    }
  }

  // check we can skip correctly
  int numSkippingTests = atLeast(50);

  for (int j = 0; j < numSkippingTests; j++) {
    int num = TestUtil::nextInt(random(), 100, min(numDocs - 1, 999));
    shared_ptr<PostingsEnum> dp = MultiFields::getTermPositionsEnum(
        reader, L"numbers", make_shared<BytesRef>(L"hundred"));
    int doc = dp->advance(num);
    TestUtil::assertEquals(num, doc);
    int freq = dp->freq();
    for (int i = 0; i < freq; i++) {
      wstring storedNumbers = reader->document(doc)[L"numbers"];
      dp->nextPosition();
      int start = dp->startOffset();
      assert(start >= 0);
      int end = dp->endOffset();
      assert(end >= 0 && end >= start);
      // check that the offsets correspond to the term in the src text
      assertTrue(storedNumbers.substr(start, end - start)->equals(L"hundred"));
      if (withPayloads) {
        // check that we have a payload and it starts with "pos"
        assertNotNull(dp->getPayload());
        shared_ptr<BytesRef> payload = dp->getPayload();
        assertTrue(StringHelper::startsWith(payload->utf8ToString(), L"pos:"));
      } // note: withPayloads=false doesnt necessarily mean we dont have them
        // from MockAnalyzer!
    }
  }

  // check that other fields (without offsets) work correctly

  for (int i = 0; i < numDocs; i++) {
    shared_ptr<PostingsEnum> dp = MultiFields::getTermDocsEnum(
        reader, L"id", make_shared<BytesRef>(L"" + to_wstring(i)), 0);
    TestUtil::assertEquals(i, dp->nextDoc());
    TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS, dp->nextDoc());
  }

  delete reader;
  delete dir;
}

void TestPostingsOffsets::testRandom() 
{
  // token -> docID -> tokens
  const unordered_map<wstring,
                      unordered_map<int, deque<std::shared_ptr<Token>>>>
      actualTokens =
          unordered_map<wstring,
                        unordered_map<int, deque<std::shared_ptr<Token>>>>();

  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir, iwc);

  constexpr int numDocs = atLeast(20);
  // final int numDocs = atLeast(5);

  shared_ptr<FieldType> ft = make_shared<FieldType>(TextField::TYPE_NOT_STORED);

  // TODO: randomize what IndexOptions we use; also test
  // changing this up in one IW buffered segment...:
  ft->setIndexOptions(IndexOptions::DOCS_AND_FREQS_AND_POSITIONS_AND_OFFSETS);
  if (random()->nextBoolean()) {
    ft->setStoreTermVectors(true);
    ft->setStoreTermVectorOffsets(random()->nextBoolean());
    ft->setStoreTermVectorPositions(random()->nextBoolean());
  }

  for (int docCount = 0; docCount < numDocs; docCount++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(make_shared<NumericDocValuesField>(L"id", docCount));
    deque<std::shared_ptr<Token>> tokens = deque<std::shared_ptr<Token>>();
    constexpr int numTokens = atLeast(100);
    // final int numTokens = atLeast(20);
    int pos = -1;
    int offset = 0;
    // System.out.println("doc id=" + docCount);
    for (int tokenCount = 0; tokenCount < numTokens; tokenCount++) {
      const wstring text;
      if (random()->nextBoolean()) {
        text = L"a";
      } else if (random()->nextBoolean()) {
        text = L"b";
      } else if (random()->nextBoolean()) {
        text = L"c";
      } else {
        text = L"d";
      }

      int posIncr = random()->nextBoolean() ? 1 : random()->nextInt(5);
      if (tokenCount == 0 && posIncr == 0) {
        posIncr = 1;
      }
      constexpr int offIncr =
          random()->nextBoolean() ? 0 : random()->nextInt(5);
      constexpr int tokenOffset = random()->nextInt(5);

      shared_ptr<Token> *const token = makeToken(
          text, posIncr, offset + offIncr, offset + offIncr + tokenOffset);
      if (actualTokens.find(text) == actualTokens.end()) {
        actualTokens.emplace(
            text, unordered_map<int, deque<std::shared_ptr<Token>>>());
      }
      const unordered_map<int, deque<std::shared_ptr<Token>>> postingsByDoc =
          actualTokens[text];
      if (postingsByDoc.find(docCount) == postingsByDoc.end()) {
        postingsByDoc.emplace(docCount, deque<std::shared_ptr<Token>>());
      }
      postingsByDoc[docCount].push_back(token);
      tokens.push_back(token);
      pos += posIncr;
      // stuff abs position into type:
      token->setType(L"" + to_wstring(pos));
      offset += offIncr + tokenOffset;
      // System.out.println("  " + token + " posIncr=" +
      // token.getPositionIncrement() + " pos=" + pos + " off=" +
      // token.startOffset() + "/" + token.endOffset() + " (freq=" +
      // postingsByDoc.get(docCount).size() + ")");
    }
    doc->push_back(make_shared<Field>(
        L"content",
        make_shared<CannedTokenStream>(
            tokens.toArray(std::deque<std::shared_ptr<Token>>(tokens.size()))),
        ft));
    w->addDocument(doc);
  }
  shared_ptr<DirectoryReader> *const r = w->getReader();
  delete w;

  const std::deque<wstring> terms =
      std::deque<wstring>{L"a", L"b", L"c", L"d"};
  for (shared_ptr<LeafReaderContext> ctx : r->leaves()) {
    // TODO: improve this
    shared_ptr<LeafReader> sub = ctx->reader();
    // System.out.println("\nsub=" + sub);
    shared_ptr<TermsEnum> *const termsEnum = sub->terms(L"content")->begin();
    shared_ptr<PostingsEnum> docs = nullptr;
    shared_ptr<PostingsEnum> docsAndPositions = nullptr;
    shared_ptr<PostingsEnum> docsAndPositionsAndOffsets = nullptr;
    std::deque<int> docIDToID(sub->maxDoc());
    shared_ptr<NumericDocValues> values = DocValues::getNumeric(sub, L"id");
    for (int i = 0; i < sub->maxDoc(); i++) {
      TestUtil::assertEquals(i, values->nextDoc());
      docIDToID[i] = static_cast<int>(values->longValue());
    }

    for (auto term : terms) {
      // System.out.println("  term=" + term);
      if (termsEnum->seekExact(make_shared<BytesRef>(term))) {
        docs = termsEnum->postings(docs);
        assertNotNull(docs);
        int doc;
        // System.out.println("    doc/freq");
        while ((doc = docs->nextDoc()) != DocIdSetIterator::NO_MORE_DOCS) {
          const deque<std::shared_ptr<Token>> expected =
              actualTokens[term][docIDToID[doc]];
          // System.out.println("      doc=" + docIDToID[doc] + " docID=" + doc
          // + " " + expected.size() + " freq");
          assertNotNull(expected);
          TestUtil::assertEquals(expected.size(), docs->freq());
        }

        // explicitly exclude offsets here
        docsAndPositions =
            termsEnum->postings(docsAndPositions, PostingsEnum::ALL);
        assertNotNull(docsAndPositions);
        // System.out.println("    doc/freq/pos");
        while ((doc = docsAndPositions->nextDoc()) !=
               DocIdSetIterator::NO_MORE_DOCS) {
          const deque<std::shared_ptr<Token>> expected =
              actualTokens[term][docIDToID[doc]];
          // System.out.println("      doc=" + docIDToID[doc] + " " +
          // expected.size() + " freq");
          assertNotNull(expected);
          TestUtil::assertEquals(expected.size(), docsAndPositions->freq());
          for (auto token : expected) {
            int pos = stoi(token->type());
            // System.out.println("        pos=" + pos);
            TestUtil::assertEquals(pos, docsAndPositions->nextPosition());
          }
        }

        docsAndPositionsAndOffsets =
            termsEnum->postings(docsAndPositions, PostingsEnum::ALL);
        assertNotNull(docsAndPositionsAndOffsets);
        // System.out.println("    doc/freq/pos/offs");
        while ((doc = docsAndPositionsAndOffsets->nextDoc()) !=
               DocIdSetIterator::NO_MORE_DOCS) {
          const deque<std::shared_ptr<Token>> expected =
              actualTokens[term][docIDToID[doc]];
          // System.out.println("      doc=" + docIDToID[doc] + " " +
          // expected.size() + " freq");
          assertNotNull(expected);
          TestUtil::assertEquals(expected.size(),
                                 docsAndPositionsAndOffsets->freq());
          for (auto token : expected) {
            int pos = stoi(token->type());
            // System.out.println("        pos=" + pos);
            TestUtil::assertEquals(pos,
                                   docsAndPositionsAndOffsets->nextPosition());
            TestUtil::assertEquals(token->startOffset(),
                                   docsAndPositionsAndOffsets->startOffset());
            TestUtil::assertEquals(token->endOffset(),
                                   docsAndPositionsAndOffsets->endOffset());
          }
        }
      }
    }
    // TODO: test advance:
  }
  r->close();
  delete dir;
}

void TestPostingsOffsets::testWithUnindexedFields() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> riw =
      make_shared<RandomIndexWriter>(random(), dir, iwc);
  for (int i = 0; i < 100; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    // ensure at least one doc is indexed with offsets
    if (i < 99 && random()->nextInt(2) == 0) {
      // stored only
      shared_ptr<FieldType> ft = make_shared<FieldType>();
      ft->setStored(true);
      doc->push_back(make_shared<Field>(L"foo", L"boo!", ft));
    } else {
      shared_ptr<FieldType> ft = make_shared<FieldType>(TextField::TYPE_STORED);
      ft->setIndexOptions(
          IndexOptions::DOCS_AND_FREQS_AND_POSITIONS_AND_OFFSETS);
      if (random()->nextBoolean()) {
        // store some term vectors for the checkindex cross-check
        ft->setStoreTermVectors(true);
        ft->setStoreTermVectorPositions(true);
        ft->setStoreTermVectorOffsets(true);
      }
      doc->push_back(make_shared<Field>(L"foo", L"bar", ft));
    }
    riw->addDocument(doc);
  }
  shared_ptr<CompositeReader> ir = riw->getReader();
  shared_ptr<FieldInfos> fis = MultiFields::getMergedFieldInfos(ir);
  TestUtil::assertEquals(IndexOptions::DOCS_AND_FREQS_AND_POSITIONS_AND_OFFSETS,
                         fis->fieldInfo(L"foo")->getIndexOptions());
  delete ir;
  delete ir;
  delete riw;
  delete dir;
}

void TestPostingsOffsets::testAddFieldTwice() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<FieldType> customType3 =
      make_shared<FieldType>(TextField::TYPE_STORED);
  customType3->setStoreTermVectors(true);
  customType3->setStoreTermVectorPositions(true);
  customType3->setStoreTermVectorOffsets(true);
  customType3->setIndexOptions(
      IndexOptions::DOCS_AND_FREQS_AND_POSITIONS_AND_OFFSETS);
  doc->push_back(make_shared<Field>(
      L"content3", L"here is more content with aaa aaa aaa", customType3));
  doc->push_back(make_shared<Field>(
      L"content3", L"here is more content with aaa aaa aaa", customType3));
  iw->addDocument(doc);
  delete iw;
  delete dir; // checkindex
}

void TestPostingsOffsets::testNegativeOffsets() 
{
  expectThrows(invalid_argument::typeid, [&]() {
    checkTokens(
        std::deque<std::shared_ptr<Token>>{makeToken(L"foo", 1, -1, -1)});
  });
}

void TestPostingsOffsets::testIllegalOffsets() 
{
  expectThrows(invalid_argument::typeid, [&]() {
    checkTokens(
        std::deque<std::shared_ptr<Token>>{makeToken(L"foo", 1, 1, 0)});
  });
}

void TestPostingsOffsets::testIllegalOffsetsAcrossFieldInstances() throw(
    runtime_error)
{
  expectThrows(invalid_argument::typeid, [&]() {
    checkTokens(
        std::deque<std::shared_ptr<Token>>{makeToken(L"use", 1, 150, 160)},
        std::deque<std::shared_ptr<Token>>{makeToken(L"use", 1, 50, 60)});
  });
}

void TestPostingsOffsets::testBackwardsOffsets() 
{
  expectThrows(invalid_argument::typeid, [&]() {
    checkTokens(std::deque<std::shared_ptr<Token>>{
        makeToken(L"foo", 1, 0, 3), makeToken(L"foo", 1, 4, 7),
        makeToken(L"foo", 0, 3, 6)});
  });
}

void TestPostingsOffsets::testStackedTokens() 
{
  checkTokens(std::deque<std::shared_ptr<Token>>{makeToken(L"foo", 1, 0, 3),
                                                  makeToken(L"foo", 0, 0, 3),
                                                  makeToken(L"foo", 0, 0, 3)});
}

void TestPostingsOffsets::testCrazyOffsetGap() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Analyzer> analyzer =
      make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
  shared_ptr<IndexWriter> iw =
      make_shared<IndexWriter>(dir, make_shared<IndexWriterConfig>(analyzer));
  // add good document
  shared_ptr<Document> doc = make_shared<Document>();
  iw->addDocument(doc);
  expectThrows(invalid_argument::typeid, [&]() {
    shared_ptr<FieldType> ft =
        make_shared<FieldType>(TextField::TYPE_NOT_STORED);
    ft->setIndexOptions(IndexOptions::DOCS_AND_FREQS_AND_POSITIONS_AND_OFFSETS);
    doc->add(make_shared<Field>(L"foo", L"bar", ft));
    doc->add(make_shared<Field>(L"foo", L"bar", ft));
    iw->addDocument(doc);
  });
  iw->commit();
  delete iw;

  // make sure we see our good doc
  shared_ptr<DirectoryReader> r = DirectoryReader::open(dir);
  TestUtil::assertEquals(1, r->numDocs());
  r->close();
  delete dir;
}

TestPostingsOffsets::AnalyzerAnonymousInnerClass::AnalyzerAnonymousInnerClass(
    shared_ptr<TestPostingsOffsets> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestPostingsOffsets::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  return make_shared<Analyzer::TokenStreamComponents>(
      make_shared<MockTokenizer>(MockTokenizer::KEYWORD, false));
}

int TestPostingsOffsets::AnalyzerAnonymousInnerClass::getOffsetGap(
    const wstring &fieldName)
{
  return -10;
}

void TestPostingsOffsets::testLegalbutVeryLargeOffsets() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> iw =
      make_shared<IndexWriter>(dir, newIndexWriterConfig(nullptr));
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<Token> t1 =
      make_shared<Token>(L"foo", 0, numeric_limits<int>::max() - 500);
  if (random()->nextBoolean()) {
    t1->setPayload(make_shared<BytesRef>(L"test"));
  }
  shared_ptr<Token> t2 = make_shared<Token>(
      L"foo", numeric_limits<int>::max() - 500, numeric_limits<int>::max());
  shared_ptr<TokenStream> tokenStream = make_shared<CannedTokenStream>(
      std::deque<std::shared_ptr<Token>>{t1, t2});
  shared_ptr<FieldType> ft = make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  ft->setIndexOptions(IndexOptions::DOCS_AND_FREQS_AND_POSITIONS_AND_OFFSETS);
  // store some term vectors for the checkindex cross-check
  ft->setStoreTermVectors(true);
  ft->setStoreTermVectorPositions(true);
  ft->setStoreTermVectorOffsets(true);
  shared_ptr<Field> field = make_shared<Field>(L"foo", tokenStream, ft);
  doc->push_back(field);
  iw->addDocument(doc);
  delete iw;
  delete dir;
}

void TestPostingsOffsets::checkTokens(
    std::deque<std::shared_ptr<Token>> &field1,
    std::deque<std::shared_ptr<Token>> &field2) 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> riw =
      make_shared<RandomIndexWriter>(random(), dir, iwc);
  bool success = false;
  try {
    shared_ptr<FieldType> ft =
        make_shared<FieldType>(TextField::TYPE_NOT_STORED);
    ft->setIndexOptions(IndexOptions::DOCS_AND_FREQS_AND_POSITIONS_AND_OFFSETS);
    // store some term vectors for the checkindex cross-check
    ft->setStoreTermVectors(true);
    ft->setStoreTermVectorPositions(true);
    ft->setStoreTermVectorOffsets(true);

    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(make_shared<Field>(
        L"body", make_shared<CannedTokenStream>(field1), ft));
    doc->push_back(make_shared<Field>(
        L"body", make_shared<CannedTokenStream>(field2), ft));
    riw->addDocument(doc);
    delete riw;
    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (success) {
      IOUtils::close({dir});
    } else {
      IOUtils::closeWhileHandlingException({riw, dir});
    }
  }
}

void TestPostingsOffsets::checkTokens(
    std::deque<std::shared_ptr<Token>> &tokens) 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> riw =
      make_shared<RandomIndexWriter>(random(), dir, iwc);
  bool success = false;
  try {
    shared_ptr<FieldType> ft =
        make_shared<FieldType>(TextField::TYPE_NOT_STORED);
    ft->setIndexOptions(IndexOptions::DOCS_AND_FREQS_AND_POSITIONS_AND_OFFSETS);
    // store some term vectors for the checkindex cross-check
    ft->setStoreTermVectors(true);
    ft->setStoreTermVectorPositions(true);
    ft->setStoreTermVectorOffsets(true);

    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(make_shared<Field>(
        L"body", make_shared<CannedTokenStream>(tokens), ft));
    riw->addDocument(doc);
    delete riw;
    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (success) {
      IOUtils::close({dir});
    } else {
      IOUtils::closeWhileHandlingException({riw, dir});
    }
  }
}

shared_ptr<Token> TestPostingsOffsets::makeToken(const wstring &text,
                                                 int posIncr, int startOffset,
                                                 int endOffset)
{
  shared_ptr<Token> *const t = make_shared<Token>();
  t->append(text);
  t->setPositionIncrement(posIncr);
  t->setOffset(startOffset, endOffset);
  return t;
}
} // namespace org::apache::lucene::index