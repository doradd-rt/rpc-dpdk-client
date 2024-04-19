#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

#include "stats.h"

extern volatile bool force_quit;

class Manager {
  std::vector<Stats *> workers;

  double compute_throughput(uint64_t duration_milli) {
    /* std::cout << "(Duration is " << duration_milli << " ms) "; */
    uint64_t all_reqs = 0;
    for (Stats *w : workers) {
      all_reqs += w->get_req_count();
    }

    return (1000.0 * all_reqs) / duration_milli;
  }

  std::vector<double> compute_latency_percentiles() {
    // Put all latency samples in a new vector
    std::vector<uint64_t> all_samples(workers.size() * SAMPLES);
    uint64_t hz = rte_get_timer_hz();

    uint64_t idx = 0;
    for (Stats *w : workers) {
      auto *worker_samples = w->get_samples();
      for (uint32_t i = 0; i < SAMPLES; i++) {
        // Convert cycles to usec
        uint64_t lat = (worker_samples[i] * 1e6) / hz;
        all_samples[idx++] = lat;
      }
    }

    // 10, 50, 90, 95, 99
    std::vector<double> percentiles(5);
    percentiles[0] = Stats::percentile(all_samples, 10);
    percentiles[1] = Stats::percentile(all_samples, 50);
    percentiles[2] = Stats::percentile(all_samples, 90);
    percentiles[3] = Stats::percentile(all_samples, 95);
    percentiles[4] = Stats::percentile(all_samples, 99);

    return percentiles;
  }

public:
  Manager(std::vector<Stats *> w) : workers(w) {}

  int manager_main(uint32_t duration, FILE* log) {
    std::cout << "Warmup period: 1 sec\n";
    std::this_thread::sleep_for(std::chrono::seconds(1));

    for (Stats *w : workers) {
      w->start_measure();
    }
    auto start_time = std::chrono::system_clock::now();

    std::cout << "Experiment period: " << duration << " sec\n";
    std::this_thread::sleep_for(std::chrono::seconds(duration));

    std::cout << "Experiment end. Processing samples..." << std::endl;

    for (Stats *w : workers) {
      w->stop_load();
    }
    auto end_time = std::chrono::system_clock::now();

    uint64_t duration_milli =
        std::chrono::duration_cast<std::chrono::milliseconds>(end_time -
                                                              start_time)
            .count();
    std::cout << "Throughput is " << compute_throughput(duration_milli) << std::endl;

    std::cout << "Latency\n 10%\t50%\t90%\t95%\t99%\n";
    for (double p : compute_latency_percentiles())
      std::cout << p << "\t";
    std::cout << std::endl;

    fprintf(log, "%f %f\n", compute_throughput(duration_milli), compute_latency_percentiles()[4]);
    return 0;
  }
};
