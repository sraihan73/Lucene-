#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <cctype>
#include <memory>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/benchmark/byTask/feeds/DocData.h"

#include  "core/src/java/org/apache/lucene/document/Field.h"
#include  "core/src/java/org/apache/lucene/document/Document.h"
#include  "core/src/java/org/apache/lucene/document/FieldType.h"
#include  "core/src/java/org/apache/lucene/benchmark/byTask/utils/Config.h"
#include  "core/src/java/org/apache/lucene/benchmark/byTask/feeds/ContentSource.h"

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
namespace org::apache::lucene::benchmark::byTask::feeds
{

using Config = org::apache::lucene::benchmark::byTask::utils::Config;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;

/**
 * Creates {@link Document} objects. Uses a {@link ContentSource} to generate
 * {@link DocData} objects. Supports the following parameters:
 * <ul>
 * <li><b>content.source</b> - specifies the {@link ContentSource} class to use
 * (default <b>SingleDocSource</b>).
 * <li><b>doc.stored</b> - specifies whether fields should be stored (default
 * <b>false</b>).
 * <li><b>doc.body.stored</b> - specifies whether the body field should be
 * stored (default = <b>doc.stored</b>). <li><b>doc.tokenized</b> - specifies
 * whether fields should be tokenized (default <b>true</b>).
 * <li><b>doc.body.tokenized</b> - specifies whether the
 * body field should be tokenized (default = <b>doc.tokenized</b>).
 * <li><b>doc.body.offsets</b> - specifies whether to add offsets into the
 * postings index for the body field.  It is useful for highlighting.  (default
 * <b>false</b>) <li><b>doc.tokenized.norms</b> - specifies whether norms should
 * be stored in the index or not. (default <b>false</b>).
 * <li><b>doc.body.tokenized.norms</b> - specifies whether norms should be
 * stored in the index for the body field. This can be set to true, while
 * <code>doc.tokenized.norms</code> is set to false, to allow norms storing just
 * for the body field. (default <b>true</b>).
 * <li><b>doc.term.deque</b> - specifies whether term vectors should be stored
 * for fields (default <b>false</b>).
 * <li><b>doc.term.deque.positions</b> - specifies whether term vectors should
 * be stored with positions (default <b>false</b>).
 * <li><b>doc.term.deque.offsets</b> - specifies whether term vectors should be
 * stored with offsets (default <b>false</b>).
 * <li><b>doc.store.body.bytes</b> - specifies whether to store the raw bytes of
 * the document's content in the document (default <b>false</b>).
 * <li><b>doc.reuse.fields</b> - specifies whether Field and Document objects
 * should be reused (default <b>true</b>).
 * <li><b>doc.index.props</b> - specifies whether the properties returned by
 * <li><b>doc.random.id.limit</b> - if specified, docs will be assigned random
 * IDs from 0 to this limit.  This is useful with UpdateDoc
 * for testing performance of IndexWriter.updateDocument.
 * {@link DocData#getProps()} will be indexed. (default <b>false</b>).
 * </ul>
 */
class DocMaker : public std::enable_shared_from_this<DocMaker>
{
  GET_CLASS_NAME(DocMaker)

private:
  class LeftOver : public std::enable_shared_from_this<LeftOver>
  {
    GET_CLASS_NAME(LeftOver)
  private:
    std::shared_ptr<DocData> docdata;
    int cnt = 0;
  };

private:
  std::shared_ptr<Random> r;
  int updateDocIDLimit = 0;

  /**
   * Document state, supports reuse of field instances
   * across documents (see <code>reuseFields</code> parameter).
   */
protected:
  class DocState : public std::enable_shared_from_this<DocState>
  {
    GET_CLASS_NAME(DocState)

  private:
    const std::unordered_map<std::wstring, std::shared_ptr<Field>> fields;
    const std::unordered_map<std::wstring, std::shared_ptr<Field>>
        numericFields;
    const bool reuseFields;

  public:
    const std::shared_ptr<Document> doc;
    std::shared_ptr<DocData> docData = std::make_shared<DocData>();

    DocState(bool reuseFields, std::shared_ptr<FieldType> ft,
             std::shared_ptr<FieldType> bodyFt);

