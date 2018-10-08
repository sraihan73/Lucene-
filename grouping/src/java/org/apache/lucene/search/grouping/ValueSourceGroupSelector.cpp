using namespace std;

#include "ValueSourceGroupSelector.h"

namespace org::apache::lucene::search::grouping
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using FunctionValues = org::apache::lucene::queries::function::FunctionValues;
using ValueSource = org::apache::lucene::queries::function::ValueSource;
using MutableValue = org::apache::lucene::util::mutable_::MutableValue;

template <typename T1, typename T1>
ValueSourceGroupSelector::ValueSourceGroupSelector(
    shared_ptr<ValueSource> valueSource, unordered_map<T1> context)
    : valueSource(valueSource), context(context)
{
}

void ValueSourceGroupSelector::setNextReader(
    shared_ptr<LeafReaderContext> readerContext) 
{
  shared_ptr<FunctionValues> values =
      valueSource->getValues(context, readerContext);
  this->filler = values->getValueFiller();
}

State ValueSourceGroupSelector::advanceTo(int doc) 
{
  this->filler->fillValue(doc);
  if (secondPassGroups != nullptr) {
    if (secondPassGroups->contains(filler->getValue()) == false) {
      return State::SKIP;
    }
  }
  return State::ACCEPT;
}

shared_ptr<MutableValue> ValueSourceGroupSelector::currentValue()
{
  return filler->getValue();
}

shared_ptr<MutableValue> ValueSourceGroupSelector::copyValue()
{
  return filler->getValue()->duplicate();
}

void ValueSourceGroupSelector::setGroups(
    shared_ptr<deque<SearchGroup<std::shared_ptr<MutableValue>>>>
        searchGroups)
{
  secondPassGroups = unordered_set<>();
  for (auto group : searchGroups) {
    secondPassGroups->add(group->groupValue);
  }
}
} // namespace org::apache::lucene::search::grouping