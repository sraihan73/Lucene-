using namespace std;

#include "SortField.h"

namespace org::apache::lucene::search
{
using BytesRef = org::apache::lucene::util::BytesRef;
const shared_ptr<SortField> SortField::FIELD_SCORE =
    make_shared<SortField>(nullptr, Type::SCORE);
const shared_ptr<SortField> SortField::FIELD_DOC =
    make_shared<SortField>(nullptr, Type::DOC);

SortField::SortField(const wstring &field, Type type)
{
  initFieldType(field, type);
}

SortField::SortField(const wstring &field, Type type, bool reverse)
{
  initFieldType(field, type);
  this->reverse = reverse;
}

const any SortField::STRING_FIRST = make_shared<ObjectAnonymousInnerClass>();

SortField::ObjectAnonymousInnerClass::ObjectAnonymousInnerClass() {}

wstring SortField::ObjectAnonymousInnerClass::toString()
{
  return L"SortField.STRING_FIRST";
}

const any SortField::STRING_LAST = make_shared<ObjectAnonymousInnerClass2>();

SortField::ObjectAnonymousInnerClass2::ObjectAnonymousInnerClass2() {}

wstring SortField::ObjectAnonymousInnerClass2::toString()
{
  return L"SortField.STRING_LAST";
}

any SortField::getMissingValue() { return missingValue; }

void SortField::setMissingValue(any missingValue)
{
  if (type == Type::STRING || type == Type::STRING_VAL) {
    if (missingValue != STRING_FIRST && missingValue != STRING_LAST) {
      throw invalid_argument(L"For STRING type, missing value must be either "
                             L"STRING_FIRST or STRING_LAST");
    }
  } else if (type == Type::INT) {
    if (missingValue != nullptr && missingValue.type() != Integer::typeid) {
      throw invalid_argument(L"Missing values for Type.INT can only be of type "
                             L"java.lang.Integer, but got " +
                             missingValue.type());
    }
  } else if (type == Type::LONG) {
    if (missingValue != nullptr && missingValue.type() != Long::typeid) {
      throw invalid_argument(L"Missing values for Type.LONG can only be of "
                             L"type java.lang.Long, but got " +
                             missingValue.type());
    }
  } else if (type == Type::FLOAT) {
    if (missingValue != nullptr && missingValue.type() != Float::typeid) {
      throw invalid_argument(L"Missing values for Type.FLOAT can only be of "
                             L"type java.lang.Float, but got " +
                             missingValue.type());
    }
  } else if (type == Type::DOUBLE) {
    if (missingValue != nullptr && missingValue.type() != Double::typeid) {
      throw invalid_argument(L"Missing values for Type.DOUBLE can only be of "
                             L"type java.lang.Double, but got " +
                             missingValue.type());
    }
  } else {
    throw invalid_argument(
        L"Missing value only works for numeric or STRING types");
  }
  this->missingValue = missingValue;
}

SortField::SortField(const wstring &field,
                     shared_ptr<FieldComparatorSource> comparator)
{
  initFieldType(field, Type::CUSTOM);
  this->comparatorSource = comparator;
}

SortField::SortField(const wstring &field,
                     shared_ptr<FieldComparatorSource> comparator, bool reverse)
{
  initFieldType(field, Type::CUSTOM);
  this->reverse = reverse;
  this->comparatorSource = comparator;
}

void SortField::initFieldType(const wstring &field, Type type)
{
  this->type = type;
  if (field == L"") {
    if (type != Type::SCORE && type != Type::DOC) {
      throw invalid_argument(
          L"field can only be null when type is SCORE or DOC");
    }
  } else {
    this->field = field;
  }
}

wstring SortField::getField() { return field; }

SortField::Type SortField::getType() { return type; }

bool SortField::getReverse() { return reverse; }

shared_ptr<FieldComparatorSource> SortField::getComparatorSource()
{
  return comparatorSource;
}

wstring SortField::toString()
{
  shared_ptr<StringBuilder> buffer = make_shared<StringBuilder>();
  switch (type) {
  case org::apache::lucene::search::SortField::Type::SCORE:
    buffer->append(L"<score>");
    break;

  case org::apache::lucene::search::SortField::Type::DOC:
    buffer->append(L"<doc>");
    break;

  case org::apache::lucene::search::SortField::Type::STRING:
    buffer->append(wstring(L"<string") + L": \"")
        ->append(field)
        ->append(L"\">");
    break;

  case org::apache::lucene::search::SortField::Type::STRING_VAL:
    buffer->append(wstring(L"<string_val") + L": \"")
        ->append(field)
        ->append(L"\">");
    break;

  case org::apache::lucene::search::SortField::Type::INT:
    buffer->append(wstring(L"<int") + L": \"")->append(field)->append(L"\">");
    break;

  case org::apache::lucene::search::SortField::Type::LONG:
    buffer->append(L"<long: \"")->append(field)->append(L"\">");
    break;

  case org::apache::lucene::search::SortField::Type::FLOAT:
    buffer->append(wstring(L"<float") + L": \"")->append(field)->append(L"\">");
    break;

  case org::apache::lucene::search::SortField::Type::DOUBLE:
    buffer->append(wstring(L"<double") + L": \"")
        ->append(field)
        ->append(L"\">");
    break;

  case org::apache::lucene::search::SortField::Type::CUSTOM:
    buffer->append(L"<custom:\"")
        ->append(field)
        ->append(L"\": ")
        ->append(comparatorSource)
        ->append(L'>');
    break;

  case org::apache::lucene::search::SortField::Type::REWRITEABLE:
    buffer->append(L"<rewriteable: \"")->append(field)->append(L"\">");
    break;

  default:
    buffer->append(L"<???: \"")->append(field)->append(L"\">");
    break;
  }

  if (reverse) {
    buffer->append(L'!');
  }
  if (missingValue != nullptr) {
    buffer->append(L" missingValue=");
    buffer->append(missingValue);
  }

  return buffer->toString();
}

bool SortField::equals(any o)
{
  if (shared_from_this() == o) {
    return true;
  }
  if (!(std::dynamic_pointer_cast<SortField>(o) != nullptr)) {
    return false;
  }
  shared_ptr<SortField> *const other = any_cast<std::shared_ptr<SortField>>(o);
  return (Objects::equals(other->field, this->field) &&
          other->type == this->type && other->reverse == this->reverse &&
          Objects::equals(this->comparatorSource, other->comparatorSource) &&
          Objects::equals(this->missingValue, other->missingValue));
}

int SortField::hashCode()
{
  return Objects::hash(field, type, reverse, comparatorSource, missingValue);
}

void SortField::setBytesComparator(
    shared_ptr<Comparator<std::shared_ptr<BytesRef>>> b)
{
  bytesComparator = b;
}

shared_ptr<Comparator<std::shared_ptr<BytesRef>>>
SortField::getBytesComparator()
{
  return bytesComparator;
}

// C++ TODO: Java wildcard generics are not converted to C++:
// ORIGINAL LINE: public FieldComparator<?> getComparator(final int numHits,
// final int sortPos)
shared_ptr < FieldComparator <
    ? >> SortField::getComparator(int const numHits, int const sortPos)
{

  switch (type) {
  case org::apache::lucene::search::SortField::Type::SCORE:
    return make_shared<FieldComparator::RelevanceComparator>(numHits);

  case org::apache::lucene::search::SortField::Type::DOC:
    return make_shared<FieldComparator::DocComparator>(numHits);

  case org::apache::lucene::search::SortField::Type::INT:
    return make_shared<FieldComparator::IntComparator>(
        numHits, field, any_cast<optional<int>>(missingValue));

  case org::apache::lucene::search::SortField::Type::FLOAT:
    return make_shared<FieldComparator::FloatComparator>(
        numHits, field, any_cast<optional<float>>(missingValue));

  case org::apache::lucene::search::SortField::Type::LONG:
    return make_shared<FieldComparator::LongComparator>(
        numHits, field, any_cast<optional<int64_t>>(missingValue));

  case org::apache::lucene::search::SortField::Type::DOUBLE:
    return make_shared<FieldComparator::DoubleComparator>(
        numHits, field, any_cast<optional<double>>(missingValue));

  case org::apache::lucene::search::SortField::Type::CUSTOM:
    assert(comparatorSource != nullptr);
    return comparatorSource->newComparator(field, numHits, sortPos, reverse);

  case org::apache::lucene::search::SortField::Type::STRING:
    return make_shared<FieldComparator::TermOrdValComparator>(
        numHits, field, missingValue == STRING_LAST);

  case org::apache::lucene::search::SortField::Type::STRING_VAL:
    return make_shared<FieldComparator::TermValComparator>(
        numHits, field, missingValue == STRING_LAST);

  case org::apache::lucene::search::SortField::Type::REWRITEABLE:
    throw make_shared<IllegalStateException>(
        L"SortField needs to be rewritten through Sort.rewrite(..) and "
        L"SortField.rewrite(..)");

  default:
    throw make_shared<IllegalStateException>(L"Illegal sort type: " + type);
  }
}

shared_ptr<SortField>
SortField::rewrite(shared_ptr<IndexSearcher> searcher) 
{
  return shared_from_this();
}

bool SortField::needsScores() { return type == Type::SCORE; }
} // namespace org::apache::lucene::search