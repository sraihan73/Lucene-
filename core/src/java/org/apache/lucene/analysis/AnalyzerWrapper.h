#pragma once
#include "Analyzer.h"
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis
{
class Analyzer;
}

namespace org::apache::lucene::analysis
{
class TokenStream;
}
namespace org::apache::lucene::util
{
class AttributeFactory;
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

using AttributeFactory = org::apache::lucene::util::AttributeFactory;

/**
 * Extension to {@link Analyzer} suitable for Analyzers which wrap
 * other Analyzers.
 *
 * <p>{@link #getWrappedAnalyzer(std::wstring)} allows the Analyzer
 * to wrap multiple Analyzers which are selected on a per field basis.
 *
 * <p>{@link #wrapComponents(std::wstring, Analyzer.TokenStreamComponents)} allows the
 * TokenStreamComponents of the wrapped Analyzer to then be wrapped
 * (such as adding a new {@link TokenFilter} to form new TokenStreamComponents.
 *
 * <p>{@link #wrapReader(std::wstring, Reader)} allows the Reader of the wrapped
 * Analyzer to then be wrapped (such as adding a new {@link CharFilter}.
 *
 * <p><b>Important:</b> If you do not want to wrap the TokenStream
 * using {@link #wrapComponents(std::wstring, Analyzer.TokenStreamComponents)}
 * or the Reader using {@link #wrapReader(std::wstring, Reader)} and just delegate
 * to other analyzers (like by field name), use {@link
 * DelegatingAnalyzerWrapper} as superclass!
 *
 * @see DelegatingAnalyzerWrapper
 */
class AnalyzerWrapper : public Analyzer
{
  GET_CLASS_NAME(AnalyzerWrapper)

  /**
   * Creates a new AnalyzerWrapper with the given reuse strategy.
   * <p>If you want to wrap a single delegate Analyzer you can probably
   * reuse its strategy when instantiating this subclass:
   * {@code super(delegate.getReuseStrategy());}.
GET_CLASS_NAME(:)
   * <p>If you choose different analyzers per field, use
   * {@link #PER_FIELD_REUSE_STRATEGY}.
   * @see #getReuseStrategy()
   */
protected:
  AnalyzerWrapper(std::shared_ptr<ReuseStrategy> reuseStrategy);

  /**
   * Retrieves the wrapped Analyzer appropriate for analyzing the field with
   * the given name
   *
   * @param fieldName Name of the field which is to be analyzed
   * @return Analyzer for the field with the given name.  Assumed to be non-null
   */
  virtual std::shared_ptr<Analyzer>
  getWrappedAnalyzer(const std::wstring &fieldName) = 0;

  /**
   * Wraps / alters the given TokenStreamComponents, taken from the wrapped
   * Analyzer, to form new components. It is through this method that new
   * TokenFilters can be added by AnalyzerWrappers. By default, the given
   * components are returned.
   *
   * @param fieldName
   *          Name of the field which is to be analyzed
   * @param components
   *          TokenStreamComponents taken from the wrapped Analyzer
   * @return Wrapped / altered TokenStreamComponents.
   */
  virtual std::shared_ptr<TokenStreamComponents>
  wrapComponents(const std::wstring &fieldName,
                 std::shared_ptr<TokenStreamComponents> components);

  /**
   * Wraps / alters the given TokenStream for normalization purposes, taken
   * from the wrapped Analyzer, to form new components. It is through this
   * method that new TokenFilters can be added by AnalyzerWrappers. By default,
   * the given token stream are returned.
   *
   * @param fieldName
   *          Name of the field which is to be analyzed
   * @param in
   *          TokenStream taken from the wrapped Analyzer
   * @return Wrapped / altered TokenStreamComponents.
   */
  virtual std::shared_ptr<TokenStream>
  wrapTokenStreamForNormalization(const std::wstring &fieldName,
                                  std::shared_ptr<TokenStream> in_);

  /**
   * Wraps / alters the given Reader. Through this method AnalyzerWrappers can
   * implement {@link #initReader(std::wstring, Reader)}. By default, the given reader
   * is returned.
   *
   * @param fieldName
   *          name of the field which is to be analyzed
   * @param reader
   *          the reader to wrap
   * @return the wrapped reader
   */
  virtual std::shared_ptr<Reader> wrapReader(const std::wstring &fieldName,
                                             std::shared_ptr<Reader> reader);

  /**
   * Wraps / alters the given Reader. Through this method AnalyzerWrappers can
   * implement {@link #initReaderForNormalization(std::wstring, Reader)}. By default,
   * the given reader  is returned.
   *
   * @param fieldName
   *          name of the field which is to be analyzed
   * @param reader
   *          the reader to wrap
   * @return the wrapped reader
   */
  virtual std::shared_ptr<Reader>
  wrapReaderForNormalization(const std::wstring &fieldName,
                             std::shared_ptr<Reader> reader);

  std::shared_ptr<TokenStreamComponents>
  createComponents(const std::wstring &fieldName) override final;

  std::shared_ptr<TokenStream>
  normalize(const std::wstring &fieldName,
            std::shared_ptr<TokenStream> in_) override final;

public:
  int getPositionIncrementGap(const std::wstring &fieldName) override;

  int getOffsetGap(const std::wstring &fieldName) override;

  std::shared_ptr<Reader>
  initReader(const std::wstring &fieldName,
             std::shared_ptr<Reader> reader) override final;

protected:
  std::shared_ptr<Reader>
  initReaderForNormalization(const std::wstring &fieldName,
                             std::shared_ptr<Reader> reader) override final;

  std::shared_ptr<AttributeFactory>
  attributeFactory(const std::wstring &fieldName) override final;

protected:
  std::shared_ptr<AnalyzerWrapper> shared_from_this()
  {
    return std::static_pointer_cast<AnalyzerWrapper>(
        Analyzer::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis
