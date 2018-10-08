using namespace std;

#include "FeatureField.h"

namespace org::apache::lucene::document
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using TermFrequencyAttribute =
    org::apache::lucene::analysis::tokenattributes::TermFrequencyAttribute;
using IndexOptions = org::apache::lucene::index::IndexOptions;
using IndexReader = org::apache::lucene::index::IndexReader;
using Term = org::apache::lucene::index::Term;
using TermContext = org::apache::lucene::index::TermContext;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using BoostQuery = org::apache::lucene::search::BoostQuery;
using Explanation = org::apache::lucene::search::Explanation;
using Query = org::apache::lucene::search::Query;
using BM25Similarity =
    org::apache::lucene::search::similarities::BM25Similarity;
using SimScorer =
    org::apache::lucene::search::similarities::Similarity::SimScorer;
using BytesRef = org::apache::lucene::util::BytesRef;
const shared_ptr<FieldType> FeatureField::FIELD_TYPE = make_shared<FieldType>();

FeatureField::StaticConstructor::StaticConstructor()
{
  FIELD_TYPE->setTokenized(false);
  FIELD_TYPE->setOmitNorms(true);
  FIELD_TYPE->setIndexOptions(IndexOptions::DOCS_AND_FREQS);
}

FeatureField::StaticConstructor FeatureField::staticConstructor;

FeatureField::FeatureField(const wstring &fieldName, const wstring &featureName,
                           float featureValue)
    : Field(fieldName, featureName, FIELD_TYPE)
{
  setFeatureValue(featureValue);
}

void FeatureField::setFeatureValue(float featureValue)
{
  if (Float::isFinite(featureValue) == false) {
    throw invalid_argument(L"featureValue must be finite, got: " +
                           to_wstring(featureValue) + L" for feature " +
                           fieldsData + L" on field " + name_);
  }
  if (featureValue < Float::MIN_NORMAL) {
    throw invalid_argument(
        L"featureValue must be a positive normal float, got: " +
        to_wstring(featureValue) + L"for feature " + fieldsData +
        L" on field " + name_ +
        L" which is less than the minimum positive normal float: " +
        Float::MIN_NORMAL);
  }
  this->featureValue = featureValue;
}

shared_ptr<TokenStream> FeatureField::tokenStream(shared_ptr<Analyzer> analyzer,
                                                  shared_ptr<TokenStream> reuse)
{
  shared_ptr<FeatureTokenStream> stream;
  if (std::dynamic_pointer_cast<FeatureTokenStream>(reuse) != nullptr) {
    stream = std::static_pointer_cast<FeatureTokenStream>(reuse);
  } else {
    stream = make_shared<FeatureTokenStream>();
  }

  int freqBits = Float::floatToIntBits(featureValue);
  stream->setValues(
      any_cast<wstring>(fieldsData),
      static_cast<int>(static_cast<unsigned int>(freqBits) >> 15));
  return stream;
}

FeatureField::FeatureTokenStream::FeatureTokenStream() {}

void FeatureField::FeatureTokenStream::setValues(const wstring &value, int freq)
{
  this->value = value;
  this->freq = freq;
}

bool FeatureField::FeatureTokenStream::incrementToken()
{
  if (used) {
    return false;
  }
  clearAttributes();
  termAttribute->append(value);
  freqAttribute->setTermFrequency(freq);
  used = true;
  return true;
}

void FeatureField::FeatureTokenStream::reset() { used = false; }

FeatureField::FeatureTokenStream::~FeatureTokenStream() { value = L""; }

float FeatureField::decodeFeatureValue(float freq)
{
  if (freq > MAX_FREQ) {
    // This is never used in practice but callers of the SimScorer API might
    // occasionally call it on eg. Float.MAX_VALUE to compute the max score
    // so we need to be consistent.
    return numeric_limits<float>::max();
  }
  int tf = static_cast<int>(freq); // lossless
  int featureBits = tf << 15;
  return Float::intBitsToFloat(featureBits);
}

shared_ptr<FeatureFunction> FeatureField::FeatureFunction::rewrite(
    shared_ptr<IndexReader> reader) 
{
  return shared_from_this();
}

FeatureField::LogFunction::LogFunction(float a) : scalingFactor(a) {}

