using namespace std;

#include "TestSumDocFreq.h"

namespace org::apache::lucene::index
{
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestSumDocFreq::testSumDocFreq() 
{
  constexpr int numDocs = atLeast(500);

  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);

  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<Field> id = newStringField(L"id", L"", Field::Store::NO);
  shared_ptr<Field> field1 = newTextField(L"foo", L"", Field::Store::NO);
  shared_ptr<Field> field2 = newTextField(L"bar", L"", Field::Store::NO);
  doc->push_back(id);
  doc->push_back(field1);
  doc->push_back(field2);
  for (int i = 0; i < numDocs; i++) {
    id->setStringValue(L"" + to_wstring(i));
    wchar_t ch1 = static_cast<wchar_t>(TestUtil::nextInt(random(), L'a', L'z'));
    wchar_t ch2 = static_cast<wchar_t>(TestUtil::nextInt(random(), L'a', L'z'));
    field1->setStringValue(L"" + StringHelper::toString(ch1) + L" " +
                           StringHelper::toString(ch2));
    ch1 = static_cast<wchar_t>(TestUtil::nextInt(random(), L'a', L'z'));
    ch2 = static_cast<wchar_t>(TestUtil::nextInt(random(), L'a', L'z'));
    field2->setStringValue(L"" + StringHelper::toString(ch1) + L" " +
                           StringHelper::toString(ch2));
    writer->addDocument(doc);
  }

  shared_ptr<IndexReader> ir = writer->getReader();

  assertSumDocFreq(ir);
  delete ir;

  int numDeletions = atLeast(20);
  for (int i = 0; i < numDeletions; i++) {
    writer->deleteDocuments(
        make_shared<Term>(L"id", L"" + random()->nextInt(numDocs)));
  }
  writer->forceMerge(1);
  delete writer;

  ir = DirectoryReader::open(dir);
  assertSumDocFreq(ir);
  delete ir;
  delete dir;
}

void TestSumDocFreq::assertSumDocFreq(shared_ptr<IndexReader> ir) throw(
    runtime_error)
{
  // compute sumDocFreq across all fields
  shared_ptr<Fields> fields = MultiFields::getFields(ir);

  for (auto f : fields) {
    shared_ptr<Terms> terms = fields->terms(f);
    int64_t sumDocFreq = terms->getSumDocFreq();
    if (sumDocFreq == -1) {
      if (VERBOSE) {
        wcout << L"skipping field: " << f
              << L", codec does not support sumDocFreq" << endl;
      }
      continue;
    }

    int64_t computedSumDocFreq = 0;
    shared_ptr<TermsEnum> termsEnum = terms->begin();
    while (termsEnum->next() != nullptr) {
      computedSumDocFreq += termsEnum->docFreq();
    }
    TestUtil::assertEquals(computedSumDocFreq, sumDocFreq);
  }
}
} // namespace org::apache::lucene::index