using namespace std;

#include "TestFloatPointNearestNeighbor.h"

namespace org::apache::lucene::document
{
using Codec = org::apache::lucene::codecs::Codec;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using PointValues = org::apache::lucene::index::PointValues;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using SerialMergeScheduler = org::apache::lucene::index::SerialMergeScheduler;
using Term = org::apache::lucene::index::Term;
using FieldDoc = org::apache::lucene::search::FieldDoc;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using ScoreDoc = org::apache::lucene::search::ScoreDoc;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestFloatPointNearestNeighbor::testNearestNeighborWithDeletedDocs() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir, getIndexWriterConfig());
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<FloatPoint>(L"point", 40.0f, 50.0f));
  doc->push_back(make_shared<StringField>(L"id", L"0", Field::Store::YES));
  w->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(make_shared<FloatPoint>(L"point", 45.0f, 55.0f));
  doc->push_back(make_shared<StringField>(L"id", L"1", Field::Store::YES));
  w->addDocument(doc);

  shared_ptr<DirectoryReader> r = w->getReader();
  // can't wrap because we require Lucene60PointsFormat directly but e.g.
  // ParallelReader wraps with its own points impl:
  shared_ptr<IndexSearcher> s = newSearcher(r, false);
  shared_ptr<FieldDoc> hit = std::static_pointer_cast<FieldDoc>(
      FloatPointNearestNeighbor::nearest(s, L"point", 1, {40.0f, 50.0f})
          ->scoreDocs[0]);
  TestUtil::assertEquals(L"0",
                         r->document(hit->doc).getField(L"id").stringValue());
  r->close();

  w->deleteDocuments(make_shared<Term>(L"id", L"0"));
  r = w->getReader();
  // can't wrap because we require Lucene60PointsFormat directly but e.g.
  // ParallelReader wraps with its own points impl:
  s = newSearcher(r, false);
  hit = std::static_pointer_cast<FieldDoc>(
      LatLonPoint::nearest(s, L"point", 40.0, 50.0, 1)->scoreDocs[0]);
  TestUtil::assertEquals(L"1",
                         r->document(hit->doc).getField(L"id").stringValue());
  r->close();
  delete w;
  delete dir;
}

void TestFloatPointNearestNeighbor::
    testNearestNeighborWithAllDeletedDocs() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir, getIndexWriterConfig());
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<FloatPoint>(L"point", 40.0f, 50.0f));
  doc->push_back(make_shared<StringField>(L"id", L"0", Field::Store::YES));
  w->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<FloatPoint>(L"point", 45.0f, 55.0f));
  doc->push_back(make_shared<StringField>(L"id", L"1", Field::Store::YES));
  w->addDocument(doc);

  shared_ptr<DirectoryReader> r = w->getReader();
  // can't wrap because we require Lucene60PointsFormat directly but e.g.
  // ParallelReader wraps with its own points impl:
  shared_ptr<IndexSearcher> s = newSearcher(r, false);
  shared_ptr<FieldDoc> hit = std::static_pointer_cast<FieldDoc>(
      FloatPointNearestNeighbor::nearest(s, L"point", 1, {40.0f, 50.0f})
          ->scoreDocs[0]);
  TestUtil::assertEquals(L"0",
                         r->document(hit->doc).getField(L"id").stringValue());
  r->close();

  w->deleteDocuments(make_shared<Term>(L"id", L"0"));
  w->deleteDocuments(make_shared<Term>(L"id", L"1"));
  r = w->getReader();
  // can't wrap because we require Lucene60PointsFormat directly but e.g.
  // ParallelReader wraps with its own points impl:
  s = newSearcher(r, false);
  TestUtil::assertEquals(
      0, FloatPointNearestNeighbor::nearest(s, L"point", 1, {40.0f, 50.0f})
             ->scoreDocs.length);
  r->close();
  delete w;
  delete dir;
}

void TestFloatPointNearestNeighbor::testTieBreakByDocID() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> w =
      make_shared<IndexWriter>(dir, getIndexWriterConfig());
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<FloatPoint>(L"point", 40.0f, 50.0f));
  doc->push_back(make_shared<StringField>(L"id", L"0", Field::Store::YES));
  w->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<FloatPoint>(L"point", 40.0f, 50.0f));
  doc->push_back(make_shared<StringField>(L"id", L"1", Field::Store::YES));
  w->addDocument(doc);

  shared_ptr<DirectoryReader> r = DirectoryReader::open(w);
  // can't wrap because we require Lucene60PointsFormat directly but e.g.
  // ParallelReader wraps with its own points impl:
  std::deque<std::shared_ptr<ScoreDoc>> hits =
      FloatPointNearestNeighbor::nearest(newSearcher(r, false), L"point", 2,
                                         {45.0f, 50.0f})
          ->scoreDocs;
  TestUtil::assertEquals(
      L"0", r->document(hits[0]->doc).getField(L"id").stringValue());
  TestUtil::assertEquals(
      L"1", r->document(hits[1]->doc).getField(L"id").stringValue());

  r->close();
  delete w;
  delete dir;
}

void TestFloatPointNearestNeighbor::testNearestNeighborWithNoDocs() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir, getIndexWriterConfig());
  shared_ptr<DirectoryReader> r = w->getReader();
  // can't wrap because we require Lucene60PointsFormat directly but e.g.
  // ParallelReader wraps with its own points impl:
  TestUtil::assertEquals(
      0, FloatPointNearestNeighbor::nearest(newSearcher(r, false), L"point", 1,
                                            {40.0f, 50.0f})
             ->scoreDocs.length);
  r->close();
  delete w;
  delete dir;
}

