using namespace std;

#include "TestDemo.h"

namespace org::apache::lucene
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using namespace org::apache::lucene::search;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestDemo::testDemo() 
{
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());

  // Store the index in memory:
  shared_ptr<Directory> directory = newDirectory();
  // To store an index on disk, use this instead:
  // Directory directory = FSDirectory.open(new File("/tmp/testindex"));
  shared_ptr<RandomIndexWriter> iwriter =
      make_shared<RandomIndexWriter>(random(), directory, analyzer);
  shared_ptr<Document> doc = make_shared<Document>();
  wstring longTerm = L"longtermlongtermlongtermlongtermlongtermlongtermlongterm"
                     L"longtermlongtermlongtermlongtermlongtermlongtermlongterm"
                     L"longtermlongtermlongtermlongterm";
  wstring text = L"This is the text to be indexed. " + longTerm;
  doc->push_back(newTextField(L"fieldname", text, Field::Store::YES));
  iwriter->addDocument(doc);
  delete iwriter;

  // Now search the index:
  shared_ptr<IndexReader> ireader =
      DirectoryReader::open(directory); // read-only=true
  shared_ptr<IndexSearcher> isearcher = newSearcher(ireader);

  assertEquals(1, isearcher
                      ->search(make_shared<TermQuery>(
                                   make_shared<Term>(L"fieldname", longTerm)),
                               1)
                      ->totalHits);
  shared_ptr<Query> query =
      make_shared<TermQuery>(make_shared<Term>(L"fieldname", L"text"));
  shared_ptr<TopDocs> hits = isearcher->search(query, 1);
  assertEquals(1, hits->totalHits);
  // Iterate through the results:
  for (int i = 0; i < hits->scoreDocs.size(); i++) {
    shared_ptr<Document> hitDoc = isearcher->doc(hits->scoreDocs[i]->doc);
    assertEquals(text, hitDoc[L"fieldname"]);
  }

  // Test simple phrase query
  shared_ptr<PhraseQuery> phraseQuery =
      make_shared<PhraseQuery>(L"fieldname", L"to", L"be");
  assertEquals(1, isearcher->search(phraseQuery, 1)->totalHits);

  delete ireader;
  delete directory;
  delete analyzer;
}
} // namespace org::apache::lucene