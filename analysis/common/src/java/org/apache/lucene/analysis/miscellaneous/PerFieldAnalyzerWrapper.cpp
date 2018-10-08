using namespace std;

#include "PerFieldAnalyzerWrapper.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using DelegatingAnalyzerWrapper =
    org::apache::lucene::analysis::DelegatingAnalyzerWrapper;

PerFieldAnalyzerWrapper::PerFieldAnalyzerWrapper(
    shared_ptr<Analyzer> defaultAnalyzer)
    : PerFieldAnalyzerWrapper(defaultAnalyzer, nullptr)
{
}

PerFieldAnalyzerWrapper::PerFieldAnalyzerWrapper(
    shared_ptr<Analyzer> defaultAnalyzer,
    unordered_map<wstring, std::shared_ptr<Analyzer>> &fieldAnalyzers)
    : org::apache::lucene::analysis::DelegatingAnalyzerWrapper(
          PER_FIELD_REUSE_STRATEGY),
      defaultAnalyzer(defaultAnalyzer),
      fieldAnalyzers(
          (fieldAnalyzers.size() > 0)
              ? fieldAnalyzers
              : Collections::emptyMap<wstring, std::shared_ptr<Analyzer>>())
{
}

shared_ptr<Analyzer>
PerFieldAnalyzerWrapper::getWrappedAnalyzer(const wstring &fieldName)
{
  shared_ptr<Analyzer> analyzer = fieldAnalyzers[fieldName];
  return (analyzer != nullptr) ? analyzer : defaultAnalyzer;
}

wstring PerFieldAnalyzerWrapper::toString()
{
  return L"PerFieldAnalyzerWrapper(" + fieldAnalyzers + L", default=" +
         defaultAnalyzer + L")";
}
} // namespace org::apache::lucene::analysis::miscellaneous