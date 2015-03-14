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

#include <pypa/types.hh>
#include <cassert>
#include <cstring>
#include <iconv.h>
#include <errno.h>

namespace pypa {

inline bool isxdigit(char c) {
    return (c >= '0' && c <= '9')
        || (c >= 'a' && c <= 'f')
        || (c >= 'A' && c <= 'F');
}

inline bool islower(char c) {
    return (c >= 'a' && c <= 'z');
}

inline unsigned int hex_digit_to_dec(char c) {
    if (isdigit(c))
        return c - '0';
    else if (islower(c))
        return 10 + c - 'a';
    return 10 + c - 'A';
}

unsigned int number_from_hex_chars(char const * & s, int num_bytes_to_read) {
    assert(num_bytes_to_read <= 8);
    unsigned int result = 0;
    for(int i=0; i<num_bytes_to_read; ++i) {
        char c = *s;
        if(!isxdigit(c))
            assert(0 && ""); // TODO emit error
        s++;
        result = (result << 4) | hex_digit_to_dec(c);
    }
    return result;
}

static String convert_utf32le_to_utf8(unsigned int cp) {
    char out[8];
    memset(out, 0, sizeof(out));
    size_t in_bytes_left = 4;
    size_t out_bytes_left = sizeof(out);
    iconv_t cd = iconv_open("UTF-8", "UTF-32le");
    char* in_position = (char*)&cp;
    char* out_position = out;
    int rc = iconv(cd, &in_position, &in_bytes_left, &out_position, &out_bytes_left);
    assert(rc != -1 && "iconv failed");
    iconv_close(cd);
    return String(out, out_position-out);
}

String make_string(String const & input, bool & unicode) {
    String result;
    size_t first_quote  = input.find_first_of("\"'");
    assert(first_quote != String::npos);
    char quote = input[first_quote];
    size_t string_start = input.find_first_not_of(quote, first_quote);

    char const * qst = input.c_str() + first_quote;
    char const * tmp = input.c_str();

    bool raw = false;
    // bool bytes = false;

    while(tmp != qst) {
        switch(*tmp) {
        case 'u': case 'U':
            unicode = true;
            break;
        case 'r': case 'R':
            raw = true;
            break;
        case 'b': case 'B':
            // bytes = true;
            unicode = false;
            break;
        default:
            assert("Unknown character prefix" && false);
            break;
        }
        ++tmp;
    }


    if(string_start == String::npos) {
        // Empty string
        return result;
    }

    assert((string_start - first_quote) < input.size());
    size_t string_end =  input.size() - (string_start - first_quote);
    assert(string_end != String::npos && string_start <= string_end);

    char const * s = input.c_str() + string_start;
    char const * end = input.c_str() + string_end;

    std::back_insert_iterator<String> p((result));

    while(s < end) {
        char c = *s;
        if(raw) {
            *p++ = *s++;
        }
        else { // !raw
            switch(*s) {
            case '\\':
                ++s;
                assert(s < end);
                c = *s;
                ++s;
                switch(c) {
                case '\n': break;
                case '\\': case '\'': case '\"': *p++ = c; break;
                case 'b': *p++ = '\b'; break;
                case 'f': *p++ = '\014'; break; /* FF */
                case 't': *p++ = '\t'; break;
                case 'n': *p++ = '\n'; break;
                case 'r': *p++ = '\r'; break;
                case 'v': *p++ = '\013'; break;
                case 'a': *p++ = '\007'; break;
                case '0': case '1': case '2': case '3':
                case '4': case '5': case '6': case '7':
                    c = c - '0';
                    if (s < end && '0' <= *s && *s <= '7') {
                        c = (c<<3) + *s++ - '0';
                        if (s < end && '0' <= *s && *s <= '7') {
                            c = (c<<3) + *s++ - '0';
                        }
                    }
                    *p++ = c;
                    break;
                case 'x': {
                    unsigned int x = number_from_hex_chars(s, 2);
                    if(unicode) {
                        String str = convert_utf32le_to_utf8(x);
                        std::copy(str.begin(), str.end(), p);
                    } else {
                        *p++ = static_cast<unsigned char>(x);
                    }
                    break;
                }
                default:
                    if(unicode && (c == 'u' || c == 'U')) {
                        unsigned int x = number_from_hex_chars(s, c == 'u' ? 4 : 8);
                        String str = convert_utf32le_to_utf8(x);
                        std::copy(str.begin(), str.end(), p);
                        break;
                    }

                    *p++ = '\\';
                    s--;
                }
            break;
            default:
                *p++ = *s++;
                break;
            }
        } // else !raw
    }
    return result;
}


}
