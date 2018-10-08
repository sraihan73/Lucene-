using namespace std;

#include "PackedTokenAttributeImpl.h"
#include "../../util/AttributeImpl.h"
#include "../../util/AttributeReflector.h"

namespace org::apache::lucene::analysis::tokenattributes
{
using AttributeImpl = org::apache::lucene::util::AttributeImpl;
using AttributeReflector = org::apache::lucene::util::AttributeReflector;

PackedTokenAttributeImpl::PackedTokenAttributeImpl() {}

void PackedTokenAttributeImpl::setPositionIncrement(int positionIncrement)
{
  if (positionIncrement < 0) {
    throw invalid_argument(L"Increment must be zero or greater: " +
                           to_wstring(positionIncrement));
  }
  this->positionIncrement = positionIncrement;
}

int PackedTokenAttributeImpl::getPositionIncrement()
{
  return positionIncrement;
}

void PackedTokenAttributeImpl::setPositionLength(int positionLength)
{
  if (positionLength < 1) {
    throw invalid_argument(L"Position length must be 1 or greater: got " +
                           to_wstring(positionLength));
  }
  this->positionLength = positionLength;
}

int PackedTokenAttributeImpl::getPositionLength() { return positionLength; }

int PackedTokenAttributeImpl::startOffset() { return startOffset_; }

int PackedTokenAttributeImpl::endOffset() { return endOffset_; }

void PackedTokenAttributeImpl::setOffset(int startOffset, int endOffset)
{
  if (startOffset < 0 || endOffset < startOffset) {
    throw invalid_argument(wstring(L"startOffset must be non-negative, and "
                                   L"endOffset must be >= startOffset; got ") +
                           L"startOffset=" + to_wstring(startOffset) +
                           L",endOffset=" + to_wstring(endOffset));
  }
  this->startOffset_ = startOffset;
  this->endOffset_ = endOffset;
}

wstring PackedTokenAttributeImpl::type() { return type_; }

void PackedTokenAttributeImpl::setType(const wstring &type)
{
  this->type_ = type;
}

void PackedTokenAttributeImpl::setTermFrequency(int termFrequency)
{
  if (termFrequency < 1) {
    throw invalid_argument(L"Term frequency must be 1 or greater; got " +
                           to_wstring(termFrequency));
  }
  this->termFrequency = termFrequency;
}

int PackedTokenAttributeImpl::getTermFrequency() { return termFrequency; }

void PackedTokenAttributeImpl::clear()
{
  CharTermAttributeImpl::clear();
  positionIncrement = positionLength = 1;
  termFrequency = 1;
  startOffset_ = endOffset_ = 0;
  type_ = DEFAULT_TYPE;
}

void PackedTokenAttributeImpl::end()
{
  CharTermAttributeImpl::end();
  // super.end already calls this.clear, so we only set values that are
  // different from clear:
  positionIncrement = 0;
}

shared_ptr<PackedTokenAttributeImpl> PackedTokenAttributeImpl::clone()
{
  return std::static_pointer_cast<PackedTokenAttributeImpl>(
      CharTermAttributeImpl::clone());
}

bool PackedTokenAttributeImpl::equals(any obj)
{
  if (obj == shared_from_this()) {
    return true;
  }

  if (std::dynamic_pointer_cast<PackedTokenAttributeImpl>(obj) != nullptr) {
    shared_ptr<PackedTokenAttributeImpl> *const other =
        any_cast<std::shared_ptr<PackedTokenAttributeImpl>>(obj);
    return (startOffset_ == other->startOffset_ &&
            endOffset_ == other->endOffset_ &&
            positionIncrement == other->positionIncrement &&
            positionLength == other->positionLength &&
            (type_ == L"" ? other->type_ == L"" : type_ == other->type_) &&
            termFrequency == other->termFrequency &&
            CharTermAttributeImpl::equals(obj));
  } else {
    return false;
  }
}

int PackedTokenAttributeImpl::hashCode()
{
  int code = CharTermAttributeImpl::hashCode();
  code = code * 31 + startOffset_;
  code = code * 31 + endOffset_;
  code = code * 31 + positionIncrement;
  code = code * 31 + positionLength;
  if (type_ != L"") {
    code = code * 31 + type_.hashCode();
  }
  code = code * 31 + termFrequency;
  return code;
}

void PackedTokenAttributeImpl::copyTo(shared_ptr<AttributeImpl> target)
{
  if (std::dynamic_pointer_cast<PackedTokenAttributeImpl>(target) != nullptr) {
    shared_ptr<PackedTokenAttributeImpl> *const to =
        std::static_pointer_cast<PackedTokenAttributeImpl>(target);
    to->copyBuffer(buffer(), 0, length());
    to->positionIncrement = positionIncrement;
    to->positionLength = positionLength;
    to->startOffset_ = startOffset_;
    to->endOffset_ = endOffset_;
    to->type_ = type_;
    to->termFrequency = termFrequency;
  } else {
    CharTermAttributeImpl::copyTo(target);
    (std::static_pointer_cast<OffsetAttribute>(target))
        ->setOffset(startOffset_, endOffset_);
    (std::static_pointer_cast<PositionIncrementAttribute>(target))
        ->setPositionIncrement(positionIncrement);
    (std::static_pointer_cast<PositionLengthAttribute>(target))
        ->setPositionLength(positionLength);
    (std::static_pointer_cast<TypeAttribute>(target))->setType(type_);
    (std::static_pointer_cast<TermFrequencyAttribute>(target))
        ->setTermFrequency(termFrequency);
  }
}

void PackedTokenAttributeImpl::reflectWith(AttributeReflector reflector)
{
  CharTermAttributeImpl::reflectWith(reflector);
  reflector(OffsetAttribute::typeid, L"startOffset", startOffset_);
  reflector(OffsetAttribute::typeid, L"endOffset", endOffset_);
  reflector(PositionIncrementAttribute::typeid, L"positionIncrement",
            positionIncrement);
  reflector(PositionLengthAttribute::typeid, L"positionLength", positionLength);
  reflector(TypeAttribute::typeid, L"type", type_);
  reflector(TermFrequencyAttribute::typeid, L"termFrequency", termFrequency);
}
} // namespace org::apache::lucene::analysis::tokenattributes