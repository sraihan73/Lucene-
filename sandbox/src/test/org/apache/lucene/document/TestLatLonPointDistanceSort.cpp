using namespace std;

#include "TestLatLonPointDistanceSort.h"

namespace org::apache::lucene::document
{
using GeoTestUtil = org::apache::lucene::geo::GeoTestUtil;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using SerialMergeScheduler = org::apache::lucene::index::SerialMergeScheduler;
using FieldDoc = org::apache::lucene::search::FieldDoc;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using MatchAllDocsQuery = org::apache::lucene::search::MatchAllDocsQuery;
using Sort = org::apache::lucene::search::Sort;
using SortField = org::apache::lucene::search::SortField;
using TopDocs = org::apache::lucene::search::TopDocs;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using SloppyMath = org::apache::lucene::util::SloppyMath;
using TestUtil = org::apache::lucene::util::TestUtil;
//    import static org.apache.lucene.geo.GeoEncodingUtils.decodeLatitude;
//    import static org.apache.lucene.geo.GeoEncodingUtils.decodeLongitude;
//    import static org.apache.lucene.geo.GeoEncodingUtils.encodeLatitude;
//    import static org.apache.lucene.geo.GeoEncodingUtils.encodeLongitude;

void TestLatLonPointDistanceSort::testDistanceSort() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir);

  // add some docs
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(
      make_shared<LatLonDocValuesField>(L"location", 40.759011, -73.9844722));
  iw->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(
      make_shared<LatLonDocValuesField>(L"location", 40.718266, -74.007819));
  iw->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(
      make_shared<LatLonDocValuesField>(L"location", 40.7051157, -74.0088305));
  iw->addDocument(doc);

  shared_ptr<IndexReader> reader = iw->getReader();
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);
  delete iw;

  shared_ptr<Sort> sort =
      make_shared<Sort>(LatLonDocValuesField::newDistanceSort(
          L"location", 40.7143528, -74.0059731));
  shared_ptr<TopDocs> td =
      searcher->search(make_shared<MatchAllDocsQuery>(), 3, sort);

  shared_ptr<FieldDoc> d = std::static_pointer_cast<FieldDoc>(td->scoreDocs[0]);
  assertEquals(462.1028401330431, any_cast<optional<double>>(d->fields[0]),
               0.0);

  d = std::static_pointer_cast<FieldDoc>(td->scoreDocs[1]);
  assertEquals(1054.9842850974826, any_cast<optional<double>>(d->fields[0]),
               0.0);

  d = std::static_pointer_cast<FieldDoc>(td->scoreDocs[2]);
  assertEquals(5285.881528419706, any_cast<optional<double>>(d->fields[0]),
               0.0);

  delete reader;
  delete dir;
}

void TestLatLonPointDistanceSort::testMissingLast() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir);

  // missing
  shared_ptr<Document> doc = make_shared<Document>();
  iw->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(
      make_shared<LatLonDocValuesField>(L"location", 40.718266, -74.007819));
  iw->addDocument(doc);

  doc = make_shared<Document>();
  doc->push_back(
      make_shared<LatLonDocValuesField>(L"location", 40.7051157, -74.0088305));
  iw->addDocument(doc);

  shared_ptr<IndexReader> reader = iw->getReader();
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);
  delete iw;

  shared_ptr<Sort> sort =
      make_shared<Sort>(LatLonDocValuesField::newDistanceSort(
          L"location", 40.7143528, -74.0059731));
  shared_ptr<TopDocs> td =
      searcher->search(make_shared<MatchAllDocsQuery>(), 3, sort);

  shared_ptr<FieldDoc> d = std::static_pointer_cast<FieldDoc>(td->scoreDocs[0]);
  assertEquals(462.1028401330431, any_cast<optional<double>>(d->fields[0]),
               0.0);

  d = std::static_pointer_cast<FieldDoc>(td->scoreDocs[1]);
  assertEquals(1054.9842850974826, any_cast<optional<double>>(d->fields[0]),
               0.0);

  d = std::static_pointer_cast<FieldDoc>(td->scoreDocs[2]);
  assertEquals(numeric_limits<double>::infinity(),
               any_cast<optional<double>>(d->fields[0]), 0.0);

  delete reader;
  delete dir;
}

