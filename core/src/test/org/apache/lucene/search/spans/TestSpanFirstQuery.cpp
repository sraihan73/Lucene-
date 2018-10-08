using namespace std;

#include "TestSpanFirstQuery.h"

namespace org::apache::lucene::search::spans
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using IndexReader = org::apache::lucene::index::IndexReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using CharacterRunAutomaton =
    org::apache::lucene::util::automaton::CharacterRunAutomaton;
using RegExp = org::apache::lucene::util::automaton::RegExp;
using namespace org::apache::lucene::search::spans;
//    import static org.apache.lucene.search.spans.SpanTestUtil.*;

void TestSpanFirstQuery::testStartPositions() 
{
  shared_ptr<Directory> dir = newDirectory();

  // mimic StopAnalyzer
  shared_ptr<CharacterRunAutomaton> stopSet =
      make_shared<CharacterRunAutomaton>(
          (make_shared<RegExp>(L"the|a|of"))->toAutomaton());
  shared_ptr<Analyzer> analyzer =
      make_shared<MockAnalyzer>(random(), MockTokenizer::SIMPLE, true, stopSet);

  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir, analyzer);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(
      newTextField(L"field", L"the quick brown fox", Field::Store::NO));
  writer->addDocument(doc);
  shared_ptr<Document> doc2 = make_shared<Document>();
  doc2->push_back(newTextField(L"field", L"quick brown fox", Field::Store::NO));
  writer->addDocument(doc2);

  shared_ptr<IndexReader> reader = writer->getReader();
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);

  // user queries on "starts-with quick"
  shared_ptr<SpanQuery> sfq = SpanTestUtil::spanFirstQuery(
      SpanTestUtil::spanTermQuery(L"field", L"quick"), 1);
  assertEquals(1, searcher->search(sfq, 10)->totalHits);

  // user queries on "starts-with the quick"
  shared_ptr<SpanQuery> include = SpanTestUtil::spanFirstQuery(
      SpanTestUtil::spanTermQuery(L"field", L"quick"), 2);
  sfq = SpanTestUtil::spanNotQuery(include, sfq);
  assertEquals(1, searcher->search(sfq, 10)->totalHits);

  delete writer;
  delete reader;
  delete dir;
}
} // namespace org::apache::lucene::search::spans