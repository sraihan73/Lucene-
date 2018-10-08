#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>
#include <deque>

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

/**
 * Expert: A ScoreDoc which also contains information about
 * how to sort the referenced document.  In addition to the
 * document number and score, this object contains an array
 * of values for the document from the field(s) used to sort.
 * For example, if the sort criteria was to sort by fields
 * "a", "b" then "c", the <code>fields</code> object array
 * will have three elements, corresponding respectively to
 * the term values for the document in fields "a", "b" and "c".
 * The class of each element in the array will be either
 * Integer, Float or std::wstring depending on the type of values
 * in the terms of each field.
 *
 * <p>Created: Feb 11, 2004 1:23:38 PM
 *
 * @since   lucene 1.4
 * @see ScoreDoc
 * @see TopFieldDocs
 */
class FieldDoc : public ScoreDoc
{
  GET_CLASS_NAME(FieldDoc)

  /** Expert: The values which are used to sort the referenced document.
   * The order of these will match the original sort criteria given by a
   * Sort object.  Each Object will have been returned from
   * the <code>value</code> method corresponding
   * FieldComparator used to sort this field.
   * @see Sort
   * @see IndexSearcher#search(Query,int,Sort)
   */
public:
  std::deque<std::any> fields;

  /** Expert: Creates one of these objects with empty sort information. */
  FieldDoc(int doc, float score);

  /** Expert: Creates one of these objects with the given sort information. */
  FieldDoc(int doc, float score, std::deque<std::any> &fields);

  /** Expert: Creates one of these objects with the given sort information. */
  FieldDoc(int doc, float score, std::deque<std::any> &fields, int shardIndex);

  // A convenience method for debugging.
  virtual std::wstring toString();

protected:
  std::shared_ptr<FieldDoc> shared_from_this()
  {
    return std::static_pointer_cast<FieldDoc>(ScoreDoc::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
