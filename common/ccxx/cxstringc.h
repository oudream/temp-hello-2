#ifndef CX_CT_X2_CXSTRINGC_H
#define CX_CT_X2_CXSTRINGC_H


#include "cxglobal.h"


class CCXX_EXPORT CxStringC {
public:
    //***
    //be care begin : malloc
    //***
    /**
     * Duplicate null terminated text into the heap.
     * @param text to duplicate.
     * @return duplicate copy of text allocated from heap.
     */
    static char *create(const char *text);

    /**
     * Duplicate null terminated text of specific size to heap.
     * @param text to duplicate.
     * @param size of text, maximum space allocated.
     * @return duplicate copy of text allocated on heap.
     */
    static char *createLeft(const char *text, size_t size);

    inline static char *createRight(const char *text, size_t size) { return create(pos(text, -(signed) size)); }

    inline static char *create(const char *text, size_t offset, size_t len) {
        return createLeft(pos(text, offset), len);
    }
    //***
    //be care end : malloc
    //***

    /**
     * Safely set a null terminated string buffer in memory.  If the text
     * is too large to fit into the buffer, it is truncated to the size.
     * @param buffer to set.
     * @param size of buffer ( = string.size() , include \0 at the end).  Includes null byte at end of string.
     * @param text to set in buffer.
     * @return pointer to text buffer.
     */
    static char *set(char *buffer, size_t size, const char *text);

    /**
     * Safely set a null terminated string buffer in memory.  If the text
     * is too large to fit into the buffer, it is truncated to the size.
     * @param buffer to set.
     * @param size of buffer.  Includes null byte at end of string.
     * @param text to set in buffer.
     * @param max size of text to set.
     * @return pointer to text buffer.
     */
    static char *set(char *buffer, size_t size, const char *text, size_t max);

    /**
     * A thread-safe token parsing routine for null terminated strings.  This
     * is related to strtok, but with safety checks for nullptr values and a
     * number of enhancements including support for quoted text that may
     * contain token separators within quotes.  The text string is modified
     * as it is parsed.
     * @param text string to examine for tokens.
     * @param last token position or set to nullptr for start of string.
     * @param list of characters to use as token separators.
     * @param quote pairs of characters for quoted text or nullptr if not used.
     * @param end of line marker characters or nullptr if not used.
     * @return token extracted from string or nullptr if no more tokens found.
     * e.g. :
            char buff[33];
            char *tokens = nullptr;
            unsigned count = 0;
            const char *tp;
            const char *array[5];

            memset(buff, ' ', 32);
            CxStringC::set(buff, sizeof(buff), "this is \"a test\"");
            while(nullptr != (tp = CxString::token(buff, &tokens, " ", "\"\"")) && count < 4)
                array[count++] = tp;
     * array[0] == "this";
     * array[1] == "is";
     * array[2] == "a test";
     */
    static char *
    token(char *text, char **last, const char *list, const char *quote = nullptr, const char *end = nullptr);

    /**
     * Unquote a quoted null terminated string.  Returns updated string
     * position and replaces trailing quote with null byte if quoted.
     * @param text to examine.
     * @param quote pairs of character for open and close quote.
     * @return new text pointer if quoted, nullptr if unchanged.
     * e.g. : unquote("[abc]", "[]()") == "abc"
     */
    static char *unquote(char *text, const char *quote);

    /**
     * Depreciated case insensitive string comparison function.
     * @param text1 to compare.
     * @param text2 to compare.
     * @return 0 if equal, >0 if text1 > text2, <0 if text1 < text2.
     */
    static int compareCase(const char *text1, const char *text2);

    /**
     * Return start of string after characters to trim from beginning.
     * This function does not modify memory.
     * @param text buffer to examine.
     * @param list of characters to skip from start.
     * @return position in text past lead trim.
     */
    static char *trimLeft(char *text, const char *list);

    /**
     * Strip trailing characters from the text string.  This function will
     * modify memory.
     * @param text buffer to examine.
     * @param list of characters to chop from trailing end of string.
     * @return pointer to text buffer.
     */
    static char *trimRight(char *text, const char *list);

    /**
     * Skip lead and remove trailing characters from a text string.  This
     * function will modify memory.
     * @param text buffer to examine.
     * @param list of characters to trim and chop.
     * @return position in text past lead trim.
     */
    static char *trim(char *text, const char *list);

    /**
     * Offset until next occurrence of character in a text or length.
     * @param text buffer to examine.
     * @param list of characters to search for.
     * @return offset to next occurrence or length of string.
     */
    static size_t strcspnRight(char *text, const char *list);

    /**
     * inside after lead characters in a null terminated string.
     * @param text buffer to examine.
     * @param list of characters to search for.
     * @return pointer to first part of string past insided characters.
     */
    static char *inside(char *text, const char *list);

    /**
     * inside after lead characters in a null terminated string.
     * @param text buffer to examine.
     * @param list of characters to search for.
     * @return pointer to first part of string past insided characters.
     */
    static char *insideRight(char *text, const char *list);

    /**
     * Skip after lead characters in a null terminated string.
     * @param text pointer to start at.
     * @param list of characters to skip when found.
     * @return pointer to first part of string past skipped characters.
     */
    static char *skip(char *text, const char *list);

    /**
     * Skip before trailing characters in a null terminated string.
     * @param text pointer to start at.
     * @param list of characters to skip when found.
     * @return pointer to last part of string past skipped characters.
     */
    static char *skipRight(char *text, const char *list);

    /**
     * Dump hex data to a string buffer.
     * @param binary memory to dump.
     * @param string to save into.
     * @param format string to convert with.
     * @return number of bytes processed.
     */
    static unsigned hexdump(const unsigned char *binary, char *string, const char *format);

    /**
     * Pack hex data from a string buffer.
     * @param binary memory to pack.
     * @param string to save into.
     * @param format string to convert with.
     * @return number of bytes processed.
     */
    static unsigned hexpack(unsigned char *binary, const char *string, const char *format);

    static unsigned hexsize(const char *format);

    /**
     * Standard radix 64 encoding.
     * @param string of encoded text save into.
     * @param binary data to encode.
     * @param size of binary data to encode.
     * @param width of string buffer for data if partial supported.
     * @return number of bytes encoded.
     */
    static size_t b64encode(char *string, const cx::uint8 *binary, size_t size, size_t width = 0);

    /**
     * Standard radix 64 decoding.
     * @param binary data to save.
     * @param string of encoded text.
     * @param size of destination buffer.
     * @return number of bytes actually decoded.
     */
    static size_t b64decode(cx::uint8 *binary, const char *string, size_t size);

private:
    /**
     * Compute position in string.
     * @param text of string.
     * @param offset from start, negative values from end.
     * @return pointer to string position.
     */
    static const char *pos(const char *text, signed long offset);

};


#endif //CX_CT_X2_CXSTRINGC_H
