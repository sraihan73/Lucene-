#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/AnalyzerWrapper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/Analyzer.h"

#include  "core/src/java/org/apache/lucene/analysis/TokenStreamComponents.h"

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
namespace org::apache::lucene::analysis::shingle
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using AnalyzerWrapper = org::apache::lucene::analysis::AnalyzerWrapper;

/**
 * A ShingleAnalyzerWrapper wraps a {@link ShingleFilter} around another {@link
 * Analyzer}. <p> A shingle is another name for a token based n-gram.
 * </p>
 */
class ShingleAnalyzerWrapper final : public AnalyzerWrapper
{
  GET_CLASS_NAME(ShingleAnalyzerWrapper)

private:
  const std::shared_ptr<Analyzer> delegate_;
  const int maxShingleSize;
  const int minShingleSize;
  const std::wstring tokenSeparator;
  const bool outputUnigrams;
  const bool outputUnigramsIfNoShingles;
  const std::wstring fillerToken;

public:
  ShingleAnalyzerWrapper(std::shared_ptr<Analyzer> defaultAnalyzer);

  ShingleAnalyzerWrapper(std::shared_ptr<Analyzer> defaultAnalyzer,
                         int maxShingleSize);

  ShingleAnalyzerWrapper(std::shared_ptr<Analyzer> defaultAnalyzer,
                         int minShingleSize, int maxShingleSize);

  /**
   * Creates a new ShingleAnalyzerWrapper
   *
   * @param delegate Analyzer whose TokenStream is to be filtered
   * @param minShingleSize Min shingle (token ngram) size
   * @param maxShingleSize Max shingle size
   * @param tokenSeparator Used to separate input stream tokens in output
   * shingles
   * @param outputUnigrams Whether or not the filter shall pass the original
   *        tokens to the output stream
   * @param outputUnigramsIfNoShingles Overrides the behavior of
   * outputUnigrams==false for those times when no shingles are available
   * (because there are fewer than minShingleSize tokens in the input stream)?
   *        Note that if outputUnigrams==true, then unigrams are always output,
   *        regardless of whether any shingles are available.
   * @param fillerToken filler token to use when positionIncrement is more than
   * 1
   */
  ShingleAnalyzerWrapper(std::shared_ptr<Analyzer> delegate_,
                         int minShingleSize, int maxShingleSize,
                         const std::wstring &tokenSeparator,
                         bool outputUnigrams, bool outputUnigramsIfNoShingles,
                         const std::wstring &fillerToken);

  /**
   * Wraps {@link StandardAnalyzer}.
   */
  ShingleAnalyzerWrapper();

  /**
   * Wraps {@link StandardAnalyzer}.
   */
  ShingleAnalyzerWrapper(int minShingleSize, int maxShingleSize);

  /**
   * The max shingle (token ngram) size
   *
   * @return The max shingle (token ngram) size
   */
  int getMaxShingleSize();

  /**
   * The min shingle (token ngram) size
   *
   * @return The min shingle (token ngram) size
   */
  int getMinShingleSize();

  std::wstring getTokenSeparator();

  bool isOutputUnigrams();

  bool isOutputUnigramsIfNoShingles();

  std::wstring getFillerToken();

  std::shared_ptr<Analyzer>
  getWrappedAnalyzer(const std::wstring &fieldName) override final;

protected:
  std::shared_ptr<Analyzer::TokenStreamComponents> wrapComponents(
      const std::wstring &fieldName,
      std::shared_ptr<Analyzer::TokenStreamComponents> components) override;

protected:
  std::shared_ptr<ShingleAnalyzerWrapper> shared_from_this()
  {
    return std::static_pointer_cast<ShingleAnalyzerWrapper>(
        org.apache.lucene.analysis.AnalyzerWrapper::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/shingle/
