#pragma once
#include "stringhelper.h"
#include <algorithm>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/document/Document.h"

#include  "core/src/java/org/apache/lucene/search/suggest/Suggestion.h"
#include  "core/src/java/org/apache/lucene/index/RandomIndexWriter.h"
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
namespace org::apache::lucene::search::suggest
{

using Document = org::apache::lucene::document::Document;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

// See: https://issues.apache.org/jira/browse/SOLR-12028 Tests cannot remove
// files on Windows machines occasionally
class DocumentDictionaryTest : public LuceneTestCase
{
  GET_CLASS_NAME(DocumentDictionaryTest)

public:
  static const std::wstring FIELD_NAME;
  static const std::wstring WEIGHT_FIELD_NAME;
  static const std::wstring PAYLOAD_FIELD_NAME;
  static const std::wstring CONTEXT_FIELD_NAME;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testEmptyReader() throws
  // java.io.IOException
  virtual void testEmptyReader() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testBasic() throws java.io.IOException
  virtual void testBasic() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testWithOptionalPayload() throws
  // java.io.IOException
  virtual void testWithOptionalPayload() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testWithoutPayload() throws
  // java.io.IOException
  virtual void testWithoutPayload() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testWithContexts() throws
  // java.io.IOException
  virtual void testWithContexts() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testWithDeletions() throws
  // java.io.IOException
  virtual void testWithDeletions() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testMultiValuedField() throws
  // java.io.IOException
  virtual void testMultiValuedField() ;

  /** Returns Pair(deque of invalid document terms, Map of document term -&gt;
   * document) */
private:
  std::unordered_map::Entry<
      std::deque<std::wstring>,
      std::unordered_map<std::wstring, std::shared_ptr<Document>>>
  generateIndexDocuments(int ndocs, bool requiresContexts);

  std::deque<std::shared_ptr<Suggestion>> indexMultiValuedDocuments(
      int numDocs,
      std::shared_ptr<RandomIndexWriter> writer) ;

private:
  class Suggestion : public std::enable_shared_from_this<Suggestion>
  {
    GET_CLASS_NAME(Suggestion)
  private:
    int64_t weight = 0;
    std::shared_ptr<BytesRef> payload;
    std::shared_ptr<Set<std::shared_ptr<BytesRef>>> contexts;
    std::shared_ptr<BytesRef> term;
  };

protected:
  std::shared_ptr<DocumentDictionaryTest> shared_from_this()
  {
    return std::static_pointer_cast<DocumentDictionaryTest>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/suggest/
