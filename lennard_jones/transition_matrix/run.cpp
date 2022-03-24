#include <iostream>
#include <string>
#include <memory>
#include <fstream>
#include "feasst.h"

using namespace feasst;

static feasst::ArgumentParse args("A grand canonical ensemble transition-matrix Monte Carlo simulation of a bulk Lennard Jones fluid.", {
  {"--num_procs", "number of processors", "12"},
  {"--num_hours", "number of hours before restart", "1."},
  {"--max_particles", "maximum number of particles", "370"},
  {"--temperature", "temperature", "1.5"},
  {"--mu", "chemical potential", "-2.352321"},
  {"--min_sweeps", "minimum number of TM sweeps before termination", "10"}});

std::shared_ptr<MonteCarlo> mc(int thread, int mn, int mx) {
  const std::string steps_per = str(1e6);
  auto mc = MakeMonteCarlo();
  mc->set(MakeRandomMT19937({{"seed", "71234567"}}));
  mc->add(MakeConfiguration({{"cubic_box_length", "8"},
                             {"particle_type", install_dir() + "/forcefield/lj.fstprt"}}));
  mc->add(MakePotential({{"Model", "LennardJones"}}));
  mc->add(MakePotential({{"VisitModel", "LongRangeCorrections"}}));
  mc->set(MakeThermoParams({{"beta", str(1./args.get_double("--temperature"))},
                            {"chemical_potential", args.get("--mu")}}));
  mc->set(MakeFlatHistogram(
        MakeMacrostateNumParticles(
            Histogram({{"width", "1"}, {"max", str(mx)}, {"min", str(mn)}})),
        MakeTransitionMatrix({{"min_sweeps", args.get("--min_sweeps")}})));
  mc->add(MakeTrialTranslate({{"weight", "1."}, {"tunable_param", "1."}}));
  mc->add(MakeTrialTransfer({{"weight", "4"}, {"particle_type", "0"}}));
  mc->add(MakeCheckEnergy({{"steps_per", steps_per}, {"tolerance", "0.0001"}}));
  mc->add(MakeTune({{"steps_per", steps_per}, {"stop_after_phase", "0"}}));
  mc->add(MakeLogAndMovie({{"steps_per", steps_per},
                           {"file_name", "clones" + str(thread)},
                           {"file_name_append_phase", "True"}}));
  mc->add(MakeEnergy({{"steps_per_write", steps_per},
                      {"file_name", "en" + str(thread) + ".txt"},
                      {"file_name_append_phase", "True"},
                      {"start_after_phase", "0"},
                      {"multistate", "True"}}));
  mc->add(MakeCriteriaUpdater({{"steps_per", steps_per}}));
  mc->add(MakeCriteriaWriter({{"steps_per", steps_per},
                              {"file_name", "clones" + str(thread) + "_crit.txt"},
                              {"file_name_append_phase", "True"}}));
  mc->set(MakeCheckpoint({{"file_name", "checkpoint" + str(thread) + ".fst"},
                          {"num_hours", str(0.1*args.get_int("--num_procs")*args.get_double("--num_hours"))},
                          {"num_hours_terminate", str(0.9*args.get_int("--num_procs")*args.get_double("--num_hours"))}}));
  return mc;
}

int main(int argc, char ** argv) {
  std::cout << version() << std::endl
            << args.parse(argc, argv) << std::endl;
  const auto windows = WindowExponential({
    {"alpha", "2.5"},
    {"num", args.get("--num_procs")},
    {"maximum", args.get("--max_particles")}}).boundaries();

  Clones clones;
  for (int proc = 0; proc < static_cast<int>(windows.size()); ++proc) {
      clones.add(mc(proc, windows[proc][0], windows[proc][1]));
  }
  clones.set(MakeCheckpoint({{"file_name", "checkpoint.fst"}}));
  clones.initialize_and_run_until_complete({{"ln_prob_file", "ln_prob.txt"},
                                            {"omp_batch", "1e6"}});

  // output the macrostate distribution and macrostate energies
  LnProbability ln_prob;
  std::vector<double> energy;
  ln_prob = clones.ln_prob(NULL, &energy, "Energy");
  std::ofstream file("ln_prob_energ.csv");
  file << "macro,ln_prob,energy" << std::endl;
  for (int macro = 0; macro < ln_prob.size(); ++macro) {
    file << macro << "," << ln_prob.value(macro) << "," << energy[macro] << std::endl;
  }
}
