using namespace std;

#include "AbstractTestCase.h"

namespace org::apache::lucene::search::vectorhighlight
{
using namespace org::apache::lucene::analysis;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using TermToBytesRefAttribute =
    org::apache::lucene::analysis::tokenattributes::TermToBytesRefAttribute;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using TextField = org::apache::lucene::document::TextField;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using OpenMode = org::apache::lucene::index::IndexWriterConfig::OpenMode;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using Term = org::apache::lucene::index::Term;
using BoostQuery = org::apache::lucene::search::BoostQuery;
using DisjunctionMaxQuery = org::apache::lucene::search::DisjunctionMaxQuery;
using PhraseQuery = org::apache::lucene::search::PhraseQuery;
using Query = org::apache::lucene::search::Query;
using TermQuery = org::apache::lucene::search::TermQuery;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
const wstring AbstractTestCase::F = L"f";
const wstring AbstractTestCase::F1 = L"f1";
const wstring AbstractTestCase::F2 = L"f2";
std::deque<wstring> const AbstractTestCase::shortMVValues = {
    L"", L"", L"a b c", L"", L"d e"};
std::deque<wstring> const AbstractTestCase::longMVValues = {
    L"Followings are the examples of customizable parameters and actual "
    L"examples of customization:",
    L"The most search engines use only one of these methods. Even the search "
    L"engines that says they can use the both methods basically"};
std::deque<wstring> const AbstractTestCase::biMVValues = {
    L"\nLucene/Solr does not require such additional hardware.",
    L"\nWhen you talk about processing speed, the"};
std::deque<wstring> const AbstractTestCase::strMVValues = {L"abc", L"defg",
                                                            L"hijkl"};

void AbstractTestCase::setUp() 
{
  LuceneTestCase::setUp();
  analyzerW =
      make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, false);
  analyzerB = make_shared<BigramAnalyzer>();
  analyzerK =
      make_shared<MockAnalyzer>(random(), MockTokenizer::KEYWORD, false);
  dir = newDirectory();
}

void AbstractTestCase::tearDown() 
{
  if (reader != nullptr) {
    delete reader;
    reader.reset();
  }
  delete dir;
  LuceneTestCase::tearDown();
}

shared_ptr<Query> AbstractTestCase::tq(const wstring &text)
{
  return tq(1.0F, text);
}

shared_ptr<Query> AbstractTestCase::tq(float boost, const wstring &text)
{
  return tq(boost, F, text);
}

shared_ptr<Query> AbstractTestCase::tq(const wstring &field,
                                       const wstring &text)
{
  return tq(1.0F, field, text);
}

shared_ptr<Query> AbstractTestCase::tq(float boost, const wstring &field,
                                       const wstring &text)
{
  shared_ptr<Query> query =
      make_shared<TermQuery>(make_shared<Term>(field, text));
  if (boost != 1.0f) {
    query = make_shared<BoostQuery>(query, boost);
  }
  return query;
}

shared_ptr<Query> AbstractTestCase::pqF(deque<wstring> &texts)
{
  return pqF(1.0F, texts);
}

shared_ptr<Query> AbstractTestCase::pqF(float boost, deque<wstring> &texts)
{
  return pqF(boost, 0, {texts});
}

shared_ptr<Query> AbstractTestCase::pqF(float boost, int slop,
                                        deque<wstring> &texts)
{
  return pq(boost, slop, F, {texts});
}

shared_ptr<Query> AbstractTestCase::pq(const wstring &field,
                                       deque<wstring> &texts)
{
  return pq(1.0F, 0, field, {texts});
}

shared_ptr<Query> AbstractTestCase::pq(float boost, const wstring &field,
                                       deque<wstring> &texts)
{
  return pq(boost, 0, field, {texts});
}

shared_ptr<Query> AbstractTestCase::pq(float boost, int slop,
                                       const wstring &field,
                                       deque<wstring> &texts)
{
  shared_ptr<Query> query = make_shared<PhraseQuery>(slop, field, texts);
  if (boost != 1.0f) {
    query = make_shared<BoostQuery>(query, boost);
  }
  return query;
}

shared_ptr<Query> AbstractTestCase::dmq(deque<Query> &queries)
{
  return dmq(0.0F, queries);
}

shared_ptr<Query> AbstractTestCase::dmq(float tieBreakerMultiplier,
                                        deque<Query> &queries)
{
  return make_shared<DisjunctionMaxQuery>(Arrays::asList(queries),
                                          tieBreakerMultiplier);
}

void AbstractTestCase::assertCollectionQueries(
    shared_ptr<deque<std::shared_ptr<Query>>> actual,
    deque<Query> &expected)
{
  assertEquals(expected->length, actual->size());
  for (shared_ptr<Query> query : expected) {
    assertTrue(actual->contains(query));
  }
}

