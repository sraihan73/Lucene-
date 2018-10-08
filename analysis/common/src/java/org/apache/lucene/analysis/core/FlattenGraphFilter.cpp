using namespace std;

#include "FlattenGraphFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/OffsetAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/PositionLengthAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/AttributeSource.h"

namespace org::apache::lucene::analysis::core
{
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using SynonymGraphFilter =
    org::apache::lucene::analysis::synonym::SynonymGraphFilter;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using PositionLengthAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionLengthAttribute;
using AttributeSource = org::apache::lucene::util::AttributeSource;
using RollingBuffer = org::apache::lucene::util::RollingBuffer;

void FlattenGraphFilter::InputNode::reset()
{
  tokens.clear();
  node = -1;
  outputNode = -1;
  maxToNode = -1;
  nextOut = 0;
}

void FlattenGraphFilter::OutputNode::reset()
{
  inputNodes.clear();
  node = -1;
  nextOut = 0;
  startOffset = -1;
  endOffset = -1;
}

FlattenGraphFilter::RollingBufferAnonymousInnerClass::
    RollingBufferAnonymousInnerClass()
{
}

shared_ptr<InputNode>
FlattenGraphFilter::RollingBufferAnonymousInnerClass::newInstance()
{
  return make_shared<InputNode>();
}

FlattenGraphFilter::RollingBufferAnonymousInnerClass2::
    RollingBufferAnonymousInnerClass2()
{
}

shared_ptr<OutputNode>
FlattenGraphFilter::RollingBufferAnonymousInnerClass2::newInstance()
{
  return make_shared<OutputNode>();
}

FlattenGraphFilter::FlattenGraphFilter(shared_ptr<TokenStream> in_)
    : org::apache::lucene::analysis::TokenFilter(in_)
{
}

bool FlattenGraphFilter::releaseBufferedToken()
{

  // We only need the while loop (retry) if we have a hole (an output node that
  // has no tokens leaving):
  while (outputFrom < outputNodes::getMaxPos()) {
    shared_ptr<OutputNode> output = outputNodes->get(outputFrom);
    if (output->inputNodes->isEmpty()) {
      // No tokens arrived to this node, which happens for the first node
      // after a hole:
      // System.out.println("    skip empty outputFrom=" + outputFrom);
      outputFrom++;
      continue;
    }

    int maxToNode = -1;
    for (int inputNodeID : output->inputNodes) {
      shared_ptr<InputNode> inputNode = inputNodes->get(inputNodeID);
      assert(inputNode->outputNode == outputFrom);
      maxToNode = max(maxToNode, inputNode->maxToNode);
    }
    // System.out.println("  release maxToNode=" + maxToNode + " vs inputFrom="
    // + inputFrom);

    // TODO: we could shrink the frontier here somewhat if we
    // always output posLen=1 as part of our "sausagizing":
    if (maxToNode <= inputFrom || done) {
      // System.out.println("  output node merged these inputs: " +
      // output.inputNodes);
      // These tokens are now frozen
      assert((output->nextOut < output->inputNodes->size(),
              L"output.nextOut=" + to_wstring(output->nextOut) +
                  L" vs output.inputNodes.size()=" +
                  output->inputNodes->size()));
      shared_ptr<InputNode> inputNode =
          inputNodes->get(output->inputNodes->get(output->nextOut));
      if (done && inputNode->tokens->size() == 0 &&
          outputFrom >= outputNodes::getMaxPos()) {
        return false;
      }
      if (inputNode->tokens->size() == 0) {
        assert(inputNode->nextOut == 0);
        assert(output->nextOut == 0);
        // Hole dest nodes should never be merged since 1) we always
        // assign them to a new output position, and 2) since they never
        // have arriving tokens they cannot be pushed:
        assert((output->inputNodes->size() == 1, output->inputNodes->size()));
        outputFrom++;
        inputNodes::freeBefore(output->inputNodes->get(0));
        outputNodes::freeBefore(outputFrom);
        continue;
      }

      assert(inputNode->nextOut < inputNode->tokens->size());

      restoreState(inputNode->tokens->get(inputNode->nextOut));

      // Correct posInc
      assert(outputFrom >= lastOutputFrom);
      posIncAtt->setPositionIncrement(outputFrom - lastOutputFrom);
      int toInputNodeID = inputNode->node + posLenAtt->getPositionLength();
      shared_ptr<InputNode> toInputNode = inputNodes->get(toInputNodeID);

      // Correct posLen
      assert(toInputNode->outputNode > outputFrom);
      posLenAtt->setPositionLength(toInputNode->outputNode - outputFrom);
      lastOutputFrom = outputFrom;
      inputNode->nextOut++;
      // System.out.println("  ret " + this);

      shared_ptr<OutputNode> outputEndNode =
          outputNodes->get(toInputNode->outputNode);

      // Correct offsets

      // This is a bit messy; we must do this so offset don't go backwards,
      // which would otherwise happen if the replacement has more tokens
      // than the input:
      int startOffset = max(lastStartOffset, output->startOffset);

      // We must do this in case the incoming tokens have broken offsets:
      int endOffset = max(startOffset, outputEndNode->endOffset);

      offsetAtt->setOffset(startOffset, endOffset);
      lastStartOffset = startOffset;

      if (inputNode->nextOut == inputNode->tokens->size()) {
        output->nextOut++;
        if (output->nextOut == output->inputNodes->size()) {
          outputFrom++;
          inputNodes::freeBefore(output->inputNodes->get(0));
          outputNodes::freeBefore(outputFrom);
        }
      }

      return true;
    } else {
      return false;
    }
  }

  // System.out.println("    break false");
  return false;
}

bool FlattenGraphFilter::incrementToken() 
{
  // System.out.println("\nF.increment inputFrom=" + inputFrom + " outputFrom="
  // + outputFrom);

  while (true) {
    if (releaseBufferedToken()) {
      // retOutputFrom += posIncAtt.getPositionIncrement();
      // System.out.println("    return buffered: " + termAtt + " " +
      // retOutputFrom + "-" + (retOutputFrom + posLenAtt.getPositionLength()));
      // printStates();
      return true;
    } else if (done) {
      // System.out.println("    done, return false");
      return false;
    }

    if (input->incrementToken()) {
      // Input node this token leaves from:
      inputFrom += posIncAtt->getPositionIncrement();

      int startOffset = offsetAtt->startOffset();
      int endOffset = offsetAtt->endOffset();

      // Input node this token goes to:
      int inputTo = inputFrom + posLenAtt->getPositionLength();
      // System.out.println("  input.inc " + termAtt + ": " + inputFrom + "-" +
      // inputTo);

      shared_ptr<InputNode> src = inputNodes->get(inputFrom);
      if (src->node == -1) {
        // This means the "from" node of this token was never seen as a "to"
        // node, which should only happen if we just crossed a hole.  This is a
        // challenging case for us because we normally rely on the full
        // dependencies expressed by the arcs to assign outgoing node IDs.  It
        // would be better if tokens were never dropped but instead just marked
        // deleted with a new TermDeletedAttribute (bool valued) ... but
        // until that future, we have a hack here to forcefully jump the output
        // node ID:
        assert(src->outputNode == -1);
        src->node = inputFrom;

        src->outputNode = outputNodes::getMaxPos() + 1;
        // System.out.println("    hole: force to outputNode=" +
        // src.outputNode);
        shared_ptr<OutputNode> outSrc = outputNodes->get(src->outputNode);

        // Not assigned yet:
        assert(outSrc->node == -1);
        outSrc->node = src->outputNode;
        outSrc->inputNodes->add(inputFrom);
        outSrc->startOffset = startOffset;
      } else {
        shared_ptr<OutputNode> outSrc = outputNodes->get(src->outputNode);
        if (outSrc->startOffset == -1 || startOffset > outSrc->startOffset) {
          // "shrink wrap" the offsets so the original tokens (with most
          // restrictive offsets) win:
          outSrc->startOffset = max(startOffset, outSrc->startOffset);
        }
      }

      // Buffer this token:
      src->tokens->add(captureState());
      src->maxToNode = max(src->maxToNode, inputTo);
      maxLookaheadUsed = max(maxLookaheadUsed, inputNodes::getBufferSize());

      shared_ptr<InputNode> dest = inputNodes->get(inputTo);
      if (dest->node == -1) {
        // Common case: first time a token is arriving to this input position:
        dest->node = inputTo;
      }

      // Always number output nodes sequentially:
      int outputEndNode = src->outputNode + 1;

      if (outputEndNode > dest->outputNode) {
        if (dest->outputNode != -1) {
          bool removed = outputNodes->get(dest->outputNode)
                             .inputNodes->remove(static_cast<Integer>(inputTo));
          assert(removed);
        }
        // System.out.println("    increase output node: " + dest.outputNode + "
        // vs " + outputEndNode);
        outputNodes->get(outputEndNode).inputNodes->add(inputTo);
        dest->outputNode = outputEndNode;

        // Since all we ever do is merge incoming nodes together, and then
        // renumber the merged nodes sequentially, we should only ever assign
        // smaller node numbers:
        assert((outputEndNode <= inputTo,
                L"outputEndNode=" + to_wstring(outputEndNode) +
                    L" vs inputTo=" + to_wstring(inputTo)));
      }

      shared_ptr<OutputNode> outDest = outputNodes->get(dest->outputNode);
      // "shrink wrap" the offsets so the original tokens (with most
      // restrictive offsets) win:
      if (outDest->endOffset == -1 || endOffset < outDest->endOffset) {
        outDest->endOffset = endOffset;
      }

    } else {
      // System.out.println("  got false from input");
      input->end();
      finalPosInc = posIncAtt->getPositionIncrement();
      finalOffset = offsetAtt->endOffset();
      done = true;
      // Don't return false here: we need to force release any buffered tokens
      // now
    }
  }
}

void FlattenGraphFilter::end() 
{
  if (done == false) {
    TokenFilter::end();
  } else {
    // NOTE, shady: don't call super.end, because we did already from
    // incrementToken
  }

  clearAttributes();
  if (done) {
    // On exc, done is false, and we will not have set these:
    posIncAtt->setPositionIncrement(finalPosInc);
    offsetAtt->setOffset(finalOffset, finalOffset);
  } else {
    TokenFilter::end();
  }
}

void FlattenGraphFilter::reset() 
{
  // System.out.println("F: reset");
  TokenFilter::reset();
  inputFrom = -1;
  inputNodes::reset();
  shared_ptr<InputNode> in_ = inputNodes->get(0);
  in_->node = 0;
  in_->outputNode = 0;

  outputNodes::reset();
  shared_ptr<OutputNode> out = outputNodes->get(0);
  out->node = 0;
  out->inputNodes->add(0);
  out->startOffset = 0;
  outputFrom = 0;
  // retOutputFrom = -1;
  lastOutputFrom = -1;
  done = false;
  finalPosInc = -1;
  finalOffset = -1;
  lastStartOffset = 0;
  maxLookaheadUsed = 0;
}

int FlattenGraphFilter::getMaxLookaheadUsed() { return maxLookaheadUsed; }
} // namespace org::apache::lucene::analysis::core