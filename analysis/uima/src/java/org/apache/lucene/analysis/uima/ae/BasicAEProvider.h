#pragma once
#include "AEProvider.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>

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
namespace org::apache::lucene::analysis::uima::ae
{

using org::apache::uima::analysis_engine::AnalysisEngine;
using org::apache::uima::analysis_engine::AnalysisEngineDescription;
using org::apache::uima::resource::ResourceInitializationException;
using org::apache::uima::util::XMLInputSource;

/**
 * Basic {@link AEProvider} which just instantiates a UIMA {@link
 * AnalysisEngine} with no additional metadata, parameters or resources
 */
class BasicAEProvider : public std::enable_shared_from_this<BasicAEProvider>,
                        public AEProvider
{
  GET_CLASS_NAME(BasicAEProvider)

private:
  const std::wstring aePath;
  std::shared_ptr<AnalysisEngineDescription> cachedDescription;

public:
  BasicAEProvider(const std::wstring &aePath);

  std::shared_ptr<AnalysisEngine>
  getAE()  override;

protected:
  virtual void
  configureDescription(std::shared_ptr<AnalysisEngineDescription> description);

private:
  std::shared_ptr<XMLInputSource> getInputSource() ;
};

} // namespace org::apache::lucene::analysis::uima::ae
