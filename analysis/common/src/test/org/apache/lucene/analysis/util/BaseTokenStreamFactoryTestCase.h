#pragma once
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/BaseTokenStreamTestCase.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/util/AbstractAnalysisFactory.h"

#include  "core/src/java/org/apache/lucene/analysis/util/ResourceLoader.h"
#include  "core/src/java/org/apache/lucene/util/Version.h"
#include  "core/src/java/org/apache/lucene/analysis/util/TokenizerFactory.h"
#include  "core/src/java/org/apache/lucene/analysis/util/TokenFilterFactory.h"
#include  "core/src/java/org/apache/lucene/analysis/util/CharFilterFactory.h"

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
namespace org::apache::lucene::analysis::util
{

using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using Version = org::apache::lucene::util::Version;

/**
 * Base class for testing tokenstream factories.
 * <p>
 * Example usage:
 * <pre class="prettyprint">
 *   Reader reader = new StringReader("Some Text to Analyze");
 *   reader = charFilterFactory("htmlstrip").create(reader);
 *   TokenStream stream = tokenizerFactory("standard").create(reader);
 *   stream = tokenFilterFactory("lowercase").create(stream);
 *   stream = tokenFilterFactory("asciifolding").create(stream);
 *   assertTokenStreamContents(stream, new std::wstring[] { "some", "text", "to",
 * "analyze" });
 * </pre>
 */
// TODO: this has to be here, since the abstract factories are not in
// lucene-core, so test-framework doesnt know about them... this also means we
// currently cannot use this in other analysis modules :(
// TODO: maybe after we improve the abstract factory/SPI apis, they can sit in
// core and resolve this.
class BaseTokenStreamFactoryTestCase : public BaseTokenStreamTestCase
{
  GET_CLASS_NAME(BaseTokenStreamFactoryTestCase)

private:
  std::shared_ptr<AbstractAnalysisFactory> analysisFactory(
      std::type_info clazz, std::shared_ptr<Version> matchVersion,
      std::shared_ptr<ResourceLoader> loader,
      std::deque<std::wstring> &keysAndValues) ;

  /**
   * Returns a fully initialized TokenizerFactory with the specified name and
   * key-value arguments.
   * {@link ClasspathResourceLoader} is used for loading resources, so any
   * required ones should be on the test classpath.
   */
protected:
  virtual std::shared_ptr<TokenizerFactory> tokenizerFactory(
      const std::wstring &name,
      std::deque<std::wstring> &keysAndValues) ;

  /**
   * Returns a fully initialized TokenizerFactory with the specified name and
   * key-value arguments.
   * {@link ClasspathResourceLoader} is used for loading resources, so any
   * required ones should be on the test classpath.
   */
  virtual std::shared_ptr<TokenizerFactory> tokenizerFactory(
      const std::wstring &name, std::shared_ptr<Version> version,
      std::deque<std::wstring> &keysAndValues) ;

  /**
   * Returns a fully initialized TokenizerFactory with the specified name,
   * version, resource loader, and key-value arguments.
   */
  virtual std::shared_ptr<TokenizerFactory> tokenizerFactory(
      const std::wstring &name, std::shared_ptr<Version> matchVersion,
      std::shared_ptr<ResourceLoader> loader,
      std::deque<std::wstring> &keysAndValues) ;

  /**
   * Returns a fully initialized TokenFilterFactory with the specified name and
   * key-value arguments.
   * {@link ClasspathResourceLoader} is used for loading resources, so any
   * required ones should be on the test classpath.
   */
  virtual std::shared_ptr<TokenFilterFactory> tokenFilterFactory(
      const std::wstring &name, std::shared_ptr<Version> version,
      std::deque<std::wstring> &keysAndValues) ;

  /**
   * Returns a fully initialized TokenFilterFactory with the specified name and
   * key-value arguments.
   * {@link ClasspathResourceLoader} is used for loading resources, so any
   * required ones should be on the test classpath.
   */
  virtual std::shared_ptr<TokenFilterFactory> tokenFilterFactory(
      const std::wstring &name,
      std::deque<std::wstring> &keysAndValues) ;

  /**
   * Returns a fully initialized TokenFilterFactory with the specified name,
   * version, resource loader, and key-value arguments.
   */
  virtual std::shared_ptr<TokenFilterFactory> tokenFilterFactory(
      const std::wstring &name, std::shared_ptr<Version> matchVersion,
      std::shared_ptr<ResourceLoader> loader,
      std::deque<std::wstring> &keysAndValues) ;

  /**
   * Returns a fully initialized CharFilterFactory with the specified name and
   * key-value arguments.
   * {@link ClasspathResourceLoader} is used for loading resources, so any
   * required ones should be on the test classpath.
   */
  virtual std::shared_ptr<CharFilterFactory> charFilterFactory(
      const std::wstring &name,
      std::deque<std::wstring> &keysAndValues) ;

  /**
   * Returns a fully initialized CharFilterFactory with the specified name,
   * version, resource loader, and key-value arguments.
   */
  virtual std::shared_ptr<CharFilterFactory> charFilterFactory(
      const std::wstring &name, std::shared_ptr<Version> matchVersion,
      std::shared_ptr<ResourceLoader> loader,
      std::deque<std::wstring> &keysAndValues) ;

protected:
  std::shared_ptr<BaseTokenStreamFactoryTestCase> shared_from_this()
  {
    return std::static_pointer_cast<BaseTokenStreamFactoryTestCase>(
        org.apache.lucene.analysis.BaseTokenStreamTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/util/
