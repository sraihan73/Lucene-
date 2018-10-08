#pragma once
#include "PerfTask.h"
#include "stringhelper.h"
#include <any>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::benchmark::byTask
{
class PerfRunData;
}

namespace org::apache::lucene::analysis
{
class Analyzer;
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

using Analyzer = org::apache::lucene::analysis::Analyzer;
using PerfRunData = org::apache::lucene::benchmark::byTask::PerfRunData;

/**
 * Task to support benchmarking collation.
 * <br>
 * <ul>
 *  <li> <code>NewCollationAnalyzer</code> with the default jdk impl
 *  <li> <code>NewCollationAnalyzer(impl:icu)</code> specify an impl (jdk,icu)
 * </ul>
 */
class NewCollationAnalyzerTask : public PerfTask
{
  GET_CLASS_NAME(NewCollationAnalyzerTask)
  /**
   * Different Collation implementations: currently
   * limited to what is provided in the JDK and ICU.
   *
   * @see <a href="http://site.icu-project.org/charts/collation-icu4j-sun">
   *      Comparison of implementations</a>
   */
public:
  class Implementation final
  {
    GET_CLASS_NAME(Implementation)
  public:
    static Implementation JDK;
    static Implementation ICU;

  private:
    static std::deque<Implementation> valueList;

    class StaticConstructor
    {
      GET_CLASS_NAME(StaticConstructor)
    public:
      StaticConstructor();
    };

    static StaticConstructor staticConstructor;

  public:
    enum class InnerEnum { GET_CLASS_NAME(InnerEnum) JDK, ICU };

    const InnerEnum innerEnumValue;

  private:
    const std::wstring nameValue;
    const int ordinalValue;
    static int nextOrdinal;

  public:
    std::wstring className;
    std::wstring collatorClassName;

    Implementation(const std::wstring &name, InnerEnum innerEnum,
                   std::shared_ptr<NewCollationAnalyzerTask> outerInstance,
                   const std::wstring &className,
                   const std::wstring &collatorClassName);

  public:
    bool operator==(const Implementation &other);

    bool operator!=(const Implementation &other);

    static std::deque<Implementation> values();

    int ordinal();

    std::wstring toString();

    static Implementation valueOf(const std::wstring &name);
  };

private:
  Implementation impl = Implementation::JDK;

public:
  NewCollationAnalyzerTask(std::shared_ptr<PerfRunData> runData);

  static std::shared_ptr<Analyzer>
  createAnalyzer(std::shared_ptr<Locale> locale,
                 Implementation impl) ;

  int doLogic()  override;

  void setParams(const std::wstring &params) override;

  bool supportsParams() override;

protected:
  std::shared_ptr<NewCollationAnalyzerTask> shared_from_this()
  {
    return std::static_pointer_cast<NewCollationAnalyzerTask>(
        PerfTask::shared_from_this());
  }
};

} // namespace org::apache::lucene::benchmark::byTask::tasks
