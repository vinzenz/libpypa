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
