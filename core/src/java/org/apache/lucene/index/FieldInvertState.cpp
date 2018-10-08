using namespace std;

#include "FieldInvertState.h"

namespace org::apache::lucene::index
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PayloadAttribute =
    org::apache::lucene::analysis::tokenattributes::PayloadAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using TermFrequencyAttribute =
    org::apache::lucene::analysis::tokenattributes::TermFrequencyAttribute;
using TermToBytesRefAttribute =
    org::apache::lucene::analysis::tokenattributes::TermToBytesRefAttribute;
using AttributeSource = org::apache::lucene::util::AttributeSource;

FieldInvertState::FieldInvertState(int indexCreatedVersionMajor,
                                   const wstring &name)
    : indexCreatedVersionMajor(indexCreatedVersionMajor), name(name)
{
}

FieldInvertState::FieldInvertState(int indexCreatedVersionMajor,
                                   const wstring &name, int position,
                                   int length, int numOverlap, int offset)
    : FieldInvertState(indexCreatedVersionMajor, name)
{
  this->position = position;
  this->length = length;
  this->numOverlap = numOverlap;
  this->offset = offset;
}

void FieldInvertState::reset()
{
  position = -1;
  length = 0;
  numOverlap = 0;
  offset = 0;
  maxTermFrequency = 0;
  uniqueTermCount = 0;
  lastStartOffset = 0;
  lastPosition = 0;
}

void FieldInvertState::setAttributeSource(
    shared_ptr<AttributeSource> attributeSource)
{
  if (this->attributeSource != attributeSource) {
    this->attributeSource = attributeSource;
    termAttribute =
        attributeSource->getAttribute(TermToBytesRefAttribute::typeid);
    termFreqAttribute =
        attributeSource->addAttribute(TermFrequencyAttribute::typeid);
    posIncrAttribute =
        attributeSource->addAttribute(PositionIncrementAttribute::typeid);
    offsetAttribute = attributeSource->addAttribute(OffsetAttribute::typeid);
    payloadAttribute = attributeSource->getAttribute(PayloadAttribute::typeid);
  }
}

int FieldInvertState::getPosition() { return position; }

int FieldInvertState::getLength() { return length; }

void FieldInvertState::setLength(int length) { this->length = length; }

int FieldInvertState::getNumOverlap() { return numOverlap; }

void FieldInvertState::setNumOverlap(int numOverlap)
{
  this->numOverlap = numOverlap;
}

int FieldInvertState::getOffset() { return offset; }

int FieldInvertState::getMaxTermFrequency() { return maxTermFrequency; }

int FieldInvertState::getUniqueTermCount() { return uniqueTermCount; }

shared_ptr<AttributeSource> FieldInvertState::getAttributeSource()
{
  return attributeSource;
}

wstring FieldInvertState::getName() { return name; }

int FieldInvertState::getIndexCreatedVersionMajor()
{
  return indexCreatedVersionMajor;
}
} // namespace org::apache::lucene::index