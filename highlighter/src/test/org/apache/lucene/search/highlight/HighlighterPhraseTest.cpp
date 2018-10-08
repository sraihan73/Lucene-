using namespace std;

#include "HighlighterPhraseTest.h"

namespace org::apache::lucene::search::highlight
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using MockTokenFilter = org::apache::lucene::analysis::MockTokenFilter;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using Token = org::apache::lucene::analysis::Token;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using Store = org::apache::lucene::document::Field::Store;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using TextField = org::apache::lucene::document::TextField;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Term = org::apache::lucene::index::Term;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using PhraseQuery = org::apache::lucene::search::PhraseQuery;
using Query = org::apache::lucene::search::Query;
using SimpleCollector = org::apache::lucene::search::SimpleCollector;
using TopDocs = org::apache::lucene::search::TopDocs;
using SpanNearQuery = org::apache::lucene::search::spans::SpanNearQuery;
using SpanQuery = org::apache::lucene::search::spans::SpanQuery;
using SpanTermQuery = org::apache::lucene::search::spans::SpanTermQuery;
using Directory = org::apache::lucene::store::Directory;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
const wstring HighlighterPhraseTest::FIELD = L"text";

void HighlighterPhraseTest::testConcurrentPhrase() throw(
    IOException, InvalidTokenOffsetsException)
{
  const wstring TEXT = L"the fox jumped";
  shared_ptr<Directory> *const directory = newDirectory();
  shared_ptr<IndexWriter> *const indexWriter = make_shared<IndexWriter>(
      directory, newIndexWriterConfig(make_shared<MockAnalyzer>(
                     random(), MockTokenizer::WHITESPACE, false)));
  try {
    shared_ptr<Document> *const document = make_shared<Document>();
    shared_ptr<FieldType> customType =
        make_shared<FieldType>(TextField::TYPE_NOT_STORED);
    customType->setStoreTermVectorOffsets(true);
    customType->setStoreTermVectorPositions(true);
    customType->setStoreTermVectors(true);
    document->push_back(make_shared<Field>(
        FIELD, make_shared<TokenStreamConcurrent>(), customType));
    indexWriter->addDocument(document);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    delete indexWriter;
  }
  shared_ptr<IndexReader> *const indexReader = DirectoryReader::open(directory);
  try {
    assertEquals(1, indexReader->numDocs());
    shared_ptr<IndexSearcher> *const indexSearcher = newSearcher(indexReader);
    shared_ptr<PhraseQuery> *const phraseQuery =
        make_shared<PhraseQuery>(FIELD, L"fox", L"jumped");
    shared_ptr<TopDocs> hits = indexSearcher->search(phraseQuery, 1);
    assertEquals(1, hits->totalHits);
    shared_ptr<Highlighter> *const highlighter = make_shared<Highlighter>(
        make_shared<SimpleHTMLFormatter>(), make_shared<SimpleHTMLEncoder>(),
        make_shared<QueryScorer>(phraseQuery));

    shared_ptr<TokenStream> *const tokenStream =
        TokenSources::getTermVectorTokenStreamOrNull(
            FIELD, indexReader->getTermVectors(0), -1);
    assertEquals(highlighter->getBestFragment(
                     make_shared<TokenStreamConcurrent>(), TEXT),
                 highlighter->getBestFragment(tokenStream, TEXT));
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    delete indexReader;
    delete directory;
  }
}

