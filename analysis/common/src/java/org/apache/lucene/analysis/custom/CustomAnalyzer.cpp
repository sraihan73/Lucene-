using namespace std;

#include "CustomAnalyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/Version.h"
#include "../util/AbstractAnalysisFactory.h"
#include "../util/CharFilterFactory.h"
#include "../util/ClasspathResourceLoader.h"
#include "../util/FilesystemResourceLoader.h"
#include "../util/MultiTermAwareComponent.h"
#include "../util/ResourceLoader.h"
#include "../util/ResourceLoaderAware.h"
#include "../util/TokenFilterFactory.h"
#include "../util/TokenizerFactory.h"

namespace org::apache::lucene::analysis::custom
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using ConditionalTokenFilter =
    org::apache::lucene::analysis::miscellaneous::ConditionalTokenFilter;
using ConditionalTokenFilterFactory =
    org::apache::lucene::analysis::miscellaneous::ConditionalTokenFilterFactory;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using AbstractAnalysisFactory =
    org::apache::lucene::analysis::util::AbstractAnalysisFactory;
using CharFilterFactory =
    org::apache::lucene::analysis::util::CharFilterFactory;
using ClasspathResourceLoader =
    org::apache::lucene::analysis::util::ClasspathResourceLoader;
using FilesystemResourceLoader =
    org::apache::lucene::analysis::util::FilesystemResourceLoader;
using MultiTermAwareComponent =
    org::apache::lucene::analysis::util::MultiTermAwareComponent;
using ResourceLoader = org::apache::lucene::analysis::util::ResourceLoader;
using ResourceLoaderAware =
    org::apache::lucene::analysis::util::ResourceLoaderAware;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;
using TokenizerFactory = org::apache::lucene::analysis::util::TokenizerFactory;
using SetOnce = org::apache::lucene::util::SetOnce;
using Version = org::apache::lucene::util::Version;
//    import static
//    org.apache.lucene.analysis.util.AnalysisSPILoader.newFactoryClassInstance;

shared_ptr<Builder> CustomAnalyzer::builder()
{
  return builder(make_shared<ClasspathResourceLoader>(
      CustomAnalyzer::typeid->getClassLoader()));
}

shared_ptr<Builder> CustomAnalyzer::builder(shared_ptr<Path> configDir)
{
  return builder(make_shared<FilesystemResourceLoader>(
      configDir, CustomAnalyzer::typeid->getClassLoader()));
}

shared_ptr<Builder> CustomAnalyzer::builder(shared_ptr<ResourceLoader> loader)
{
  return make_shared<Builder>(loader);
}

CustomAnalyzer::CustomAnalyzer(
    shared_ptr<Version> defaultMatchVersion,
    std::deque<std::shared_ptr<CharFilterFactory>> &charFilters,
    shared_ptr<TokenizerFactory> tokenizer,
    std::deque<std::shared_ptr<TokenFilterFactory>> &tokenFilters,
    optional<int> &posIncGap, optional<int> &offsetGap)
    : charFilters(charFilters), tokenizer(tokenizer),
      tokenFilters(tokenFilters), posIncGap(posIncGap), offsetGap(offsetGap)
{
  if (defaultMatchVersion != nullptr) {
    setVersion(defaultMatchVersion);
  }
}

shared_ptr<Reader> CustomAnalyzer::initReader(const wstring &fieldName,
                                              shared_ptr<Reader> reader)
{
  for (auto charFilter : charFilters) {
    reader = charFilter->create(reader);
  }
  return reader;
}

shared_ptr<Reader>
CustomAnalyzer::initReaderForNormalization(const wstring &fieldName,
                                           shared_ptr<Reader> reader)
{
  for (auto charFilter : charFilters) {
    if (std::dynamic_pointer_cast<MultiTermAwareComponent>(charFilter) !=
        nullptr) {
      charFilter = std::static_pointer_cast<CharFilterFactory>(
          (std::static_pointer_cast<MultiTermAwareComponent>(charFilter))
              ->getMultiTermComponent());
      reader = charFilter->create(reader);
    }
  }
  return reader;
}

