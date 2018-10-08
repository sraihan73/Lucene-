#pragma once
#include "AbstractAnalysisFactory.h"
#include "AnalysisSPILoader.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <typeinfo>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/Tokenizer.h"

#include  "core/src/java/org/apache/lucene/util/AttributeFactory.h"

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

using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;

/**
 * Abstract parent class for analysis factories that create {@link Tokenizer}
 * instances.
 */
class TokenizerFactory : public AbstractAnalysisFactory
{
  GET_CLASS_NAME(TokenizerFactory)

private:
  static const std::shared_ptr<
      AnalysisSPILoader<std::shared_ptr<TokenizerFactory>>>
      loader;

  /** looks up a tokenizer by name from context classpath */
public:
  static std::shared_ptr<TokenizerFactory>
  forName(const std::wstring &name,
          std::unordered_map<std::wstring, std::wstring> &args);

  /** looks up a tokenizer class by name from context classpath */
  static std::type_info lookupClass(const std::wstring &name);

  /** returns a deque of all available tokenizer names from context classpath */
  static std::shared_ptr<Set<std::wstring>> availableTokenizers();

  /**
   * Reloads the factory deque from the given {@link ClassLoader}.
   * Changes to the factories are visible after the method ends, all
   * iterators ({@link #availableTokenizers()},...) stay consistent.
   *
   * <p><b>NOTE:</b> Only new factories are added, existing ones are
   * never removed or replaced.
   *
   * <p><em>This method is expensive and should only be called for discovery
   * of new factories on the given classpath/classloader!</em>
   */
  static void reloadTokenizers(std::shared_ptr<ClassLoader> classloader);

  /**
   * Initialize this factory via a set of key-value pairs.
   */
protected:
  TokenizerFactory(std::unordered_map<std::wstring, std::wstring> &args);

  /** Creates a TokenStream of the specified input using the default attribute
   * factory. */
public:
  std::shared_ptr<Tokenizer> create();

  /** Creates a TokenStream of the specified input using the given
   * AttributeFactory */
  virtual std::shared_ptr<Tokenizer>
  create(std::shared_ptr<AttributeFactory> factory) = 0;

protected:
  std::shared_ptr<TokenizerFactory> shared_from_this()
  {
    return std::static_pointer_cast<TokenizerFactory>(
        AbstractAnalysisFactory::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/util/
