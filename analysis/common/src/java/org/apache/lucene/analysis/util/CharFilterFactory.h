#pragma once
#include "AbstractAnalysisFactory.h"
#include "AnalysisSPILoader.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <typeinfo>
#include <unordered_map>

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

/**
 * Abstract parent class for analysis factories that create {@link CharFilter}
 * instances.
 */
class CharFilterFactory : public AbstractAnalysisFactory
{
  GET_CLASS_NAME(CharFilterFactory)

private:
  static const std::shared_ptr<
      AnalysisSPILoader<std::shared_ptr<CharFilterFactory>>>
      loader;

  /** looks up a charfilter by name from context classpath */
public:
  static std::shared_ptr<CharFilterFactory>
  forName(const std::wstring &name,
          std::unordered_map<std::wstring, std::wstring> &args);

  /** looks up a charfilter class by name from context classpath */
  static std::type_info lookupClass(const std::wstring &name);

  /** returns a deque of all available charfilter names */
  static std::shared_ptr<Set<std::wstring>> availableCharFilters();

  /**
   * Reloads the factory deque from the given {@link ClassLoader}.
   * Changes to the factories are visible after the method ends, all
   * iterators ({@link #availableCharFilters()},...) stay consistent.
   *
   * <p><b>NOTE:</b> Only new factories are added, existing ones are
   * never removed or replaced.
   *
   * <p><em>This method is expensive and should only be called for discovery
   * of new factories on the given classpath/classloader!</em>
   */
  static void reloadCharFilters(std::shared_ptr<ClassLoader> classloader);

  /**
   * Initialize this factory via a set of key-value pairs.
   */
protected:
  CharFilterFactory(std::unordered_map<std::wstring, std::wstring> &args);

  /** Wraps the given Reader with a CharFilter. */
public:
  virtual std::shared_ptr<Reader> create(std::shared_ptr<Reader> input) = 0;

protected:
  std::shared_ptr<CharFilterFactory> shared_from_this()
  {
    return std::static_pointer_cast<CharFilterFactory>(
        AbstractAnalysisFactory::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::util
