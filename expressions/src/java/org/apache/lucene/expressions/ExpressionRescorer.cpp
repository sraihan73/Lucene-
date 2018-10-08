using namespace std;

#include "ExpressionRescorer.h"

namespace org::apache::lucene::expressions
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using ReaderUtil = org::apache::lucene::index::ReaderUtil;
using DoubleValues = org::apache::lucene::search::DoubleValues;
using Explanation = org::apache::lucene::search::Explanation;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Rescorer = org::apache::lucene::search::Rescorer;
using Sort = org::apache::lucene::search::Sort;
using SortRescorer = org::apache::lucene::search::SortRescorer;

ExpressionRescorer::ExpressionRescorer(shared_ptr<Expression> expression,
                                       shared_ptr<Bindings> bindings)
    : org::apache::lucene::search::SortRescorer(
          new Sort(expression->getSortField(bindings, true))),
      expression(expression), bindings(bindings)
{
}

shared_ptr<DoubleValues> ExpressionRescorer::scores(int doc, float score)
{
  return make_shared<DoubleValuesAnonymousInnerClass>(doc, score);
}

ExpressionRescorer::DoubleValuesAnonymousInnerClass::
    DoubleValuesAnonymousInnerClass(int doc, float score)
{
  this->doc = doc;
  this->score = score;
}

double ExpressionRescorer::DoubleValuesAnonymousInnerClass::doubleValue() throw(
    IOException)
{
  return score;
}

bool ExpressionRescorer::DoubleValuesAnonymousInnerClass::advanceExact(
    int target) 
{
  assert(doc == target);
  return true;
}

shared_ptr<Explanation>
ExpressionRescorer::explain(shared_ptr<IndexSearcher> searcher,
                            shared_ptr<Explanation> firstPassExplanation,
                            int docID) 
{
  shared_ptr<Explanation> superExpl =
      SortRescorer::explain(searcher, firstPassExplanation, docID);

  deque<std::shared_ptr<LeafReaderContext>> leaves =
      searcher->getIndexReader()->leaves();
  int subReader = ReaderUtil::subIndex(docID, leaves);
  shared_ptr<LeafReaderContext> readerContext = leaves[subReader];
  int docIDInSegment = docID - readerContext->docBase;

  return expression->getDoubleValuesSource(bindings)->explain(
      readerContext, docIDInSegment, superExpl);
}
} // namespace org::apache::lucene::expressions