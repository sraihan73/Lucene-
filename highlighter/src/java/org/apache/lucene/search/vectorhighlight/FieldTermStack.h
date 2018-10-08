#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <algorithm>
#include <any>
#include <cmath>
#include <deque>
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/vectorhighlight/TermInfo.h"

#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/search/vectorhighlight/FieldQuery.h"

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

/**
 * <code>FieldTermStack</code> is a stack that keeps query terms in the
 * specified field of the document to be highlighted.
 */
class FieldTermStack : public std::enable_shared_from_this<FieldTermStack>
{
  GET_CLASS_NAME(FieldTermStack)

private:
  const std::wstring fieldName;

public:
  std::deque<std::shared_ptr<TermInfo>> termList =
      std::deque<std::shared_ptr<TermInfo>>();

  // public static void main( std::wstring[] args ) throws Exception {
  //  Analyzer analyzer = new WhitespaceAnalyzer(Version.LATEST);
  //  QueryParser parser = new QueryParser(Version.LATEST,  "f", analyzer );
  //  Query query = parser.parse( "a x:b" );
  //  FieldQuery fieldQuery = new FieldQuery( query, true, false );

  //  Directory dir = new RAMDirectory();
  //  IndexWriter writer = new IndexWriter(dir, new
  //  IndexWriterConfig(Version.LATEST, analyzer)); Document doc = new
  //  Document(); FieldType ft = new FieldType(TextField.TYPE_STORED);
  //  ft.setStoreTermVectors(true);
  //  ft.setStoreTermVectorOffsets(true);
  //  ft.setStoreTermVectorPositions(true);
  //  doc.add( new Field( "f", ft, "a a a b b c a b b c d e f" ) );
  //  doc.add( new Field( "f", ft, "b a b a f" ) );
  //  writer.addDocument( doc );
  //  writer.close();

  //  IndexReader reader = IndexReader.open(dir1);
  //  new FieldTermStack( reader, 0, "f", fieldQuery );
  //  reader.close();
  //}

  /**
   * a constructor.
   *
   * @param reader IndexReader of the index
   * @param docId document id to be highlighted
   * @param fieldName field of the document to be highlighted
   * @param fieldQuery FieldQuery object
   * @throws IOException If there is a low-level I/O error
   */
  FieldTermStack(std::shared_ptr<IndexReader> reader, int docId,
                 const std::wstring &fieldName,
                 std::shared_ptr<FieldQuery> fieldQuery) ;

  /**
   * @return field name
   */
  virtual std::wstring getFieldName();

  /**
   * @return the top TermInfo object of the stack
   */
  virtual std::shared_ptr<TermInfo> pop();

  /**
   * @param termInfo the TermInfo object to be put on the top of the stack
   */
  virtual void push(std::shared_ptr<TermInfo> termInfo);

  /**
   * to know whether the stack is empty
   *
   * @return true if the stack is empty, false if not
   */
  virtual bool isEmpty();

  /**
   * Single term with its position/offsets in the document and IDF weight.
   * It is Comparable but considers only position.
   */
public:
  class TermInfo : public std::enable_shared_from_this<TermInfo>,
                   public Comparable<std::shared_ptr<TermInfo>>
  {
    GET_CLASS_NAME(TermInfo)

  private:
    const std::wstring text;
    const int startOffset;
    const int endOffset;
    const int position;

    // IDF-weight of this term
    const float weight;

    // pointer to other TermInfo's at the same position.
    // this is a circular deque, so with no syns, just points to itself
    std::shared_ptr<TermInfo> next;

  public:
    TermInfo(const std::wstring &text, int startOffset, int endOffset,
             int position, float weight);

    virtual void setNext(std::shared_ptr<TermInfo> next);
    /**
     * Returns the next TermInfo at this same position.
     * This is a circular deque!
     */
    virtual std::shared_ptr<TermInfo> getNext();
    virtual std::wstring getText();
    virtual int getStartOffset();
    virtual int getEndOffset();
    virtual int getPosition();
    virtual float getWeight();

    virtual std::wstring toString();

    int compareTo(std::shared_ptr<TermInfo> o) override;
    virtual int hashCode();

    bool equals(std::any obj) override;
  };
};

} // #include  "core/src/java/org/apache/lucene/search/vectorhighlight/
