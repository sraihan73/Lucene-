#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/Analyzer.h"

#include  "core/src/java/org/apache/lucene/queryparser/classic/QueryParser.h"
#include  "core/src/java/org/apache/lucene/queryparser/xml/QueryBuilderFactory.h"
#include  "core/src/java/org/apache/lucene/search/Query.h"
#include  "core/src/java/org/apache/lucene/queryparser/xml/ParserException.h"
#include  "core/src/java/org/apache/lucene/search/spans/SpanQuery.h"
#include  "core/src/java/org/apache/lucene/queryparser/xml/QueryBuilder.h"

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
namespace org::apache::lucene::queryparser::xml
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using QueryParser = org::apache::lucene::queryparser::classic::QueryParser;
using namespace org::apache::lucene::queryparser::xml::builders;
using Query = org::apache::lucene::search::Query;
using SpanQuery = org::apache::lucene::search::spans::SpanQuery;
using org::w3c::dom::Document;
using org::w3c::dom::Element;
using org::xml::sax::EntityResolver;
using org::xml::sax::ErrorHandler;

/**
 * Assembles a QueryBuilder which uses only core Lucene Query objects
 */
class CoreParser : public std::enable_shared_from_this<CoreParser>,
                   public QueryBuilder,
                   public SpanQueryBuilder
{
  GET_CLASS_NAME(CoreParser)

protected:
  std::wstring defaultField;
  std::shared_ptr<Analyzer> analyzer;
  std::shared_ptr<QueryParser> parser;
  std::shared_ptr<QueryBuilderFactory> queryFactory;
  const std::shared_ptr<SpanQueryBuilderFactory> spanFactory;

  /**
   * Construct an XML parser that uses a single instance QueryParser for
   * handling UserQuery tags - all parse operations are synchronised on this
   * parser
   *
   * @param parser A QueryParser which will be synchronized on during parse
   * calls.
   */
public:
  CoreParser(std::shared_ptr<Analyzer> analyzer,
             std::shared_ptr<QueryParser> parser);

  /**
   * Constructs an XML parser that creates a QueryParser for each UserQuery
   * request.
   *
   * @param defaultField The default field name used by QueryParsers constructed
   * for UserQuery tags
   */
  CoreParser(const std::wstring &defaultField,
             std::shared_ptr<Analyzer> analyzer);

protected:
  CoreParser(const std::wstring &defaultField,
             std::shared_ptr<Analyzer> analyzer,
             std::shared_ptr<QueryParser> parser);

  /**
   * Parses the given stream as XML file and returns a {@link Query}.
   * By default this disallows external entities for security reasons.
   */
public:
  virtual std::shared_ptr<Query>
  parse(std::shared_ptr<InputStream> xmlStream) ;

  // for test use
  virtual std::shared_ptr<SpanQuery> parseAsSpanQuery(
      std::shared_ptr<InputStream> xmlStream) ;

  virtual void addQueryBuilder(const std::wstring &nodeName,
                               std::shared_ptr<QueryBuilder> builder);

  virtual void addSpanBuilder(const std::wstring &nodeName,
                              std::shared_ptr<SpanQueryBuilder> builder);

  virtual void addSpanQueryBuilder(const std::wstring &nodeName,
                                   std::shared_ptr<SpanQueryBuilder> builder);

  /**
   * Returns a SAX {@link EntityResolver} to be used by {@link DocumentBuilder}.
   * By default this returns {@link #DISALLOW_EXTERNAL_ENTITY_RESOLVER}, which
   * disallows the expansion of external entities (for security reasons). To
   * restore legacy behavior, override this method to return {@code null}.
   */
protected:
  virtual std::shared_ptr<EntityResolver> getEntityResolver();

  /**
   * Subclass and override to return a SAX {@link ErrorHandler} to be used by
{@link DocumentBuilder}.
   * By default this returns {@code null} so no error handler is used.
GET_CLASS_NAME(and)
   * This method can be used to redirect XML parse errors/warnings to a custom
logger.
   */
  virtual std::shared_ptr<ErrorHandler> getErrorHandler();

private:
  std::shared_ptr<Document>
  parseXML(std::shared_ptr<InputStream> pXmlFile) ;

public:
  std::shared_ptr<Query>
  getQuery(std::shared_ptr<Element> e)  override;

  std::shared_ptr<SpanQuery>
  getSpanQuery(std::shared_ptr<Element> e)  override;

  static const std::shared_ptr<EntityResolver>
      DISALLOW_EXTERNAL_ENTITY_RESOLVER;
};

} // #include  "core/src/java/org/apache/lucene/queryparser/xml/