shared_ptr<Analyzer::TokenStreamComponents>
CustomAnalyzer::createComponents(const wstring &fieldName)
{
  shared_ptr<Tokenizer> *const tk =
      tokenizer->create(attributeFactory(fieldName));
  shared_ptr<TokenStream> ts = tk;
  for (auto filter : tokenFilters) {
    ts = filter->create(ts);
  }
  return make_shared<Analyzer::TokenStreamComponents>(tk, ts);
}

shared_ptr<TokenStream> CustomAnalyzer::normalize(const wstring &fieldName,
                                                  shared_ptr<TokenStream> in_)
{
  shared_ptr<TokenStream> result = in_;
  // tokenizers can return a tokenfilter if the tokenizer does normalization,
  // although this is really bogus/abstraction violation...
  if (std::dynamic_pointer_cast<MultiTermAwareComponent>(tokenizer) !=
      nullptr) {
    shared_ptr<TokenFilterFactory> filter =
        std::static_pointer_cast<TokenFilterFactory>(
            (std::static_pointer_cast<MultiTermAwareComponent>(tokenizer))
                ->getMultiTermComponent());
    result = filter->create(result);
  }
  for (auto filter : tokenFilters) {
    if (std::dynamic_pointer_cast<MultiTermAwareComponent>(filter) != nullptr) {
      filter = std::static_pointer_cast<TokenFilterFactory>(
          (std::static_pointer_cast<MultiTermAwareComponent>(filter))
              ->getMultiTermComponent());
      result = filter->create(result);
    }
  }
  return result;
}

int CustomAnalyzer::getPositionIncrementGap(const wstring &fieldName)
{
  // use default from Analyzer base class if null
  return (!posIncGap) ? Analyzer::getPositionIncrementGap(fieldName)
                      : posIncGap.value();
}

int CustomAnalyzer::getOffsetGap(const wstring &fieldName)
{
  // use default from Analyzer base class if null
  return (!offsetGap) ? Analyzer::getOffsetGap(fieldName) : offsetGap.value();
}

deque<std::shared_ptr<CharFilterFactory>>
CustomAnalyzer::getCharFilterFactories()
{
  return Collections::unmodifiableList(Arrays::asList(charFilters));
}

shared_ptr<TokenizerFactory> CustomAnalyzer::getTokenizerFactory()
{
  return tokenizer;
}

deque<std::shared_ptr<TokenFilterFactory>>
CustomAnalyzer::getTokenFilterFactories()
{
  return Collections::unmodifiableList(Arrays::asList(tokenFilters));
}

wstring CustomAnalyzer::toString()
{
  shared_ptr<StringBuilder> *const sb =
      (make_shared<StringBuilder>(getClass().getSimpleName()))->append(L'(');
  for (auto filter : charFilters) {
    sb->append(filter)->append(L',');
  }
  sb->append(tokenizer);
  for (auto filter : tokenFilters) {
    sb->append(L',')->append(filter);
  }
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return sb->append(L')')->toString();
}

CustomAnalyzer::Builder::Builder(shared_ptr<ResourceLoader> loader)
    : loader(loader)
{
}

shared_ptr<Builder>
CustomAnalyzer::Builder::withDefaultMatchVersion(shared_ptr<Version> version)
{
  Objects::requireNonNull(version, L"version may not be null");
  if (componentsAdded) {
    throw make_shared<IllegalStateException>(
        wstring(L"You may only set the default match version before adding "
                L"tokenizers, ") +
        L"token filters, or char filters.");
  }
  this->defaultMatchVersion->set(version);
  return shared_from_this();
}

shared_ptr<Builder>
CustomAnalyzer::Builder::withPositionIncrementGap(int posIncGap)
{
  if (posIncGap < 0) {
    throw invalid_argument(L"posIncGap must be >= 0");
  }
  this->posIncGap->set(posIncGap);
  return shared_from_this();
}

