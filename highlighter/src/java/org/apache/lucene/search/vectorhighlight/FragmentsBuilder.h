#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/IndexReader.h"

#include  "core/src/java/org/apache/lucene/search/vectorhighlight/FieldFragList.h"
#include  "core/src/java/org/apache/lucene/search/highlight/Encoder.h"

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

using IndexReader = org::apache::lucene::index::IndexReader;
using Encoder = org::apache::lucene::search::highlight::Encoder;

/**
 * {@link org.apache.lucene.search.vectorhighlight.FragmentsBuilder} is an
interface for fragments (snippets) builder classes.
 * A {@link org.apache.lucene.search.vectorhighlight.FragmentsBuilder} class can
be plugged in to GET_CLASS_NAME(es.)
 * {@link org.apache.lucene.search.vectorhighlight.FastVectorHighlighter}.
 */
class FragmentsBuilder
{
  GET_CLASS_NAME(FragmentsBuilder)

  /**
   * create a fragment.
   *
   * @param reader IndexReader of the index
   * @param docId document id to be highlighted
   * @param fieldName field of the document to be highlighted
   * @param fieldFragList FieldFragList object
   * @return a created fragment or null when no fragment created
   * @throws IOException If there is a low-level I/O error
   */
public:
  virtual std::wstring
  createFragment(std::shared_ptr<IndexReader> reader, int docId,
                 const std::wstring &fieldName,
                 std::shared_ptr<FieldFragList> fieldFragList) = 0;

  /**
   * create multiple fragments.
   *
   * @param reader IndexReader of the index
   * @param docId document id to be highlighter
   * @param fieldName field of the document to be highlighted
   * @param fieldFragList FieldFragList object
   * @param maxNumFragments maximum number of fragments
   * @return created fragments or null when no fragments created.
   *         size of the array can be less than maxNumFragments
   * @throws IOException If there is a low-level I/O error
   */
  virtual std::deque<std::wstring>
  createFragments(std::shared_ptr<IndexReader> reader, int docId,
                  const std::wstring &fieldName,
                  std::shared_ptr<FieldFragList> fieldFragList,
                  int maxNumFragments) = 0;

  /**
   * create a fragment.
   *
   * @param reader IndexReader of the index
   * @param docId document id to be highlighted
   * @param fieldName field of the document to be highlighted
   * @param fieldFragList FieldFragList object
   * @param preTags pre-tags to be used to highlight terms
   * @param postTags post-tags to be used to highlight terms
   * @param encoder an encoder that generates encoded text
   * @return a created fragment or null when no fragment created
   * @throws IOException If there is a low-level I/O error
   */
  virtual std::wstring
  createFragment(std::shared_ptr<IndexReader> reader, int docId,
                 const std::wstring &fieldName,
                 std::shared_ptr<FieldFragList> fieldFragList,
                 std::deque<std::wstring> &preTags,
                 std::deque<std::wstring> &postTags,
                 std::shared_ptr<Encoder> encoder) = 0;

  /**
   * create multiple fragments.
   *
   * @param reader IndexReader of the index
   * @param docId document id to be highlighter
   * @param fieldName field of the document to be highlighted
   * @param fieldFragList FieldFragList object
   * @param maxNumFragments maximum number of fragments
   * @param preTags pre-tags to be used to highlight terms
   * @param postTags post-tags to be used to highlight terms
   * @param encoder an encoder that generates encoded text
   * @return created fragments or null when no fragments created.
   *         size of the array can be less than maxNumFragments
   * @throws IOException If there is a low-level I/O error
   */
  virtual std::deque<std::wstring>
  createFragments(std::shared_ptr<IndexReader> reader, int docId,
                  const std::wstring &fieldName,
                  std::shared_ptr<FieldFragList> fieldFragList,
                  int maxNumFragments, std::deque<std::wstring> &preTags,
                  std::deque<std::wstring> &postTags,
                  std::shared_ptr<Encoder> encoder) = 0;
};

} // #include  "core/src/java/org/apache/lucene/search/vectorhighlight/
