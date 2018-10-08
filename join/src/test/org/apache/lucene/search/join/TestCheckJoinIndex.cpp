using namespace std;

#include "TestCheckJoinIndex.h"

namespace org::apache::lucene::search::join
{
using Document = org::apache::lucene::document::Document;
using Store = org::apache::lucene::document::Field::Store;
using StringField = org::apache::lucene::document::StringField;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using NoMergePolicy = org::apache::lucene::index::NoMergePolicy;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using MatchNoDocsQuery = org::apache::lucene::search::MatchNoDocsQuery;
using TermQuery = org::apache::lucene::search::TermQuery;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestCheckJoinIndex::testNoParent() 
{
  shared_ptr<Directory> *const dir = newDirectory();
  shared_ptr<RandomIndexWriter> *const w =
      make_shared<RandomIndexWriter>(random(), dir);
  constexpr int numDocs = TestUtil::nextInt(random(), 1, 3);
  for (int i = 0; i < numDocs; ++i) {
    w->addDocument(make_shared<Document>());
  }
  shared_ptr<IndexReader> *const reader = w->getReader();
  delete w;
  shared_ptr<BitSetProducer> parentsFilter =
      make_shared<QueryBitSetProducer>(make_shared<MatchNoDocsQuery>());
  try {
    CheckJoinIndex::check(reader, parentsFilter);
    fail(L"Invalid index");
  } catch (const IllegalStateException &e) {
    // expected
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    delete reader;
    delete dir;
  }
}

void TestCheckJoinIndex::testOrphans() 
{
  shared_ptr<Directory> *const dir = newDirectory();
  shared_ptr<RandomIndexWriter> *const w =
      make_shared<RandomIndexWriter>(random(), dir);

  {
    // Add a first valid block
    deque<std::shared_ptr<Document>> block =
        deque<std::shared_ptr<Document>>();
    constexpr int numChildren = TestUtil::nextInt(random(), 0, 3);
    for (int i = 0; i < numChildren; ++i) {
      block.push_back(make_shared<Document>());
    }
    shared_ptr<Document> parent = make_shared<Document>();
    parent->push_back(make_shared<StringField>(L"parent", L"true", Store::NO));
    block.push_back(parent);
    w->addDocuments(block);
  }

  {
    // Then a block with no parent
    deque<std::shared_ptr<Document>> block =
        deque<std::shared_ptr<Document>>();
    constexpr int numChildren = TestUtil::nextInt(random(), 1, 3);
    for (int i = 0; i < numChildren; ++i) {
      block.push_back(make_shared<Document>());
    }
    w->addDocuments(block);
  }

  shared_ptr<IndexReader> *const reader = w->getReader();
  delete w;
  shared_ptr<BitSetProducer> parentsFilter = make_shared<QueryBitSetProducer>(
      make_shared<TermQuery>(make_shared<Term>(L"parent", L"true")));
  try {
    CheckJoinIndex::check(reader, parentsFilter);
    fail(L"Invalid index");
  } catch (const IllegalStateException &e) {
    // expected
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    delete reader;
    delete dir;
  }
}

void TestCheckJoinIndex::testInconsistentDeletes() 
{
  shared_ptr<Directory> *const dir = newDirectory();
  shared_ptr<IndexWriterConfig> *const iwc = newIndexWriterConfig();
  iwc->setMergePolicy(
      NoMergePolicy::INSTANCE); // so that deletions don't trigger merges
  shared_ptr<RandomIndexWriter> *const w =
      make_shared<RandomIndexWriter>(random(), dir, iwc);

  deque<std::shared_ptr<Document>> block = deque<std::shared_ptr<Document>>();
  constexpr int numChildren = TestUtil::nextInt(random(), 1, 3);
  for (int i = 0; i < numChildren; ++i) {
    shared_ptr<Document> doc = make_shared<Document>();
    // C++ TODO: There is no native C++ equivalent to 'toString':
    doc->push_back(
        make_shared<StringField>(L"child", Integer::toString(i), Store::NO));
    block.push_back(doc);
  }
  shared_ptr<Document> parent = make_shared<Document>();
  parent->push_back(make_shared<StringField>(L"parent", L"true", Store::NO));
  block.push_back(parent);
  w->addDocuments(block);

  if (random()->nextBoolean()) {
    w->deleteDocuments(make_shared<Term>(L"parent", L"true"));
  } else {
    // delete any of the children
    // C++ TODO: There is no native C++ equivalent to 'toString':
    w->deleteDocuments(make_shared<Term>(
        L"child", Integer::toString(random()->nextInt(numChildren))));
  }

  shared_ptr<IndexReader> *const reader = w->getReader();
  delete w;

  shared_ptr<BitSetProducer> parentsFilter = make_shared<QueryBitSetProducer>(
      make_shared<TermQuery>(make_shared<Term>(L"parent", L"true")));
  try {
    CheckJoinIndex::check(reader, parentsFilter);
    fail(L"Invalid index");
  } catch (const IllegalStateException &e) {
    // expected
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    delete reader;
    delete dir;
  }
}
} // namespace org::apache::lucene::search::join