using namespace std;

#include "NormsFieldExistsQuery.h"

namespace org::apache::lucene::search
{
using StringField = org::apache::lucene::document::StringField;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using FieldInfos = org::apache::lucene::index::FieldInfos;
using LeafReader = org::apache::lucene::index::LeafReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;

NormsFieldExistsQuery::NormsFieldExistsQuery(const wstring &field)
    : field(Objects::requireNonNull(field))
{
}

wstring NormsFieldExistsQuery::getField() { return field; }

bool NormsFieldExistsQuery::equals(any other)
{
  return sameClassAs(other) &&
         field ==
             (any_cast<std::shared_ptr<NormsFieldExistsQuery>>(other)).field;
}

int NormsFieldExistsQuery::hashCode()
{
  return 31 * classHash() + field.hashCode();
}

wstring NormsFieldExistsQuery::toString(const wstring &field)
{
  return L"NormsFieldExistsQuery [field=" + this->field + L"]";
}

shared_ptr<Weight>
NormsFieldExistsQuery::createWeight(shared_ptr<IndexSearcher> searcher,
                                    bool needsScores,
                                    float boost) 
{
  return make_shared<ConstantScoreWeightAnonymousInnerClass>(shared_from_this(),
                                                             boost);
}

NormsFieldExistsQuery::ConstantScoreWeightAnonymousInnerClass::
    ConstantScoreWeightAnonymousInnerClass(
        shared_ptr<NormsFieldExistsQuery> outerInstance, float boost)
    : ConstantScoreWeight(outerInstance, boost)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Scorer>
NormsFieldExistsQuery::ConstantScoreWeightAnonymousInnerClass::scorer(
    shared_ptr<LeafReaderContext> context) 
{
  shared_ptr<FieldInfos> fieldInfos = context->reader()->getFieldInfos();
  shared_ptr<FieldInfo> fieldInfo = fieldInfos->fieldInfo(outerInstance->field);
  if (fieldInfo == nullptr || fieldInfo->hasNorms() == false) {
    return nullptr;
  }
  shared_ptr<LeafReader> reader = context->reader();
  shared_ptr<DocIdSetIterator> iterator =
      reader->getNormValues(outerInstance->field);
  return make_shared<ConstantScoreScorer>(shared_from_this(), score(),
                                          iterator);
}

bool NormsFieldExistsQuery::ConstantScoreWeightAnonymousInnerClass::isCacheable(
    shared_ptr<LeafReaderContext> ctx)
{
  return true;
}
} // namespace org::apache::lucene::search