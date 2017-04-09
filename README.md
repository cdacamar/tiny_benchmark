## What is tiny_benchmark?

`tiny_benchmark` is just a simple C++ benchmarking lib I use for approximating run times of various data structures I'm testing

## How to use

```cpp
#include <vector>

#include <tiny_benchmark.h>

int main() {
  INIT(); // initialize the library

  SECTION("BENCHMARK std::vector<int> stuff...")
  {
    // benchmark ctor
    MEASURE_EXPR(" ctor std::vector<int>", std::vector<int> v);

    MEASURE("push_back(1)", v.push_back(1), v); // since push_back doesn't return anything we want to escape the vector itself
    MEASURE("emplace_back(1)", v.emplace_back(1), v);

    START_MEASURE();
    v.push_back(2); v.push_back(3); v.push_back(4);
    STOP_MEASURE("inserting 3 elements back-to-back");

    MEASURE_EXPR("inserting 1000 elements",
    for (int i = 0;i != 1000;++i) {
      v.push_back(i);

      // so the optimizer doesn't blow up our variable
      tiny_bench::escape(v.data());
    });
  }
}
```

---

### Disclaimer

`tiny_benchmark` is by no means a production library and shouldn't really be used by anyone :joy: