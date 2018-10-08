using namespace std;

#include "LongValuesSource.h"

namespace org::apache::lucene::search
{
using DocValues = org::apache::lucene::index::DocValues;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;

shared_ptr<SortField> LongValuesSource::getSortField(bool reverse)
{
  return make_shared<LongValuesSortField>(shared_from_this(), reverse);
}

shared_ptr<DoubleValuesSource> LongValuesSource::toDoubleValuesSource()
{
  return make_shared<DoubleLongValuesSource>(shared_from_this());
}

LongValuesSource::DoubleLongValuesSource::DoubleLongValuesSource(
    shared_ptr<LongValuesSource> inner)
    : inner(inner)
{
}

shared_ptr<DoubleValues> LongValuesSource::DoubleLongValuesSource::getValues(
    shared_ptr<LeafReaderContext> ctx,
    shared_ptr<DoubleValues> scores) 
{
  shared_ptr<LongValues> v = inner->getValues(ctx, scores);
  return make_shared<DoubleValuesAnonymousInnerClass>(shared_from_this(), v);
}

LongValuesSource::DoubleLongValuesSource::DoubleValuesAnonymousInnerClass::
    DoubleValuesAnonymousInnerClass(
        shared_ptr<DoubleLongValuesSource> outerInstance,
        shared_ptr<org::apache::lucene::search::LongValues> v)
{
  this->outerInstance = outerInstance;
  this->v = v;
}

double LongValuesSource::DoubleLongValuesSource::
    DoubleValuesAnonymousInnerClass::doubleValue() 
{
  return static_cast<double>(v->longValue());
}

bool LongValuesSource::DoubleLongValuesSource::DoubleValuesAnonymousInnerClass::
    advanceExact(int doc) 
{
  return v->advanceExact(doc);
}

shared_ptr<DoubleValuesSource>
LongValuesSource::DoubleLongValuesSource::rewrite(
    shared_ptr<IndexSearcher> searcher) 
{
  return inner->rewrite(searcher)->toDoubleValuesSource();
}

bool LongValuesSource::DoubleLongValuesSource::isCacheable(
    shared_ptr<LeafReaderContext> ctx)
{
  return inner->isCacheable(ctx);
}

wstring LongValuesSource::DoubleLongValuesSource::toString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return L"double(" + inner->toString() + L")";
}

bool LongValuesSource::DoubleLongValuesSource::needsScores()
{
  return inner->needsScores();
}

bool LongValuesSource::DoubleLongValuesSource::equals(any o)
{
  if (shared_from_this() == o) {
    return true;
  }
  if (o == nullptr || getClass() != o.type()) {
    return false;
  }
  shared_ptr<DoubleLongValuesSource> that =
      any_cast<std::shared_ptr<DoubleLongValuesSource>>(o);
  return Objects::equals(inner, that->inner);
}

int LongValuesSource::DoubleLongValuesSource::hashCode()
{
  return Objects::hash(inner);
}

shared_ptr<LongValuesSource>
LongValuesSource::fromLongField(const wstring &field)
{
  return make_shared<FieldValuesSource>(field);
}

shared_ptr<LongValuesSource>
LongValuesSource::fromIntField(const wstring &field)
{
  return fromLongField(field);
}

shared_ptr<LongValuesSource> LongValuesSource::constant(int64_t value)
{
  return make_shared<ConstantLongValuesSource>(value);
}

LongValuesSource::ConstantLongValuesSource::ConstantLongValuesSource(
    int64_t value)
    : value(value)
{
}

shared_ptr<LongValues> LongValuesSource::ConstantLongValuesSource::getValues(
    shared_ptr<LeafReaderContext> ctx,
    shared_ptr<DoubleValues> scores) 
{
  return make_shared<LongValuesAnonymousInnerClass>(shared_from_this());
}

LongValuesSource::ConstantLongValuesSource::LongValuesAnonymousInnerClass::
    LongValuesAnonymousInnerClass(
        shared_ptr<ConstantLongValuesSource> outerInstance)
{
  this->outerInstance = outerInstance;
}

int64_t LongValuesSource::ConstantLongValuesSource::
    LongValuesAnonymousInnerClass::longValue() 
{
  return outerInstance->value;
}

bool LongValuesSource::ConstantLongValuesSource::LongValuesAnonymousInnerClass::
    advanceExact(int doc) 
{
  return true;
}

bool LongValuesSource::ConstantLongValuesSource::isCacheable(
    shared_ptr<LeafReaderContext> ctx)
{
  return true;
}

bool LongValuesSource::ConstantLongValuesSource::needsScores() { return false; }

int LongValuesSource::ConstantLongValuesSource::hashCode()
{
  return Objects::hash(value);
}

bool LongValuesSource::ConstantLongValuesSource::equals(any o)
{
  if (shared_from_this() == o) {
    return true;
  }
  if (o == nullptr || getClass() != o.type()) {
    return false;
  }
  shared_ptr<ConstantLongValuesSource> that =
      any_cast<std::shared_ptr<ConstantLongValuesSource>>(o);
  return value == that->value;
}

wstring LongValuesSource::ConstantLongValuesSource::toString()
{
  return L"constant(" + to_wstring(value) + L")";
}

shared_ptr<LongValuesSource>
LongValuesSource::ConstantLongValuesSource::rewrite(
    shared_ptr<IndexSearcher> searcher) 
{
  return shared_from_this();
}

LongValuesSource::FieldValuesSource::FieldValuesSource(const wstring &field)
    : field(field)
{
}