    /**
     * Returns a field corresponding to the field name. If
     * <code>reuseFields</code> was set to true, then it attempts to reuse a
     * Field instance. If such a field does not exist, it creates a new one.
     */
    virtual std::shared_ptr<Field> getField(const std::wstring &name,
                                            std::shared_ptr<FieldType> ft);

    virtual std::shared_ptr<Field> getNumericField(const std::wstring &name,
                                                   std::type_info numericType);
  };

private:
  bool storeBytes = false;

private:
  class DateUtil : public std::enable_shared_from_this<DateUtil>
  {
    GET_CLASS_NAME(DateUtil)
  public:
    std::shared_ptr<SimpleDateFormat> parser =
        std::make_shared<SimpleDateFormat>(L"dd-MMM-yyyy HH:mm:ss",
                                           Locale::ENGLISH);
    std::shared_ptr<Calendar> cal =
        Calendar::getInstance(TimeZone::getTimeZone(L"GMT"), Locale::ROOT);
    std::shared_ptr<ParsePosition> pos = std::make_shared<ParsePosition>(0);
    DateUtil();
  };

  // leftovers are thread local, because it is unsafe to share residues between
  // threads
private:
  std::shared_ptr<ThreadLocal<std::shared_ptr<LeftOver>>> leftovr =
      std::make_shared<ThreadLocal<std::shared_ptr<LeftOver>>>();
  std::shared_ptr<ThreadLocal<std::shared_ptr<DocState>>> docState =
      std::make_shared<ThreadLocal<std::shared_ptr<DocState>>>();
  std::shared_ptr<ThreadLocal<std::shared_ptr<DateUtil>>> dateParsers =
      std::make_shared<ThreadLocal<std::shared_ptr<DateUtil>>>();

public:
  static const std::wstring BODY_FIELD;
  static const std::wstring TITLE_FIELD;
  static const std::wstring DATE_FIELD;
  static const std::wstring DATE_MSEC_FIELD;
  static const std::wstring TIME_SEC_FIELD;
  static const std::wstring ID_FIELD;
  static const std::wstring BYTES_FIELD;
  static const std::wstring NAME_FIELD;

protected:
  std::shared_ptr<Config> config;

  std::shared_ptr<FieldType> valType;
  std::shared_ptr<FieldType> bodyValType;

  std::shared_ptr<ContentSource> source;
  bool reuseFields = false;
  bool indexProperties = false;

private:
  const std::shared_ptr<AtomicInteger> numDocsCreated =
      std::make_shared<AtomicInteger>();

public:
  DocMaker();

  // create a doc
  // use only part of the body, modify it to keep the rest (or use all if
  // size==0). reset the docdata properties so they are not added more than
  // once.
private:
  std::shared_ptr<Document>
  createDocument(std::shared_ptr<DocData> docData, int size,
                 int cnt) ;

  void resetLeftovers();

protected:
  virtual std::shared_ptr<DocState> getDocState();

  /**
   * Closes the {@link DocMaker}. The base implementation closes the
   * {@link ContentSource}, and it can be overridden to do more work (but make
   * sure to call super.close()).
   */
public:
  virtual ~DocMaker();

  /**
   * Creates a {@link Document} object ready for indexing. This method uses the
   * {@link ContentSource} to get the next document from the source, and creates
   * a {@link Document} object from the returned fields. If
   * <code>reuseFields</code> was set to true, it will reuse {@link Document}
   * and {@link Field} instances.
   */
  virtual std::shared_ptr<Document> makeDocument() ;

  /**
   * Same as {@link #makeDocument()}, only this method creates a document of the
   * given size input by <code>size</code>.
   */
  virtual std::shared_ptr<Document>
  makeDocument(int size) ;

  /** Reset inputs so that the test run would behave, input wise, as if it just
   * started. */
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual void resetInputs() ;

  /** Set the configuration parameters of this doc maker. */
  virtual void setConfig(std::shared_ptr<Config> config,
                         std::shared_ptr<ContentSource> source);
};

} // #include  "core/src/java/org/apache/lucene/benchmark/byTask/feeds/
