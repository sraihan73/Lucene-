using namespace std;

#include "SampleWSTokenizerAnnotator.h"

namespace org::apache::lucene::analysis::uima::an
{
using org::apache::uima::UimaContext;
using org::apache::uima::analysis_component::JCasAnnotator_ImplBase;
using org::apache::uima::analysis_engine::AnalysisEngineProcessException;
using org::apache::uima::cas::Type;
using org::apache::uima::cas::text::AnnotationFS;
using org::apache::uima::jcas::JCas;
using org::apache::uima::resource::ResourceInitializationException;
const wstring SampleWSTokenizerAnnotator::TOKEN_TYPE =
    L"org.apache.lucene.uima.ts.TokenAnnotation";
const wstring SampleWSTokenizerAnnotator::SENTENCE_TYPE =
    L"org.apache.lucene.uima.ts.SentenceAnnotation";
const wstring SampleWSTokenizerAnnotator::WHITESPACE = L" ";

void SampleWSTokenizerAnnotator::initialize(
    shared_ptr<UimaContext> aContext) 
{
  JCasAnnotator_ImplBase::initialize(aContext);
  lineEnd = wstring::valueOf(aContext->getConfigParameterValue(L"line-end"));
}

void SampleWSTokenizerAnnotator::process(shared_ptr<JCas> jCas) throw(
    AnalysisEngineProcessException)
{
  shared_ptr<Type> sentenceType =
      jCas->getCas().getTypeSystem().getType(SENTENCE_TYPE);
  shared_ptr<Type> tokenType =
      jCas->getCas().getTypeSystem().getType(TOKEN_TYPE);
  int i = 0;
  for (wstring sentenceString : jCas->getDocumentText()->split(lineEnd)) {
    // add the sentence
    shared_ptr<AnnotationFS> sentenceAnnotation =
        jCas->getCas().createAnnotation(sentenceType, i,
                                        sentenceString.length());
    jCas->addFsToIndexes(sentenceAnnotation);
    i += sentenceString.length();
  }

  // get tokens
  int j = 0;
  for (wstring tokenString : jCas->getDocumentText()->split(WHITESPACE)) {
    int tokenLength = tokenString.length();
    shared_ptr<AnnotationFS> tokenAnnotation =
        jCas->getCas().createAnnotation(tokenType, j, j + tokenLength);
    jCas->addFsToIndexes(tokenAnnotation);
    j += tokenLength;
  }
}
} // namespace org::apache::lucene::analysis::uima::an