void HighlighterPhraseTest::testConcurrentSpan() throw(
    IOException, InvalidTokenOffsetsException)
{
  const wstring TEXT = L"the fox jumped";
  shared_ptr<Directory> *const directory = newDirectory();
  shared_ptr<IndexWriter> *const indexWriter = make_shared<IndexWriter>(
      directory, newIndexWriterConfig(make_shared<MockAnalyzer>(
                     random(), MockTokenizer::WHITESPACE, false)));
  try {
    shared_ptr<Document> *const document = make_shared<Document>();

    shared_ptr<FieldType> customType =
        make_shared<FieldType>(TextField::TYPE_NOT_STORED);
    customType->setStoreTermVectorOffsets(true);
    customType->setStoreTermVectorPositions(true);
    customType->setStoreTermVectors(true);
    document->push_back(make_shared<Field>(
        FIELD, make_shared<TokenStreamConcurrent>(), customType));
    indexWriter->addDocument(document);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    delete indexWriter;
  }
  shared_ptr<IndexReader> *const indexReader = DirectoryReader::open(directory);
  try {
    assertEquals(1, indexReader->numDocs());
    shared_ptr<IndexSearcher> *const indexSearcher = newSearcher(indexReader);
    shared_ptr<Query> *const phraseQuery = make_shared<SpanNearQuery>(
        std::deque<std::shared_ptr<SpanQuery>>{
            make_shared<SpanTermQuery>(make_shared<Term>(FIELD, L"fox")),
            make_shared<SpanTermQuery>(make_shared<Term>(FIELD, L"jumped"))},
        0, true);
    shared_ptr<FixedBitSet> *const bitset =
        make_shared<FixedBitSet>(indexReader->maxDoc());
    indexSearcher->search(phraseQuery,
                          make_shared<SimpleCollectorAnonymousInnerClass>(
                              shared_from_this(), bitset));
    assertEquals(1, bitset->cardinality());
    constexpr int maxDoc = indexReader->maxDoc();
    shared_ptr<Highlighter> *const highlighter = make_shared<Highlighter>(
        make_shared<SimpleHTMLFormatter>(), make_shared<SimpleHTMLEncoder>(),
        make_shared<QueryScorer>(phraseQuery));
    for (int position = bitset->nextSetBit(0); position < maxDoc - 1;
         position = bitset->nextSetBit(position + 1)) {
      assertEquals(0, position);
      shared_ptr<TokenStream> *const tokenStream =
          TokenSources::getTermVectorTokenStreamOrNull(
              FIELD, indexReader->getTermVectors(position), -1);
      assertEquals(highlighter->getBestFragment(
                       make_shared<TokenStreamConcurrent>(), TEXT),
                   highlighter->getBestFragment(tokenStream, TEXT));
    }
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    delete indexReader;
    delete directory;
  }
}

HighlighterPhraseTest::SimpleCollectorAnonymousInnerClass::
    SimpleCollectorAnonymousInnerClass(
        shared_ptr<HighlighterPhraseTest> outerInstance,
        shared_ptr<FixedBitSet> bitset)
{
  this->outerInstance = outerInstance;
  this->bitset = bitset;
}

void HighlighterPhraseTest::SimpleCollectorAnonymousInnerClass::collect(int i)
{
  bitset->set(this->baseDoc + i);
}

void HighlighterPhraseTest::SimpleCollectorAnonymousInnerClass::doSetNextReader(
    shared_ptr<LeafReaderContext> context) 
{
  this->baseDoc = context->docBase;
}

void HighlighterPhraseTest::SimpleCollectorAnonymousInnerClass::setScorer(
    shared_ptr<org::apache::lucene::search::Scorer> scorer)
{
  // Do Nothing
}

bool HighlighterPhraseTest::SimpleCollectorAnonymousInnerClass::needsScores()
{
  return false;
}

