#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/OffsetAttribute.h"

#include  "core/src/java/org/apache/lucene/analysis/TokenStream.h"

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
namespace org::apache::lucene::search::highlight
{

using TokenStream = org::apache::lucene::analysis::TokenStream;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;

/**
 * {@link Fragmenter} implementation which breaks text up into same-size
 * fragments with no concerns over spotting sentence boundaries.
 */
class SimpleFragmenter : public std::enable_shared_from_this<SimpleFragmenter>,
                         public Fragmenter
{
  GET_CLASS_NAME(SimpleFragmenter)
private:
  static constexpr int DEFAULT_FRAGMENT_SIZE = 100;
  int currentNumFrags = 0;
  int fragmentSize = 0;
  std::shared_ptr<OffsetAttribute> offsetAtt;

public:
  SimpleFragmenter();

  /**
   *
   * @param fragmentSize size in number of characters of each fragment
   */
  SimpleFragmenter(int fragmentSize);

  /* (non-Javadoc)
   * @see org.apache.lucene.search.highlight.Fragmenter#start(java.lang.std::wstring,
   * org.apache.lucene.analysis.TokenStream)
   */
  void start(const std::wstring &originalText,
             std::shared_ptr<TokenStream> stream) override;

  /* (non-Javadoc)
   * @see org.apache.lucene.search.highlight.Fragmenter#isNewFragment()
   */
  bool isNewFragment() override;

  /**
   * @return size in number of characters of each fragment
   */
  virtual int getFragmentSize();

  /**
   * @param size size in characters of each fragment
   */
  virtual void setFragmentSize(int size);
};

} // #include  "core/src/java/org/apache/lucene/search/highlight/
