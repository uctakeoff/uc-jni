//
// Created by ushiyake on 2017/08/01.
//

#ifndef UC_ANDROID_LOG_HPP
#define UC_ANDROID_LOG_HPP
#include <iostream>
#include <sstream>
#include <type_traits>
#include <android/log.h>
#ifndef ANDROID_LOG_MIN_PRIORITY
#define ANDROID_LOG_MIN_PRIORITY ANDROID_LOG_VERBOSE
#endif

namespace uc {
namespace android {
    template <android_LogPriority Priority> struct is_enable_priority
    {
        static constexpr bool value = Priority >= ANDROID_LOG_MIN_PRIORITY;
    };

    template <android_LogPriority Priority> class log_stream
    {
    public:
        log_stream(const char* tags) : tags_(tags)
        {
        }
        ~log_stream()
        {
            __android_log_write(Priority, tags_, buf.str().c_str());
        }
        template<typename T> log_stream& operator<<(T const& val)
        {
            buf << val;
            return *this;
        }
    private:
        std::stringstream buf;
        const char* tags_;
    };

    class null_stream
    {
    public:
        null_stream(const char*){}
        template<typename T> null_stream& operator<<(T const&) { return *this; }
    };

    template <android_LogPriority Priority> using log = std::conditional_t<is_enable_priority<Priority>::value, log_stream<Priority>, null_stream>;
}
}

#define LOGD uc::android::log< ANDROID_LOG_DEBUG >(__func__) << "#" << __LINE__ << " : "

#endif
