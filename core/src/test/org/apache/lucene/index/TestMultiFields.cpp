using namespace std;

#include "TestMultiFields.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using Directory = org::apache::lucene::store::Directory;
using Bits = org::apache::lucene::util::Bits;
using BytesRef = org::apache::lucene::util::BytesRef;
using IOSupplier = org::apache::lucene::util::IOSupplier;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;
using UnicodeUtil = org::apache::lucene::util::UnicodeUtil;

void TestMultiFields::testRandom() 
{

  int num = atLeast(2);
  for (int iter = 0; iter < num; iter++) {
    if (VERBOSE) {
      wcout << L"TEST: iter=" << iter << endl;
    }

    shared_ptr<Directory> dir = newDirectory();

    shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
        dir,
        newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
            ->setMergePolicy(make_shared<FilterMergePolicyAnonymousInnerClass>(
                shared_from_this(), NoMergePolicy::INSTANCE)));
    unordered_map<std::shared_ptr<BytesRef>, deque<int>> docs =
        unordered_map<std::shared_ptr<BytesRef>, deque<int>>();
    shared_ptr<Set<int>> deleted = unordered_set<int>();
    deque<std::shared_ptr<BytesRef>> terms =
        deque<std::shared_ptr<BytesRef>>();

    int numDocs = TestUtil::nextInt(random(), 1, 100 * RANDOM_MULTIPLIER);
    shared_ptr<Document> doc = make_shared<Document>();
    shared_ptr<Field> f = newStringField(L"field", L"", Field::Store::NO);
    doc->push_back(f);
    shared_ptr<Field> id = newStringField(L"id", L"", Field::Store::NO);
    doc->push_back(id);

    bool onlyUniqueTerms = random()->nextBoolean();
    if (VERBOSE) {
      wcout << L"TEST: onlyUniqueTerms=" << onlyUniqueTerms << L" numDocs="
            << numDocs << endl;
    }
    shared_ptr<Set<std::shared_ptr<BytesRef>>> uniqueTerms =
        unordered_set<std::shared_ptr<BytesRef>>();
    for (int i = 0; i < numDocs; i++) {

      if (!onlyUniqueTerms && random()->nextBoolean() && terms.size() > 0) {
        // re-use existing term
        shared_ptr<BytesRef> term = terms[random()->nextInt(terms.size())];
        docs[term].push_back(i);
        f->setStringValue(term->utf8ToString());
      } else {
        wstring s = TestUtil::randomUnicodeString(random(), 10);
        shared_ptr<BytesRef> term = make_shared<BytesRef>(s);
        if (docs.find(term) == docs.end()) {
          docs.emplace(term, deque<int>());
        }
        docs[term].push_back(i);
        terms.push_back(term);
        uniqueTerms->add(term);
        f->setStringValue(s);
      }
      id->setStringValue(L"" + to_wstring(i));
      w->addDocument(doc);
      if (random()->nextInt(4) == 1) {
        w->commit();
      }
      if (i > 0 && random()->nextInt(20) == 1) {
        int delID = random()->nextInt(i);
        deleted->add(delID);
        w->deleteDocuments({make_shared<Term>(L"id", L"" + to_wstring(delID))});
        if (VERBOSE) {
          wcout << L"TEST: delete " << delID << endl;
        }
      }
    }

    if (VERBOSE) {
      deque<std::shared_ptr<BytesRef>> termsList =
          deque<std::shared_ptr<BytesRef>>(uniqueTerms);
      sort(termsList.begin(), termsList.end());
      wcout << L"TEST: terms in UTF-8 order:" << endl;
      for (auto b : termsList) {
        wcout << L"  " << UnicodeUtil::toHexString(b->utf8ToString()) << L" "
              << b << endl;
        for (auto docID : docs[b]) {
          if (deleted->contains(docID)) {
            wcout << L"    " << docID << L" (deleted)" << endl;
          } else {
            wcout << L"    " << docID << endl;
          }
        }
      }
    }

    shared_ptr<IndexReader> reader = w->getReader();
    delete w;
    if (VERBOSE) {
      wcout << L"TEST: reader=" << reader << endl;
    }

    shared_ptr<Bits> liveDocs = MultiFields::getLiveDocs(reader);
    for (auto delDoc : deleted) {
      assertFalse(liveDocs->get(delDoc));
    }

    for (int i = 0; i < 100; i++) {
      shared_ptr<BytesRef> term = terms[random()->nextInt(terms.size())];
      if (VERBOSE) {
        wcout << L"TEST: seek term="
              << UnicodeUtil::toHexString(term->utf8ToString()) << L" " << term
              << endl;
      }

      shared_ptr<PostingsEnum> postingsEnum = TestUtil::docs(
          random(), reader, L"field", term, nullptr, PostingsEnum::NONE);
      assertNotNull(postingsEnum);

      for (auto docID : docs[term]) {
        TestUtil::assertEquals(docID, postingsEnum->nextDoc());
      }
      TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS,
                             postingsEnum->nextDoc());
    }

    delete reader;
    delete dir;
  }
}

TestMultiFields::FilterMergePolicyAnonymousInnerClass::
    FilterMergePolicyAnonymousInnerClass(
        shared_ptr<TestMultiFields> outerInstance,
        shared_ptr<org::apache::lucene::index::MergePolicy> INSTANCE)
    : FilterMergePolicy(INSTANCE)
{
  this->outerInstance = outerInstance;
}

bool TestMultiFields::FilterMergePolicyAnonymousInnerClass::
    keepFullyDeletedSegment(
        IOSupplier<std::shared_ptr<CodecReader>> readerIOSupplier)
{
  // we can do this because we use NoMergePolicy (and dont merge to "nothing")
  return true;
}

void TestMultiFields::testSeparateEnums() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  shared_ptr<Document> d = make_shared<Document>();
  d->push_back(newStringField(L"f", L"j", Field::Store::NO));
  w->addDocument(d);
  w->commit();
  w->addDocument(d);
  shared_ptr<IndexReader> r = w->getReader();
  delete w;
  shared_ptr<PostingsEnum> d1 =
      TestUtil::docs(random(), r, L"f", make_shared<BytesRef>(L"j"), nullptr,
                     PostingsEnum::NONE);
  shared_ptr<PostingsEnum> d2 =
      TestUtil::docs(random(), r, L"f", make_shared<BytesRef>(L"j"), nullptr,
                     PostingsEnum::NONE);
  TestUtil::assertEquals(0, d1->nextDoc());
  TestUtil::assertEquals(0, d2->nextDoc());
  delete r;
  delete dir;
}

void TestMultiFields::testTermDocsEnum() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  shared_ptr<Document> d = make_shared<Document>();
  d->push_back(newStringField(L"f", L"j", Field::Store::NO));
  w->addDocument(d);
  w->commit();
  w->addDocument(d);
  shared_ptr<IndexReader> r = w->getReader();
  delete w;
  shared_ptr<PostingsEnum> de =
      MultiFields::getTermDocsEnum(r, L"f", make_shared<BytesRef>(L"j"));
  TestUtil::assertEquals(0, de->nextDoc());
  TestUtil::assertEquals(1, de->nextDoc());
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS, de->nextDoc());
  delete r;
  delete dir;
}
} // namespace org::apache::lucene::index