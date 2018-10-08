using namespace std;

#include "CharFilter.h"

namespace org::apache::lucene::analysis
{

CharFilter::CharFilter(shared_ptr<Reader> input)
    : java::io::Reader(input), input(input)
{
}

CharFilter::~CharFilter() { input->close(); }

int CharFilter::correctOffset(int currentOff)
{
  constexpr int corrected = correct(currentOff);
  return (std::dynamic_pointer_cast<CharFilter>(input) != nullptr)
             ? (std::static_pointer_cast<CharFilter>(input))
                   ->correctOffset(corrected)
             : corrected;
}
} // namespace org::apache::lucene::analysis