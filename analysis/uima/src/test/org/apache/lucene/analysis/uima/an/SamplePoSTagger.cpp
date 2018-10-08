using namespace std;

#include "SamplePoSTagger.h"

namespace org::apache::lucene::analysis::uima::an
{
using org::apache::uima::analysis_component::JCasAnnotator_ImplBase;
using org::apache::uima::analysis_engine::AnalysisEngineProcessException;
using org::apache::uima::cas::Feature;
using org::apache::uima::cas::Type;
using org::apache::uima::jcas::JCas;
using org::apache::uima::jcas::tcas::Annotation;
const wstring SamplePoSTagger::NUM = L"NUM";
const wstring SamplePoSTagger::WORD = L"WORD";
const wstring SamplePoSTagger::TYPE_NAME =
    L"org.apache.lucene.uima.ts.TokenAnnotation";
const wstring SamplePoSTagger::FEATURE_NAME = L"pos";

void SamplePoSTagger::process(shared_ptr<JCas> jcas) throw(
    AnalysisEngineProcessException)
{
  shared_ptr<Type> type = jcas->getCas().getTypeSystem().getType(TYPE_NAME);
  shared_ptr<Feature> posFeature = type->getFeatureByBaseName(FEATURE_NAME);

  for (shared_ptr<Annotation> annotation : jcas->getAnnotationIndex(type)) {
    wstring text = annotation->getCoveredText();
    wstring pos = extractPoS(text);
    annotation->setStringValue(posFeature, pos);
  }
}

wstring SamplePoSTagger::extractPoS(const wstring &text)
{
  try {
    stod(text);
    return NUM;
  } catch (const runtime_error &e) {
    return WORD;
  }
}
} // namespace org::apache::lucene::analysis::uima::an