shared_ptr<Builder> CustomAnalyzer::Builder::withOffsetGap(int offsetGap)
{
  if (offsetGap < 0) {
    throw invalid_argument(L"offsetGap must be >= 0");
  }
  this->offsetGap->set(offsetGap);
  return shared_from_this();
}

shared_ptr<Builder> CustomAnalyzer::Builder::withTokenizer(
    type_info factory, deque<wstring> &params) 
{
  return withTokenizer(factory, paramsToMap({params}));
}

shared_ptr<Builder> CustomAnalyzer::Builder::withTokenizer(
    type_info factory,
    unordered_map<wstring, wstring> &params) 
{
  Objects::requireNonNull(factory, L"Tokenizer factory may not be null");
  tokenizer->set(applyResourceLoader(
      newFactoryClassInstance(factory, applyDefaultParams(params))));
  componentsAdded = true;
  return shared_from_this();
}

shared_ptr<Builder> CustomAnalyzer::Builder::withTokenizer(
    const wstring &name, deque<wstring> &params) 
{
  return withTokenizer(name, paramsToMap({params}));
}

shared_ptr<Builder> CustomAnalyzer::Builder::withTokenizer(
    const wstring &name,
    unordered_map<wstring, wstring> &params) 
{
  Objects::requireNonNull(name, L"Tokenizer name may not be null");
  tokenizer->set(applyResourceLoader(
      TokenizerFactory::forName(name, applyDefaultParams(params))));
  componentsAdded = true;
  return shared_from_this();
}

shared_ptr<Builder> CustomAnalyzer::Builder::addTokenFilter(
    type_info factory, deque<wstring> &params) 
{
  return addTokenFilter(factory, paramsToMap({params}));
}

shared_ptr<Builder> CustomAnalyzer::Builder::addTokenFilter(
    type_info factory,
    unordered_map<wstring, wstring> &params) 
{
  Objects::requireNonNull(factory, L"TokenFilter name may not be null");
  tokenFilters.push_back(applyResourceLoader(
      newFactoryClassInstance(factory, applyDefaultParams(params))));
  componentsAdded = true;
  return shared_from_this();
}

shared_ptr<Builder> CustomAnalyzer::Builder::addTokenFilter(
    const wstring &name, deque<wstring> &params) 
{
  return addTokenFilter(name, paramsToMap({params}));
}

shared_ptr<Builder> CustomAnalyzer::Builder::addTokenFilter(
    const wstring &name,
    unordered_map<wstring, wstring> &params) 
{
  Objects::requireNonNull(name, L"TokenFilter name may not be null");
  tokenFilters.push_back(applyResourceLoader(
      TokenFilterFactory::forName(name, applyDefaultParams(params))));
  componentsAdded = true;
  return shared_from_this();
}

shared_ptr<Builder>
CustomAnalyzer::Builder::addTokenFilter(shared_ptr<TokenFilterFactory> factory)
{
  Objects::requireNonNull(factory, L"TokenFilterFactory may not be null");
  tokenFilters.push_back(factory);
  componentsAdded = true;
  return shared_from_this();
}

shared_ptr<Builder> CustomAnalyzer::Builder::addCharFilter(
    type_info factory, deque<wstring> &params) 
{
  return addCharFilter(factory, paramsToMap({params}));
}

shared_ptr<Builder> CustomAnalyzer::Builder::addCharFilter(
    type_info factory,
    unordered_map<wstring, wstring> &params) 
{
  Objects::requireNonNull(factory, L"CharFilter name may not be null");
  charFilters.push_back(applyResourceLoader(
      newFactoryClassInstance(factory, applyDefaultParams(params))));
  componentsAdded = true;
  return shared_from_this();
}

shared_ptr<Builder> CustomAnalyzer::Builder::addCharFilter(
    const wstring &name, deque<wstring> &params) 
{
  return addCharFilter(name, paramsToMap({params}));
}

