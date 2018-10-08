using namespace std;

#include "SweetSpotSimilarity.h"

namespace org::apache::lucene::misc
{
using ClassicSimilarity =
    org::apache::lucene::search::similarities::ClassicSimilarity;

SweetSpotSimilarity::SweetSpotSimilarity()
    : org::apache::lucene::search::similarities::ClassicSimilarity()
{
}

void SweetSpotSimilarity::setBaselineTfFactors(float base, float min)
{
  tf_min = min;
  tf_base = base;
}

void SweetSpotSimilarity::setHyperbolicTfFactors(float min, float max,
                                                 double base, float xoffset)
{
  tf_hyper_min = min;
  tf_hyper_max = max;
  tf_hyper_base = base;
  tf_hyper_xoffset = xoffset;
}

void SweetSpotSimilarity::setLengthNormFactors(int min, int max,
                                               float steepness,
                                               bool discountOverlaps)
{
  this->ln_min = min;
  this->ln_max = max;
  this->ln_steep = steepness;
  this->discountOverlaps = discountOverlaps;
}

float SweetSpotSimilarity::lengthNorm(int numTerms)
{
  constexpr int l = ln_min;
  constexpr int h = ln_max;
  constexpr float s = ln_steep;

  return static_cast<float>(
      1.0f / sqrt((s * static_cast<float>(abs(numTerms - l) +
                                          abs(numTerms - h) - (h - l))) +
                  1.0f));
}

float SweetSpotSimilarity::tf(float freq) { return baselineTf(freq); }

float SweetSpotSimilarity::baselineTf(float freq)
{

  if (0.0f == freq) {
    return 0.0f;
  }

  return (freq <= tf_min)
             ? tf_base
             : static_cast<float>(sqrt(freq + (tf_base * tf_base) - tf_min));
}

float SweetSpotSimilarity::hyperbolicTf(float freq)
{
  if (0.0f == freq) {
    return 0.0f;
  }

  constexpr float min = tf_hyper_min;
  constexpr float max = tf_hyper_max;
  constexpr double base = tf_hyper_base;
  constexpr float xoffset = tf_hyper_xoffset;
  constexpr double x = static_cast<double>(freq - xoffset);

  constexpr float result =
      min + static_cast<float>((max - min) / 2.0f *
                               (((pow(base, x) - pow(base, -x)) /
                                 (pow(base, x) + pow(base, -x))) +
                                1.0));

  return isnan(result) ? max : result;
}

wstring SweetSpotSimilarity::toString()
{
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  sb->append(L"SweetSpotSimilarity")
      ->append(L"(")
      ->append(L"ln_min=" + to_wstring(ln_min) + L", ")
      ->append(L"ln_max=")
      ->append(ln_max)
      ->append(L", ")
      ->append(L"ln_steep=")
      ->append(ln_steep)
      ->append(L", ")
      ->append(L"tf_base=")
      ->append(tf_base)
      ->append(L", ")
      ->append(L"tf_min=")
      ->append(tf_min)
      ->append(L", ")
      ->append(L"tf_hyper_min=")
      ->append(tf_hyper_min)
      ->append(L", ")
      ->append(L"tf_hyper_max=")
      ->append(tf_hyper_max)
      ->append(L", ")
      ->append(L"tf_hyper_base=")
      ->append(tf_hyper_base)
      ->append(L", ")
      ->append(L"tf_hyper_xoffset=")
      ->append(tf_hyper_xoffset)
      ->append(L")");
  return sb->toString();
}
} // namespace org::apache::lucene::misc