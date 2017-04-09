/*
MIT License

Copyright (c) 2017 Cameron DaCamara

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <chrono>
#include <iostream>
#include <sstream>
#include <tuple>

#define INIT() tiny_bench::stopwatch sw_

#define START_MEASURE() sw_.start();

#define STOP_MEASURE_IMPL(msg) sw_.stop(); tiny_bench::log_time(msg, sw_.ticks())
#define STOP_MEASURE(msg) STOP_MEASURE_IMPL(msg)

// just escape every return value we see so the optimizer doesn't try to elminate the function call
#define MEASURE_IMPL(msg, ...) sw_.start(); tiny_bench::escape((__VA_ARGS__)); sw_.stop(); tiny_bench::log_time(#__VA_ARGS__ msg, sw_.ticks())
#define MEASURE(msg, ...) MEASURE_IMPL(msg, __VA_ARGS__)

#define MEASURE_EXPR_IMPL(msg, ...) sw_.start(); __VA_ARGS__; sw_.stop(); tiny_bench::log_time(#__VA_ARGS__ msg, sw_.ticks())
#define MEASURE_EXPR(msg, ...) MEASURE_EXPR_IMPL(msg, __VA_ARGS__)

#define SECTION(name) for (bool s = true; s; s = false, tiny_bench::measure_section_t(name))

namespace tiny_bench {

class stopwatch
{
public:
  typedef std::chrono::high_resolution_clock clock_t;

  stopwatch(): start_(clock_t::now()), stop_(start_) { }

  void start() { start_ = clock_t::now(); }
  void stop()  { stop_  = clock_t::now(); }

  clock_t::duration ticks() const { return stop_ - start_; }

  // helpers
  template <typename tick_t>
  tick_t to_ticks() const { return std::chrono::duration_cast<tick_t>(ticks()); }

  std::chrono::milliseconds to_ms() const { return to_ticks<std::chrono::milliseconds>(); }

private:
  clock_t::time_point start_;
  clock_t::time_point stop_;
};

std::vector<std::tuple<std::string, std::string, std::string, std::string>> g_measure_times;

template <typename T, typename TickT>
void log_time(const T& message, TickT tick) {
  using namespace std::chrono;

  g_measure_times.emplace_back();
  auto& entry = g_measure_times.back();

  std::stringstream ss;
  // storing (ns), (ms), (s)
  ss << duration_cast<nanoseconds>(tick).count();
  std::get<0>(entry) = ss.str();
  ss.str("");
  ss.clear();

  ss << duration_cast<milliseconds>(tick).count();
  std::get<1>(entry) = ss.str();
  ss.str("");
  ss.clear();

  ss << duration_cast<seconds>(tick).count();
  std::get<2>(entry) = ss.str();
  ss.str("");
  ss.clear();

  std::get<3>(entry) = message;
}

void pretty_print_log() {
  std::string headers[] = {
    "time (ns)",
    "time (ms)",
    "time (s)",
    "message"
  };

  std::size_t longest[] = {
    headers[0].size(),
    headers[1].size(),
    headers[2].size()
  };

  for (const auto& entry : g_measure_times) {
    longest[0] = std::max(longest[0], std::get<0>(entry).size());
    longest[1] = std::max(longest[1], std::get<1>(entry).size());
    longest[2] = std::max(longest[2], std::get<2>(entry).size());
  }

  auto sp_fill = [](const std::string& str, std::size_t rjust) {
    std::cout << str;
    rjust -= str.size();
    while (rjust-- > 0) std::cout.put(' ');
  };

  sp_fill(headers[0], longest[0]);
  std::cout << " | ";
  sp_fill(headers[1], longest[1]);
  std::cout << " | ";
  sp_fill(headers[2], longest[2]);
  std::cout << " | ";
  std::cout << headers[3];
  std::cout.put('\n');

  // entries
  for (const auto& entry : g_measure_times) {
    sp_fill(std::get<0>(entry), longest[0]);
    std::cout << " | ";
    sp_fill(std::get<1>(entry), longest[1]);
    std::cout << " | ";
    sp_fill(std::get<2>(entry), longest[2]);
    std::cout << " | ";
    std::cout << std::get<3>(entry);
    std::cout.put('\n');
  }

  g_measure_times.clear();
}

struct measure_section_t {
  std::string name;

  measure_section_t(std::string name): name(std::move(name)) {
    std::cout << "--- BEGIN " << this->name << " ---\n";
  }
  ~measure_section_t() {
    pretty_print_log();

    std::cout << "--- END " << name << " ---\n";
  }
};

// basically taken from facebook/folly
#ifdef _MSC_VER
#pragma optimize("", off)

inline void no_optimize(const void*) { }

#pragma optimize("", on)
#else
inline void no_optimize(const void* data) {
  asm volatile("" ::"m"(data) : "memory");
}
#endif // _MSC_VER

template <typename T>
inline void escape(T&& data) {
  no_optimize(&data);
}

} // namespace tiny_bench