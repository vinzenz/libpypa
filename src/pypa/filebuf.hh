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
#ifndef GUARD_PYPA_FILEBUF_HH_INCLUDED
#define GUARD_PYPA_FILEBUF_HH_INCLUDED

#include <cstddef>

namespace pypa {

union file_handle_t{ int unix; void * win; };

class FileBuf {
    static const unsigned BufferSize = 32;
    file_handle_t handle_;
    char buffer_[BufferSize];
    unsigned position_;
    unsigned length_;
    char current_;
    bool utf8_;
public:
    FileBuf(char const * file_path);
    ~FileBuf();

    char next();
    char current() const;
    bool eof() const;
    bool utf8() const;
private:
    bool fill_buffer();
};

};

#endif //GUARD_PYPA_FILEBUF_HH_INCLUDED
