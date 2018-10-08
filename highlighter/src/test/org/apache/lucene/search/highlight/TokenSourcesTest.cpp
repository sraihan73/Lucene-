using namespace std;

#include "TokenSourcesTest.h"

namespace org::apache::lucene::search::highlight
{
using com::carrotsearch::randomizedtesting::annotations::Repeat;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using CannedTokenStream = org::apache::lucene::analysis::CannedTokenStream;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Token = org::apache::lucene::analysis::Token;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PayloadAttribute =
    org::apache::lucene::analysis::tokenattributes::PayloadAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using TextField = org::apache::lucene::document::TextField;
using BaseTermVectorsFormatTestCase =
    org::apache::lucene::index::BaseTermVectorsFormatTestCase;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using Fields = org::apache::lucene::index::Fields;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using DisjunctionMaxQuery = org::apache::lucene::search::DisjunctionMaxQuery;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using TopDocs = org::apache::lucene::search::TopDocs;
using SpanNearQuery = org::apache::lucene::search::spans::SpanNearQuery;
using SpanQuery = org::apache::lucene::search::spans::SpanQuery;
using SpanTermQuery = org::apache::lucene::search::spans::SpanTermQuery;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using TestUtil = org::apache::lucene::util::TestUtil;
const wstring TokenSourcesTest::FIELD = L"text";

bool TokenSourcesTest::OverlappingTokenStream::incrementToken()
{
  this->i++;
  if (this->i >= this->tokens.size()) {
    return false;
  }
  clearAttributes();
  termAttribute->setEmpty()->append(this->tokens[i]);
  offsetAttribute->setOffset(this->tokens[i]->startOffset(),
                             this->tokens[i]->endOffset());
  positionIncrementAttribute->setPositionIncrement(
      this->tokens[i]->getPositionIncrement());
  return true;
}

void TokenSourcesTest::OverlappingTokenStream::reset()
{
  this->i = -1;
  this->tokens = std::deque<std::shared_ptr<Token>>{
      make_shared<Token>(L"the", 0, 3),   make_shared<Token>(L"{fox}", 0, 7),
      make_shared<Token>(L"fox", 4, 7),   make_shared<Token>(L"did", 8, 11),
      make_shared<Token>(L"not", 12, 15), make_shared<Token>(L"jump", 16, 20)};
  this->tokens[1]->setPositionIncrement(0);
}

void TokenSourcesTest::testOverlapWithOffset() throw(
    IOException, InvalidTokenOffsetsException)
{
  const wstring TEXT = L"the fox did not jump";
  shared_ptr<Directory> *const directory = newDirectory();
  shared_ptr<IndexWriter> *const indexWriter =
      make_shared<IndexWriter>(directory, newIndexWriterConfig(nullptr));
  try {
    shared_ptr<Document> *const document = make_shared<Document>();
    shared_ptr<FieldType> customType =
        make_shared<FieldType>(TextField::TYPE_NOT_STORED);
    customType->setStoreTermVectors(true);
    // no positions!
    customType->setStoreTermVectorOffsets(true);
    document->push_back(make_shared<Field>(
        FIELD, make_shared<OverlappingTokenStream>(), customType));
    indexWriter->addDocument(document);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    delete indexWriter;
  }
  shared_ptr<IndexReader> *const indexReader = DirectoryReader::open(directory);
  TestUtil::assertEquals(1, indexReader->numDocs());
  shared_ptr<IndexSearcher> *const indexSearcher = newSearcher(indexReader);
  try {
    shared_ptr<DisjunctionMaxQuery> *const query =
        make_shared<DisjunctionMaxQuery>(
            Arrays::asList(
                make_shared<SpanTermQuery>(make_shared<Term>(FIELD, L"{fox}")),
                make_shared<SpanTermQuery>(make_shared<Term>(FIELD, L"fox"))),
            1);
    // final Query phraseQuery = new SpanNearQuery(new SpanQuery[] {
    // new SpanTermQuery(new Term(FIELD, "{fox}")),
    // new SpanTermQuery(new Term(FIELD, "fox")) }, 0, true);

    shared_ptr<TopDocs> hits = indexSearcher->search(query, 1);
    TestUtil::assertEquals(1, hits->totalHits);
    shared_ptr<Highlighter> *const highlighter = make_shared<Highlighter>(
        make_shared<SimpleHTMLFormatter>(), make_shared<SimpleHTMLEncoder>(),
        make_shared<QueryScorer>(query));
    shared_ptr<TokenStream> *const tokenStream =
        TokenSources::getTermVectorTokenStreamOrNull(
            FIELD, indexReader->getTermVectors(0), -1);
    TestUtil::assertEquals(L"<B>the fox</B> did not jump",
                           highlighter->getBestFragment(tokenStream, TEXT));
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    delete indexReader;
    delete directory;
  }
}

void TokenSourcesTest::testOverlapWithPositionsAndOffset() throw(
    IOException, InvalidTokenOffsetsException)
{
  const wstring TEXT = L"the fox did not jump";
  shared_ptr<Directory> *const directory = newDirectory();
  shared_ptr<IndexWriter> *const indexWriter =
      make_shared<IndexWriter>(directory, newIndexWriterConfig(nullptr));
  try {
    shared_ptr<Document> *const document = make_shared<Document>();
    shared_ptr<FieldType> customType =
        make_shared<FieldType>(TextField::TYPE_NOT_STORED);
    customType->setStoreTermVectors(true);
    customType->setStoreTermVectorOffsets(true);
    customType->setStoreTermVectorPositions(true);
    document->push_back(make_shared<Field>(
        FIELD, make_shared<OverlappingTokenStream>(), customType));
    indexWriter->addDocument(document);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    delete indexWriter;
  }
  shared_ptr<IndexReader> *const indexReader = DirectoryReader::open(directory);
  try {
    TestUtil::assertEquals(1, indexReader->numDocs());
    shared_ptr<IndexSearcher> *const indexSearcher = newSearcher(indexReader);
    shared_ptr<DisjunctionMaxQuery> *const query =
        make_shared<DisjunctionMaxQuery>(
            Arrays::asList(
                make_shared<SpanTermQuery>(make_shared<Term>(FIELD, L"{fox}")),
                make_shared<SpanTermQuery>(make_shared<Term>(FIELD, L"fox"))),
            1);
    // final Query phraseQuery = new SpanNearQuery(new SpanQuery[] {
    // new SpanTermQuery(new Term(FIELD, "{fox}")),
    // new SpanTermQuery(new Term(FIELD, "fox")) }, 0, true);

    shared_ptr<TopDocs> hits = indexSearcher->search(query, 1);
    TestUtil::assertEquals(1, hits->totalHits);
    shared_ptr<Highlighter> *const highlighter = make_shared<Highlighter>(
        make_shared<SimpleHTMLFormatter>(), make_shared<SimpleHTMLEncoder>(),
        make_shared<QueryScorer>(query));
    shared_ptr<TokenStream> *const tokenStream =
        TokenSources::getTermVectorTokenStreamOrNull(
            FIELD, indexReader->getTermVectors(0), -1);
    TestUtil::assertEquals(L"<B>the fox</B> did not jump",
                           highlighter->getBestFragment(tokenStream, TEXT));
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    delete indexReader;
    delete directory;
  }
}

void TokenSourcesTest::testOverlapWithOffsetExactPhrase() throw(
    IOException, InvalidTokenOffsetsException)
{
  const wstring TEXT = L"the fox did not jump";
  shared_ptr<Directory> *const directory = newDirectory();
  shared_ptr<IndexWriter> *const indexWriter =
      make_shared<IndexWriter>(directory, newIndexWriterConfig(nullptr));
  try {
    shared_ptr<Document> *const document = make_shared<Document>();
    shared_ptr<FieldType> customType =
        make_shared<FieldType>(TextField::TYPE_NOT_STORED);
    customType->setStoreTermVectors(true);
    // no positions!
    customType->setStoreTermVectorOffsets(true);
    document->push_back(make_shared<Field>(
        FIELD, make_shared<OverlappingTokenStream>(), customType));
    indexWriter->addDocument(document);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    delete indexWriter;
  }
  shared_ptr<IndexReader> *const indexReader = DirectoryReader::open(directory);
  try {
    TestUtil::assertEquals(1, indexReader->numDocs());
    shared_ptr<IndexSearcher> *const indexSearcher = newSearcher(indexReader);
    // final DisjunctionMaxQuery query = new DisjunctionMaxQuery(1);
    // query.add(new SpanTermQuery(new Term(FIELD, "{fox}")));
    // query.add(new SpanTermQuery(new Term(FIELD, "fox")));
    shared_ptr<Query> *const phraseQuery = make_shared<SpanNearQuery>(
        std::deque<std::shared_ptr<SpanQuery>>{
            make_shared<SpanTermQuery>(make_shared<Term>(FIELD, L"the")),
            make_shared<SpanTermQuery>(make_shared<Term>(FIELD, L"fox"))},
        0, true);

    shared_ptr<TopDocs> hits = indexSearcher->search(phraseQuery, 1);
    TestUtil::assertEquals(1, hits->totalHits);
    shared_ptr<Highlighter> *const highlighter = make_shared<Highlighter>(
        make_shared<SimpleHTMLFormatter>(), make_shared<SimpleHTMLEncoder>(),
        make_shared<QueryScorer>(phraseQuery));
    shared_ptr<TokenStream> *const tokenStream =
        TokenSources::getTermVectorTokenStreamOrNull(
            FIELD, indexReader->getTermVectors(0), -1);
    TestUtil::assertEquals(L"<B>the fox</B> did not jump",
                           highlighter->getBestFragment(tokenStream, TEXT));
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    delete indexReader;
    delete directory;
  }
}

void TokenSourcesTest::testOverlapWithPositionsAndOffsetExactPhrase() throw(
    IOException, InvalidTokenOffsetsException)
{
  const wstring TEXT = L"the fox did not jump";
  shared_ptr<Directory> *const directory = newDirectory();
  shared_ptr<IndexWriter> *const indexWriter =
      make_shared<IndexWriter>(directory, newIndexWriterConfig(nullptr));
  try {
    shared_ptr<Document> *const document = make_shared<Document>();
    shared_ptr<FieldType> customType =
        make_shared<FieldType>(TextField::TYPE_NOT_STORED);
    customType->setStoreTermVectors(true);
    customType->setStoreTermVectorPositions(true);
    customType->setStoreTermVectorOffsets(true);
    document->push_back(make_shared<Field>(
        FIELD, make_shared<OverlappingTokenStream>(), customType));
    indexWriter->addDocument(document);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    delete indexWriter;
  }
  shared_ptr<IndexReader> *const indexReader = DirectoryReader::open(directory);
  try {
    TestUtil::assertEquals(1, indexReader->numDocs());
    shared_ptr<IndexSearcher> *const indexSearcher = newSearcher(indexReader);
    // final DisjunctionMaxQuery query = new DisjunctionMaxQuery(1);
    // query.add(new SpanTermQuery(new Term(FIELD, "the")));
    // query.add(new SpanTermQuery(new Term(FIELD, "fox")));
    shared_ptr<Query> *const phraseQuery = make_shared<SpanNearQuery>(
        std::deque<std::shared_ptr<SpanQuery>>{
            make_shared<SpanTermQuery>(make_shared<Term>(FIELD, L"the")),
            make_shared<SpanTermQuery>(make_shared<Term>(FIELD, L"fox"))},
        0, true);

    shared_ptr<TopDocs> hits = indexSearcher->search(phraseQuery, 1);
    TestUtil::assertEquals(1, hits->totalHits);
    shared_ptr<Highlighter> *const highlighter = make_shared<Highlighter>(
        make_shared<SimpleHTMLFormatter>(), make_shared<SimpleHTMLEncoder>(),
        make_shared<QueryScorer>(phraseQuery));
    shared_ptr<TokenStream> *const tokenStream =
        TokenSources::getTermVectorTokenStreamOrNull(
            FIELD, indexReader->getTermVectors(0), -1);
    TestUtil::assertEquals(L"<B>the fox</B> did not jump",
                           highlighter->getBestFragment(tokenStream, TEXT));
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    delete indexReader;
    delete directory;
  }
}

void TokenSourcesTest::testTermVectorWithoutOffsetsDoesntWork() throw(
    IOException, InvalidTokenOffsetsException)
{
  shared_ptr<Directory> *const directory = newDirectory();
  shared_ptr<IndexWriter> *const indexWriter =
      make_shared<IndexWriter>(directory, newIndexWriterConfig(nullptr));
  try {
    shared_ptr<Document> *const document = make_shared<Document>();
    shared_ptr<FieldType> customType =
        make_shared<FieldType>(TextField::TYPE_NOT_STORED);
    customType->setStoreTermVectors(true);
    customType->setStoreTermVectorOffsets(false);
    customType->setStoreTermVectorPositions(true);
    document->push_back(make_shared<Field>(
        FIELD, make_shared<OverlappingTokenStream>(), customType));
    indexWriter->addDocument(document);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    delete indexWriter;
  }
  shared_ptr<IndexReader> *const indexReader = DirectoryReader::open(directory);
  try {
    TestUtil::assertEquals(1, indexReader->numDocs());
    shared_ptr<TokenStream> *const tokenStream =
        TokenSources::getTermVectorTokenStreamOrNull(
            FIELD, indexReader->getTermVectors(0), -1);
    assertNull(tokenStream);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    delete indexReader;
    delete directory;
  }
}

shared_ptr<Token> TokenSourcesTest::getToken(const wstring &text)
{
  shared_ptr<Token> t =
      make_shared<Token>(text, curOffset, curOffset + text.length());
  t->setPayload(make_shared<BytesRef>(text));
  curOffset++;
  return t;
}

void TokenSourcesTest::testPayloads() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<FieldType> myFieldType =
      make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  myFieldType->setStoreTermVectors(true);
  myFieldType->setStoreTermVectorOffsets(true);
  myFieldType->setStoreTermVectorPositions(true);
  myFieldType->setStoreTermVectorPayloads(true);

