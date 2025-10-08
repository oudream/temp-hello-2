#include "cxstringc.h"


char *CxStringC::create(const char *cp)
{
    char *mem;

    if (!cp)
        return nullptr;

    size_t len = strlen(cp) + 1;
    mem = (char *) malloc(len);
    CX_ASSERT_MSG(mem != nullptr, "string dup allocation error");
    set(mem, len, cp);
    return mem;
}

char *CxStringC::createLeft(const char *cp, size_t size)
{
    char *mem;

    if (!cp)
        return nullptr;

    if (!size)
        size = strlen(cp);

    mem = (char *) malloc(++size);
    CX_ASSERT_MSG(mem != nullptr, "string dup allocation error");
    set(mem, size, cp);
    return mem;
}

char *CxStringC::set(char *str, size_t size, const char *s)
{
    if (!str)
        return nullptr;

    if (size < 2)
        return str;

    if (!s)
        s = "";

    size_t l = strlen(s);

    if (l >= size)
        l = size - 1;

    if (!l)
    {
        *str = 0;
        return str;
    }

    memmove(str, s, l);
    str[l] = 0;
    return str;
}

char *CxStringC::set(char *str, size_t size, const char *s, size_t len)
{
    if (!str)
        return nullptr;

    if (size < 2)
        return str;

    if (!s)
        s = "";

    size_t l = strlen(s);
    if (l >= size)
        l = size - 1;

    if (l > len)
        l = len;

    if (!l)
    {
        *str = 0;
        return str;
    }

    memmove(str, s, l);
    str[l] = 0;
    return str;
}

char *CxStringC::token(char *text, char **token, const char *clist, const char *quote, const char *eol)
{
    char *result;

    if (!eol)
        eol = "";

    if (!token || !clist)
        return nullptr;

    if (!*token)
        *token = text;

    if (!**token)
    {
        *token = text;
        return nullptr;
    }

    while (**token && strchr(clist, **token))
        ++*token;

    result = *token;

    if (*result && *eol && nullptr != (eol = strchr(eol, *result)))
    {
        if (eol[0] != eol[1] || *result == eol[1])
        {
            *token = text;
            return nullptr;
        }
    }

    if (!*result)
    {
        *token = text;
        return nullptr;
    }

    while (quote && *quote && *result != *quote)
    {
        quote += 2;
    }

    if (quote && *quote)
    {
        ++result;
        ++quote;
        *token = strchr(result, *quote);
        if (!*token)
            *token = result + strlen(result);
        else
        {
            **token = 0;
            ++(*token);
        }
        return result;
    }

    while (**token && !strchr(clist, **token))
        ++(*token);

    if (**token)
    {
        **token = 0;
        ++(*token);
    }

    return result;
}

char *CxStringC::unquote(char *str, const char *clist)
{
    assert(clist != nullptr);

    size_t len = strlen(str);
    if (!len || !str)
        return nullptr;

    while (clist[0])
    {
        if (*str == clist[0] && str[len - 1] == clist[1])
        {
            str[len - 1] = 0;
            return ++str;
        }
        clist += 2;
    }
    return str;
}

int CxStringC::compareCase(const char *s1, const char *s2)
{
    if (!s1)
        s1 = "";

    if (!s2)
        s2 = "";

#ifdef _WIN32
    return _stricmp(s1, s2);
#else
    return strcasecmp(s1, s2);
#endif
}

char *CxStringC::trimLeft(char *str, const char *clist)
{
    if (!str)
        return nullptr;

    if (!clist)
        return str;

    while (*str && strchr(clist, *str))
        ++str;

    return str;
}

char *CxStringC::trimRight(char *str, const char *clist)
{
    if (!str)
        return nullptr;

    if (!clist)
        return str;

    size_t offset = strlen(str);
    while (offset && strchr(clist, str[offset - 1]))
        str[--offset] = 0;
    return str;
}

char *CxStringC::trim(char *str, const char *clist)
{
    str = trimLeft(str, clist);
    trimRight(str, clist);
    return str;
}

size_t CxStringC::strcspnRight(char *str, const char *clist)
{
    size_t size = strlen(str);

    if (!str)
        return 0;

    if (!clist)
        return size;

    size_t pos = size - 1;
    while (str[pos])
    {
        if (strchr(clist, str[pos]))
            return pos;
        --pos;
    }
    return size;
}

char *CxStringC::inside(char *str, const char *clist)
{
    if (!str || !clist)
        return nullptr;

    while (*str)
    {
        if (strchr(clist, *str))
            return str;
        ++str;
    }
    return nullptr;
}

char *CxStringC::insideRight(char *str, const char *clist)
{
    if (!str || !clist)
        return nullptr;

    str += strlen(str) - 1;
    while (*str)
    {
        if (strchr(clist, *str))
            return str;
        --str;
    }
    return nullptr;
}

char *CxStringC::skip(char *str, const char *clist)
{
    if (!str || !clist)
        return nullptr;

    while (*str && strchr(clist, *str))
        ++str;

    if (*str)
        return str;

    return nullptr;
}

char *CxStringC::skipRight(char *str, const char *clist)
{
    size_t len = strlen(str);

    if (!len || !clist)
        return nullptr;

    while (len > 0)
    {
        if (!strchr(clist, str[--len]))
            return str;
    }
    return nullptr;
}

