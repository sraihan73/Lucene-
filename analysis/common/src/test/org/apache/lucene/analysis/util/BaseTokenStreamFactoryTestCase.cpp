using namespace std;

#include "BaseTokenStreamFactoryTestCase.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/Version.h"
#include "../../../../../../java/org/apache/lucene/analysis/util/AbstractAnalysisFactory.h"
#include "../../../../../../java/org/apache/lucene/analysis/util/CharFilterFactory.h"
#include "../../../../../../java/org/apache/lucene/analysis/util/ClasspathResourceLoader.h"
#include "../../../../../../java/org/apache/lucene/analysis/util/ResourceLoader.h"
#include "../../../../../../java/org/apache/lucene/analysis/util/ResourceLoaderAware.h"
#include "../../../../../../java/org/apache/lucene/analysis/util/TokenFilterFactory.h"
#include "../../../../../../java/org/apache/lucene/analysis/util/TokenizerFactory.h"

namespace org::apache::lucene::analysis::util
{
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using Version = org::apache::lucene::util::Version;

shared_ptr<AbstractAnalysisFactory>
BaseTokenStreamFactoryTestCase::analysisFactory(
    type_info clazz, shared_ptr<Version> matchVersion,
    shared_ptr<ResourceLoader> loader,
    deque<wstring> &keysAndValues) 
{
  if (keysAndValues->length % 2 == 1) {
    throw invalid_argument(L"invalid keysAndValues map_obj");
  }
  unordered_map<wstring, wstring> args = unordered_map<wstring, wstring>();
  for (int i = 0; i < keysAndValues->length; i += 2) {
    wstring previous = args.emplace(keysAndValues[i], keysAndValues[i + 1]);
    assertNull(L"duplicate values for key: " + keysAndValues[i], previous);
  }
  if (matchVersion != nullptr) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    wstring previous =
        args.emplace(L"luceneMatchVersion", matchVersion->toString());
    assertNull(L"duplicate values for key: luceneMatchVersion", previous);
  }
  shared_ptr<AbstractAnalysisFactory> factory = nullptr;
  try {
    factory = clazz.getConstructor(unordered_map::typeid).newInstance(args);
  } catch (const InvocationTargetException &e) {
    // to simplify tests that check for illegal parameters
    if (dynamic_cast<invalid_argument>(e->getCause()) != nullptr) {
      throw static_cast<invalid_argument>(e->getCause());
    } else {
      throw e;
    }
  }
  if (std::dynamic_pointer_cast<ResourceLoaderAware>(factory) != nullptr) {
    (std::static_pointer_cast<ResourceLoaderAware>(factory))->inform(loader);
  }
  return factory;
}

shared_ptr<TokenizerFactory> BaseTokenStreamFactoryTestCase::tokenizerFactory(
    const wstring &name, deque<wstring> &keysAndValues) 
{
  return tokenizerFactory(name, Version::LATEST, keysAndValues);
}

shared_ptr<TokenizerFactory> BaseTokenStreamFactoryTestCase::tokenizerFactory(
    const wstring &name, shared_ptr<Version> version,
    deque<wstring> &keysAndValues) 
{
  return tokenizerFactory(name, version,
                          make_shared<ClasspathResourceLoader>(getClass()),
                          {keysAndValues});
}

shared_ptr<TokenizerFactory> BaseTokenStreamFactoryTestCase::tokenizerFactory(
    const wstring &name, shared_ptr<Version> matchVersion,
    shared_ptr<ResourceLoader> loader,
    deque<wstring> &keysAndValues) 
{
  return std::static_pointer_cast<TokenizerFactory>(
      analysisFactory(TokenizerFactory::lookupClass(name), matchVersion, loader,
                      {keysAndValues}));
}

shared_ptr<TokenFilterFactory>
BaseTokenStreamFactoryTestCase::tokenFilterFactory(
    const wstring &name, shared_ptr<Version> version,
    deque<wstring> &keysAndValues) 
{
  return tokenFilterFactory(name, version,
                            make_shared<ClasspathResourceLoader>(getClass()),
                            keysAndValues);
}

shared_ptr<TokenFilterFactory>
BaseTokenStreamFactoryTestCase::tokenFilterFactory(
    const wstring &name, deque<wstring> &keysAndValues) 
{
  return tokenFilterFactory(name, Version::LATEST, keysAndValues);
}

shared_ptr<TokenFilterFactory>
BaseTokenStreamFactoryTestCase::tokenFilterFactory(
    const wstring &name, shared_ptr<Version> matchVersion,
    shared_ptr<ResourceLoader> loader,
    deque<wstring> &keysAndValues) 
{
  return std::static_pointer_cast<TokenFilterFactory>(
      analysisFactory(TokenFilterFactory::lookupClass(name), matchVersion,
                      loader, {keysAndValues}));
}

shared_ptr<CharFilterFactory> BaseTokenStreamFactoryTestCase::charFilterFactory(
    const wstring &name, deque<wstring> &keysAndValues) 
{
  return charFilterFactory(
      name, {Version::LATEST, make_shared<ClasspathResourceLoader>(getClass()),
             keysAndValues});
}

shared_ptr<CharFilterFactory> BaseTokenStreamFactoryTestCase::charFilterFactory(
    const wstring &name, shared_ptr<Version> matchVersion,
    shared_ptr<ResourceLoader> loader,
    deque<wstring> &keysAndValues) 
{
  return std::static_pointer_cast<CharFilterFactory>(
      analysisFactory(CharFilterFactory::lookupClass(name), matchVersion,
                      loader, {keysAndValues}));
}
} // namespace org::apache::lucene::analysis::util