void HighlighterPhraseTest::testSparsePhrase() throw(
    IOException, InvalidTokenOffsetsException)
{
  const wstring TEXT = L"the fox did not jump";
  shared_ptr<Directory> *const directory = newDirectory();
  shared_ptr<IndexWriter> *const indexWriter = make_shared<IndexWriter>(
      directory, newIndexWriterConfig(make_shared<MockAnalyzer>(
                     random(), MockTokenizer::WHITESPACE, false)));
  try {
    shared_ptr<Document> *const document = make_shared<Document>();

    shared_ptr<FieldType> customType =
        make_shared<FieldType>(TextField::TYPE_NOT_STORED);
    customType->setStoreTermVectorOffsets(true);
    customType->setStoreTermVectorPositions(true);
    customType->setStoreTermVectors(true);
    document->push_back(make_shared<Field>(
        FIELD, make_shared<TokenStreamSparse>(), customType));
    indexWriter->addDocument(document);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    delete indexWriter;
  }
  shared_ptr<IndexReader> *const indexReader = DirectoryReader::open(directory);
  try {
    assertEquals(1, indexReader->numDocs());
    shared_ptr<IndexSearcher> *const indexSearcher = newSearcher(indexReader);
    shared_ptr<PhraseQuery> *const phraseQuery =
        make_shared<PhraseQuery>(FIELD, L"did", L"jump");
    shared_ptr<TopDocs> hits = indexSearcher->search(phraseQuery, 1);
    assertEquals(0, hits->totalHits);
    shared_ptr<Highlighter> *const highlighter = make_shared<Highlighter>(
        make_shared<SimpleHTMLFormatter>(), make_shared<SimpleHTMLEncoder>(),
        make_shared<QueryScorer>(phraseQuery));
    shared_ptr<TokenStream> *const tokenStream =
        TokenSources::getTermVectorTokenStreamOrNull(
            FIELD, indexReader->getTermVectors(0), -1);
    assertEquals(
        highlighter->getBestFragment(make_shared<TokenStreamSparse>(), TEXT),
        highlighter->getBestFragment(tokenStream, TEXT));
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    delete indexReader;
    delete directory;
  }
}

void HighlighterPhraseTest::testSparsePhraseWithNoPositions() throw(
    IOException, InvalidTokenOffsetsException)
{
  const wstring TEXT = L"the fox did not jump";
  shared_ptr<Directory> *const directory = newDirectory();
  shared_ptr<IndexWriter> *const indexWriter = make_shared<IndexWriter>(
      directory, newIndexWriterConfig(make_shared<MockAnalyzer>(
                     random(), MockTokenizer::WHITESPACE, false)));
  try {
    shared_ptr<Document> *const document = make_shared<Document>();

    shared_ptr<FieldType> customType =
        make_shared<FieldType>(TextField::TYPE_STORED);
    customType->setStoreTermVectorOffsets(true);
    customType->setStoreTermVectors(true);
    document->push_back(make_shared<Field>(FIELD, TEXT, customType));
    indexWriter->addDocument(document);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    delete indexWriter;
  }
  shared_ptr<IndexReader> *const indexReader = DirectoryReader::open(directory);
  try {
    assertEquals(1, indexReader->numDocs());
    shared_ptr<IndexSearcher> *const indexSearcher = newSearcher(indexReader);
    shared_ptr<PhraseQuery> *const phraseQuery =
        make_shared<PhraseQuery>(1, FIELD, L"did", L"jump");
    shared_ptr<TopDocs> hits = indexSearcher->search(phraseQuery, 1);
    assertEquals(1, hits->totalHits);
    shared_ptr<Highlighter> *const highlighter = make_shared<Highlighter>(
        make_shared<SimpleHTMLFormatter>(), make_shared<SimpleHTMLEncoder>(),
        make_shared<QueryScorer>(phraseQuery));
    shared_ptr<TokenStream> *const tokenStream =
        TokenSources::getTermVectorTokenStreamOrNull(
            FIELD, indexReader->getTermVectors(0), -1);
    assertEquals(L"the fox <B>did</B> not <B>jump</B>",
                 highlighter->getBestFragment(tokenStream, TEXT));
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    delete indexReader;
    delete directory;
  }
}

