#pragma once
#include "PerfTask.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
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
 * Create a new {@link org.apache.lucene.analysis.Analyzer} and set it it in the
 * getRunData() for use by all future tasks.
 *
 */
class NewAnalyzerTask : public PerfTask
{
  GET_CLASS_NAME(NewAnalyzerTask)
private:
  std::deque<std::wstring> analyzerNames;
  int current = 0;

public:
  NewAnalyzerTask(std::shared_ptr<PerfRunData> runData);

  static std::shared_ptr<Analyzer>
  createAnalyzer(const std::wstring &className) ;

  int doLogic()  override;

  /**
   * Set the params (analyzerName only),  Comma-separate deque of Analyzer class
   * names.  If the Analyzer lives in org.apache.lucene.analysis, the name can
   * be shortened by dropping the o.a.l.a part of the Fully Qualified Class
   * Name. <p> Analyzer names may also refer to previously defined
   * AnalyzerFactory's. <p> Example Declaration: {"NewAnalyzer"
   * NewAnalyzer(WhitespaceAnalyzer, SimpleAnalyzer, StopAnalyzer,
   * standard.StandardAnalyzer) &gt; <p> Example AnalyzerFactory usage: <pre>
   * -AnalyzerFactory(name:'whitespace tokenized',WhitespaceTokenizer)
   * -NewAnalyzer('whitespace tokenized')
   * </pre>
   * @param params analyzerClassName, or empty for the StandardAnalyzer
   */
  void setParams(const std::wstring &params) override;

  /* (non-Javadoc)
   * @see org.apache.lucene.benchmark.byTask.tasks.PerfTask#supportsParams()
   */
  bool supportsParams() override;

protected:
  std::shared_ptr<NewAnalyzerTask> shared_from_this()
  {
    return std::static_pointer_cast<NewAnalyzerTask>(
        PerfTask::shared_from_this());
  }
};

} // namespace org::apache::lucene::benchmark::byTask::tasks
