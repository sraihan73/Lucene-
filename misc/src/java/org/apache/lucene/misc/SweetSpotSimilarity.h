#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <cmath>
#include <memory>
#include <string>

/*
 * Licensed to the Syed Mamun Raihan (sraihan.com) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * sraihan.com licenses this file to You under GPLv3 License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
namespace org::apache::lucene::misc
{

using ClassicSimilarity =
    org::apache::lucene::search::similarities::ClassicSimilarity;

/**
 * <p>
 * A similarity with a lengthNorm that provides for a "plateau" of
 * equally good lengths, and tf helper functions.
 * </p>
 * <p>
 * For lengthNorm, A min/max can be specified to define the
 * plateau of lengths that should all have a norm of 1.0.
 * Below the min, and above the max the lengthNorm drops off in a
 * sqrt function.
 * </p>
 * <p>
 * For tf, baselineTf and hyperbolicTf functions are provided, which
 * subclasses can choose between.
 * </p>
 *
 * @see <a href="doc-files/ss.gnuplot">A Gnuplot file used to generate some of
 * the visualizations refrenced from each function.</a>
 */
class SweetSpotSimilarity : public ClassicSimilarity
{
  GET_CLASS_NAME(SweetSpotSimilarity)

private:
  int ln_min = 1;
  int ln_max = 1;
  float ln_steep = 0.5f;

  float tf_base = 0.0f;
  float tf_min = 0.0f;

  float tf_hyper_min = 0.0f;
  float tf_hyper_max = 2.0f;
  double tf_hyper_base = 1.3;
  float tf_hyper_xoffset = 10.0f;

public:
  SweetSpotSimilarity();

  /**
   * Sets the baseline and minimum function variables for baselineTf
   *
   * @see #baselineTf
   */
  virtual void setBaselineTfFactors(float base, float min);

  /**
   * Sets the function variables for the hyperbolicTf functions
   *
   * @param min the minimum tf value to ever be returned (default: 0.0)
   * @param max the maximum tf value to ever be returned (default: 2.0)
   * @param base the base value to be used in the exponential for the hyperbolic
   * function (default: 1.3)
   * @param xoffset the midpoint of the hyperbolic function (default: 10.0)
   * @see #hyperbolicTf
   */
  virtual void setHyperbolicTfFactors(float min, float max, double base,
                                      float xoffset);

  /**
   * Sets the default function variables used by lengthNorm when no field
   * specific variables have been set.
   *
   * @see #lengthNorm
   */
  virtual void setLengthNormFactors(int min, int max, float steepness,
                                    bool discountOverlaps);

  /**
   * Implemented as:
   * <code>
   * 1/sqrt( steepness * (abs(x-min) + abs(x-max) - (max-min)) + 1 )
   * </code>.
   *
   * <p>
   * This degrades to <code>1/sqrt(x)</code> when min and max are both 1 and
   * steepness is 0.5
   * </p>
   *
   * <p>
   * :TODO: potential optimization is to just flat out return 1.0f if numTerms
   * is between min and max.
   * </p>
   *
   * @see #setLengthNormFactors
   * @see <a href="doc-files/ss.computeLengthNorm.svg">An SVG visualization of
   * this function</a>
   */
  float lengthNorm(int numTerms) override;

  /**
   * Delegates to baselineTf
   *
   * @see #baselineTf
   */
  float tf(float freq) override;

  /**
   * Implemented as:
   * <code>
   *  (x &lt;= min) &#63; base : sqrt(x+(base**2)-min)
   * </code>
   * ...but with a special case check for 0.
   * <p>
   * This degrates to <code>sqrt(x)</code> when min and base are both 0
   * </p>
   *
   * @see #setBaselineTfFactors
   * @see <a href="doc-files/ss.baselineTf.svg">An SVG visualization of this
   * function</a>
   */
  virtual float baselineTf(float freq);

  /**
   * Uses a hyperbolic tangent function that allows for a hard max...
   *
   * <code>
   * tf(x)=min+(max-min)/2*(((base**(x-xoffset)-base**-(x-xoffset))/(base**(x-xoffset)+base**-(x-xoffset)))+1)
   * </code>
   *
   * <p>
   * This code is provided as a convenience for subclasses that want
   * to use a hyperbolic tf function.
   * </p>
   *
   * @see #setHyperbolicTfFactors
   * @see <a href="doc-files/ss.hyperbolicTf.svg">An SVG visualization of this
   * function</a>
   */
  virtual float hyperbolicTf(float freq);

  virtual std::wstring toString();

protected:
  std::shared_ptr<SweetSpotSimilarity> shared_from_this()
  {
    return std::static_pointer_cast<SweetSpotSimilarity>(
        org.apache.lucene.search.similarities
            .ClassicSimilarity::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/misc/
