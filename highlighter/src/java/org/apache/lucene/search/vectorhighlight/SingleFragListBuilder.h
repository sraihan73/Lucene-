#pragma once
#include "stringhelper.h"
#include <limits>
#include <deque>
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/vectorhighlight/FieldFragList.h"

#include  "core/src/java/org/apache/lucene/search/vectorhighlight/FieldPhraseList.h"

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
namespace org::apache::lucene::search::vectorhighlight
{

/**
 * An implementation class of {@link FragListBuilder} that generates one {@link
 * WeightedFragInfo} object. Typical use case of this class is that you can get
 * an entire field contents by using both of this class and {@link
 * SimpleFragmentsBuilder}.<br> <pre class="prettyprint"> FastVectorHighlighter
 * h = new FastVectorHighlighter( true, true, new SingleFragListBuilder(), new
 * SimpleFragmentsBuilder() );
 * </pre>
 */
class SingleFragListBuilder
    : public std::enable_shared_from_this<SingleFragListBuilder>,
      public FragListBuilder
{
  GET_CLASS_NAME(SingleFragListBuilder)

public:
  std::shared_ptr<FieldFragList>
  createFieldFragList(std::shared_ptr<FieldPhraseList> fieldPhraseList,
                      int fragCharSize) override;
};

} // #include  "core/src/java/org/apache/lucene/search/vectorhighlight/
