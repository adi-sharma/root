/// \file concurrentfill.cxx
/// \ingroup Tutorials
/// \author Axel Naumann <axel@cern.ch>
/// \date 2015-07-09
/// \warning This is part of the ROOT 7 prototype! It will change without notice. It might trigger earthquakes. Feedback is welcome!

/*************************************************************************
 * Copyright (C) 1995-2015, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#include "ROOT/THist.h"
#include "ROOT/THistConcurrentFill.h"

#include <iostream>
#include <future>
#include <random>

using namespace Experimental;

double wasteCPUTime(std::mt19937& gen) {
  // Simulate number crunching through gen and ridiculous num bits
  return std::generate_canonical<double, 100>(gen)
     + std::generate_canonical<double, 100>(gen)
     + std::generate_canonical<double, 100>(gen)
     + std::generate_canonical<double, 100>(gen)
     + std::generate_canonical<double, 100>(gen);
}

using Filler_t = Experimental::THistConcurrentFiller<Experimental::TH2D, 1024>;

/// This function is called within each thread: it spends some CPU time and then
/// fills a number into the histogram, through the Filler_t. This is repeated
/// several times.
void theTask(Filler_t filler) {
  std::mt19937 gen;

  for (int i = 0;  i < 3000000; ++i)
    filler.Fill({wasteCPUTime(gen), wasteCPUTime(gen)});
}

/// This example fills a histogram concurrently, from several threads.
void concurrentHistFill(Experimental::TH2D& hist) {
  // THistConcurrentFillManager allows multiple threads to fill the histogram
  // concurrently.
  //
  // Details: each thread's Fill() calls are buffered. once the buffer is full,
  // the THistConcurrentFillManager locks and flushes the buffer into the
  // histogram.
  Experimental::THistConcurrentFillManager<Experimental::TH2D> fillMgr(hist);

  std::array<std::thread, 8> threads;

  // Let the threads fill the histogram concurrently.
  for (auto& thr: threads) {
    // Each thread calls fill(), passing a dedicated filler per thread.
    thr = std::thread(theTask, fillMgr.MakeFiller());
  }


  // Join them.
  for (auto& thr: threads)
    thr.join();
}

void concurrentfill() {
  // This histogram will be filled from several threads.
  Experimental::TH2D hist{{100, 0., 1.}, {{0., 1., 2., 3.,10.}}};

  concurrentHistFill(hist);

  std::cout << hist.GetEntries() << '\n';
}
