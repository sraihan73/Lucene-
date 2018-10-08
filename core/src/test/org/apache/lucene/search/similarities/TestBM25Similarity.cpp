using namespace std;

#include "TestBM25Similarity.h"

namespace org::apache::lucene::search::similarities
{
using Document = org::apache::lucene::document::Document;
using Store = org::apache::lucene::document::Field::Store;
using TextField = org::apache::lucene::document::TextField;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using SegmentInfos = org::apache::lucene::index::SegmentInfos;
using Term = org::apache::lucene::index::Term;
using Explanation = org::apache::lucene::search::Explanation;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using TermQuery = org::apache::lucene::search::TermQuery;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using Version = org::apache::lucene::util::Version;

void TestBM25Similarity::testIllegalK1()
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    make_shared<BM25Similarity>(numeric_limits<float>::infinity(), 0.75f);
  });
  assertTrue(expected.what()->contains(L"illegal k1 value"));

  expected = expectThrows(invalid_argument::typeid,
                          [&]() { make_shared<BM25Similarity>(-1, 0.75f); });
  assertTrue(expected.what()->contains(L"illegal k1 value"));

  expected = expectThrows(invalid_argument::typeid,
                          [&]() { make_shared<BM25Similarity>(NAN, 0.75f); });
  assertTrue(expected.what()->contains(L"illegal k1 value"));
}

void TestBM25Similarity::testIllegalB()
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    make_shared<BM25Similarity>(1.2f, 2.0f);
  });
  assertTrue(expected.what()->contains(L"illegal b value"));

  expected = expectThrows(invalid_argument::typeid,
                          [&]() { make_shared<BM25Similarity>(1.2f, -1.0f); });
  assertTrue(expected.what()->contains(L"illegal b value"));

  expected = expectThrows(invalid_argument::typeid, [&]() {
    make_shared<BM25Similarity>(1.2f, numeric_limits<float>::infinity());
  });
  assertTrue(expected.what()->contains(L"illegal b value"));

  expected = expectThrows(invalid_argument::typeid,
                          [&]() { make_shared<BM25Similarity>(1.2f, NAN); });
  assertTrue(expected.what()->contains(L"illegal b value"));
}

void TestBM25Similarity::testLengthEncodingBackwardCompatibility() throw(
    IOException)
{
  shared_ptr<Similarity> similarity = make_shared<BM25Similarity>();
  for (auto indexCreatedVersionMajor :
       std::deque<int>{Version::LUCENE_6_0_0->major, Version::LATEST->major}) {
    for (auto length : std::deque<int>{
             1, 2,
             4}) { // these length values are encoded accurately on both cases
      shared_ptr<Directory> dir = newDirectory();
      // set the version on the directory
      (make_shared<SegmentInfos>(indexCreatedVersionMajor))->commit(dir);
      shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
          dir, newIndexWriterConfig()->setSimilarity(similarity));
      shared_ptr<Document> doc = make_shared<Document>();
      wstring value = IntStream::range(0, length)
                          .mapToObj([&](any i) { L"b"; })
                          .collect(Collectors::joining(L" "));
      doc->push_back(make_shared<TextField>(L"foo", value, Store::NO));
      w->addDocument(doc);
      shared_ptr<IndexReader> reader = DirectoryReader::open(w);
      shared_ptr<IndexSearcher> searcher = newSearcher(reader);
      searcher->setSimilarity(similarity);
      shared_ptr<Explanation> expl = searcher->explain(
          make_shared<TermQuery>(make_shared<Term>(L"foo", L"b")), 0);
      shared_ptr<Explanation> docLen = findExplanation(expl, L"fieldLength");
      assertNotNull(docLen);
      // C++ TODO: There is no native C++ equivalent to 'toString':
      assertEquals(docLen->toString(), length,
                   static_cast<int>(docLen->getValue()));
      delete w;
      delete reader;
      delete dir;
    }
  }
}

shared_ptr<Explanation>
TestBM25Similarity::findExplanation(shared_ptr<Explanation> expl,
                                    const wstring &text)
{
  if (expl->getDescription() == text) {
    return expl;
  } else {
    for (auto sub : expl->getDetails()) {
      shared_ptr<Explanation> match = findExplanation(sub, text);
      if (match != nullptr) {
        return match;
      }
    }
  }
  return nullptr;
}
} // namespace org::apache::lucene::search::similarities