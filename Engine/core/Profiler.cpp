
#include "Profiler.hpp"

#include <ctime>
#include <string>
#include <chrono>
#include <algorithm>
#include <fstream>
#include <thread>
#include <iostream>

Instrumentor& Instrumentor::Get() {
    static Instrumentor instance;
    return instance;
}

void Instrumentor::BeginSession()
{
    m_ProfileCount = 0;
    m_SessionStartTime = std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::steady_clock::now()).time_since_epoch().count();

    auto filepath = std::format("profile_{}.json", time(nullptr));

    m_OutputStream.open(filepath);
    m_OutputStream << '[';
}

void Instrumentor::EndSession() {
    m_OutputStream << ']';
    m_OutputStream.close();
}

void Instrumentor::WriteProfile(const ProfileResult& result) {

    if (m_ProfileCount++ > 0)
        m_OutputStream << ',';

    std::string name = result.name;
    std::replace(name.begin(), name.end(), '"', '\'');

    int64_t normalizedStart = result.start - m_SessionStartTime;

    // Write the "X" (Complete) event
    m_OutputStream << "{";
    m_OutputStream << "\"cat\":\"function\",";
    m_OutputStream << "\"dur\":" << (result.end - result.start) << ",";
    m_OutputStream << "\"name\":\"" << name << "\",";
    m_OutputStream << "\"ph\":\"X\",";
    m_OutputStream << "\"pid\":0,";
    m_OutputStream << "\"tid\":" << result.threadID << ",";
    m_OutputStream << "\"ts\":" << normalizedStart;
    m_OutputStream << "}";
}

Timer::Timer(const char* name) : m_Name(name), m_Stopped(false) {
    m_StartTimepoint = std::chrono::steady_clock::now();
}

Timer::~Timer() {
    if (!m_Stopped) Stop();
}

void Timer::Stop() {
    auto endTimepoint = std::chrono::steady_clock::now();

    int64_t start = std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTimepoint).time_since_epoch().count();
    int64_t end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch().count();

    uint32_t threadID = static_cast<uint32_t>(std::hash<std::thread::id>{}(std::this_thread::get_id()));

    Instrumentor::Get().WriteProfile({m_Name, start, end, threadID});

    m_Stopped = true;
}
