#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis
{
class TokenStream;
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
namespace org::apache::lucene::search::highlight
{
using TokenStream = org::apache::lucene::analysis::TokenStream;

/**
 * Implements the policy for breaking text into multiple fragments for
 * consideration by the {@link Highlighter} class. A sophisticated
 * implementation may do this on the basis of detecting end of sentences in the
 * text.
 */
class Fragmenter
{
  GET_CLASS_NAME(Fragmenter)

  /**
   * Initializes the Fragmenter. You can grab references to the Attributes you
   * are interested in from tokenStream and then access the values in {@link
   * #isNewFragment()}.
   *
   * @param originalText the original source text
   * @param tokenStream the {@link TokenStream} to be fragmented
   */
public:
  virtual void start(const std::wstring &originalText,
                     std::shared_ptr<TokenStream> tokenStream) = 0;

  /**
   * Test to see if this token from the stream should be held in a new
   * TextFragment. Every time this is called, the TokenStream
   * passed to start(std::wstring, TokenStream) will have been incremented.
   *
   */
  virtual bool isNewFragment() = 0;
};

} // namespace org::apache::lucene::search::highlight
