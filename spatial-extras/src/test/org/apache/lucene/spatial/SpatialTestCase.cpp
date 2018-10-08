using namespace std;

#include "SpatialTestCase.h"

namespace org::apache::lucene::spatial
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using ScoreDoc = org::apache::lucene::search::ScoreDoc;
using TopDocs = org::apache::lucene::search::TopDocs;
using Directory = org::apache::lucene::store::Directory;
using IOUtils = org::apache::lucene::util::IOUtils;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::apache::lucene::util::LuceneTestCase::SuppressSysoutChecks;
using org::locationtech::spatial4j::context::SpatialContext;
using org::locationtech::spatial4j::distance::DistanceUtils;
using org::locationtech::spatial4j::shape::Point;
using org::locationtech::spatial4j::shape::Rectangle;
//    import static
//    com.carrotsearch.randomizedtesting.RandomizedTest.randomDouble; import
//    static com.carrotsearch.randomizedtesting.RandomizedTest.randomGaussian;
//    import static com.carrotsearch.randomizedtesting.RandomizedTest.randomInt;
//    import static
//    com.carrotsearch.randomizedtesting.RandomizedTest.randomIntBetween;

void SpatialTestCase::setUp() 
{
  LuceneTestCase::setUp();
  directory = newDirectory();
  analyzer = make_shared<MockAnalyzer>(random());
  indexWriter = make_shared<RandomIndexWriter>(
      random(), directory,
      LuceneTestCase::newIndexWriterConfig(random(), analyzer));
  indexReader = indexWriter->getReader();
  indexSearcher = newSearcher(indexReader);
}

void SpatialTestCase::tearDown() 
{
  IOUtils::close({indexWriter, indexReader, analyzer, directory});
  LuceneTestCase::tearDown();
}

void SpatialTestCase::addDocument(shared_ptr<Document> doc) 
{
  indexWriter->addDocument(doc);
}

void SpatialTestCase::addDocumentsAndCommit(
    deque<std::shared_ptr<Document>> &documents) 
{
  for (auto document : documents) {
    indexWriter->addDocument(document);
  }
  commit();
}

void SpatialTestCase::deleteAll() 
{
  indexWriter->deleteAll();
}

void SpatialTestCase::commit() 
{
  indexWriter->commit();
  shared_ptr<DirectoryReader> newReader =
      DirectoryReader::openIfChanged(indexReader);
  if (newReader != nullptr) {
    IOUtils::close({indexReader});
    indexReader = newReader;
  }
  indexSearcher = newSearcher(indexReader);
}

void SpatialTestCase::verifyDocumentsIndexed(int numDocs)
{
  assertEquals(numDocs, indexReader->numDocs());
}

shared_ptr<SearchResults> SpatialTestCase::executeQuery(shared_ptr<Query> query,
                                                        int numDocs)
{
  try {
    shared_ptr<TopDocs> topDocs = indexSearcher->search(query, numDocs);

    deque<std::shared_ptr<SearchResult>> results =
        deque<std::shared_ptr<SearchResult>>();
    for (auto scoreDoc : topDocs->scoreDocs) {
      results.push_back(make_shared<SearchResult>(
          scoreDoc->score, indexSearcher->doc(scoreDoc->doc)));
    }
    return make_shared<SearchResults>(topDocs->totalHits, results);
  } catch (const IOException &ioe) {
    // C++ TODO: This exception's constructor requires only one argument:
    // ORIGINAL LINE: throw new RuntimeException("IOException thrown while
    // executing query", ioe);
    throw runtime_error(L"IOException thrown while executing query");
  }
}

shared_ptr<Point> SpatialTestCase::randomPoint()
{
  shared_ptr<Rectangle> *const WB = ctx->getWorldBounds();
  return ctx->makePoint(randomIntBetween(static_cast<int>(WB->getMinX()),
                                         static_cast<int>(WB->getMaxX())),
                        randomIntBetween(static_cast<int>(WB->getMinY()),
                                         static_cast<int>(WB->getMaxY())));
}

shared_ptr<Rectangle> SpatialTestCase::randomRectangle()
{
  return randomRectangle(ctx->getWorldBounds());
}