bool FeatureField::LogFunction::equals(any obj)
{
  if (obj == nullptr || getClass() != obj.type()) {
    return false;
  }
  shared_ptr<LogFunction> that = any_cast<std::shared_ptr<LogFunction>>(obj);
  return scalingFactor == that->scalingFactor;
}

int FeatureField::LogFunction::hashCode()
{
  return Float::hashCode(scalingFactor);
}

wstring FeatureField::LogFunction::toString()
{
  return L"LogFunction(scalingFactor=" + to_wstring(scalingFactor) + L")";
}

shared_ptr<SimScorer> FeatureField::LogFunction::scorer(const wstring &field,
                                                        float weight)
{
  return make_shared<SimScorerAnonymousInnerClass>(shared_from_this(), weight);
}

FeatureField::LogFunction::SimScorerAnonymousInnerClass::
    SimScorerAnonymousInnerClass(shared_ptr<LogFunction> outerInstance,
                                 float weight)
{
  this->outerInstance = outerInstance;
  this->weight = weight;
}

float FeatureField::LogFunction::SimScorerAnonymousInnerClass::score(int doc,
                                                                     float freq)
{
  return static_cast<float>(
      weight * log(outerInstance->scalingFactor + decodeFeatureValue(freq)));
}

float FeatureField::LogFunction::SimScorerAnonymousInnerClass::
    computeSlopFactor(int distance)
{
  throw make_shared<UnsupportedOperationException>();
}

float FeatureField::LogFunction::SimScorerAnonymousInnerClass::
    computePayloadFactor(int doc, int start, int end,
                         shared_ptr<BytesRef> payload)
{
  throw make_shared<UnsupportedOperationException>();
}

shared_ptr<Explanation>
FeatureField::LogFunction::explain(const wstring &field, const wstring &feature,
                                   float w, int doc,
                                   int freq) 
{
  float featureValue = decodeFeatureValue(freq);
  float score = scorer(field, w)->score(doc, freq);
  return Explanation::match(
      score,
      L"Log function on the " + field + L" field for the " + feature +
          L" feature, computed as w * log(a + S) from:",
      {Explanation::match(w, L"w, weight of this function"),
       Explanation::match(scalingFactor, L"a, scaling factor"),
       Explanation::match(featureValue, L"S, feature value")});
}

FeatureField::SaturationFunction::SaturationFunction(const wstring &field,
                                                     const wstring &feature,
                                                     optional<float> &pivot)
    : field(field), feature(feature), pivot(pivot)
{
}

shared_ptr<FeatureFunction> FeatureField::SaturationFunction::rewrite(
    shared_ptr<IndexReader> reader) 
{
  if (pivot) {
    return FeatureFunction::rewrite(reader);
  }
  float newPivot = computePivotFeatureValue(reader, field, feature);
  return make_shared<SaturationFunction>(field, feature, newPivot);
}

bool FeatureField::SaturationFunction::equals(any obj)
{
  if (obj == nullptr || getClass() != obj.type()) {
    return false;
  }
  shared_ptr<SaturationFunction> that =
      any_cast<std::shared_ptr<SaturationFunction>>(obj);
  return Objects::equals(field, that->field) &&
         Objects::equals(feature, that->feature) &&
         Objects::equals(pivot, that->pivot);
}

int FeatureField::SaturationFunction::hashCode()
{
  return Objects::hash(field, feature, pivot);
}

wstring FeatureField::SaturationFunction::toString()
{
  return L"SaturationFunction(pivot=" + pivot + L")";
}

shared_ptr<SimScorer>
FeatureField::SaturationFunction::scorer(const wstring &field, float weight)
{
  if (!pivot) {
    throw make_shared<IllegalStateException>(L"Rewrite first");
  }
  constexpr float pivot = this->pivot; // unbox
  return make_shared<SimScorerAnonymousInnerClass>(shared_from_this(), weight,
                                                   pivot);
}

FeatureField::SaturationFunction::SimScorerAnonymousInnerClass::
    SimScorerAnonymousInnerClass(shared_ptr<SaturationFunction> outerInstance,
                                 float weight, float pivot)
{
  this->outerInstance = outerInstance;
  this->weight = weight;
  this->pivot = pivot;
}

