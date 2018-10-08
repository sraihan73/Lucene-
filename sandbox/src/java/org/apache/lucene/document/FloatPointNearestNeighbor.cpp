using namespace std;

#include "FloatPointNearestNeighbor.h"

namespace org::apache::lucene::document
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using PointValues = org::apache::lucene::index::PointValues;
using FieldDoc = org::apache::lucene::search::FieldDoc;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using ScoreDoc = org::apache::lucene::search::ScoreDoc;
using TopFieldDocs = org::apache::lucene::search::TopFieldDocs;
using Bits = org::apache::lucene::util::Bits;
using BytesRef = org::apache::lucene::util::BytesRef;
using BKDReader = org::apache::lucene::util::bkd::BKDReader;

FloatPointNearestNeighbor::Cell::Cell(shared_ptr<BKDReader::IndexTree> index,
                                      int readerIndex,
                                      std::deque<char> &minPacked,
                                      std::deque<char> &maxPacked,
                                      double distanceSquared)
    : readerIndex(readerIndex), minPacked(minPacked.clone()),
      maxPacked(maxPacked.clone()), index(index),
      distanceSquared(distanceSquared)
{
}

int FloatPointNearestNeighbor::Cell::compareTo(shared_ptr<Cell> other)
{
  return Double::compare(distanceSquared, other->distanceSquared);
}

wstring FloatPointNearestNeighbor::Cell::toString()
{
  return L"Cell(readerIndex=" + to_wstring(readerIndex) + L" nodeID=" +
         to_wstring(index->getNodeID()) + L" isLeaf=" +
         StringHelper::toString(index->isLeafNode()) + L" distanceSquared=" +
         to_wstring(distanceSquared) + L")";
}

FloatPointNearestNeighbor::NearestVisitor::NearestVisitor(
    shared_ptr<PriorityQueue<std::shared_ptr<NearestHit>>> hitQueue, int topN,
    std::deque<float> &origin)
    : topN(topN), hitQueue(hitQueue), origin(origin)
{
  dims = origin.size();
  min = std::deque<float>(dims);
  max = std::deque<float>(dims);
  Arrays::fill(min, -numeric_limits<float>::infinity());
  Arrays::fill(max, numeric_limits<float>::infinity());
}

void FloatPointNearestNeighbor::NearestVisitor::visit(int docID)
{
  throw make_shared<AssertionError>();
}

float FloatPointNearestNeighbor::NearestVisitor::getMinDelta(float distance)
{
  int exponent =
      Float::floatToIntBits(distance) >>
      MANTISSA_BITS; // extract biased exponent (distance is positive)
  if (exponent == 0) {
    return Float::MIN_VALUE;
  } else {
    exponent = exponent <= MANTISSA_BITS
                   ? 1
                   : exponent - MANTISSA_BITS; // Avoid underflow
    return Float::intBitsToFloat(exponent << MANTISSA_BITS);
  }
}

void FloatPointNearestNeighbor::NearestVisitor::maybeUpdateMinMax()
{
  if (updateMinMaxCounter < 1024 || (updateMinMaxCounter & 0x3F) == 0x3F) {
    shared_ptr<NearestHit> hit = hitQueue->peek();
    float distance = static_cast<float>(sqrt(hit->distanceSquared));
    float minDelta = getMinDelta(distance);
    // std::wstring oldMin = Arrays.toString(min);
    // std::wstring oldMax = Arrays.toString(max);
    for (int d = 0; d < dims; ++d) {
      min[d] = (origin[d] - distance) - minDelta;
      max[d] = (origin[d] + distance) + minDelta;
      // System.out.println("origin[" + d + "] (" + origin[d] + ") - distance ("
      // + distance + ") - minDelta (" + minDelta + ") = min[" + d + "] (" +
      // min[d] + ")"); System.out.println("origin[" + d + "] (" + origin[d] +
      // ") + distance (" + distance + ") + minDelta (" + minDelta + ") = max["
      // + d + "] (" + max[d] + ")");
    }
    // System.out.println("maybeUpdateMinMax:  min: " + oldMin + " -> " +
    // Arrays.toString(min) + "   max: " + oldMax + " -> " +
    // Arrays.toString(max));
  }
  ++updateMinMaxCounter;
}

