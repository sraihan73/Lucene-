#pragma once
#include "stringhelper.h"
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

/**
 * BlockTree terms dictionary.
 *
 * <p>
 * This terms dictionary organizes all terms into blocks according to
 * shared prefix, such that each block has enough terms, and then stores
 * the prefix trie in memory as an FST as the index structure.  It allows
 * you to plug in your own {@link
 * org.apache.lucene.codecs.PostingsWriterBase} to implement the
 * postings.
 * </p>
 *
 * <p>See {@link org.apache.lucene.codecs.blocktree.BlockTreeTermsWriter}
 *   for the file format.
 * </p>
 */
namespace org::apache::lucene::codecs::blocktree
{
}
