using namespace std;

#include "AEProviderFactory.h"
#include "AEProvider.h"
#include "BasicAEProvider.h"
#include "OverridingParamsAEProvider.h"

namespace org::apache::lucene::analysis::uima::ae
{

const shared_ptr<AEProviderFactory> AEProviderFactory::instance =
    make_shared<AEProviderFactory>();

AEProviderFactory::AEProviderFactory()
{
  // Singleton
}

shared_ptr<AEProviderFactory> AEProviderFactory::getInstance()
{
  return instance;
}

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<AEProvider>
AEProviderFactory::getAEProvider(const wstring &keyPrefix,
                                 const wstring &aePath,
                                 unordered_map<wstring, any> &runtimeParameters)
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  wstring key =
      (make_shared<StringBuilder>(keyPrefix != L"" ? keyPrefix : L""))
          ->append(aePath)
          ->append(runtimeParameters.size() > 0 ? runtimeParameters.toString()
                                                : L"")
          ->toString();
  if (providerCache[key] == nullptr) {
    shared_ptr<AEProvider> aeProvider;
    if (runtimeParameters.size() > 0) {
      aeProvider =
          make_shared<OverridingParamsAEProvider>(aePath, runtimeParameters);
    } else {
      aeProvider = make_shared<BasicAEProvider>(aePath);
    }
    providerCache.emplace(key, aeProvider);
  }
  return providerCache[key];
}

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<AEProvider> AEProviderFactory::getAEProvider(const wstring &aePath)
{
  return getAEProvider(L"", aePath, nullptr);
}

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<AEProvider>
AEProviderFactory::getAEProvider(const wstring &aePath,
                                 unordered_map<wstring, any> &runtimeParameters)
{
  return getAEProvider(L"", aePath, runtimeParameters);
}
} // namespace org::apache::lucene::analysis::uima::ae