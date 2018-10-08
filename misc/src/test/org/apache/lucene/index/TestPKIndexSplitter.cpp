using namespace std;

#include "TestPKIndexSplitter.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using OpenMode = org::apache::lucene::index::IndexWriterConfig::OpenMode;
using Directory = org::apache::lucene::store::Directory;
using Bits = org::apache::lucene::util::Bits;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestPKIndexSplitter::testSplit() 
{
  shared_ptr<NumberFormat> format = make_shared<DecimalFormat>(
      L"000000000", DecimalFormatSymbols::getInstance(Locale::ROOT));
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(
                                    random(), MockTokenizer::WHITESPACE, false))
               ->setOpenMode(OpenMode::CREATE)
               ->setMergePolicy(NoMergePolicy::INSTANCE));
  for (int x = 0; x < 11; x++) {
    shared_ptr<Document> doc = createDocument(x, L"1", 3, format);
    w->addDocument(doc);
    if (x % 3 == 0) {
      w->commit();
    }
  }
  for (int x = 11; x < 20; x++) {
    shared_ptr<Document> doc = createDocument(x, L"2", 3, format);
    w->addDocument(doc);
    if (x % 3 == 0) {
      w->commit();
    }
  }
  delete w;

  shared_ptr<Term> *const midTerm =
      make_shared<Term>(L"id", format->format(11));

  checkSplitting(dir, midTerm, 11, 9);

  // delete some documents
  w = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(
                                    random(), MockTokenizer::WHITESPACE, false))
               ->setOpenMode(OpenMode::APPEND)
               ->setMergePolicy(NoMergePolicy::INSTANCE));
  w->deleteDocuments({midTerm});
  w->deleteDocuments({make_shared<Term>(L"id", format->format(2))});
  delete w;

  checkSplitting(dir, midTerm, 10, 8);

  delete dir;
}

void TestPKIndexSplitter::checkSplitting(shared_ptr<Directory> dir,
                                         shared_ptr<Term> splitTerm,
                                         int leftCount,
                                         int rightCount) 
{
  shared_ptr<Directory> dir1 = newDirectory();
  shared_ptr<Directory> dir2 = newDirectory();
  shared_ptr<PKIndexSplitter> splitter = make_shared<PKIndexSplitter>(
      dir, dir1, dir2, splitTerm,
      newIndexWriterConfig(make_shared<MockAnalyzer>(random())),
      newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  splitter->split();

  shared_ptr<IndexReader> ir1 = DirectoryReader::open(dir1);
  shared_ptr<IndexReader> ir2 = DirectoryReader::open(dir2);
  assertEquals(leftCount, ir1->numDocs());
  assertEquals(rightCount, ir2->numDocs());

  checkContents(ir1, L"1");
  checkContents(ir2, L"2");

  delete ir1;
  delete ir2;

  delete dir1;
  delete dir2;
}

void TestPKIndexSplitter::checkContents(
    shared_ptr<IndexReader> ir, const wstring &indexname) 
{
  shared_ptr<Bits> *const liveDocs = MultiFields::getLiveDocs(ir);
  for (int i = 0; i < ir->maxDoc(); i++) {
    if (liveDocs == nullptr || liveDocs->get(i)) {
      assertEquals(indexname, ir->document(i)[L"indexname"]);
    }
  }
}

shared_ptr<Document>
TestPKIndexSplitter::createDocument(int n, const wstring &indexName,
                                    int numFields,
                                    shared_ptr<NumberFormat> format)
{
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  shared_ptr<Document> doc = make_shared<Document>();
  wstring id = format->format(n);
  doc->push_back(newStringField(L"id", id, Field::Store::YES));
  doc->push_back(newStringField(L"indexname", indexName, Field::Store::YES));
  sb->append(L"a");
  sb->append(n);
  doc->push_back(newTextField(L"field1", sb->toString(), Field::Store::YES));
  sb->append(L" b");
  sb->append(n);
  for (int i = 1; i < numFields; i++) {
    doc->push_back(newTextField(L"field" + to_wstring(i + 1), sb->toString(),
                                Field::Store::YES));
  }
  return doc;
}
} // namespace org::apache::lucene::index