float FeatureField::SaturationFunction::SimScorerAnonymousInnerClass::score(
    int doc, float freq)
{
  float f = decodeFeatureValue(freq);
  // should be f / (f + k) but we rewrite it to
  // 1 - k / (f + k) to make sure it doesn't decrease
  // with f in spite of rounding
  return weight * (1 - pivot / (f + pivot));
}

float FeatureField::SaturationFunction::SimScorerAnonymousInnerClass::
    computeSlopFactor(int distance)
{
  throw make_shared<UnsupportedOperationException>();
}

float FeatureField::SaturationFunction::SimScorerAnonymousInnerClass::
    computePayloadFactor(int doc, int start, int end,
                         shared_ptr<BytesRef> payload)
{
  throw make_shared<UnsupportedOperationException>();
}

shared_ptr<Explanation>
FeatureField::SaturationFunction::explain(const wstring &field,
                                          const wstring &feature, float weight,
                                          int doc, int freq) 
{
  float featureValue = decodeFeatureValue(freq);
  float score = scorer(field, weight)->score(doc, freq);
  return Explanation::match(
      score,
      L"Saturation function on the " + field + L" field for the " + feature +
          L" feature, computed as w * S / (S + k) from:",
      {Explanation::match(weight, L"w, weight of this function"),
       Explanation::match(pivot, L"k, pivot feature value that would give a "
                                 L"score contribution equal to w/2"),
       Explanation::match(featureValue, L"S, feature value")});
}

FeatureField::SigmoidFunction::SigmoidFunction(float pivot, float a)
    : pivot(pivot), a(a), pivotPa(pow(pivot, a))
{
}

bool FeatureField::SigmoidFunction::equals(any obj)
{
  if (obj == nullptr || getClass() != obj.type()) {
    return false;
  }
  shared_ptr<SigmoidFunction> that =
      any_cast<std::shared_ptr<SigmoidFunction>>(obj);
  return pivot == that->pivot && a == that->a;
}

int FeatureField::SigmoidFunction::hashCode()
{
  int h = Float::hashCode(pivot);
  h = 31 * h + Float::hashCode(a);
  return h;
}

wstring FeatureField::SigmoidFunction::toString()
{
  return L"SigmoidFunction(pivot=" + to_wstring(pivot) + L", a=" +
         to_wstring(a) + L")";
}

shared_ptr<SimScorer>
FeatureField::SigmoidFunction::scorer(const wstring &field, float weight)
{
  return make_shared<SimScorerAnonymousInnerClass>(shared_from_this(), weight);
}

FeatureField::SigmoidFunction::SimScorerAnonymousInnerClass::
    SimScorerAnonymousInnerClass(shared_ptr<SigmoidFunction> outerInstance,
                                 float weight)
{
  this->outerInstance = outerInstance;
  this->weight = weight;
}

float FeatureField::SigmoidFunction::SimScorerAnonymousInnerClass::score(
    int doc, float freq)
{
  float f = decodeFeatureValue(freq);
  // should be f^a / (f^a + k^a) but we rewrite it to
  // 1 - k^a / (f + k^a) to make sure it doesn't decrease
  // with f in spite of rounding
  return static_cast<float>(
      weight * (1 - outerInstance->pivotPa /
                        (pow(f, outerInstance->a) + outerInstance->pivotPa)));
}

float FeatureField::SigmoidFunction::SimScorerAnonymousInnerClass::
    computeSlopFactor(int distance)
{
  throw make_shared<UnsupportedOperationException>();
}

float FeatureField::SigmoidFunction::SimScorerAnonymousInnerClass::
    computePayloadFactor(int doc, int start, int end,
                         shared_ptr<BytesRef> payload)
{
  throw make_shared<UnsupportedOperationException>();
}

shared_ptr<Explanation>
FeatureField::SigmoidFunction::explain(const wstring &field,
                                       const wstring &feature, float weight,
                                       int doc, int freq) 
{
  float featureValue = decodeFeatureValue(freq);
  float score = scorer(field, weight)->score(doc, freq);
  return Explanation::match(
      score,
      L"Sigmoid function on the " + field + L" field for the " + feature +
          L" feature, computed as w * S^a / (S^a + k^a) from:",
      {Explanation::match(weight, L"w, weight of this function"),
       Explanation::match(pivot, L"k, pivot feature value that would give a "
                                 L"score contribution equal to w/2"),
       Explanation::match(pivot,
                          L"a, exponent, higher values make the function grow "
                          L"slower before k and faster after k"),
       Explanation::match(featureValue, L"S, feature value")});
}