  curOffset = 0;

  std::deque<std::shared_ptr<Token>> tokens = {
      getToken(L"foxes"), getToken(L"can"), getToken(L"jump"),
      getToken(L"high")};

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<Field>(
      L"field", make_shared<CannedTokenStream>(tokens), myFieldType));
  writer->addDocument(doc);

  shared_ptr<IndexReader> reader = writer->getReader();
  delete writer;
  TestUtil::assertEquals(1, reader->numDocs());

  shared_ptr<TokenStream> ts = TokenSources::getTermVectorTokenStreamOrNull(
      L"field", reader->getTermVectors(0), -1);

  shared_ptr<CharTermAttribute> termAtt =
      ts->getAttribute(CharTermAttribute::typeid);
  shared_ptr<PositionIncrementAttribute> posIncAtt =
      ts->getAttribute(PositionIncrementAttribute::typeid);
  shared_ptr<OffsetAttribute> offsetAtt =
      ts->getAttribute(OffsetAttribute::typeid);
  shared_ptr<PayloadAttribute> payloadAtt =
      ts->addAttribute(PayloadAttribute::typeid);

  ts->reset();
  for (auto token : tokens) {
    assertTrue(ts->incrementToken());
    // C++ TODO: There is no native C++ equivalent to 'toString':
    TestUtil::assertEquals(token->toString(), termAtt->toString());
    TestUtil::assertEquals(token->getPositionIncrement(),
                           posIncAtt->getPositionIncrement());
    TestUtil::assertEquals(token->getPayload(), payloadAtt->getPayload());
    TestUtil::assertEquals(token->startOffset(), offsetAtt->startOffset());
    TestUtil::assertEquals(token->endOffset(), offsetAtt->endOffset());
  }

  assertFalse(ts->incrementToken());

  delete reader;
  delete dir;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Repeat(iterations = 10) public void testRandomizedRoundTrip()
