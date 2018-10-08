using namespace std;

#include "TestCustomTermFreq.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using TermFrequencyAttribute =
    org::apache::lucene::analysis::tokenattributes::TermFrequencyAttribute;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using TextField = org::apache::lucene::document::TextField;
using CollectionStatistics = org::apache::lucene::search::CollectionStatistics;
using TermStatistics = org::apache::lucene::search::TermStatistics;
using Similarity = org::apache::lucene::search::similarities::Similarity;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using IOUtils = org::apache::lucene::util::IOUtils;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
//    import static org.apache.lucene.index.PostingsEnum.NO_MORE_DOCS;

TestCustomTermFreq::CannedTermFreqs::CannedTermFreqs(
    std::deque<wstring> &terms, std::deque<int> &termFreqs)
    : terms(terms), termFreqs(termFreqs)
{
  assert(terms.size() == termFreqs.size());
}

bool TestCustomTermFreq::CannedTermFreqs::incrementToken()
{
  if (upto == terms.size()) {
    return false;
  }

  clearAttributes();

  termAtt->append(terms[upto]);
  termFreqAtt->setTermFrequency(termFreqs[upto]);

  upto++;
  return true;
}

void TestCustomTermFreq::CannedTermFreqs::reset() { upto = 0; }

void TestCustomTermFreq::testSingletonTermsOneDoc() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      dir, make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random())));

  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<FieldType> fieldType =
      make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  fieldType->setIndexOptions(IndexOptions::DOCS_AND_FREQS);
  shared_ptr<Field> field = make_shared<Field>(
      L"field",
      make_shared<CannedTermFreqs>(std::deque<wstring>{L"foo", L"bar"},
                                   std::deque<int>{42, 128}),
      fieldType);
  doc->push_back(field);
  w->addDocument(doc);
  shared_ptr<IndexReader> r = DirectoryReader::open(w);
  shared_ptr<PostingsEnum> postings =
      MultiFields::getTermDocsEnum(r, L"field", make_shared<BytesRef>(L"bar"));
  assertNotNull(postings);
  assertEquals(0, postings->nextDoc());
  assertEquals(128, postings->freq());
  assertEquals(NO_MORE_DOCS, postings->nextDoc());

  postings =
      MultiFields::getTermDocsEnum(r, L"field", make_shared<BytesRef>(L"foo"));
  assertNotNull(postings);
  assertEquals(0, postings->nextDoc());
  assertEquals(42, postings->freq());
  assertEquals(NO_MORE_DOCS, postings->nextDoc());

  IOUtils::close({r, w, dir});
}

void TestCustomTermFreq::testSingletonTermsTwoDocs() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      dir, make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random())));

  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<FieldType> fieldType =
      make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  fieldType->setIndexOptions(IndexOptions::DOCS_AND_FREQS);
  shared_ptr<Field> field = make_shared<Field>(
      L"field",
      make_shared<CannedTermFreqs>(std::deque<wstring>{L"foo", L"bar"},
                                   std::deque<int>{42, 128}),
      fieldType);
  doc->push_back(field);
  w->addDocument(doc);

  doc = make_shared<Document>();
  field = make_shared<Field>(
      L"field",
      make_shared<CannedTermFreqs>(std::deque<wstring>{L"foo", L"bar"},
                                   std::deque<int>{50, 50}),
      fieldType);
  doc->push_back(field);
  w->addDocument(doc);

  shared_ptr<IndexReader> r = DirectoryReader::open(w);
  shared_ptr<PostingsEnum> postings =
      MultiFields::getTermDocsEnum(r, L"field", make_shared<BytesRef>(L"bar"));
  assertNotNull(postings);
  assertEquals(0, postings->nextDoc());
  assertEquals(128, postings->freq());
  assertEquals(1, postings->nextDoc());
  assertEquals(50, postings->freq());
  assertEquals(NO_MORE_DOCS, postings->nextDoc());

  postings =
      MultiFields::getTermDocsEnum(r, L"field", make_shared<BytesRef>(L"foo"));
  assertNotNull(postings);
  assertEquals(0, postings->nextDoc());
  assertEquals(42, postings->freq());
  assertEquals(1, postings->nextDoc());
  assertEquals(50, postings->freq());
  assertEquals(NO_MORE_DOCS, postings->nextDoc());

  IOUtils::close({r, w, dir});
}

