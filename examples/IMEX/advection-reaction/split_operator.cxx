/*************************************************************
 * Advection-Reaction equation
 *
 * Split into advective and reaction parts. Can be simulated
 * using unsplit methods (the two parts are just combined),
 * but intended for testing split schemes
 *
 * Currently one of the RHS functions has to be called physics_run,
 * so here physics_run contains advection term only.
 *
 * Grid file simple_xz.nc contains:
 * - nx = 68
 * - ny = 5
 * - dx = 1. / 64   so X domain has length 1
 *
 * In BOUT.inp:
 * - Domain is set to periodic in X
 * - The Z domain is set to size 1 (1 / 2*pi th of a torus)
 *
 *************************************************************/

#include <bout.hxx>
#include <bout/physicsmodel.hxx>
#include <initialprofiles.hxx>

class Split_operator : public PhysicsModel {
protected:
  int init(bool UNUSED(restarting)) override;
  int convective(BoutReal UNUSED(time)) override;
  int diffusive(BoutReal UNUSED(time)) override;
};




Field3D U;   // Evolving variable

Field3D phi; // Potential used for advection

BoutReal rate; // Reaction rate

int Split_operator::init(bool UNUSED(restarting)) {
  // Give the solver two RHS functions
  setSplitOperator(true);
  
  // Get options
  auto globalOptions = Options::root();
  auto options = globalOptions["split"];
  rate = options["rate"].withDefault(1.0);

  // Get phi settings from BOUT.inp
  phi.setBoundary("phi");
  initial_profile("phi", phi);
  phi.applyBoundary();
  
  // Save phi to file for reference
  SAVE_ONCE(phi);

  // Just solving one variable, U
  SOLVE_FOR(U);

  return 0;
}

int Split_operator::convective(BoutReal UNUSED(time)) {
  // Need communication
  U.getMesh()->communicate(U);

  // Form of advection operator for reduced MHD type models
  ddt(U) = -bracket(phi, U, BRACKET_SIMPLE);
  
  return 0;
}

int Split_operator::diffusive(BoutReal UNUSED(time)) {
  // A simple reaction operator. No communication needed
  ddt(U) = rate * (1.-U);

  return 0;
}


BOUTMAIN(Split_operator)