void HighlighterPhraseTest::testSparseSpan() throw(IOException,
                                                   InvalidTokenOffsetsException)
{
  const wstring TEXT = L"the fox did not jump";
  shared_ptr<Directory> *const directory = newDirectory();
  shared_ptr<IndexWriter> *const indexWriter = make_shared<IndexWriter>(
      directory, newIndexWriterConfig(make_shared<MockAnalyzer>(
                     random(), MockTokenizer::WHITESPACE, false)));
  try {
    shared_ptr<Document> *const document = make_shared<Document>();
    shared_ptr<FieldType> customType =
        make_shared<FieldType>(TextField::TYPE_NOT_STORED);
    customType->setStoreTermVectorOffsets(true);
    customType->setStoreTermVectorPositions(true);
    customType->setStoreTermVectors(true);
    document->push_back(make_shared<Field>(
        FIELD, make_shared<TokenStreamSparse>(), customType));
    indexWriter->addDocument(document);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    delete indexWriter;
  }
  shared_ptr<IndexReader> *const indexReader = DirectoryReader::open(directory);
  try {
    assertEquals(1, indexReader->numDocs());
    shared_ptr<IndexSearcher> *const indexSearcher = newSearcher(indexReader);
    shared_ptr<Query> *const phraseQuery = make_shared<SpanNearQuery>(
        std::deque<std::shared_ptr<SpanQuery>>{
            make_shared<SpanTermQuery>(make_shared<Term>(FIELD, L"did")),
            make_shared<SpanTermQuery>(make_shared<Term>(FIELD, L"jump"))},
        0, true);

    shared_ptr<TopDocs> hits = indexSearcher->search(phraseQuery, 1);
    assertEquals(0, hits->totalHits);
    shared_ptr<Highlighter> *const highlighter = make_shared<Highlighter>(
        make_shared<SimpleHTMLFormatter>(), make_shared<SimpleHTMLEncoder>(),
        make_shared<QueryScorer>(phraseQuery));
    shared_ptr<TokenStream> *const tokenStream =
        TokenSources::getTermVectorTokenStreamOrNull(
            FIELD, indexReader->getTermVectors(0), -1);
    assertEquals(
        highlighter->getBestFragment(make_shared<TokenStreamSparse>(), TEXT),
        highlighter->getBestFragment(tokenStream, TEXT));
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    delete indexReader;
    delete directory;
  }
}

void HighlighterPhraseTest::testStopWords() throw(IOException,
                                                  InvalidTokenOffsetsException)
{
  shared_ptr<MockAnalyzer> stopAnalyzer =
      make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, true,
                                MockTokenFilter::ENGLISH_STOPSET);
  const wstring TEXT = L"the ab the the cd the the the ef the";
  shared_ptr<Directory> *const directory = newDirectory();
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.index.IndexWriter
  // indexWriter = new org.apache.lucene.index.IndexWriter(directory,
  // newIndexWriterConfig(stopAnalyzer)))
  {
    org::apache::lucene::index::IndexWriter indexWriter =
        org::apache::lucene::index::IndexWriter(
            directory, newIndexWriterConfig(stopAnalyzer));
    shared_ptr<Document> *const document = make_shared<Document>();
    document->push_back(newTextField(FIELD, TEXT, Store::YES));
    indexWriter->addDocument(document);
  }
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.index.IndexReader
  // indexReader = org.apache.lucene.index.DirectoryReader.open(directory))
  {
    org::apache::lucene::index::IndexReader indexReader =
        org::apache::lucene::index::DirectoryReader::open(directory);
    try {
      assertEquals(1, indexReader->numDocs());
      shared_ptr<IndexSearcher> *const indexSearcher = newSearcher(indexReader);
      // equivalent of "ab the the cd the the the ef"
      shared_ptr<PhraseQuery> *const phraseQuery =
          (make_shared<PhraseQuery::Builder>())
              ->add(make_shared<Term>(FIELD, L"ab"), 0)
              ->add(make_shared<Term>(FIELD, L"cd"), 3)
              ->add(make_shared<Term>(FIELD, L"ef"), 7)
              ->build();

      shared_ptr<TopDocs> hits = indexSearcher->search(phraseQuery, 100);
      assertEquals(1, hits->totalHits);
      shared_ptr<Highlighter> *const highlighter = make_shared<Highlighter>(
          make_shared<SimpleHTMLFormatter>(), make_shared<SimpleHTMLEncoder>(),
          make_shared<QueryScorer>(phraseQuery));
      assertEquals(
          1,
          highlighter->getBestFragments(stopAnalyzer, FIELD, TEXT, 10).size());
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      delete directory;
    }
  }
}