shared_ptr<Builder> CustomAnalyzer::Builder::addCharFilter(
    const wstring &name,
    unordered_map<wstring, wstring> &params) 
{
  Objects::requireNonNull(name, L"CharFilter name may not be null");
  charFilters.push_back(applyResourceLoader(
      CharFilterFactory::forName(name, applyDefaultParams(params))));
  componentsAdded = true;
  return shared_from_this();
}

shared_ptr<ConditionBuilder>
CustomAnalyzer::Builder::when(const wstring &name,
                              deque<wstring> &params) 
{
  return when(name, paramsToMap({params}));
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings("unchecked") public ConditionBuilder
// when(std::wstring name, java.util.Map<std::wstring, std::wstring> params) throws
// java.io.IOException
shared_ptr<ConditionBuilder> CustomAnalyzer::Builder::when(
    const wstring &name,
    unordered_map<wstring, wstring> &params) 
{
  type_info clazz = TokenFilterFactory::lookupClass(name);
  if (ConditionalTokenFilterFactory::typeid->isAssignableFrom(clazz) == false) {
    throw invalid_argument(L"TokenFilterFactory " + name +
                           L" is not a ConditionalTokenFilterFactory");
  }
  return when(static_cast<type_info>(clazz), params);
}

shared_ptr<ConditionBuilder>
CustomAnalyzer::Builder::when(type_info factory,
                              deque<wstring> &params) 
{
  return when(factory, paramsToMap({params}));
}

shared_ptr<ConditionBuilder> CustomAnalyzer::Builder::when(
    type_info factory,
    unordered_map<wstring, wstring> &params) 
{
  return when(newFactoryClassInstance(factory, applyDefaultParams(params)));
}

shared_ptr<ConditionBuilder>
CustomAnalyzer::Builder::when(shared_ptr<ConditionalTokenFilterFactory> factory)
{
  return make_shared<ConditionBuilder>(factory, shared_from_this());
}

shared_ptr<ConditionBuilder>
CustomAnalyzer::Builder::whenTerm(function<bool(std::wstring *)> &predicate)
{
  return make_shared<ConditionBuilder>(
      make_shared<ConditionalTokenFilterFactoryAnonymousInnerClass>(
          shared_from_this(), Collections::emptyMap(), predicate),
      shared_from_this());
}

CustomAnalyzer::Builder::ConditionalTokenFilterFactoryAnonymousInnerClass::
    ConditionalTokenFilterFactoryAnonymousInnerClass(
        shared_ptr<Builder> outerInstance, shared_ptr<UnknownType> emptyMap,
        function<bool(std::wstring *)> &predicate)
    : org::apache::lucene::analysis::miscellaneous::
          ConditionalTokenFilterFactory(emptyMap)
{
  this->outerInstance = outerInstance;
  this->predicate = predicate;
}

shared_ptr<ConditionalTokenFilter>
CustomAnalyzer::Builder::ConditionalTokenFilterFactoryAnonymousInnerClass::
    create(shared_ptr<TokenStream> input,
           function<TokenStream *(TokenStream *)> &inner)
{
  return make_shared<ConditionalTokenFilterAnonymousInnerClass>(
      shared_from_this(), input, inner);
}

CustomAnalyzer::Builder::ConditionalTokenFilterFactoryAnonymousInnerClass::
    ConditionalTokenFilterAnonymousInnerClass::
        ConditionalTokenFilterAnonymousInnerClass(
            shared_ptr<ConditionalTokenFilterFactoryAnonymousInnerClass>
                outerInstance,
            shared_ptr<TokenStream> input,
            function<TokenStream *(TokenStream *)> &inner)
    : org::apache::lucene::analysis::miscellaneous::ConditionalTokenFilter(
          input, inner)
{
  this->outerInstance = outerInstance;
  termAtt = addAttribute(CharTermAttribute::typeid);
}

bool CustomAnalyzer::Builder::ConditionalTokenFilterFactoryAnonymousInnerClass::
    ConditionalTokenFilterAnonymousInnerClass::shouldFilter()
{
  return outerInstance->predicate(termAtt);
}

shared_ptr<CustomAnalyzer> CustomAnalyzer::Builder::build()
{
  if (tokenizer->get() == nullptr) {
    throw make_shared<IllegalStateException>(
        L"You have to set at least a tokenizer.");
  }
  return make_shared<CustomAnalyzer>(
      defaultMatchVersion->get(),
      charFilters.toArray(
          std::deque<std::shared_ptr<CharFilterFactory>>(charFilters.size())),
      tokenizer->get(),
      tokenFilters.toArray(std::deque<std::shared_ptr<TokenFilterFactory>>(
          tokenFilters.size())),
      posIncGap->get(), offsetGap->get());
}

unordered_map<wstring, wstring> CustomAnalyzer::Builder::applyDefaultParams(
    unordered_map<wstring, wstring> &map_obj)
{
  if (defaultMatchVersion->get() != nullptr &&
      map_obj.find(AbstractAnalysisFactory::LUCENE_MATCH_VERSION_PARAM) ==
          map_obj.end()) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    map_obj.emplace(AbstractAnalysisFactory::LUCENE_MATCH_VERSION_PARAM,
                defaultMatchVersion->get()->toString());
  }
  return map_obj;
}

