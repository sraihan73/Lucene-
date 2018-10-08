using namespace std;

#include "SampleEntityAnnotator.h"

namespace org::apache::lucene::analysis::uima::an
{
using org::apache::uima::TokenAnnotation;
using org::apache::uima::analysis_component::JCasAnnotator_ImplBase;
using org::apache::uima::analysis_engine::AnalysisEngineProcessException;
using org::apache::uima::cas::Feature;
using org::apache::uima::cas::Type;
using org::apache::uima::cas::text::AnnotationFS;
using org::apache::uima::jcas::JCas;
using org::apache::uima::jcas::tcas::Annotation;
const wstring SampleEntityAnnotator::NP = L"np";
const wstring SampleEntityAnnotator::NPS = L"nps";
const wstring SampleEntityAnnotator::TYPE_NAME =
    L"org.apache.lucene.analysis.uima.ts.EntityAnnotation";
const wstring SampleEntityAnnotator::ENTITY_FEATURE = L"entity";
const wstring SampleEntityAnnotator::NAME_FEATURE = L"entity";

void SampleEntityAnnotator::process(shared_ptr<JCas> jcas) throw(
    AnalysisEngineProcessException)
{
  shared_ptr<Type> type = jcas->getCas().getTypeSystem().getType(TYPE_NAME);
  shared_ptr<Feature> entityFeature =
      type->getFeatureByBaseName(ENTITY_FEATURE);
  shared_ptr<Feature> nameFeature = type->getFeatureByBaseName(NAME_FEATURE);

  for (shared_ptr<Annotation> annotation :
       jcas->getAnnotationIndex(TokenAnnotation::type)) {
    wstring tokenPOS =
        (std::static_pointer_cast<TokenAnnotation>(annotation))->getPosTag();

    if (NP == tokenPOS || NPS == tokenPOS) {
      shared_ptr<AnnotationFS> entityAnnotation =
          jcas->getCas().createAnnotation(type, annotation->getBegin(),
                                          annotation->getEnd());

      entityAnnotation->setStringValue(entityFeature,
                                       annotation->getCoveredText());

      wstring name = L"OTHER"; // "OTHER" makes no sense. In practice, "PERSON",
                               // "COUNTRY", "E-MAIL", etc.
      if (annotation->getCoveredText().equals(L"Apache")) {
        name = L"ORGANIZATION";
      }
      entityAnnotation->setStringValue(nameFeature, name);

      jcas->addFsToIndexes(entityAnnotation);
    }
  }
}
} // namespace org::apache::lucene::analysis::uima::an