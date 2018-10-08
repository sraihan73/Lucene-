using namespace std;

#include "Expression.h"
#include "js/JavascriptCompiler.h"

namespace org::apache::lucene::expressions
{
using JavascriptCompiler =
    org::apache::lucene::expressions::js::JavascriptCompiler;
using DoubleValues = org::apache::lucene::search::DoubleValues;
using DoubleValuesSource = org::apache::lucene::search::DoubleValuesSource;
using Rescorer = org::apache::lucene::search::Rescorer;
using SortField = org::apache::lucene::search::SortField;

Expression::Expression(const wstring &sourceText,
                       std::deque<wstring> &variables)
    : sourceText(sourceText), variables(variables)
{
}

shared_ptr<DoubleValuesSource>
Expression::getDoubleValuesSource(shared_ptr<Bindings> bindings)
{
  return make_shared<ExpressionValueSource>(bindings, shared_from_this());
}

shared_ptr<SortField> Expression::getSortField(shared_ptr<Bindings> bindings,
                                               bool reverse)
{
  return getDoubleValuesSource(bindings)->getSortField(reverse);
}

shared_ptr<Rescorer> Expression::getRescorer(shared_ptr<Bindings> bindings)
{
  return make_shared<ExpressionRescorer>(shared_from_this(), bindings);
}
} // namespace org::apache::lucene::expressions