#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::document
{
class Document;
}

namespace org::apache::lucene::search
{
class Query;
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
namespace org::apache::lucene::search
{

using Document = org::apache::lucene::document::Document;

/**
 * subclass of TestSimpleExplanations that adds a lot of filler docs which will
 * be ignored at query time. These filler docs will either all be empty in which
 * case the queries will be unmodified, or they will all use terms from same set
 * of source data as our regular docs (to emphasis the DocFreq factor in
 * scoring), in which case the queries will be wrapped so they can be excluded.
 */
// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Slow public class TestSimpleExplanationsWithFillerDocs
// extends TestSimpleExplanations
class TestSimpleExplanationsWithFillerDocs : public TestSimpleExplanations
{

  /** num of empty docs injected between every doc in the index */
private:
  static const int NUM_FILLER_DOCS = BooleanScorer::SIZE;
  /** num of empty docs injected prior to the first doc in the (main) index */
  static int PRE_FILLER_DOCS;
  /**
   * If non-null then the filler docs are not empty, and need to be filtered out
   * from queries using this as both field name &amp; field value
   */
public:
  static std::wstring EXTRA;

private:
  static const std::shared_ptr<Document> EMPTY_DOC;

  /**
   * Replaces the index created by our superclass with a new one that includes a
lot of docs filler docs.
   * {@link #qtest} will account for these extra filler docs.
GET_CLASS_NAME(with)
   * @see #qtest
   */
public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @BeforeClass public static void replaceIndex() throws
  // Exception
  static void replaceIndex() ;

private:
  static std::shared_ptr<Document> makeFillerDoc();

  /**
   * Adjusts <code>expDocNrs</code> based on the filler docs injected in the
   * index, and if neccessary wraps the <code>q</code> in a BooleanQuery that
   * will filter out all filler docs using the {@link #EXTRA} field.
   *
   * @see #replaceIndex
   */
public:
  void qtest(std::shared_ptr<Query> q,
             std::deque<int> &expDocNrs)  override;

  void testMA1()  override;
  void testMA2()  override;

protected:
  std::shared_ptr<TestSimpleExplanationsWithFillerDocs> shared_from_this()
  {
    return std::static_pointer_cast<TestSimpleExplanationsWithFillerDocs>(
        TestSimpleExplanations::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