void TestCustomTermFreq::testRepeatTermsOneDoc() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      dir, make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random())));

  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<FieldType> fieldType =
      make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  fieldType->setIndexOptions(IndexOptions::DOCS_AND_FREQS);
  shared_ptr<Field> field = make_shared<Field>(
      L"field",
      make_shared<CannedTermFreqs>(
          std::deque<wstring>{L"foo", L"bar", L"foo", L"bar"},
          std::deque<int>{42, 128, 17, 100}),
      fieldType);
  doc->push_back(field);
  w->addDocument(doc);
  shared_ptr<IndexReader> r = DirectoryReader::open(w);
  shared_ptr<PostingsEnum> postings =
      MultiFields::getTermDocsEnum(r, L"field", make_shared<BytesRef>(L"bar"));
  assertNotNull(postings);
  assertEquals(0, postings->nextDoc());
  assertEquals(228, postings->freq());
  assertEquals(NO_MORE_DOCS, postings->nextDoc());

  postings =
      MultiFields::getTermDocsEnum(r, L"field", make_shared<BytesRef>(L"foo"));
  assertNotNull(postings);
  assertEquals(0, postings->nextDoc());
  assertEquals(59, postings->freq());
  assertEquals(NO_MORE_DOCS, postings->nextDoc());

  IOUtils::close({r, w, dir});
}

void TestCustomTermFreq::testRepeatTermsTwoDocs() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      dir, make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random())));

  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<FieldType> fieldType =
      make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  fieldType->setIndexOptions(IndexOptions::DOCS_AND_FREQS);
  shared_ptr<Field> field = make_shared<Field>(
      L"field",
      make_shared<CannedTermFreqs>(
          std::deque<wstring>{L"foo", L"bar", L"foo", L"bar"},
          std::deque<int>{42, 128, 17, 100}),
      fieldType);
  doc->push_back(field);
  w->addDocument(doc);

  doc = make_shared<Document>();
  fieldType->setIndexOptions(IndexOptions::DOCS_AND_FREQS);
  field = make_shared<Field>(
      L"field",
      make_shared<CannedTermFreqs>(
          std::deque<wstring>{L"foo", L"bar", L"foo", L"bar"},
          std::deque<int>{50, 60, 70, 80}),
      fieldType);
  doc->push_back(field);
  w->addDocument(doc);

  shared_ptr<IndexReader> r = DirectoryReader::open(w);
  shared_ptr<PostingsEnum> postings =
      MultiFields::getTermDocsEnum(r, L"field", make_shared<BytesRef>(L"bar"));
  assertNotNull(postings);
  assertEquals(0, postings->nextDoc());
  assertEquals(228, postings->freq());
  assertEquals(1, postings->nextDoc());
  assertEquals(140, postings->freq());
  assertEquals(NO_MORE_DOCS, postings->nextDoc());

  postings =
      MultiFields::getTermDocsEnum(r, L"field", make_shared<BytesRef>(L"foo"));
  assertNotNull(postings);
  assertEquals(0, postings->nextDoc());
  assertEquals(59, postings->freq());
  assertEquals(1, postings->nextDoc());
  assertEquals(120, postings->freq());
  assertEquals(NO_MORE_DOCS, postings->nextDoc());

  IOUtils::close({r, w, dir});
}

void TestCustomTermFreq::testTotalTermFreq() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      dir, make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random())));

  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<FieldType> fieldType =
      make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  fieldType->setIndexOptions(IndexOptions::DOCS_AND_FREQS);
  shared_ptr<Field> field = make_shared<Field>(
      L"field",
      make_shared<CannedTermFreqs>(
          std::deque<wstring>{L"foo", L"bar", L"foo", L"bar"},
          std::deque<int>{42, 128, 17, 100}),
      fieldType);
  doc->push_back(field);
  w->addDocument(doc);

  doc = make_shared<Document>();
  fieldType->setIndexOptions(IndexOptions::DOCS_AND_FREQS);
  field = make_shared<Field>(
      L"field",
      make_shared<CannedTermFreqs>(
          std::deque<wstring>{L"foo", L"bar", L"foo", L"bar"},
          std::deque<int>{50, 60, 70, 80}),
      fieldType);
  doc->push_back(field);
  w->addDocument(doc);

  shared_ptr<IndexReader> r = DirectoryReader::open(w);

  shared_ptr<TermsEnum> termsEnum = MultiFields::getTerms(r, L"field")->begin();
  assertTrue(termsEnum->seekExact(make_shared<BytesRef>(L"foo")));
  assertEquals(179, termsEnum->totalTermFreq());
  assertTrue(termsEnum->seekExact(make_shared<BytesRef>(L"bar")));
  assertEquals(368, termsEnum->totalTermFreq());

  IOUtils::close({r, w, dir});
}

