#pragma once
#include "stringhelper.h"
#include <memory>

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
 * Subclasses of CharFilter can be chained to filter a Reader
 * They can be used as {@link java.io.Reader} with additional offset
GET_CLASS_NAME(es)
 * correction. {@link Tokenizer}s will automatically use {@link #correctOffset}
 * if a CharFilter subclass is used.
 * <p>
 * This class is abstract: at a minimum you must implement {@link #read(char[],
int, int)},
 * transforming the input in some way from {@link #input}, and {@link
#correct(int)} GET_CLASS_NAME(is)
 * to adjust the offsets to match the originals.
 * <p>
 * You can optionally provide more efficient implementations of additional
methods
 * like {@link #read()}, {@link #read(char[])}, {@link
#read(java.nio.CharBuffer)},
 * but this is not required.
 * <p>
 * For examples and integration with {@link Analyzer}, see the
 * {@link org.apache.lucene.analysis Analysis package documentation}.
 */
// the way java.io.FilterReader should work!
class CharFilter : public Reader
{
  GET_CLASS_NAME(CharFilter)
  /**
   * The underlying character-input stream.
   */
protected:
  const std::shared_ptr<Reader> input;

  /**
   * Create a new CharFilter wrapping the provided reader.
   * @param input a Reader, can also be a CharFilter for chaining.
   */
public:
  CharFilter(std::shared_ptr<Reader> input);

  /**
   * Closes the underlying input stream.
   * <p>
   * <b>NOTE:</b>
   * The default implementation closes the input Reader, so
   * be sure to call <code>super.close()</code> when overriding this method.
   */
  virtual ~CharFilter();

  /**
   * Subclasses override to correct the current offset.
   *
   * @param currentOff current offset
   * @return corrected offset
   */
protected:
  virtual int correct(int currentOff) = 0;

  /**
   * Chains the corrected offset through the input
   * CharFilter(s).
   */
public:
  int correctOffset(int currentOff);

protected:
  std::shared_ptr<CharFilter> shared_from_this()
  {
    return std::static_pointer_cast<CharFilter>(
        java.io.Reader::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/
