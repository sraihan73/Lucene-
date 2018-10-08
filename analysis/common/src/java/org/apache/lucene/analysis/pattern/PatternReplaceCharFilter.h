#pragma once
#include "../charfilter/BaseCharFilter.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

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
namespace org::apache::lucene::analysis::pattern
{

using BaseCharFilter =
    org::apache::lucene::analysis::charfilter::BaseCharFilter;

/**
 * CharFilter that uses a regular expression for the target of replace string.
 * The pattern match will be done in each "block" in char stream.
 *
 * <p>
 * ex1) source="aa&nbsp;&nbsp;bb&nbsp;aa&nbsp;bb", pattern="(aa)\\s+(bb)"
 * replacement="$1#$2"<br> output="aa#bb&nbsp;aa#bb"
 * </p>
 *
 * NOTE: If you produce a phrase that has different length to source string
 * and the field is used for highlighting for a term of the phrase, you will
 * face a trouble.
 *
 * <p>
 * ex2) source="aa123bb", pattern="(aa)\\d+(bb)" replacement="$1&nbsp;$2"<br>
 * output="aa&nbsp;bb"<br>
 * and you want to search bb and highlight it, you will get<br>
 * highlight snippet="aa1&lt;em&gt;23bb&lt;/em&gt;"
 * </p>
 *
 * @since Solr 1.5
 */
class PatternReplaceCharFilter : public BaseCharFilter
{
  GET_CLASS_NAME(PatternReplaceCharFilter)

private:
  const std::shared_ptr<Pattern> pattern;
  const std::wstring replacement;
  std::shared_ptr<Reader> transformedInput;

public:
  PatternReplaceCharFilter(std::shared_ptr<Pattern> pattern,
                           const std::wstring &replacement,
                           std::shared_ptr<Reader> in_);

  int read(std::deque<wchar_t> &cbuf, int off,
           int len)  override;

private:
  void fill() ;

public:
  int read()  override;

protected:
  int correct(int currentOff) override;

  /**
   * Replace pattern in input and mark correction offsets.
   */
public:
  virtual std::shared_ptr<std::wstring>
  processPattern(std::shared_ptr<std::wstring> input);

protected:
  std::shared_ptr<PatternReplaceCharFilter> shared_from_this()
  {
    return std::static_pointer_cast<PatternReplaceCharFilter>(
        org.apache.lucene.analysis.charfilter
            .BaseCharFilter::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/pattern/
