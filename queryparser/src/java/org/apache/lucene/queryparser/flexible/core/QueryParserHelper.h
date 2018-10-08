#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::queryparser::flexible::core::processors
{
class QueryNodeProcessor;
}

namespace org::apache::lucene::queryparser::flexible::core::parser
{
class SyntaxParser;
}
namespace org::apache::lucene::queryparser::flexible::core::builders
{
class QueryBuilder;
}
namespace org::apache::lucene::queryparser::flexible::core::config
{
class QueryConfigHandler;
}
namespace org::apache::lucene::queryparser::flexible::core
{
class QueryNodeException;
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
namespace org::apache::lucene::queryparser::flexible::core
{

using QueryBuilder =
    org::apache::lucene::queryparser::flexible::core::builders::QueryBuilder;
using QueryConfigHandler = org::apache::lucene::queryparser::flexible::core::
    config::QueryConfigHandler;
using SyntaxParser =
    org::apache::lucene::queryparser::flexible::core::parser::SyntaxParser;
using QueryNodeProcessor = org::apache::lucene::queryparser::flexible::core::
    processors::QueryNodeProcessor;

/**
 * This class is a helper for the query parser framework, it does all the three
 * query parser phrases at once: text parsing, query processing and query
 * building.
 * <p>
 * It contains methods that allows the user to change the implementation used on
 * the three phases.
 *
 * @see QueryNodeProcessor
 * @see SyntaxParser
 * @see QueryBuilder
 * @see QueryConfigHandler
 */
class QueryParserHelper : public std::enable_shared_from_this<QueryParserHelper>
{
  GET_CLASS_NAME(QueryParserHelper)

private:
  std::shared_ptr<QueryNodeProcessor> processor;

  std::shared_ptr<SyntaxParser> syntaxParser;

  std::shared_ptr<QueryBuilder> builder;

  std::shared_ptr<QueryConfigHandler> config;

  /**
   * Creates a query parser helper object using the specified configuration,
   * text parser, processor and builder.
   *
   * @param queryConfigHandler
   *          the query configuration handler that will be initially set to this
   *          helper
   * @param syntaxParser
   *          the text parser that will be initially set to this helper
   * @param processor
   *          the query processor that will be initially set to this helper
   * @param builder
   *          the query builder that will be initially set to this helper
   *
   * @see QueryNodeProcessor
   * @see SyntaxParser
   * @see QueryBuilder
   * @see QueryConfigHandler
   */
public:
  QueryParserHelper(std::shared_ptr<QueryConfigHandler> queryConfigHandler,
                    std::shared_ptr<SyntaxParser> syntaxParser,
                    std::shared_ptr<QueryNodeProcessor> processor,
                    std::shared_ptr<QueryBuilder> builder);

  /**
   * Returns the processor object used to process the query node tree, it
   * returns <code>null</code> if no processor is used.
   *
   * @return the actual processor used to process the query node tree,
   *         <code>null</code> if no processor is used
   *
   * @see QueryNodeProcessor
   * @see #setQueryNodeProcessor(QueryNodeProcessor)
   */
  virtual std::shared_ptr<QueryNodeProcessor> getQueryNodeProcessor();

  /**
   * Sets the processor that will be used to process the query node tree. If
   * there is any {@link QueryConfigHandler} returned by
   * {@link #getQueryConfigHandler()}, it will be set on the processor. The
   * argument can be <code>null</code>, which means that no processor will be
   * used to process the query node tree.
   *
   * @param processor
   *          the processor that will be used to process the query node tree,
   *          this argument can be <code>null</code>
   *
   * @see #getQueryNodeProcessor()
   * @see QueryNodeProcessor
   */
  virtual void
  setQueryNodeProcessor(std::shared_ptr<QueryNodeProcessor> processor);

  /**
   * Sets the text parser that will be used to parse the query string, it cannot
   * be <code>null</code>.
   *
   * @param syntaxParser
   *          the text parser that will be used to parse the query string
   *
   * @see #getSyntaxParser()
   * @see SyntaxParser
   */
  virtual void setSyntaxParser(std::shared_ptr<SyntaxParser> syntaxParser);

  /**
   * The query builder that will be used to build an object from the query node
   * tree. It cannot be <code>null</code>.
   *
   * @param queryBuilder
   *          the query builder used to build something from the query node tree
   *
   * @see #getQueryBuilder()
   * @see QueryBuilder
   */
  virtual void setQueryBuilder(std::shared_ptr<QueryBuilder> queryBuilder);

  /**
   * Returns the query configuration handler, which is used during the query
   * node tree processing. It can be <code>null</code>.
   *
   * @return the query configuration handler used on the query processing,
   *         <code>null</code> if not query configuration handler is defined
   *
   * @see QueryConfigHandler
   * @see #setQueryConfigHandler(QueryConfigHandler)
   */
  virtual std::shared_ptr<QueryConfigHandler> getQueryConfigHandler();

  /**
   * Returns the query builder used to build a object from the query node tree.
   * The object produced by this builder is returned by
   * {@link #parse(std::wstring, std::wstring)}.
   *
   * @return the query builder
   *
   * @see #setQueryBuilder(QueryBuilder)
   * @see QueryBuilder
   */
  virtual std::shared_ptr<QueryBuilder> getQueryBuilder();

  /**
   * Returns the text parser used to build a query node tree from a query
   * string. The default text parser instance returned by this method is a
   * {@link SyntaxParser}.
   *
   * @return the text parse used to build query node trees.
   *
   * @see SyntaxParser
   * @see #setSyntaxParser(SyntaxParser)
   */
  virtual std::shared_ptr<SyntaxParser> getSyntaxParser();

  /**
   * Sets the query configuration handler that will be used during query
   * processing. It can be <code>null</code>. It's also set to the processor
   * returned by {@link #getQueryNodeProcessor()}.
   *
   * @param config
   *          the query configuration handler used during query processing, it
   *          can be <code>null</code>
   *
   * @see #getQueryConfigHandler()
   * @see QueryConfigHandler
   */
  virtual void
  setQueryConfigHandler(std::shared_ptr<QueryConfigHandler> config);

  /**
   * Parses a query string to an object, usually some query object.<br>
   * <br>
   * In this method the three phases are executed: <br>
   * <br>
   * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;1st - the query string is parsed using the
   * text parser returned by {@link #getSyntaxParser()}, the result is a query
   * node tree <br>
   * <br>
   * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;2nd - the query node tree is processed by the
   * processor returned by {@link #getQueryNodeProcessor()} <br>
   * <br>
   * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;3th - a object is built from the query node
   * tree using the builder returned by {@link #getQueryBuilder()}
   *
   * @param query
   *          the query string
   * @param defaultField
   *          the default field used by the text parser
   *
   * @return the object built from the query
   *
   * @throws QueryNodeException
   *           if something wrong happens along the three phases
   */
  virtual std::any
  parse(const std::wstring &query,
        const std::wstring &defaultField) ;
};

} // namespace org::apache::lucene::queryparser::flexible::core
