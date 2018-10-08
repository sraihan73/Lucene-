#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/MultiTermQuery.h"

#include  "core/src/java/org/apache/lucene/search/RewriteMethod.h"
#include  "core/src/java/org/apache/lucene/analysis/Analyzer.h"

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
namespace org::apache::lucene::queryparser::flexible::standard
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using DateTools = org::apache::lucene::document::DateTools;
using Resolution = org::apache::lucene::document::DateTools::Resolution;
using MultiTermQuery = org::apache::lucene::search::MultiTermQuery;

/**
 * Configuration options common across queryparser implementations.
 */
class CommonQueryParserConfiguration
{
  GET_CLASS_NAME(CommonQueryParserConfiguration)

  /**
   * Set to <code>true</code> to allow leading wildcard characters.
   * <p>
   * When set, <code>*</code> or <code>?</code> are allowed as the first
   * character of a PrefixQuery and WildcardQuery. Note that this can produce
   * very slow queries on big indexes.
   * <p>
   * Default: false.
   */
public:
  virtual void setAllowLeadingWildcard(bool allowLeadingWildcard) = 0;

  /**
   * Set to <code>true</code> to enable position increments in result query.
   * <p>
   * When set, result phrase and multi-phrase queries will be aware of position
   * increments. Useful when e.g. a StopFilter increases the position increment
   * of the token that follows an omitted token.
   * <p>
   * Default: false.
   */
  virtual void setEnablePositionIncrements(bool enabled) = 0;

  /**
   * @see #setEnablePositionIncrements(bool)
   */
  virtual bool getEnablePositionIncrements() = 0;

  /**
   * By default, it uses
   * {@link MultiTermQuery#CONSTANT_SCORE_REWRITE} when creating a
   * prefix, wildcard and range queries. This implementation is generally
   * preferable because it a) Runs faster b) Does not have the scarcity of terms
   * unduly influence score c) avoids any {@link TooManyListenersException}
   * exception. However, if your application really needs to use the
   * old-fashioned bool queries expansion rewriting and the above points are
   * not relevant then use this change the rewrite method.
   */
  virtual void setMultiTermRewriteMethod(
      std::shared_ptr<MultiTermQuery::RewriteMethod> method) = 0;

  /**
   * @see
   * #setMultiTermRewriteMethod(org.apache.lucene.search.MultiTermQuery.RewriteMethod)
   */
  virtual std::shared_ptr<MultiTermQuery::RewriteMethod>
  getMultiTermRewriteMethod() = 0;

  /**
   * Set the prefix length for fuzzy queries. Default is 0.
   *
   * @param fuzzyPrefixLength
   *          The fuzzyPrefixLength to set.
   */
  virtual void setFuzzyPrefixLength(int fuzzyPrefixLength) = 0;

  /**
   * Set locale used by date range parsing.
   */
  virtual void setLocale(std::shared_ptr<Locale> locale) = 0;

  /**
   * Returns current locale, allowing access by subclasses.
   */
  virtual std::shared_ptr<Locale> getLocale() = 0;

  virtual void setTimeZone(std::shared_ptr<TimeZone> timeZone) = 0;

  virtual std::shared_ptr<TimeZone> getTimeZone() = 0;

  /**
   * Sets the default slop for phrases. If zero, then exact phrase matches are
   * required. Default value is zero.
   */
  virtual void setPhraseSlop(int defaultPhraseSlop) = 0;

  virtual std::shared_ptr<Analyzer> getAnalyzer() = 0;

  /**
   * @see #setAllowLeadingWildcard(bool)
   */
  virtual bool getAllowLeadingWildcard() = 0;

  /**
   * Get the minimal similarity for fuzzy queries.
   */
  virtual float getFuzzyMinSim() = 0;

  /**
   * Get the prefix length for fuzzy queries.
   *
   * @return Returns the fuzzyPrefixLength.
   */
  virtual int getFuzzyPrefixLength() = 0;

  /**
   * Gets the default slop for phrases.
   */
  virtual int getPhraseSlop() = 0;

  /**
   * Set the minimum similarity for fuzzy queries. Default is defined on
   * {@link FuzzyQuery#defaultMinSimilarity}.
   */
  virtual void setFuzzyMinSim(float fuzzyMinSim) = 0;

  /**
   * Sets the default {@link Resolution} used for certain field when
   * no {@link Resolution} is defined for this field.
   *
   * @param dateResolution the default {@link Resolution}
   */
  virtual void setDateResolution(DateTools::Resolution dateResolution) = 0;
};
} // #include  "core/src/java/org/apache/lucene/queryparser/flexible/standard/
