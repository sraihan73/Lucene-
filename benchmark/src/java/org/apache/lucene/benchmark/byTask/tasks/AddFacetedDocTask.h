#pragma once
#include "AddDocTask.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::facet
{
class FacetsConfig;
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

using PerfRunData = org::apache::lucene::benchmark::byTask::PerfRunData;
using FacetsConfig = org::apache::lucene::facet::FacetsConfig;

/**
 * Add a faceted document.
 * <p>
 * Config properties:
 * <ul>
 * <li><b>with.facets</b>=&lt;tells whether to actually add any facets to the
 * document| Default: true&gt; <br>
 * This config property allows to easily compare the performance of adding docs
 * with and without facets. Note that facets are created even when this is
 * false, just that they are not added to the document (nor to the taxonomy).
 * </ul>
 * <p>
 * See {@link AddDocTask} for general document parameters and configuration.
 * <p>
 * Makes use of the {@link FacetSource} in effect - see {@link PerfRunData} for
 * facet source settings.
 */
class AddFacetedDocTask : public AddDocTask
{
  GET_CLASS_NAME(AddFacetedDocTask)

private:
  std::shared_ptr<FacetsConfig> config;

public:
  AddFacetedDocTask(std::shared_ptr<PerfRunData> runData);

  void setup()  override;

protected:
  std::wstring getLogMessage(int recsCount) override;

public:
  int doLogic()  override;

protected:
  std::shared_ptr<AddFacetedDocTask> shared_from_this()
  {
    return std::static_pointer_cast<AddFacetedDocTask>(
        AddDocTask::shared_from_this());
  }
};

} // namespace org::apache::lucene::benchmark::byTask::tasks
