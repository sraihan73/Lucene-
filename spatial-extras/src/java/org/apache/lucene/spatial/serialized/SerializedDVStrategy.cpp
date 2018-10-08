using namespace std;

#include "SerializedDVStrategy.h"

namespace org::apache::lucene::spatial::serialized
{
using BinaryDocValuesField =
    org::apache::lucene::document::BinaryDocValuesField;
using Field = org::apache::lucene::document::Field;
using BinaryDocValues = org::apache::lucene::index::BinaryDocValues;
using DocValues = org::apache::lucene::index::DocValues;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using ConstantScoreScorer = org::apache::lucene::search::ConstantScoreScorer;
using ConstantScoreWeight = org::apache::lucene::search::ConstantScoreWeight;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using DoubleValuesSource = org::apache::lucene::search::DoubleValuesSource;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using Scorer = org::apache::lucene::search::Scorer;
using TwoPhaseIterator = org::apache::lucene::search::TwoPhaseIterator;
using Weight = org::apache::lucene::search::Weight;
using ShapeValues = org::apache::lucene::spatial::ShapeValues;
using ShapeValuesSource = org::apache::lucene::spatial::ShapeValuesSource;
using SpatialStrategy = org::apache::lucene::spatial::SpatialStrategy;
using SpatialArgs = org::apache::lucene::spatial::query::SpatialArgs;
using DistanceToShapeValueSource =
    org::apache::lucene::spatial::util::DistanceToShapeValueSource;
using ShapeValuesPredicate =
    org::apache::lucene::spatial::util::ShapeValuesPredicate;
using BytesRef = org::apache::lucene::util::BytesRef;
using org::locationtech::spatial4j::context::SpatialContext;
using org::locationtech::spatial4j::io::BinaryCodec;
using org::locationtech::spatial4j::shape::Point;
using org::locationtech::spatial4j::shape::Shape;

SerializedDVStrategy::SerializedDVStrategy(shared_ptr<SpatialContext> ctx,
                                           const wstring &fieldName)
    : org::apache::lucene::spatial::SpatialStrategy(ctx, fieldName)
{
}

std::deque<std::shared_ptr<Field>>
SerializedDVStrategy::createIndexableFields(shared_ptr<Shape> shape)
{
  int bufSize = max(128, static_cast<int>(this->indexLastBufSize *
                                          1.5)); // 50% headroom over last
  shared_ptr<ByteArrayOutputStream> byteStream =
      make_shared<ByteArrayOutputStream>(bufSize);
  shared_ptr<BytesRef> *const bytesRef =
      make_shared<BytesRef>(); // receiver of byteStream's bytes
  try {
    ctx->getBinaryCodec().writeShape(make_shared<DataOutputStream>(byteStream),
                                     shape);
    // this is a hack to avoid redundant byte array copying by
    // byteStream.toByteArray()
    byteStream->writeTo(make_shared<FilterOutputStreamAnonymousInnerClass>(
        shared_from_this(), bytesRef));
  } catch (const IOException &e) {
    throw runtime_error(e);
  }
  this->indexLastBufSize = bytesRef->length; // cache heuristic
  return std::deque<std::shared_ptr<Field>>{
      make_shared<BinaryDocValuesField>(getFieldName(), bytesRef)};
}

SerializedDVStrategy::FilterOutputStreamAnonymousInnerClass::
    FilterOutputStreamAnonymousInnerClass(
        shared_ptr<SerializedDVStrategy> outerInstance,
        shared_ptr<BytesRef> bytesRef)
    : java::io::FilterOutputStream(nullptr)
{
  this->outerInstance = outerInstance;
  this->bytesRef = bytesRef;
}

void SerializedDVStrategy::FilterOutputStreamAnonymousInnerClass::write(
    std::deque<char> &b, int off, int len) 
{
  bytesRef->bytes = b;
  bytesRef->offset = off;
  bytesRef->length = len;
}

shared_ptr<DoubleValuesSource>
SerializedDVStrategy::makeDistanceValueSource(shared_ptr<Point> queryPoint,
                                              double multiplier)
{
  // TODO if makeShapeValueSource gets lifted to the top; this could become a
  // generic impl.
  return make_shared<DistanceToShapeValueSource>(makeShapeValueSource(),
                                                 queryPoint, multiplier, ctx);
}

shared_ptr<Query> SerializedDVStrategy::makeQuery(shared_ptr<SpatialArgs> args)
{
  shared_ptr<ShapeValuesSource> shapeValueSource = makeShapeValueSource();
  shared_ptr<ShapeValuesPredicate> predicateValueSource =
      make_shared<ShapeValuesPredicate>(shapeValueSource, args->getOperation(),
                                        args->getShape());
  return make_shared<PredicateValueSourceQuery>(predicateValueSource);
}

shared_ptr<ShapeValuesSource> SerializedDVStrategy::makeShapeValueSource()
{
  return make_shared<ShapeDocValueSource>(getFieldName(),
                                          ctx->getBinaryCodec());
}

SerializedDVStrategy::PredicateValueSourceQuery::PredicateValueSourceQuery(
    shared_ptr<ShapeValuesPredicate> predicateValueSource)
    : predicateValueSource(predicateValueSource)
{
}

shared_ptr<Weight>
SerializedDVStrategy::PredicateValueSourceQuery::createWeight(
    shared_ptr<IndexSearcher> searcher, bool needsScores,
    float boost) 
{
  return make_shared<ConstantScoreWeightAnonymousInnerClass>(shared_from_this(),
                                                             boost);
}

SerializedDVStrategy::PredicateValueSourceQuery::
    ConstantScoreWeightAnonymousInnerClass::
        ConstantScoreWeightAnonymousInnerClass(
            shared_ptr<PredicateValueSourceQuery> outerInstance, float boost)
    : org::apache::lucene::search::ConstantScoreWeight(outerInstance, boost)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Scorer> SerializedDVStrategy::PredicateValueSourceQuery::
    ConstantScoreWeightAnonymousInnerClass::scorer(
        shared_ptr<LeafReaderContext> context) 
{
  shared_ptr<DocIdSetIterator> approximation =
      DocIdSetIterator::all(context->reader()->maxDoc());
  shared_ptr<TwoPhaseIterator> it =
      outerInstance->predicateValueSource->iterator(context, approximation);
  return make_shared<ConstantScoreScorer>(shared_from_this(), score(), it);
}

bool SerializedDVStrategy::PredicateValueSourceQuery::
    ConstantScoreWeightAnonymousInnerClass::isCacheable(
        shared_ptr<LeafReaderContext> ctx)
{
  return outerInstance->predicateValueSource->isCacheable(ctx);
}

bool SerializedDVStrategy::PredicateValueSourceQuery::equals(any other)
{
  return sameClassAs(other) &&
         predicateValueSource->equals(
             (any_cast<std::shared_ptr<PredicateValueSourceQuery>>(other))
                 .predicateValueSource);
}

int SerializedDVStrategy::PredicateValueSourceQuery::hashCode()
{
  return classHash() + 31 * predicateValueSource->hashCode();
}

wstring
SerializedDVStrategy::PredicateValueSourceQuery::toString(const wstring &field)
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return L"PredicateValueSourceQuery(" + predicateValueSource->toString() +
         L")";
}

