using namespace std;

#include "TestTeeSinkTokenFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CachingTokenFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/LowerCaseFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/standard/StandardFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/standard/StandardTokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/document/Document.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/document/Field.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/document/FieldType.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/document/TextField.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/DirectoryReader.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/IndexReader.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/IndexWriter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/PostingsEnum.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/Terms.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/TermsEnum.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/search/DocIdSetIterator.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/store/Directory.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockAnalyzer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/util/English.h"
#include "../../../../../../java/org/apache/lucene/analysis/sinks/TeeSinkTokenFilter.h"

namespace org::apache::lucene::analysis::sinks
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using CachingTokenFilter = org::apache::lucene::analysis::CachingTokenFilter;
using FilteringTokenFilter =
    org::apache::lucene::analysis::FilteringTokenFilter;
using LowerCaseFilter = org::apache::lucene::analysis::LowerCaseFilter;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using StandardFilter = org::apache::lucene::analysis::standard::StandardFilter;
using StandardTokenizer =
    org::apache::lucene::analysis::standard::StandardTokenizer;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using TextField = org::apache::lucene::document::TextField;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using Directory = org::apache::lucene::store::Directory;
using English = org::apache::lucene::util::English;

void TestTeeSinkTokenFilter::setUp() 
{
  BaseTokenStreamTestCase::setUp();
  tokens1 =
      std::deque<wstring>{L"The",  L"quick", L"Burgundy", L"Fox", L"jumped",
                           L"over", L"the",   L"lazy",     L"Red", L"Dogs"};
  tokens2 = std::deque<wstring>{L"The",  L"Lazy", L"Dogs", L"should",
                                 L"stay", L"on",   L"the",  L"porch"};
  buffer1 = make_shared<StringBuilder>();

  for (int i = 0; i < tokens1.size(); i++) {
    buffer1->append(tokens1[i])->append(L' ');
  }
  buffer2 = make_shared<StringBuilder>();
  for (int i = 0; i < tokens2.size(); i++) {
    buffer2->append(tokens2[i])->append(L' ');
  }
}

void TestTeeSinkTokenFilter::
    testEndOffsetPositionWithTeeSinkTokenFilter() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Analyzer> analyzer =
      make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, false);
  shared_ptr<IndexWriter> w =
      make_shared<IndexWriter>(dir, newIndexWriterConfig(analyzer));
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<TokenStream> tokenStream =
      analyzer->tokenStream(L"field", L"abcd   ");
  shared_ptr<TeeSinkTokenFilter> tee =
      make_shared<TeeSinkTokenFilter>(tokenStream);
  shared_ptr<TokenStream> sink = tee->newSinkTokenStream();
  shared_ptr<FieldType> ft = make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  ft->setStoreTermVectors(true);
  ft->setStoreTermVectorOffsets(true);
  ft->setStoreTermVectorPositions(true);
  shared_ptr<Field> f1 = make_shared<Field>(L"field", tee, ft);
  shared_ptr<Field> f2 = make_shared<Field>(L"field", sink, ft);
  doc->push_back(f1);
  doc->push_back(f2);
  w->addDocument(doc);
  delete w;

  shared_ptr<IndexReader> r = DirectoryReader::open(dir);
  shared_ptr<Terms> deque = r->getTermVectors(0)->terms(L"field");
  assertEquals(1, deque->size());
  shared_ptr<TermsEnum> termsEnum = deque->begin();
  termsEnum->next();
  assertEquals(2, termsEnum->totalTermFreq());
  shared_ptr<PostingsEnum> positions =
      termsEnum->postings(nullptr, PostingsEnum::ALL);
  assertTrue(positions->nextDoc() != DocIdSetIterator::NO_MORE_DOCS);
  assertEquals(2, positions->freq());
  positions->nextPosition();
  assertEquals(0, positions->startOffset());
  assertEquals(4, positions->endOffset());
  positions->nextPosition();
  assertEquals(8, positions->startOffset());
  assertEquals(12, positions->endOffset());
  assertEquals(DocIdSetIterator::NO_MORE_DOCS, positions->nextDoc());
  delete r;
  delete dir;
  delete analyzer;
}

