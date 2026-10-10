#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include <vector>

#include "sim.h"
#include "MemoryHierarchy.h"
#include "SimulationResults.h"

int main (int argc, char *argv[]) {
   FILE *fp;			// File pointer.
   char *trace_file;		// This variable holds the trace file name.
   cache_params_t params;	// Look at the sim.h header file for the definition of struct cache_params_t.
   char rw;			// This variable holds the request's type (read or write) obtained from the trace.
   uint32_t addr;		// This variable holds the request's address obtained from the trace.

   if (argc != 9) {
      printf("Error: Expected 8 command-line arguments but was provided %d.\n", (argc - 1));
      exit(EXIT_FAILURE);
   }

   params.BLOCKSIZE = (uint32_t) atoi(argv[1]);
   params.L1_SIZE   = (uint32_t) atoi(argv[2]);
   params.L1_ASSOC  = (uint32_t) atoi(argv[3]);
   params.L2_SIZE   = (uint32_t) atoi(argv[4]);
   params.L2_ASSOC  = (uint32_t) atoi(argv[5]);
   params.PREF_N    = (uint32_t) atoi(argv[6]);
   params.PREF_M    = (uint32_t) atoi(argv[7]);
   trace_file       = argv[8];

   // Open the trace file for reading.
   fp = fopen(trace_file, "r");
   if (fp == (FILE *) NULL) {
      // Exit with an error if file open failed.
      printf("Error: Unable to open file %s\n", trace_file);
      exit(EXIT_FAILURE);
   }

   // Print simulator configuration.
   printf("===== Simulator configuration =====\n");
   printf("BLOCKSIZE:  %u\n", params.BLOCKSIZE);
   printf("L1_SIZE:    %u\n", params.L1_SIZE);
   printf("L1_ASSOC:   %u\n", params.L1_ASSOC);
   printf("L2_SIZE:    %u\n", params.L2_SIZE);
   printf("L2_ASSOC:   %u\n", params.L2_ASSOC);
   printf("PREF_N:     %u\n", params.PREF_N);
   printf("PREF_M:     %u\n", params.PREF_M);
   printf("trace_file: %s\n", trace_file);
   printf("\n");

   std::vector<CacheConfig> configs;

   CacheConfig l1;
   l1.blockSize    = params.BLOCKSIZE;
   l1.associativity = params.L1_ASSOC;
   l1.numSets       = params.L1_SIZE / (params.BLOCKSIZE * params.L1_ASSOC);
   configs.push_back(l1);

   if (params.L2_SIZE > 0) {
      CacheConfig l2;
      l2.blockSize     = params.BLOCKSIZE;
      l2.associativity = params.L2_ASSOC;
      l2.numSets       = params.L2_SIZE / (params.BLOCKSIZE * params.L2_ASSOC);
      configs.push_back(l2);
   }

   if (params.L2_SIZE > 0) {
      configs[1].prefN = params.PREF_N;
      configs[1].prefM = params.PREF_M;
   } else {
      configs[0].prefN = params.PREF_N;
      configs[0].prefM = params.PREF_M;
   }

   MemoryHierarchy hierarchy(configs);

   // Read requests from the trace file and issue each one to the hierarchy.
   while (fscanf(fp, "%c %x\n", &rw, &addr) == 2) {
      if (rw != 'r' && rw != 'w') {
         printf("Error: Unknown request type %c.\n", rw);
         exit(EXIT_FAILURE);
      }

      hierarchy.access(addr, rw);
   }

   fclose(fp);

   // Gather the final counters from every level and report.
   std::vector<AccessStats> perLevel;
   for (int i = 0; i < (int) configs.size(); i++) {
      perLevel.push_back(hierarchy.getLevelStats(i));
   }

   hierarchy.printAllContents();
   printf("===== Measurements =====\n");
   SimulationResults results(perLevel);
   results.print();

   return(0);
}