void FloatPointNearestNeighbor::NearestVisitor::visit(
    int docID, std::deque<char> &packedValue)
{
  // System.out.println("visit docID=" + docID + " liveDocs=" + curLiveDocs);

  if (curLiveDocs != nullptr && curLiveDocs->get(docID) == false) {
    return;
  }

  std::deque<float> docPoint(dims);
  for (int d = 0, offset = 0; d < dims; ++d, offset += Float::BYTES) {
    docPoint[d] = FloatPoint::decodeDimension(packedValue, offset);
    if (docPoint[d] > max[d] || docPoint[d] < min[d]) {

      // if (docPoint[d] > max[d]) {
      //   System.out.println("  skipped because docPoint[" + d + "] (" +
      //   docPoint[d] + ") > max[" + d + "] (" + max[d] + ")");
      // } else {
      //   System.out.println("  skipped because docPoint[" + d + "] (" +
      //   docPoint[d] + ") < min[" + d + "] (" + min[d] + ")");
      // }

      return;
    }
  }

  double distanceSquared = euclideanDistanceSquared(origin, docPoint);

  // System.out.println("    visit docID=" + docID + " distanceSquared=" +
  // distanceSquared + " value: " + Arrays.toString(docPoint));

  int fullDocID = curDocBase + docID;

  if (hitQueue->size() == topN) { // queue already full
    shared_ptr<NearestHit> bottom = hitQueue->peek();
    // System.out.println("      bottom distanceSquared=" +
    // bottom.distanceSquared);
    if (distanceSquared < bottom->distanceSquared ||
        (distanceSquared == bottom->distanceSquared &&
         fullDocID < bottom->docID)) {
      hitQueue->poll();
      bottom->docID = fullDocID;
      bottom->distanceSquared = distanceSquared;
      hitQueue->offer(bottom);
      // System.out.println("      ** keep1, now bottom=" + bottom);
      maybeUpdateMinMax();
    }
  } else {
    shared_ptr<NearestHit> hit = make_shared<NearestHit>();
    hit->docID = fullDocID;
    hit->distanceSquared = distanceSquared;
    hitQueue->offer(hit);
    // System.out.println("      ** keep2, new addition=" + hit);
  }
}

PointValues::Relation FloatPointNearestNeighbor::NearestVisitor::compare(
    std::deque<char> &minPackedValue, std::deque<char> &maxPackedValue)
{
  throw make_shared<AssertionError>();
}

wstring FloatPointNearestNeighbor::NearestHit::toString()
{
  return L"NearestHit(docID=" + to_wstring(docID) + L" distanceSquared=" +
         to_wstring(distanceSquared) + L")";
}