deque<std::shared_ptr<BytesRef>>
AbstractTestCase::analyze(const wstring &text, const wstring &field,
                          shared_ptr<Analyzer> analyzer) 
{
  deque<std::shared_ptr<BytesRef>> bytesRefs =
      deque<std::shared_ptr<BytesRef>>();

  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (TokenStream tokenStream =
  // analyzer.tokenStream(field, text))
  {
    TokenStream tokenStream = analyzer->tokenStream(field, text);
    shared_ptr<TermToBytesRefAttribute> termAttribute =
        tokenStream->getAttribute(TermToBytesRefAttribute::typeid);

    tokenStream->reset();

    while (tokenStream->incrementToken()) {
      bytesRefs.push_back(BytesRef::deepCopyOf(termAttribute->getBytesRef()));
    }

    tokenStream->end();
  }

  return bytesRefs;
}

shared_ptr<PhraseQuery>
AbstractTestCase::toPhraseQuery(deque<std::shared_ptr<BytesRef>> &bytesRefs,
                                const wstring &field)
{
  return make_shared<PhraseQuery>(
      field, bytesRefs.toArray(std::deque<std::shared_ptr<BytesRef>>(0)));
}

shared_ptr<TokenStreamComponents>
AbstractTestCase::BigramAnalyzer::createComponents(const wstring &fieldName)
{
  return make_shared<TokenStreamComponents>(make_shared<BasicNGramTokenizer>());
}

const wstring AbstractTestCase::BasicNGramTokenizer::DEFAULT_DELIMITERS =
    L" \t\n.,";

AbstractTestCase::BasicNGramTokenizer::BasicNGramTokenizer()
    : BasicNGramTokenizer(DEFAULT_N_SIZE)
{
}

AbstractTestCase::BasicNGramTokenizer::BasicNGramTokenizer(int n)
    : BasicNGramTokenizer(n, DEFAULT_DELIMITERS)
{
}

AbstractTestCase::BasicNGramTokenizer::BasicNGramTokenizer(
    const wstring &delimiters)
    : BasicNGramTokenizer(DEFAULT_N_SIZE, delimiters)
{
}

AbstractTestCase::BasicNGramTokenizer::BasicNGramTokenizer(
    int n, const wstring &delimiters)
    : Tokenizer(), n(n), delimiters(delimiters)
{
  startTerm = 0;
  nextStartOffset = 0;
  snippet = L"";
  snippetBuffer = make_shared<StringBuilder>();
  charBuffer = std::deque<wchar_t>(BUFFER_SIZE);
  charBufferIndex = BUFFER_SIZE;
  charBufferLen = 0;
  ch = 0;
}

bool AbstractTestCase::BasicNGramTokenizer::incrementToken() 
{
  if (!getNextPartialSnippet()) {
    return false;
  }
  clearAttributes();
  termAtt->setEmpty()->append(snippet, startTerm, startTerm + lenTerm);
  offsetAtt->setOffset(correctOffset(startOffset),
                       correctOffset(startOffset + lenTerm));
  return true;
}

int AbstractTestCase::BasicNGramTokenizer::getFinalOffset()
{
  return nextStartOffset;
}

void AbstractTestCase::BasicNGramTokenizer::end() 
{
  Tokenizer::end();
  offsetAtt->setOffset(getFinalOffset(), getFinalOffset());
}

bool AbstractTestCase::BasicNGramTokenizer::getNextPartialSnippet() throw(
    IOException)
{
  if (snippet != L"" && snippet.length() >= startTerm + 1 + n) {
    startTerm++;
    startOffset++;
    lenTerm = n;
    return true;
  }
  return getNextSnippet();
}

bool AbstractTestCase::BasicNGramTokenizer::getNextSnippet() 
{
  startTerm = 0;
  startOffset = nextStartOffset;
  snippetBuffer->remove(0, snippetBuffer->length());
  while (true) {
    if (ch != -1) {
      ch = readCharFromBuffer();
    }
    if (ch == -1) {
      break;
    } else if (!isDelimiter(ch)) {
      snippetBuffer->append(static_cast<wchar_t>(ch));
    } else if (snippetBuffer->length() > 0) {
      break;
    } else {
      startOffset++;
    }
  }
  if (snippetBuffer->length() == 0) {
    return false;
  }
  snippet = snippetBuffer->toString();
  lenTerm = snippet.length() >= n ? n : snippet.length();
  return true;
}

int AbstractTestCase::BasicNGramTokenizer::readCharFromBuffer() throw(
    IOException)
{
  if (charBufferIndex >= charBufferLen) {
    charBufferLen = input->read(charBuffer);
    if (charBufferLen == -1) {
      return -1;
    }
    charBufferIndex = 0;
  }
  int c = charBuffer[charBufferIndex++];
  nextStartOffset++;
  return c;
}

bool AbstractTestCase::BasicNGramTokenizer::isDelimiter(int c)
{
  return delimiters.find(c) != wstring::npos;
}

void AbstractTestCase::BasicNGramTokenizer::reset() 
{
  Tokenizer::reset();
  startTerm = 0;
  nextStartOffset = 0;
  snippet = L"";
  snippetBuffer->setLength(0);
  charBufferIndex = BUFFER_SIZE;
  charBufferLen = 0;
  ch = 0;
}

void AbstractTestCase::make1d1fIndex(const wstring &value) 
{
  make1dmfIndex({value});
}

void AbstractTestCase::make1d1fIndexB(const wstring &value) 
{
  make1dmfIndexB({value});
}

void AbstractTestCase::make1dmfIndex(deque<wstring> &values) throw(
    runtime_error)
{
  make1dmfIndex(analyzerW, values);
}

void AbstractTestCase::make1dmfIndexB(deque<wstring> &values) throw(
    runtime_error)
{
  make1dmfIndex(analyzerB, values);
}

void AbstractTestCase::make1dmfIndex(
    shared_ptr<Analyzer> analyzer, deque<wstring> &values) 
{
  shared_ptr<IndexWriter> writer =
      make_shared<IndexWriter>(dir, (make_shared<IndexWriterConfig>(analyzer))
                                        ->setOpenMode(OpenMode::CREATE));
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<FieldType> customType =
      make_shared<FieldType>(TextField::TYPE_STORED);
  customType->setStoreTermVectors(true);
  customType->setStoreTermVectorOffsets(true);
  customType->setStoreTermVectorPositions(true);
  for (wstring value : values) {
    doc->push_back(make_shared<Field>(F, value, customType));
  }
  writer->addDocument(doc);
  delete writer;
  if (reader != nullptr) {
    delete reader;
  }
  reader = DirectoryReader::open(dir);
}

void AbstractTestCase::make1dmfIndexNA(deque<wstring> &values) throw(
    runtime_error)
{
  shared_ptr<IndexWriter> writer =
      make_shared<IndexWriter>(dir, (make_shared<IndexWriterConfig>(analyzerK))
                                        ->setOpenMode(OpenMode::CREATE));
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<FieldType> customType =
      make_shared<FieldType>(TextField::TYPE_STORED);
  customType->setStoreTermVectors(true);
  customType->setStoreTermVectorOffsets(true);
  customType->setStoreTermVectorPositions(true);
  for (wstring value : values) {
    doc->push_back(make_shared<Field>(F, value, customType));
    // doc.add( new Field( F, value, Store.YES, Index.NOT_ANALYZED,
    // TermVector.WITH_POSITIONS_OFFSETS ) );
  }
  writer->addDocument(doc);
  delete writer;
  if (reader != nullptr) {
    delete reader;
  }
  reader = DirectoryReader::open(dir);
}

void AbstractTestCase::makeIndexShortMV() 
{

  //  0
  // ""
  //  1
  // ""

  //  234567
  // "a b c"
  //  0 1 2

  //  8
  // ""

  //   111
  //  9012
  // "d e"
  //  3 4
  make1dmfIndex(shortMVValues);
}

void AbstractTestCase::makeIndexLongMV() 
{
  //           11111111112222222222333333333344444444445555555555666666666677777777778888888888999
  // 012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012
  // Followings are the examples of customizable parameters and actual examples
  // of customization: 0          1   2   3        4  5            6          7
  // 8      9        10 11

  //        1 2
  // 999999900000000001111111111222222222233333333334444444444555555555566666666667777777777888888888899999999990000000000111111111122
  // 345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901
  // The most search engines use only one of these methods. Even the search
  // engines that says they can use the both methods basically 12  13  (14) (15)
  // 16  17   18  19 20    21       22   23 (24)   (25)     26   27   28   29 30
  // 31  32   33      34

  make1dmfIndex(longMVValues);
}

void AbstractTestCase::makeIndexLongMVB() 
{
  // "*" ... LF

  //           1111111111222222222233333333334444444444555555
  // 01234567890123456789012345678901234567890123456789012345
  // *Lucene/Solr does not require such additional hardware.
  //  Lu 0        do 10    re 15   su 21       na 31
  //   uc 1        oe 11    eq 16   uc 22       al 32
  //    ce 2        es 12    qu 17   ch 23         ha 33
  //     en 3          no 13  ui 18     ad 24       ar 34
  //      ne 4          ot 14  ir 19     dd 25       rd 35
  //       e/ 5                 re 20     di 26       dw 36
  //        /S 6                           it 27       wa 37
  //         So 7                           ti 28       ar 38
  //          ol 8                           io 29       re 39
  //           lr 9                           on 30

  // 5555666666666677777777778888888888999999999
  // 6789012345678901234567890123456789012345678
  // *When you talk about processing speed, the
  //  Wh 40         ab 48     es 56         th 65
  //   he 41         bo 49     ss 57         he 66
  //    en 42         ou 50     si 58
  //       yo 43       ut 51     in 59
  //        ou 44         pr 52   ng 60
  //           ta 45       ro 53     sp 61
  //            al 46       oc 54     pe 62
  //             lk 47       ce 55     ee 63
  //                                    ed 64

  make1dmfIndexB(biMVValues);
}

void AbstractTestCase::makeIndexStrMV() 
{

  //  0123
  // "abc"

  //  34567
  // "defg"

  //     111
  //  789012
  // "hijkl"
  make1dmfIndexNA(strMVValues);
}
} // namespace org::apache::lucene::search::vectorhighlight