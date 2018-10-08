using namespace std;

#include "LongRangeCounter.h"

namespace org::apache::lucene::facet::range
{

LongRangeCounter::LongRangeCounter(
    std::deque<std::shared_ptr<LongRange>> &ranges)
    : root(split(0, elementaryIntervals->size(), elementaryIntervals)),
      boundaries(std::deque<int64_t>(elementaryIntervals->size())),
      leafCounts(std::deque<int>(boundaries.size()))
{
  // Maps all range inclusive endpoints to int flags; 1
  // = start of interval, 2 = end of interval.  We need to
  // track the start vs end case separately because if a
  // given point is both, then it must be its own
  // elementary interval:
  unordered_map<int64_t, int> endsMap = unordered_map<int64_t, int>();

  endsMap.emplace(numeric_limits<int64_t>::min(), 1);
  endsMap.emplace(numeric_limits<int64_t>::max(), 2);

  for (auto range : ranges) {
    optional<int> cur = endsMap[range->min];
    if (!cur) {
      endsMap.emplace(range->min, 1);
    } else {
      endsMap.emplace(range->min, cur.value() | 1);
    }
    cur = endsMap[range->max];
    if (!cur) {
      endsMap.emplace(range->max, 2);
    } else {
      endsMap.emplace(range->max, cur.value() | 2);
    }
  }

  deque<int64_t> endsList = deque<int64_t>(endsMap.keySet());
  sort(endsList.begin(), endsList.end());

  // Build elementaryIntervals (a 1D Venn diagram):
  deque<std::shared_ptr<InclusiveRange>> elementaryIntervals =
      deque<std::shared_ptr<InclusiveRange>>();
  int upto0 = 1;
  int64_t v = endsList[0];
  int64_t prev;
  if (endsMap[v] == 3) {
    elementaryIntervals.push_back(make_shared<InclusiveRange>(v, v));
    prev = v + 1;
  } else {
    prev = v;
  }

  while (upto0 < endsList.size()) {
    v = endsList[upto0];
    int flags = endsMap[v];
    // System.out.println("  v=" + v + " flags=" + flags);
    if (flags == 3) {
      // This point is both an end and a start; we need to
      // separate it:
      if (v > prev) {
        elementaryIntervals.push_back(make_shared<InclusiveRange>(prev, v - 1));
      }
      elementaryIntervals.push_back(make_shared<InclusiveRange>(v, v));
      prev = v + 1;
    } else if (flags == 1) {
      // This point is only the start of an interval;
      // attach it to next interval:
      if (v > prev) {
        elementaryIntervals.push_back(make_shared<InclusiveRange>(prev, v - 1));
      }
      prev = v;
    } else {
      assert(flags == 2);
      // This point is only the end of an interval; attach
      // it to last interval:
      elementaryIntervals.push_back(make_shared<InclusiveRange>(prev, v));
      prev = v + 1;
    }
    // System.out.println("    ints=" + elementaryIntervals);
    upto0++;
  }

  // Build binary tree on top of intervals:

  // Set outputs, so we know which range to output for
  // each node in the tree:
  for (int i = 0; i < ranges.size(); i++) {
    root->addOutputs(i, ranges[i]);
  }

  // Set boundaries (ends of each elementary interval):
  for (int i = 0; i < boundaries.size(); i++) {
    boundaries[i] = elementaryIntervals[i]->end;
  }

  // System.out.println("ranges: " + Arrays.toString(ranges));
  // System.out.println("intervals: " + elementaryIntervals);
  // System.out.println("boundaries: " + Arrays.toString(boundaries));
  // System.out.println("root:\n" + root);
}

void LongRangeCounter::add(int64_t v)
{
  // System.out.println("add v=" + v);

  // NOTE: this works too, but it's ~6% slower on a simple
  // test with a high-freq TermQuery w/ range faceting on
  // wikimediumall:
  /*
  int index = Arrays.binarySearch(boundaries, v);
  if (index < 0) {
    index = -index-1;
  }
  leafCounts[index]++;
  */

  // Binary search to find matched elementary range; we
  // are guaranteed to find a match because the last
  // boundary is Long.MAX_VALUE:

  int lo = 0;
  int hi = boundaries.size() - 1;
  while (true) {
    int mid = static_cast<int>(static_cast<unsigned int>((lo + hi)) >> 1);
    // System.out.println("  cycle lo=" + lo + " hi=" + hi + " mid=" + mid + "
    // boundary=" + boundaries[mid] + " to " + boundaries[mid+1]);
    if (v <= boundaries[mid]) {
      if (mid == 0) {
        leafCounts[0]++;
        return;
      } else {
        hi = mid - 1;
      }
    } else if (v > boundaries[mid + 1]) {
      lo = mid + 1;
    } else {
      leafCounts[mid + 1]++;
      // System.out.println("  incr @ " + (mid+1) + "; now " +
      // leafCounts[mid+1]);
      return;
    }
  }
}

int LongRangeCounter::fillCounts(std::deque<int> &counts)
{
  // System.out.println("  rollup");
  missingCount = 0;
  leafUpto = 0;
  rollup(root, counts, false);
  return missingCount;
}

int LongRangeCounter::rollup(shared_ptr<LongRangeNode> node,
                             std::deque<int> &counts, bool sawOutputs)
{
  int count;
  sawOutputs |= node->outputs.size() > 0;
  if (node->left != nullptr) {
    count = rollup(node->left, counts, sawOutputs);
    count += rollup(node->right, counts, sawOutputs);
  } else {
    // Leaf:
    count = leafCounts[leafUpto];
    leafUpto++;
    if (!sawOutputs) {
      // This is a missing count (no output ranges were
      // seen "above" us):
      missingCount += count;
    }
  }
  if (node->outputs.size() > 0) {
    for (auto rangeIndex : node->outputs) {
      counts[rangeIndex] += count;
    }
  }
  // System.out.println("  rollup node=" + node.start + " to " + node.end + ":
  // count=" + count);
  return count;
}

shared_ptr<LongRangeNode> LongRangeCounter::split(
    int start, int end,
    deque<std::shared_ptr<InclusiveRange>> &elementaryIntervals)
{
  if (start == end - 1) {
    // leaf
    shared_ptr<InclusiveRange> range = elementaryIntervals[start];
    return make_shared<LongRangeNode>(range->start, range->end, nullptr,
                                      nullptr, start);
  } else {
    int mid = static_cast<int>(static_cast<unsigned int>((start + end)) >> 1);
    shared_ptr<LongRangeNode> left = split(start, mid, elementaryIntervals);
    shared_ptr<LongRangeNode> right = split(mid, end, elementaryIntervals);
    return make_shared<LongRangeNode>(left->start, right->end, left, right, -1);
  }
}

LongRangeCounter::InclusiveRange::InclusiveRange(int64_t start, int64_t end)
    : start(start), end(end)
{
  assert(end >= start);
}

wstring LongRangeCounter::InclusiveRange::toString()
{
  return to_wstring(start) + L" to " + to_wstring(end);
}

LongRangeCounter::LongRangeNode::LongRangeNode(int64_t start, int64_t end,
                                               shared_ptr<LongRangeNode> left,
                                               shared_ptr<LongRangeNode> right,
                                               int leafIndex)
    : left(left), right(right), start(start), end(end), leafIndex(leafIndex)
{
}

wstring LongRangeCounter::LongRangeNode::toString()
{
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  toString(sb, 0);
  return sb->toString();
}

void LongRangeCounter::LongRangeNode::indent(shared_ptr<StringBuilder> sb,
                                             int depth)
{
  for (int i = 0; i < depth; i++) {
    sb->append(L"  ");
  }
}

void LongRangeCounter::LongRangeNode::addOutputs(int index,
                                                 shared_ptr<LongRange> range)
{
  if (start >= range->min && end <= range->max) {
    // Our range is fully included in the incoming
    // range; add to our output deque:
    if (outputs.empty()) {
      outputs = deque<>();
    }
    outputs.push_back(index);
  } else if (left != nullptr) {
    assert(right != nullptr);
    // Recurse:
    left->addOutputs(index, range);
    right->addOutputs(index, range);
  }
}

void LongRangeCounter::LongRangeNode::toString(shared_ptr<StringBuilder> sb,
                                               int depth)
{
  indent(sb, depth);
  if (left == nullptr) {
    assert(right == nullptr);
    sb->append(L"leaf: " + to_wstring(start) + L" to " + to_wstring(end));
  } else {
    sb->append(L"node: " + to_wstring(start) + L" to " + to_wstring(end));
  }
  if (outputs.size() > 0) {
    sb->append(L" outputs=");
    sb->append(outputs);
  }
  sb->append(L'\n');

  if (left != nullptr) {
    assert(right != nullptr);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    left->toString(sb, depth + 1);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    right->toString(sb, depth + 1);
  }
}
} // namespace org::apache::lucene::facet::range