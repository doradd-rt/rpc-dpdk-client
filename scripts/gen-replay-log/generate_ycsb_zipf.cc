#include "zipfian_random.h"
#include <cstdio>
#include <cstring>
#include <array>
#include <vector>
#include <algorithm>
#include <random>
#include <iterator>
#include <fstream>

#define ROW_COUNT  10'000'000
#define TX_COUNT   1'000'000 // 4M for Caracal
#define ROW_PER_TX 10
#define NrMSBContentionKey 6

using Rand = foedus::assorted::ZipfianRandom;

// Contention: 7 keys of 10 are chosen from 77 (2^7) set of 10M (2^24)
// nr_lsb is 17 (24-7) to mask 7 keys into a fixed space.
std::array<uint32_t, ROW_PER_TX> gen_keys(Rand* r, int contention)
{
  int nr_lsb = 63 - __builtin_clzll(ROW_COUNT) - NrMSBContentionKey;
  size_t mask = 0;
  if (nr_lsb > 0) mask = (1 << nr_lsb) - 1;

  int NrContKey = 0;
  if (contention) NrContKey = 7;

  std::array<uint32_t, ROW_PER_TX> keys;
  for (int i = 0; i < ROW_PER_TX; i++) {
 again:
    keys[i] = r->next() % ROW_COUNT;
    // if branch below skip the idx-0, i.e., the most contended key
    // since "& mask" will opt it out, thus keys are [1, 10M-1] 
    // To stay consistent w/ Caracal, we keep these here.
    if (i < NrContKey) {
      keys[i] &= ~mask;
    } else {
      if ((keys[i] & mask) == 0)
        goto again;
    }
    for (int j = 0; j < i; j++)
      if (keys[i] == keys[j])
        goto again;
  }

  return keys;
}

uint16_t gen_write_set(int contention) 
{
  // Set 0:10 read-write ratio for contented workloads
  if (contention)
    return static_cast<uint16_t>((1 << 10) - 1);

  std::vector<uint8_t> binDigits = {0, 0, 0, 0, 0, 0, 0, 0, 1, 1};
  uint16_t result = 0;
  std::random_device rd;
  std::mt19937 g(rd());
  std::shuffle(binDigits.begin(), binDigits.end(), g);

  for (uint8_t digit : binDigits)
    result = (result << 1) | digit;

  return result;
}

void gen_bin_txn(Rand* rand, std::ofstream* f, int contention)
{
  auto keys = gen_keys(rand, contention);
  auto ws   = gen_write_set(contention);
  int padding = 86;

  // pack
  for (const uint32_t& key : keys) 
  {
    for (size_t i = 0; i < sizeof(uint32_t); i++) 
    {
      uint8_t byte = (key >> (i * 8)) & 0xFF;
      f->write(reinterpret_cast<const char*>(&byte), sizeof(uint8_t));
    }
  }

  f->write(reinterpret_cast<const char*>(&ws), sizeof(uint16_t));
  
  for (int i = 0; i < padding; i++)
  {
    uint8_t zeroByte = 0x00;
    f->write(reinterpret_cast<const char*>(&zeroByte), sizeof(uint8_t));
  }
}

int main(int argc, char** argv) 
{
  // Validate minimum and maximum number of arguments
  if (argc < 5 || argc > 7)
  {
    fprintf(stderr, "Usage: ./program -d distribution [-s zipf_s] -c contention\n");
    return -1;
  }

  // Validate the -d flag and its value
  if (strcmp(argv[1], "-d") != 0)
  {
    fprintf(stderr, "Error: Missing or invalid -d flag\n");
    fprintf(stderr, "Usage: ./program -d distribution [-s zipf_s] -c contention\n");
    return -1;
  }

  // Parse the distribution type
  double zipf_s = 0; // Default for uniform
  bool is_zipfian = false;
  if (strcmp(argv[2], "zipfian") == 0)
  {
    is_zipfian = true;
    printf("Generating with zipfian distribution\n");

    // Check if the -s parameter is provided
    if (argc == 7 && strcmp(argv[3], "-s") == 0)
    {
      zipf_s = atof(argv[4]); // Convert to double
      if (zipf_s <= 0.0)
      {
        fprintf(stderr, "Error: zipf_s must be a positive value\n");
        return -1;
      }
      printf("Using custom zipf_s: %.2f\n", zipf_s);
    } 
    else if (argc == 5)
    {
      zipf_s = 0.9; // Default zipf_s
      printf("Using default zipf_s: 0.9\n");
    }
    else
    {
      fprintf(stderr, "Usage: ./program -d distribution [-s zipf_s] -c contention\n");
      return -1;
    }
  }
  else if (strcmp(argv[2], "uniform") == 0)
  {
    printf("Generating with uniform distribution\n");
    zipf_s = 0; // No additional parameter needed
  }
  else
  {
    fprintf(stderr, "Error: Distribution should be either 'uniform' or 'zipfian'\n");
    return -1;
  }

  // Validate the -c flag
  const char* contention_arg = (argc == 7 && strcmp(argv[3], "-s") == 0) ? argv[6] : argv[4];
  if (strcmp(contention_arg, "cont") != 0 && strcmp(contention_arg, "no_cont") != 0)
  {
    fprintf(stderr, "Error: Missing or invalid contention argument. Must be 'cont' or 'no_cont'\n");
    fprintf(stderr, "Usage: ./program -d distribution [-s zipf_s] -c contention\n");
    return -1;
  }

  int contention = (strcmp(contention_arg, "cont") == 0) ? 1 : 0;
  printf("Generating with %s accesses\n", contention ? "contended" : "no contended");

  // Initialize random generator
  Rand rand;
  rand.init(ROW_COUNT, zipf_s, 1238);

  // Generate log file name
  char log_name[50];
  if (is_zipfian) 
  {
    snprintf(log_name, sizeof(log_name), "ycsb_%s_%.2f_%s.txt", argv[2], zipf_s, contention ? "cont" : "no_cont");
  } 
  else 
  {
    snprintf(log_name, sizeof(log_name), "ycsb_%s_%s.txt", argv[2], contention ? "cont" : "no_cont");
  }

  // Open output log file
  std::ofstream outLog(log_name, std::ios::binary);
  if (!outLog)
  {
    fprintf(stderr, "Error: Unable to open output log file: %s\n", log_name);
    return -1;
  }

  // Write transaction count to the log
  uint32_t count = TX_COUNT;
  outLog.write(reinterpret_cast<const char*>(&count), sizeof(uint32_t));

  // Generate transactions
  for (int i = 0; i < TX_COUNT; i++)
  {
    gen_bin_txn(&rand, &outLog, contention);
  }

  outLog.close();
  printf("Log file generated: %s\n", log_name);

  return 0;
}
