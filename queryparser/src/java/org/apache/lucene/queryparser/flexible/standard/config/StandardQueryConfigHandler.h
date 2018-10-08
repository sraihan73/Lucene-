#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis
{
class Analyzer;
}

namespace org::apache::lucene::search
{
class MultiTermQuery;
}
namespace org::apache::lucene::search
{
class RewriteMethod;
}
namespace org::apache::lucene::document
{
class DateTools;
}
namespace org::apache::lucene::queryparser::flexible::standard::config
{
class FuzzyConfig;
}
namespace org::apache::lucene::queryparser::flexible::standard::config
{
class PointsConfig;
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
namespace org::apache::lucene::queryparser::flexible::standard::config
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using DateTools = org::apache::lucene::document::DateTools;
using Resolution = org::apache::lucene::document::DateTools::Resolution;
using ConfigurationKey =
    org::apache::lucene::queryparser::flexible::core::config::ConfigurationKey;
using QueryConfigHandler = org::apache::lucene::queryparser::flexible::core::
    config::QueryConfigHandler;
using MultiTermQuery = org::apache::lucene::search::MultiTermQuery;
using RewriteMethod =
    org::apache::lucene::search::MultiTermQuery::RewriteMethod;

/**
 * This query configuration handler is used for almost every processor defined
 * in the {@link StandardQueryNodeProcessorPipeline} processor pipeline. It
 * holds configuration methods that reproduce the configuration methods that
 * could be set on the old lucene 2.4 QueryParser class.
 *
 * @see StandardQueryNodeProcessorPipeline
 */
class StandardQueryConfigHandler : public QueryConfigHandler
{
  GET_CLASS_NAME(StandardQueryConfigHandler)

