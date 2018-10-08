#pragma once
#include "AnalyzerWrapper.h"
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis
{
class TokenStream;
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
namespace org::apache::lucene::analysis
{

/**
 * An analyzer wrapper, that doesn't allow to wrap components or readers.
 * By disallowing it, it means that the thread local resources can be delegated
 * to the delegate analyzer, and not also be allocated on this analyzer.
 * This wrapper class is the base class of all analyzers that just delegate to
 * another analyzer, e.g. per field name.
 *
 * <p>This solves the problem of per field analyzer wrapper, where it also
 * maintains a thread local per field token stream components, while it can
 * safely delegate those and not also hold these data structures, which can
 * become expensive memory wise.
 *
 * <p><b>Please note:</b> This analyzer uses a private {@link
 * Analyzer.ReuseStrategy}, which is returned by {@link #getReuseStrategy()}.
 * This strategy is used when delegating. If you wrap this analyzer again and
 * reuse this strategy, no delegation is done and the given fallback is used.
 */
class DelegatingAnalyzerWrapper : public AnalyzerWrapper
{
  GET_CLASS_NAME(DelegatingAnalyzerWrapper)

  /**
   * Constructor.
   * @param fallbackStrategy is the strategy to use if delegation is not
   * possible This is to support the common pattern:
   *  {@code new OtherWrapper(thisWrapper.getReuseStrategy())}
   */
protected:
  DelegatingAnalyzerWrapper(std::shared_ptr<ReuseStrategy> fallbackStrategy);

  std::shared_ptr<TokenStreamComponents> wrapComponents(
      const std::wstring &fieldName,
      std::shared_ptr<TokenStreamComponents> components) override final;

  std::shared_ptr<TokenStream> wrapTokenStreamForNormalization(
      const std::wstring &fieldName,
      std::shared_ptr<TokenStream> in_) override final;

  std::shared_ptr<Reader>
  wrapReader(const std::wstring &fieldName,
             std::shared_ptr<Reader> reader) override final;

  std::shared_ptr<Reader>
  wrapReaderForNormalization(const std::wstring &fieldName,
                             std::shared_ptr<Reader> reader) override final;

private:
  class DelegatingReuseStrategy final : public ReuseStrategy
  {
    GET_CLASS_NAME(DelegatingReuseStrategy)
  public:
    std::shared_ptr<DelegatingAnalyzerWrapper> wrapper;

  private:
    const std::shared_ptr<ReuseStrategy> fallbackStrategy;

  public:
    DelegatingReuseStrategy(std::shared_ptr<ReuseStrategy> fallbackStrategy);

    std::shared_ptr<TokenStreamComponents>
    getReusableComponents(std::shared_ptr<Analyzer> analyzer,
                          const std::wstring &fieldName) override;

    void setReusableComponents(
        std::shared_ptr<Analyzer> analyzer, const std::wstring &fieldName,
        std::shared_ptr<TokenStreamComponents> components) override;

  protected:
    std::shared_ptr<DelegatingReuseStrategy> shared_from_this()
    {
      return std::static_pointer_cast<DelegatingReuseStrategy>(
          ReuseStrategy::shared_from_this());
    }
  };

protected:
  std::shared_ptr<DelegatingAnalyzerWrapper> shared_from_this()
  {
    return std::static_pointer_cast<DelegatingAnalyzerWrapper>(
        AnalyzerWrapper::shared_from_this());
  }
};
} // namespace org::apache::lucene::analysis
