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
 * This package provides dictionary-driven lemmatization ("accurate stemming")
 * filter and analyzer for the Polish Language, driven by the
 * <a href="http://morfologik.blogspot.com/">Morfologik library</a> developed
 * by Dawid Weiss and Marcin Miłkowski.
 * <p>
 * The MorfologikFilter yields one or more terms for each token. Each
 * of those terms is given the same position in the index.
 * </p>
 */
namespace org::apache::lucene::analysis::morfologik
{
}
