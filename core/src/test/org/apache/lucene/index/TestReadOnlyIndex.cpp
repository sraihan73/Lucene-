using namespace std;

#include "TestReadOnlyIndex.h"

namespace org::apache::lucene::index
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using PhraseQuery = org::apache::lucene::search::PhraseQuery;
using Query = org::apache::lucene::search::Query;
using TermQuery = org::apache::lucene::search::TermQuery;
using TopDocs = org::apache::lucene::search::TopDocs;
using Directory = org::apache::lucene::store::Directory;
using FSDirectory = org::apache::lucene::store::FSDirectory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::junit::AfterClass;
using org::junit::BeforeClass;
const wstring TestReadOnlyIndex::longTerm =
    L"longtermlongtermlongtermlongtermlongtermlongtermlongtermlongtermlongterml"
    L"ongtermlongtermlongtermlongtermlongtermlongtermlongtermlongtermlongterm";
const wstring TestReadOnlyIndex::text =
    L"This is the text to be indexed. " + longTerm;
shared_ptr<java::nio::file::Path> TestReadOnlyIndex::indexPath;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void buildIndex() throws Exception
void TestReadOnlyIndex::buildIndex() 
{
  indexPath = Files::createTempDirectory(L"readonlyindex").toAbsolutePath();

  // borrows from TestDemo, but not important to keep in sync with demo
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<Directory> directory = newFSDirectory(indexPath);
  shared_ptr<RandomIndexWriter> iwriter =
      make_shared<RandomIndexWriter>(random(), directory, analyzer);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newTextField(L"fieldname", text, Field::Store::YES));
  iwriter->addDocument(doc);
  delete iwriter;
  delete directory;
  delete analyzer;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @AfterClass public static void afterClass() throws Exception
void TestReadOnlyIndex::afterClass() ; }

void TestReadOnlyIndex::testReadOnlyIndex() 
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  runWithRestrictedPermissions(
      shared_from_this()::doTestReadOnlyIndex,
      make_shared<RuntimePermission>(L"*"),
      make_shared<PropertyPermission>(L"*", L"read"),
      make_shared<FilePermission>(indexPath->toString(), L"read"),
      make_shared<FilePermission>(indexPath->resolve(L"-")->toString(),
                                  L"read"));
}

shared_ptr<Void> TestReadOnlyIndex::doTestReadOnlyIndex() 
{
  shared_ptr<Directory> dir = FSDirectory::open(indexPath);
  shared_ptr<IndexReader> ireader = DirectoryReader::open(dir);
  shared_ptr<IndexSearcher> isearcher = newSearcher(ireader);

  // borrows from TestDemo, but not important to keep in sync with demo

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
  return nullptr; // void
}
} // namespace org::apache::lucene::index