#include <iostream>
#include <string>
#include <memory>
#include <fstream>
#include "feasst.h"

using namespace feasst;

void compute(const std::string xyz_file,
  const std::string out_file,
  const double xy = 0,
  const double xz = 0,
  const double yz = 0) {
  System system;
  system.add(MakeConfiguration({{"particle_type0", install_dir() + "/forcefield/lj.fstprt"},
    {"xyz_file", xyz_file}, {"xy", str(xy)}, {"xz", str(xz)}, {"yz", str(yz)}}));
  system.add(MakePotential(MakeLennardJones()));
  system.add(MakePotential(MakeLongRangeCorrections()));
  system.energy();
  std::ofstream file(out_file, std::fstream::app);
  file << xyz_file << ","
       << MAX_PRECISION
       << system.stored_energy() << ","
       << system.stored_energy_profile()[0] << ","
       << system.stored_energy_profile()[1] << std::endl;
}

int main(int argc, char ** argv) {
  std::cout << version() << std::endl;
  std::string out_file = "results.csv";
  std::ofstream file(out_file);
  file << "xyz,u_total,u_lj,u_lrc" << std::endl;
  file.close();
  compute("lj_sample_config_periodic4.xyz", out_file);
  compute("lj_triclinic_sample_config_periodic3.xyz", out_file,
          1.7364817766693041,
          2.5881904510252074,
          0.42863479791864567);
}