void TestTeeSinkTokenFilter::testGeneral() 
{
  shared_ptr<TeeSinkTokenFilter> *const source =
      make_shared<TeeSinkTokenFilter>(
          whitespaceMockTokenizer(buffer1->toString()));
  shared_ptr<TokenStream> *const sink = source->newSinkTokenStream();

  source->addAttribute(
      BaseTokenStreamTestCase::CheckClearAttributesAttribute::typeid);
  sink->addAttribute(
      BaseTokenStreamTestCase::CheckClearAttributesAttribute::typeid);

  assertTokenStreamContents(source, tokens1);
  assertTokenStreamContents(sink, tokens1);
}

void TestTeeSinkTokenFilter::testMultipleSources() 
{
  shared_ptr<TeeSinkTokenFilter> *const tee1 = make_shared<TeeSinkTokenFilter>(
      whitespaceMockTokenizer(buffer1->toString()));
  shared_ptr<TokenStream> *const source1 =
      make_shared<CachingTokenFilter>(tee1);

  tee1->addAttribute(
      BaseTokenStreamTestCase::CheckClearAttributesAttribute::typeid);

  shared_ptr<MockTokenizer> tokenizer = make_shared<MockTokenizer>(
      tee1->getAttributeFactory(), MockTokenizer::WHITESPACE, false);
  tokenizer->setReader(make_shared<StringReader>(buffer2->toString()));
  shared_ptr<TeeSinkTokenFilter> *const tee2 =
      make_shared<TeeSinkTokenFilter>(tokenizer);
  shared_ptr<TokenStream> *const source2 = tee2;

  assertTokenStreamContents(source1, tokens1);
  assertTokenStreamContents(source2, tokens2);

  shared_ptr<TokenStream> lowerCasing = make_shared<LowerCaseFilter>(source1);
  std::deque<wstring> lowerCaseTokens(tokens1.size());
  for (int i = 0; i < tokens1.size(); i++) {
    lowerCaseTokens[i] = tokens1[i].toLowerCase(Locale::ROOT);
  }
  assertTokenStreamContents(lowerCasing, lowerCaseTokens);
}

