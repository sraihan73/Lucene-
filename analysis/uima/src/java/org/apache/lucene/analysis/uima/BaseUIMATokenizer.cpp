using namespace std;

#include "BaseUIMATokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/AttributeFactory.h"
#include "ae/AEProviderFactory.h"

namespace org::apache::lucene::analysis::uima
{
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using AEProviderFactory =
    org::apache::lucene::analysis::uima::ae::AEProviderFactory;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;
using org::apache::uima::analysis_engine::AnalysisEngine;
using org::apache::uima::analysis_engine::AnalysisEngineProcessException;
using org::apache::uima::cas::CAS;
using org::apache::uima::cas::FSIterator;
using org::apache::uima::cas::text::AnnotationFS;
using org::apache::uima::resource::ResourceInitializationException;

BaseUIMATokenizer::BaseUIMATokenizer(
    shared_ptr<AttributeFactory> factory, const wstring &descriptorPath,
    unordered_map<wstring, any> &configurationParameters)
    : org::apache::lucene::analysis::Tokenizer(factory),
      descriptorPath(descriptorPath),
      configurationParameters(configurationParameters)
{
}

void BaseUIMATokenizer::analyzeInput() throw(ResourceInitializationException,
                                             AnalysisEngineProcessException,
                                             IOException)
{
  if (ae == nullptr) {
    ae = org::apache::lucene::analysis::uima::ae
             ->AEProviderFactory::getInstance()
             ->getAEProvider(L"", descriptorPath, configurationParameters)
             ->getAE();
  }
  if (cas == nullptr) {
    cas = ae->newCAS();
  } else {
    cas->reset();
  }
  cas->setDocumentText(toString(input));
  ae->process(cas);
}

wstring
BaseUIMATokenizer::toString(shared_ptr<Reader> reader) 
{
  shared_ptr<StringBuilder> stringBuilder = make_shared<StringBuilder>();
  int ch;
  while ((ch = reader->read()) > -1) {
    stringBuilder->append(static_cast<wchar_t>(ch));
  }
  return stringBuilder->toString();
}

void BaseUIMATokenizer::reset() 
{
  Tokenizer::reset();
  iterator.reset();
}
} // namespace org::apache::lucene::analysis::uima