#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::document
{
class FieldType;
}

namespace org::apache::lucene::util
{
class BytesRef;
}
namespace org::apache::lucene::analysis
{
class Analyzer;
}
namespace org::apache::lucene::analysis
{
class TokenStream;
}
namespace org::apache::lucene::search::suggest::document
{
class CompletionTokenStream;
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
namespace org::apache::lucene::search::suggest::document
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using BytesRef = org::apache::lucene::util::BytesRef;

/**
 * <p>
 * Field that indexes a string value and a weight as a weighted completion
 * against a named suggester.
 * Field is tokenized, not stored and stores documents, frequencies and
 * positions. Field can be used to provide near real time document suggestions.
 * </p>
 * <p>
 * Besides the usual {@link org.apache.lucene.analysis.Analyzer}s,
 * {@link CompletionAnalyzer}
 * can be used to tune suggest field only parameters
 * (e.g. preserving token separators, preserving position increments
 * when converting the token stream to an automaton)
 * </p>
 * <p>
 * Example indexing usage:
 * <pre class="prettyprint">
 * document.add(new SuggestField(name, "suggestion", 4));
 * </pre>
 * To perform document suggestions based on the this field, use
 * {@link SuggestIndexSearcher#suggest(CompletionQuery, int, bool)}
 *
 * @lucene.experimental
 */
class SuggestField : public Field
{
  GET_CLASS_NAME(SuggestField)

  /** Default field type for suggest field */
public:
  static const std::shared_ptr<FieldType> FIELD_TYPE;

private:
  class StaticConstructor
      : public std::enable_shared_from_this<StaticConstructor>
  {
    GET_CLASS_NAME(StaticConstructor)
  public:
    StaticConstructor();
  };

private:
  static SuggestField::StaticConstructor staticConstructor;

public:
  static constexpr char TYPE = 0;

private:
  const std::shared_ptr<BytesRef> surfaceForm;
  const int weight;

  /**
   * Creates a {@link SuggestField}
   *
   * @param name   field name
   * @param value  field value to get suggestions on
   * @param weight field weight
   *
   * @throws IllegalArgumentException if either the name or value is null,
   * if value is an empty string, if the weight is negative, if value contains
   * any reserved characters
   */
public:
  SuggestField(const std::wstring &name, const std::wstring &value, int weight);

  std::shared_ptr<TokenStream>
  tokenStream(std::shared_ptr<Analyzer> analyzer,
              std::shared_ptr<TokenStream> reuse) override;

  /**
   * Wraps a <code>stream</code> with a CompletionTokenStream.
   *
   * Subclasses can override this method to change the indexing pipeline.
   */
protected:
  virtual std::shared_ptr<CompletionTokenStream>
  wrapTokenStream(std::shared_ptr<TokenStream> stream);

  /**
   * Returns a byte to denote the type of the field
   */
  virtual char type();

private:
  std::shared_ptr<BytesRef> buildSuggestPayload();

  bool isReserved(wchar_t c);

protected:
  std::shared_ptr<SuggestField> shared_from_this()
  {
    return std::static_pointer_cast<SuggestField>(
        org.apache.lucene.document.Field::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::suggest::document