shared_ptr<StandardTokenizer>
TestTeeSinkTokenFilter::standardTokenizer(shared_ptr<StringBuilder> builder)
{
  shared_ptr<StandardTokenizer> tokenizer = make_shared<StandardTokenizer>();
  tokenizer->setReader(make_shared<StringReader>(builder->toString()));
  return tokenizer;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings("resource") public void performance() throws
// Exception
void TestTeeSinkTokenFilter::performance() 
{
  std::deque<int> tokCount = {100, 500, 1000, 2000, 5000, 10000};
  std::deque<int> modCounts = {1, 2, 5, 10, 20, 50, 100, 200, 500};
  for (int k = 0; k < tokCount.size(); k++) {
    shared_ptr<StringBuilder> buffer = make_shared<StringBuilder>();
    wcout << L"-----Tokens: " << tokCount[k] << L"-----" << endl;
    for (int i = 0; i < tokCount[k]; i++) {
      buffer->append(English::intToEnglish(i).toUpperCase(Locale::ROOT))
          ->append(L' ');
    }
    // make sure we produce the same tokens
    shared_ptr<TeeSinkTokenFilter> teeStream = make_shared<TeeSinkTokenFilter>(
        make_shared<StandardFilter>(standardTokenizer(buffer)));
    shared_ptr<TokenStream> sink =
        make_shared<ModuloTokenFilter>(teeStream->newSinkTokenStream(), 100);
    teeStream->consumeAllTokens();
    shared_ptr<TokenStream> stream = make_shared<ModuloTokenFilter>(
        make_shared<StandardFilter>(standardTokenizer(buffer)), 100);
    shared_ptr<CharTermAttribute> tfTok =
        stream->addAttribute(CharTermAttribute::typeid);
    shared_ptr<CharTermAttribute> sinkTok =
        sink->addAttribute(CharTermAttribute::typeid);
    for (int i = 0; stream->incrementToken(); i++) {
      assertTrue(sink->incrementToken());
      assertTrue(tfTok + L" is not equal to " + sinkTok + L" at token: " +
                     to_wstring(i),
                 tfTok->equals(sinkTok) == true);
    }

    // simulate two fields, each being analyzed once, for 20 documents
    for (int j = 0; j < modCounts.size(); j++) {
      int tfPos = 0;
      int64_t start = System::currentTimeMillis();
      for (int i = 0; i < 20; i++) {
        stream = make_shared<StandardFilter>(standardTokenizer(buffer));
        shared_ptr<PositionIncrementAttribute> posIncrAtt =
            stream->getAttribute(PositionIncrementAttribute::typeid);
        while (stream->incrementToken()) {
          tfPos += posIncrAtt->getPositionIncrement();
        }
        stream = make_shared<ModuloTokenFilter>(
            make_shared<StandardFilter>(standardTokenizer(buffer)),
            modCounts[j]);
        posIncrAtt = stream->getAttribute(PositionIncrementAttribute::typeid);
        while (stream->incrementToken()) {
          tfPos += posIncrAtt->getPositionIncrement();
        }
      }
      int64_t finish = System::currentTimeMillis();
      wcout << L"ModCount: " << modCounts[j] << L" Two fields took "
            << (finish - start) << L" ms" << endl;
      int sinkPos = 0;
      // simulate one field with one sink
      start = System::currentTimeMillis();
      for (int i = 0; i < 20; i++) {
        teeStream = make_shared<TeeSinkTokenFilter>(
            make_shared<StandardFilter>(standardTokenizer(buffer)));
        sink = make_shared<ModuloTokenFilter>(teeStream->newSinkTokenStream(),
                                              modCounts[j]);
        shared_ptr<PositionIncrementAttribute> posIncrAtt =
            teeStream->getAttribute(PositionIncrementAttribute::typeid);
        while (teeStream->incrementToken()) {
          sinkPos += posIncrAtt->getPositionIncrement();
        }
        // System.out.println("Modulo--------");
        posIncrAtt = sink->getAttribute(PositionIncrementAttribute::typeid);
        while (sink->incrementToken()) {
          sinkPos += posIncrAtt->getPositionIncrement();
        }
      }
      finish = System::currentTimeMillis();
      wcout << L"ModCount: " << modCounts[j] << L" Tee fields took "
            << (finish - start) << L" ms" << endl;
      assertTrue(to_wstring(sinkPos) + L" does not equal: " + to_wstring(tfPos),
                 sinkPos == tfPos);
    }
    wcout << L"- End Tokens: " << tokCount[k] << L"-----" << endl;
  }
}

TestTeeSinkTokenFilter::ModuloTokenFilter::ModuloTokenFilter(
    shared_ptr<TokenStream> input, int mc)
    : org::apache::lucene::analysis::TokenFilter(input)
{
  modCount = mc;
}

bool TestTeeSinkTokenFilter::ModuloTokenFilter::incrementToken() throw(
    IOException)
{
  bool hasNext;
  for (hasNext = input->incrementToken(); hasNext && count % modCount != 0;
       hasNext = input->incrementToken()) {
    count++;
  }
  count++;
  return hasNext;
}

TestTeeSinkTokenFilter::ModuloSinkFilter::ModuloSinkFilter(
    shared_ptr<TokenStream> input, int mc)
    : org::apache::lucene::analysis::FilteringTokenFilter(input)
{
  modCount = mc;
}

bool TestTeeSinkTokenFilter::ModuloSinkFilter::accept() 
{
  bool b = count % modCount == 0;
  count++;
  return b;
}
} // namespace org::apache::lucene::analysis::sinks