using namespace std;

#include "TestStressAdvance.h"

namespace org::apache::lucene::index
{
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestStressAdvance::testStressAdvance() 
{
  for (int iter = 0; iter < 3; iter++) {
    if (VERBOSE) {
      wcout << L"\nTEST: iter=" << iter << endl;
    }
    shared_ptr<Directory> dir = newDirectory();
    shared_ptr<RandomIndexWriter> w =
        make_shared<RandomIndexWriter>(random(), dir);
    shared_ptr<Set<int>> *const aDocs = unordered_set<int>();
    shared_ptr<Document> *const doc = make_shared<Document>();
    shared_ptr<Field> *const f =
        newStringField(L"field", L"", Field::Store::NO);
    doc->push_back(f);
    shared_ptr<Field> *const idField =
        newStringField(L"id", L"", Field::Store::YES);
    doc->push_back(idField);
    int num = atLeast(4097);
    if (VERBOSE) {
      wcout << L"\nTEST: numDocs=" << num << endl;
    }
    for (int id = 0; id < num; id++) {
      if (random()->nextInt(4) == 3) {
        f->setStringValue(L"a");
        aDocs->add(id);
      } else {
        f->setStringValue(L"b");
      }
      idField->setStringValue(L"" + to_wstring(id));
      w->addDocument(doc);
      if (VERBOSE) {
        wcout << L"\nTEST: doc upto " << id << endl;
      }
    }

    w->forceMerge(1);

    const deque<int> aDocIDs = deque<int>();
    const deque<int> bDocIDs = deque<int>();

    shared_ptr<DirectoryReader> *const r = w->getReader();
    const std::deque<int> idToDocID = std::deque<int>(r->maxDoc());
    for (int docID = 0; docID < idToDocID.size(); docID++) {
      int id = static_cast<Integer>(r->document(docID)->get(L"id"));
      if (aDocs->contains(id)) {
        aDocIDs.push_back(docID);
      } else {
        bDocIDs.push_back(docID);
      }
    }
    shared_ptr<TermsEnum> *const te =
        getOnlyLeafReader(r)->terms(L"field")->begin();

    shared_ptr<PostingsEnum> de = nullptr;
    for (int iter2 = 0; iter2 < 10; iter2++) {
      if (VERBOSE) {
        wcout << L"\nTEST: iter=" << iter << L" iter2=" << iter2 << endl;
      }
      TestUtil::assertEquals(TermsEnum::SeekStatus::FOUND,
                             te->seekCeil(make_shared<BytesRef>(L"a")));
      de = TestUtil::docs(random(), te, de, PostingsEnum::NONE);
      testOne(de, aDocIDs);

      TestUtil::assertEquals(TermsEnum::SeekStatus::FOUND,
                             te->seekCeil(make_shared<BytesRef>(L"b")));
      de = TestUtil::docs(random(), te, de, PostingsEnum::NONE);
      testOne(de, bDocIDs);
    }

    delete w;
    r->close();
    delete dir;
  }
}

void TestStressAdvance::testOne(shared_ptr<PostingsEnum> docs,
                                deque<int> &expected) 
{
  if (VERBOSE) {
    wcout << L"test" << endl;
  }
  int upto = -1;
  while (upto < expected.size()) {
    if (VERBOSE) {
      wcout << L"  cycle upto=" << upto << L" of " << expected.size() << endl;
    }
    constexpr int docID;
    if (random()->nextInt(4) == 1 || upto == expected.size() - 1) {
      // test nextDoc()
      if (VERBOSE) {
        wcout << L"    do nextDoc" << endl;
      }
      upto++;
      docID = docs->nextDoc();
    } else {
      // test advance()
      constexpr int inc =
          TestUtil::nextInt(random(), 1, expected.size() - 1 - upto);
      if (VERBOSE) {
        wcout << L"    do advance inc=" << inc << endl;
      }
      upto += inc;
      docID = docs->advance(expected[upto]);
    }
    if (upto == expected.size()) {
      if (VERBOSE) {
        wcout << L"  expect docID=" << DocIdSetIterator::NO_MORE_DOCS
              << L" actual=" << docID << endl;
      }
      TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS, docID);
    } else {
      if (VERBOSE) {
        wcout << L"  expect docID=" << expected[upto] << L" actual=" << docID
              << endl;
      }
      assertTrue(docID != DocIdSetIterator::NO_MORE_DOCS);
      TestUtil::assertEquals(expected[upto], docID);
    }
  }
}
} // namespace org::apache::lucene::index