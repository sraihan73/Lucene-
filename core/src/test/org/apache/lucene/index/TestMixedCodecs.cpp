using namespace std;

#include "TestMixedCodecs.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Codec = org::apache::lucene::codecs::Codec;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestMixedCodecs::test() 
{

  constexpr int NUM_DOCS = atLeast(1000);

  shared_ptr<Directory> *const dir = newDirectory();
  shared_ptr<RandomIndexWriter> w = nullptr;

  int docsLeftInThisSegment = 0;

  int docUpto = 0;
  while (docUpto < NUM_DOCS) {
    if (VERBOSE) {
      wcout << L"TEST: " << docUpto << L" of " << NUM_DOCS << endl;
    }
    if (docsLeftInThisSegment == 0) {
      shared_ptr<IndexWriterConfig> *const iwc =
          newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
      if (random()->nextBoolean()) {
        // Make sure we aggressively mix in SimpleText
        // since it has different impls for all codec
        // formats...
        iwc->setCodec(Codec::forName(L"SimpleText"));
      }
      if (w != nullptr) {
        delete w;
      }
      w = make_shared<RandomIndexWriter>(random(), dir, iwc);
      docsLeftInThisSegment = TestUtil::nextInt(random(), 10, 100);
    }
    shared_ptr<Document> *const doc = make_shared<Document>();
    doc->push_back(
        newStringField(L"id", to_wstring(docUpto), Field::Store::YES));
    w->addDocument(doc);
    docUpto++;
    docsLeftInThisSegment--;
  }

  if (VERBOSE) {
    wcout << L"\nTEST: now delete..." << endl;
  }

  // Random delete half the docs:
  shared_ptr<Set<int>> *const deleted = unordered_set<int>();
  while (deleted->size() < NUM_DOCS / 2) {
    const optional<int> toDelete = random()->nextInt(NUM_DOCS);
    if (!deleted->contains(toDelete)) {
      deleted->add(toDelete);
      w->deleteDocuments(make_shared<Term>(L"id", wstring::valueOf(toDelete)));
      if (random()->nextInt(17) == 6) {
        shared_ptr<IndexReader> *const r = w->getReader();
        TestUtil::assertEquals(NUM_DOCS - deleted->size(), r->numDocs());
        delete r;
      }
    }
  }

  delete w;
  delete dir;
}
} // namespace org::apache::lucene::index