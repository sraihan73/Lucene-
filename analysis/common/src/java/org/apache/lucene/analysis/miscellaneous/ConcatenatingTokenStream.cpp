using namespace std;

#include "ConcatenatingTokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/OffsetAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/AttributeSource.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/IOUtils.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using Attribute = org::apache::lucene::util::Attribute;
using AttributeSource = org::apache::lucene::util::AttributeSource;
using IOUtils = org::apache::lucene::util::IOUtils;

ConcatenatingTokenStream::ConcatenatingTokenStream(deque<TokenStream> &sources)
    : org::apache::lucene::analysis::TokenStream(combineSources(sources)),
      sources(sources),
      sourceOffsets(
          std::deque<std::shared_ptr<OffsetAttribute>>(sources.size())),
      offsetAtt(addAttribute(OffsetAttribute::typeid))
{
  for (int i = 0; i < sources.size(); i++) {
    this->sourceOffsets[i] = sources[i]->addAttribute(OffsetAttribute::typeid);
  }
}

shared_ptr<AttributeSource>
ConcatenatingTokenStream::combineSources(deque<TokenStream> &sources)
{
  shared_ptr<AttributeSource> base = sources[0]->cloneAttributes();
  try {
    for (int i = 1; i < sources.size(); i++) {
      Iterator<type_info> it = sources[i]->getAttributeClassesIterator();
      while (it->hasNext()) {
        base->addAttribute(it->next());
        it++;
      }
      // check attributes can be captured
      sources[i]->copyTo(base);
    }
    return base;
  } catch (const invalid_argument &e) {
    // C++ TODO: This exception's constructor requires only one argument:
    // ORIGINAL LINE: throw new IllegalArgumentException("Attempted to
    // concatenate TokenStreams with different attribute types", e);
    throw invalid_argument(L"Attempted to concatenate TokenStreams with "
                           L"different attribute types");
  }
}

bool ConcatenatingTokenStream::incrementToken() 
{
  while (sources[currentSource]->incrementToken() == false) {
    if (currentSource >= sources.size() - 1) {
      return false;
    }
    sources[currentSource]->end();
    shared_ptr<OffsetAttribute> att = sourceOffsets[currentSource];
    if (att != nullptr) {
      offsetIncrement += att->endOffset();
    }
    currentSource++;
  }

  clearAttributes();
  sources[currentSource]->copyTo(shared_from_this());
  offsetAtt->setOffset(offsetAtt->startOffset() + offsetIncrement,
                       offsetAtt->endOffset() + offsetIncrement);

  return true;
}

void ConcatenatingTokenStream::end() 
{
  sources[currentSource]->end();
  TokenStream::end();
}

void ConcatenatingTokenStream::reset() 
{
  for (auto source : sources) {
    source->reset();
  }
  TokenStream::reset();
}

ConcatenatingTokenStream::~ConcatenatingTokenStream()
{
  try {
    IOUtils::close(sources);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    // C++ NOTE: There is no explicit call to the base class destructor in C++:
    //        super.close();
  }
}
} // namespace org::apache::lucene::analysis::miscellaneous