void HighlighterPhraseTest::testInOrderWithStopWords() throw(
    IOException, InvalidTokenOffsetsException)
{
  shared_ptr<MockAnalyzer> stopAnalyzer =
      make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, true,
                                MockTokenFilter::ENGLISH_STOPSET);
  const wstring TEXT =
      L"the cd the ab the the the the the the the ab the cd the";
  shared_ptr<Directory> *const directory = newDirectory();
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.index.IndexWriter
  // indexWriter = new org.apache.lucene.index.IndexWriter(directory,
  // newIndexWriterConfig(stopAnalyzer)))
  {
    org::apache::lucene::index::IndexWriter indexWriter =
        org::apache::lucene::index::IndexWriter(
            directory, newIndexWriterConfig(stopAnalyzer));
    shared_ptr<Document> *const document = make_shared<Document>();
    document->push_back(newTextField(FIELD, TEXT, Store::YES));
    indexWriter->addDocument(document);
  }
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.index.IndexReader
  // indexReader = org.apache.lucene.index.DirectoryReader.open(directory))
  {
    org::apache::lucene::index::IndexReader indexReader =
        org::apache::lucene::index::DirectoryReader::open(directory);
    try {
      assertEquals(1, indexReader->numDocs());
      shared_ptr<IndexSearcher> *const indexSearcher = newSearcher(indexReader);
      // equivalent of "ab the cd"
      shared_ptr<PhraseQuery> *const phraseQuery =
          (make_shared<PhraseQuery::Builder>())
              ->add(make_shared<Term>(FIELD, L"ab"), 0)
              ->add(make_shared<Term>(FIELD, L"cd"), 2)
              ->build();

      shared_ptr<TopDocs> hits = indexSearcher->search(phraseQuery, 100);
      assertEquals(1, hits->totalHits);

      shared_ptr<Highlighter> *const highlighter = make_shared<Highlighter>(
          make_shared<SimpleHTMLFormatter>(), make_shared<SimpleHTMLEncoder>(),
          make_shared<QueryScorer>(phraseQuery));
      std::deque<wstring> frags =
          highlighter->getBestFragments(stopAnalyzer, FIELD, TEXT, 10);
      assertEquals(1, frags.size());
      assertTrue(L"contains <B>ab</B> the <B>cd</B>",
                 (frags[0].find(L"<B>ab</B> the <B>cd</B>") != wstring::npos));
      assertTrue(L"does not contain <B>cd</B> the <B>ab</B>",
                 (!frags[0].find(L"<B>cd</B> the <B>ab</B>") != wstring::npos));
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      delete directory;
    }
  }
}

HighlighterPhraseTest::TokenStreamSparse::TokenStreamSparse() { reset(); }

bool HighlighterPhraseTest::TokenStreamSparse::incrementToken()
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

void HighlighterPhraseTest::TokenStreamSparse::reset()
{
  this->i = -1;
  this->tokens = std::deque<std::shared_ptr<Token>>{
      make_shared<Token>(L"the", 0, 3), make_shared<Token>(L"fox", 4, 7),
      make_shared<Token>(L"did", 8, 11), make_shared<Token>(L"jump", 16, 20)};
  this->tokens[3]->setPositionIncrement(2);
}

HighlighterPhraseTest::TokenStreamConcurrent::TokenStreamConcurrent()
{
  reset();
}

bool HighlighterPhraseTest::TokenStreamConcurrent::incrementToken()
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

void HighlighterPhraseTest::TokenStreamConcurrent::reset()
{
  this->i = -1;
  this->tokens = std::deque<std::shared_ptr<Token>>{
      make_shared<Token>(L"the", 0, 3), make_shared<Token>(L"fox", 4, 7),
      make_shared<Token>(L"jump", 8, 14), make_shared<Token>(L"jumped", 8, 14)};
  this->tokens[3]->setPositionIncrement(0);
}
} // namespace org::apache::lucene::search::highlight