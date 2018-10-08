using namespace std;

#include "TestSortedSetFieldSource.h"

namespace org::apache::lucene::queries::function
{
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using SortedSetDocValuesField =
    org::apache::lucene::document::SortedSetDocValuesField;
using LeafReader = org::apache::lucene::index::LeafReader;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using SortedSetFieldSource =
    org::apache::lucene::queries::function::valuesource::SortedSetFieldSource;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using SortField = org::apache::lucene::search::SortField;
using SortedSetSortField = org::apache::lucene::search::SortedSetSortField;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestSortedSetFieldSource::testSimple() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> writer =
      make_shared<IndexWriter>(dir, newIndexWriterConfig(nullptr));
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"value", make_shared<BytesRef>(L"baz")));
  doc->push_back(newStringField(L"id", L"2", Field::Store::YES));
  writer->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"value", make_shared<BytesRef>(L"foo")));
  doc->push_back(make_shared<SortedSetDocValuesField>(
      L"value", make_shared<BytesRef>(L"bar")));
  doc->push_back(newStringField(L"id", L"1", Field::Store::YES));
  writer->addDocument(doc);
  writer->forceMerge(1);
  delete writer;

  shared_ptr<DirectoryReader> ir = DirectoryReader::open(dir);
  shared_ptr<IndexSearcher> searcher = newSearcher(ir);
  shared_ptr<LeafReader> ar = getOnlyLeafReader(ir);

  shared_ptr<ValueSource> vs = make_shared<SortedSetFieldSource>(L"value");
  shared_ptr<FunctionValues> values =
      vs->getValues(Collections::emptyMap(), ar->getContext());
  assertEquals(L"baz", values->strVal(0));
  assertEquals(L"bar", values->strVal(1));

  // test SortField optimization
  constexpr bool reverse = random()->nextBoolean();
  shared_ptr<SortField> vssf = vs->getSortField(reverse);
  shared_ptr<SortField> sf = make_shared<SortedSetSortField>(L"value", reverse);
  assertEquals(sf, vssf);

  vssf = vssf->rewrite(searcher);
  sf = sf->rewrite(searcher);
  assertEquals(sf, vssf);

  ir->close();
  delete dir;
}
} // namespace org::apache::lucene::queries::function