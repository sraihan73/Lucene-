#pragma once
#include "PerfTask.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis::util
{
class CharFilterFactory;
}

namespace org::apache::lucene::analysis::util
{
class TokenizerFactory;
}
namespace org::apache::lucene::analysis::util
{
class TokenFilterFactory;
}
namespace org::apache::lucene::benchmark::byTask
{
class PerfRunData;
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
namespace org::apache::lucene::benchmark::byTask::tasks
{

using CharFilterFactory =
    org::apache::lucene::analysis::util::CharFilterFactory;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;
using TokenizerFactory = org::apache::lucene::analysis::util::TokenizerFactory;
using PerfRunData = org::apache::lucene::benchmark::byTask::PerfRunData;

/**
 * Analyzer factory construction task.  The name given to the constructed
 * factory may be given to NewAnalyzerTask, which will call
 * AnalyzerFactory.create().
 *
 * Params are in the form argname:argvalue or argname:"argvalue" or
 * argname:'argvalue'; use backslashes to escape '"' or "'" inside a quoted
 * value when it's used as the enclosing quotation mark,
 *
 * Specify params in a comma separated deque of the following, in order:
 * <ol>
 *   <li>Analyzer args:
 *     <ul>
 *       <li><b>Required</b>:
 * <code>name:<i>analyzer-factory-name</i></code></li> <li>Optional:
 * <tt>positionIncrementGap:<i>int value</i></tt> (default: 0)</li>
 *       <li>Optional: <tt>offsetGap:<i>int value</i></tt> (default: 1)</li>
 *     </ul>
 *   </li>
 *   <li>zero or more CharFilterFactory's, followed by</li>
 *   <li>exactly one TokenizerFactory, followed by</li>
 *   <li>zero or more TokenFilterFactory's</li>
 * </ol>
 *
 * Each component analysis factory may specify <tt>luceneMatchVersion</tt>
 * (defaults to
 * {@link Version#LATEST}) and any of the args understood by the specified
 * *Factory class, in the above-describe param format.
 * <p>
 * Example:
 * <pre>
 *     -AnalyzerFactory(name:'strip html, fold to ascii, whitespace tokenize,
 * max 10k tokens', positionIncrementGap:100, HTMLStripCharFilter,
 *                      MappingCharFilter(mapping:'mapping-FoldToASCII.txt'),
 *                      WhitespaceTokenizer(luceneMatchVersion:LUCENE_5_0_0),
 *                      TokenLimitFilter(maxTokenCount:10000,
 * consumeAllTokens:false))
 *     [...]
 *     -NewAnalyzer('strip html, fold to ascii, whitespace tokenize, max 10k
 * tokens')
 * </pre>
 * <p>
 * AnalyzerFactory will direct analysis component factories to look for
 * resources under the directory specified in the "work.dir" property.
 */
class AnalyzerFactoryTask : public PerfTask
{
  GET_CLASS_NAME(AnalyzerFactoryTask)
private:
  static const std::wstring LUCENE_ANALYSIS_PACKAGE_PREFIX;
  static const std::shared_ptr<Pattern> ANALYSIS_COMPONENT_SUFFIX_PATTERN;
  static const std::shared_ptr<Pattern> TRAILING_DOT_ZERO_PATTERN;

private:
  enum class ArgType {
    GET_CLASS_NAME(ArgType) ANALYZER_ARG,
    ANALYZER_ARG_OR_CHARFILTER_OR_TOKENIZER,
    TOKENFILTER
  };

public:
  std::wstring factoryName = L"";
  std::optional<int> positionIncrementGap = std::nullopt;
  std::optional<int> offsetGap = std::nullopt;

private:
  std::deque<std::shared_ptr<CharFilterFactory>> charFilterFactories =
      std::deque<std::shared_ptr<CharFilterFactory>>();
  std::shared_ptr<TokenizerFactory> tokenizerFactory = nullptr;
  std::deque<std::shared_ptr<TokenFilterFactory>> tokenFilterFactories =
      std::deque<std::shared_ptr<TokenFilterFactory>>();

public:
  AnalyzerFactoryTask(std::shared_ptr<PerfRunData> runData);

  int doLogic() override;

  /**
   * Sets the params.
   * Analysis component factory names may optionally include the "Factory"
   * suffix.
   *
   * @param params analysis pipeline specification: name, (optional)
   * positionIncrementGap, (optional) offsetGap, 0+ CharFilterFactory's, 1
   * TokenizerFactory, and 0+ TokenFilterFactory's
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Override @SuppressWarnings("fallthrough") public void
  // setParams(std::wstring params)
  void setParams(const std::wstring &params) override;

  /**
   * Instantiates the given analysis factory class after pulling params from
   * the given stream tokenizer, then stores the result in the appropriate
   * pipeline component deque.
   *
   * @param stok stream tokenizer from which to draw analysis factory params
   * @param clazz analysis factory class to instantiate
   */
private:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings("fallthrough") private void
  // createAnalysisPipelineComponent(java.io.StreamTokenizer stok, Class clazz)
  void createAnalysisPipelineComponent(std::shared_ptr<StreamTokenizer> stok,
                                       std::type_info clazz);

  /**
   * This method looks up a class with its fully qualified name (FQN), or a
   * short-name class-simplename, or with a package suffix, assuming
   * "org.apache.lucene.analysis." as the package prefix (e.g.
   * "standard.ClassicTokenizerFactory" -&gt;
   * "org.apache.lucene.analysis.standard.ClassicTokenizerFactory").
   *
   * If className contains a period, the class is first looked up as-is,
   * assuming that it is an FQN.  If this fails, lookup is retried after
   * prepending the Lucene analysis package prefix to the class name.
   *
   * If className does not contain a period, the analysis SPI
   * *Factory.lookupClass() methods are used to find the class.
   *
   * @param className The name or the short name of the class.
   * @param expectedType The superclass className is expected to extend
   * @return the loaded class.
   * @throws ClassNotFoundException if lookup fails
   */
public:
  template <typename T>
  std::type_info lookupAnalysisClass(
      const std::wstring &className,
      std::type_info<T> &expectedType) ;

  /* (non-Javadoc)
   * @see org.apache.lucene.benchmark.byTask.tasks.PerfTask#supportsParams()
   */
  bool supportsParams() override;

  /** Returns the current line in the algorithm file */
  virtual int lineno(std::shared_ptr<StreamTokenizer> stok);

protected:
  std::shared_ptr<AnalyzerFactoryTask> shared_from_this()
  {
    return std::static_pointer_cast<AnalyzerFactoryTask>(
        PerfTask::shared_from_this());
  }
};

} // namespace org::apache::lucene::benchmark::byTask::tasks
