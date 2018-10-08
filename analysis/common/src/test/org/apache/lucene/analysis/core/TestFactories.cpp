using namespace std;

#include "TestFactories.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/AttributeFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/Version.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/util/AbstractAnalysisFactory.h"
#include "../../../../../../java/org/apache/lucene/analysis/util/CharFilterFactory.h"
#include "../../../../../../java/org/apache/lucene/analysis/util/MultiTermAwareComponent.h"
#include "../../../../../../java/org/apache/lucene/analysis/util/ResourceLoaderAware.h"
#include "../../../../../../java/org/apache/lucene/analysis/util/TokenFilterFactory.h"
#include "../util/StringMockResourceLoader.h"

namespace org::apache::lucene::analysis::core
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using DelimitedTermFrequencyTokenFilterFactory = org::apache::lucene::analysis::
    miscellaneous::DelimitedTermFrequencyTokenFilterFactory;
using AbstractAnalysisFactory =
    org::apache::lucene::analysis::util::AbstractAnalysisFactory;
using CharFilterFactory =
    org::apache::lucene::analysis::util::CharFilterFactory;
using MultiTermAwareComponent =
    org::apache::lucene::analysis::util::MultiTermAwareComponent;
using ResourceLoaderAware =
    org::apache::lucene::analysis::util::ResourceLoaderAware;
using StringMockResourceLoader =
    org::apache::lucene::analysis::util::StringMockResourceLoader;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;
using TokenizerFactory = org::apache::lucene::analysis::util::TokenizerFactory;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;
using Version = org::apache::lucene::util::Version;
const shared_ptr<java::util::Set<type_info>>
    TestFactories::EXCLUDE_FACTORIES_RANDOM_DATA =
        unordered_set<type_info>(java::util::Arrays::asList(
            org::apache::lucene::analysis::miscellaneous::
                DelimitedTermFrequencyTokenFilterFactory::typeid));

void TestFactories::test() 
{
  for (auto tokenizer : TokenizerFactory::availableTokenizers()) {
    doTestTokenizer(tokenizer);
  }

  for (auto tokenFilter : TokenFilterFactory::availableTokenFilters()) {
    doTestTokenFilter(tokenFilter);
  }

  for (auto charFilter : CharFilterFactory::availableCharFilters()) {
    doTestCharFilter(charFilter);
  }
}

void TestFactories::doTestTokenizer(const wstring &tokenizer) 
{
  type_info factoryClazz = TokenizerFactory::lookupClass(tokenizer);
  shared_ptr<TokenizerFactory> factory =
      std::static_pointer_cast<TokenizerFactory>(initialize(factoryClazz));
  if (factory != nullptr) {
    // we managed to fully create an instance. check a few more things:

    // if it implements MultiTermAware, sanity check its impl
    if (std::dynamic_pointer_cast<MultiTermAwareComponent>(factory) !=
        nullptr) {
      shared_ptr<AbstractAnalysisFactory> mtc =
          (std::static_pointer_cast<MultiTermAwareComponent>(factory))
              ->getMultiTermComponent();
      assertNotNull(mtc);
      // it's not ok to return e.g. a charfilter here: but a tokenizer could
      // wrap a filter around it
      assertFalse(std::dynamic_pointer_cast<CharFilterFactory>(mtc) != nullptr);
    }

    if (!EXCLUDE_FACTORIES_RANDOM_DATA->contains(factory->getClass())) {
      // beast it just a little, it shouldnt throw exceptions:
      // (it should have thrown them in initialize)
      shared_ptr<Analyzer> a =
          make_shared<FactoryAnalyzer>(factory, nullptr, nullptr);
      checkRandomData(random(), a, 20, 20, false, false);
      delete a;
    }
  }
}

void TestFactories::doTestTokenFilter(const wstring &tokenfilter) throw(
    IOException)
{
  type_info factoryClazz = TokenFilterFactory::lookupClass(tokenfilter);
  shared_ptr<TokenFilterFactory> factory =
      std::static_pointer_cast<TokenFilterFactory>(initialize(factoryClazz));
  if (factory != nullptr) {
    // we managed to fully create an instance. check a few more things:

    // if it implements MultiTermAware, sanity check its impl
    if (std::dynamic_pointer_cast<MultiTermAwareComponent>(factory) !=
        nullptr) {
      shared_ptr<AbstractAnalysisFactory> mtc =
          (std::static_pointer_cast<MultiTermAwareComponent>(factory))
              ->getMultiTermComponent();
      assertNotNull(mtc);
      // it's not ok to return a charfilter or tokenizer here, this makes no
      // sense
      assertTrue(std::dynamic_pointer_cast<TokenFilterFactory>(mtc) != nullptr);
    }

    if (!EXCLUDE_FACTORIES_RANDOM_DATA->contains(factory->getClass())) {
      // beast it just a little, it shouldnt throw exceptions:
      // (it should have thrown them in initialize)
      shared_ptr<Analyzer> a =
          make_shared<FactoryAnalyzer>(assertingTokenizer, factory, nullptr);
      checkRandomData(random(), a, 20, 20, false, false);
      delete a;
    }
  }
}

