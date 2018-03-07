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
#include <pypa/filebuf.hh>
#include <fstream>

#if defined(WIN32)
#include <windows.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif
#include <cstdio>

namespace pypa {

    inline file_handle_t open_file(char const * file_path) {
        file_handle_t v{};
#if defined(WIN32)
        v = ::CreateFileA(file_path, GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);
#else
        v = ::open(file_path, O_RDONLY);
#endif
        return v;
    }

    FileBuf::FileBuf(char const * file_path)
    : handle_(open_file(file_path))
    , buffer_{}
    , position_{0}
    , length_{0}
    , line_{1}
    , current_{EOF}
    , eof_{true}
    , utf8_{false}
    {
        eof_ = !fill_buffer();
        if(length_ >= 3) {
            utf8_ = buffer_[0] == '\xEF'
                 && buffer_[1] == '\xBB'
                 && buffer_[2] == '\xBF';
            position_ += utf8_ ? 3 : 0;
        }
    }

    FileBuf::~FileBuf() {
#if defined(WIN32)
        if (handle_!= INVALID_HANDLE_VALUE) {
            CloseHandle(handle_);
        }
#else
        ::close(handle_);
#endif
    }

    bool FileBuf::utf8() const {
        return utf8_;
    }

    char FileBuf::next() {
        if(position_ >= length_) {
            if(length_ == 0 || !fill_buffer()) {
                current_ = -1;
                eof_ = true;
            }
        }
        if (position_ < length_) {
            eof_ = false;
            current_ = unsigned(buffer_[position_++]);
            if(current_ == '\n' || current_ == '\x0c') line_++;
        }
        return current_;
    }

    char FileBuf::current() const {
        return current_;
    }

    bool FileBuf::eof() const {
        return eof_;
    }

    bool FileBuf::fill_buffer() {
#if defined(WIN32)
        int n = -1;
        if (handle_!= INVALID_HANDLE_VALUE) {
            DWORD bytes_read = 0;
            if (::ReadFile(handle_, buffer_, BufferSize, &bytes_read, 0)) {
                n = int(bytes_read);
            }
        }
#else
        int n = read(handle_, buffer_, BufferSize);
#endif
        position_ = 0;
        length_ = n <= 0 ? 0 : unsigned(n);
        return length_ != 0;
    }


    FileBufReader::FileBufReader(const std::string & file_name)
    : file_name_(file_name), buf_(file_name.c_str())
    {
    }

    std::string FileBufReader::get_line(size_t idx) {
        idx = idx ? idx - 1 : idx;
        std::ifstream ifs(get_filename());
        std::string line;
        size_t lineno = 1;
        while(std::getline(ifs, line) && lineno != idx) {
            ++lineno;
        }
        return line;

    }

    std::string FileBufReader::next_line() {
        std::string line;
        char c;
        do {
            c = buf_.next();
            if(buf_.eof())
                break;
            line.push_back(c);
        } while(c != '\n' && c != '\x0c');
        return line;
    }

}


