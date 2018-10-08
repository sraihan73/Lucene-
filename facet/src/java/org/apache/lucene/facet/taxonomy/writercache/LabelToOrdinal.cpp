using namespace std;

#include "LabelToOrdinal.h"

namespace org::apache::lucene::facet::taxonomy::writercache
{
using FacetLabel = org::apache::lucene::facet::taxonomy::FacetLabel;

LabelToOrdinal::LabelToOrdinal() {}

int LabelToOrdinal::getMaxOrdinal() { return this->counter; }

int LabelToOrdinal::getNextOrdinal() { return this->counter++; }
} // namespace org::apache::lucene::facet::taxonomy::writercache