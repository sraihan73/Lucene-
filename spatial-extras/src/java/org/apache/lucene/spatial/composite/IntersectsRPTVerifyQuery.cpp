using namespace std;

#include "IntersectsRPTVerifyQuery.h"

namespace org::apache::lucene::spatial::composite
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using ConstantScoreScorer = org::apache::lucene::search::ConstantScoreScorer;
using ConstantScoreWeight = org::apache::lucene::search::ConstantScoreWeight;
using DocIdSet = org::apache::lucene::search::DocIdSet;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using Scorer = org::apache::lucene::search::Scorer;
using TwoPhaseIterator = org::apache::lucene::search::TwoPhaseIterator;
using Weight = org::apache::lucene::search::Weight;
using AbstractVisitingPrefixTreeQuery =
    org::apache::lucene::spatial::prefix::AbstractVisitingPrefixTreeQuery;
using Cell = org::apache::lucene::spatial::prefix::tree::Cell;
using SpatialPrefixTree =
    org::apache::lucene::spatial::prefix::tree::SpatialPrefixTree;
using ShapeValuesPredicate =
    org::apache::lucene::spatial::util::ShapeValuesPredicate;
using DocIdSetBuilder = org::apache::lucene::util::DocIdSetBuilder;
using org::locationtech::spatial4j::shape::Shape;
using org::locationtech::spatial4j::shape::SpatialRelation;

IntersectsRPTVerifyQuery::IntersectsRPTVerifyQuery(
    shared_ptr<Shape> queryShape, const wstring &fieldName,
    shared_ptr<SpatialPrefixTree> grid, int detailLevel,
    int prefixGridScanLevel,
    shared_ptr<ShapeValuesPredicate> predicateValueSource)
    : intersectsDiffQuery(make_shared<IntersectsDifferentiatingQuery>(
          queryShape, fieldName, grid, detailLevel, prefixGridScanLevel)),
      predicateValueSource(predicateValueSource)
{
}

wstring IntersectsRPTVerifyQuery::toString(const wstring &field)
{
  return L"IntersectsVerified(fieldName=" + field + L")";
}

bool IntersectsRPTVerifyQuery::equals(any other)
{
  return sameClassAs(other) && equalsTo(getClass().cast(other));
}

bool IntersectsRPTVerifyQuery::equalsTo(
    shared_ptr<IntersectsRPTVerifyQuery> other)
{
  return intersectsDiffQuery->equals(other->intersectsDiffQuery) &&
         predicateValueSource->equals(other->predicateValueSource);
}

int IntersectsRPTVerifyQuery::hashCode()
{
  int result = classHash();
  result = 31 * result + intersectsDiffQuery->hashCode();
  result = 31 * result + predicateValueSource->hashCode();
  return result;
}

shared_ptr<Weight>
IntersectsRPTVerifyQuery::createWeight(shared_ptr<IndexSearcher> searcher,
                                       bool needsScores,
                                       float boost) 
{

  return make_shared<ConstantScoreWeightAnonymousInnerClass>(shared_from_this(),
                                                             boost);
}

IntersectsRPTVerifyQuery::ConstantScoreWeightAnonymousInnerClass::
    ConstantScoreWeightAnonymousInnerClass(
        shared_ptr<IntersectsRPTVerifyQuery> outerInstance, float boost)
    : org::apache::lucene::search::ConstantScoreWeight(outerInstance, boost)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Scorer>
IntersectsRPTVerifyQuery::ConstantScoreWeightAnonymousInnerClass::scorer(
    shared_ptr<LeafReaderContext> context) 
{
  // Compute approx & exact
  shared_ptr<IntersectsDifferentiatingQuery::IntersectsDifferentiatingVisitor>
      *const result = outerInstance->intersectsDiffQuery->compute(context);
  if (result->approxDocIdSet == nullptr) {
    return nullptr;
  }
  shared_ptr<DocIdSetIterator> *const approxDISI =
      result->approxDocIdSet->begin();
  if (approxDISI == nullptr) {
    return nullptr;
  }
  shared_ptr<DocIdSetIterator> *const exactIterator;
  if (result->exactDocIdSet != nullptr) {
    // If both sets are the same, there's nothing to verify; we needn't return a
    // TwoPhaseIterator
    if (result->approxDocIdSet == result->exactDocIdSet) {
      return make_shared<ConstantScoreScorer>(shared_from_this(), score(),
                                              approxDISI);
    }
    exactIterator = result->exactDocIdSet->begin();
    assert(exactIterator != nullptr);
  } else {
    exactIterator.reset();
  }

  shared_ptr<TwoPhaseIterator> *const twoPhaseIterator =
      make_shared<TwoPhaseIteratorAnonymousInnerClass>(
          shared_from_this(), context, approxDISI, exactIterator);

  return make_shared<ConstantScoreScorer>(shared_from_this(), score(),
                                          twoPhaseIterator);
}

IntersectsRPTVerifyQuery::ConstantScoreWeightAnonymousInnerClass::
    TwoPhaseIteratorAnonymousInnerClass::TwoPhaseIteratorAnonymousInnerClass(
        shared_ptr<ConstantScoreWeightAnonymousInnerClass> outerInstance,
        shared_ptr<LeafReaderContext> context,
        shared_ptr<DocIdSetIterator> approxDISI,
        shared_ptr<DocIdSetIterator> exactIterator)
    : org::apache::lucene::search::TwoPhaseIterator(approxDISI)
{
  this->outerInstance = outerInstance;
  this->context = context;
  this->approxDISI = approxDISI;
  this->exactIterator = exactIterator;
  predFuncValues = outerInstance->outerInstance.predicateValueSource->iterator(
      context, approxDISI);
}

