#pragma once
#include "stringhelper.h"
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/Directory.h"

#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include  "core/src/java/org/apache/lucene/index/RandomIndexWriter.h"

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
namespace org::apache::lucene::queries::mlt
{

using IndexReader = org::apache::lucene::index::IndexReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestMoreLikeThis : public LuceneTestCase
{
  GET_CLASS_NAME(TestMoreLikeThis)

private:
  static const std::wstring SHOP_TYPE;
  static const std::wstring FOR_SALE;
  static const std::wstring NOT_FOR_SALE;

  std::shared_ptr<Directory> directory;
  std::shared_ptr<IndexReader> reader;
  std::shared_ptr<IndexSearcher> searcher;

public:
  void setUp()  override;

  void tearDown()  override;

private:
  void addDoc(std::shared_ptr<RandomIndexWriter> writer,
              const std::wstring &text) ;

  void addDoc(std::shared_ptr<RandomIndexWriter> writer,
              std::deque<std::wstring> &texts) ;

public:
  virtual void testBoostFactor() ;

private:
  std::unordered_map<std::wstring, float>
  getOriginalValues() ;

  // LUCENE-3326
public:
  virtual void testMultiFields() ;

  // LUCENE-5725
  virtual void testMultiValues() ;

  // just basic equals/hashcode etc
  virtual void testMoreLikeThisQuery() ;

  virtual void testTopN() ;

private:
  std::deque<std::wstring> generateStrSeq(int from, int size);

  int addShopDoc(std::shared_ptr<RandomIndexWriter> writer,
                 const std::wstring &type, std::deque<std::wstring> &weSell,
                 std::deque<std::wstring> &weDontSell) ;

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @AwaitsFix(bugUrl =
  // "https://issues.apache.org/jira/browse/LUCENE-7161") public void
  // testMultiFieldShouldReturnPerFieldBooleanQuery() throws Exception
  virtual void
  testMultiFieldShouldReturnPerFieldBooleanQuery() ;
  // TODO: add tests for the MoreLikeThisQuery

protected:
  std::shared_ptr<TestMoreLikeThis> shared_from_this()
  {
    return std::static_pointer_cast<TestMoreLikeThis>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/queries/mlt/
