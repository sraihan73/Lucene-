#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharFilter.h"
#include "stringhelper.h"
#include <memory>
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
namespace org::apache::lucene::analysis::fa
{

using CharFilter = org::apache::lucene::analysis::CharFilter;

/**
 * CharFilter that replaces instances of Zero-width non-joiner with an
 * ordinary space.
 */
class PersianCharFilter : public CharFilter
{
  GET_CLASS_NAME(PersianCharFilter)

public:
  PersianCharFilter(std::shared_ptr<Reader> in_);

  int read(std::deque<wchar_t> &cbuf, int off,
           int len)  override;

  // optimized impl: some other charfilters consume with read()
  int read()  override;

protected:
  int correct(int currentOff) override;

protected:
  std::shared_ptr<PersianCharFilter> shared_from_this()
  {
    return std::static_pointer_cast<PersianCharFilter>(
        org.apache.lucene.analysis.CharFilter::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/fa/