unordered_map<wstring, wstring>
CustomAnalyzer::Builder::paramsToMap(deque<wstring> &params)
{
  if (params->length % 2 != 0) {
    throw invalid_argument(
        L"Key-value pairs expected, so the number of params must be even.");
  }
  const unordered_map<wstring, wstring> map_obj = unordered_map<wstring, wstring>();
  for (int i = 0; i < params->length; i += 2) {
    Objects::requireNonNull(params[i], L"Key of param may not be null.");
    map_obj.emplace(params[i], params[i + 1]);
  }
  return map_obj;
}

template <typename T>
T CustomAnalyzer::Builder::applyResourceLoader(T factory) 
{
  if (std::static_pointer_cast<ResourceLoaderAware>(factory) != nullptr) {
    (std::static_pointer_cast<ResourceLoaderAware>(factory))->inform(loader);
  }
  return factory;
}

CustomAnalyzer::ConditionBuilder::ConditionBuilder(
    shared_ptr<ConditionalTokenFilterFactory> factory,
    shared_ptr<Builder> parent)
    : factory(factory), parent(parent)
{
}

shared_ptr<ConditionBuilder> CustomAnalyzer::ConditionBuilder::addTokenFilter(
    const wstring &name,
    unordered_map<wstring, wstring> &params) 
{
  innerFilters.push_back(
      TokenFilterFactory::forName(name, parent->applyDefaultParams(params)));
  return shared_from_this();
}

shared_ptr<ConditionBuilder> CustomAnalyzer::ConditionBuilder::addTokenFilter(
    const wstring &name, deque<wstring> &params) 
{
  return addTokenFilter(name, parent->paramsToMap({params}));
}

shared_ptr<ConditionBuilder> CustomAnalyzer::ConditionBuilder::addTokenFilter(
    type_info factory,
    unordered_map<wstring, wstring> &params) 
{
  innerFilters.push_back(
      newFactoryClassInstance(factory, parent->applyDefaultParams(params)));
  return shared_from_this();
}

shared_ptr<ConditionBuilder> CustomAnalyzer::ConditionBuilder::addTokenFilter(
    type_info factory, deque<wstring> &params) 
{
  return addTokenFilter(factory, parent->paramsToMap({params}));
}

shared_ptr<Builder>
CustomAnalyzer::ConditionBuilder::endwhen() 
{
  factory->setInnerFilters(innerFilters);
  parent->applyResourceLoader(factory);
  parent->addTokenFilter(factory);
  return parent;
}
} // namespace org::apache::lucene::analysis::custom