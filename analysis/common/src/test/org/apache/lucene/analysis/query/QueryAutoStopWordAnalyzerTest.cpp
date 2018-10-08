using namespace std;

#include "QueryAutoStopWordAnalyzerTest.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/document/Document.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/document/Field.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/document/TextField.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/DirectoryReader.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/IndexReader.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/IndexWriter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/IndexWriterConfig.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/store/RAMDirectory.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockAnalyzer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/query/QueryAutoStopWordAnalyzer.h"

namespace org::apache::lucene::analysis::query
{
using namespace org::apache::lucene::analysis;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using TextField = org::apache::lucene::document::TextField;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using RAMDirectory = org::apache::lucene::store::RAMDirectory;

void QueryAutoStopWordAnalyzerTest::setUp() 
{
  BaseTokenStreamTestCase::setUp();
  dir = make_shared<RAMDirectory>();
  appAnalyzer =
      make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, false);
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, make_shared<IndexWriterConfig>(appAnalyzer));
  int numDocs = 200;
  for (int i = 0; i < numDocs; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    wstring variedFieldValue = variedFieldValues[i % variedFieldValues.size()];
    wstring repetitiveFieldValue =
        repetitiveFieldValues[i % repetitiveFieldValues.size()];
    doc->push_back(make_shared<TextField>(L"variedField", variedFieldValue,
                                          Field::Store::YES));
    doc->push_back(make_shared<TextField>(
        L"repetitiveField", repetitiveFieldValue, Field::Store::YES));
    writer->addDocument(doc);
  }
  delete writer;
  reader = DirectoryReader::open(dir);
}

void QueryAutoStopWordAnalyzerTest::tearDown() 
{
  delete appAnalyzer;
  delete reader;
  BaseTokenStreamTestCase::tearDown();
}

void QueryAutoStopWordAnalyzerTest::testNoStopwords() 
{
  // Note: an empty deque of fields passed in
  protectedAnalyzer = make_shared<QueryAutoStopWordAnalyzer>(
      appAnalyzer, reader, Collections::emptyList<wstring>(), 1);
  shared_ptr<TokenStream> protectedTokenStream =
      protectedAnalyzer->tokenStream(L"variedField", L"quick");
  assertTokenStreamContents(protectedTokenStream,
                            std::deque<wstring>{L"quick"});

  protectedTokenStream =
      protectedAnalyzer->tokenStream(L"repetitiveField", L"boring");
  assertTokenStreamContents(protectedTokenStream,
                            std::deque<wstring>{L"boring"});
  delete protectedAnalyzer;
}

void QueryAutoStopWordAnalyzerTest::testDefaultStopwordsAllFields() throw(
    runtime_error)
{
  protectedAnalyzer =
      make_shared<QueryAutoStopWordAnalyzer>(appAnalyzer, reader);
  shared_ptr<TokenStream> protectedTokenStream =
      protectedAnalyzer->tokenStream(L"repetitiveField", L"boring");
  assertTokenStreamContents(
      protectedTokenStream,
      std::deque<wstring>(
          0)); // Default stop word filtering will remove boring
  delete protectedAnalyzer;
}

void QueryAutoStopWordAnalyzerTest::
    testStopwordsAllFieldsMaxPercentDocs() 
{
  protectedAnalyzer =
      make_shared<QueryAutoStopWordAnalyzer>(appAnalyzer, reader, 1.0f / 2.0f);

  shared_ptr<TokenStream> protectedTokenStream =
      protectedAnalyzer->tokenStream(L"repetitiveField", L"boring");
  // A filter on terms in > one half of docs remove boring
  assertTokenStreamContents(protectedTokenStream, std::deque<wstring>(0));

  protectedTokenStream =
      protectedAnalyzer->tokenStream(L"repetitiveField", L"vaguelyboring");
  // A filter on terms in > half of docs should not remove vaguelyBoring
  assertTokenStreamContents(protectedTokenStream,
                            std::deque<wstring>{L"vaguelyboring"});
  delete protectedAnalyzer;

  protectedAnalyzer =
      make_shared<QueryAutoStopWordAnalyzer>(appAnalyzer, reader, 1.0f / 4.0f);
  protectedTokenStream =
      protectedAnalyzer->tokenStream(L"repetitiveField", L"vaguelyboring");
  // A filter on terms in > quarter of docs should remove vaguelyBoring
  assertTokenStreamContents(protectedTokenStream, std::deque<wstring>(0));
  delete protectedAnalyzer;
}

