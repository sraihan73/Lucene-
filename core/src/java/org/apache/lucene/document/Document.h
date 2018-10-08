#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/IndexableField.h"

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
namespace org::apache::lucene::document
{

using IndexableField = org::apache::lucene::index::IndexableField;
using BytesRef = org::apache::lucene::util::BytesRef;

/** Documents are the unit of indexing and search.
 *
 * A Document is a set of fields.  Each field has a name and a textual value.
 * A field may be {@link org.apache.lucene.index.IndexableFieldType#stored()
 * stored} with the document, in which case it is returned with search hits on
 * the document.  Thus each document should typically contain one or more stored
 * fields which uniquely identify it.
 *
 * <p>Note that fields which are <i>not</i> {@link
 * org.apache.lucene.index.IndexableFieldType#stored() stored} are <i>not</i>
 * available in documents retrieved from the index, e.g. with {@link
 * ScoreDoc#doc} or {@link IndexReader#document(int)}.
 */

class Document final : public std::enable_shared_from_this<Document>,
                       public std::deque<std::shared_ptr<IndexableField>>
{
  GET_CLASS_NAME(Document)

private:
  const std::deque<std::shared_ptr<IndexableField>> fields =
      std::deque<std::shared_ptr<IndexableField>>();

  /** Constructs a new document with no fields. */
public:
  Document();

  std::shared_ptr<Iterator<std::shared_ptr<IndexableField>>>
  iterator() override;

  /**
   * <p>Adds a field to a document.  Several fields may be added with
   * the same name.  In this case, if the fields are indexed, their text is
   * treated as though appended for the purposes of search.</p>
   * <p> Note that add like the removeField(s) methods only makes sense
   * prior to adding a document to an index. These methods cannot
   * be used to change the content of an existing index! In order to achieve
   * this, a document has to be deleted from an index and a new changed version
   * of that document has to be added.</p>
   */
  void add(std::shared_ptr<IndexableField> field);

  /**
   * <p>Removes field with the specified name from the document.
   * If multiple fields exist with this name, this method removes the first
   * field that has been added. If there is no field with the specified name,
   * the document remains unchanged.</p> <p> Note that the removeField(s)
   * methods like the add method only make sense prior to adding a document to
   * an index. These methods cannot be used to change the content of an existing
   * index! In order to achieve this, a document has to be deleted from an index
   * and a new changed version of that document has to be added.</p>
   */
  void removeField(const std::wstring &name);

  /**
   * <p>Removes all fields with the given name from the document.
   * If there is no field with the specified name, the document remains
   * unchanged.</p> <p> Note that the removeField(s) methods like the add method
   * only make sense prior to adding a document to an index. These methods
   * cannot be used to change the content of an existing index! In order to
   * achieve this, a document has to be deleted from an index and a new changed
   * version of that document has to be added.</p>
   */
  void removeFields(const std::wstring &name);

  /**
   * Returns an array of byte arrays for of the fields that have the name
   * specified as the method parameter.  This method returns an empty array when
   * there are no matching fields.  It never returns null.
   *
   * @param name the name of the field
   * @return a <code>BytesRef[]</code> of binary field values
   */
  std::deque<std::shared_ptr<BytesRef>>
  getBinaryValues(const std::wstring &name);

  /**
   * Returns an array of bytes for the first (or only) field that has the name
   * specified as the method parameter. This method will return
   * <code>null</code> if no binary fields with the specified name are
   * available. There may be non-binary fields with the same name.
   *
   * @param name the name of the field.
   * @return a <code>BytesRef</code> containing the binary field value or
   * <code>null</code>
   */
  std::shared_ptr<BytesRef> getBinaryValue(const std::wstring &name);

  /** Returns a field with the given name if any exist in this document, or
   * null.  If multiple fields exists with this name, this method returns the
   * first value added.
   */
  std::shared_ptr<IndexableField> getField(const std::wstring &name);

  /**
   * Returns an array of {@link IndexableField}s with the given name.
   * This method returns an empty array when there are no
   * matching fields.  It never returns null.
   *
   * @param name the name of the field
   * @return a <code>Field[]</code> array
   */
  std::deque<std::shared_ptr<IndexableField>>
  getFields(const std::wstring &name);

  /** Returns a List of all the fields in a document.
   * <p>Note that fields which are <i>not</i> stored are
   * <i>not</i> available in documents retrieved from the
   * index, e.g. {@link IndexSearcher#doc(int)} or {@link
   * IndexReader#document(int)}.
   *
   * @return an immutable <code>List&lt;Field&gt;</code>
   */
  std::deque<std::shared_ptr<IndexableField>> getFields();

private:
  static std::deque<std::wstring> const NO_STRINGS;

  /**
   * Returns an array of values of the field specified as the method parameter.
   * This method returns an empty array when there are no
   * matching fields.  It never returns null.
   * For a numeric {@link StoredField} it returns the string value of the
   * number. If you want the actual numeric field instances back, use {@link
   * #getFields}.
   * @param name the name of the field
   * @return a <code>std::wstring[]</code> of field values
   */
public:
  std::deque<std::wstring> getValues(const std::wstring &name);

  /** Returns the string value of the field with the given name if any exist in
   * this document, or null.  If multiple fields exist with this name, this
   * method returns the first value added. If only binary fields with this name
   * exist, returns null.
   * For a numeric {@link StoredField} it returns the string value of the
   * number. If you want the actual numeric field instance back, use {@link
   * #getField}.
   */
  std::wstring get(const std::wstring &name);

  /** Prints the fields of a document for human consumption. */
  virtual std::wstring toString();

  /** Removes all the fields from document. */
  void clear();
};

} // #include  "core/src/java/org/apache/lucene/document/
