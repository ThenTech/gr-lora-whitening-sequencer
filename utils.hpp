#ifndef UTILS_HPP
#define UTILS_HPP

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <exception>

#include <stdarg.h>
#include <cxxabi.h>

#ifndef __cplusplus
#error A C++ compiler is required!
#endif

namespace Exceptions {
    /**
     *	\brief
     *		Generic exception with message.
     *	\param	msg
     *		The message to be shown when encountering this exception.
     */
    class Exception : public std::exception {
    private:
       std::string _msg;
    public:
        explicit Exception(const std::string& msg) : _msg(msg) {}
        virtual ~Exception() /*_GLIBCXX_USE_NOEXCEPT*/ {}

        virtual const std::string getMessage() const /*_GLIBCXX_USE_NOEXCEPT*/ {
            return "Exception: " + _msg;
        }

        virtual const std::string getMsg() const /*_GLIBCXX_USE_NOEXCEPT*/ {
            return _msg;
        }
    };

    /**
     *	\brief
     *		Casting exception with message.
     *	\param	src
     *		The object to be casted.
     *	\param	dest
     *		The object to cast to.
     */
    class CastingException : public Exception  {
    public:
        CastingException(const std::string& src, const std::string& dest)
            : Exception("Cannot cast \"" + src + "\" to object of type \"" + dest + "\"!") {}

        const std::string getMessage() const /*_GLIBCXX_USE_NOEXCEPT*/ {
            return "CastingException: " + Exception::getMsg();
        }
    };

    /**
     *	\brief
     *		File read exception.
     *	\param	name
     *		The name of the file to be read.
     */
    class FileReadException : public Exception  {
    public:
        FileReadException(const std::string& name)
            : Exception("Cannot read from file: " + name) {}

        const std::string getMessage() const /*_GLIBCXX_USE_NOEXCEPT*/ {
            return "FileReadException: " + Exception::getMsg();
        }
    };

    /**
     *	\brief
     *		File write exception.
     *	\param	name
     *		The name of the file to be written.
     */
    class FileWriteException : public Exception  {
    public:
        FileWriteException(const std::string& name)
            : Exception("Cannot write to file: " + name) {}

        const std::string getMessage() const /*_GLIBCXX_USE_NOEXCEPT*/ {
            return "FileWriteException: " + Exception::getMsg();
        }
    };
}

