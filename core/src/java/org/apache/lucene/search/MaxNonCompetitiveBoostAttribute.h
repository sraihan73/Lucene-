#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class BytesRef;
}

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
namespace org::apache::lucene::search
{

using Attribute = org::apache::lucene::util::Attribute;
using BytesRef = org::apache::lucene::util::BytesRef;

/** Add this {@link Attribute} to a fresh {@link AttributeSource} before calling
 * {@link MultiTermQuery#getTermsEnum(Terms,AttributeSource)}.
 * {@link FuzzyQuery} is using this to control its internal behaviour
 * to only return competitive terms.
 * <p><b>Please note:</b> This attribute is intended to be added by the {@link
 * MultiTermQuery.RewriteMethod} to an empty {@link AttributeSource} that is
 * shared for all segments during query rewrite. This attribute source is passed
 * to all segment enums on {@link
 * MultiTermQuery#getTermsEnum(Terms,AttributeSource)}.
 * {@link TopTermsRewrite} uses this attribute to
 * inform all enums about the current boost, that is not competitive.
 * @lucene.internal
 */
class MaxNonCompetitiveBoostAttribute : public Attribute
{
  GET_CLASS_NAME(MaxNonCompetitiveBoostAttribute)
  /** This is the maximum boost that would not be competitive. */
public:
  virtual void setMaxNonCompetitiveBoost(float maxNonCompetitiveBoost) = 0;
  /** This is the maximum boost that would not be competitive. Default is
   * negative infinity, which means every term is competitive. */
  virtual float getMaxNonCompetitiveBoost() = 0;
  /** This is the term or <code>null</code> of the term that triggered the boost
   * change. */
  virtual void
  setCompetitiveTerm(std::shared_ptr<BytesRef> competitiveTerm) = 0;
  /** This is the term or <code>null</code> of the term that triggered the boost
   * change. Default is <code>null</code>, which means every term is
   * competitoive. */
  virtual std::shared_ptr<BytesRef> getCompetitiveTerm() = 0;
};

} // namespace org::apache::lucene::search
