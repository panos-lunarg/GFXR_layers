#include <cstring>
#include <assert.h>

#if defined(__ANDROID__)
#include <android/log.h>
#else
#include <stdio.h>
#endif

namespace base_layer
{

std::string ConvertFormatVaListToString(const std::string& format_string, va_list& var_args)
{
    va_list var_args_copy;
    va_copy(var_args_copy, var_args);
    try
    {
        // Determine how much space is needed in the new string
        const int32_t sz = std::vsnprintf(nullptr, 0, format_string.c_str(), var_args) + 1;

        // Create a result string and clear it with spaces and then copy the formatted
        // string results into it.
        std::string result_string(sz, ' ');
        std::vsnprintf(&result_string.front(), sz, format_string.c_str(), var_args_copy);
        va_end(var_args_copy);
        return result_string;
    }
    catch (...)
    {
        va_end(var_args_copy);
        return "";
    }
}

void base_layer_print_info(const char* format, ...)
{
    va_list args;
    va_start(args, format);

    const std::string printf_msg = ConvertFormatVaListToString(format, args);

#if defined(__ANDROID__)
    __android_log_print(ANDROID_LOG_INFO, "[" LAYER_NAME "]", "%s", printf_msg.c_str());
#else
    printf("[" LAYER_NAME "] ");
    fprintf(stdout, "%s", printf_msg.c_str());
#endif

    va_end(args);
}

void base_layer_print_error(const char* format, ...)
{
    va_list args;
    va_start(args, format);

    const std::string printf_msg = ConvertFormatVaListToString(format, args);

#if defined(__ANDROID__)
    __android_log_print(ANDROID_LOG_ERROR, "[" LAYER_NAME "]", "%s", printf_msg.c_str());
#else
    fprintf(stderr, "[" LAYER_NAME "] ");
    fprintf(stderr, "%s", printf_msg.c_str());
#endif

    va_end(args);
}

} // namespace base_layer
