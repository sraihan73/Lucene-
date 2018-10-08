#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis
{
class Analyzer;
}

namespace org::apache::lucene::index
{
class IndexReader;
}
namespace org::apache::lucene::store
{
class Directory;
}
namespace org::apache::lucene::document
{
class Document;
}
namespace org::apache::lucene::document
{
class FieldType;
}
namespace org::apache::lucene::search
{
class ScoreDoc;
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
namespace org::apache::lucene::classification::utils
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using Document = org::apache::lucene::document::Document;
using FieldType = org::apache::lucene::document::FieldType;
using IndexReader = org::apache::lucene::index::IndexReader;
using ScoreDoc = org::apache::lucene::search::ScoreDoc;
using Directory = org::apache::lucene::store::Directory;

/**
 * Utility class for creating training / test / cross validation indexes from
 * the original index.
 */
class DatasetSplitter : public std::enable_shared_from_this<DatasetSplitter>
{
  GET_CLASS_NAME(DatasetSplitter)

private:
  const double crossValidationRatio;
  const double testRatio;

  /**
   * Create a {@link DatasetSplitter} by giving test and cross validation IDXs
   * sizes
   *
   * @param testRatio            the ratio of the original index to be used for
   * the test IDX as a <code>double</code> between 0.0 and 1.0
   * @param crossValidationRatio the ratio of the original index to be used for
   * the c.v. IDX as a <code>double</code> between 0.0 and 1.0
   */
public:
  DatasetSplitter(double testRatio, double crossValidationRatio);

  /**
   * Split a given index into 3 indexes for training, test and cross validation
   * tasks respectively
   *
   * @param originalIndex        an {@link org.apache.lucene.index.LeafReader}
   * on the source index
   * @param trainingIndex        a {@link Directory} used to write the training
   * index
   * @param testIndex            a {@link Directory} used to write the test
   * index
   * @param crossValidationIndex a {@link Directory} used to write the cross
   * validation index
   * @param analyzer             {@link Analyzer} used to create the new docs
   * @param termVectors          {@code true} if term vectors should be kept
   * @param classFieldName       name of the field used as the label for
   * classification; this must be indexed with sorted doc values
   * @param fieldNames           names of fields that need to be put in the new
   * indexes or <code>null</code> if all should be used
   * @throws IOException if any writing operation fails on any of the indexes
   */
  virtual void split(std::shared_ptr<IndexReader> originalIndex,
                     std::shared_ptr<Directory> trainingIndex,
                     std::shared_ptr<Directory> testIndex,
                     std::shared_ptr<Directory> crossValidationIndex,
                     std::shared_ptr<Analyzer> analyzer, bool termVectors,
                     const std::wstring &classFieldName,
                     std::deque<std::wstring> &fieldNames) ;

private:
  std::shared_ptr<Document>
  createNewDoc(std::shared_ptr<IndexReader> originalIndex,
               std::shared_ptr<FieldType> ft,
               std::shared_ptr<ScoreDoc> scoreDoc,
               std::deque<std::wstring> &fieldNames) ;
};

} // namespace org::apache::lucene::classification::utils
