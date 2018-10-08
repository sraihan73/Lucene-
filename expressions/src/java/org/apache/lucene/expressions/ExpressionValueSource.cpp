using namespace std;

#include "ExpressionValueSource.h"

namespace org::apache::lucene::expressions
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using DoubleValues = org::apache::lucene::search::DoubleValues;
using DoubleValuesSource = org::apache::lucene::search::DoubleValuesSource;
using Explanation = org::apache::lucene::search::Explanation;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;

ExpressionValueSource::ExpressionValueSource(shared_ptr<Bindings> bindings,
                                             shared_ptr<Expression> expression)
    : variables(std::deque<std::shared_ptr<DoubleValuesSource>>(
          expression->variables.size())),
      expression(Objects::requireNonNull(expression)), needsScores(needsScores_)
{
  if (bindings == nullptr) {
    throw make_shared<NullPointerException>();
  }
  bool needsScores = false;
  for (int i = 0; i < variables.size(); i++) {
    shared_ptr<DoubleValuesSource> source =
        bindings->getDoubleValuesSource(expression->variables[i]);
    if (source == nullptr) {
      throw runtime_error(L"Internal error. Variable (" +
                          expression->variables[i] + L") does not exist.");
    }
    needsScores |= source->needsScores();
    variables[i] = source;
  }
}

ExpressionValueSource::ExpressionValueSource(
    std::deque<std::shared_ptr<DoubleValuesSource>> &variables,
    shared_ptr<Expression> expression, bool needsScores)
    : variables(variables), expression(expression), needsScores(needsScores)
{
}

shared_ptr<DoubleValues> ExpressionValueSource::getValues(
    shared_ptr<LeafReaderContext> readerContext,
    shared_ptr<DoubleValues> scores) 
{
  unordered_map<wstring, std::shared_ptr<DoubleValues>> valuesCache =
      unordered_map<wstring, std::shared_ptr<DoubleValues>>();
  std::deque<std::shared_ptr<DoubleValues>> externalValues(
      expression->variables.size());

  for (int i = 0; i < variables.size(); ++i) {
    wstring externalName = expression->variables[i];
    shared_ptr<DoubleValues> values = valuesCache[externalName];
    if (values == nullptr) {
      values = variables[i]->getValues(readerContext, scores);
      if (values == nullptr) {
        throw runtime_error(L"Internal error. External (" + externalName +
                            L") does not exist.");
      }
      valuesCache.emplace(externalName, values);
    }
    externalValues[i] = zeroWhenUnpositioned(values);
  }

  return make_shared<ExpressionFunctionValues>(expression, externalValues);
}

shared_ptr<DoubleValues>
ExpressionValueSource::zeroWhenUnpositioned(shared_ptr<DoubleValues> in_)
{
  return make_shared<DoubleValuesAnonymousInnerClass>(in_);
}

ExpressionValueSource::DoubleValuesAnonymousInnerClass::
    DoubleValuesAnonymousInnerClass(shared_ptr<DoubleValues> in_)
{
  this->in_ = in_;
}

double
ExpressionValueSource::DoubleValuesAnonymousInnerClass::doubleValue() throw(
    IOException)
{
  return positioned ? in_->doubleValue() : 0;
}

bool ExpressionValueSource::DoubleValuesAnonymousInnerClass::advanceExact(
    int doc) 
{
  return positioned = in_->advanceExact(doc);
}

wstring ExpressionValueSource::toString()
{
  return L"expr(" + expression->sourceText + L")";
}

int ExpressionValueSource::hashCode()
{
  constexpr int prime = 31;
  int result = 1;
  result = prime * result +
           ((expression == nullptr) ? 0 : expression->sourceText.hashCode());
  result = prime * result + (needsScores_ ? 1231 : 1237);
  result = prime * result + Arrays::hashCode(variables);
  return result;
}

bool ExpressionValueSource::equals(any obj)
{
  if (shared_from_this() == obj) {
    return true;
  }
  if (obj == nullptr) {
    return false;
  }
  if (getClass() != obj.type()) {
    return false;
  }
  shared_ptr<ExpressionValueSource> other =
      any_cast<std::shared_ptr<ExpressionValueSource>>(obj);
  if (expression == nullptr) {
    if (other->expression != nullptr) {
      return false;
    }
  } else if (expression->sourceText != other->expression->sourceText) {
    return false;
  }
  if (needsScores_ != other->needsScores_) {
    return false;
  }
  if (!Arrays::equals(variables, other->variables)) {
    return false;
  }
  return true;
}

bool ExpressionValueSource::needsScores() { return needsScores_; }

bool ExpressionValueSource::isCacheable(shared_ptr<LeafReaderContext> ctx)
{
  for (auto v : variables) {
    if (v->isCacheable(ctx) == false) {
      return false;
    }
  }
  return true;
}

shared_ptr<Explanation> ExpressionValueSource::explain(
    shared_ptr<LeafReaderContext> ctx, int docId,
    shared_ptr<Explanation> scoreExplanation) 
{
  std::deque<std::shared_ptr<Explanation>> explanations(variables.size());
  shared_ptr<DoubleValues> dv =
      getValues(ctx, DoubleValuesSource::constant(scoreExplanation->getValue())
                         ->getValues(ctx, nullptr));
  if (dv->advanceExact(docId) == false) {
    return Explanation::noMatch(expression->sourceText);
  }
  int i = 0;
  for (auto var : variables) {
    explanations[i++] = var->explain(ctx, docId, scoreExplanation);
  }
  return Explanation::match(static_cast<float>(dv->doubleValue()),
                            expression->sourceText + L", computed from:",
                            explanations);
}

shared_ptr<DoubleValuesSource> ExpressionValueSource::rewrite(
    shared_ptr<IndexSearcher> searcher) 
{
  bool changed = false;
  std::deque<std::shared_ptr<DoubleValuesSource>> rewritten(variables.size());
  for (int i = 0; i < variables.size(); i++) {
    rewritten[i] = variables[i]->rewrite(searcher);
    changed |= (rewritten[i] == variables[i]);
  }
  if (changed) {
    return make_shared<ExpressionValueSource>(rewritten, expression,
                                              needsScores_);
  }
  return shared_from_this();
}
} // namespace org::apache::lucene::expressions