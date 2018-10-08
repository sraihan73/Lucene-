using namespace std;

#include "TestAllAnalyzersHaveFactories.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CachingTokenFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/LowerCaseFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/StopFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/Version.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/CrankyTokenFilter.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockCharFilter.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockFixedLengthPayloadFilter.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockGraphTokenFilter.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockHoleInjectingTokenFilter.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockLowerCaseFilter.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockRandomLookaheadTokenFilter.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockSynonymFilter.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenFilter.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockVariableLengthPayloadFilter.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/SimplePayloadFilter.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/ValidatingTokenFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/KeywordTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/UnicodeWhitespaceTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/miscellaneous/PatternKeywordMarkerFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/miscellaneous/SetKeywordMarkerFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/path/ReversePathHierarchyTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/sinks/TeeSinkTokenFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/snowball/SnowballFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/sr/SerbianNormalizationRegularFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/util/CharFilterFactory.h"
#include "../../../../../../java/org/apache/lucene/analysis/util/ResourceLoader.h"
#include "../../../../../../java/org/apache/lucene/analysis/util/ResourceLoaderAware.h"
#include "../../../../../../java/org/apache/lucene/analysis/util/TokenFilterFactory.h"
#include "../../../../../../java/org/apache/lucene/analysis/util/TokenizerFactory.h"
#include "TestRandomChains.h"

