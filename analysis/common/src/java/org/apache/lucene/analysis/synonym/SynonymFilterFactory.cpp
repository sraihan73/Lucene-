using namespace std;

#include "SynonymFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/LowerCaseFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../core/WhitespaceTokenizer.h"
#include "../util/ResourceLoader.h"
#include "../util/TokenizerFactory.h"
#include "SolrSynonymParser.h"
#include "SynonymFilter.h"
#include "SynonymMap.h"
#include "WordnetSynonymParser.h"

namespace org::apache::lucene::analysis::synonym
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using LowerCaseFilter = org::apache::lucene::analysis::LowerCaseFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using FlattenGraphFilterFactory =
    org::apache::lucene::analysis::core::FlattenGraphFilterFactory;
using WhitespaceTokenizer =
    org::apache::lucene::analysis::core::WhitespaceTokenizer;
using ResourceLoader = org::apache::lucene::analysis::util::ResourceLoader;
using ResourceLoaderAware =
    org::apache::lucene::analysis::util::ResourceLoaderAware;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;
using TokenizerFactory = org::apache::lucene::analysis::util::TokenizerFactory;

SynonymFilterFactory::SynonymFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args),
      ignoreCase(getBoolean(args, L"ignoreCase", false)),
      tokenizerFactory(get(args, L"tokenizerFactory")),
      synonyms(require(args, L"synonyms")), format(get(args, L"format")),
      expand(getBoolean(args, L"expand", true)),
      analyzerName(get(args, L"analyzer"))
{

  if (analyzerName != L"" && tokenizerFactory != L"") {
    throw invalid_argument(
        L"Analyzer and TokenizerFactory can't be specified both: " +
        analyzerName + L" and " + tokenizerFactory);
  }

  if (tokenizerFactory != L"") {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    tokArgs.emplace(L"luceneMatchVersion", getLuceneMatchVersion()->toString());
    for (shared_ptr<Iterator<wstring>> itr = args.keySet().begin();
         itr->hasNext();) {
      wstring key = itr->next();
      tokArgs.emplace(key.replaceAll(L"^tokenizerFactory\\.", L""), args[key]);
      itr->remove();
    }
  }
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<TokenStream>
SynonymFilterFactory::create(shared_ptr<TokenStream> input)
{
  // if the fst is null, it means there's actually no synonyms... just return
  // the original stream as there is nothing to do here.
  return map_obj->fst == nullptr
             ? input
             : make_shared<SynonymFilter>(input, map_obj, ignoreCase);
}

void SynonymFilterFactory::inform(shared_ptr<ResourceLoader> loader) throw(
    IOException)
{
  shared_ptr<TokenizerFactory> *const factory =
      tokenizerFactory == L"" ? nullptr
                              : loadTokenizerFactory(loader, tokenizerFactory);
  shared_ptr<Analyzer> analyzer;

  if (analyzerName != L"") {
    analyzer = loadAnalyzer(loader, analyzerName);
  } else {
    analyzer =
        make_shared<AnalyzerAnonymousInnerClass>(shared_from_this(), factory);
  }

  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.analysis.Analyzer a =
  // analyzer)
  {
    org::apache::lucene::analysis::Analyzer a = analyzer;
    try {
      wstring formatClass = format;
      if (format == L"" || format == L"solr") {
        formatClass = SolrSynonymParser::typeid->getName();
      } else if (format == L"wordnet") {
        formatClass = WordnetSynonymParser::typeid->getName();
      }
      // TODO: expose dedup as a parameter?
      map_obj = loadSynonyms(loader, formatClass, true, a);
    } catch (const ParseException &e) {
      throw make_shared<IOException>(L"Error parsing synonyms file:", e);
    }
  }
}

SynonymFilterFactory::AnalyzerAnonymousInnerClass::AnalyzerAnonymousInnerClass(
    shared_ptr<SynonymFilterFactory> outerInstance,
    shared_ptr<TokenizerFactory> factory)
{
  this->outerInstance = outerInstance;
  this->factory = factory;
}

shared_ptr<Analyzer::TokenStreamComponents>
SynonymFilterFactory::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = factory == nullptr
                                        ? make_shared<WhitespaceTokenizer>()
                                        : factory->create();
  shared_ptr<TokenStream> stream = outerInstance->ignoreCase
                                       ? make_shared<LowerCaseFilter>(tokenizer)
                                       : tokenizer;
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer, stream);
}

shared_ptr<SynonymMap> SynonymFilterFactory::loadSynonyms(
    shared_ptr<ResourceLoader> loader, const wstring &cname, bool dedup,
    shared_ptr<Analyzer> analyzer) 
{
  shared_ptr<CharsetDecoder> decoder =
      StandardCharsets::UTF_8::newDecoder()
          .onMalformedInput(CodingErrorAction::REPORT)
          .onUnmappableCharacter(CodingErrorAction::REPORT);

  shared_ptr<SynonymMap::Parser> parser;
  type_info clazz = loader->findClass(cname, SynonymMap::Parser::typeid);
  try {
    parser =
        clazz.getConstructor(bool ::typeid, bool ::typeid, Analyzer::typeid)
            .newInstance(dedup, expand, analyzer);
  } catch (const runtime_error &e) {
    throw runtime_error(e);
  }

  deque<wstring> files = splitFileNames(synonyms);
  for (auto file : files) {
    decoder->reset();
    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (final java.io.Reader isr = new
    // java.io.InputStreamReader(loader.openResource(file), decoder))
    {
      shared_ptr<java::io::Reader> *const isr =
          make_shared<java.io::InputStreamReader>(loader->openResource(file),
                                                  decoder);
      parser->parse(isr);
    }
  }
  return parser->build();
}

shared_ptr<TokenizerFactory> SynonymFilterFactory::loadTokenizerFactory(
    shared_ptr<ResourceLoader> loader, const wstring &cname) 
{
  type_info clazz = loader->findClass(cname, TokenizerFactory::typeid);
  try {
    shared_ptr<TokenizerFactory> tokFactory =
        clazz.getConstructor(unordered_map::typeid).newInstance(tokArgs);
    if (std::dynamic_pointer_cast<ResourceLoaderAware>(tokFactory) != nullptr) {
      (std::static_pointer_cast<ResourceLoaderAware>(tokFactory))
          ->inform(loader);
    }
    return tokFactory;
  } catch (const runtime_error &e) {
    throw runtime_error(e);
  }
}

shared_ptr<Analyzer>
SynonymFilterFactory::loadAnalyzer(shared_ptr<ResourceLoader> loader,
                                   const wstring &cname) 
{
  type_info clazz = loader->findClass(cname, Analyzer::typeid);
  try {
    shared_ptr<Analyzer> analyzer = clazz.getConstructor().newInstance();
    if (std::dynamic_pointer_cast<ResourceLoaderAware>(analyzer) != nullptr) {
      (std::static_pointer_cast<ResourceLoaderAware>(analyzer))->inform(loader);
    }
    return analyzer;
  } catch (const runtime_error &e) {
    throw runtime_error(e);
  }
}
} // namespace org::apache::lucene::analysis::synonym