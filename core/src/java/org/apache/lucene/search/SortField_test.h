#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/FieldComparatorSource.h"

#include  "core/src/java/org/apache/lucene/util/BytesRef.h"
namespace org::apache::lucene::search
{
template <typename T>
class FieldComparator;
}
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"

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

using BytesRef = org::apache::lucene::util::BytesRef;

/**
 * Stores information about how to sort documents by terms in an individual
 * field.  Fields must be indexed in order to sort by them.
 *
 * <p>Created: Feb 11, 2004 1:25:29 PM
 *
 * @since   lucene 1.4
 * @see Sort
 */
class SortField : public std::enable_shared_from_this<SortField>
{
  GET_CLASS_NAME(SortField)

  /**
   * Specifies the type of the terms to be sorted, or special types such as
   * CUSTOM
   */
public:
  enum class Type {
    /** Sort by document score (relevance).  Sort values are Float and higher
     * values are at the front. */
    SCORE,

    /** Sort by document number (index order).  Sort values are Integer and
     * lower values are at the front. */
    DOC,

    /** Sort using term values as Strings.  Sort values are std::wstring and lower
     * values are at the front. */
    STRING,

    /** Sort using term values as encoded Integers.  Sort values are Integer and
     * lower values are at the front. */
    INT,

    /** Sort using term values as encoded Floats.  Sort values are Float and
     * lower values are at the front. */
    FLOAT,

    /** Sort using term values as encoded Longs.  Sort values are Long and
     * lower values are at the front. */
    LONG,

    /** Sort using term values as encoded Doubles.  Sort values are Double and
     * lower values are at the front. */
    DOUBLE,

    /** Sort using a custom Comparator.  Sort values are any Comparable and
     * sorting is done according to natural order. */
    CUSTOM,

    /** Sort using term values as Strings, but comparing by
     * value (using std::wstring.compareTo) for all comparisons.
     * This is typically slower than {@link #STRING}, which
     * uses ordinals to do the sorting. */
    STRING_VAL,

    /** Force rewriting of SortField using {@link
     * SortField#rewrite(IndexSearcher)} before it can be used for sorting */
    REWRITEABLE
  };

  /** Represents sorting by document score (relevance). */
public:
  static const std::shared_ptr<SortField> FIELD_SCORE;

  /** Represents sorting by document number (index order). */
  static const std::shared_ptr<SortField> FIELD_DOC;

private:
  std::wstring field;
  Type type = static_cast<Type>(0); // defaults to determining type dynamically
public:
  bool reverse = false; // defaults to natural order

  // Used for CUSTOM sort
private:
  std::shared_ptr<FieldComparatorSource> comparatorSource;

  // Used for 'sortMissingFirst/Last'
protected:
  std::any missingValue = nullptr;

  /** Creates a sort by terms in the given field with the type of term
   * values explicitly given.
   * @param field  Name of field to sort by.  Can be <code>null</code> if
   *               <code>type</code> is SCORE or DOC.
   * @param type   Type of values in the terms.
   */
public:
  SortField(const std::wstring &field, Type type);

  /** Creates a sort, possibly in reverse, by terms in the given field with the
   * type of term values explicitly given.
   * @param field  Name of field to sort by.  Can be <code>null</code> if
   *               <code>type</code> is SCORE or DOC.
   * @param type   Type of values in the terms.
   * @param reverse True if natural order should be reversed.
   */
  SortField(const std::wstring &field, Type type, bool reverse);

  /** Pass this to {@link #setMissingValue} to have missing
   *  string values sort first. */
  static const std::any STRING_FIRST;

private:
  class ObjectAnonymousInnerClass : public std::any
  {
    GET_CLASS_NAME(ObjectAnonymousInnerClass)
  public:
    ObjectAnonymousInnerClass();

    virtual std::wstring toString();

  protected:
    std::shared_ptr<ObjectAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ObjectAnonymousInnerClass>(
          Object::shared_from_this());
    }
  };

  /** Pass this to {@link #setMissingValue} to have missing
   *  string values sort last. */
public:
  static const std::any STRING_LAST;

