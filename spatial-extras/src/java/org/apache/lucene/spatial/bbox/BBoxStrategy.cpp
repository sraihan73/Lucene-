using namespace std;

#include "BBoxStrategy.h"

namespace org::apache::lucene::spatial::bbox
{
using DoubleDocValuesField =
    org::apache::lucene::document::DoubleDocValuesField;
using DoublePoint = org::apache::lucene::document::DoublePoint;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using StoredField = org::apache::lucene::document::StoredField;
using StringField = org::apache::lucene::document::StringField;
using DocValuesType = org::apache::lucene::index::DocValuesType;
using IndexOptions = org::apache::lucene::index::IndexOptions;
using Term = org::apache::lucene::index::Term;
using BooleanClause = org::apache::lucene::search::BooleanClause;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using ConstantScoreQuery = org::apache::lucene::search::ConstantScoreQuery;
using DoubleValuesSource = org::apache::lucene::search::DoubleValuesSource;
using Query = org::apache::lucene::search::Query;
using TermQuery = org::apache::lucene::search::TermQuery;
using ShapeValuesSource = org::apache::lucene::spatial::ShapeValuesSource;
using SpatialStrategy = org::apache::lucene::spatial::SpatialStrategy;
using SpatialArgs = org::apache::lucene::spatial::query::SpatialArgs;
using SpatialOperation = org::apache::lucene::spatial::query::SpatialOperation;
using UnsupportedSpatialOperation =
    org::apache::lucene::spatial::query::UnsupportedSpatialOperation;
using DistanceToShapeValueSource =
    org::apache::lucene::spatial::util::DistanceToShapeValueSource;
using org::locationtech::spatial4j::context::SpatialContext;
using org::locationtech::spatial4j::shape::Point;
using org::locationtech::spatial4j::shape::Rectangle;
using org::locationtech::spatial4j::shape::Shape;
shared_ptr<org::apache::lucene::document::FieldType>
    BBoxStrategy::DEFAULT_FIELDTYPE;

BBoxStrategy::StaticConstructor::StaticConstructor()
{
  // Default: pointValues + docValues
  shared_ptr<FieldType> type = make_shared<FieldType>();
  type->setDimensions(1, Double::BYTES);          // pointValues (assume Double)
  type->setDocValuesType(DocValuesType::NUMERIC); // docValues
  type->setStored(false);
  type->freeze();
  DEFAULT_FIELDTYPE = type;
}

BBoxStrategy::StaticConstructor BBoxStrategy::staticConstructor;
const wstring BBoxStrategy::SUFFIX_MINX = L"__minX";
const wstring BBoxStrategy::SUFFIX_MAXX = L"__maxX";
const wstring BBoxStrategy::SUFFIX_MINY = L"__minY";
const wstring BBoxStrategy::SUFFIX_MAXY = L"__maxY";
const wstring BBoxStrategy::SUFFIX_XDL = L"__xdl";

shared_ptr<BBoxStrategy>
BBoxStrategy::newInstance(shared_ptr<SpatialContext> ctx,
                          const wstring &fieldNamePrefix)
{
  return make_shared<BBoxStrategy>(ctx, fieldNamePrefix, DEFAULT_FIELDTYPE);
}

BBoxStrategy::BBoxStrategy(shared_ptr<SpatialContext> ctx,
                           const wstring &fieldNamePrefix,
                           shared_ptr<FieldType> fieldType)
    : org::apache::lucene::spatial::SpatialStrategy(ctx, fieldNamePrefix),
      field_bbox(fieldNamePrefix), field_minX(fieldNamePrefix + SUFFIX_MINX),
      field_minY(fieldNamePrefix + SUFFIX_MINY),
      field_maxX(fieldNamePrefix + SUFFIX_MAXX),
      field_maxY(fieldNamePrefix + SUFFIX_MAXY),
      field_xdl(fieldNamePrefix + SUFFIX_XDL), optionsFieldType(fieldType),
      fieldsLen(numQuads * 4 + (xdlFieldType != nullptr ? 1 : 0))
{

  fieldType->freeze();

  int numQuads = 0;
  if ((this->hasStored = fieldType->stored())) {
    numQuads++;
  }
  if ((this->hasDocVals = fieldType->docValuesType() != DocValuesType::NONE)) {
    numQuads++;
  }
  if ((this->hasPointVals = fieldType->pointDimensionCount() > 0)) {
    numQuads++;
  }

  if (hasPointVals) { // if we have an index...
    xdlFieldType = make_shared<FieldType>(StringField::TYPE_NOT_STORED);
    xdlFieldType->setIndexOptions(IndexOptions::DOCS);
    xdlFieldType->freeze();
  } else {
    xdlFieldType.reset();
  }
}

shared_ptr<FieldType> BBoxStrategy::getFieldType() { return optionsFieldType; }

std::deque<std::shared_ptr<Field>>
BBoxStrategy::createIndexableFields(shared_ptr<Shape> shape)
{
  return createIndexableFields(shape->getBoundingBox());
}

std::deque<std::shared_ptr<Field>>
BBoxStrategy::createIndexableFields(shared_ptr<Rectangle> bbox)
{
  std::deque<std::shared_ptr<Field>> fields(fieldsLen);
  int idx = -1;
  if (hasStored) {
    fields[++idx] = make_shared<StoredField>(field_minX, bbox->getMinX());
    fields[++idx] = make_shared<StoredField>(field_minY, bbox->getMinY());
    fields[++idx] = make_shared<StoredField>(field_maxX, bbox->getMaxX());
    fields[++idx] = make_shared<StoredField>(field_maxY, bbox->getMaxY());
  }
  if (hasDocVals) {
    fields[++idx] =
        make_shared<DoubleDocValuesField>(field_minX, bbox->getMinX());
    fields[++idx] =
        make_shared<DoubleDocValuesField>(field_minY, bbox->getMinY());
    fields[++idx] =
        make_shared<DoubleDocValuesField>(field_maxX, bbox->getMaxX());
    fields[++idx] =
        make_shared<DoubleDocValuesField>(field_maxY, bbox->getMaxY());
  }
  if (hasPointVals) {
    fields[++idx] = make_shared<DoublePoint>(field_minX, bbox->getMinX());
    fields[++idx] = make_shared<DoublePoint>(field_minY, bbox->getMinY());
    fields[++idx] = make_shared<DoublePoint>(field_maxX, bbox->getMaxX());
    fields[++idx] = make_shared<DoublePoint>(field_maxY, bbox->getMaxY());
  }
  if (xdlFieldType != nullptr) {
    fields[++idx] = make_shared<Field>(
        field_xdl, bbox->getCrossesDateLine() ? L"T" : L"F", xdlFieldType);
  }
  assert(idx == fields.size() - 1);
  return fields;
}

shared_ptr<ShapeValuesSource> BBoxStrategy::makeShapeValueSource()
{
  return make_shared<BBoxValueSource>(shared_from_this());
}

shared_ptr<DoubleValuesSource>
BBoxStrategy::makeDistanceValueSource(shared_ptr<Point> queryPoint,
                                      double multiplier)
{
  // TODO if makeShapeValueSource gets lifted to the top; this could become a
  // generic impl.
  return make_shared<DistanceToShapeValueSource>(makeShapeValueSource(),
                                                 queryPoint, multiplier, ctx);
}

shared_ptr<DoubleValuesSource>
BBoxStrategy::makeOverlapRatioValueSource(shared_ptr<Rectangle> queryBox,
                                          double queryTargetProportion)
{
  return make_shared<BBoxOverlapRatioValueSource>(makeShapeValueSource(),
                                                  ctx->isGeo(), queryBox,
                                                  queryTargetProportion, 0.0);
}

shared_ptr<Query> BBoxStrategy::makeQuery(shared_ptr<SpatialArgs> args)
{
  shared_ptr<Shape> shape = args->getShape();
  if (!(std::dynamic_pointer_cast<Rectangle>(shape) != nullptr)) {
    throw make_shared<UnsupportedOperationException>(
        L"Can only query by Rectangle, not " + shape);
  }

  shared_ptr<Rectangle> bbox = std::static_pointer_cast<Rectangle>(shape);
  shared_ptr<Query> spatial;

  // Useful for understanding Relations:
  // http://edndoc.esri.com/arcsde/9.1/general_topics/understand_spatial_relations.htm
  shared_ptr<SpatialOperation> op = args->getOperation();
  if (op == SpatialOperation::BBoxIntersects) {
    spatial = makeIntersects(bbox);
  } else if (op == SpatialOperation::BBoxWithin) {
    spatial = makeWithin(bbox);
  } else if (op == SpatialOperation::Contains) {
    spatial = makeContains(bbox);
  } else if (op == SpatialOperation::Intersects) {
    spatial = makeIntersects(bbox);
  } else if (op == SpatialOperation::IsEqualTo) {
    spatial = makeEquals(bbox);
  } else if (op == SpatialOperation::IsDisjointTo) {
    spatial = makeDisjoint(bbox);
  } else if (op == SpatialOperation::IsWithin) {
    spatial = makeWithin(bbox);
  } else { // no Overlaps support yet
    throw make_shared<UnsupportedSpatialOperation>(op);
  }
  return make_shared<ConstantScoreQuery>(spatial);
}

shared_ptr<Query> BBoxStrategy::makeContains(shared_ptr<Rectangle> bbox)
{

  // general case
  // docMinX <= queryExtent.getMinX() AND docMinY <= queryExtent.getMinY() AND
  // docMaxX >= queryExtent.getMaxX() AND docMaxY >= queryExtent.getMaxY()

  // Y conditions
  // docMinY <= queryExtent.getMinY() AND docMaxY >= queryExtent.getMaxY()
  shared_ptr<Query> qMinY = this->makeNumericRangeQuery(
      field_minY, nullopt, bbox->getMinY(), false, true);
  shared_ptr<Query> qMaxY = this->makeNumericRangeQuery(
      field_maxY, bbox->getMaxY(), nullopt, true, false);
  shared_ptr<Query> yConditions =
      this->makeQuery(BooleanClause::Occur::MUST, qMinY, qMaxY);

  // X conditions
  shared_ptr<Query> xConditions;

  // queries that do not cross the date line
  if (!bbox->getCrossesDateLine()) {

    // X Conditions for documents that do not cross the date line,
    // documents that contain the min X and max X of the query envelope,
    // docMinX <= queryExtent.getMinX() AND docMaxX >= queryExtent.getMaxX()
    shared_ptr<Query> qMinX = this->makeNumericRangeQuery(
        field_minX, nullopt, bbox->getMinX(), false, true);
    shared_ptr<Query> qMaxX = this->makeNumericRangeQuery(
        field_maxX, bbox->getMaxX(), nullopt, true, false);
    shared_ptr<Query> qMinMax =
        this->makeQuery(BooleanClause::Occur::MUST, qMinX, qMaxX);
    shared_ptr<Query> qNonXDL = this->makeXDL(false, qMinMax);

    if (!ctx->isGeo()) {
      xConditions = qNonXDL;
    } else {
      // X Conditions for documents that cross the date line,
      // the left portion of the document contains the min X of the query
      // OR the right portion of the document contains the max X of the query,
      // docMinXLeft <= queryExtent.getMinX() OR docMaxXRight >=
      // queryExtent.getMaxX()
      shared_ptr<Query> qXDLLeft = this->makeNumericRangeQuery(
          field_minX, nullopt, bbox->getMinX(), false, true);
      shared_ptr<Query> qXDLRight = this->makeNumericRangeQuery(
          field_maxX, bbox->getMaxX(), nullopt, true, false);
      shared_ptr<Query> qXDLLeftRight =
          this->makeQuery(BooleanClause::Occur::SHOULD, qXDLLeft, qXDLRight);
      shared_ptr<Query> qXDL = this->makeXDL(true, qXDLLeftRight);

      shared_ptr<Query> qEdgeDL = nullptr;
      if (bbox->getMinX() == bbox->getMaxX() && abs(bbox->getMinX()) == 180) {
        double edge = bbox->getMinX() * -1; // opposite dateline edge
        qEdgeDL = makeQuery(BooleanClause::Occur::SHOULD,
                            makeNumberTermQuery(field_minX, edge),
                            makeNumberTermQuery(field_maxX, edge));
      }

      // apply the non-XDL and XDL conditions
      xConditions =
          this->makeQuery(BooleanClause::Occur::SHOULD, qNonXDL, qXDL, qEdgeDL);
    }
  } else {
    // queries that cross the date line

    // No need to search for documents that do not cross the date line

    // X Conditions for documents that cross the date line,
    // the left portion of the document contains the min X of the query
    // AND the right portion of the document contains the max X of the query,
    // docMinXLeft <= queryExtent.getMinX() AND docMaxXRight >=
    // queryExtent.getMaxX()
    shared_ptr<Query> qXDLLeft = this->makeNumericRangeQuery(
        field_minX, nullopt, bbox->getMinX(), false, true);
    shared_ptr<Query> qXDLRight = this->makeNumericRangeQuery(
        field_maxX, bbox->getMaxX(), nullopt, true, false);
    shared_ptr<Query> qXDLLeftRight = this->makeXDL(
        true, this->makeQuery(BooleanClause::Occur::MUST, qXDLLeft, qXDLRight));

    shared_ptr<Query> qWorld = makeQuery(BooleanClause::Occur::MUST,
                                         makeNumberTermQuery(field_minX, -180),
                                         makeNumberTermQuery(field_maxX, 180));

    xConditions =
        makeQuery(BooleanClause::Occur::SHOULD, qXDLLeftRight, qWorld);
  }

  // both X and Y conditions must occur
  return this->makeQuery(BooleanClause::Occur::MUST, xConditions, yConditions);
}

shared_ptr<Query> BBoxStrategy::makeDisjoint(shared_ptr<Rectangle> bbox)
{

  // general case
  // docMinX > queryExtent.getMaxX() OR docMaxX < queryExtent.getMinX() OR
  // docMinY > queryExtent.getMaxY() OR docMaxY < queryExtent.getMinY()

  // Y conditions
  // docMinY > queryExtent.getMaxY() OR docMaxY < queryExtent.getMinY()
  shared_ptr<Query> qMinY = this->makeNumericRangeQuery(
      field_minY, bbox->getMaxY(), nullopt, false, false);
  shared_ptr<Query> qMaxY = this->makeNumericRangeQuery(
      field_maxY, nullopt, bbox->getMinY(), false, false);
  shared_ptr<Query> yConditions =
      this->makeQuery(BooleanClause::Occur::SHOULD, qMinY, qMaxY);

  // X conditions
  shared_ptr<Query> xConditions;

  // queries that do not cross the date line
  if (!bbox->getCrossesDateLine()) {

    // X Conditions for documents that do not cross the date line,
    // docMinX > queryExtent.getMaxX() OR docMaxX < queryExtent.getMinX()
    shared_ptr<Query> qMinX = this->makeNumericRangeQuery(
        field_minX, bbox->getMaxX(), nullopt, false, false);
    if (bbox->getMinX() == -180.0 &&
        ctx->isGeo()) { // touches dateline; -180 == 180
      shared_ptr<BooleanQuery::Builder> bq =
          make_shared<BooleanQuery::Builder>();
      bq->add(qMinX, BooleanClause::Occur::MUST);
      bq->add(makeNumberTermQuery(field_maxX, 180.0),
              BooleanClause::Occur::MUST_NOT);
      qMinX = bq->build();
    }
    shared_ptr<Query> qMaxX = this->makeNumericRangeQuery(
        field_maxX, nullopt, bbox->getMinX(), false, false);

    if (bbox->getMaxX() == 180.0 &&
        ctx->isGeo()) { // touches dateline; -180 == 180
      shared_ptr<BooleanQuery::Builder> bq =
          make_shared<BooleanQuery::Builder>();
      bq->add(qMaxX, BooleanClause::Occur::MUST);
      bq->add(makeNumberTermQuery(field_minX, -180.0),
              BooleanClause::Occur::MUST_NOT);
      qMaxX = bq->build();
    }
    shared_ptr<Query> qMinMax =
        this->makeQuery(BooleanClause::Occur::SHOULD, qMinX, qMaxX);
    shared_ptr<Query> qNonXDL = this->makeXDL(false, qMinMax);

    if (!ctx->isGeo()) {
      xConditions = qNonXDL;
    } else {
      // X Conditions for documents that cross the date line,

      // both the left and right portions of the document must be disjoint to
      // the query (docMinXLeft > queryExtent.getMaxX() OR docMaxXLeft <
      // queryExtent.getMinX()) AND (docMinXRight > queryExtent.getMaxX() OR
      // docMaxXRight < queryExtent.getMinX()) where: docMaxXLeft = 180.0,
      // docMinXRight = -180.0 (docMaxXLeft  < queryExtent.getMinX()) equates to
      // (180.0  < queryExtent.getMinX()) and is ignored (docMinXRight >
      // queryExtent.getMaxX()) equates to (-180.0 > queryExtent.getMaxX()) and
      // is ignored
      shared_ptr<Query> qMinXLeft = this->makeNumericRangeQuery(
          field_minX, bbox->getMaxX(), nullopt, false, false);
      shared_ptr<Query> qMaxXRight = this->makeNumericRangeQuery(
          field_maxX, nullopt, bbox->getMinX(), false, false);
      shared_ptr<Query> qLeftRight =
          this->makeQuery(BooleanClause::Occur::MUST, qMinXLeft, qMaxXRight);
      shared_ptr<Query> qXDL = this->makeXDL(true, qLeftRight);

      // apply the non-XDL and XDL conditions
      xConditions =
          this->makeQuery(BooleanClause::Occur::SHOULD, qNonXDL, qXDL);
    }
    // queries that cross the date line
  } else {

    // X Conditions for documents that do not cross the date line,
    // the document must be disjoint to both the left and right query portions
    // (docMinX > queryExtent.getMaxX()Left OR docMaxX < queryExtent.getMinX())
    // AND (docMinX > queryExtent.getMaxX() OR docMaxX <
    // queryExtent.getMinX()Left) where: queryExtent.getMaxX()Left = 180.0,
    // queryExtent.getMinX()Left = -180.0
    shared_ptr<Query> qMinXLeft =
        this->makeNumericRangeQuery(field_minX, 180.0, nullopt, false, false);
    shared_ptr<Query> qMaxXLeft = this->makeNumericRangeQuery(
        field_maxX, nullopt, bbox->getMinX(), false, false);
    shared_ptr<Query> qMinXRight = this->makeNumericRangeQuery(
        field_minX, bbox->getMaxX(), nullopt, false, false);
    shared_ptr<Query> qMaxXRight =
        this->makeNumericRangeQuery(field_maxX, nullopt, -180.0, false, false);
    shared_ptr<Query> qLeft =
        this->makeQuery(BooleanClause::Occur::SHOULD, qMinXLeft, qMaxXLeft);
    shared_ptr<Query> qRight =
        this->makeQuery(BooleanClause::Occur::SHOULD, qMinXRight, qMaxXRight);
    shared_ptr<Query> qLeftRight =
        this->makeQuery(BooleanClause::Occur::MUST, qLeft, qRight);

    // No need to search for documents that do not cross the date line

    xConditions = this->makeXDL(false, qLeftRight);
  }

  // either X or Y conditions should occur
  return this->makeQuery(BooleanClause::Occur::SHOULD, xConditions,
                         yConditions);
}

shared_ptr<Query> BBoxStrategy::makeEquals(shared_ptr<Rectangle> bbox)
{

  // docMinX = queryExtent.getMinX() AND docMinY = queryExtent.getMinY() AND
  // docMaxX = queryExtent.getMaxX() AND docMaxY = queryExtent.getMaxY()
  shared_ptr<Query> qMinX = makeNumberTermQuery(field_minX, bbox->getMinX());
  shared_ptr<Query> qMinY = makeNumberTermQuery(field_minY, bbox->getMinY());
  shared_ptr<Query> qMaxX = makeNumberTermQuery(field_maxX, bbox->getMaxX());
  shared_ptr<Query> qMaxY = makeNumberTermQuery(field_maxY, bbox->getMaxY());
  return makeQuery(BooleanClause::Occur::MUST, qMinX, qMinY, qMaxX, qMaxY);
}

shared_ptr<Query> BBoxStrategy::makeIntersects(shared_ptr<Rectangle> bbox)
{

  // the original intersects query does not work for envelopes that cross the
  // date line, switch to a NOT Disjoint query

  // MUST_NOT causes a problem when it's the only clause type within a
  // BooleanQuery, to get around it we add all documents as a SHOULD

  // there must be an envelope, it must not be disjoint
  shared_ptr<Query> qHasEnv;
  if (ctx->isGeo()) {
    shared_ptr<Query> qIsNonXDL = this->makeXDL(false);
    shared_ptr<Query> qIsXDL = ctx->isGeo() ? this->makeXDL(true) : nullptr;
    qHasEnv = this->makeQuery(BooleanClause::Occur::SHOULD, qIsNonXDL, qIsXDL);
  } else {
    qHasEnv = this->makeXDL(false);
  }

  shared_ptr<BooleanQuery::Builder> qNotDisjoint =
      make_shared<BooleanQuery::Builder>();
  qNotDisjoint->add(qHasEnv, BooleanClause::Occur::MUST);
  shared_ptr<Query> qDisjoint = makeDisjoint(bbox);
  qNotDisjoint->add(qDisjoint, BooleanClause::Occur::MUST_NOT);

  // Query qDisjoint = makeDisjoint();
  // BooleanQuery qNotDisjoint = new BooleanQuery();
  // qNotDisjoint.add(new MatchAllDocsQuery(),BooleanClause.Occur.SHOULD);
  // qNotDisjoint.add(qDisjoint,BooleanClause.Occur.MUST_NOT);
  return qNotDisjoint->build();
}

shared_ptr<BooleanQuery> BBoxStrategy::makeQuery(BooleanClause::Occur occur,
                                                 deque<Query> &queries)
{
  shared_ptr<BooleanQuery::Builder> bq = make_shared<BooleanQuery::Builder>();
  for (shared_ptr<Query> query : queries) {
    if (query != nullptr) {
      bq->add(query, occur);
    }
  }
  return bq->build();
}

shared_ptr<Query> BBoxStrategy::makeWithin(shared_ptr<Rectangle> bbox)
{

  // general case
  // docMinX >= queryExtent.getMinX() AND docMinY >= queryExtent.getMinY() AND
  // docMaxX <= queryExtent.getMaxX() AND docMaxY <= queryExtent.getMaxY()

  // Y conditions
  // docMinY >= queryExtent.getMinY() AND docMaxY <= queryExtent.getMaxY()
  shared_ptr<Query> qMinY = this->makeNumericRangeQuery(
      field_minY, bbox->getMinY(), nullopt, true, false);
  shared_ptr<Query> qMaxY = this->makeNumericRangeQuery(
      field_maxY, nullopt, bbox->getMaxY(), false, true);
  shared_ptr<Query> yConditions =
      this->makeQuery(BooleanClause::Occur::MUST, qMinY, qMaxY);

  // X conditions
  shared_ptr<Query> xConditions;

  if (ctx->isGeo() && bbox->getMinX() == -180.0 && bbox->getMaxX() == 180.0) {
    // if query world-wraps, only the y condition matters
    return yConditions;

  } else if (!bbox->getCrossesDateLine()) {
    // queries that do not cross the date line

    // docMinX >= queryExtent.getMinX() AND docMaxX <= queryExtent.getMaxX()
    shared_ptr<Query> qMinX = this->makeNumericRangeQuery(
        field_minX, bbox->getMinX(), nullopt, true, false);
    shared_ptr<Query> qMaxX = this->makeNumericRangeQuery(
        field_maxX, nullopt, bbox->getMaxX(), false, true);
    shared_ptr<Query> qMinMax =
        this->makeQuery(BooleanClause::Occur::MUST, qMinX, qMaxX);

    double edge = 0; // none, otherwise opposite dateline of query
    if (bbox->getMinX() == -180.0) {
      edge = 180;
    } else if (bbox->getMaxX() == 180.0) {
      edge = -180;
    }
    if (edge != 0 && ctx->isGeo()) {
      shared_ptr<Query> edgeQ = makeQuery(
          BooleanClause::Occur::MUST, makeNumberTermQuery(field_minX, edge),
          makeNumberTermQuery(field_maxX, edge));
      qMinMax = makeQuery(BooleanClause::Occur::SHOULD, qMinMax, edgeQ);
    }

    xConditions = this->makeXDL(false, qMinMax);

    // queries that cross the date line
  } else {

    // X Conditions for documents that do not cross the date line

    // the document should be within the left portion of the query
    // docMinX >= queryExtent.getMinX() AND docMaxX <= 180.0
    shared_ptr<Query> qMinXLeft = this->makeNumericRangeQuery(
        field_minX, bbox->getMinX(), nullopt, true, false);
    shared_ptr<Query> qMaxXLeft =
        this->makeNumericRangeQuery(field_maxX, nullopt, 180.0, false, true);
    shared_ptr<Query> qLeft =
        this->makeQuery(BooleanClause::Occur::MUST, qMinXLeft, qMaxXLeft);

    // the document should be within the right portion of the query
    // docMinX >= -180.0 AND docMaxX <= queryExtent.getMaxX()
    shared_ptr<Query> qMinXRight =
        this->makeNumericRangeQuery(field_minX, -180.0, nullopt, true, false);
    shared_ptr<Query> qMaxXRight = this->makeNumericRangeQuery(
        field_maxX, nullopt, bbox->getMaxX(), false, true);
    shared_ptr<Query> qRight =
        this->makeQuery(BooleanClause::Occur::MUST, qMinXRight, qMaxXRight);

    // either left or right conditions should occur,
    // apply the left and right conditions to documents that do not cross the
    // date line
    shared_ptr<Query> qLeftRight =
        this->makeQuery(BooleanClause::Occur::SHOULD, qLeft, qRight);
    shared_ptr<Query> qNonXDL = this->makeXDL(false, qLeftRight);

    // X Conditions for documents that cross the date line,
    // the left portion of the document must be within the left portion of the
    // query, AND the right portion of the document must be within the right
    // portion of the query docMinXLeft >= queryExtent.getMinX() AND docMaxXLeft
    // <= 180.0 AND docMinXRight >= -180.0 AND docMaxXRight <=
    // queryExtent.getMaxX()
    shared_ptr<Query> qXDLLeft = this->makeNumericRangeQuery(
        field_minX, bbox->getMinX(), nullopt, true, false);
    shared_ptr<Query> qXDLRight = this->makeNumericRangeQuery(
        field_maxX, nullopt, bbox->getMaxX(), false, true);
    shared_ptr<Query> qXDLLeftRight =
        this->makeQuery(BooleanClause::Occur::MUST, qXDLLeft, qXDLRight);
    shared_ptr<Query> qXDL = this->makeXDL(true, qXDLLeftRight);

    // apply the non-XDL and XDL conditions
    xConditions = this->makeQuery(BooleanClause::Occur::SHOULD, qNonXDL, qXDL);
  }

  // both X and Y conditions must occur
  return this->makeQuery(BooleanClause::Occur::MUST, xConditions, yConditions);
}

shared_ptr<Query> BBoxStrategy::makeXDL(bool crossedDateLine)
{
  // The 'T' and 'F' values match solr fields
  return make_shared<TermQuery>(
      make_shared<Term>(field_xdl, crossedDateLine ? L"T" : L"F"));
}

shared_ptr<Query> BBoxStrategy::makeXDL(bool crossedDateLine,
                                        shared_ptr<Query> query)
{
  if (!ctx->isGeo()) {
    assert(!crossedDateLine);
    return query;
  }
  shared_ptr<BooleanQuery::Builder> bq = make_shared<BooleanQuery::Builder>();
  bq->add(this->makeXDL(crossedDateLine), BooleanClause::Occur::MUST);
  bq->add(query, BooleanClause::Occur::MUST);
  return bq->build();
}

shared_ptr<Query> BBoxStrategy::makeNumberTermQuery(const wstring &field,
                                                    double number)
{
  if (hasPointVals) {
    return DoublePoint::newExactQuery(field, number);
  }
  throw make_shared<UnsupportedOperationException>(
      L"An index is required for this operation.");
}

shared_ptr<Query> BBoxStrategy::makeNumericRangeQuery(const wstring &fieldname,
                                                      optional<double> &min,
                                                      optional<double> &max,
                                                      bool minInclusive,
                                                      bool maxInclusive)
{
  if (hasPointVals) {
    if (!min) {
      min = -numeric_limits<double>::infinity();
    }

    if (!max) {
      max = numeric_limits<double>::infinity();
    }

    if (minInclusive == false) {
      min = Math::nextUp(min);
    }

    if (maxInclusive == false) {
      max = Math::nextDown(max);
    }

    return DoublePoint::newRangeQuery(fieldname, min, max);
  }
  throw make_shared<UnsupportedOperationException>(
      L"An index is required for this operation.");
}
} // namespace org::apache::lucene::spatial::bbox