namespace org::apache::lucene::analysis::core
{
using CachingTokenFilter = org::apache::lucene::analysis::CachingTokenFilter;
using CharFilter = org::apache::lucene::analysis::CharFilter;
using CrankyTokenFilter = org::apache::lucene::analysis::CrankyTokenFilter;
using MockCharFilter = org::apache::lucene::analysis::MockCharFilter;
using MockFixedLengthPayloadFilter =
    org::apache::lucene::analysis::MockFixedLengthPayloadFilter;
using MockGraphTokenFilter =
    org::apache::lucene::analysis::MockGraphTokenFilter;
using MockHoleInjectingTokenFilter =
    org::apache::lucene::analysis::MockHoleInjectingTokenFilter;
using MockLowerCaseFilter = org::apache::lucene::analysis::MockLowerCaseFilter;
using MockRandomLookaheadTokenFilter =
    org::apache::lucene::analysis::MockRandomLookaheadTokenFilter;
using MockSynonymFilter = org::apache::lucene::analysis::MockSynonymFilter;
using MockTokenFilter = org::apache::lucene::analysis::MockTokenFilter;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using MockVariableLengthPayloadFilter =
    org::apache::lucene::analysis::MockVariableLengthPayloadFilter;
using SimplePayloadFilter = org::apache::lucene::analysis::SimplePayloadFilter;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using ValidatingTokenFilter =
    org::apache::lucene::analysis::ValidatingTokenFilter;
using PatternKeywordMarkerFilter =
    org::apache::lucene::analysis::miscellaneous::PatternKeywordMarkerFilter;
using SetKeywordMarkerFilter =
    org::apache::lucene::analysis::miscellaneous::SetKeywordMarkerFilter;
using ReversePathHierarchyTokenizer =
    org::apache::lucene::analysis::path::ReversePathHierarchyTokenizer;
using TeeSinkTokenFilter =
    org::apache::lucene::analysis::sinks::TeeSinkTokenFilter;
using SnowballFilter = org::apache::lucene::analysis::snowball::SnowballFilter;
using SerbianNormalizationRegularFilter =
    org::apache::lucene::analysis::sr::SerbianNormalizationRegularFilter;
using CharFilterFactory =
    org::apache::lucene::analysis::util::CharFilterFactory;
using ResourceLoader = org::apache::lucene::analysis::util::ResourceLoader;
using ResourceLoaderAware =
    org::apache::lucene::analysis::util::ResourceLoaderAware;
using StringMockResourceLoader =
    org::apache::lucene::analysis::util::StringMockResourceLoader;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;
using TokenizerFactory = org::apache::lucene::analysis::util::TokenizerFactory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using Version = org::apache::lucene::util::Version;
const shared_ptr<java::util::Set<type_info>>
    TestAllAnalyzersHaveFactories::testComponents =
        java::util::Collections::newSetFromMap(
            make_shared<java::util::IdentityHashMap<type_info, bool>>());

TestAllAnalyzersHaveFactories::StaticConstructor::StaticConstructor()
{
  Collections::addAll<type_info>(
      testComponents, MockTokenizer::typeid, MockCharFilter::typeid,
      MockFixedLengthPayloadFilter::typeid, MockGraphTokenFilter::typeid,
      MockHoleInjectingTokenFilter::typeid, MockLowerCaseFilter::typeid,
      MockRandomLookaheadTokenFilter::typeid, MockSynonymFilter::typeid,
      MockTokenFilter::typeid, MockVariableLengthPayloadFilter::typeid,
      ValidatingTokenFilter::typeid, CrankyTokenFilter::typeid,
      SimplePayloadFilter::typeid);
  Collections::addAll<type_info>(crazyComponents, CachingTokenFilter::typeid,
                                 TeeSinkTokenFilter::typeid);
  Collections::addAll<type_info>(
      oddlyNamedComponents, ReversePathHierarchyTokenizer::typeid,
      SnowballFilter::typeid, PatternKeywordMarkerFilter::typeid,
      SetKeywordMarkerFilter::typeid, UnicodeWhitespaceTokenizer::typeid,
      org::apache::lucene::analysis::StopFilter::typeid,
      org::apache::lucene::analysis::LowerCaseFilter::typeid);
  tokenFiltersWithoutFactory->add(SerbianNormalizationRegularFilter::typeid);
}

TestAllAnalyzersHaveFactories::StaticConstructor
    TestAllAnalyzersHaveFactories::staticConstructor;
const shared_ptr<java::util::Set<type_info>>
    TestAllAnalyzersHaveFactories::crazyComponents =
        java::util::Collections::newSetFromMap(
            make_shared<java::util::IdentityHashMap<type_info, bool>>());
const shared_ptr<java::util::Set<type_info>>
    TestAllAnalyzersHaveFactories::oddlyNamedComponents =
        java::util::Collections::newSetFromMap(
            make_shared<java::util::IdentityHashMap<type_info, bool>>());
const shared_ptr<java::util::Set<type_info>>
    TestAllAnalyzersHaveFactories::tokenFiltersWithoutFactory =
        unordered_set<type_info>();
const shared_ptr<org::apache::lucene::analysis::util::ResourceLoader>
    TestAllAnalyzersHaveFactories::loader = make_shared<
        org::apache::lucene::analysis::util::StringMockResourceLoader>(L"");

void TestAllAnalyzersHaveFactories::test() 
{
  deque<type_info> analysisClasses =
      TestRandomChains::getClassesForPackage(L"org.apache.lucene.analysis");

  for (auto c : analysisClasses) {
    constexpr int modifiers = c.getModifiers();
    if (Modifier::isAbstract(modifiers) || !Modifier::isPublic(modifiers) ||
        c.isSynthetic() || c.isAnonymousClass() || c.isMemberClass() ||
        c.isInterface() || testComponents->contains(c) ||
        crazyComponents->contains(c) || oddlyNamedComponents->contains(c) ||
        tokenFiltersWithoutFactory->contains(c) ||
        c.isAnnotationPresent(Deprecated::typeid) ||
        !(Tokenizer::typeid->isAssignableFrom(c) ||
          TokenFilter::typeid->isAssignableFrom(c) ||
          CharFilter::typeid->isAssignableFrom(c))) {
      continue;
    }

    unordered_map<wstring, wstring> args = unordered_map<wstring, wstring>();
    // C++ TODO: There is no native C++ equivalent to 'toString':
    args.emplace(L"luceneMatchVersion", Version::LATEST->toString());

    if (Tokenizer::typeid->isAssignableFrom(c)) {
      wstring clazzName = c.getSimpleName();
      assertTrue(StringHelper::endsWith(clazzName, L"Tokenizer"));
      wstring simpleName = clazzName.substr(0, clazzName.length() - 9);
      assertNotNull(TokenizerFactory::lookupClass(simpleName));
      shared_ptr<TokenizerFactory> instance = nullptr;
      try {
        instance = TokenizerFactory::forName(simpleName, args);
        assertNotNull(instance);
        if (std::dynamic_pointer_cast<ResourceLoaderAware>(instance) !=
            nullptr) {
          (std::static_pointer_cast<ResourceLoaderAware>(instance))
              ->inform(loader);
        }
        assertSame(c, instance->create()->getClass());
      } catch (const invalid_argument &e) {
        // TODO: For now pass because some factories have not yet a default
        // config that always works
      }
    } else if (TokenFilter::typeid->isAssignableFrom(c)) {
      wstring clazzName = c.getSimpleName();
      assertTrue(StringHelper::endsWith(clazzName, L"Filter"));
      wstring simpleName = clazzName.substr(
          0, clazzName.length() -
                 (StringHelper::endsWith(clazzName, L"TokenFilter") ? 11 : 6));
      assertNotNull(TokenFilterFactory::lookupClass(simpleName));
      shared_ptr<TokenFilterFactory> instance = nullptr;
      try {
        instance = TokenFilterFactory::forName(simpleName, args);
        assertNotNull(instance);
        if (std::dynamic_pointer_cast<ResourceLoaderAware>(instance) !=
            nullptr) {
          (std::static_pointer_cast<ResourceLoaderAware>(instance))
              ->inform(loader);
        }
        type_info createdClazz =
            instance->create(make_shared<KeywordTokenizer>())->getClass();
        // only check instance if factory have wrapped at all!
        if (KeywordTokenizer::typeid != createdClazz) {
          assertSame(c, createdClazz);
        }
      } catch (const invalid_argument &e) {
        // TODO: For now pass because some factories have not yet a default
        // config that always works
      }
    } else if (CharFilter::typeid->isAssignableFrom(c)) {
      wstring clazzName = c.getSimpleName();
      assertTrue(StringHelper::endsWith(clazzName, L"CharFilter"));
      wstring simpleName = clazzName.substr(0, clazzName.length() - 10);
      assertNotNull(CharFilterFactory::lookupClass(simpleName));
      shared_ptr<CharFilterFactory> instance = nullptr;
      try {
        instance = CharFilterFactory::forName(simpleName, args);
        assertNotNull(instance);
        if (std::dynamic_pointer_cast<ResourceLoaderAware>(instance) !=
            nullptr) {
          (std::static_pointer_cast<ResourceLoaderAware>(instance))
              ->inform(loader);
        }
        type_info createdClazz =
            instance->create(make_shared<StringReader>(L""))->getClass();
        // only check instance if factory have wrapped at all!
        if (StringReader::typeid != createdClazz) {
          assertSame(c, createdClazz);
        }
      } catch (const invalid_argument &e) {
        // TODO: For now pass because some factories have not yet a default
        // config that always works
      }
    }
  }
}
} // namespace org::apache::lucene::analysis::core