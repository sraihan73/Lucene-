using namespace std;

#include "AlcoholicMergePolicy.h"

namespace org::apache::lucene::index
{
using TestUtil = org::apache::lucene::util::TestUtil;

AlcoholicMergePolicy::AlcoholicMergePolicy(shared_ptr<TimeZone> tz,
                                           shared_ptr<Random> random)
    : random(random), calendar(make_shared<GregorianCalendar>(tz, Locale::ROOT))
{
  calendar->setTimeInMillis(
      TestUtil::nextLong(random, 0, numeric_limits<int64_t>::max()));
  maxMergeSize =
      TestUtil::nextInt(random, 1024 * 1024, numeric_limits<int>::max());
}

int64_t AlcoholicMergePolicy::size(
    shared_ptr<SegmentCommitInfo> info,
    shared_ptr<MergeContext> mergeContext) 
{
  int hourOfDay = calendar->get(Calendar::HOUR_OF_DAY);
  if (hourOfDay < 6 || hourOfDay > 20 || random->nextInt(23) == 5) {

    std::deque<Drink> values = Drink::values();
    // pick a random drink during the day
    return values[random->nextInt(values.size())].drunkFactor *
           info->sizeInBytes();
  }

  return info->sizeInBytes();
}

Drink Drink::Beer(L"Beer", InnerEnum::Beer, 15);
Drink Drink::Wine(L"Wine", InnerEnum::Wine, 17);
Drink Drink::Champagne(L"Champagne", InnerEnum::Champagne, 21);
Drink Drink::WhiteRussian(L"WhiteRussian", InnerEnum::WhiteRussian, 22);
Drink Drink::SingleMalt(L"SingleMalt", InnerEnum::SingleMalt, 30);

deque<Drink> Drink::valueList;

Drink::StaticConstructor::StaticConstructor()
{
  valueList.push_back(Beer);
  valueList.push_back(Wine);
  valueList.push_back(Champagne);
  valueList.push_back(WhiteRussian);
  valueList.push_back(SingleMalt);
}

Drink::StaticConstructor Drink::staticConstructor;
int Drink::nextOrdinal = 0;

AlcoholicMergePolicy::Drink::Drink(
    const wstring &name, InnerEnum innerEnum,
    shared_ptr<AlcoholicMergePolicy> outerInstance, int64_t drunkFactor)
    : nameValue(name), ordinalValue(nextOrdinal++), innerEnumValue(innerEnum)
{
  this->outerInstance = outerInstance;
  this->drunkFactor = drunkFactor;
}

bool Drink::operator==(const Drink &other)
{
  return this->ordinalValue == other.ordinalValue;
}

bool Drink::operator!=(const Drink &other)
{
  return this->ordinalValue != other.ordinalValue;
}

deque<Drink> Drink::values() { return valueList; }

int Drink::ordinal() { return ordinalValue; }

wstring Drink::toString() { return nameValue; }

Drink Drink::valueOf(const wstring &name)
{
  for (auto enumInstance : Drink::valueList) {
    if (enumInstance.nameValue == name) {
      return enumInstance;
    }
  }
}
} // namespace org::apache::lucene::index