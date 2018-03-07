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
#ifndef GUARD_PYPA_READER_HH_INCLUDED
#define GUARD_PYPA_READER_HH_INCLUDED

#include <cstddef>
#include <string>

namespace pypa {

class Reader {
public:
    Reader() {}
    virtual ~Reader() {}

    virtual bool set_encoding(const std::string & coding) = 0;
    virtual std::string next_line() = 0;
    virtual std::string get_line(size_t idx) = 0;
    virtual unsigned get_line_number() const = 0;
    virtual std::string get_filename() const = 0;
    virtual bool eof() const = 0;
};

}

#endif //GUARD_PYPA_READER_HH_INCLUDED
