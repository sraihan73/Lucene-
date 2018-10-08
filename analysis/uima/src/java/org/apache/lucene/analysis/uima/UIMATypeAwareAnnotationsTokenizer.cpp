using namespace std;

#include "UIMATypeAwareAnnotationsTokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/OffsetAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/TypeAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/AttributeFactory.h"

namespace org::apache::lucene::analysis::uima
{
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using TypeAttribute =
    org::apache::lucene::analysis::tokenattributes::TypeAttribute;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;
using org::apache::uima::analysis_engine::AnalysisEngineProcessException;
using org::apache::uima::cas::CASException;
using org::apache::uima::cas::FeaturePath;
using org::apache::uima::cas::Type;
using org::apache::uima::cas::text::AnnotationFS;
using org::apache::uima::resource::ResourceInitializationException;

UIMATypeAwareAnnotationsTokenizer::UIMATypeAwareAnnotationsTokenizer(
    const wstring &descriptorPath, const wstring &tokenType,
    const wstring &typeAttributeFeaturePath,
    unordered_map<wstring, any> &configurationParameters)
    : UIMATypeAwareAnnotationsTokenizer(
          descriptorPath, tokenType, typeAttributeFeaturePath,
          configurationParameters, DEFAULT_TOKEN_ATTRIBUTE_FACTORY)
{
}

UIMATypeAwareAnnotationsTokenizer::UIMATypeAwareAnnotationsTokenizer(
    const wstring &descriptorPath, const wstring &tokenType,
    const wstring &typeAttributeFeaturePath,
    unordered_map<wstring, any> &configurationParameters,
    shared_ptr<AttributeFactory> factory)
    : BaseUIMATokenizer(factory, descriptorPath, configurationParameters),
      typeAttr(addAttribute(TypeAttribute::typeid)),
      termAttr(addAttribute(CharTermAttribute::typeid)),
      offsetAttr(addAttribute(OffsetAttribute::typeid)),
      tokenTypeString(tokenType),
      typeAttributeFeaturePath(typeAttributeFeaturePath)
{
}

void UIMATypeAwareAnnotationsTokenizer::initializeIterator() 
{
  try {
    analyzeInput();
  }
  // C++ TODO: There is no equivalent in C++ to Java 'multi-catch' syntax:
  catch (AnalysisEngineProcessException | ResourceInitializationException e) {
    throw make_shared<IOException>(e);
  }
  featurePath = cas->createFeaturePath();
  try {
    featurePath->initialize(typeAttributeFeaturePath);
  } catch (const CASException &e) {
    featurePath.reset();
    throw make_shared<IOException>(e);
  }
  finalOffset = correctOffset(cas->getDocumentText()->length());
  shared_ptr<Type> tokenType = cas->getTypeSystem().getType(tokenTypeString);
  iterator = cas->getAnnotationIndex(tokenType).begin();
}

bool UIMATypeAwareAnnotationsTokenizer::incrementToken() 
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
    typeAttr->setType(featurePath->getValueAsString(next));
    return true;
  } else {
    return false;
  }
}

void UIMATypeAwareAnnotationsTokenizer::end() 
{
  BaseUIMATokenizer::end();
  offsetAttr->setOffset(finalOffset, finalOffset);
}
} // namespace org::apache::lucene::analysis::uima