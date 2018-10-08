using namespace std;

#include "OverridingParamsAEProvider.h"

namespace org::apache::lucene::analysis::uima::ae
{
using org::apache::uima::analysis_engine::AnalysisEngineDescription;

OverridingParamsAEProvider::OverridingParamsAEProvider(
    const wstring &aePath, unordered_map<wstring, any> &runtimeParameters)
    : BasicAEProvider(aePath), runtimeParameters(runtimeParameters)
{
}

void OverridingParamsAEProvider::configureDescription(
    shared_ptr<AnalysisEngineDescription> description)
{
  for (auto attributeName : runtimeParameters) {
    any val = getRuntimeValue(description, attributeName.first);
    description->getAnalysisEngineMetaData()
        .getConfigurationParameterSettings()
        .setParameterValue(attributeName.first, val);
  }
}

any OverridingParamsAEProvider::getRuntimeValue(
    shared_ptr<AnalysisEngineDescription> desc, const wstring &attributeName)
{
  wstring type = desc->getAnalysisEngineMetaData()
                     .getConfigurationParameterDeclarations()
                     .getConfigurationParameter(nullptr, attributeName)
                     .getType();
  // TODO : do it via reflection ? i.e. Class paramType = Class.forName(type)...
  any val = nullptr;
  any runtimeValue = runtimeParameters[attributeName];
  if (runtimeValue != nullptr) {
    if (L"std::wstring" == type) {
      val = wstring::valueOf(runtimeValue);
    } else if (L"Integer" == type) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      val = stoi(runtimeValue.toString());
    } else if (L"Boolean" == type) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      val = StringHelper::fromString<bool>(runtimeValue.toString());
    } else if (L"Float" == type) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      val = stof(runtimeValue.toString());
    }
  }

  return val;
}
} // namespace org::apache::lucene::analysis::uima::ae