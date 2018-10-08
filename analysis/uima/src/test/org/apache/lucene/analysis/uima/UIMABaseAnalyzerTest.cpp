using namespace std;

#include "UIMABaseAnalyzerTest.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/document/Document.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/document/Field.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/document/TextField.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/DirectoryReader.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/IndexWriter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/IndexWriterConfig.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/search/MatchAllDocsQuery.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/search/TopDocs.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/store/Directory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/store/RAMDirectory.h"
#include "../../../../../../java/org/apache/lucene/analysis/uima/UIMABaseAnalyzer.h"

namespace org::apache::lucene::analysis::uima
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using TextField = org::apache::lucene::document::TextField;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using MatchAllDocsQuery = org::apache::lucene::search::MatchAllDocsQuery;
using TopDocs = org::apache::lucene::search::TopDocs;
using Directory = org::apache::lucene::store::Directory;
using RAMDirectory = org::apache::lucene::store::RAMDirectory;
using org::junit::After;
using org::junit::Before;
using org::junit::Test;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Override @Before public void setUp() throws Exception
void UIMABaseAnalyzerTest::setUp() 
{
  BaseTokenStreamTestCase::setUp();
  analyzer = make_shared<UIMABaseAnalyzer>(L"/uima/AggregateSentenceAE.xml",
                                           L"org.apache.uima.TokenAnnotation",
                                           nullptr);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Override @After public void tearDown() throws Exception
void UIMABaseAnalyzerTest::tearDown() 
{
  delete analyzer;
  BaseTokenStreamTestCase::tearDown();
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void baseUIMAAnalyzerStreamTest() throws
// Exception
void UIMABaseAnalyzerTest::baseUIMAAnalyzerStreamTest() 
{
  shared_ptr<TokenStream> ts =
      analyzer->tokenStream(L"text", L"the big brown fox jumped on the wood");
  assertTokenStreamContents(ts, std::deque<wstring>{L"the", L"big", L"brown",
                                                     L"fox", L"jumped", L"on",
                                                     L"the", L"wood"});
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void baseUIMAAnalyzerIntegrationTest() throws
// Exception
void UIMABaseAnalyzerTest::baseUIMAAnalyzerIntegrationTest() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = make_shared<RAMDirectory>();
  shared_ptr<IndexWriter> writer =
      make_shared<IndexWriter>(dir, make_shared<IndexWriterConfig>(analyzer));
  // add the first doc
  shared_ptr<Document> doc = make_shared<Document>();
  wstring dummyTitle = L"this is a dummy title ";
  doc->push_back(
      make_shared<TextField>(L"title", dummyTitle, Field::Store::YES));
  wstring dummyContent = L"there is some content written here";
  doc->push_back(
      make_shared<TextField>(L"contents", dummyContent, Field::Store::YES));
  writer->addDocument(doc);
  writer->commit();

  // try the search over the first doc
  shared_ptr<DirectoryReader> directoryReader = DirectoryReader::open(dir);
  shared_ptr<IndexSearcher> indexSearcher = newSearcher(directoryReader);
  shared_ptr<TopDocs> result =
      indexSearcher->search(make_shared<MatchAllDocsQuery>(), 1);
  assertTrue(result->totalHits > 0);
  shared_ptr<Document> d = indexSearcher->doc(result->scoreDocs[0]->doc);
  assertNotNull(d);
  assertNotNull(d->getField(L"title"));
  assertEquals(dummyTitle, d->getField(L"title")->stringValue());
  assertNotNull(d->getField(L"contents"));
  assertEquals(dummyContent, d->getField(L"contents")->stringValue());

  // add a second doc
  doc = make_shared<Document>();
  wstring dogmasTitle = L"dogmas";
  doc->push_back(
      make_shared<TextField>(L"title", dogmasTitle, Field::Store::YES));
  wstring dogmasContents = L"white men can't jump";
  doc->push_back(
      make_shared<TextField>(L"contents", dogmasContents, Field::Store::YES));
  writer->addDocument(doc);
  writer->commit();

  directoryReader->close();
  directoryReader = DirectoryReader::open(dir);
  indexSearcher = newSearcher(directoryReader);
  result = indexSearcher->search(make_shared<MatchAllDocsQuery>(), 2);
  shared_ptr<Document> d1 = indexSearcher->doc(result->scoreDocs[1]->doc);
  assertNotNull(d1);
  assertNotNull(d1->getField(L"title"));
  assertEquals(dogmasTitle, d1->getField(L"title")->stringValue());
  assertNotNull(d1->getField(L"contents"));
  assertEquals(dogmasContents, d1->getField(L"contents")->stringValue());

  // do a matchalldocs query to retrieve both docs
  result = indexSearcher->search(make_shared<MatchAllDocsQuery>(), 2);
  assertEquals(2, result->totalHits);
  delete writer;
  delete indexSearcher->getIndexReader();
  delete dir;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test @AwaitsFix(bugUrl =
// "https://issues.apache.org/jira/browse/LUCENE-3869") public void
// testRandomStrings() throws Exception
void UIMABaseAnalyzerTest::testRandomStrings() 
{
  shared_ptr<Analyzer> analyzer = make_shared<UIMABaseAnalyzer>(
      L"/uima/TestAggregateSentenceAE.xml",
      L"org.apache.lucene.uima.ts.TokenAnnotation", nullptr);
  checkRandomData(random(), analyzer, 100 * RANDOM_MULTIPLIER);
  delete analyzer;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test @AwaitsFix(bugUrl =
// "https://issues.apache.org/jira/browse/LUCENE-3869") public void
// testRandomStringsWithConfigurationParameters() throws Exception
void UIMABaseAnalyzerTest::testRandomStringsWithConfigurationParameters() throw(
    runtime_error)
{
  unordered_map<wstring, any> cp = unordered_map<wstring, any>();
  cp.emplace(L"line-end", L"\r");
  shared_ptr<Analyzer> analyzer = make_shared<UIMABaseAnalyzer>(
      L"/uima/TestWSTokenizerAE.xml",
      L"org.apache.lucene.uima.ts.TokenAnnotation", cp);
  checkRandomData(random(), analyzer, 100 * RANDOM_MULTIPLIER);
  delete analyzer;
}
} // namespace org::apache::lucene::analysis::uima