// throws Exception
void TokenSourcesTest::testRandomizedRoundTrip() 
{
  constexpr int distinct = TestUtil::nextInt(random(), 1, 10);

  std::deque<wstring> terms(distinct);
  std::deque<std::shared_ptr<BytesRef>> termBytes(distinct);
  for (int i = 0; i < distinct; ++i) {
    terms[i] = TestUtil::randomRealisticUnicodeString(random());
    termBytes[i] = make_shared<BytesRef>(terms[i]);
  }

  shared_ptr<BaseTermVectorsFormatTestCase::RandomTokenStream>
      *const rTokenStream =
          make_shared<BaseTermVectorsFormatTestCase::RandomTokenStream>(
              TestUtil::nextInt(random(), 1, 10), terms, termBytes);
  // check to see if the token streams might have non-deterministic testable
  // result
  constexpr bool storeTermVectorPositions = random()->nextBoolean();
  const std::deque<int> startOffsets = rTokenStream->getStartOffsets();
  const std::deque<int> positionsIncrements =
      rTokenStream->getPositionsIncrements();
  for (int i = 1; i < positionsIncrements.size(); i++) {
    if (storeTermVectorPositions && positionsIncrements[i] != 0) {
      continue;
    }
    // TODO should RandomTokenStream ensure endOffsets for tokens at same
    // position and same startOffset are greater
    // than previous token's endOffset?  That would increase the testable
    // possibilities.
    if (startOffsets[i] == startOffsets[i - 1]) {
      if (VERBOSE) {
        wcout << L"Skipping test because can't easily validate random "
                 L"token-stream is correct."
              << endl;
      }
      return;
    }
  }

  // sanity check itself
  assertTokenStreamContents(
      rTokenStream, rTokenStream->getTerms(), rTokenStream->getStartOffsets(),
      rTokenStream->getEndOffsets(), rTokenStream->getPositionsIncrements());

  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<FieldType> myFieldType =
      make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  myFieldType->setStoreTermVectors(true);
  myFieldType->setStoreTermVectorOffsets(true);
  myFieldType->setStoreTermVectorPositions(storeTermVectorPositions);
  // payloads require positions; it will throw an error otherwise
  myFieldType->setStoreTermVectorPayloads(storeTermVectorPositions &&
                                          random()->nextBoolean());

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<Field>(L"field", rTokenStream, myFieldType));
  writer->addDocument(doc);

  shared_ptr<IndexReader> reader = writer->getReader();
  delete writer;
  TestUtil::assertEquals(1, reader->numDocs());

  shared_ptr<TokenStream> vectorTokenStream =
      TokenSources::getTermVectorTokenStreamOrNull(
          L"field", reader->getTermVectors(0), -1);

  // sometimes check payloads
  shared_ptr<PayloadAttribute> payloadAttribute = nullptr;
  if (myFieldType->storeTermVectorPayloads() && usually()) {
    payloadAttribute =
        vectorTokenStream->addAttribute(PayloadAttribute::typeid);
  }
  assertTokenStreamContents(vectorTokenStream, rTokenStream->getTerms(),
                            rTokenStream->getStartOffsets(),
                            rTokenStream->getEndOffsets(),
                            myFieldType->storeTermVectorPositions()
                                ? rTokenStream->getPositionsIncrements()
                                : nullptr);
  // test payloads
  if (payloadAttribute != nullptr) {
    vectorTokenStream->reset();
    for (int i = 0; vectorTokenStream->incrementToken(); i++) {
      TestUtil::assertEquals(rTokenStream->getPayloads()[i],
                             payloadAttribute->getPayload());
    }
  }

  delete reader;
  delete dir;
}

