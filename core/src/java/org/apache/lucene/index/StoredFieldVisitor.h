#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
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
namespace org::apache::lucene::index
{

/**
 * Expert: provides a low-level means of accessing the stored field
 * values in an index.  See {@link IndexReader#document(int,
 * StoredFieldVisitor)}.
 *
 * <p><b>NOTE</b>: a {@code StoredFieldVisitor} implementation
 * should not try to load or visit other stored documents in
 * the same reader because the implementation of stored
 * fields for most codecs is not reeentrant and you will see
 * strange exceptions as a result.
 *
 * <p>See {@link DocumentStoredFieldVisitor}, which is a
 * <code>StoredFieldVisitor</code> that builds the
 * {@link Document} containing all stored fields.  This is
 * used by {@link IndexReader#document(int)}.
 *
 * @lucene.experimental */

class StoredFieldVisitor
    : public std::enable_shared_from_this<StoredFieldVisitor>
{
  GET_CLASS_NAME(StoredFieldVisitor)

  /** Sole constructor. (For invocation by subclass
   * constructors, typically implicit.) */
protected:
  StoredFieldVisitor();

  /** Process a binary field.
   * @param value newly allocated byte array with the binary contents.
   */
public:
  virtual void binaryField(std::shared_ptr<FieldInfo> fieldInfo,
                           std::deque<char> &value) ;

  /** Process a string field; the provided byte[] value is a UTF-8 encoded
   * string value. */
  virtual void stringField(std::shared_ptr<FieldInfo> fieldInfo,
                           std::deque<char> &value) ;

  /** Process a int numeric field. */
  virtual void intField(std::shared_ptr<FieldInfo> fieldInfo,
                        int value) ;

  /** Process a long numeric field. */
  virtual void longField(std::shared_ptr<FieldInfo> fieldInfo,
                         int64_t value) ;

  /** Process a float numeric field. */
  virtual void floatField(std::shared_ptr<FieldInfo> fieldInfo,
                          float value) ;

  /** Process a double numeric field. */
  virtual void doubleField(std::shared_ptr<FieldInfo> fieldInfo,
                           double value) ;

  /**
   * Hook before processing a field.
   * Before a field is processed, this method is invoked so that
   * subclasses can return a {@link Status} representing whether
   * they need that particular field or not, or to stop processing
   * entirely.
   */
  virtual Status needsField(std::shared_ptr<FieldInfo> fieldInfo) = 0;

  /**
   * Enumeration of possible return values for {@link #needsField}.
   */
public:
  enum class Status {
    GET_CLASS_NAME(Status)
    /** YES: the field should be visited. */
    YES,
    /** NO: don't visit this field, but continue processing fields for this
       document. */
    NO,
    /** STOP: don't visit this field and stop processing any other fields for
       this document. */
    STOP
  };
};

} // #include  "core/src/java/org/apache/lucene/index/