namespace std {
    /**	\brief	Trim whitespace from the start of the given string (in-place).
     *
     *	\param	s
     *		A reference to the string to perform the operation.
     */
    static inline void ltrim(std::string &s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(),
                std::not1(std::ptr_fun<int, int>(std::isspace))));
    }

    /**	\brief	Trim whitespace from the end of the given string (in-place).
     *
     *	\param	s
     *		A reference to the string to perform the operation.
     */
    static inline void rtrim(std::string &s) {
        s.erase(std::find_if(s.rbegin(), s.rend(),
                std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
    }

    /**	\brief	Trim whitespace from both start and end of the given string (in-place).
     *
     *	\param	s
     *		A reference to the string to perform the operation.
     */
    static inline void trim(std::string &s) {
        ltrim(s);
        rtrim(s);
    }

    /**	\brief	Trim whitespace from both start and end of the given string (copying).
     *
     *	\param	s
     *		A copy of the string to perform the operation.
     */
    static inline std::string trimmed(std::string s) {
        trim(s);
        return s;
    }

    /**	\brief	Erase everything up until the last occurrence of erase_to in the given string (in-place).
     *
     *	\param	str
     *		A reference to the string to perform the operation.
     *	\param	erase_to
     *		A reference to the string to look for.
     */
    static inline void strEraseToLast(string &str, const string& erase_to) {
        string::size_type pos = str.find_last_of(erase_to);
        if (pos < str.npos)
            str = str.substr(pos);
        //str = str.substr(str.find_last_of(erase_to));
    }

    /**	\brief	Erase everything up until the last occurrence of erase_to in the given string (copying).
     *
     *	\param	str
     *		A copy of the string to perform the operation.
     *	\param	erase_to
     *		A reference to the string to look for.
     */
    static inline std::string strErasedToLast(string str, const string& erase_to) {
        strEraseToLast(str, erase_to);
        return str;
    }

    /**	\brief	Transform the string contents to uppercase (within the current locale) (in-place).
     *
     *	\param	str
     *		A reference to the string to perform the operation.
     */
    static inline void strToUpper(std::string &str) {
        std::transform(str.begin(), str.end(), str.begin(),
            [](std::string::value_type ch) {
                return std::use_facet<std::ctype<std::string::value_type>>(std::locale()).toupper(ch);
            }
        );
    }

    /**	\brief	Transform the string contents to uppercase (within the current locale) (copying).
     *
     *	\param	str
     *		A copy of the string to perform the operation.
     */
    static inline std::string strToUppercase(std::string str) {
        strToUpper(str);
        return str;
    }

    /**	\brief	Replace all occurrences of from with to in the given std::string str.
     *
     *	\param	str
     *		A reference to the string to replace a substring.
     *	\param	from
     *		A reference to a string to replace.
     *	\param	to
     *		A reference to a string to replace with.
     */
    static inline void strReplaceAll(std::string &str, const std::string& from, const std::string& to) {
        size_t start_pos = 0;
        while((start_pos = str.find(from, start_pos)) != std::string::npos) {
            str.replace(start_pos, from.length(), to);
            start_pos += to.length();
        }
    }

    /**	\brief	Returns the internal actual class name of the given object o.
     *
     *	**Uses __abi::__cxa_demangle__ which is part of <cxxabi.h> included in all GCC compilers.**
     *
     *	If GCC is not used, type2name will revert to typeid(o).name() instead.
     *
     *	\tparam	T
     *		The type of object to get the name demangled from.
     *	\param	o
     *		The object to demangle the name from.
     *	\return
     *		Returns the class name of o.
     */
    template <class T>
    static inline const std::string type2name(T const& o) {
        #ifdef _CXXABI_H
            char *demang = abi::__cxa_demangle(typeid(o).name(), nullptr, nullptr, nullptr);
            std::string s(demang);
            std::free(demang);
        #else
            string s(typeid(o).name());
        #endif

        std::strReplaceAll(s, "std::", "");           // Remove std:: from output
        return s;
    }
}

namespace SysUtils {
    /**	\brief	C-like format the given args to the given buffer.
     *
     *		TO-DO:	The creation of buffer might be better if moved inside theis function instead.
     *				This would require to estimate the length of the final string though...
     *
     *	\param	buffer
     *		The character buffer to store the formatted data.
     *	\param	length
     *		The length of the allocated character buffer.
     *	\param	format
     *		The char* containing the way to format the args (like the first argument of printf()).
     *	\param	...
     *		A variable length list of arguments to format into the buffer (like the second argument of printf()).
     *	\return
     *		Returns an std::string withthe formatted data.
     */
    inline std::string stringFormat(char* buffer, size_t length, const char* format, ...) {
        va_list args;

        va_start(args, format);
        #ifdef _MSC_VER
            vsprintf_s(buffer, length, format, args);
        #else
            (void) length;
            vsprintf(buffer, format, args);
        #endif
        va_end(args);

        return std::string(buffer);
    }

    /**	\brief
     *		Convert the given char* to a variable of type T.
     *		Use this method instead of the raw C functions: atoi, atof, atol, atoll.
     *
     *	\tparam	T
     *		The type of object to cast to.
     *	\param	buffer
     *		The character buffer to convert.
     *	\return
     *		Returns a variable of type T with the value as given in buffer.
     */
    template <class T>
    inline T lexical_cast(const char* buffer) {
        T out;
        std::stringstream cast;

        if (std::strToUppercase(std::string(buffer)).substr(0, 2) == "0X")
                cast << std::hex << buffer;
        else	cast << buffer;

        if (!(cast >> out))
            throw Exceptions::CastingException(buffer, std::type2name(out));
        return out;
    }

    /**	\brief	Read the given file and return a pointer to a string containing its contents.
     *
     *	\param	filename
     *		The (path and) name of the file to read.
     *
     *	\return	Returns a string pointer.
     *
     *	\exception	FileReadException
     *		Throws FileReadException if the file could not be read properly.
     */
    const std::string* readStringFromFile(const std::string filename) {
        std::string *str = new std::string();
        std::fstream file(filename);

        try {
            file.seekg(0, std::ios::end);
            str->reserve((size_t) file.tellg());
            file.seekg(0, std::ios::beg);

            str->assign((std::istreambuf_iterator<char>(file)),
                         std::istreambuf_iterator<char>());
        } catch (...) {
            file.close();
            throw Exceptions::FileReadException(filename);
        }

        file.close();
        return str;
    }

    /**	\brief	Write the given string to the given file.
     *
     *	\param	filename
     *		The (path and) name of the file to write to (will be created if it does not exist).
     *	\param	str
     *		The string to write to a file.
     *
     *	\exception	FileWriteException
     *		Throws FileWriteException if the file could not be written properly.
     */
    void writeStringToFile(const std::string filename, const std::string& str) {
        std::ofstream file(filename);

        try {
            file << str;
        } catch (...) {
            file.close();
            throw Exceptions::FileWriteException(filename);
        }

        file.close();
    }
}


#endif // UTILS_HPP
