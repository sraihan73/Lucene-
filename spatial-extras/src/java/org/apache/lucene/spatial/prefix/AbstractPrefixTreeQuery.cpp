using namespace std;

#include "AbstractPrefixTreeQuery.h"

namespace org::apache::lucene::spatial::prefix
{
using LeafReader = org::apache::lucene::index::LeafReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using ConstantScoreScorer = org::apache::lucene::search::ConstantScoreScorer;
using ConstantScoreWeight = org::apache::lucene::search::ConstantScoreWeight;
using DocIdSet = org::apache::lucene::search::DocIdSet;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using Scorer = org::apache::lucene::search::Scorer;
using Weight = org::apache::lucene::search::Weight;
using SpatialPrefixTree =
    org::apache::lucene::spatial::prefix::tree::SpatialPrefixTree;
using BitSet = org::apache::lucene::util::BitSet;
using DocIdSetBuilder = org::apache::lucene::util::DocIdSetBuilder;
using org::locationtech::spatial4j::shape::Shape;

AbstractPrefixTreeQuery::AbstractPrefixTreeQuery(
    shared_ptr<Shape> queryShape, const wstring &fieldName,
    shared_ptr<SpatialPrefixTree> grid, int detailLevel)
    : queryShape(queryShape), fieldName(fieldName), grid(grid),
      detailLevel(detailLevel)
{
}

bool AbstractPrefixTreeQuery::equals(any o)
{
  return sameClassAs(o) && equalsTo(getClass().cast(o));
}

bool AbstractPrefixTreeQuery::equalsTo(
    shared_ptr<AbstractPrefixTreeQuery> other)
{
  return detailLevel == other->detailLevel && fieldName == other->fieldName &&
         queryShape->equals(other->queryShape);
}

int AbstractPrefixTreeQuery::hashCode()
{
  int result = classHash();
  result = 31 * result + queryShape->hashCode();
  result = 31 * result + fieldName.hashCode();
  result = 31 * result + detailLevel;
  return result;
}

shared_ptr<Weight>
AbstractPrefixTreeQuery::createWeight(shared_ptr<IndexSearcher> searcher,
                                      bool needsScores,
                                      float boost) 
{
  return make_shared<ConstantScoreWeightAnonymousInnerClass>(shared_from_this(),
                                                             boost);
}

AbstractPrefixTreeQuery::ConstantScoreWeightAnonymousInnerClass::
    ConstantScoreWeightAnonymousInnerClass(
        shared_ptr<AbstractPrefixTreeQuery> outerInstance, float boost)
    : org::apache::lucene::search::ConstantScoreWeight(outerInstance, boost)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Scorer>
AbstractPrefixTreeQuery::ConstantScoreWeightAnonymousInnerClass::scorer(
    shared_ptr<LeafReaderContext> context) 
{
  shared_ptr<DocIdSet> docSet = outerInstance->getDocIdSet(context);
  if (docSet == nullptr) {
    return nullptr;
  }
  shared_ptr<DocIdSetIterator> disi = docSet->begin();
  if (disi == nullptr) {
    return nullptr;
  }
  return make_shared<ConstantScoreScorer>(shared_from_this(), score(), disi);
}

bool AbstractPrefixTreeQuery::ConstantScoreWeightAnonymousInnerClass::
    isCacheable(shared_ptr<LeafReaderContext> ctx)
{
  return true;
}

AbstractPrefixTreeQuery::BaseTermsEnumTraverser::BaseTermsEnumTraverser(
    shared_ptr<AbstractPrefixTreeQuery> outerInstance,
    shared_ptr<LeafReaderContext> context) 
    : context(context), maxDoc(reader::maxDoc()),
      terms(reader::terms(outerInstance->fieldName)),
      outerInstance(outerInstance)
{
  shared_ptr<LeafReader> reader = context->reader();
  if (terms != nullptr) {
    this->termsEnum = terms->begin();
  } else {
    this->termsEnum.reset();
  }
}

void AbstractPrefixTreeQuery::BaseTermsEnumTraverser::collectDocs(
    shared_ptr<BitSet> bitSet) 
{
  assert(termsEnum != nullptr);
  postingsEnum = termsEnum->postings(postingsEnum, PostingsEnum::NONE);
  bitSet->or (postingsEnum);
}

void AbstractPrefixTreeQuery::BaseTermsEnumTraverser::collectDocs(
    shared_ptr<DocIdSetBuilder> docSetBuilder) 
{
  assert(termsEnum != nullptr);
  postingsEnum = termsEnum->postings(postingsEnum, PostingsEnum::NONE);
  docSetBuilder->add(postingsEnum);
}
} // namespace org::apache::lucene::spatial::prefix