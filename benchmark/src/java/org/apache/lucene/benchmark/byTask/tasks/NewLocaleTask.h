#pragma once
#include "PerfTask.h"
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
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

using PerfRunData = org::apache::lucene::benchmark::byTask::PerfRunData;

/**
 * Set a {@link java.util.Locale} for use in benchmarking.
 * <p>
 * Locales can be specified in the following ways:
 * <ul>
 *  <li><code>de</code>: Language "de"
 *  <li><code>en,US</code>: Language "en", country "US"
 *  <li><code>no,NO,NY</code>: Language "no", country "NO", variant "NY"
 *  <li><code>ROOT</code>: The root (language-agnostic) Locale
 *  <li>&lt;empty string&gt;: Erase the Locale (null)
 * </ul>
 */
class NewLocaleTask : public PerfTask
{
  GET_CLASS_NAME(NewLocaleTask)
private:
  std::wstring language;
  std::wstring country;
  std::wstring variant;

  /**
   * Create a new {@link java.util.Locale} and set it it in the getRunData() for
   * use by all future tasks.
   */
public:
  NewLocaleTask(std::shared_ptr<PerfRunData> runData);

  static std::shared_ptr<Locale> createLocale(const std::wstring &language,
                                              const std::wstring &country,
                                              const std::wstring &variant);

  int doLogic()  override;

  void setParams(const std::wstring &params) override;

  bool supportsParams() override;

protected:
  std::shared_ptr<NewLocaleTask> shared_from_this()
  {
    return std::static_pointer_cast<NewLocaleTask>(
        PerfTask::shared_from_this());
  }
};

} // namespace org::apache::lucene::benchmark::byTask::tasks
