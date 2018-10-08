using namespace std;

#include "NearestNeighbor.h"

namespace org::apache::lucene::document
{
using Rectangle = org::apache::lucene::geo::Rectangle;
using IntersectVisitor =
    org::apache::lucene::index::PointValues::IntersectVisitor;
using Relation = org::apache::lucene::index::PointValues::Relation;
using Bits = org::apache::lucene::util::Bits;
using BytesRef = org::apache::lucene::util::BytesRef;
using SloppyMath = org::apache::lucene::util::SloppyMath;
using IndexTree = org::apache::lucene::util::bkd::BKDReader::IndexTree;
using IntersectState =
    org::apache::lucene::util::bkd::BKDReader::IntersectState;
using BKDReader = org::apache::lucene::util::bkd::BKDReader;
//    import static org.apache.lucene.geo.GeoEncodingUtils.decodeLatitude;
//    import static org.apache.lucene.geo.GeoEncodingUtils.decodeLongitude;

NearestNeighbor::Cell::Cell(shared_ptr<IndexTree> index, int readerIndex,
                            std::deque<char> &minPacked,
                            std::deque<char> &maxPacked, double distanceMeters)
    : readerIndex(readerIndex), minPacked(minPacked.clone()),
      maxPacked(maxPacked.clone()), index(index), distanceMeters(distanceMeters)
{
}

int NearestNeighbor::Cell::compareTo(shared_ptr<Cell> other)
{
  return Double::compare(distanceMeters, other->distanceMeters);
}

wstring NearestNeighbor::Cell::toString()
{
  double minLat = decodeLatitude(minPacked, 0);
  double minLon = decodeLongitude(minPacked, Integer::BYTES);
  double maxLat = decodeLatitude(maxPacked, 0);
  double maxLon = decodeLongitude(maxPacked, Integer::BYTES);
  return L"Cell(readerIndex=" + to_wstring(readerIndex) + L" nodeID=" +
         to_wstring(index->getNodeID()) + L" isLeaf=" +
         StringHelper::toString(index->isLeafNode()) + L" lat=" +
         to_wstring(minLat) + L" TO " + to_wstring(maxLat) + L", lon=" +
         to_wstring(minLon) + L" TO " + to_wstring(maxLon) +
         L"; distanceMeters=" + to_wstring(distanceMeters) + L")";
}

NearestNeighbor::NearestVisitor::NearestVisitor(
    shared_ptr<PriorityQueue<std::shared_ptr<NearestHit>>> hitQueue, int topN,
    double pointLat, double pointLon)
    : topN(topN), hitQueue(hitQueue), pointLat(pointLat), pointLon(pointLon)
{
}

void NearestNeighbor::NearestVisitor::visit(int docID)
{
  throw make_shared<AssertionError>();
}

void NearestNeighbor::NearestVisitor::maybeUpdateBBox()
{
  if (setBottomCounter < 1024 || (setBottomCounter & 0x3F) == 0x3F) {
    shared_ptr<NearestHit> hit = hitQueue->peek();
    shared_ptr<Rectangle> box =
        Rectangle::fromPointDistance(pointLat, pointLon, hit->distanceMeters);
    // System.out.println("    update bbox to " + box);
    minLat = box->minLat;
    maxLat = box->maxLat;
    if (box->crossesDateline()) {
      // box1
      minLon = -numeric_limits<double>::infinity();
      maxLon = box->maxLon;
      // box2
      minLon2 = box->minLon;
    } else {
      minLon = box->minLon;
      maxLon = box->maxLon;
      // disable box2
      minLon2 = numeric_limits<double>::infinity();
    }
  }
  setBottomCounter++;
}

void NearestNeighbor::NearestVisitor::visit(int docID,
                                            std::deque<char> &packedValue)
{
  // System.out.println("visit docID=" + docID + " liveDocs=" + curLiveDocs);

  if (curLiveDocs != nullptr && curLiveDocs->get(docID) == false) {
    return;
  }

  // TODO: work in int space, use haversinSortKey

  double docLatitude = decodeLatitude(packedValue, 0);
  double docLongitude = decodeLongitude(packedValue, Integer::BYTES);

  // test bounding box
  if (docLatitude < minLat || docLatitude > maxLat) {
    return;
  }
  if ((docLongitude < minLon || docLongitude > maxLon) &&
      (docLongitude < minLon2)) {
    return;
  }

  double distanceMeters =
      SloppyMath::haversinMeters(pointLat, pointLon, docLatitude, docLongitude);

  // System.out.println("    visit docID=" + docID + " distanceMeters=" +
  // distanceMeters + " docLat=" + docLatitude + " docLon=" + docLongitude);

  int fullDocID = curDocBase + docID;

  if (hitQueue->size() == topN) {
    // queue already full
    shared_ptr<NearestHit> hit = hitQueue->peek();
    // System.out.println("      bottom distanceMeters=" + hit.distanceMeters);
    // we don't collect docs in order here, so we must also test the tie-break
    // case ourselves:
    if (distanceMeters < hit->distanceMeters ||
        (distanceMeters == hit->distanceMeters && fullDocID < hit->docID)) {
      hitQueue->poll();
      hit->docID = fullDocID;
      hit->distanceMeters = distanceMeters;
      hitQueue->offer(hit);
      // System.out.println("      ** keep2, now bottom=" + hit);
      maybeUpdateBBox();
    }

  } else {
    shared_ptr<NearestHit> hit = make_shared<NearestHit>();
    hit->docID = fullDocID;
    hit->distanceMeters = distanceMeters;
    hitQueue->offer(hit);
    // System.out.println("      ** keep1, now bottom=" + hit);
  }
}

Relation
NearestNeighbor::NearestVisitor::compare(std::deque<char> &minPackedValue,
                                         std::deque<char> &maxPackedValue)
{
  throw make_shared<AssertionError>();
}

wstring NearestNeighbor::NearestHit::toString()
{
  return L"NearestHit(docID=" + to_wstring(docID) + L" distanceMeters=" +
         to_wstring(distanceMeters) + L")";
}

std::deque<std::shared_ptr<NearestHit>>
NearestNeighbor::nearest(double pointLat, double pointLon,
                         deque<std::shared_ptr<BKDReader>> &readers,
                         deque<std::shared_ptr<Bits>> &liveDocs,
                         deque<int> &docBases, int const n) 
{

  // System.out.println("NEAREST: readers=" + readers + " liveDocs=" + liveDocs
  // + " pointLat=" + pointLat + " pointLon=" + pointLon);
  // Holds closest collected points seen so far:
  // TODO: if we used lucene's PQ we could just updateTop instead of poll/offer:
  shared_ptr<PriorityQueue<std::shared_ptr<NearestHit>>> *const hitQueue =
      make_shared<PriorityQueue<std::shared_ptr<NearestHit>>>(
          n, make_shared<ComparatorAnonymousInnerClass>());

  // Holds all cells, sorted by closest to the point:
  shared_ptr<PriorityQueue<std::shared_ptr<Cell>>> cellQueue =
      make_shared<PriorityQueue<std::shared_ptr<Cell>>>();

  shared_ptr<NearestVisitor> visitor =
      make_shared<NearestVisitor>(hitQueue, n, pointLat, pointLon);
  deque<std::shared_ptr<IntersectState>> states =
      deque<std::shared_ptr<IntersectState>>();

  // Add root cell for each reader into the queue:
  int bytesPerDim = -1;

  for (int i = 0; i < readers.size(); i++) {
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
    shared_ptr<IntersectState> state = reader->getIntersectState(visitor);
    states.push_back(state);

    cellQueue->offer(
        make_shared<Cell>(state->index, i, reader->getMinPackedValue(),
                          reader->getMaxPackedValue(),
                          approxBestDistance(minPackedValue, maxPackedValue,
                                             pointLat, pointLon)));
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

      double cellMinLat = decodeLatitude(cell->minPacked, 0);
      double cellMinLon = decodeLongitude(cell->minPacked, Integer::BYTES);
      double cellMaxLat = decodeLatitude(cell->maxPacked, 0);
      double cellMaxLon = decodeLongitude(cell->maxPacked, Integer::BYTES);

      if (cellMaxLat < visitor->minLat || visitor->maxLat < cellMinLat ||
          ((cellMaxLon < visitor->minLon || visitor->maxLon < cellMinLon) &&
           cellMaxLon < visitor->minLon2)) {
        // this cell is outside our search bbox; don't bother exploring any more
        continue;
      }

      shared_ptr<BytesRef> splitValue =
          BytesRef::deepCopyOf(cell->index->getSplitDimValue());
      int splitDim = cell->index->getSplitDim();

      // we must clone the index so that we we can recurse left and right
      // "concurrently":
      shared_ptr<IndexTree> newIndex = cell->index->clone();
      std::deque<char> splitPackedValue = cell->maxPacked.clone();
      System::arraycopy(splitValue->bytes, splitValue->offset, splitPackedValue,
                        splitDim * bytesPerDim, bytesPerDim);

      cell->index->pushLeft();
      cellQueue->offer(make_shared<Cell>(
          cell->index, cell->readerIndex, cell->minPacked, splitPackedValue,
          approxBestDistance(cell->minPacked, splitPackedValue, pointLat,
                             pointLon)));

      splitPackedValue = cell->minPacked.clone();
      System::arraycopy(splitValue->bytes, splitValue->offset, splitPackedValue,
                        splitDim * bytesPerDim, bytesPerDim);

      newIndex->pushRight();
      cellQueue->offer(make_shared<Cell>(
          newIndex, cell->readerIndex, splitPackedValue, cell->maxPacked,
          approxBestDistance(splitPackedValue, cell->maxPacked, pointLat,
                             pointLon)));
    }
  }