bool IntersectsRPTVerifyQuery::ConstantScoreWeightAnonymousInnerClass::
    TwoPhaseIteratorAnonymousInnerClass::matches() 
{
  constexpr int doc = approxDISI->docID();
  if (exactIterator != nullptr) {
    if (exactIterator->docID() < doc) {
      exactIterator->advance(doc);
    }
    if (exactIterator->docID() == doc) {
      return true;
    }
  }
  return predFuncValues::matches();
}

float IntersectsRPTVerifyQuery::ConstantScoreWeightAnonymousInnerClass::
    TwoPhaseIteratorAnonymousInnerClass::matchCost()
{
  return 100; // TODO: use cost of exactIterator.advance() and
              // predFuncValues.cost()
}

bool IntersectsRPTVerifyQuery::ConstantScoreWeightAnonymousInnerClass::
    isCacheable(shared_ptr<LeafReaderContext> ctx)
{
  return outerInstance->predicateValueSource->isCacheable(ctx);
}

IntersectsRPTVerifyQuery::IntersectsDifferentiatingQuery::
    IntersectsDifferentiatingQuery(shared_ptr<Shape> queryShape,
                                   const wstring &fieldName,
                                   shared_ptr<SpatialPrefixTree> grid,
                                   int detailLevel, int prefixGridScanLevel)
    : org::apache::lucene::spatial::prefix::AbstractVisitingPrefixTreeQuery(
          queryShape, fieldName, grid, detailLevel, prefixGridScanLevel)
{
}

shared_ptr<IntersectsDifferentiatingQuery::IntersectsDifferentiatingVisitor>
IntersectsRPTVerifyQuery::IntersectsDifferentiatingQuery::compute(
    shared_ptr<LeafReaderContext> context) 
{
  shared_ptr<IntersectsDifferentiatingQuery::IntersectsDifferentiatingVisitor>
      *const result = make_shared<
          IntersectsDifferentiatingQuery::IntersectsDifferentiatingVisitor>(
          shared_from_this(), context);
  result->getDocIdSet(); // computes
  return result;
}

IntersectsRPTVerifyQuery::IntersectsDifferentiatingQuery::
    IntersectsDifferentiatingVisitor::IntersectsDifferentiatingVisitor(
        shared_ptr<IntersectsRPTVerifyQuery::IntersectsDifferentiatingQuery>
            outerInstance,
        shared_ptr<LeafReaderContext> context) 
    : VisitorTemplate(outerInstance, context), outerInstance(outerInstance)
{
}

void IntersectsRPTVerifyQuery::IntersectsDifferentiatingQuery::
    IntersectsDifferentiatingVisitor::start() 
{
  approxBuilder = make_shared<DocIdSetBuilder>(maxDoc, terms);
  exactBuilder = make_shared<DocIdSetBuilder>(maxDoc, terms);
}

shared_ptr<DocIdSet> IntersectsRPTVerifyQuery::IntersectsDifferentiatingQuery::
    IntersectsDifferentiatingVisitor::finish() 
{
  if (exactIsEmpty) {
    exactDocIdSet.reset();
  } else {
    exactDocIdSet = exactBuilder->build();
  }
  if (approxIsEmpty) {
    approxDocIdSet = exactDocIdSet; // optimization
  } else {
    if (exactDocIdSet != nullptr) {
      approxBuilder->add(exactDocIdSet->begin());
    }
    approxDocIdSet = approxBuilder->build();
  }
  return nullptr; // unused in this weird re-use of AVPTQ
}

bool IntersectsRPTVerifyQuery::IntersectsDifferentiatingQuery::
    IntersectsDifferentiatingVisitor::visitPrefix(shared_ptr<Cell> cell) throw(
        IOException)
{
  if (cell->getShapeRel() == SpatialRelation::WITHIN) {
    exactIsEmpty = false;
    collectDocs(exactBuilder); // note: we'll add exact to approx on finish()
    return false;
  } else if (cell->getLevel() == outerInstance->detailLevel) {
    approxIsEmpty = false;
    collectDocs(approxBuilder);
    return false;
  }
  return true;
}

void IntersectsRPTVerifyQuery::IntersectsDifferentiatingQuery::
    IntersectsDifferentiatingVisitor::visitLeaf(shared_ptr<Cell> cell) throw(
        IOException)
{
  if (cell->getShapeRel() == SpatialRelation::WITHIN) {
    exactIsEmpty = false;
    collectDocs(exactBuilder); // note: we'll add exact to approx on finish()
  } else {
    approxIsEmpty = false;
    collectDocs(approxBuilder);
  }
}

shared_ptr<DocIdSet>
IntersectsRPTVerifyQuery::IntersectsDifferentiatingQuery::getDocIdSet(
    shared_ptr<LeafReaderContext> context) 
{
  throw make_shared<IllegalStateException>();
}

wstring IntersectsRPTVerifyQuery::IntersectsDifferentiatingQuery::toString(
    const wstring &field)
{
  throw make_shared<IllegalStateException>();
}
} // namespace org::apache::lucene::spatial::composite