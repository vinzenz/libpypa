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
#include <string>

#include <pypa/reader.hh>

namespace pypa {

#if defined(WIN32)
    typedef void* file_handle_t;
#else
    typedef int file_handle_t;
#endif

class FileBuf {
    static const unsigned BufferSize = 32;
    file_handle_t handle_;
    char buffer_[BufferSize];
    unsigned position_;
    unsigned length_;
    unsigned line_;
    char current_;
    bool eof_;
    bool utf8_;
public:
    FileBuf(char const * file_path);
    ~FileBuf();

    unsigned line() const { return line_; }
    char next();
    char current() const;
    bool eof() const;
    bool utf8() const;
private:
    bool fill_buffer();
};


class FileBufReader : public Reader {
public:
    FileBufReader(const std::string & file_name);
    ~FileBufReader() override {}

    bool set_encoding(const std::string & coding) override { return true; }
    std::string next_line() override;
    std::string get_line(size_t idx) override;
    unsigned get_line_number() const override { return buf_.line(); }
    std::string get_filename() const override { return file_name_; }
    bool eof() const override { return buf_.eof(); }

private:
    std::string file_name_;
    FileBuf buf_;
};

}

#endif //GUARD_PYPA_FILEBUF_HH_INCLUDED
