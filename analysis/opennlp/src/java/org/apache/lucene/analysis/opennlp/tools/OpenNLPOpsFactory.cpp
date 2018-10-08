using namespace std;

#include "OpenNLPOpsFactory.h"
#include "../../../../../../../../../common/src/java/org/apache/lucene/analysis/util/ResourceLoader.h"
#include "NLPChunkerOp.h"
#include "NLPLemmatizerOp.h"
#include "NLPNERTaggerOp.h"
#include "NLPPOSTaggerOp.h"
#include "NLPSentenceDetectorOp.h"
#include "NLPTokenizerOp.h"

namespace org::apache::lucene::analysis::opennlp::tools
{
using opennlp::tools::chunker::ChunkerModel;
using opennlp::tools::lemmatizer::LemmatizerModel;
using opennlp::tools::namefind::TokenNameFinderModel;
using opennlp::tools::postag::POSModel;
using opennlp::tools::sentdetect::SentenceModel;
using opennlp::tools::tokenize::TokenizerModel;
using ResourceLoader = org::apache::lucene::analysis::util::ResourceLoader;
unordered_map<wstring,
              std::shared_ptr<opennlp::tools::sentdetect::SentenceModel>>
    OpenNLPOpsFactory::sentenceModels =
        make_shared<java::util::concurrent::ConcurrentHashMap<
            wstring,
            std::shared_ptr<opennlp::tools::sentdetect::SentenceModel>>>();
shared_ptr<java::util::concurrent::ConcurrentHashMap<
    wstring, std::shared_ptr<opennlp::tools::tokenize::TokenizerModel>>>
    OpenNLPOpsFactory::tokenizerModels =
        make_shared<java::util::concurrent::ConcurrentHashMap<
            wstring,
            std::shared_ptr<opennlp::tools::tokenize::TokenizerModel>>>();
shared_ptr<java::util::concurrent::ConcurrentHashMap<
    wstring, std::shared_ptr<opennlp::tools::postag::POSModel>>>
    OpenNLPOpsFactory::posTaggerModels =
        make_shared<java::util::concurrent::ConcurrentHashMap<
            wstring, std::shared_ptr<opennlp::tools::postag::POSModel>>>();
shared_ptr<java::util::concurrent::ConcurrentHashMap<
    wstring, std::shared_ptr<opennlp::tools::chunker::ChunkerModel>>>
    OpenNLPOpsFactory::chunkerModels =
        make_shared<java::util::concurrent::ConcurrentHashMap<
            wstring, std::shared_ptr<opennlp::tools::chunker::ChunkerModel>>>();
unordered_map<wstring,
              std::shared_ptr<opennlp::tools::namefind::TokenNameFinderModel>>
    OpenNLPOpsFactory::nerModels =
        make_shared<java::util::concurrent::ConcurrentHashMap<
            wstring,
            std::shared_ptr<opennlp::tools::namefind::TokenNameFinderModel>>>();
unordered_map<wstring,
              std::shared_ptr<opennlp::tools::lemmatizer::LemmatizerModel>>
    OpenNLPOpsFactory::lemmatizerModels =
        make_shared<java::util::concurrent::ConcurrentHashMap<
            wstring,
            std::shared_ptr<opennlp::tools::lemmatizer::LemmatizerModel>>>();
unordered_map<wstring, wstring> OpenNLPOpsFactory::lemmaDictionaries =
    make_shared<java::util::concurrent::ConcurrentHashMap<wstring, wstring>>();

shared_ptr<NLPSentenceDetectorOp> OpenNLPOpsFactory::getSentenceDetector(
    const wstring &modelName) 
{
  if (modelName != L"") {
    shared_ptr<SentenceModel> model = sentenceModels[modelName];
    return make_shared<NLPSentenceDetectorOp>(model);
  } else {
    return make_shared<NLPSentenceDetectorOp>();
  }
}

shared_ptr<SentenceModel> OpenNLPOpsFactory::getSentenceModel(
    const wstring &modelName,
    shared_ptr<ResourceLoader> loader) 
{
  shared_ptr<SentenceModel> model = sentenceModels[modelName];
  if (model == nullptr) {
    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (java.io.InputStream resource =
    // loader.openResource(modelName))
    {
      java::io::InputStream resource = loader->openResource(modelName);
      model = make_shared<SentenceModel>(resource);
    }
    sentenceModels.emplace(modelName, model);
  }
  return model;
}

shared_ptr<NLPTokenizerOp>
OpenNLPOpsFactory::getTokenizer(const wstring &modelName) 
{
  if (modelName == L"") {
    return make_shared<NLPTokenizerOp>();
  } else {
    shared_ptr<TokenizerModel> model = tokenizerModels->get(modelName);
    return make_shared<NLPTokenizerOp>(model);
  }
}

shared_ptr<TokenizerModel> OpenNLPOpsFactory::getTokenizerModel(
    const wstring &modelName,
    shared_ptr<ResourceLoader> loader) 
{
  shared_ptr<TokenizerModel> model = tokenizerModels->get(modelName);
  if (model == nullptr) {
    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (java.io.InputStream resource =
    // loader.openResource(modelName))
    {
      java::io::InputStream resource = loader->openResource(modelName);
      model = make_shared<TokenizerModel>(resource);
    }
    tokenizerModels->put(modelName, model);
  }
  return model;
}

shared_ptr<NLPPOSTaggerOp>
OpenNLPOpsFactory::getPOSTagger(const wstring &modelName) 
{
  shared_ptr<POSModel> model = posTaggerModels->get(modelName);
  return make_shared<NLPPOSTaggerOp>(model);
}

shared_ptr<POSModel> OpenNLPOpsFactory::getPOSTaggerModel(
    const wstring &modelName,
    shared_ptr<ResourceLoader> loader) 
{
  shared_ptr<POSModel> model = posTaggerModels->get(modelName);
  if (model == nullptr) {
    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (java.io.InputStream resource =
    // loader.openResource(modelName))
    {
      java::io::InputStream resource = loader->openResource(modelName);
      model = make_shared<POSModel>(resource);
    }
    posTaggerModels->put(modelName, model);
  }
  return model;
}

shared_ptr<NLPChunkerOp>
OpenNLPOpsFactory::getChunker(const wstring &modelName) 
{
  shared_ptr<ChunkerModel> model = chunkerModels->get(modelName);
  return make_shared<NLPChunkerOp>(model);
}

shared_ptr<ChunkerModel> OpenNLPOpsFactory::getChunkerModel(
    const wstring &modelName,
    shared_ptr<ResourceLoader> loader) 
{
  shared_ptr<ChunkerModel> model = chunkerModels->get(modelName);
  if (model == nullptr) {
    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (java.io.InputStream resource =
    // loader.openResource(modelName))
    {
      java::io::InputStream resource = loader->openResource(modelName);
      model = make_shared<ChunkerModel>(resource);
    }
    chunkerModels->put(modelName, model);
  }
  return model;
}

shared_ptr<NLPNERTaggerOp>
OpenNLPOpsFactory::getNERTagger(const wstring &modelName) 
{
  shared_ptr<TokenNameFinderModel> model = nerModels[modelName];
  return make_shared<NLPNERTaggerOp>(model);
}

shared_ptr<TokenNameFinderModel> OpenNLPOpsFactory::getNERTaggerModel(
    const wstring &modelName,
    shared_ptr<ResourceLoader> loader) 
{
  shared_ptr<TokenNameFinderModel> model = nerModels[modelName];
  if (model == nullptr) {
    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (java.io.InputStream resource =
    // loader.openResource(modelName))
    {
      java::io::InputStream resource = loader->openResource(modelName);
      model = make_shared<TokenNameFinderModel>(resource);
    }
    nerModels.emplace(modelName, model);
  }
  return model;
}

shared_ptr<NLPLemmatizerOp> OpenNLPOpsFactory::getLemmatizer(
    const wstring &dictionaryFile,
    const wstring &lemmatizerModelFile) 
{
  assert((dictionaryFile != L"" || lemmatizerModelFile != L"",
          L"At least one parameter must be non-null"));
  shared_ptr<InputStream> dictionaryInputStream = nullptr;
  if (dictionaryFile != L"") {
    wstring dictionary = lemmaDictionaries[dictionaryFile];
    dictionaryInputStream = make_shared<ByteArrayInputStream>(
        dictionary.getBytes(StandardCharsets::UTF_8));
  }
  shared_ptr<LemmatizerModel> lemmatizerModel =
      lemmatizerModelFile == L"" ? nullptr
                                 : lemmatizerModels[lemmatizerModelFile];
  return make_shared<NLPLemmatizerOp>(dictionaryInputStream, lemmatizerModel);
}

wstring OpenNLPOpsFactory::getLemmatizerDictionary(
    const wstring &dictionaryFile,
    shared_ptr<ResourceLoader> loader) 
{
  wstring dictionary = lemmaDictionaries[dictionaryFile];
  if (dictionary == L"") {
    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (java.io.Reader reader = new
    // java.io.InputStreamReader(loader.openResource(dictionaryFile),
    // java.nio.charset.StandardCharsets.UTF_8))
    {
      java::io::Reader reader = java::io::InputStreamReader(
          loader->openResource(dictionaryFile),
          java::nio::charset::StandardCharsets::UTF_8);
      shared_ptr<StringBuilder> builder = make_shared<StringBuilder>();
      std::deque<wchar_t> chars(8092);
      int numRead = 0;
      do {
        numRead = reader.read(chars, 0, chars.size());
        if (numRead > 0) {
          builder->append(chars, 0, numRead);
        }
      } while (numRead > 0);
      dictionary = builder->toString();
      lemmaDictionaries.emplace(dictionaryFile, dictionary);
    }
  }
  return dictionary;
}

shared_ptr<LemmatizerModel> OpenNLPOpsFactory::getLemmatizerModel(
    const wstring &modelName,
    shared_ptr<ResourceLoader> loader) 
{
  shared_ptr<LemmatizerModel> model = lemmatizerModels[modelName];
  if (model == nullptr) {
    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (java.io.InputStream resource =
    // loader.openResource(modelName))
    {
      java::io::InputStream resource = loader->openResource(modelName);
      model = make_shared<LemmatizerModel>(resource);
    }
    lemmatizerModels.emplace(modelName, model);
  }
  return model;
}

void OpenNLPOpsFactory::clearModels()
{
  sentenceModels.clear();
  tokenizerModels->clear();
  posTaggerModels->clear();
  chunkerModels->clear();
  nerModels.clear();
  lemmaDictionaries.clear();
}
} // namespace org::apache::lucene::analysis::opennlp::tools