void TestFactories::doTestCharFilter(const wstring &charfilter) throw(
    IOException)
{
  type_info factoryClazz = CharFilterFactory::lookupClass(charfilter);
  shared_ptr<CharFilterFactory> factory =
      std::static_pointer_cast<CharFilterFactory>(initialize(factoryClazz));
  if (factory != nullptr) {
    // we managed to fully create an instance. check a few more things:

    // if it implements MultiTermAware, sanity check its impl
    if (std::dynamic_pointer_cast<MultiTermAwareComponent>(factory) !=
        nullptr) {
      shared_ptr<AbstractAnalysisFactory> mtc =
          (std::static_pointer_cast<MultiTermAwareComponent>(factory))
              ->getMultiTermComponent();
      assertNotNull(mtc);
      // it's not ok to return a tokenizer or tokenfilter here, this makes no
      // sense
      assertTrue(std::dynamic_pointer_cast<CharFilterFactory>(mtc) != nullptr);
    }

    if (!EXCLUDE_FACTORIES_RANDOM_DATA->contains(factory->getClass())) {
      // beast it just a little, it shouldnt throw exceptions:
      // (it should have thrown them in initialize)
      shared_ptr<Analyzer> a =
          make_shared<FactoryAnalyzer>(assertingTokenizer, nullptr, factory);
      checkRandomData(random(), a, 20, 20, false, false);
      delete a;
    }
  }
}

shared_ptr<AbstractAnalysisFactory>
TestFactories::initialize(type_info factoryClazz) 
{
  unordered_map<wstring, wstring> args = unordered_map<wstring, wstring>();
  // C++ TODO: There is no native C++ equivalent to 'toString':
  args.emplace(L"luceneMatchVersion", Version::LATEST->toString());
  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: Constructor<? extends
  // org.apache.lucene.analysis.util.AbstractAnalysisFactory> ctor;
  shared_ptr < Constructor < ? extends AbstractAnalysisFactory >> ctor;
  try {
    ctor = factoryClazz.getConstructor(unordered_map::typeid);
  } catch (const runtime_error &e) {
    throw runtime_error(L"factory '" + factoryClazz +
                        L"' does not have a proper ctor!");
  }

  shared_ptr<AbstractAnalysisFactory> factory = nullptr;
  try {
    factory = ctor->newInstance(args);
  }
  // C++ TODO: There is no equivalent in C++ to Java 'multi-catch' syntax:
  catch (InstantiationException | IllegalAccessException e) {
    throw runtime_error(e);
  } catch (const InvocationTargetException &e) {
    if (dynamic_cast<invalid_argument>(e->getCause()) != nullptr) {
      // it's ok if we dont provide the right parameters to throw this
      return nullptr;
    }
  }

  if (std::dynamic_pointer_cast<ResourceLoaderAware>(factory) != nullptr) {
    try {
      (std::static_pointer_cast<ResourceLoaderAware>(factory))
          ->inform(make_shared<StringMockResourceLoader>(L""));
    } catch (const IOException &ignored) {
      // it's ok if the right files arent available or whatever to throw this
    } catch (const invalid_argument &ignored) {
      // is this ok? I guess so
    }
  }
  return factory;
}

TestFactories::TokenizerFactoryAnonymousInnerClass::
    TokenizerFactoryAnonymousInnerClass(unordered_map<wstring, wstring> &java)
    : org::apache::lucene::analysis::util::TokenizerFactory(
          HashMap<std::wstring, std::wstring>)
{
}

shared_ptr<MockTokenizer>
TestFactories::TokenizerFactoryAnonymousInnerClass::create(
    shared_ptr<AttributeFactory> factory)
{
  return make_shared<MockTokenizer>(factory);
}

TestFactories::FactoryAnalyzer::FactoryAnalyzer(
    shared_ptr<TokenizerFactory> tokenizer,
    shared_ptr<TokenFilterFactory> tokenfilter,
    shared_ptr<CharFilterFactory> charFilter)
    : tokenizer(tokenizer), charFilter(charFilter), tokenfilter(tokenfilter)
{
  assert(tokenizer != nullptr);
}

shared_ptr<Analyzer::TokenStreamComponents>
TestFactories::FactoryAnalyzer::createComponents(const wstring &fieldName)
{
  shared_ptr<Tokenizer> tf =
      tokenizer->create(BaseTokenStreamTestCase::newAttributeFactory());
  if (tokenfilter != nullptr) {
    return make_shared<Analyzer::TokenStreamComponents>(
        tf, tokenfilter->create(tf));
  } else {
    return make_shared<Analyzer::TokenStreamComponents>(tf);
  }
}

shared_ptr<Reader>
TestFactories::FactoryAnalyzer::initReader(const wstring &fieldName,
                                           shared_ptr<Reader> reader)
{
  if (charFilter != nullptr) {
    return charFilter->create(reader);
  } else {
    return reader;
  }
}
} // namespace org::apache::lucene::analysis::core