std::deque<std::shared_ptr<NearestHit>> FloatPointNearestNeighbor::nearest(
    deque<std::shared_ptr<BKDReader>> &readers,
    deque<std::shared_ptr<Bits>> &liveDocs, deque<int> &docBases,
    int const topN, std::deque<float> &origin) 
{

  // System.out.println("NEAREST: readers=" + readers + " liveDocs=" + liveDocs
  // + " origin: " + Arrays.toString(origin));

  // Holds closest collected points seen so far:
  // TODO: if we used lucene's PQ we could just updateTop instead of poll/offer:
  shared_ptr<PriorityQueue<std::shared_ptr<NearestHit>>> *const hitQueue =
      make_shared<PriorityQueue<std::shared_ptr<NearestHit>>>(topN, [&](a, b) {
        int cmp = Double::compare(a::distanceSquared, b::distanceSquared);
        return cmp != 0 ? -cmp : b::docID - a::docID;
      });

  // Holds all cells, sorted by closest to the point:
  shared_ptr<PriorityQueue<std::shared_ptr<Cell>>> cellQueue =
      make_shared<PriorityQueue<std::shared_ptr<Cell>>>();

  shared_ptr<NearestVisitor> visitor =
      make_shared<NearestVisitor>(hitQueue, topN, origin);
  deque<std::shared_ptr<BKDReader::IntersectState>> states =
      deque<std::shared_ptr<BKDReader::IntersectState>>();

  // Add root cell for each reader into the queue:
  int bytesPerDim = -1;

  for (int i = 0; i < readers.size(); ++i) {
    shared_ptr<BKDReader> reader = readers[i];
    if (bytesPerDim == -1) {
      bytesPerDim = reader->getBytesPerDimension();
    } else if (bytesPerDim != reader->getBytesPerDimension()) {
      throw make_shared<IllegalStateException>(
          L"bytesPerDim changed from " + to_wstring(bytesPerDim) + L" to " +
          to_wstring(reader->getBytesPerDimension()) + L" across readers");
    }
    std::deque<char> minPackedValue = reader->getMinPackedValue();
    std::deque<char> maxPackedValue = reader->getMaxPackedValue();
    shared_ptr<BKDReader::IntersectState> state =
        reader->getIntersectState(visitor);
    states.push_back(state);

    cellQueue->offer(make_shared<Cell>(
        state->index, i, reader->getMinPackedValue(),
        reader->getMaxPackedValue(),
        approxBestDistanceSquared(minPackedValue, maxPackedValue, origin)));
  }

  while (cellQueue->size() > 0) {
    shared_ptr<Cell> cell = cellQueue->poll();
    // System.out.println("  visit " + cell);

    // TODO: if we replace approxBestDistance with actualBestDistance, we can
    // put an opto here to break once this "best" cell is fully outside of the
    // hitQueue bottom's radius:
    shared_ptr<BKDReader> reader = readers[cell->readerIndex];

    if (cell->index->isLeafNode()) {
      // System.out.println("    leaf");
      // Leaf block: visit all points and possibly collect them:
      visitor->curDocBase = docBases[cell->readerIndex];
      visitor->curLiveDocs = liveDocs[cell->readerIndex];
      reader->visitLeafBlockValues(cell->index, states[cell->readerIndex]);
      // System.out.println("    now " + hitQueue.size() + " hits");
    } else {
      // System.out.println("    non-leaf");
      // Non-leaf block: split into two cells and put them back into the queue:

      if (hitQueue->size() == topN) {
        for (int d = 0, offset = 0; d < visitor->dims;
             ++d, offset += Float::BYTES) {
          float cellMaxAtDim =
              FloatPoint::decodeDimension(cell->maxPacked, offset);
          float cellMinAtDim =
              FloatPoint::decodeDimension(cell->minPacked, offset);
          if (cellMaxAtDim < visitor->min[d] ||
              cellMinAtDim > visitor->max[d]) {
            // this cell is outside our search radius; don't bother exploring
            // any more

            // if (cellMaxAtDim < visitor.min[d]) {
            //   System.out.println("  skipped because cell max at " + d + " ("
            //   + cellMaxAtDim + ") < visitor.min[" + d + "] (" +
            //   visitor.min[d] + ")");
            // } else {
            //   System.out.println("  skipped because cell min at " + d + " ("
            //   + cellMinAtDim + ") > visitor.max[" + d + "] (" +
            //   visitor.max[d] + ")");
            // }

            goto LOOP_OVER_CELLSContinue;
          }
        }
      }
      shared_ptr<BytesRef> splitValue =
          BytesRef::deepCopyOf(cell->index->getSplitDimValue());
      int splitDim = cell->index->getSplitDim();

      // we must clone the index so that we we can recurse left and right
      // "concurrently":
      shared_ptr<BKDReader::IndexTree> newIndex = cell->index->clone();
      std::deque<char> splitPackedValue = cell->maxPacked.clone();
      System::arraycopy(splitValue->bytes, splitValue->offset, splitPackedValue,
                        splitDim * bytesPerDim, bytesPerDim);

      cell->index->pushLeft();
      cellQueue->offer(make_shared<Cell>(
          cell->index, cell->readerIndex, cell->minPacked, splitPackedValue,
          approxBestDistanceSquared(cell->minPacked, splitPackedValue,
                                    origin)));

      splitPackedValue = cell->minPacked.clone();
      System::arraycopy(splitValue->bytes, splitValue->offset, splitPackedValue,
                        splitDim * bytesPerDim, bytesPerDim);

      newIndex->pushRight();
      cellQueue->offer(make_shared<Cell>(
          newIndex, cell->readerIndex, splitPackedValue, cell->maxPacked,
          approxBestDistanceSquared(splitPackedValue, cell->maxPacked,
                                    origin)));
    }
  LOOP_OVER_CELLSContinue:;
  }
LOOP_OVER_CELLSBreak:

  std::deque<std::shared_ptr<NearestHit>> hits(hitQueue->size());
  int downTo = hitQueue->size() - 1;
  while (hitQueue->size() != 0) {
    hits[downTo] = hitQueue->poll();
    downTo--;
  }
  return hits;
}

