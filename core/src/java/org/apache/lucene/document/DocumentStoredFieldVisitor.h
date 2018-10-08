#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/document/Document.h"

#include  "core/src/java/org/apache/lucene/index/FieldInfo.h"

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

using FieldInfo = org::apache::lucene::index::FieldInfo;
using StoredFieldVisitor = org::apache::lucene::index::StoredFieldVisitor;

/** A {@link StoredFieldVisitor} that creates a {@link
 *  Document} from stored fields.
 *  <p>
 *  This visitor supports loading all stored fields, or only specific
 *  requested fields provided from a {@link Set}.
 *  <p>
 *  This is used by {@link IndexReader#document(int)} to load a
 *  document.
 *
 * @lucene.experimental */

class DocumentStoredFieldVisitor : public StoredFieldVisitor
{
  GET_CLASS_NAME(DocumentStoredFieldVisitor)
private:
  const std::shared_ptr<Document> doc = std::make_shared<Document>();
  const std::shared_ptr<Set<std::wstring>> fieldsToAdd;

  /**
   * Load only fields named in the provided <code>Set&lt;std::wstring&gt;</code>.
   * @param fieldsToAdd Set of fields to load, or <code>null</code> (all
   * fields).
   */
public:
  DocumentStoredFieldVisitor(std::shared_ptr<Set<std::wstring>> fieldsToAdd);

  /** Load only fields named in the provided fields. */
  DocumentStoredFieldVisitor(std::deque<std::wstring> &fields);

  /** Load all stored fields. */
  DocumentStoredFieldVisitor();

  void binaryField(std::shared_ptr<FieldInfo> fieldInfo,
                   std::deque<char> &value)  override;

  void stringField(std::shared_ptr<FieldInfo> fieldInfo,
                   std::deque<char> &value)  override;

  void intField(std::shared_ptr<FieldInfo> fieldInfo, int value) override;

  void longField(std::shared_ptr<FieldInfo> fieldInfo,
                 int64_t value) override;

  void floatField(std::shared_ptr<FieldInfo> fieldInfo, float value) override;

  void doubleField(std::shared_ptr<FieldInfo> fieldInfo, double value) override;

  Status
  needsField(std::shared_ptr<FieldInfo> fieldInfo)  override;

  /**
   * Retrieve the visited document.
   * @return {@link Document} populated with stored fields. Note that only
   *         the stored information in the field instances is valid,
   *         data such as indexing options, term deque options,
   *         etc is not set.
   */
  virtual std::shared_ptr<Document> getDocument();

protected:
  std::shared_ptr<DocumentStoredFieldVisitor> shared_from_this()
  {
    return std::static_pointer_cast<DocumentStoredFieldVisitor>(
        org.apache.lucene.index.StoredFieldVisitor::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/document/
