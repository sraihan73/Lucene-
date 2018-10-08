using namespace std;

#include "Hyphenation.h"

namespace org::apache::lucene::analysis::compound::hyphenation
{

Hyphenation::Hyphenation(std::deque<int> &points) { hyphenPoints = points; }

int Hyphenation::length() { return hyphenPoints.size(); }

std::deque<int> Hyphenation::getHyphenationPoints() { return hyphenPoints; }
} // namespace org::apache::lucene::analysis::compound::hyphenation