private:
  class ObjectAnonymousInnerClass2 : public std::any
  {
    GET_CLASS_NAME(ObjectAnonymousInnerClass2)
  public:
    ObjectAnonymousInnerClass2();

    virtual std::wstring toString();

  protected:
    std::shared_ptr<ObjectAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<ObjectAnonymousInnerClass2>(
          Object::shared_from_this());
    }
  };

  /** Return the value to use for documents that don't have a value.
   *  A value of {@code null} indicates that default should be used. */
public:
  virtual std::any getMissingValue();

  /** Set the value to use for documents that don't have a value. */
  virtual void setMissingValue(std::any missingValue);

  /** Creates a sort with a custom comparison function.
   * @param field Name of field to sort by; cannot be <code>null</code>.
   * @param comparator Returns a comparator for sorting hits.
   */
  SortField(const std::wstring &field,
            std::shared_ptr<FieldComparatorSource> comparator);

  /** Creates a sort, possibly in reverse, with a custom comparison function.
   * @param field Name of field to sort by; cannot be <code>null</code>.
   * @param comparator Returns a comparator for sorting hits.
   * @param reverse True if natural order should be reversed.
   */
  SortField(const std::wstring &field,
            std::shared_ptr<FieldComparatorSource> comparator, bool reverse);

  // Sets field & type, and ensures field is not NULL unless
  // type is SCORE or DOC
private:
  void initFieldType(const std::wstring &field, Type type);

  /** Returns the name of the field.  Could return <code>null</code>
   * if the sort is by SCORE or DOC.
   * @return Name of field, possibly <code>null</code>.
   */
public:
  virtual std::wstring getField();

  /** Returns the type of contents in the field.
   * @return One of the constants SCORE, DOC, STRING, INT or FLOAT.
   */
  virtual Type getType();

  /** Returns whether the sort should be reversed.
   * @return  True if natural order should be reversed.
   */
  virtual bool getReverse();

  /** Returns the {@link FieldComparatorSource} used for
   * custom sorting
   */
  virtual std::shared_ptr<FieldComparatorSource> getComparatorSource();

  virtual std::wstring toString();

  /** Returns true if <code>o</code> is equal to this.  If a
   *  {@link FieldComparatorSource} was provided, it must properly
   *  implement equals (unless a singleton is always used). */
  virtual bool equals(std::any o);

  /** Returns a hash code for this {@link SortField} instance.  If a
   *  {@link FieldComparatorSource} was provided, it must properly
   *  implement hashCode (unless a singleton is always used). */
  virtual int hashCode();

private:
  std::shared_ptr<Comparator<std::shared_ptr<BytesRef>>> bytesComparator =
      Comparator::naturalOrder();

public:
  virtual void
  setBytesComparator(std::shared_ptr<Comparator<std::shared_ptr<BytesRef>>> b);

  virtual std::shared_ptr<Comparator<std::shared_ptr<BytesRef>>>
  getBytesComparator();

  /** Returns the {@link FieldComparator} to use for
   * sorting.
   *
   * @lucene.experimental
   *
   * @param numHits number of top hits the queue will store
   * @param sortPos position of this SortField within {@link
   *   Sort}.  The comparator is primary if sortPos==0,
   *   secondary if sortPos==1, etc.  Some comparators can
   *   optimize themselves when they are the primary sort.
   * @return {@link FieldComparator} to use when sorting
   */
  // C++ TODO: Java wildcard generics are not converted to C++:
  // ORIGINAL LINE: public FieldComparator<?> getComparator(final int numHits,
  // final int sortPos)
  virtual std::shared_ptr < FieldComparator <
      ? >> getComparator(int const numHits, int const sortPos);

  /**
   * Rewrites this SortField, returning a new SortField if a change is made.
   * Subclasses should override this define their rewriting behavior when this
   * SortField is of type {@link SortField.Type#REWRITEABLE}
GET_CLASS_NAME(es)
   *
   * @param searcher IndexSearcher to use during rewriting
   * @return New rewritten SortField, or {@code this} if nothing has changed.
   * @throws IOException Can be thrown by the rewriting
   * @lucene.experimental
   */
  virtual std::shared_ptr<SortField>
  rewrite(std::shared_ptr<IndexSearcher> searcher) ;

  /** Whether the relevance score is needed to sort documents. */
  virtual bool needsScores();
};

} // #include  "core/src/java/org/apache/lucene/search/