void TestFloatPointNearestNeighbor::testNearestNeighborRandom() throw(
    runtime_error)
{
  shared_ptr<Directory> dir;
  int numPoints = atLeast(5000);
  if (numPoints > 100000) {
    dir = newFSDirectory(createTempDir(getClass().getSimpleName()));
  } else {
    dir = newDirectory();
  }
  shared_ptr<IndexWriterConfig> iwc = getIndexWriterConfig();
  iwc->setMergePolicy(newLogMergePolicy());
  iwc->setMergeScheduler(make_shared<SerialMergeScheduler>());
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir, iwc);

  int dims = TestUtil::nextInt(random(), 1, PointValues::MAX_DIMENSIONS);
  // C++ NOTE: The following call to the 'RectangularVectors' helper class
  // reproduces the rectangular array initialization that is automatic in Java:
  // ORIGINAL LINE: float[][] values = new float[numPoints][dims];
  std::deque<std::deque<float>> values =
      RectangularVectors::ReturnRectangularFloatVector(numPoints, dims);
  for (int id = 0; id < numPoints; ++id) {
    for (int dim = 0; dim < dims; ++dim) {
      optional<float> f = NAN;
      while (isnan(f)) {
        f = Float::intBitsToFloat(random()->nextInt());
      }
      values[id][dim] = f;
    }
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(make_shared<FloatPoint>(L"point", values[id]));
    doc->push_back(make_shared<StoredField>(L"id", id));
    w->addDocument(doc);
  }

  if (random()->nextBoolean()) {
    w->forceMerge(1);
  }

  shared_ptr<DirectoryReader> r = w->getReader();
  if (VERBOSE) {
    wcout << L"TEST: reader=" << r << endl;
  }
  // can't wrap because we require Lucene60PointsFormat directly but e.g.
  // ParallelReader wraps with its own points impl:
  shared_ptr<IndexSearcher> s = newSearcher(r, false);
  int iters = atLeast(100);
  for (int iter = 0; iter < iters; ++iter) {
    if (VERBOSE) {
      wcout << L"\nTEST: iter=" << iter << endl;
    }
    std::deque<float> origin(dims);
    for (int dim = 0; dim < dims; ++dim) {
      optional<float> f = NAN;
      while (isnan(f)) {
        f = Float::intBitsToFloat(random()->nextInt());
      }
      origin[dim] = f;
    }

    // dumb brute force search to get the expected result:
    std::deque<std::shared_ptr<FloatPointNearestNeighbor::NearestHit>>
        expectedHits(numPoints);
    for (int id = 0; id < numPoints; ++id) {
      shared_ptr<FloatPointNearestNeighbor::NearestHit> hit =
          make_shared<FloatPointNearestNeighbor::NearestHit>();
      hit->distanceSquared =
          FloatPointNearestNeighbor::euclideanDistanceSquared(origin,
                                                              values[id]);
      hit->docID = id;
      expectedHits[id] = hit;
    }

    Arrays::sort(expectedHits, [&](a, b) {
      int cmp = Double::compare(a::distanceSquared, b::distanceSquared);
      return cmp != 0 ? cmp : a::docID - b::docID;
    });

    int topK = TestUtil::nextInt(random(), 1, numPoints);

    if (VERBOSE) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      wcout << L"\nhits for origin=" << Arrays->toString(origin) << endl;
    }

    std::deque<std::shared_ptr<ScoreDoc>> hits =
        FloatPointNearestNeighbor::nearest(s, L"point", topK, origin)
            ->scoreDocs;
    assertEquals(L"fewer than expected hits: ", topK, hits.size());

    if (VERBOSE) {
      for (int i = 0; i < topK; ++i) {
        shared_ptr<FloatPointNearestNeighbor::NearestHit> expected =
            expectedHits[i];
        shared_ptr<FieldDoc> actual =
            std::static_pointer_cast<FieldDoc>(hits[i]);
        shared_ptr<Document> actualDoc = r->document(actual->doc);
        wcout << L"hit " << i << endl;
        // C++ TODO: There is no native C++ equivalent to 'toString':
        wcout << L"  expected id=" << expected->docID << L"  "
              << Arrays->toString(values[expected->docID]) << L"  distance="
              << static_cast<float>(sqrt(expected->distanceSquared))
              << L"  distanceSquared=" << expected->distanceSquared << endl;
        wcout << L"  actual id=" << actualDoc->getField(L"id") << L" distance="
              << actual->fields[0] << endl;
      }
    }

    for (int i = 0; i < topK; ++i) {
      shared_ptr<FloatPointNearestNeighbor::NearestHit> expected =
          expectedHits[i];
      shared_ptr<FieldDoc> actual = std::static_pointer_cast<FieldDoc>(hits[i]);
      assertEquals(L"hit " + to_wstring(i) + L":", expected->docID,
                   actual->doc);
      assertEquals(L"hit " + to_wstring(i) + L":",
                   static_cast<float>(sqrt(expected->distanceSquared)),
                   any_cast<optional<float>>(actual->fields[0]), 0.000001);
    }
  }

  r->close();
  delete w;
  delete dir;
}

shared_ptr<IndexWriterConfig>
TestFloatPointNearestNeighbor::getIndexWriterConfig()
{
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig();
  iwc->setCodec(Codec::forName(L"Lucene70"));
  return iwc;
}
} // namespace org::apache::lucene::document