double FloatPointNearestNeighbor::approxBestDistanceSquared(
    std::deque<char> &minPackedValue, std::deque<char> &maxPackedValue,
    std::deque<float> &value)
{
  bool insideCell = true;
  std::deque<float> min(value.size());
  std::deque<float> max(value.size());
  std::deque<double> closest(value.size());
  for (int i = 0, offset = 0; i < value.size(); ++i, offset += Float::BYTES) {
    min[i] = FloatPoint::decodeDimension(minPackedValue, offset);
    max[i] = FloatPoint::decodeDimension(maxPackedValue, offset);
    if (insideCell) {
      if (value[i] < min[i] || value[i] > max[i]) {
        insideCell = false;
      }
    }
    double minDiff =
        abs(static_cast<double>(value[i]) - static_cast<double>(min[i]));
    double maxDiff =
        abs(static_cast<double>(value[i]) - static_cast<double>(max[i]));
    closest[i] = minDiff < maxDiff ? minDiff : maxDiff;
  }
  if (insideCell) {
    return 0.0f;
  }
  double sumOfSquaredDiffs = 0.0;
  for (int d = 0; d < value.size(); ++d) {
    sumOfSquaredDiffs += closest[d] * closest[d];
  }
  return sumOfSquaredDiffs;
}

double
FloatPointNearestNeighbor::euclideanDistanceSquared(std::deque<float> &a,
                                                    std::deque<float> &b)
{
  double sumOfSquaredDifferences = 0.0;
  for (int d = 0; d < a.size(); ++d) {
    double diff = static_cast<double>(a[d]) - static_cast<double>(b[d]);
    sumOfSquaredDifferences += diff * diff;
  }
  return sumOfSquaredDifferences;
}

shared_ptr<TopFieldDocs>
FloatPointNearestNeighbor::nearest(shared_ptr<IndexSearcher> searcher,
                                   const wstring &field, int topN,
                                   deque<float> &origin) 
{
  if (topN < 1) {
    throw invalid_argument(L"topN must be at least 1; got " + to_wstring(topN));
  }
  if (field == L"") {
    throw invalid_argument(L"field must not be null");
  }
  if (searcher == nullptr) {
    throw invalid_argument(L"searcher must not be null");
  }
  deque<std::shared_ptr<BKDReader>> readers =
      deque<std::shared_ptr<BKDReader>>();
  deque<int> docBases = deque<int>();
  deque<std::shared_ptr<Bits>> liveDocs = deque<std::shared_ptr<Bits>>();
  int totalHits = 0;
  for (auto leaf : searcher->getIndexReader()->leaves()) {
    shared_ptr<PointValues> points = leaf->reader()->getPointValues(field);
    if (points != nullptr) {
      if (std::dynamic_pointer_cast<BKDReader>(points) != nullptr == false) {
        throw invalid_argument(L"can only run on Lucene60PointsReader points "
                               L"implementation, but got " +
                               points);
      }
      totalHits += points->getDocCount();
      readers.push_back(std::static_pointer_cast<BKDReader>(points));
      docBases.push_back(leaf->docBase);
      liveDocs.push_back(leaf->reader()->getLiveDocs());
    }
  }

  std::deque<std::shared_ptr<NearestHit>> hits =
      nearest(readers, liveDocs, docBases, {topN, origin});

  // Convert to TopFieldDocs:
  std::deque<std::shared_ptr<ScoreDoc>> scoreDocs(hits.size());
  for (int i = 0; i < hits.size(); i++) {
    shared_ptr<NearestHit> hit = hits[i];
    scoreDocs[i] = make_shared<FieldDoc>(
        hit->docID, 0.0f,
        std::deque<any>{static_cast<float>(sqrt(hit->distanceSquared))});
  }
  return make_shared<TopFieldDocs>(totalHits, scoreDocs, nullptr, 0.0f);
}
} // namespace org::apache::lucene::document