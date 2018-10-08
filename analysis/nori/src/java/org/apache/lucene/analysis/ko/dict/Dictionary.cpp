using namespace std;

#include "Dictionary.h"

namespace org::apache::lucene::analysis::ko::dict
{
using Tag = org::apache::lucene::analysis::ko::POS::Tag;
using Type = org::apache::lucene::analysis::ko::POS::Type;

Morpheme::Morpheme(Tag posTag, const wstring &surfaceForm)
    : posTag(posTag), surfaceForm(surfaceForm)
{
}
} // namespace org::apache::lucene::analysis::ko::dict