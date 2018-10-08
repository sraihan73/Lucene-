using namespace std;

#include "TestTerms.h"

namespace org::apache::lucene::index
{
using CannedBinaryTokenStream =
    org::apache::lucene::analysis::CannedBinaryTokenStream;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using TextField = org::apache::lucene::document::TextField;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestTerms::testTermMinMaxBasic() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newTextField(L"field", L"a b c cc ddd", Field::Store::NO));
  w->addDocument(doc);
  shared_ptr<IndexReader> r = w->getReader();
  shared_ptr<Terms> terms = MultiFields::getTerms(r, L"field");
  TestUtil::assertEquals(make_shared<BytesRef>(L"a"), terms->getMin());
  TestUtil::assertEquals(make_shared<BytesRef>(L"ddd"), terms->getMax());
  delete r;
  delete w;
  delete dir;
}

void TestTerms::testTermMinMaxRandom() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir);
  int numDocs = atLeast(100);
  shared_ptr<BytesRef> minTerm = nullptr;
  shared_ptr<BytesRef> maxTerm = nullptr;
  for (int i = 0; i < numDocs; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    shared_ptr<Field> field =
        make_shared<TextField>(L"field", L"", Field::Store::NO);
    doc->push_back(field);
    // System.out.println("  doc " + i);
    std::deque<std::shared_ptr<CannedBinaryTokenStream::BinaryToken>> tokens(
        atLeast(10));
    for (int j = 0; j < tokens.size(); j++) {
      std::deque<char> bytes(TestUtil::nextInt(random(), 1, 20));
      random()->nextBytes(bytes);
      shared_ptr<BytesRef> tokenBytes = make_shared<BytesRef>(bytes);
      // System.out.println("    token " + tokenBytes);
      if (minTerm == nullptr || tokenBytes->compareTo(minTerm) < 0) {
        // System.out.println("      ** new min");
        minTerm = tokenBytes;
      }
      if (maxTerm == nullptr || tokenBytes->compareTo(maxTerm) > 0) {
        // System.out.println("      ** new max");
        maxTerm = tokenBytes;
      }
      tokens[j] = make_shared<CannedBinaryTokenStream::BinaryToken>(tokenBytes);
    }
    field->setTokenStream(make_shared<CannedBinaryTokenStream>(tokens));
    w->addDocument(doc);
  }

  shared_ptr<IndexReader> r = w->getReader();
  shared_ptr<Terms> terms = MultiFields::getTerms(r, L"field");
  TestUtil::assertEquals(minTerm, terms->getMin());
  TestUtil::assertEquals(maxTerm, terms->getMax());

  delete r;
  delete w;
  delete dir;
}
} // namespace org::apache::lucene::index