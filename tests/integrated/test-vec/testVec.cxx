
#include <bout/physicsmodel.hxx>
#include <vecops.hxx>         // Gives the vec diff options

class VecTest : public PhysicsModel {
protected:
  int init(bool restarting);
  int rhs(BoutReal t);

public:
  Field3D n;
  Vector3D gradPerpN;
  string ownOpType;
};


int VecTest::init(bool UNUSED(restarting)) {
  TRACE("Halt in VecTest::init");
  SOLVE_FOR(n);
  return 0;
}

int VecTest::rhs(BoutReal UNUSED(t)) {
  TRACE("Halt in VecTest::rhs");
  mesh->communicate(n);
  gradPerpN = Grad_perp(n);
  mesh->communicate(gradPerpN);
  ddt(n) = 0;
  return 0;
}

BOUTMAIN(VecTest);
