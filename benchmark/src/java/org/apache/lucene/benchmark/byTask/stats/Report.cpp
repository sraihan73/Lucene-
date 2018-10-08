using namespace std;

#include "Report.h"

namespace org::apache::lucene::benchmark::byTask::stats
{

Report::Report(const wstring &text, int size, int reported, int outOf)
{
  this->text = text;
  this->size = size;
  this->reported = reported;
  this->outOf = outOf;
}

int Report::getOutOf() { return outOf; }

int Report::getSize() { return size; }

wstring Report::getText() { return text; }

int Report::getReported() { return reported; }
} // namespace org::apache::lucene::benchmark::byTask::stats