const float FeatureField::MAX_WEIGHT = sizeof(int64_t);

shared_ptr<Query> FeatureField::newLogQuery(const wstring &fieldName,
                                            const wstring &featureName,
                                            float weight, float scalingFactor)
{
  if (weight <= 0 || weight > MAX_WEIGHT) {
    throw invalid_argument(L"weight must be in (0, " + to_wstring(MAX_WEIGHT) +
                           L"], got: " + to_wstring(weight));
  }
  if (scalingFactor < 1 || Float::isFinite(scalingFactor) == false) {
    throw invalid_argument(L"scalingFactor must be >= 1, got: " +
                           to_wstring(scalingFactor));
  }
  shared_ptr<Query> q = make_shared<FeatureQuery>(
      fieldName, featureName, make_shared<LogFunction>(scalingFactor));
  if (weight != 1.0f) {
    q = make_shared<BoostQuery>(q, weight);
  }
  return q;
}

shared_ptr<Query> FeatureField::newSaturationQuery(const wstring &fieldName,
                                                   const wstring &featureName,
                                                   float weight, float pivot)
{
  return newSaturationQuery(fieldName, featureName, weight,
                            static_cast<Float>(pivot));
}

shared_ptr<Query> FeatureField::newSaturationQuery(const wstring &fieldName,
                                                   const wstring &featureName)
{
  return newSaturationQuery(fieldName, featureName, 1.0f, nullopt);
}

shared_ptr<Query> FeatureField::newSaturationQuery(const wstring &fieldName,
                                                   const wstring &featureName,
                                                   float weight,
                                                   optional<float> &pivot)
{
  if (weight <= 0 || weight > MAX_WEIGHT) {
    throw invalid_argument(L"weight must be in (0, " + to_wstring(MAX_WEIGHT) +
                           L"], got: " + to_wstring(weight));
  }
  if (pivot && (pivot <= 0 || Float::isFinite(pivot) == false)) {
    throw invalid_argument(L"pivot must be > 0, got: " + pivot);
  }
  shared_ptr<Query> q = make_shared<FeatureQuery>(
      fieldName, featureName,
      make_shared<SaturationFunction>(fieldName, featureName, pivot));
  if (weight != 1.0f) {
    q = make_shared<BoostQuery>(q, weight);
  }
  return q;
}

shared_ptr<Query> FeatureField::newSigmoidQuery(const wstring &fieldName,
                                                const wstring &featureName,
                                                float weight, float pivot,
                                                float exp)
{
  if (weight <= 0 || weight > MAX_WEIGHT) {
    throw invalid_argument(L"weight must be in (0, " + to_wstring(MAX_WEIGHT) +
                           L"], got: " + to_wstring(weight));
  }
  if (pivot <= 0 || Float::isFinite(pivot) == false) {
    throw invalid_argument(L"pivot must be > 0, got: " + to_wstring(pivot));
  }
  if (exp <= 0 || Float::isFinite(exp) == false) {
    throw invalid_argument(L"exp must be > 0, got: " + to_wstring(exp));
  }
  shared_ptr<Query> q = make_shared<FeatureQuery>(
      fieldName, featureName, make_shared<SigmoidFunction>(pivot, exp));
  if (weight != 1.0f) {
    q = make_shared<BoostQuery>(q, weight);
  }
  return q;
}

float FeatureField::computePivotFeatureValue(
    shared_ptr<IndexReader> reader, const wstring &featureField,
    const wstring &featureName) 
{
  shared_ptr<Term> term = make_shared<Term>(featureField, featureName);
  shared_ptr<TermContext> context =
      TermContext::build(reader->getContext(), term);
  if (context->docFreq() == 0) {
    // avoid division by 0
    // The return value doesn't matter much here, the term doesn't exist,
    // it will never be used for scoring. Just Make sure to return a legal
    // value.
    return 1;
  }
  float avgFreq = static_cast<float>(
      static_cast<double>(context->totalTermFreq()) / context->docFreq());
  return decodeFeatureValue(avgFreq);
}
} // namespace org::apache::lucene::document