#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::document
{
class FieldType;
}

namespace org::apache::lucene::document
{
class Field;
}
namespace org::apache::lucene::index
{
class IndexableField;
}
namespace org::apache::lucene::document
{
class Document;
}
namespace org::apache::lucene::index
{
class SegmentCommitInfo;
}
namespace org::apache::lucene::store
{
class Directory;
}
namespace org::apache::lucene::analysis
{
class Analyzer;
}
namespace org::apache::lucene::search::similarities
{
class Similarity;
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
namespace org::apache::lucene::index
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using TextField = org::apache::lucene::document::TextField;
using Similarity = org::apache::lucene::search::similarities::Similarity;
using Directory = org::apache::lucene::store::Directory;

class DocHelper : public std::enable_shared_from_this<DocHelper>
{
  GET_CLASS_NAME(DocHelper)

public:
  static const std::shared_ptr<FieldType> customType;
  static const std::wstring FIELD_1_TEXT;
  static const std::wstring TEXT_FIELD_1_KEY;
  static std::shared_ptr<Field> textField1;

private:
  class StaticConstructor
      : public std::enable_shared_from_this<StaticConstructor>
  {
    GET_CLASS_NAME(StaticConstructor)
  public:
    StaticConstructor();
  };

private:
  static DocHelper::StaticConstructor staticConstructor;

public:
  static const std::shared_ptr<FieldType> customType2;
  static const std::wstring FIELD_2_TEXT;
  // Fields will be lexicographically sorted.  So, the order is: field, text,
  // two
  static std::deque<int> const FIELD_2_FREQS;
  static const std::wstring TEXT_FIELD_2_KEY;
  static std::shared_ptr<Field> textField2;

  static const std::shared_ptr<FieldType> customType3;
  static const std::wstring FIELD_3_TEXT;
  static const std::wstring TEXT_FIELD_3_KEY;
  static std::shared_ptr<Field> textField3;

  static const std::wstring KEYWORD_TEXT;
  static const std::wstring KEYWORD_FIELD_KEY;
  static std::shared_ptr<Field> keyField;

  static const std::shared_ptr<FieldType> customType5;
  static const std::wstring NO_NORMS_TEXT;
  static const std::wstring NO_NORMS_KEY;
  static std::shared_ptr<Field> noNormsField;

  static const std::shared_ptr<FieldType> customType6;
  static const std::wstring NO_TF_TEXT;
  static const std::wstring NO_TF_KEY;
  static std::shared_ptr<Field> noTFField;

  static const std::shared_ptr<FieldType> customType7;
  static const std::wstring UNINDEXED_FIELD_TEXT;
  static const std::wstring UNINDEXED_FIELD_KEY;
  static std::shared_ptr<Field> unIndField;

  static const std::wstring UNSTORED_1_FIELD_TEXT;
  static const std::wstring UNSTORED_FIELD_1_KEY;
  static std::shared_ptr<Field> unStoredField1;

  static const std::shared_ptr<FieldType> customType8;
  static const std::wstring UNSTORED_2_FIELD_TEXT;
  static const std::wstring UNSTORED_FIELD_2_KEY;
  static std::shared_ptr<Field> unStoredField2;

  static const std::wstring LAZY_FIELD_BINARY_KEY;
  static std::deque<char> LAZY_FIELD_BINARY_BYTES;
  static std::shared_ptr<Field> lazyFieldBinary;

  static const std::wstring LAZY_FIELD_KEY;
  static const std::wstring LAZY_FIELD_TEXT;
  static std::shared_ptr<Field> lazyField;

  static const std::wstring LARGE_LAZY_FIELD_KEY;
  static std::wstring LARGE_LAZY_FIELD_TEXT;
  static std::shared_ptr<Field> largeLazyField;

  // From Issue 509
  static const std::wstring FIELD_UTF1_TEXT;
  static const std::wstring TEXT_FIELD_UTF1_KEY;
  static std::shared_ptr<Field> textUtfField1;

  static const std::wstring FIELD_UTF2_TEXT;
  // Fields will be lexicographically sorted.  So, the order is: field, text,
  // two
  static std::deque<int> const FIELD_UTF2_FREQS;
  static const std::wstring TEXT_FIELD_UTF2_KEY;
  static std::shared_ptr<Field> textUtfField2;

  static std::unordered_map<std::wstring, std::any> nameValues;

  // ordered deque of all the fields...
  // could use LinkedHashMap for this purpose if Java1.4 is OK
  static std::deque<std::shared_ptr<Field>> fields;

  static std::unordered_map<std::wstring, std::shared_ptr<IndexableField>> all;
  static std::unordered_map<std::wstring, std::shared_ptr<IndexableField>>
      indexed;
  static std::unordered_map<std::wstring, std::shared_ptr<IndexableField>>
      stored;
  static std::unordered_map<std::wstring, std::shared_ptr<IndexableField>>
      unstored;
  static std::unordered_map<std::wstring, std::shared_ptr<IndexableField>>
      unindexed;
  static std::unordered_map<std::wstring, std::shared_ptr<IndexableField>>
      termvector;
  static std::unordered_map<std::wstring, std::shared_ptr<IndexableField>>
      notermvector;
  static std::unordered_map<std::wstring, std::shared_ptr<IndexableField>> lazy;
  static std::unordered_map<std::wstring, std::shared_ptr<IndexableField>>
      noNorms;
  static std::unordered_map<std::wstring, std::shared_ptr<IndexableField>> noTf;

private:
  static void
  add(std::unordered_map<std::wstring, std::shared_ptr<IndexableField>> &map_obj,
      std::shared_ptr<IndexableField> field);

  /**
   * Adds the fields above to a document
   * @param doc The document to write
   */
public:
  static void setupDoc(std::shared_ptr<Document> doc);

  /**
   * Writes the document to the directory using a segment
   * named "test"; returns the SegmentInfo describing the new
   * segment
   */
  static std::shared_ptr<SegmentCommitInfo>
  writeDoc(std::shared_ptr<Random> random, std::shared_ptr<Directory> dir,
           std::shared_ptr<Document> doc) ;

  /**
   * Writes the document to the directory using the analyzer
   * and the similarity score; returns the SegmentInfo
   * describing the new segment
   */
  static std::shared_ptr<SegmentCommitInfo>
  writeDoc(std::shared_ptr<Random> random, std::shared_ptr<Directory> dir,
           std::shared_ptr<Analyzer> analyzer,
           std::shared_ptr<Similarity> similarity,
           std::shared_ptr<Document> doc) ;

  static int numFields(std::shared_ptr<Document> doc);

  static std::shared_ptr<Document>
  createDocument(int n, const std::wstring &indexName, int numFields);
};

} // namespace org::apache::lucene::index
