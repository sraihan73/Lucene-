using namespace std;

#include "UIMAAnnotationsTokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/OffsetAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/AttributeFactory.h"

namespace org::apache::lucene::analysis::uima
{
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;
using org::apache::uima::analysis_engine::AnalysisEngineProcessException;
using org::apache::uima::cas::Type;
using org::apache::uima::cas::text::AnnotationFS;
using org::apache::uima::resource::ResourceInitializationException;

UIMAAnnotationsTokenizer::UIMAAnnotationsTokenizer(
    const wstring &descriptorPath, const wstring &tokenType,
    unordered_map<wstring, any> &configurationParameters)
    : UIMAAnnotationsTokenizer(descriptorPath, tokenType,
                               configurationParameters,
                               DEFAULT_TOKEN_ATTRIBUTE_FACTORY)
{
}

UIMAAnnotationsTokenizer::UIMAAnnotationsTokenizer(
    const wstring &descriptorPath, const wstring &tokenType,
    unordered_map<wstring, any> &configurationParameters,
    shared_ptr<AttributeFactory> factory)
    : BaseUIMATokenizer(factory, descriptorPath, configurationParameters),
      termAttr(addAttribute(CharTermAttribute::typeid)),
      offsetAttr(addAttribute(OffsetAttribute::typeid)),
      tokenTypeString(tokenType)
{
}

void UIMAAnnotationsTokenizer::initializeIterator() 
{
  try {
    analyzeInput();
  }
  // C++ TODO: There is no equivalent in C++ to Java 'multi-catch' syntax:
  catch (AnalysisEngineProcessException | ResourceInitializationException e) {
    throw make_shared<IOException>(e);
  }
  finalOffset = correctOffset(cas->getDocumentText()->length());
  shared_ptr<Type> tokenType = cas->getTypeSystem().getType(tokenTypeString);
  iterator = cas->getAnnotationIndex(tokenType).begin();
}

bool UIMAAnnotationsTokenizer::incrementToken() 
{
  if (iterator == nullptr) {
    initializeIterator();
  }
  if (iterator->hasNext()) {
    clearAttributes();
    shared_ptr<AnnotationFS> next = iterator->next();
    termAttr->append(next->getCoveredText());
    offsetAttr->setOffset(correctOffset(next->getBegin()),
                          correctOffset(next->getEnd()));
    return true;
  } else {
    return false;
  }
}

void UIMAAnnotationsTokenizer::end() 
{
  BaseUIMATokenizer::end();
  offsetAttr->setOffset(finalOffset, finalOffset);
}
} // namespace org::apache::lucene::analysis::uima