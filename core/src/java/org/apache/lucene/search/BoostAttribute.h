#pragma once
#include "stringhelper.h"
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

/** Add this {@link Attribute} to a {@link TermsEnum} returned by {@link
 * MultiTermQuery#getTermsEnum(Terms,AttributeSource)} and update the boost on
 * each returned term. This enables to control the boost factor for each
 * matching term in {@link MultiTermQuery#SCORING_BOOLEAN_REWRITE} or
 * {@link TopTermsRewrite} mode.
 * {@link FuzzyQuery} is using this to take the edit distance into account.
 * <p><b>Please note:</b> This attribute is intended to be added only by the
 * TermsEnum to itself in its constructor and consumed by the {@link
 * MultiTermQuery.RewriteMethod}.
 * @lucene.internal
 */
class BoostAttribute : public Attribute
{
  GET_CLASS_NAME(BoostAttribute)
  /** Sets the boost in this attribute */
public:
  virtual void setBoost(float boost) = 0;
  /** Retrieves the boost, default is {@code 1.0f}. */
  virtual float getBoost() = 0;
};

} // namespace org::apache::lucene::search
