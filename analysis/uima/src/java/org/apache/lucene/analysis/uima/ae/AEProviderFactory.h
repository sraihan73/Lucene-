#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/uima/ae/AEProvider.h"

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

/**
 * Singleton factory class responsible of {@link AEProvider}s' creation
 */
class AEProviderFactory : public std::enable_shared_from_this<AEProviderFactory>
{
  GET_CLASS_NAME(AEProviderFactory)

private:
  static const std::shared_ptr<AEProviderFactory> instance;

  const std::unordered_map<std::wstring, std::shared_ptr<AEProvider>>
      providerCache =
          std::unordered_map<std::wstring, std::shared_ptr<AEProvider>>();

  AEProviderFactory();

public:
  static std::shared_ptr<AEProviderFactory> getInstance();

  /**
   * @param keyPrefix         a prefix of the key used to cache the AEProvider
   * @param aePath            the AnalysisEngine descriptor path
   * @param runtimeParameters map_obj of runtime parameters to configure inside the
   * AnalysisEngine
   * @return AEProvider
   */
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual std::shared_ptr<AEProvider>
  getAEProvider(const std::wstring &keyPrefix, const std::wstring &aePath,
                std::unordered_map<std::wstring, std::any> &runtimeParameters);

  /**
   * @param aePath the AnalysisEngine descriptor path
   * @return AEProvider
   */
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual std::shared_ptr<AEProvider> getAEProvider(const std::wstring &aePath);

  /**
   * @param aePath            the AnalysisEngine descriptor path
   * @param runtimeParameters map_obj of runtime parameters to configure inside the
   * AnalysisEngine
   * @return AEProvider
   */
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual std::shared_ptr<AEProvider>
  getAEProvider(const std::wstring &aePath,
                std::unordered_map<std::wstring, std::any> &runtimeParameters);
};

} // #include  "core/src/java/org/apache/lucene/analysis/uima/ae/