bool LongValuesSource::FieldValuesSource::equals(any o)
{
  if (shared_from_this() == o) {
    return true;
  }
  if (o == nullptr || getClass() != o.type()) {
    return false;
  }
  shared_ptr<FieldValuesSource> that =
      any_cast<std::shared_ptr<FieldValuesSource>>(o);
  return Objects::equals(field, that->field);
}

wstring LongValuesSource::FieldValuesSource::toString()
{
  return L"long(" + field + L")";
}

int LongValuesSource::FieldValuesSource::hashCode()
{
  return Objects::hash(field);
}

shared_ptr<LongValues> LongValuesSource::FieldValuesSource::getValues(
    shared_ptr<LeafReaderContext> ctx,
    shared_ptr<DoubleValues> scores) 
{
  shared_ptr<NumericDocValues> *const values =
      DocValues::getNumeric(ctx->reader(), field);
  return toLongValues(values);
}

bool LongValuesSource::FieldValuesSource::isCacheable(
    shared_ptr<LeafReaderContext> ctx)
{
  return DocValues::isCacheable(ctx, {field});
}

bool LongValuesSource::FieldValuesSource::needsScores() { return false; }

shared_ptr<LongValuesSource> LongValuesSource::FieldValuesSource::rewrite(
    shared_ptr<IndexSearcher> searcher) 
{
  return shared_from_this();
}

LongValuesSource::LongValuesSortField::LongValuesSortField(
    shared_ptr<LongValuesSource> producer, bool reverse)
    : SortField(producer->toString(), new LongValuesComparatorSource(producer),
                reverse),
      producer(producer)
{
}

bool LongValuesSource::LongValuesSortField::needsScores()
{
  return producer->needsScores();
}

wstring LongValuesSource::LongValuesSortField::toString()
{
  shared_ptr<StringBuilder> buffer = make_shared<StringBuilder>(L"<");
  buffer->append(getField())->append(L">");
  if (reverse) {
    buffer->append(L"!");
  }
  return buffer->toString();
}

shared_ptr<SortField> LongValuesSource::LongValuesSortField::rewrite(
    shared_ptr<IndexSearcher> searcher) 
{
  return make_shared<LongValuesSortField>(producer->rewrite(searcher), reverse);
}

LongValuesSource::LongValuesComparatorSource::LongValuesComparatorSource(
    shared_ptr<LongValuesSource> producer)
    : producer(producer)
{
}

shared_ptr<FieldComparator<int64_t>>
LongValuesSource::LongValuesComparatorSource::newComparator(
    const wstring &fieldname, int numHits, int sortPos, bool reversed)
{
  return make_shared<LongComparatorAnonymousInnerClass>(shared_from_this(),
                                                        numHits, fieldname);
}

LongValuesSource::LongValuesComparatorSource::
    LongComparatorAnonymousInnerClass::LongComparatorAnonymousInnerClass(
        shared_ptr<LongValuesComparatorSource> outerInstance, int numHits,
        const wstring &fieldname)
    : FieldComparator::LongComparator(numHits, fieldname, 0LL)
{
  this->outerInstance = outerInstance;
  holder = make_shared<LongValuesHolder>();
}

shared_ptr<NumericDocValues> LongValuesSource::LongValuesComparatorSource::
    LongComparatorAnonymousInnerClass::getNumericDocValues(
        shared_ptr<LeafReaderContext> context,
        const wstring &field) 
{
  ctx = context;
  return asNumericDocValues(holder);
}

void LongValuesSource::LongValuesComparatorSource::
    LongComparatorAnonymousInnerClass::setScorer(
        shared_ptr<Scorer> scorer) 
{
  holder->values = outerInstance->producer->getValues(
      ctx, DoubleValuesSource::fromScorer(scorer));
}

shared_ptr<LongValues>
LongValuesSource::toLongValues(shared_ptr<NumericDocValues> in_)
{
  return make_shared<LongValuesAnonymousInnerClass>(in_);
}

LongValuesSource::LongValuesAnonymousInnerClass::LongValuesAnonymousInnerClass(
    shared_ptr<NumericDocValues> in_)
{
  this->in_ = in_;
}

int64_t
LongValuesSource::LongValuesAnonymousInnerClass::longValue() 
{
  return in_->longValue();
}

bool LongValuesSource::LongValuesAnonymousInnerClass::advanceExact(
    int target) 
{
  return in_->advanceExact(target);
}

shared_ptr<NumericDocValues>
LongValuesSource::asNumericDocValues(shared_ptr<LongValuesHolder> in_)
{
  return make_shared<NumericDocValuesAnonymousInnerClass>(in_);
}

LongValuesSource::NumericDocValuesAnonymousInnerClass::
    NumericDocValuesAnonymousInnerClass(
        shared_ptr<
            org::apache::lucene::search::LongValuesSource::LongValuesHolder>
            in_)
{
  this->in_ = in_;
}

int64_t
LongValuesSource::NumericDocValuesAnonymousInnerClass::longValue() throw(
    IOException)
{
  return in_->values->longValue();
}

bool LongValuesSource::NumericDocValuesAnonymousInnerClass::advanceExact(
    int target) 
{
  return in_->values->advanceExact(target);
}

int LongValuesSource::NumericDocValuesAnonymousInnerClass::docID()
{
  throw make_shared<UnsupportedOperationException>();
}

int LongValuesSource::NumericDocValuesAnonymousInnerClass::nextDoc() throw(
    IOException)
{
  throw make_shared<UnsupportedOperationException>();
}

int LongValuesSource::NumericDocValuesAnonymousInnerClass::advance(
    int target) 
{
  throw make_shared<UnsupportedOperationException>();
}

int64_t LongValuesSource::NumericDocValuesAnonymousInnerClass::cost()
{
  throw make_shared<UnsupportedOperationException>();
}
} // namespace org::apache::lucene::search