void TokenSourcesTest::testMaxStartOffsetConsistency() 
{
  shared_ptr<FieldType> tvFieldType =
      make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  tvFieldType->setStoreTermVectors(true);
  tvFieldType->setStoreTermVectorOffsets(true);
  tvFieldType->setStoreTermVectorPositions(true);

  shared_ptr<Directory> dir = newDirectory();

  shared_ptr<MockAnalyzer> analyzer = make_shared<MockAnalyzer>(random());
  analyzer->setEnableChecks(false); // we don't necessarily consume the whole
                                    // stream because of limiting by startOffset
  shared_ptr<Document> doc = make_shared<Document>();
  const wstring TEXT = L" f gg h";
  doc->push_back(make_shared<Field>(
      L"fld_tv", analyzer->tokenStream(L"fooFld", TEXT), tvFieldType));
  doc->push_back(make_shared<TextField>(
      L"fld_notv", analyzer->tokenStream(L"barFld", TEXT)));

  shared_ptr<IndexReader> reader;
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.index.RandomIndexWriter
  // writer = new org.apache.lucene.index.RandomIndexWriter(random(), dir))
  {
    org::apache::lucene::index::RandomIndexWriter writer =
        org::apache::lucene::index::RandomIndexWriter(random(), dir);
    writer->addDocument(doc);
    reader = writer->getReader();
  }
  try {
    shared_ptr<Fields> tvFields = reader->getTermVectors(0);
    for (int maxStartOffset = -1; maxStartOffset <= TEXT.length();
         maxStartOffset++) {
      shared_ptr<TokenStream> tvStream = TokenSources::getTokenStream(
          L"fld_tv", tvFields, TEXT, analyzer, maxStartOffset);
      shared_ptr<TokenStream> anaStream = TokenSources::getTokenStream(
          L"fld_notv", tvFields, TEXT, analyzer, maxStartOffset);

      // assert have same tokens, none of which has a start offset >
      // maxStartOffset
      shared_ptr<OffsetAttribute> *const tvOffAtt =
          tvStream->addAttribute(OffsetAttribute::typeid);
      shared_ptr<OffsetAttribute> *const anaOffAtt =
          anaStream->addAttribute(OffsetAttribute::typeid);
      tvStream->reset();
      anaStream->reset();
      while (tvStream->incrementToken()) {
        assertTrue(anaStream->incrementToken());
        TestUtil::assertEquals(tvOffAtt->startOffset(),
                               anaOffAtt->startOffset());
        if (maxStartOffset >= 0) {
          assertTrue(tvOffAtt->startOffset() <= maxStartOffset);
        }
      }
      assertTrue(anaStream->incrementToken() == false);
      tvStream->end();
      anaStream->end();
      delete tvStream;
      delete anaStream;
    }

  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    delete reader;
  }

  delete dir;
}
} // namespace org::apache::lucene::search::highlight