SerializedDVStrategy::ShapeDocValueSource::ShapeDocValueSource(
    const wstring &fieldName, shared_ptr<BinaryCodec> binaryCodec)
    : fieldName(fieldName), binaryCodec(binaryCodec)
{
}

shared_ptr<ShapeValues> SerializedDVStrategy::ShapeDocValueSource::getValues(
    shared_ptr<LeafReaderContext> readerContext) 
{
  shared_ptr<BinaryDocValues> *const docValues =
      DocValues::getBinary(readerContext->reader(), fieldName);

  return make_shared<ShapeValuesAnonymousInnerClass>(shared_from_this(),
                                                     docValues);
}

SerializedDVStrategy::ShapeDocValueSource::ShapeValuesAnonymousInnerClass::
    ShapeValuesAnonymousInnerClass(
        shared_ptr<ShapeDocValueSource> outerInstance,
        shared_ptr<BinaryDocValues> docValues)
{
  this->outerInstance = outerInstance;
  this->docValues = docValues;
}

bool SerializedDVStrategy::ShapeDocValueSource::ShapeValuesAnonymousInnerClass::
    advanceExact(int doc) 
{
  return docValues->advanceExact(doc);
}

shared_ptr<Shape> SerializedDVStrategy::ShapeDocValueSource::
    ShapeValuesAnonymousInnerClass::value() 
{
  shared_ptr<BytesRef> bytesRef = docValues->binaryValue();
  shared_ptr<DataInputStream> dataInput =
      make_shared<DataInputStream>(make_shared<ByteArrayInputStream>(
          bytesRef->bytes, bytesRef->offset, bytesRef->length));
  return outerInstance->binaryCodec->readShape(dataInput);
}

bool SerializedDVStrategy::ShapeDocValueSource::isCacheable(
    shared_ptr<LeafReaderContext> ctx)
{
  return DocValues::isCacheable(ctx, {fieldName});
}

bool SerializedDVStrategy::ShapeDocValueSource::equals(any o)
{
  if (shared_from_this() == o) {
    return true;
  }
  if (o == nullptr || getClass() != o.type()) {
    return false;
  }

  shared_ptr<ShapeDocValueSource> that =
      any_cast<std::shared_ptr<ShapeDocValueSource>>(o);

  if (fieldName != that->fieldName) {
    return false;
  }

  return true;
}

int SerializedDVStrategy::ShapeDocValueSource::hashCode()
{
  int result = fieldName.hashCode();
  return result;
}

wstring SerializedDVStrategy::ShapeDocValueSource::toString()
{
  return L"shapeDocVal(" + fieldName + L")";
}
} // namespace org::apache::lucene::spatial::serialized