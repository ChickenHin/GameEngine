#pragma once

#include <cstdint>
#include <string>
#include <chrono>
#include <fstream>

struct ProfileResult {
    std::string name;
    int64_t start, end;
    uint32_t threadID;
};

class Instrumentor {
public:
    static Instrumentor& Get();

    void BeginSession();
    void EndSession();
    void WriteProfile(const ProfileResult& result);

private:
    std::ofstream m_OutputStream;
    int32_t m_ProfileCount;
    int64_t m_SessionStartTime;
};

class Timer {
public:
    Timer(const char* name);
    ~Timer();

    void Stop();

private:
    const char* m_Name;
    std::chrono::time_point<std::chrono::steady_clock> m_StartTimepoint;
    bool m_Stopped;
};

#define PROFILER_BEGIN_SESSION() Instrumentor::Get().BeginSession()
#define PROFILER_END_SESSION() Instrumentor::Get().EndSession()

// __func__ (standard) or __PRETTY_FUNCTION__ (GCC/Clang specific for better template names)
#if defined(__GNUC__) || defined(__clang__)
    #define PROFILE_FUNCTION() Timer _(__PRETTY_FUNCTION__)
#elif defined(_MSC_VER )
    #define PROFILE_FUNCTION() Timer _(__FUNCSIG__)
#else
    #define PROFILE_FUNCTION() Timer _(__func__)
#endif

#define PROFILE_ZONE(name) Timer _(name)
