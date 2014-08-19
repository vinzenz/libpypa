// Copyright 2014 Vinzenz Feenstra
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#ifndef GUARD_PYPA_PARSER_FUTURE_FEATURES_HH_INCLUDED
#define GUARD_PYPA_PARSER_FUTURE_FEATURES_HH_INCLUDED

namespace pypa {

struct FutureFeatures {
    bool nested_scopes;
    bool generators;
    bool division;
    bool absolute_imports;
    bool with_statement;
    bool print_function;
    bool unicode_literals;
    int  last_line; // last line with a future feature statement

    FutureFeatures()
    : nested_scopes()
    , generators()
    , division()
    , absolute_imports()
    , with_statement()
    , print_function()
    , unicode_literals()
    , last_line()
    {}
};

}

#endif // GUARD_PYPA_PARSER_FUTURE_FEATURES_HH_INCLUDED
