#pragma once
#include "AbstractAnalysisFactory.h"
#include "AnalysisSPILoader.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <typeinfo>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/TokenStream.h"

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

using TokenStream = org::apache::lucene::analysis::TokenStream;

/**
 * Abstract parent class for analysis factories that create {@link
 * org.apache.lucene.analysis.TokenFilter} instances.
 */
class TokenFilterFactory : public AbstractAnalysisFactory
{
  GET_CLASS_NAME(TokenFilterFactory)

private:
  static const std::shared_ptr<
      AnalysisSPILoader<std::shared_ptr<TokenFilterFactory>>>
      loader;

  /** looks up a tokenfilter by name from context classpath */
public:
  static std::shared_ptr<TokenFilterFactory>
  forName(const std::wstring &name,
          std::unordered_map<std::wstring, std::wstring> &args);

  /** looks up a tokenfilter class by name from context classpath */
  static std::type_info lookupClass(const std::wstring &name);

  /** returns a deque of all available tokenfilter names from context classpath
   */
  static std::shared_ptr<Set<std::wstring>> availableTokenFilters();

  /**
   * Reloads the factory deque from the given {@link ClassLoader}.
   * Changes to the factories are visible after the method ends, all
   * iterators ({@link #availableTokenFilters()},...) stay consistent.
   *
   * <p><b>NOTE:</b> Only new factories are added, existing ones are
   * never removed or replaced.
   *
   * <p><em>This method is expensive and should only be called for discovery
   * of new factories on the given classpath/classloader!</em>
   */
  static void reloadTokenFilters(std::shared_ptr<ClassLoader> classloader);

  /**
   * Initialize this factory via a set of key-value pairs.
   */
protected:
  TokenFilterFactory(std::unordered_map<std::wstring, std::wstring> &args);

  /** Transform the specified input TokenStream */
public:
  virtual std::shared_ptr<TokenStream>
  create(std::shared_ptr<TokenStream> input) = 0;

protected:
  std::shared_ptr<TokenFilterFactory> shared_from_this()
  {
    return std::static_pointer_cast<TokenFilterFactory>(
        AbstractAnalysisFactory::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/util/