void TestCustomTermFreq::testInvalidProx() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      dir, make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random())));

  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<FieldType> fieldType =
      make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  shared_ptr<Field> field = make_shared<Field>(
      L"field",
      make_shared<CannedTermFreqs>(
          std::deque<wstring>{L"foo", L"bar", L"foo", L"bar"},
          std::deque<int>{42, 128, 17, 100}),
      fieldType);
  doc->push_back(field);
  runtime_error e = expectThrows(IllegalStateException::typeid,
                                 [&]() { w->addDocument(doc); });
  assertEquals(L"field \"field\": cannot index positions while using custom "
               L"TermFrequencyAttribute",
               e.what());
  IOUtils::close({w, dir});
}

void TestCustomTermFreq::testInvalidDocsOnly() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      dir, make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random())));

  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<FieldType> fieldType =
      make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  fieldType->setIndexOptions(IndexOptions::DOCS);
  shared_ptr<Field> field = make_shared<Field>(
      L"field",
      make_shared<CannedTermFreqs>(
          std::deque<wstring>{L"foo", L"bar", L"foo", L"bar"},
          std::deque<int>{42, 128, 17, 100}),
      fieldType);
  doc->push_back(field);
  runtime_error e = expectThrows(IllegalStateException::typeid,
                                 [&]() { w->addDocument(doc); });
  assertEquals(L"field \"field\": must index term freq while using custom "
               L"TermFrequencyAttribute",
               e.what());
  IOUtils::close({w, dir});
}

void TestCustomTermFreq::testOverflowInt() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      dir, make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random())));

  shared_ptr<FieldType> fieldType =
      make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  fieldType->setIndexOptions(IndexOptions::DOCS);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(
      make_shared<Field>(L"field", L"this field should be indexed", fieldType));
  w->addDocument(doc);

  shared_ptr<Document> doc2 = make_shared<Document>();
  shared_ptr<Field> field =
      make_shared<Field>(L"field",
                         make_shared<CannedTermFreqs>(
                             std::deque<wstring>{L"foo", L"bar"},
                             std::deque<int>{3, numeric_limits<int>::max()}),
                         fieldType);
  doc2->push_back(field);
  expectThrows(invalid_argument::typeid, [&]() { w->addDocument(doc2); });

  shared_ptr<IndexReader> r = DirectoryReader::open(w);
  assertEquals(1, r->numDocs());

  IOUtils::close({r, w, dir});
}

void TestCustomTermFreq::testInvalidTermVectorPositions() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      dir, make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random())));

  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<FieldType> fieldType =
      make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  fieldType->setIndexOptions(IndexOptions::DOCS_AND_FREQS);
  fieldType->setStoreTermVectors(true);
  fieldType->setStoreTermVectorPositions(true);
  shared_ptr<Field> field = make_shared<Field>(
      L"field",
      make_shared<CannedTermFreqs>(
          std::deque<wstring>{L"foo", L"bar", L"foo", L"bar"},
          std::deque<int>{42, 128, 17, 100}),
      fieldType);
  doc->push_back(field);
  runtime_error e =
      expectThrows(invalid_argument::typeid, [&]() { w->addDocument(doc); });
  assertEquals(L"field \"field\": cannot index term deque positions while "
               L"using custom TermFrequencyAttribute",
               e.what());
  IOUtils::close({w, dir});
}

void TestCustomTermFreq::testInvalidTermVectorOffsets() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      dir, make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random())));

  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<FieldType> fieldType =
      make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  fieldType->setIndexOptions(IndexOptions::DOCS_AND_FREQS);
  fieldType->setStoreTermVectors(true);
  fieldType->setStoreTermVectorOffsets(true);
  shared_ptr<Field> field = make_shared<Field>(
      L"field",
      make_shared<CannedTermFreqs>(
          std::deque<wstring>{L"foo", L"bar", L"foo", L"bar"},
          std::deque<int>{42, 128, 17, 100}),
      fieldType);
  doc->push_back(field);
  runtime_error e =
      expectThrows(invalid_argument::typeid, [&]() { w->addDocument(doc); });
  assertEquals(L"field \"field\": cannot index term deque offsets while using "
               L"custom TermFrequencyAttribute",
               e.what());
  IOUtils::close({w, dir});
}

