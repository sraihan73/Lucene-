using namespace std;

#include "SimpleBindings.h"

namespace org::apache::lucene::expressions
{
using DoubleValuesSource = org::apache::lucene::search::DoubleValuesSource;
using SortField = org::apache::lucene::search::SortField;

SimpleBindings::SimpleBindings() {}

void SimpleBindings::add(shared_ptr<SortField> sortField)
{
  map_obj.emplace(sortField->getField(), sortField);
}

void SimpleBindings::add(const wstring &name,
                         shared_ptr<DoubleValuesSource> source)
{
  map_obj.emplace(name, source);
}

void SimpleBindings::add(const wstring &name, shared_ptr<Expression> expression)
{
  map_obj.emplace(name, expression);
}

shared_ptr<DoubleValuesSource>
SimpleBindings::getDoubleValuesSource(const wstring &name)
{
  any o = map_obj[name];
  if (o == nullptr) {
    throw invalid_argument(L"Invalid reference '" + name + L"'");
  } else if (std::dynamic_pointer_cast<Expression>(o) != nullptr) {
    return (any_cast<std::shared_ptr<Expression>>(o))
        .getDoubleValuesSource(shared_from_this());
  } else if (std::dynamic_pointer_cast<DoubleValuesSource>(o) != nullptr) {
    return (any_cast<std::shared_ptr<DoubleValuesSource>>(o));
  }
  shared_ptr<SortField> field = any_cast<std::shared_ptr<SortField>>(o);
  switch (field->getType()) {
  case INT:
    return DoubleValuesSource::fromIntField(field->getField());
  case LONG:
    return DoubleValuesSource::fromLongField(field->getField());
  case FLOAT:
    return DoubleValuesSource::fromFloatField(field->getField());
  case DOUBLE:
    return DoubleValuesSource::fromDoubleField(field->getField());
  case SCORE:
    return DoubleValuesSource::SCORES;
  default:
    throw make_shared<UnsupportedOperationException>();
  }
}

void SimpleBindings::validate()
{
  for (auto o : map_obj) {
    if (std::dynamic_pointer_cast<Expression>(o.second) != nullptr) {
      shared_ptr<Expression> expr =
          std::static_pointer_cast<Expression>(o.second);
      try {
        expr->getDoubleValuesSource(shared_from_this());
      } catch (const StackOverflowError &e) {
        throw invalid_argument(
            L"Recursion Error: Cycle detected originating in (" +
            expr->sourceText + L")");
      }
    }
  }
}
} // namespace org::apache::lucene::expressions