shared_ptr<Rectangle>
SpatialTestCase::randomRectangle(shared_ptr<Rectangle> bounds)
{
  std::deque<double> xNewStartAndWidth =
      randomSubRange(bounds->getMinX(), bounds->getWidth());
  double xMin = xNewStartAndWidth[0];
  double xMax = xMin + xNewStartAndWidth[1];
  if (bounds->getCrossesDateLine()) {
    xMin = DistanceUtils::normLonDEG(xMin);
    xMax = DistanceUtils::normLonDEG(xMax);
  }

  std::deque<double> yNewStartAndHeight =
      randomSubRange(bounds->getMinY(), bounds->getHeight());
  double yMin = yNewStartAndHeight[0];
  double yMax = yMin + yNewStartAndHeight[1];

  return ctx->makeRectangle(xMin, xMax, yMin, yMax);
}

std::deque<double> SpatialTestCase::randomSubRange(double boundStart,
                                                    double boundLen)
{
  if (boundLen >= 3 && usually()) { // typical
    // prefer integers for ease of debugability ... and prefer 1/16th of bound
    int intBoundStart = static_cast<int>(ceil(boundStart));
    int intBoundEnd = static_cast<int>(boundStart + boundLen);
    int intBoundLen = intBoundEnd - intBoundStart;
    int newLen = static_cast<int>(
        randomGaussianMeanMax(intBoundLen / 16.0, intBoundLen));
    int newStart = intBoundStart + randomInt(intBoundLen - newLen);
    return std::deque<double>{newStart, newLen};
  } else { // (no int rounding)
    double newLen = randomGaussianMeanMax(boundLen / 16, boundLen);
    double newStart =
        boundStart +
        (boundLen - newLen == 0 ? 0 : (randomDouble() % (boundLen - newLen)));
    return std::deque<double>{newStart, newLen};
  }
}

double SpatialTestCase::randomGaussianMinMeanMax(double min, double mean,
                                                 double max)
{
  assert(mean > min);
  return randomGaussianMeanMax(mean - min, max - min) + min;
}

double SpatialTestCase::randomGaussianMeanMax(double mean, double max)
{
  // DWS: I verified the results empirically
  assert(mean <= max && mean >= 0);
  double g = randomGaussian();
  double mean2 = mean;
  double flip = 1;
  if (g < 0) {
    mean2 = max - mean;
    flip = -1;
    g *= -1;
  }
  // pivot is the distance from mean2 towards max where the boundary of
  // 1 standard deviation alters the calculation
  double pivotMax = max - mean2;
  double pivot = min(mean2, pivotMax / 2); // from 0 to max-mean2
  assert(pivot >= 0 && pivotMax >= pivot && g >= 0);
  double pivotResult;
  if (g <= 1) {
    pivotResult = pivot * g;
  } else {
    pivotResult = min(pivotMax, (g - 1) * (pivotMax - pivot) + pivot);
  }

  double result = mean + flip * pivotResult;
  return (result < 0 || result > max)
             ? mean
             : result; // due this due to computational numerical precision
}

SpatialTestCase::SearchResults::SearchResults(
    int64_t numFound, deque<std::shared_ptr<SearchResult>> &results)
{
  this->numFound = numFound;
  this->results = results;
}

shared_ptr<StringBuilder> SpatialTestCase::SearchResults::toDebugString()
{
  shared_ptr<StringBuilder> str = make_shared<StringBuilder>();
  str->append(L"found: ")->append(numFound)->append(L'[');
  for (auto r : results) {
    wstring id = r->getId();
    str->append(id)->append(L", ");
  }
  str->append(L']');
  return str;
}

wstring SpatialTestCase::SearchResults::toString()
{
  return L"[found:" + to_wstring(numFound) + L" " + results + L"]";
}

SpatialTestCase::SearchResult::SearchResult(float score,
                                            shared_ptr<Document> document)
{
  this->score = score;
  this->document = document;
}

wstring SpatialTestCase::SearchResult::getId() { return document[L"id"]; }

wstring SpatialTestCase::SearchResult::toString()
{
  return L"[" + to_wstring(score) + L"=" + document + L"]";
}
} // namespace org::apache::lucene::spatial