void QueryAutoStopWordAnalyzerTest::testStopwordsPerFieldMaxPercentDocs() throw(
    runtime_error)
{
  protectedAnalyzer = make_shared<QueryAutoStopWordAnalyzer>(
      appAnalyzer, reader, Arrays::asList(L"variedField"), 1.0f / 2.0f);
  shared_ptr<TokenStream> protectedTokenStream =
      protectedAnalyzer->tokenStream(L"repetitiveField", L"boring");
  // A filter on one Field should not affect queries on another
  assertTokenStreamContents(protectedTokenStream,
                            std::deque<wstring>{L"boring"});
  delete protectedAnalyzer;

  protectedAnalyzer = make_shared<QueryAutoStopWordAnalyzer>(
      appAnalyzer, reader, Arrays::asList(L"variedField", L"repetitiveField"),
      1.0f / 2.0f);
  protectedTokenStream =
      protectedAnalyzer->tokenStream(L"repetitiveField", L"boring");
  // A filter on the right Field should affect queries on it
  assertTokenStreamContents(protectedTokenStream, std::deque<wstring>(0));
  delete protectedAnalyzer;
}

void QueryAutoStopWordAnalyzerTest::testStopwordsPerFieldMaxDocFreq() throw(
    runtime_error)
{
  protectedAnalyzer = make_shared<QueryAutoStopWordAnalyzer>(
      appAnalyzer, reader, Arrays::asList(L"repetitiveField"), 10);
  int numStopWords = protectedAnalyzer->getStopWords(L"repetitiveField").size();
  assertTrue(L"Should have identified stop words", numStopWords > 0);
  delete protectedAnalyzer;

  protectedAnalyzer = make_shared<QueryAutoStopWordAnalyzer>(
      appAnalyzer, reader, Arrays::asList(L"repetitiveField", L"variedField"),
      10);
  int numNewStopWords =
      protectedAnalyzer->getStopWords(L"repetitiveField").size() +
      protectedAnalyzer->getStopWords(L"variedField").size();
  assertTrue(L"Should have identified more stop words",
             numNewStopWords > numStopWords);
  delete protectedAnalyzer;
}

void QueryAutoStopWordAnalyzerTest::testNoFieldNamePollution() throw(
    runtime_error)
{
  protectedAnalyzer = make_shared<QueryAutoStopWordAnalyzer>(
      appAnalyzer, reader, Arrays::asList(L"repetitiveField"), 10);

  shared_ptr<TokenStream> protectedTokenStream =
      protectedAnalyzer->tokenStream(L"repetitiveField", L"boring");
  // Check filter set up OK
  assertTokenStreamContents(protectedTokenStream, std::deque<wstring>(0));

  protectedTokenStream =
      protectedAnalyzer->tokenStream(L"variedField", L"boring");
  // Filter should not prevent stopwords in one field being used in another
  assertTokenStreamContents(protectedTokenStream,
                            std::deque<wstring>{L"boring"});
  delete protectedAnalyzer;
}

void QueryAutoStopWordAnalyzerTest::testTokenStream() 
{
  shared_ptr<QueryAutoStopWordAnalyzer> a =
      make_shared<QueryAutoStopWordAnalyzer>(
          make_shared<MockAnalyzer>(random(), MockTokenizer::WHITESPACE, false),
          reader, 10);
  shared_ptr<TokenStream> ts =
      a->tokenStream(L"repetitiveField", L"this boring");
  assertTokenStreamContents(ts, std::deque<wstring>{L"this"});
  delete a;
}
} // namespace org::apache::lucene::analysis::query