  /**
   * Class holding keys for StandardQueryNodeProcessorPipeline options.
   */
public:
  class ConfigurationKeys final
      : public std::enable_shared_from_this<ConfigurationKeys>
  {
    GET_CLASS_NAME(ConfigurationKeys)

    /**
     * Key used to set whether position increments is enabled
     *
     * @see StandardQueryParser#setEnablePositionIncrements(bool)
     * @see StandardQueryParser#getEnablePositionIncrements()
     */
  public:
    static const std::shared_ptr<ConfigurationKey<bool>>
        ENABLE_POSITION_INCREMENTS;

    /**
     * Key used to set whether leading wildcards are supported
     *
     * @see StandardQueryParser#setAllowLeadingWildcard(bool)
     * @see StandardQueryParser#getAllowLeadingWildcard()
     */
    static const std::shared_ptr<ConfigurationKey<bool>> ALLOW_LEADING_WILDCARD;

    /**
     * Key used to set the {@link Analyzer} used for terms found in the query
     *
     * @see StandardQueryParser#setAnalyzer(Analyzer)
     * @see StandardQueryParser#getAnalyzer()
     */
    static const std::shared_ptr<ConfigurationKey<std::shared_ptr<Analyzer>>>
        ANALYZER;

    /**
     * Key used to set the default bool operator
     *
     * @see
     * StandardQueryParser#setDefaultOperator(org.apache.lucene.queryparser.flexible.standard.config.StandardQueryConfigHandler.Operator)
     * @see StandardQueryParser#getDefaultOperator()
     */
    static const std::shared_ptr<ConfigurationKey<Operator>> DEFAULT_OPERATOR;

    /**
     * Key used to set the default phrase slop
     *
     * @see StandardQueryParser#setPhraseSlop(int)
     * @see StandardQueryParser#getPhraseSlop()
     */
    static const std::shared_ptr<ConfigurationKey<int>> PHRASE_SLOP;

    /**
     * Key used to set the {@link Locale} used when parsing the query
     *
     * @see StandardQueryParser#setLocale(Locale)
     * @see StandardQueryParser#getLocale()
     */
    static const std::shared_ptr<ConfigurationKey<std::shared_ptr<Locale>>>
        LOCALE;

    static const std::shared_ptr<ConfigurationKey<std::shared_ptr<TimeZone>>>
        TIMEZONE;

    /**
     * Key used to set the {@link RewriteMethod} used when creating queries
     *
     * @see
     * StandardQueryParser#setMultiTermRewriteMethod(org.apache.lucene.search.MultiTermQuery.RewriteMethod)
     * @see StandardQueryParser#getMultiTermRewriteMethod()
     */
    static const std::shared_ptr<
        ConfigurationKey<std::shared_ptr<MultiTermQuery::RewriteMethod>>>
        MULTI_TERM_REWRITE_METHOD;

    /**
     * Key used to set the fields a query should be expanded to when the field
     * is <code>null</code>
     *
     * @see StandardQueryParser#setMultiFields(std::wstring[])
     * @see StandardQueryParser#getMultiFields()
     */
    static const std::shared_ptr<
        ConfigurationKey<std::deque<std::shared_ptr<std::wstring>>>>
        MULTI_FIELDS;

    /**
     * Key used to set a field to boost map_obj that is used to set the boost for
     * each field
     *
     * @see StandardQueryParser#setFieldsBoost(Map)
     * @see StandardQueryParser#getFieldsBoost()
     */
    static const std::shared_ptr<
        ConfigurationKey<std::unordered_map<std::wstring, float>>>
        FIELD_BOOST_MAP;

    /**
     * Key used to set a field to {@link Resolution} map_obj that is used
     * to normalize each date field value.
     *
     * @see StandardQueryParser#setDateResolutionMap(Map)
     * @see StandardQueryParser#getDateResolutionMap()
     */
    static const std::shared_ptr<ConfigurationKey<std::unordered_map<
        std::shared_ptr<std::wstring>, DateTools::Resolution>>>
        FIELD_DATE_RESOLUTION_MAP;

    /**
     * Key used to set the {@link FuzzyConfig} used to create fuzzy queries.
     *
     * @see StandardQueryParser#setFuzzyMinSim(float)
     * @see StandardQueryParser#setFuzzyPrefixLength(int)
     * @see StandardQueryParser#getFuzzyMinSim()
     * @see StandardQueryParser#getFuzzyPrefixLength()
     */
    static const std::shared_ptr<ConfigurationKey<std::shared_ptr<FuzzyConfig>>>
        FUZZY_CONFIG;

    /**
     * Key used to set default {@link Resolution}.
     *
     * @see
     * StandardQueryParser#setDateResolution(org.apache.lucene.document.DateTools.Resolution)
     * @see StandardQueryParser#getDateResolution()
     */
    static const std::shared_ptr<ConfigurationKey<DateTools::Resolution>>
        DATE_RESOLUTION;

    /**
     * Key used to set the boost value in {@link FieldConfig} objects.
     *
     * @see StandardQueryParser#setFieldsBoost(Map)
     * @see StandardQueryParser#getFieldsBoost()
     */
    static const std::shared_ptr<ConfigurationKey<float>> BOOST;

    /**
     * Key used to set a field to its {@link PointsConfig}.
     *
     * @see StandardQueryParser#setPointsConfigMap(Map)
     * @see StandardQueryParser#getPointsConfigMap()
     */
    static const std::shared_ptr<
        ConfigurationKey<std::shared_ptr<PointsConfig>>>
        POINTS_CONFIG;

    /**
     * Key used to set the {@link PointsConfig} in {@link FieldConfig} for point
     * fields.
     *
     * @see StandardQueryParser#setPointsConfigMap(Map)
     * @see StandardQueryParser#getPointsConfigMap()
     */
    static const std::shared_ptr<ConfigurationKey<
        std::unordered_map<std::wstring, std::shared_ptr<PointsConfig>>>>
        POINTS_CONFIG_MAP;
  };

  /**
   * Boolean Operator: AND or OR
   */
public:
  enum class Operator { GET_CLASS_NAME(Operator) AND, OR };

public:
  StandardQueryConfigHandler();

protected:
  std::shared_ptr<StandardQueryConfigHandler> shared_from_this()
  {
    return std::static_pointer_cast<StandardQueryConfigHandler>(
        org.apache.lucene.queryparser.flexible.core.config
            .QueryConfigHandler::shared_from_this());
  }
};

} // namespace org::apache::lucene::queryparser::flexible::standard::config
