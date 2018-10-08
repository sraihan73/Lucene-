#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <string>

/*
 * Licensed to the Syed Mamun Raihan (sraihan.com) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * sraihan.com licenses this file to You under GPLv3 License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
namespace org::apache::lucene::util::fst
{

using DataInput = org::apache::lucene::store::DataInput;
using DataOutput = org::apache::lucene::store::DataOutput;

/**
 * Represents the outputs for an FST, providing the basic
 * algebra required for building and traversing the FST.
 *
 * <p>Note that any operation that returns NO_OUTPUT must
 * return the same singleton object from {@link
 * #getNoOutput}.</p>
 *
 * @lucene.experimental
 */

template <typename T>
class Outputs : public std::enable_shared_from_this<Outputs>
{
  GET_CLASS_NAME(Outputs)

  // TODO: maybe change this API to allow for re-use of the
  // output instances -- this is an insane amount of garbage
  // (new object per byte/char/int) if eg used during
  // analysis

  /** Eg common("foobar", "food") -&gt; "foo" */
public:
  virtual T common(T output1, T output2) = 0;

  /** Eg subtract("foobar", "foo") -&gt; "bar" */
  virtual T subtract(T output, T inc) = 0;

  /** Eg add("foo", "bar") -&gt; "foobar" */
  virtual T add(T prefix, T output) = 0;

  /** Encode an output value into a {@link DataOutput}. */
  virtual void write(T output, std::shared_ptr<DataOutput> out) = 0;

  /** Encode an final node output value into a {@link
   *  DataOutput}.  By default this just calls {@link #write(Object,
   *  DataOutput)}. */
  virtual void
  writeFinalOutput(T output, std::shared_ptr<DataOutput> out) 
  {
    write(output, out);
  }

  /** Decode an output value previously written with {@link
   *  #write(Object, DataOutput)}. */
  virtual T read(std::shared_ptr<DataInput> in_) = 0;

  /** Skip the output; defaults to just calling {@link #read}
   *  and discarding the result. */
  virtual void skipOutput(std::shared_ptr<DataInput> in_) 
  {
    read(in_);
  }

  /** Decode an output value previously written with {@link
   *  #writeFinalOutput(Object, DataOutput)}.  By default this
   *  just calls {@link #read(DataInput)}. */
  virtual T readFinalOutput(std::shared_ptr<DataInput> in_) 
  {
    return read(in_);
  }

  /** Skip the output previously written with {@link #writeFinalOutput};
   *  defaults to just calling {@link #readFinalOutput} and discarding
   *  the result. */
  virtual void
  skipFinalOutput(std::shared_ptr<DataInput> in_) 
  {
    skipOutput(in_);
  }

  /** NOTE: this output is compared with == so you must
   *  ensure that all methods return the single object if
   *  it's really no output */
  virtual T getNoOutput() = 0;

  virtual std::wstring outputToString(T output) = 0;

  // TODO: maybe make valid(T output) public...?  for asserts

  virtual T merge(T first, T second)
  {
    throw std::make_shared<UnsupportedOperationException>();
  }

  /** Return memory usage for the provided output.
   *  @see Accountable */
  virtual int64_t ramBytesUsed(T output) = 0;
};

} // #include  "core/src/java/org/apache/lucene/util/fst/
