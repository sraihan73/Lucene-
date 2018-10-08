using namespace std;

#include "PartOfSpeechAttribute.h"
#include "../Token.h"
#include "../dict/Dictionary.h"

namespace org::apache::lucene::analysis::ko::tokenattributes
{
using Tag = org::apache::lucene::analysis::ko::POS::Tag;
using Type = org::apache::lucene::analysis::ko::POS::Type;
using Token = org::apache::lucene::analysis::ko::Token;
using Morpheme = org::apache::lucene::analysis::ko::dict::Dictionary::Morpheme;
using Attribute = org::apache::lucene::util::Attribute;
} // namespace org::apache::lucene::analysis::ko::tokenattributes