unsigned CxStringC::hexsize(const char *format)
{
    unsigned count = 0;
    char *ep;
    unsigned skip;

    while (format && *format)
    {
        while (*format && !isdigit(*format))
        {
            ++format;
            ++count;
        }
        if (isdigit(*format))
        {
            skip = (unsigned) strtol(format, &ep, 10);
            format = ep;
            count += skip * 2;
        }
    }
    return count;
}

unsigned CxStringC::hexdump(const unsigned char *binary, char *string, const char *format)
{
    unsigned count = 0;
    char *ep;
    unsigned skip;

    while (format && *format)
    {
        while (*format && !isdigit(*format))
        {
            *(string++) = *(format++);
            ++count;
        }
        if (isdigit(*format))
        {
            skip = (unsigned) strtol(format, &ep, 10);
            format = ep;
            count += skip * 2;
            while (skip--)
            {
                snprintf(string, 3, "%02x", *(binary++));
                string += 2;
            }
        }
    }
    *string = 0;
    return count;
}

static unsigned hex(char ch)
{
    if (ch >= '0' && ch <= '9')
        return ch - '0';
    else
        return toupper(ch) - 'A' + 10;
}

unsigned CxStringC::hexpack(unsigned char *binary, const char *string, const char *format)
{
    unsigned count = 0;
    char *ep;
    unsigned skip;

    while (format && *format)
    {
        while (*format && !isdigit(*format))
        {
            if (*(string++) != *(format++))
                return count;
            ++count;
        }
        if (isdigit(*format))
        {
            skip = (unsigned) strtol(format, &ep, 10);
            format = ep;
            count += skip * 2;
            while (skip--)
            {
                *(binary++) = hex(string[0]) * 16 + hex(string[1]);
                string += 2;
            }
        }
    }
    return count;
}

static const unsigned char alphabet[65] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

size_t CxStringC::b64encode(char *dest, const cx::uint8 *bin, size_t size, size_t dsize)
{
    assert(dest != nullptr && bin != nullptr);

    size_t count = 0;

    if (!dsize)
        dsize = (size * 4 / 3) + 1;

    if (!dsize || !size)
        goto end;

    unsigned bits;

    while (size >= 3 && dsize > 4)
    {
        bits = (((unsigned) bin[0]) << 16) | (((unsigned) bin[1]) << 8)
               | ((unsigned) bin[2]);
        bin += 3;
        size -= 3;
        count += 3;
        *(dest++) = alphabet[bits >> 18];
        *(dest++) = alphabet[(bits >> 12) & 0x3f];
        *(dest++) = alphabet[(bits >> 6) & 0x3f];
        *(dest++) = alphabet[bits & 0x3f];
        dsize -= 4;
    }

    if (size && dsize > 4)
    {
        bits = ((unsigned) bin[0]) << 16;
        *(dest++) = alphabet[bits >> 18];
        ++count;
        if (size == 1)
        {
            *(dest++) = alphabet[(bits >> 12) & 0x3f];
            *(dest++) = '=';
        }
        else
        {
            ++count;
            bits |= ((unsigned) bin[1]) << 8;
            *(dest++) = alphabet[(bits >> 12) & 0x3f];
            *(dest++) = alphabet[(bits >> 6) & 0x3f];
        }
        *(dest++) = '=';
    }

    end:
    *dest = 0;
    return count;
}

size_t CxStringC::b64decode(cx::uint8 *dest, const char *src, size_t size)
{
    char decoder[256];
    unsigned long bits;
    cx::uint8 c;
    unsigned i;
    size_t count = 0;

    for (i = 0; i < 256; ++i)
        decoder[i] = 64;

    for (i = 0; i < 64; ++i)
        decoder[alphabet[i]] = i;

    bits = 1;

    while (*src)
    {
        c = (cx::uint8) (*(src++));
        if (c == '=')
        {
            if (bits & 0x40000)
            {
                if (size < 2)
                    break;
                *(dest++) = (cx::uint8) ((bits >> 10) & 0xff);
                *(dest++) = (cx::uint8) ((bits >> 2) & 0xff);
                count += 2;
                break;
            }
            if ((bits & 0x1000) && size)
            {
                *(dest++) = (cx::uint8) ((bits >> 4) & 0xff);
                ++count;
            }
            break;
        }
        // end on invalid chars
        if (decoder[c] == 64)
            break;
        bits = (bits << 6) + decoder[c];
        if (bits & 0x1000000)
        {
            if (size < 3)
                break;
            *(dest++) = (cx::uint8) ((bits >> 16) & 0xff);
            *(dest++) = (cx::uint8) ((bits >> 8) & 0xff);
            *(dest++) = (cx::uint8) ((bits & 0xff));
            bits = 1;
            size -= 3;
            count += 3;
        }
    }
    return count;
}

const char *CxStringC::pos(const char *cp, signed long offset)
{
    if (!cp)
        return nullptr;

    size_t len = strlen(cp);
    if (!len)
        return cp;

    if (offset >= 0)
    {
        if ((size_t) offset > len)
            offset = len;
        return cp + offset;
    }

    offset = -offset;
    if ((size_t) offset >= len)
        return cp;

    return cp + len - offset;
}
