using namespace std;

#include "HeatmapFacetCounter.h"

namespace org::apache::lucene::spatial::prefix
{
using IndexReaderContext = org::apache::lucene::index::IndexReaderContext;
using Cell = org::apache::lucene::spatial::prefix::tree::Cell;
using CellIterator = org::apache::lucene::spatial::prefix::tree::CellIterator;
using SpatialPrefixTree =
    org::apache::lucene::spatial::prefix::tree::SpatialPrefixTree;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using Bits = org::apache::lucene::util::Bits;
using org::locationtech::spatial4j::context::SpatialContext;
using org::locationtech::spatial4j::shape::Point;
using org::locationtech::spatial4j::shape::Rectangle;
using org::locationtech::spatial4j::shape::Shape;
using org::locationtech::spatial4j::shape::SpatialRelation;

HeatmapFacetCounter::StaticConstructor::StaticConstructor()
{
  Math::multiplyExact(
      MAX_ROWS_OR_COLUMNS,
      MAX_ROWS_OR_COLUMNS); // will throw if doesn't stay within integer
}

HeatmapFacetCounter::StaticConstructor HeatmapFacetCounter::staticConstructor;

HeatmapFacetCounter::Heatmap::Heatmap(int columns, int rows,
                                      shared_ptr<Rectangle> region)
    : columns(columns), rows(rows), counts(std::deque<int>(columns * rows)),
      region(region)
{
}

int HeatmapFacetCounter::Heatmap::getCount(int x, int y)
{
  return counts[x * rows + y];
}

wstring HeatmapFacetCounter::Heatmap::toString()
{
  return L"Heatmap{" + to_wstring(columns) + L"x" + to_wstring(rows) + L" " +
         region + L'}';
}

shared_ptr<Heatmap> HeatmapFacetCounter::calcFacets(
    shared_ptr<PrefixTreeStrategy> strategy,
    shared_ptr<IndexReaderContext> context, shared_ptr<Bits> topAcceptDocs,
    shared_ptr<Shape> inputShape, int const facetLevel,
    int maxCells) 
{
  if (maxCells > (MAX_ROWS_OR_COLUMNS * MAX_ROWS_OR_COLUMNS)) {
    throw invalid_argument(L"maxCells (" + to_wstring(maxCells) +
                           L") should be <= " +
                           to_wstring(MAX_ROWS_OR_COLUMNS));
  }
  if (inputShape == nullptr) {
    inputShape = strategy->getSpatialContext()->getWorldBounds();
  }
  shared_ptr<Rectangle> *const inputRect = inputShape->getBoundingBox();
  // First get the rect of the cell at the bottom-left at depth facetLevel
  shared_ptr<SpatialPrefixTree> *const grid = strategy->getGrid();
  shared_ptr<SpatialContext> *const ctx = grid->getSpatialContext();
  shared_ptr<Point> *const cornerPt =
      ctx->makePoint(inputRect->getMinX(), inputRect->getMinY());
  shared_ptr<CellIterator> *const cellIterator =
      grid->getTreeCellIterator(cornerPt, facetLevel);
  shared_ptr<Cell> cornerCell = nullptr;
  while (cellIterator->hasNext()) {
    cornerCell = cellIterator->next();
    cellIterator++;
  }
  assert((cornerCell != nullptr && cornerCell->getLevel() == facetLevel,
          L"Cell not at target level: " + cornerCell));
  shared_ptr<Rectangle> *const cornerRect =
      std::static_pointer_cast<Rectangle>(cornerCell->getShape());
  assert(cornerRect->hasArea());
  // Now calculate the number of columns and rows necessary to cover the
  // inputRect
  double heatMinX = cornerRect->getMinX(); // note: we might change this
                                           // below...
  constexpr double cellWidth = cornerRect->getWidth();
  shared_ptr<Rectangle> *const worldRect = ctx->getWorldBounds();
  constexpr int columns =
      calcRowsOrCols(cellWidth, heatMinX, inputRect->getWidth(),
                     inputRect->getMinX(), worldRect->getWidth());
  constexpr double heatMinY = cornerRect->getMinY();
  constexpr double cellHeight = cornerRect->getHeight();
  constexpr int rows =
      calcRowsOrCols(cellHeight, heatMinY, inputRect->getHeight(),
                     inputRect->getMinY(), worldRect->getHeight());
  assert(rows > 0 && columns > 0);
  if (columns > MAX_ROWS_OR_COLUMNS || rows > MAX_ROWS_OR_COLUMNS ||
      columns * rows > maxCells) {
    throw invalid_argument(L"Too many cells (" + to_wstring(columns) + L" x " +
                           to_wstring(rows) + L") for level " +
                           to_wstring(facetLevel) + L" shape " + inputRect);
  }

  // Create resulting heatmap bounding rectangle & Heatmap object.
  constexpr double halfCellWidth = cellWidth / 2.0;
  // if X world-wraps, use world bounds' range
  if (columns * cellWidth + halfCellWidth > worldRect->getWidth()) {
    heatMinX = worldRect->getMinX();
  }
  double heatMaxX = heatMinX + columns * cellWidth;
  if (abs(heatMaxX - worldRect->getMaxX()) <
      halfCellWidth) { // numeric conditioning issue
    heatMaxX = worldRect->getMaxX();
  } else if (heatMaxX >
             worldRect->getMaxX()) { // wraps dateline (won't happen if !geo)
    heatMaxX = heatMaxX - worldRect->getMaxX() + worldRect->getMinX();
  }
  constexpr double halfCellHeight = cellHeight / 2.0;
  double heatMaxY = heatMinY + rows * cellHeight;
  if (abs(heatMaxY - worldRect->getMaxY()) <
      halfCellHeight) { // numeric conditioning issue
    heatMaxY = worldRect->getMaxY();
  }

  shared_ptr<Heatmap> *const heatmap = make_shared<Heatmap>(
      columns, rows,
      ctx->makeRectangle(heatMinX, heatMaxX, heatMinY, heatMaxY));
  if (std::dynamic_pointer_cast<Bits::MatchNoBits>(topAcceptDocs) != nullptr) {
    return heatmap; // short-circuit
  }

  // All ancestor cell counts (of facetLevel) will be captured during facet
  // visiting and applied later. If the data is
  // just points then there won't be any ancestors.
  // Facet count of ancestors covering all of the heatmap:
  std::deque<int> allCellsAncestorCount(
      1); // single-element array so it can be accumulated in the inner class
  // All other ancestors:
  unordered_map<std::shared_ptr<Rectangle>, int> ancestors =
      unordered_map<std::shared_ptr<Rectangle>, int>();

  // Now lets count some facets!
  PrefixTreeFacetCounter::compute(
      strategy, context, topAcceptDocs, inputShape, facetLevel,
      make_shared<FacetVisitorAnonymousInnerClass>(
          facetLevel, heatMinX, cellWidth, heatMinY, cellHeight, heatmap,
          allCellsAncestorCount, ancestors));

  // Update the heatmap counts with ancestor counts

  // Apply allCellsAncestorCount
  if (allCellsAncestorCount[0] > 0) {
    for (int i = 0; i < heatmap->counts.size(); i++) {
      heatmap->counts[i] += allCellsAncestorCount[0];
    }
  }

  // Apply ancestors
  //  note: This approach isn't optimized for a ton of ancestor cells. We'll
  //  potentially increment the same cells
  //    multiple times in separate passes if any ancestors overlap. IF this
  //    poses a problem, we could optimize it with additional complication by
  //    keeping track of intervals in a sorted tree structure (possible
  //    TreeMap/Set) and iterate them cleverly such that we just make one pass
  //    at this stage.

  std::deque<int> pair(2); // output of intersectInterval
  for (auto entry : ancestors) {
    shared_ptr<Rectangle> rect =
        entry.first; // from a cell (thus doesn't cross DL)
    constexpr int count = entry.second;

    // note: we approach this in a way that eliminates int overflow/underflow
    // (think huge cell, tiny heatmap)
    intersectInterval(heatMinY, heatMaxY, cellHeight, rows, rect->getMinY(),
                      rect->getMaxY(), pair);
    constexpr int startRow = pair[0];
    constexpr int endRow = pair[1];

    if (!heatmap->region->getCrossesDateLine()) {
      intersectInterval(heatMinX, heatMaxX, cellWidth, columns, rect->getMinX(),
                        rect->getMaxX(), pair);
      constexpr int startCol = pair[0];
      constexpr int endCol = pair[1];
      incrementRange(heatmap, startCol, endCol, startRow, endRow, count);

    } else {
      // note: the cell rect might intersect 2 disjoint parts of the heatmap, so
      // we do the left & right separately
      constexpr int leftColumns =
          static_cast<int>(round((180 - heatMinX) / cellWidth));
      constexpr int rightColumns = heatmap->columns - leftColumns;
      // left half of dateline:
      if (rect->getMaxX() > heatMinX) {
        intersectInterval(heatMinX, 180, cellWidth, leftColumns,
                          rect->getMinX(), rect->getMaxX(), pair);
        constexpr int startCol = pair[0];
        constexpr int endCol = pair[1];
        incrementRange(heatmap, startCol, endCol, startRow, endRow, count);
      }
      // right half of dateline
      if (rect->getMinX() < heatMaxX) {
        intersectInterval(-180, heatMaxX, cellWidth, rightColumns,
                          rect->getMinX(), rect->getMaxX(), pair);
        constexpr int startCol = pair[0] + leftColumns;
        constexpr int endCol = pair[1] + leftColumns;
        incrementRange(heatmap, startCol, endCol, startRow, endRow, count);
      }
    }
  }

  return heatmap;
}

HeatmapFacetCounter::FacetVisitorAnonymousInnerClass::
    FacetVisitorAnonymousInnerClass(
        int facetLevel, double heatMinX, double cellWidth, double heatMinY,
        double cellHeight,
        shared_ptr<
            org::apache::lucene::spatial::prefix::HeatmapFacetCounter::Heatmap>
            heatmap,
        deque<int> &allCellsAncestorCount,
        unordered_map<std::shared_ptr<Rectangle>, int> &ancestors)
{
  this->facetLevel = facetLevel;
  this->heatMinX = heatMinX;
  this->cellWidth = cellWidth;
  this->heatMinY = heatMinY;
  this->cellHeight = cellHeight;
  this->heatmap = heatmap;
  this->allCellsAncestorCount = allCellsAncestorCount;
  this->ancestors = ancestors;
}

void HeatmapFacetCounter::FacetVisitorAnonymousInnerClass::visit(
    shared_ptr<Cell> cell, int count)
{
  constexpr double heatMinX = heatmap->region->getMinX();
  shared_ptr<Rectangle> *const rect =
      std::static_pointer_cast<Rectangle>(cell->getShape());
  if (cell->getLevel() == facetLevel) { // heatmap level; count it directly
    // convert to col & row
    int column;
    if (rect->getMinX() >= heatMinX) {
      column =
          static_cast<int>(round((rect->getMinX() - heatMinX) / cellWidth));
    } else { // due to dateline wrap
      column = static_cast<int>(
          round((rect->getMinX() + 360 - heatMinX) / cellWidth));
    }
    int row =
        static_cast<int>(round((rect->getMinY() - heatMinY) / cellHeight));
    // note: unfortunately, it's possible for us to visit adjacent cells to the
    // heatmap (if the SpatialPrefixTree
    // allows adjacent cells to overlap on the seam), so we need to skip them
    if (column < 0 || column >= heatmap->columns || row < 0 ||
        row >= heatmap->rows) {
      return;
    }
    // increment
    heatmap->counts[column * heatmap->rows + row] += count;

  } else if (rect->relate(heatmap->region) ==
             SpatialRelation::CONTAINS) { // containing ancestor
    allCellsAncestorCount[0] += count;

  } else { // ancestor
    // note: not particularly efficient (possible put twice, and Integer
    // wrapper); oh well
    optional<int> existingCount = ancestors.emplace(rect, count);
    if (existingCount) {
      ancestors.emplace(rect, count + existingCount);
    }
  }
}

void HeatmapFacetCounter::intersectInterval(double heatMin, double heatMax,
                                            double heatCellLen, int numCells,
                                            double cellMin, double cellMax,
                                            std::deque<int> &out)
{
  assert(heatMin < heatMax && cellMin < cellMax);
  // precondition: we know there's an intersection
  if (heatMin >= cellMin) {
    out[0] = 0;
  } else {
    out[0] = static_cast<int>(round((cellMin - heatMin) / heatCellLen));
  }
  if (heatMax <= cellMax) {
    out[1] = numCells - 1;
  } else {
    out[1] = static_cast<int>(round((cellMax - heatMin) / heatCellLen)) - 1;
  }
}

void HeatmapFacetCounter::incrementRange(shared_ptr<Heatmap> heatmap,
                                         int startColumn, int endColumn,
                                         int startRow, int endRow, int count)
{
  // startColumn & startRow are not necessarily within the heatmap range;
  // likewise numRows/columns may overlap.
  if (startColumn < 0) {
    endColumn += startColumn;
    startColumn = 0;
  }
  endColumn = min(heatmap->columns - 1, endColumn);

  if (startRow < 0) {
    endRow += startRow;
    startRow = 0;
  }
  endRow = min(heatmap->rows - 1, endRow);

  if (startRow > endRow) {
    return; // short-circuit
  }
  for (int c = startColumn; c <= endColumn; c++) {
    int cBase = c * heatmap->rows;
    for (int r = startRow; r <= endRow; r++) {
      heatmap->counts[cBase + r] += count;
    }
  }
}

int HeatmapFacetCounter::calcRowsOrCols(double cellRange, double cellMin,
                                        double requestRange, double requestMin,
                                        double worldRange)
{
  assert(requestMin >= cellMin);
  // Idealistically this wouldn't be so complicated but we concern ourselves
  // with overflow and edge cases
  double range = (requestRange + (requestMin - cellMin));
  if (range == 0) {
    return 1;
  }
  constexpr double intervals = ceil(range / cellRange);
  if (intervals > numeric_limits<int>::max()) {
    return numeric_limits<int>::max(); // should result in an error soon (exceed
                                       // thresholds)
  }
  // ensures we don't have more intervals than world bounds (possibly due to
  // rounding/edge issue)
  constexpr int64_t intervalsMax = round(worldRange / cellRange);
  if (intervalsMax > numeric_limits<int>::max()) {
    // just return intervals
    return static_cast<int>(intervals);
  }
  return min(static_cast<int>(intervalsMax), static_cast<int>(intervals));
}

HeatmapFacetCounter::HeatmapFacetCounter() {}
} // namespace org::apache::lucene::spatial::prefix