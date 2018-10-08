using namespace std;

#include "TestMultiPassIndexSplitter.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestMultiPassIndexSplitter::setUp() 
{
  LuceneTestCase::setUp();
  dir = newDirectory();
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setMergePolicy(NoMergePolicy::INSTANCE));
  shared_ptr<Document> doc;
  for (int i = 0; i < NUM_DOCS; i++) {
    doc = make_shared<Document>();
    doc->push_back(
        newStringField(L"id", to_wstring(i) + L"", Field::Store::YES));
    doc->push_back(newTextField(L"f", to_wstring(i) + L" " + to_wstring(i),
                                Field::Store::YES));
    w->addDocument(doc);
    if (i % 3 == 0) {
      w->commit();
    }
  }
  w->commit();
  w->deleteDocuments(
      {make_shared<Term>(L"id", L"" + to_wstring(NUM_DOCS - 1))});
  delete w;
  input = DirectoryReader::open(dir);
}

void TestMultiPassIndexSplitter::tearDown() 
{
  delete input;
  delete dir;
  LuceneTestCase::tearDown();
}

void TestMultiPassIndexSplitter::testSplitRR() 
{
  shared_ptr<MultiPassIndexSplitter> splitter =
      make_shared<MultiPassIndexSplitter>();
  std::deque<std::shared_ptr<Directory>> dirs = {
      newDirectory(), newDirectory(), newDirectory()};
  splitter->split(input, dirs, false);
  shared_ptr<IndexReader> ir;
  ir = DirectoryReader::open(dirs[0]);
  assertTrue(ir->numDocs() - NUM_DOCS / 3 <= 1); // rounding error
  shared_ptr<Document> doc = ir->document(0);
  assertEquals(L"0", doc[L"id"]);
  shared_ptr<TermsEnum> te = MultiFields::getTerms(ir, L"id")->begin();
  assertEquals(TermsEnum::SeekStatus::NOT_FOUND,
               te->seekCeil(make_shared<BytesRef>(L"1")));
  assertNotSame(L"1", te->term()->utf8ToString());
  delete ir;
  ir = DirectoryReader::open(dirs[1]);
  assertTrue(ir->numDocs() - NUM_DOCS / 3 <= 1);
  doc = ir->document(0);
  assertEquals(L"1", doc[L"id"]);
  te = MultiFields::getTerms(ir, L"id")->begin();
  assertEquals(TermsEnum::SeekStatus::NOT_FOUND,
               te->seekCeil(make_shared<BytesRef>(L"0")));

  assertNotSame(L"0", te->term()->utf8ToString());
  delete ir;
  ir = DirectoryReader::open(dirs[2]);
  assertTrue(ir->numDocs() - NUM_DOCS / 3 <= 1);
  doc = ir->document(0);
  assertEquals(L"2", doc[L"id"]);

  te = MultiFields::getTerms(ir, L"id")->begin();
  assertEquals(TermsEnum::SeekStatus::NOT_FOUND,
               te->seekCeil(make_shared<BytesRef>(L"1")));
  assertNotSame(L"1", te->term());

  assertEquals(TermsEnum::SeekStatus::NOT_FOUND,
               te->seekCeil(make_shared<BytesRef>(L"0")));
  assertNotSame(L"0", te->term()->utf8ToString());
  delete ir;
  for (auto d : dirs) {
    d->close();
  }
}

void TestMultiPassIndexSplitter::testSplitSeq() 
{
  shared_ptr<MultiPassIndexSplitter> splitter =
      make_shared<MultiPassIndexSplitter>();
  std::deque<std::shared_ptr<Directory>> dirs = {
      newDirectory(), newDirectory(), newDirectory()};
  splitter->split(input, dirs, true);
  shared_ptr<IndexReader> ir;
  ir = DirectoryReader::open(dirs[0]);
  assertTrue(ir->numDocs() - NUM_DOCS / 3 <= 1);
  shared_ptr<Document> doc = ir->document(0);
  assertEquals(L"0", doc[L"id"]);
  int start = ir->numDocs();
  delete ir;
  ir = DirectoryReader::open(dirs[1]);
  assertTrue(ir->numDocs() - NUM_DOCS / 3 <= 1);
  doc = ir->document(0);
  assertEquals(to_wstring(start) + L"", doc[L"id"]);
  start += ir->numDocs();
  delete ir;
  ir = DirectoryReader::open(dirs[2]);
  assertTrue(ir->numDocs() - NUM_DOCS / 3 <= 1);
  doc = ir->document(0);
  assertEquals(to_wstring(start) + L"", doc[L"id"]);
  // make sure the deleted doc is not here
  shared_ptr<TermsEnum> te = MultiFields::getTerms(ir, L"id")->begin();
  shared_ptr<Term> t = make_shared<Term>(L"id", to_wstring(NUM_DOCS - 1) + L"");
  assertEquals(TermsEnum::SeekStatus::NOT_FOUND,
               te->seekCeil(make_shared<BytesRef>(t->text())));
  assertNotSame(t->text(), te->term()->utf8ToString());
  delete ir;
  for (auto d : dirs) {
    d->close();
  }
}
} // namespace org::apache::lucene::index