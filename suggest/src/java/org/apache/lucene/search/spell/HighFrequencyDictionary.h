#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/IndexReader.h"

#include  "core/src/java/org/apache/lucene/search/suggest/InputIterator.h"
#include  "core/src/java/org/apache/lucene/util/BytesRefBuilder.h"
#include  "core/src/java/org/apache/lucene/index/TermsEnum.h"
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"

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
namespace org::apache::lucene::search::spell
{

using IndexReader = org::apache::lucene::index::IndexReader;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using InputIterator = org::apache::lucene::search::suggest::InputIterator;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;

/**
 * HighFrequencyDictionary: terms taken from the given field
 * of a Lucene index, which appear in a number of documents
 * above a given threshold.
 *
 * Threshold is a value in [0..1] representing the minimum
 * number of documents (of the total) where a term should appear.
 *
 * Based on LuceneDictionary.
 */
class HighFrequencyDictionary
    : public std::enable_shared_from_this<HighFrequencyDictionary>,
      public Dictionary
{
  GET_CLASS_NAME(HighFrequencyDictionary)
private:
  std::shared_ptr<IndexReader> reader;
  std::wstring field;
  float thresh = 0;

  /**
   * Creates a new Dictionary, pulling source terms from
   * the specified <code>field</code> in the provided <code>reader</code>.
   * <p>
   * Terms appearing in less than <code>thresh</code> percentage of documents
   * will be excluded.
   */
public:
  HighFrequencyDictionary(std::shared_ptr<IndexReader> reader,
                          const std::wstring &field, float thresh);

  std::shared_ptr<InputIterator> getEntryIterator()  override;

public:
  class HighFrequencyIterator final
      : public std::enable_shared_from_this<HighFrequencyIterator>,
        public InputIterator
  {
    GET_CLASS_NAME(HighFrequencyIterator)
  private:
    std::shared_ptr<HighFrequencyDictionary> outerInstance;

    const std::shared_ptr<BytesRefBuilder> spare =
        std::make_shared<BytesRefBuilder>();
    const std::shared_ptr<TermsEnum> termsEnum;
    int minNumDocs = 0;
    int64_t freq = 0;

  public:
    HighFrequencyIterator(std::shared_ptr<HighFrequencyDictionary>
                              outerInstance) ;

  private:
    bool isFrequent(int freq);

  public:
    int64_t weight() override;

    std::shared_ptr<BytesRef> next()  override;

    std::shared_ptr<BytesRef> payload() override;

    bool hasPayloads() override;

    std::shared_ptr<Set<std::shared_ptr<BytesRef>>> contexts() override;

    bool hasContexts() override;
  };
};

} // #include  "core/src/java/org/apache/lucene/search/spell/
