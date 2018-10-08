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
 * {@link org.apache.lucene.analysis.TokenFilter} and {@link
 * org.apache.lucene.analysis.Analyzer} implementations that use Snowball
 * stemmers.
 * <p>
 * This project provides pre-compiled version of the Snowball stemmers
 * based on revision 500 of the Tartarus Snowball repository,
 * together with classes integrating them with the Lucene search engine.
 * </p>
 * <p>
 * A few changes has been made to the static Snowball code and compiled
 * stemmers:
 * </p>
 * <ul>
 *   <li>Class SnowballProgram is made abstract and contains new abstract method
 * stem() to avoid reflection in Lucene filter class SnowballFilter.</li>
 *   <li>All use of StringBuffers has been refactored to StringBuilder for
 * speed.</li> <li>Snowball BSD license header has been added to the Java
 * classes to avoid having RAT adding ASL headers.</li>
 * </ul>
 * <p>
 * See the Snowball <a href ="http://snowball.tartarus.org/">home page</a> for
 * more information about the algorithms.
 * </p>
 *
 * <p>
 * <b>IMPORTANT NOTICE ON BACKWARDS COMPATIBILITY!</b>
 * </p>
 * <p>
 * An index created using the Snowball module in Lucene 2.3.2 and below
 * might not be compatible with the Snowball module in Lucene 2.4 or greater.
 * </p>
 * <p>
 * For more information about this issue see:
 * https://issues.apache.org/jira/browse/LUCENE-1142
 * </p>
 */
namespace org::apache::lucene::analysis::snowball
{
}