  std::deque<std::shared_ptr<NearestHit>> hits(hitQueue->size());
  int downTo = hitQueue->size() - 1;
  while (hitQueue->size() != 0) {
    hits[downTo] = hitQueue->poll();
    downTo--;
  }

  return hits;
}

NearestNeighbor::ComparatorAnonymousInnerClass::ComparatorAnonymousInnerClass()
{
}

int NearestNeighbor::ComparatorAnonymousInnerClass::compare(
    shared_ptr<NearestHit> a, shared_ptr<NearestHit> b)
{
  // sort by opposite distanceMeters natural order
  int cmp = Double::compare(a->distanceMeters, b->distanceMeters);
  if (cmp != 0) {
    return -cmp;
  }

  // tie-break by higher docID:
  return b->docID - a->docID;
}

double NearestNeighbor::approxBestDistance(std::deque<char> &minPackedValue,
                                           std::deque<char> &maxPackedValue,
                                           double pointLat, double pointLon)
{
  double minLat = decodeLatitude(minPackedValue, 0);
  double minLon = decodeLongitude(minPackedValue, Integer::BYTES);
  double maxLat = decodeLatitude(maxPackedValue, 0);
  double maxLon = decodeLongitude(maxPackedValue, Integer::BYTES);
  return approxBestDistance(minLat, maxLat, minLon, maxLon, pointLat, pointLon);
}

double NearestNeighbor::approxBestDistance(double minLat, double maxLat,
                                           double minLon, double maxLon,
                                           double pointLat, double pointLon)
{

  // TODO: can we make this the trueBestDistance?  I.e., minimum distance
  // between the point and ANY point on the box?  we can speed things up if so,
  // but not enrolling any BKD cell whose true best distance is > bottom of the
  // current hit queue

  if (pointLat >= minLat && pointLat <= maxLat && pointLon >= minLon &&
      pointLon <= maxLon) {
    // point is inside the cell!
    return 0.0;
  }

  double d1 = SloppyMath::haversinMeters(pointLat, pointLon, minLat, minLon);
  double d2 = SloppyMath::haversinMeters(pointLat, pointLon, minLat, maxLon);
  double d3 = SloppyMath::haversinMeters(pointLat, pointLon, maxLat, maxLon);
  double d4 = SloppyMath::haversinMeters(pointLat, pointLon, maxLat, minLon);
  return min(min(d1, d2), min(d3, d4));
}
} // namespace org::apache::lucene::document