void TestLatLonPointDistanceSort::testRandom() 
{
  for (int iters = 0; iters < 100; iters++) {
    doRandomTest(10, 100);
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Nightly public void testRandomHuge() throws Exception
void TestLatLonPointDistanceSort::testRandomHuge() 
{
  for (int iters = 0; iters < 10; iters++) {
    doRandomTest(2000, 100);
  }
}

TestLatLonPointDistanceSort::Result::Result(int id, double distance)
{
  this->id = id;
  this->distance = distance;
}

int TestLatLonPointDistanceSort::Result::compareTo(shared_ptr<Result> o)
{
  int cmp = Double::compare(distance, o->distance);
  if (cmp == 0) {
    return Integer::compare(id, o->id);
  }
  return cmp;
}

int TestLatLonPointDistanceSort::Result::hashCode()
{
  constexpr int prime = 31;
  int result = 1;
  int64_t temp;
  temp = Double::doubleToLongBits(distance);
  result = prime * result +
           static_cast<int>(temp ^
                            (static_cast<int64_t>(
                                static_cast<uint64_t>(temp) >> 32)));
  result = prime * result + id;
  return result;
}

bool TestLatLonPointDistanceSort::Result::equals(any obj)
{
  if (shared_from_this() == obj) {
    return true;
  }
  if (obj == nullptr) {
    return false;
  }
  if (getClass() != obj.type()) {
    return false;
  }
  shared_ptr<Result> other = any_cast<std::shared_ptr<Result>>(obj);
  if (Double::doubleToLongBits(distance) !=
      Double::doubleToLongBits(other->distance)) {
    return false;
  }
  if (id != other->id) {
    return false;
  }
  return true;
}

wstring TestLatLonPointDistanceSort::Result::toString()
{
  return L"Result [id=" + to_wstring(id) + L", distance=" +
         to_wstring(distance) + L"]";
}

void TestLatLonPointDistanceSort::doRandomTest(
    int numDocs, int numQueries) 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig();
  // else seeds may not to reproduce:
  iwc->setMergeScheduler(make_shared<SerialMergeScheduler>());
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir, iwc);

  for (int i = 0; i < numDocs; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(make_shared<StoredField>(L"id", i));
    doc->push_back(make_shared<NumericDocValuesField>(L"id", i));
    if (random()->nextInt(10) > 7) {
      double latRaw = GeoTestUtil::nextLatitude();
      double lonRaw = GeoTestUtil::nextLongitude();
      // pre-normalize up front, so we can just use quantized value for testing
      // and do simple exact comparisons
      double lat = decodeLatitude(encodeLatitude(latRaw));
      double lon = decodeLongitude(encodeLongitude(lonRaw));

      doc->push_back(make_shared<LatLonDocValuesField>(L"field", lat, lon));
      doc->push_back(make_shared<StoredField>(L"lat", lat));
      doc->push_back(make_shared<StoredField>(L"lon", lon));
    } // otherwise "missing"
    writer->addDocument(doc);
  }
  shared_ptr<IndexReader> reader = writer->getReader();
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);

  for (int i = 0; i < numQueries; i++) {
    double lat = GeoTestUtil::nextLatitude();
    double lon = GeoTestUtil::nextLongitude();
    double missingValue = numeric_limits<double>::infinity();

    std::deque<std::shared_ptr<Result>> expected(reader->maxDoc());

    for (int doc = 0; doc < reader->maxDoc(); doc++) {
      shared_ptr<Document> targetDoc = reader->document(doc);
      constexpr double distance;
      if (targetDoc->getField(L"lat") == nullptr) {
        distance = missingValue; // missing
      } else {
        double docLatitude =
            targetDoc->getField(L"lat")->numericValue()->doubleValue();
        double docLongitude =
            targetDoc->getField(L"lon")->numericValue()->doubleValue();
        distance =
            SloppyMath::haversinMeters(lat, lon, docLatitude, docLongitude);
      }
      int id = targetDoc->getField(L"id")->numericValue()->intValue();
      expected[doc] = make_shared<Result>(id, distance);
    }

    Arrays::sort(expected);

    // randomize the topN a bit
    int topN = TestUtil::nextInt(random(), 1, reader->maxDoc());
    // sort by distance, then ID
    shared_ptr<SortField> distanceSort =
        LatLonDocValuesField::newDistanceSort(L"field", lat, lon);
    distanceSort->setMissingValue(missingValue);
    shared_ptr<Sort> sort = make_shared<Sort>(
        distanceSort, make_shared<SortField>(L"id", SortField::Type::INT));

    shared_ptr<TopDocs> topDocs =
        searcher->search(make_shared<MatchAllDocsQuery>(), topN, sort);
    for (int resultNumber = 0; resultNumber < topN; resultNumber++) {
      shared_ptr<FieldDoc> fieldDoc =
          std::static_pointer_cast<FieldDoc>(topDocs->scoreDocs[resultNumber]);
      shared_ptr<Result> actual =
          make_shared<Result>(any_cast<optional<int>>(fieldDoc->fields[1]),
                              any_cast<optional<double>>(fieldDoc->fields[0]));
      TestUtil::assertEquals(expected[resultNumber], actual);
    }

    // get page2 with searchAfter()
    if (topN < reader->maxDoc()) {
      int page2 = TestUtil::nextInt(random(), 1, reader->maxDoc() - topN);
      shared_ptr<TopDocs> topDocs2 =
          searcher->searchAfter(topDocs->scoreDocs[topN - 1],
                                make_shared<MatchAllDocsQuery>(), page2, sort);
      for (int resultNumber = 0; resultNumber < page2; resultNumber++) {
        shared_ptr<FieldDoc> fieldDoc = std::static_pointer_cast<FieldDoc>(
            topDocs2->scoreDocs[resultNumber]);
        shared_ptr<Result> actual = make_shared<Result>(
            any_cast<optional<int>>(fieldDoc->fields[1]),
            any_cast<optional<double>>(fieldDoc->fields[0]));
        TestUtil::assertEquals(expected[topN + resultNumber], actual);
      }
    }
  }
  delete reader;
  delete writer;
  delete dir;
}
} // namespace org::apache::lucene::document