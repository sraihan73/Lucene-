using namespace std;

#include "TestExceedMaxTermLength.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;
using org::junit::After;
using org::junit::Before;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Before public void createDir()
void TestExceedMaxTermLength::createDir() { dir = newDirectory(); }

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @After public void destroyDir() throws java.io.IOException
void TestExceedMaxTermLength::destroyDir() 
{
  delete dir;
  dir.reset();
}

void TestExceedMaxTermLength::test() 
{

  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(random(), make_shared<MockAnalyzer>(random())));
  try {
    shared_ptr<FieldType> *const ft = make_shared<FieldType>();
    ft->setIndexOptions(IndexOptions::DOCS_AND_FREQS_AND_POSITIONS);
    ft->setStored(random()->nextBoolean());
    ft->freeze();

    shared_ptr<Document> *const doc = make_shared<Document>();
    if (random()->nextBoolean()) {
      // totally ok short field value
      doc->push_back(make_shared<Field>(
          TestUtil::randomSimpleString(random(), 1, 10),
          TestUtil::randomSimpleString(random(), 1, 10), ft));
    }
    // problematic field
    const wstring name = TestUtil::randomSimpleString(random(), 1, 50);
    const wstring value = TestUtil::randomSimpleString(
        random(), minTestTermLength, maxTestTermLegnth);
    shared_ptr<Field> *const f = make_shared<Field>(name, value, ft);
    if (random()->nextBoolean()) {
      // totally ok short field value
      doc->push_back(make_shared<Field>(
          TestUtil::randomSimpleString(random(), 1, 10),
          TestUtil::randomSimpleString(random(), 1, 10), ft));
    }
    doc->push_back(f);

    invalid_argument expected =
        expectThrows(invalid_argument::typeid, [&]() { w->addDocument(doc); });
    wstring maxLengthMsg = to_wstring(IndexWriter::MAX_TERM_LENGTH);
    wstring msg = expected.what();
    assertTrue(L"IllegalArgumentException didn't mention 'immense term': " +
                   msg,
               msg.find(L"immense term") != wstring::npos);
    assertTrue(L"IllegalArgumentException didn't mention max length (" +
                   maxLengthMsg + L"): " + msg,
               msg.find(maxLengthMsg) != wstring::npos);
    assertTrue(L"IllegalArgumentException didn't mention field name (" + name +
                   L"): " + msg,
               msg.find(name) != wstring::npos);
    assertTrue(L"IllegalArgumentException didn't mention original message: " +
                   msg,
               msg.find(L"bytes can be at most") != wstring::npos &&
                   msg.find(L"in length; got") != wstring::npos);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    delete w;
  }
}
} // namespace org::apache::lucene::index