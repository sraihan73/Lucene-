using namespace std;

#include "CommonGramsQueryFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/PositionLengthAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/TypeAttribute.h"
#include "CommonGramsFilter.h"

namespace org::apache::lucene::analysis::commongrams
{
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using PositionLengthAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionLengthAttribute;
using TypeAttribute =
    org::apache::lucene::analysis::tokenattributes::TypeAttribute;
//    import static
//    org.apache.lucene.analysis.commongrams.CommonGramsFilter.GRAM_TYPE;

CommonGramsQueryFilter::CommonGramsQueryFilter(
    shared_ptr<CommonGramsFilter> input)
    : org::apache::lucene::analysis::TokenFilter(input)
{
}

void CommonGramsQueryFilter::reset() 
{
  TokenFilter::reset();
  previous.reset();
  previousType = L"";
  exhausted = false;
}

bool CommonGramsQueryFilter::incrementToken() 
{
  while (!exhausted && input->incrementToken()) {
    shared_ptr<State> current = captureState();

    if (previous != nullptr && !isGramType()) {
      restoreState(previous);
      previous = current;
      previousType = typeAttribute->type();

      if (isGramType()) {
        posIncAttribute->setPositionIncrement(1);
        // We must set this back to 1 (from e.g. 2 or higher) otherwise the
        // token graph is disconnected:
        posLengthAttribute->setPositionLength(1);
      }
      return true;
    }

    previous = current;
  }

  exhausted = true;

  if (previous == nullptr || GRAM_TYPE::equals(previousType)) {
    return false;
  }

  restoreState(previous);
  previous.reset();

  if (isGramType()) {
    posIncAttribute->setPositionIncrement(1);
    // We must set this back to 1 (from e.g. 2 or higher) otherwise the token
    // graph is disconnected:
    posLengthAttribute->setPositionLength(1);
  }
  return true;
}

bool CommonGramsQueryFilter::isGramType()
{
  return GRAM_TYPE::equals(typeAttribute->type());
}
} // namespace org::apache::lucene::analysis::commongrams