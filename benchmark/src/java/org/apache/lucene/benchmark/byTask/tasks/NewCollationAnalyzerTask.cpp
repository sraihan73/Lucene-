using namespace std;

#include "NewCollationAnalyzerTask.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../PerfRunData.h"

namespace org::apache::lucene::benchmark::byTask::tasks
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using PerfRunData = org::apache::lucene::benchmark::byTask::PerfRunData;

Implementation
    Implementation::JDK(L"JDK", InnerEnum::JDK,
                        L"org.apache.lucene.collation.CollationKeyAnalyzer",
                        L"java.text.Collator");
Implementation
    Implementation::ICU(L"ICU", InnerEnum::ICU,
                        L"org.apache.lucene.collation.ICUCollationKeyAnalyzer",
                        L"com.ibm.icu.text.Collator");

deque<Implementation> Implementation::valueList;

Implementation::StaticConstructor::StaticConstructor()
{
  valueList.push_back(JDK);
  valueList.push_back(ICU);
}

Implementation::StaticConstructor Implementation::staticConstructor;
int Implementation::nextOrdinal = 0;

NewCollationAnalyzerTask::Implementation::Implementation(
    const wstring &name, InnerEnum innerEnum,
    shared_ptr<NewCollationAnalyzerTask> outerInstance,
    const wstring &className, const wstring &collatorClassName)
    : nameValue(name), ordinalValue(nextOrdinal++), innerEnumValue(innerEnum)
{
  this->outerInstance = outerInstance;
  this->className = className;
  this->collatorClassName = collatorClassName;
}

bool Implementation::operator==(const Implementation &other)
{
  return this->ordinalValue == other.ordinalValue;
}

bool Implementation::operator!=(const Implementation &other)
{
  return this->ordinalValue != other.ordinalValue;
}

deque<Implementation> Implementation::values() { return valueList; }

int Implementation::ordinal() { return ordinalValue; }

wstring Implementation::toString() { return nameValue; }

Implementation Implementation::valueOf(const wstring &name)
{
  for (auto enumInstance : Implementation::valueList) {
    if (enumInstance.nameValue == name) {
      return enumInstance;
    }
  }
}

NewCollationAnalyzerTask::NewCollationAnalyzerTask(
    shared_ptr<PerfRunData> runData)
    : PerfTask(runData)
{
}

shared_ptr<Analyzer> NewCollationAnalyzerTask::createAnalyzer(
    shared_ptr<Locale> locale, Implementation impl) 
{
  constexpr type_info collatorClazz =
      type_info::forName(impl.collatorClassName);
  shared_ptr<Method> collatorMethod =
      collatorClazz.getMethod(L"getInstance", Locale::typeid);
  any collator = collatorMethod->invoke(nullptr, locale);

  constexpr type_info clazz =
      type_info::forName(impl.className).asSubclass(Analyzer::typeid);
  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: Constructor<? extends org.apache.lucene.analysis.Analyzer>
  // ctor = clazz.getConstructor(collatorClazz);
  shared_ptr < Constructor < ? extends Analyzer >> ctor =
                                   clazz.getConstructor(collatorClazz);
  return ctor->newInstance(collator);
}

int NewCollationAnalyzerTask::doLogic() 
{
  try {
    shared_ptr<Locale> locale = getRunData()->getLocale();
    if (locale == nullptr) {
      throw runtime_error(L"Locale must be set with the NewLocale task!");
    }
    shared_ptr<Analyzer> analyzer = createAnalyzer(locale, impl);
    getRunData()->setAnalyzer(analyzer);
    wcout << L"Changed Analyzer to: " << analyzer->getClassName() << L"("
          << locale << L")" << endl;
  } catch (const runtime_error &e) {
    // C++ TODO: This exception's constructor requires only one argument:
    // ORIGINAL LINE: throw new RuntimeException("Error creating Analyzer:
    // impl=" + impl, e);
    throw runtime_error(L"Error creating Analyzer: impl=" + impl);
  }
  return 1;
}

void NewCollationAnalyzerTask::setParams(const wstring &params)
{
  PerfTask::setParams(params);

  shared_ptr<StringTokenizer> st = make_shared<StringTokenizer>(params, L",");
  while (st->hasMoreTokens()) {
    wstring param = st->nextToken();
    shared_ptr<StringTokenizer> expr =
        make_shared<StringTokenizer>(param, L":");
    wstring key = expr->nextToken();
    wstring value = expr->nextToken();
    // for now we only support the "impl" parameter.
    // TODO: add strength, decomposition, etc
    if (key == L"impl") {
      // C++ TODO: The following Java case-insensitive std::wstring method call is not
      // converted:
      if (value.equalsIgnoreCase(L"icu")) {
        impl = Implementation::ICU;
      }
      // C++ TODO: The following Java case-insensitive std::wstring method call is not
      // converted:
      else if (value.equalsIgnoreCase(L"jdk")) {
        impl = Implementation::JDK;
      } else {
        throw runtime_error(L"Unknown parameter " + param);
      }
    } else {
      throw runtime_error(L"Unknown parameter " + param);
    }
  }
}

bool NewCollationAnalyzerTask::supportsParams() { return true; }
} // namespace org::apache::lucene::benchmark::byTask::tasks