void TestCustomTermFreq::testTermVectors() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      dir, make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random())));

  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<FieldType> fieldType =
      make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  fieldType->setIndexOptions(IndexOptions::DOCS_AND_FREQS);
  fieldType->setStoreTermVectors(true);
  shared_ptr<Field> field = make_shared<Field>(
      L"field",
      make_shared<CannedTermFreqs>(
          std::deque<wstring>{L"foo", L"bar", L"foo", L"bar"},
          std::deque<int>{42, 128, 17, 100}),
      fieldType);
  doc->push_back(field);
  w->addDocument(doc);

  doc = make_shared<Document>();
  fieldType->setIndexOptions(IndexOptions::DOCS_AND_FREQS);
  field = make_shared<Field>(
      L"field",
      make_shared<CannedTermFreqs>(
          std::deque<wstring>{L"foo", L"bar", L"foo", L"bar"},
          std::deque<int>{50, 60, 70, 80}),
      fieldType);
  doc->push_back(field);
  w->addDocument(doc);

  shared_ptr<IndexReader> r = DirectoryReader::open(w);

  shared_ptr<Fields> fields = r->getTermVectors(0);
  shared_ptr<TermsEnum> termsEnum = fields->terms(L"field")->begin();
  assertTrue(termsEnum->seekExact(make_shared<BytesRef>(L"bar")));
  assertEquals(228, termsEnum->totalTermFreq());
  shared_ptr<PostingsEnum> postings = termsEnum->postings(nullptr);
  assertNotNull(postings);
  assertEquals(0, postings->nextDoc());
  assertEquals(228, postings->freq());
  assertEquals(NO_MORE_DOCS, postings->nextDoc());

  assertTrue(termsEnum->seekExact(make_shared<BytesRef>(L"foo")));
  assertEquals(59, termsEnum->totalTermFreq());
  postings = termsEnum->postings(nullptr);
  assertNotNull(postings);
  assertEquals(0, postings->nextDoc());
  assertEquals(59, postings->freq());
  assertEquals(NO_MORE_DOCS, postings->nextDoc());

  fields = r->getTermVectors(1);
  termsEnum = fields->terms(L"field")->begin();
  assertTrue(termsEnum->seekExact(make_shared<BytesRef>(L"bar")));
  assertEquals(140, termsEnum->totalTermFreq());
  postings = termsEnum->postings(nullptr);
  assertNotNull(postings);
  assertEquals(0, postings->nextDoc());
  assertEquals(140, postings->freq());
  assertEquals(NO_MORE_DOCS, postings->nextDoc());

  assertTrue(termsEnum->seekExact(make_shared<BytesRef>(L"foo")));
  assertEquals(120, termsEnum->totalTermFreq());
  postings = termsEnum->postings(nullptr);
  assertNotNull(postings);
  assertEquals(0, postings->nextDoc());
  assertEquals(120, postings->freq());
  assertEquals(NO_MORE_DOCS, postings->nextDoc());

  IOUtils::close({r, w, dir});
}

const shared_ptr<NeverForgetsSimilarity>
    TestCustomTermFreq::NeverForgetsSimilarity::INSTANCE =
        make_shared<NeverForgetsSimilarity>();

TestCustomTermFreq::NeverForgetsSimilarity::NeverForgetsSimilarity()
{
  // no
}

int64_t TestCustomTermFreq::NeverForgetsSimilarity::computeNorm(
    shared_ptr<FieldInvertState> state)
{
  this->lastState = state;
  return 1;
}

shared_ptr<Similarity::SimWeight>
TestCustomTermFreq::NeverForgetsSimilarity::computeWeight(
    float boost, shared_ptr<CollectionStatistics> collectionStats,
    deque<TermStatistics> &termStats)
{
  throw make_shared<UnsupportedOperationException>();
}

shared_ptr<Similarity::SimScorer>
TestCustomTermFreq::NeverForgetsSimilarity::simScorer(
    shared_ptr<Similarity::SimWeight> weight,
    shared_ptr<LeafReaderContext> context) 
{
  throw make_shared<UnsupportedOperationException>();
}

void TestCustomTermFreq::testFieldInvertState() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  iwc->setSimilarity(NeverForgetsSimilarity::INSTANCE);
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);

  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<FieldType> fieldType =
      make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  fieldType->setIndexOptions(IndexOptions::DOCS_AND_FREQS);
  shared_ptr<Field> field = make_shared<Field>(
      L"field",
      make_shared<CannedTermFreqs>(
          std::deque<wstring>{L"foo", L"bar", L"foo", L"bar"},
          std::deque<int>{42, 128, 17, 100}),
      fieldType);
  doc->push_back(field);
  w->addDocument(doc);
  shared_ptr<FieldInvertState> fis =
      NeverForgetsSimilarity::INSTANCE::lastState;
  assertEquals(228, fis->getMaxTermFrequency());
  assertEquals(2, fis->getUniqueTermCount());
  assertEquals(0, fis->getNumOverlap());
  assertEquals(287, fis->getLength());

  IOUtils::close({w, dir});
}
} // namespace org::apache::lucene::index