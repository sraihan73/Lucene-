using namespace std;

#include "PrefixCodedTerms.h"

namespace org::apache::lucene::index
{
using IndexInput = org::apache::lucene::store::IndexInput;
using RAMFile = org::apache::lucene::store::RAMFile;
using RAMInputStream = org::apache::lucene::store::RAMInputStream;
using RAMOutputStream = org::apache::lucene::store::RAMOutputStream;
using Accountable = org::apache::lucene::util::Accountable;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using StringHelper = org::apache::lucene::util::StringHelper;

PrefixCodedTerms::PrefixCodedTerms(shared_ptr<RAMFile> buffer, int64_t size)
    : buffer(Objects::requireNonNull(buffer)), size(size)
{
}

int64_t PrefixCodedTerms::ramBytesUsed()
{
  return buffer->ramBytesUsed() + 2 * Long::BYTES;
}

void PrefixCodedTerms::setDelGen(int64_t delGen) { this->delGen = delGen; }

PrefixCodedTerms::Builder::Builder() {}

void PrefixCodedTerms::Builder::add(shared_ptr<Term> term)
{
  add(term->field(), term->bytes());
}

void PrefixCodedTerms::Builder::add(const wstring &field,
                                    shared_ptr<BytesRef> bytes)
{
  assert((lastTerm->equals(make_shared<Term>(L"")) ||
          (make_shared<Term>(field, bytes))->compareTo(lastTerm) > 0));

  try {
    constexpr int prefix;
    if (size > 0 && field == lastTerm->field_) {
      // same field as the last term
      prefix = StringHelper::bytesDifference(lastTerm->bytes_, bytes);
      output->writeVInt(prefix << 1);
    } else {
      // field change
      prefix = 0;
      output->writeVInt(1);
      output->writeString(field);
    }

    int suffix = bytes->length - prefix;
    output->writeVInt(suffix);
    output->writeBytes(bytes->bytes, bytes->offset + prefix, suffix);
    lastTermBytes->copyBytes(bytes);
    lastTerm->bytes_ = lastTermBytes->get();
    lastTerm->field_ = field;
    size += 1;
  } catch (const IOException &e) {
    throw runtime_error(e);
  }
}

shared_ptr<PrefixCodedTerms> PrefixCodedTerms::Builder::finish()
{
  try {
    delete output;
    return make_shared<PrefixCodedTerms>(buffer, size);
  } catch (const IOException &e) {
    throw runtime_error(e);
  }
}

PrefixCodedTerms::TermIterator::TermIterator(int64_t delGen,
                                             shared_ptr<RAMFile> buffer)
    : end(input->length()), delGen(delGen)
{
  try {
    input = make_shared<RAMInputStream>(L"PrefixCodedTermsIterator", buffer);
  } catch (const IOException &e) {
    throw runtime_error(e);
  }
}

shared_ptr<BytesRef> PrefixCodedTerms::TermIterator::next()
{
  if (input->getFilePointer() < end) {
    try {
      int code = input->readVInt();
      bool newField = (code & 1) != 0;
      if (newField) {
        field_ = input->readString();
      }
      int prefix = static_cast<int>(static_cast<unsigned int>(code) >> 1);
      int suffix = input->readVInt();
      readTermBytes(prefix, suffix);
      return bytes;
    } catch (const IOException &e) {
      throw runtime_error(e);
    }
  } else {
    field_ = L"";
    return nullptr;
  }
}

void PrefixCodedTerms::TermIterator::readTermBytes(
    int prefix, int suffix) 
{
  builder->grow(prefix + suffix);
  input->readBytes(builder->bytes(), prefix, suffix);
  builder->setLength(prefix + suffix);
}

wstring PrefixCodedTerms::TermIterator::field() { return field_; }

int64_t PrefixCodedTerms::TermIterator::delGen() { return delGen_; }

shared_ptr<TermIterator> PrefixCodedTerms::iterator()
{
  return make_shared<TermIterator>(delGen, buffer);
}

int64_t PrefixCodedTerms::size() { return size_; }

int PrefixCodedTerms::hashCode()
{
  int h = buffer->hashCode();
  h = 31 * h +
      static_cast<int>(delGen ^
                       (static_cast<int64_t>(
                           static_cast<uint64_t>(delGen) >> 32)));
  return h;
}

bool PrefixCodedTerms::equals(any obj)
{
  if (shared_from_this() == obj) {
    return true;
  }
  if (obj == nullptr) {
    return false;
  }
  if (getClass() != obj.type()) {
    return false;
  }
  shared_ptr<PrefixCodedTerms> other =
      any_cast<std::shared_ptr<PrefixCodedTerms>>(obj);
  return buffer->equals(other->buffer) && delGen == other->delGen;
}
} // namespace org::apache::lucene::index