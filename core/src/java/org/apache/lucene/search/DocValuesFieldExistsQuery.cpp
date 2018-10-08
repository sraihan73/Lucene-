using namespace std;

#include "DocValuesFieldExistsQuery.h"

namespace org::apache::lucene::search
{
using DocValues = org::apache::lucene::index::DocValues;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using LeafReader = org::apache::lucene::index::LeafReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;

DocValuesFieldExistsQuery::DocValuesFieldExistsQuery(const wstring &field)
    : field(Objects::requireNonNull(field))
{
}

wstring DocValuesFieldExistsQuery::getField() { return field; }

bool DocValuesFieldExistsQuery::equals(any other)
{
  return sameClassAs(other) &&
         field == (any_cast<std::shared_ptr<DocValuesFieldExistsQuery>>(other))
                      .field;
}

int DocValuesFieldExistsQuery::hashCode()
{
  return 31 * classHash() + field.hashCode();
}

wstring DocValuesFieldExistsQuery::toString(const wstring &field)
{
  return L"DocValuesFieldExistsQuery [field=" + this->field + L"]";
}

shared_ptr<Weight>
DocValuesFieldExistsQuery::createWeight(shared_ptr<IndexSearcher> searcher,
                                        bool needsScores, float boost)
{
  return make_shared<ConstantScoreWeightAnonymousInnerClass>(shared_from_this(),
                                                             boost);
}

DocValuesFieldExistsQuery::ConstantScoreWeightAnonymousInnerClass::
    ConstantScoreWeightAnonymousInnerClass(
        shared_ptr<DocValuesFieldExistsQuery> outerInstance, float boost)
    : ConstantScoreWeight(outerInstance, boost)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Scorer>
DocValuesFieldExistsQuery::ConstantScoreWeightAnonymousInnerClass::scorer(
    shared_ptr<LeafReaderContext> context) 
{
  shared_ptr<DocIdSetIterator> iterator =
      getDocValuesDocIdSetIterator(outerInstance->field, context->reader());
  if (iterator == nullptr) {
    return nullptr;
  }
  return make_shared<ConstantScoreScorer>(shared_from_this(), score(),
                                          iterator);
}

bool DocValuesFieldExistsQuery::ConstantScoreWeightAnonymousInnerClass::
    isCacheable(shared_ptr<LeafReaderContext> ctx)
{
  return DocValues::isCacheable(ctx, {outerInstance->field});
}

shared_ptr<DocIdSetIterator>
DocValuesFieldExistsQuery::getDocValuesDocIdSetIterator(
    const wstring &field, shared_ptr<LeafReader> reader) 
{
  shared_ptr<FieldInfo> fieldInfo = reader->getFieldInfos()->fieldInfo(field);
  shared_ptr<DocIdSetIterator> *const iterator;
  if (fieldInfo != nullptr) {
    switch (fieldInfo->getDocValuesType()) {
    case NONE:
      iterator.reset();
      break;
    case NUMERIC:
      iterator = reader->getNumericDocValues(field);
      break;
    case BINARY:
      iterator = reader->getBinaryDocValues(field);
      break;
    case SORTED:
      iterator = reader->getSortedDocValues(field);
      break;
    case SORTED_NUMERIC:
      iterator = reader->getSortedNumericDocValues(field);
      break;
    case SORTED_SET:
      iterator = reader->getSortedSetDocValues(field);
      break;
    default:
      throw make_shared<AssertionError>();
    }
    return iterator;
  }
  return nullptr;
}
} // namespace org::apache::lucene::search