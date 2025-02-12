/**************************************************************************
 * Testing Perpendicular Laplacian inversion using PETSc solvers
 *
 **************************************************************************
 * Copyright 2013 J.T. Omotani
 *
 * Contact: Ben Dudson, bd512@york.ac.uk
 *
 * This file is part of BOUT++.
 *
 * BOUT++ is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * BOUT++ is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with BOUT++.  If not, see <http://www.gnu.org/licenses/>.
 *
 **************************************************************************/

#include <bout/bout.hxx>
#include <bout/constants.hxx>
// #include <bout/sys/timer.hxx>
#include <bout/boutexception.hxx>
#include <bout/invert_laplace.hxx>
#include <bout/options.hxx>
#include <cmath>

BoutReal max_error_at_ystart(const Field3D& error);

int main(int argc, char** argv) {

  BoutInitialise(argc, argv);
  {
    Options* options = Options::getRoot()->getSection("petsc2nd");
    auto invert = Laplacian::create(options);
    options = Options::getRoot()->getSection("petsc4th");
    auto invert_4th = Laplacian::create(options);

    // Solving equations of the form d*Delp2(f) + 1/c*Grad_perp(c).Grad_perp(f) + a*f = b for various f, a, c, d
    Field3D f1, a1, b1, c1, d1, sol1;
    BoutReal p, q; //Use to set parameters in constructing trial functions
    Field3D error1,
        absolute_error1; //Absolute value of relative error: abs( (f1-sol1)/f1 )
    BoutReal max_error1; //Output of test

    using bout::globals::mesh;

    // Only Neumann x-boundary conditions are implemented so far, so test functions should be Neumann in x and periodic in z.
    // Use Field3D's, but solver only works on FieldPerp slices, so only use 1 y-point
    BoutReal nx = mesh->GlobalNx - 2 * mesh->xstart - 1;
    BoutReal nz = mesh->GlobalNz;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Test 1: Gaussian x-profiles, 2nd order Krylov
    p = 0.39503274;
    q = 0.20974396;
    f1.allocate();
    for (int jx = mesh->xstart; jx <= mesh->xend; jx++)
      for (int jy = 0; jy < mesh->LocalNy; jy++)
        for (int jz = 0; jz < mesh->LocalNz; jz++) {
          BoutReal x = BoutReal(mesh->getGlobalXIndex(jx) - mesh->xstart) / nx;
          BoutReal z = BoutReal(jz) / nz;
          f1(jx, jy, jz) =
              0. + exp(-(100. * pow(x - p, 2) + 1. - cos(2. * PI * (z - q))))
              - 50.
                    * (2. * p * exp(-100. * pow(-p, 2)) * x
                       + (-p * exp(-100. * pow(-p, 2))
                          - (1 - p) * exp(-100. * pow(1 - p, 2)))
                             * pow(x, 2))
                    * exp(-(
                        1.
                        - cos(2. * PI
                              * (z - q)))) //make the gradients zero at both x-boundaries
              ;
          ASSERT0(finite(f1(jx, jy, jz)));
        }
    if (mesh->firstX())
      for (int jx = mesh->xstart - 1; jx >= 0; jx--)
        for (int jy = 0; jy < mesh->LocalNy; jy++)
          for (int jz = 0; jz < mesh->LocalNz; jz++) {
            BoutReal x = BoutReal(mesh->getGlobalXIndex(jx) - mesh->xstart) / nx;
            BoutReal z = BoutReal(jz) / nz;
            f1(jx, jy, jz) =
                0. + exp(-(60. * pow(x - p, 2) + 1. - cos(2. * PI * (z - q))))
                - 50.
                      * (2. * p * exp(-60. * pow(-p, 2)) * x
                         + (-p * exp(-60. * pow(-p, 2))
                            - (1 - p) * exp(-60. * pow(1 - p, 2)))
                               * pow(x, 2))
                      * exp(-(
                          1.
                          - cos(
                              2. * PI
                              * (z - q)))); //make the gradients zero at both x-boundaries
            ASSERT0(finite(f1(jx, jy, jz)));
          }
    if (mesh->lastX())
      for (int jx = mesh->xend + 1; jx < mesh->LocalNx; jx++)
        for (int jy = 0; jy < mesh->LocalNy; jy++)
          for (int jz = 0; jz < mesh->LocalNz; jz++) {
            BoutReal x = BoutReal(mesh->getGlobalXIndex(jx) - mesh->xstart) / nx;
            BoutReal z = BoutReal(jz) / nz;
            f1(jx, jy, jz) =
                0. + exp(-(60. * pow(x - p, 2) + 1. - cos(2. * PI * (z - q))))
                - 50.
                      * (2. * p * exp(-60. * pow(-p, 2)) * x
                         + (-p * exp(-60. * pow(-p, 2))
                            - (1 - p) * exp(-60. * pow(1 - p, 2)))
                               * pow(x, 2))
                      * exp(-(
                          1.
                          - cos(
                              2. * PI
                              * (z - q)))); //make the gradients zero at both x-boundaries
            ASSERT0(finite(f1(jx, jy, jz)));
          }

    f1.applyBoundary("neumann");

    p = 0.512547;
    q = 0.30908712;
    d1.allocate();
    for (int jx = mesh->xstart; jx <= mesh->xend; jx++)
      for (int jy = 0; jy < mesh->LocalNy; jy++)
        for (int jz = 0; jz < mesh->LocalNz; jz++) {
          BoutReal x = BoutReal(mesh->getGlobalXIndex(jx) - mesh->xstart) / nx;
          BoutReal z = BoutReal(jz) / nz;
          d1(jx, jy, jz) =
              1. + 0.2 * exp(-50. * pow(x - p, 2) / 4.) * sin(2. * PI * (z - q) * 3.);
        }
    if (mesh->firstX())
      for (int jx = mesh->xstart - 1; jx >= 0; jx--)
        for (int jy = 0; jy < mesh->LocalNy; jy++)
          for (int jz = 0; jz < mesh->LocalNz; jz++) {
            BoutReal x = BoutReal(mesh->getGlobalXIndex(jx) - mesh->xstart) / nx;
            BoutReal z = BoutReal(jz) / nz;
            d1(jx, jy, jz) =
                1. + 0.2 * exp(-50. * pow(x - p, 2) / 4.) * sin(2. * PI * (z - q) * 3.);
            // 	  d1(jx, jy, jz) = d1(jx+1, jy, jz);
          }
    if (mesh->lastX())
      for (int jx = mesh->xend + 1; jx < mesh->LocalNx; jx++)
        for (int jy = 0; jy < mesh->LocalNy; jy++)
          for (int jz = 0; jz < mesh->LocalNz; jz++) {
            BoutReal x = BoutReal(mesh->getGlobalXIndex(jx) - mesh->xstart) / nx;
            BoutReal z = BoutReal(jz) / nz;
            d1(jx, jy, jz) =
                1. + 0.2 * exp(-50. * pow(x - p, 2) / 4.) * sin(2. * PI * (z - q) * 3.);
            // 	  d1(jx, jy, jz) = d1(jx-1, jy, jz);
          }

    p = 0.18439023;
    q = 0.401089473;
    c1.allocate();
    for (int jx = mesh->xstart; jx <= mesh->xend; jx++)
      for (int jy = 0; jy < mesh->LocalNy; jy++)
        for (int jz = 0; jz < mesh->LocalNz; jz++) {
          BoutReal x = BoutReal(mesh->getGlobalXIndex(jx) - mesh->xstart) / nx;
          BoutReal z = BoutReal(jz) / nz;
          c1(jx, jy, jz) =
              1. + 0.15 * exp(-50. * pow(x - p, 2) * 2.) * sin(2. * PI * (z - q) * 2.);
        }
    if (mesh->firstX())
      for (int jx = mesh->xstart - 1; jx >= 0; jx--)
        for (int jy = 0; jy < mesh->LocalNy; jy++)
          for (int jz = 0; jz < mesh->LocalNz; jz++) {
            BoutReal x = BoutReal(mesh->getGlobalXIndex(jx) - mesh->xstart) / nx;
            BoutReal z = BoutReal(jz) / nz;
            c1(jx, jy, jz) =
                1. + 0.15 * exp(-50. * pow(x - p, 2) * 2.) * sin(2. * PI * (z - q) * 2.);
            // 	  c1(jx, jy, jz) = c1(jx+1, jy, jz);
          }
    if (mesh->lastX())
      for (int jx = mesh->xend + 1; jx < mesh->LocalNx; jx++)
        for (int jy = 0; jy < mesh->LocalNy; jy++)
          for (int jz = 0; jz < mesh->LocalNz; jz++) {
            BoutReal x = BoutReal(mesh->getGlobalXIndex(jx) - mesh->xstart) / nx;
            BoutReal z = BoutReal(jz) / nz;
            c1(jx, jy, jz) =
                1. + 0.15 * exp(-50. * pow(x - p, 2) * 2.) * sin(2. * PI * (z - q) * 2.);
            // 	  c1(jx, jy, jz) = c1(jx-1, jy, jz);
          }

    p = 0.612547;
    q = 0.30908712;
    a1.allocate();
    for (int jx = mesh->xstart; jx <= mesh->xend; jx++)
      for (int jy = 0; jy < mesh->LocalNy; jy++)
        for (int jz = 0; jz < mesh->LocalNz; jz++) {
          BoutReal x = BoutReal(mesh->getGlobalXIndex(jx) - mesh->xstart) / nx;
          BoutReal z = BoutReal(jz) / nz;
          a1(jx, jy, jz) =
              -1. + 0.1 * exp(-50. * pow(x - p, 2) * 2.5) * sin(2. * PI * (z - q) * 7.);
        }
    if (mesh->firstX())
      for (int jx = mesh->xstart - 1; jx >= 0; jx--)
        for (int jy = 0; jy < mesh->LocalNy; jy++)
          for (int jz = 0; jz < mesh->LocalNz; jz++) {
            BoutReal x = BoutReal(mesh->getGlobalXIndex(jx) - mesh->xstart) / nx;
            BoutReal z = BoutReal(jz) / nz;
            a1(jx, jy, jz) =
                -1. + 0.1 * exp(-50. * pow(x - p, 2) * 2.5) * sin(2. * PI * (z - q) * 7.);
            // 	  a1(jx, jy, jz) = a1(jx+1, jy, jz);
          }
    if (mesh->lastX())
      for (int jx = mesh->xend + 1; jx < mesh->LocalNx; jx++)
        for (int jy = 0; jy < mesh->LocalNy; jy++)
          for (int jz = 0; jz < mesh->LocalNz; jz++) {
            BoutReal x = BoutReal(mesh->getGlobalXIndex(jx) - mesh->xstart) / nx;
            BoutReal z = BoutReal(jz) / nz;
            a1(jx, jy, jz) =
                -1. + 0.1 * exp(-50. * pow(x - p, 2) * 2.5) * sin(2. * PI * (z - q) * 7.);
            // 	  a1(jx, jy, jz) = a1(jx-1, jy, jz);
          }

    checkData(f1);
    checkData(a1);
    checkData(c1);
    checkData(d1);

    mesh->communicate(f1, a1, c1, d1);

    b1 = d1 * Delp2(f1) + Grad_perp(c1) * Grad_perp(f1) / c1 + a1 * f1;

    if (mesh->firstX())
      for (int jx = mesh->xstart - 1; jx >= 0; jx--)
        for (int jy = 0; jy < mesh->LocalNy; jy++)
          for (int jz = 0; jz < mesh->LocalNz; jz++) {
            b1(jx, jy, jz) = b1(jx + 1, jy, jz);
          }
    if (mesh->lastX())
      for (int jx = mesh->xend + 1; jx < mesh->LocalNx; jx++)
        for (int jy = 0; jy < mesh->LocalNy; jy++)
          for (int jz = 0; jz < mesh->LocalNz; jz++) {
            b1(jx, jy, jz) = b1(jx - 1, jy, jz);
          }

    invert->setInnerBoundaryFlags(INVERT_AC_GRAD);
    invert->setOuterBoundaryFlags(INVERT_AC_GRAD);
    invert->setCoefA(a1);
    invert->setCoefC(c1);
    invert->setCoefD(d1);

    checkData(b1);

    try {
      sol1 = invert->solve(sliceXZ(b1, mesh->ystart));
      error1 = (f1 - sol1) / f1;
      absolute_error1 = f1 - sol1;
      //     max_error1 = max_error_at_ystart(abs(error1));
      max_error1 = max_error_at_ystart(abs(absolute_error1));
    } catch (BoutException& err) {
      output << "BoutException occured in invert->solve(b1): " << err.what() << endl;
      max_error1 = -1;
    }

    output << endl << "Test 1: PETSc 2nd order" << endl;
    //   output<<"Time to set up is "<<Timer::getTime("petscsetup")<<". Time to solve is "<<Timer::getTime("petscsolve")<<endl;
    //   output<<"Magnitude of maximum relative error is "<<max_error1<<endl;
    output << "Magnitude of maximum absolute error is " << max_error1 << endl;
    //   Timer::resetTime("petscsetup");
    //   Timer::resetTime("petscsolve");

    Options dump;
    dump["a1"] = a1;
    dump["b1"] = b1;
    dump["c1"] = c1;
    dump["d1"] = d1;
    dump["f1"] = f1;
    dump["sol1"] = sol1;
    dump["error1"] = error1;
    dump["absolute_error1"] = absolute_error1;
    dump["max_error1"] = max_error1;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Test 2: Gaussian x-profiles, 4th order Krylov
    Field3D sol2;
    Field3D error2,
        absolute_error2; //Absolute value of relative error: abs( (f3-sol3)/f3 )
    BoutReal max_error2; //Output of test

    invert_4th->setInnerBoundaryFlags(INVERT_AC_GRAD);
    invert_4th->setOuterBoundaryFlags(INVERT_AC_GRAD);
    invert_4th->setGlobalFlags(INVERT_4TH_ORDER);
    invert_4th->setCoefA(a1);
    invert_4th->setCoefC(c1);
    invert_4th->setCoefD(d1);

    try {
      sol2 = invert_4th->solve(sliceXZ(b1, mesh->ystart));
      error2 = (f1 - sol2) / f1;
      absolute_error2 = f1 - sol2;
      //     max_error2 = max_error_at_ystart(abs(error2));
      max_error2 = max_error_at_ystart(abs(absolute_error2));
    } catch (BoutException& err) {
      output << "BoutException occured in invert->solve(b1): " << err.what() << endl;
      max_error2 = -1;
    }

    output << endl << "Test 2: PETSc 4th order" << endl;
    //   output<<"Time to set up is "<<Timer::getTime("petscsetup")<<". Time to solve is "<<Timer::getTime("petscsolve")<<endl;
    //   output<<"Magnitude of maximum relative error is "<<max_error2<<endl;
    output << "Magnitude of maximum absolute error is " << max_error2 << endl;
    //   Timer::resetTime("petscsetup");
    //   Timer::resetTime("petscsolve");

    dump["a2"] = a1;
    dump["b2"] = b1;
    dump["c2"] = c1;
    dump["d2"] = d1;
    dump["f2"] = f1;
    dump["sol2"] = sol2;
    dump["error2"] = error2;
    dump["absolute_error2"] = absolute_error2;
    dump["max_error2"] = max_error2;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Test 3+4: Gaussian x-profiles, z-independent coefficients and compare with SPT method
    Field2D a3, c3, d3;
    Field3D b3;
    Field3D sol3, sol4;
    Field3D error3, absolute_error3, error4, absolute_error4;
    BoutReal max_error3, max_error4;

    a3 = DC(a1);
    c3 = DC(c1);
    d3 = DC(d1);
    b3 = d3 * Delp2(f1) + Grad_perp(c3) * Grad_perp(f1) / c3 + a3 * f1;
    if (mesh->firstX())
      for (int jx = mesh->xstart - 1; jx >= 0; jx--)
        for (int jy = 0; jy < mesh->LocalNy; jy++)
          for (int jz = 0; jz < mesh->LocalNz; jz++) {
            b3(jx, jy, jz) = b3(jx + 1, jy, jz);
          }
    if (mesh->lastX())
      for (int jx = mesh->xend + 1; jx < mesh->LocalNx; jx++)
        for (int jy = 0; jy < mesh->LocalNy; jy++)
          for (int jz = 0; jz < mesh->LocalNz; jz++) {
            b3(jx, jy, jz) = b3(jx - 1, jy, jz);
          }

    invert->setInnerBoundaryFlags(INVERT_AC_GRAD);
    invert->setOuterBoundaryFlags(INVERT_AC_GRAD);
    invert->setCoefA(a3);
    invert->setCoefC(c3);
    invert->setCoefD(d3);

    try {
      sol3 = invert->solve(sliceXZ(b3, mesh->ystart));
      error3 = (f1 - sol3) / f1;
      absolute_error3 = f1 - sol3;
      //     max_error3 = max_error_at_ystart(abs(error3));
      max_error3 = max_error_at_ystart(abs(absolute_error3));
    } catch (BoutException& err) {
      output << "BoutException occured in invert->solve(b3): " << err.what() << endl;
      max_error3 = -1;
    }

    output << endl << "Test 3: with coefficients constant in z, PETSc 2nd order" << endl;
    //   output<<"Time to set up is "<<Timer::getTime("petscsetup")<<". Time to solve is "<<Timer::getTime("petscsolve")<<endl;
    //   output<<"Magnitude of maximum relative error is "<<max_error3<<endl;
    output << "Magnitude of maximum absolute error is " << max_error3 << endl;
    //   Timer::resetTime("petscsetup");
    //   Timer::resetTime("petscsolve");

    dump["a3"] = a3;
    dump["b3"] = b3;
    dump["c3"] = c3;
    dump["d3"] = d3;
    dump["f3"] = f1;
    dump["sol3"] = sol3;
    dump["error3"] = error3;
    dump["absolute_error3"] = absolute_error3;
    dump["max_error3"] = max_error3;

    Options* SPT_options;
    SPT_options = Options::getRoot()->getSection("SPT");
    auto invert_SPT = Laplacian::create(SPT_options);
    invert_SPT->setInnerBoundaryFlags(INVERT_AC_GRAD);
    invert_SPT->setOuterBoundaryFlags(INVERT_AC_GRAD | INVERT_DC_GRAD);
    invert_SPT->setCoefA(a3);
    invert_SPT->setCoefC(c3);
    invert_SPT->setCoefD(d3);

    sol4 = invert_SPT->solve(sliceXZ(b3, mesh->ystart));
    error4 = (f1 - sol4) / f1;
    absolute_error4 = f1 - sol4;
    //   max_error4 = max_error_at_ystart(abs(error4));
    max_error4 = max_error_at_ystart(abs(absolute_error4));

    output << endl << "Test 4: with coefficients constant in z, default solver" << endl;
    //   output<<"Time to set up is "<<Timer::getTime("petscsetup")<<". Time to solve is "<<Timer::getTime("petscsolve")<<endl;
    //   output<<"Magnitude of maximum relative error is "<<max_error4<<endl;
    output << "Magnitude of maximum absolute error is " << max_error4 << endl;
    //   Timer::resetTime("petscsetup");
    //   Timer::resetTime("petscsolve");

    dump["a4"] = a3;
    dump["b4"] = b3;
    dump["c4"] = c3;
    dump["d4"] = d3;
    dump["f4"] = f1;
    dump["sol4"] = sol4;
    dump["error4"] = error4;
    dump["absolute_error4"] = absolute_error4;
    dump["max_error4"] = max_error4;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Test 5: Cosine x-profiles, 2nd order Krylov
    Field3D f5, a5, b5, c5, d5, sol5;
    Field3D error5,
        absolute_error5; //Absolute value of relative error: abs( (f5-sol5)/f5 )
    BoutReal max_error5; //Output of test

    p = 0.623901;
    q = 0.01209489;
    f5.allocate();
    for (int jx = mesh->xstart; jx <= mesh->xend; jx++)
      for (int jy = 0; jy < mesh->LocalNy; jy++)
        for (int jz = 0; jz < mesh->LocalNz; jz++) {
          BoutReal x = BoutReal(mesh->getGlobalXIndex(jx) - mesh->xstart) / nx;
          BoutReal z = BoutReal(jz) / nz;
          f5(jx, jy, jz) =
              0. + exp(-(50. * pow(x - p, 2) + 1. - cos(2. * PI * (z - q))))
              - 50.
                    * (2. * p * exp(-50. * pow(-p, 2)) * x
                       + (-p * exp(-50. * pow(-p, 2))
                          - (1 - p) * exp(-50. * pow(1 - p, 2)))
                             * pow(x, 2))
                    * exp(-(
                        1.
                        - cos(2. * PI
                              * (z - q)))) //make the gradients zero at both x-boundaries
              ;
        }
    if (mesh->firstX())
      for (int jx = mesh->xstart - 1; jx >= 0; jx--)
        for (int jy = 0; jy < mesh->LocalNy; jy++)
          for (int jz = 0; jz < mesh->LocalNz; jz++) {
            BoutReal x = BoutReal(mesh->getGlobalXIndex(jx) - mesh->xstart) / nx;
            BoutReal z = BoutReal(jz) / nz;
            f5(jx, jy, jz) =
                0. + exp(-(50. * pow(x - p, 2) + 1. - cos(2. * PI * (z - q))))
                - 50.
                      * (2. * p * exp(-50. * pow(-p, 2)) * x
                         + (-p * exp(-50. * pow(-p, 2))
                            - (1 - p) * exp(-50. * pow(1 - p, 2)))
                               * pow(x, 2))
                      * exp(-(
                          1.
                          - cos(
                              2. * PI
                              * (z - q)))); //make the gradients zero at both x-boundaries
          }
    if (mesh->lastX())
      for (int jx = mesh->xend + 1; jx < mesh->LocalNx; jx++)
        for (int jy = 0; jy < mesh->LocalNy; jy++)
          for (int jz = 0; jz < mesh->LocalNz; jz++) {
            BoutReal x = BoutReal(mesh->getGlobalXIndex(jx) - mesh->xstart) / nx;
            BoutReal z = BoutReal(jz) / nz;
            f5(jx, jy, jz) =
                0. + exp(-(50. * pow(x - p, 2) + 1. - cos(2. * PI * (z - q))))
                - 50.
                      * (2. * p * exp(-50. * pow(-p, 2)) * x
                         + (-p * exp(-50. * pow(-p, 2))
                            - (1 - p) * exp(-50. * pow(1 - p, 2)))
                               * pow(x, 2))
                      * exp(-(
                          1.
                          - cos(
                              2. * PI
                              * (z - q)))); //make the gradients zero at both x-boundaries
          }

    p = 0.63298589;
    q = 0.889237890;
    d5.allocate();
    for (int jx = mesh->xstart; jx <= mesh->xend; jx++)
      for (int jy = 0; jy < mesh->LocalNy; jy++)
        for (int jz = 0; jz < mesh->LocalNz; jz++) {
          BoutReal x = BoutReal(mesh->getGlobalXIndex(jx) - mesh->xstart) / nx;
          BoutReal z = BoutReal(jz) / nz;
          d5(jx, jy, jz) = 1. + p * cos(2. * PI * x) * sin(2. * PI * (z - q) * 3.);
        }
    if (mesh->firstX())
      for (int jx = mesh->xstart - 1; jx >= 0; jx--)
        for (int jy = 0; jy < mesh->LocalNy; jy++)
          for (int jz = 0; jz < mesh->LocalNz; jz++) {
            BoutReal x = BoutReal(mesh->getGlobalXIndex(jx) - mesh->xstart) / nx;
            BoutReal z = BoutReal(jz) / nz;
            d5(jx, jy, jz) = 1. + p * cos(2. * PI * x) * sin(2. * PI * (z - q) * 3.);
          }
    if (mesh->lastX())
      for (int jx = mesh->xend + 1; jx < mesh->LocalNx; jx++)
        for (int jy = 0; jy < mesh->LocalNy; jy++)
          for (int jz = 0; jz < mesh->LocalNz; jz++) {
            BoutReal x = BoutReal(mesh->getGlobalXIndex(jx) - mesh->xstart) / nx;
            BoutReal z = BoutReal(jz) / nz;
            d5(jx, jy, jz) = 1. + p * cos(2. * PI * x) * sin(2. * PI * (z - q) * 3.);
          }

    p = 0.160983834;
    q = 0.73050121087;
    c5.allocate();
    for (int jx = mesh->xstart; jx <= mesh->xend; jx++)
      for (int jy = 0; jy < mesh->LocalNy; jy++)
        for (int jz = 0; jz < mesh->LocalNz; jz++) {
          BoutReal x = BoutReal(mesh->getGlobalXIndex(jx) - mesh->xstart) / nx;
          BoutReal z = BoutReal(jz) / nz;
          c5(jx, jy, jz) = 1. + p * cos(2. * PI * x * 5) * sin(2. * PI * (z - q) * 2.);
        }
    if (mesh->firstX())
      for (int jx = mesh->xstart - 1; jx >= 0; jx--)
        for (int jy = 0; jy < mesh->LocalNy; jy++)
          for (int jz = 0; jz < mesh->LocalNz; jz++) {
            BoutReal x = BoutReal(mesh->getGlobalXIndex(jx) - mesh->xstart) / nx;
            BoutReal z = BoutReal(jz) / nz;
            c5(jx, jy, jz) = 1. + p * cos(2. * PI * x * 5) * sin(2. * PI * (z - q) * 2.);
          }
    if (mesh->lastX())
      for (int jx = mesh->xend + 1; jx < mesh->LocalNx; jx++)
        for (int jy = 0; jy < mesh->LocalNy; jy++)
          for (int jz = 0; jz < mesh->LocalNz; jz++) {
            BoutReal x = BoutReal(mesh->getGlobalXIndex(jx) - mesh->xstart) / nx;
            BoutReal z = BoutReal(jz) / nz;
            c5(jx, jy, jz) = 1. + p * cos(2. * PI * x * 5) * sin(2. * PI * (z - q) * 2.);
          }

    p = 0.5378950;
    q = 0.2805870;
    a5.allocate();
    for (int jx = mesh->xstart; jx <= mesh->xend; jx++)
      for (int jy = 0; jy < mesh->LocalNy; jy++)
        for (int jz = 0; jz < mesh->LocalNz; jz++) {
          BoutReal x = BoutReal(mesh->getGlobalXIndex(jx) - mesh->xstart) / nx;
          BoutReal z = BoutReal(jz) / nz;
          a5(jx, jy, jz) = -1. + p * cos(2. * PI * x * 2.) * sin(2. * PI * (z - q) * 7.);
        }
    if (mesh->firstX())
      for (int jx = mesh->xstart - 1; jx >= 0; jx--)
        for (int jy = 0; jy < mesh->LocalNy; jy++)
          for (int jz = 0; jz < mesh->LocalNz; jz++) {
            BoutReal x = BoutReal(mesh->getGlobalXIndex(jx) - mesh->xstart) / nx;
            BoutReal z = BoutReal(jz) / nz;
            a5(jx, jy, jz) =
                -1. + p * cos(2. * PI * x * 2.) * sin(2. * PI * (z - q) * 7.);
          }
    if (mesh->lastX())
      for (int jx = mesh->xend + 1; jx < mesh->LocalNx; jx++)
        for (int jy = 0; jy < mesh->LocalNy; jy++)
          for (int jz = 0; jz < mesh->LocalNz; jz++) {
            BoutReal x = BoutReal(mesh->getGlobalXIndex(jx) - mesh->xstart) / nx;
            BoutReal z = BoutReal(jz) / nz;
            a5(jx, jy, jz) =
                -1. + p * cos(2. * PI * x * 2.) * sin(2. * PI * (z - q) * 7.);
          }

    f5.applyBoundary("neumann");
    mesh->communicate(f5, a5, c5, d5);

    b5 = d5 * Delp2(f5) + Grad_perp(c5) * Grad_perp(f5) / c5 + a5 * f5;
    if (mesh->firstX())
      for (int jx = mesh->xstart - 1; jx >= 0; jx--)
        for (int jy = 0; jy < mesh->LocalNy; jy++)
          for (int jz = 0; jz < mesh->LocalNz; jz++) {
            b5(jx, jy, jz) = b5(jx + 1, jy, jz);
          }
    if (mesh->lastX())
      for (int jx = mesh->xend + 1; jx < mesh->LocalNx; jx++)
        for (int jy = 0; jy < mesh->LocalNy; jy++)
          for (int jz = 0; jz < mesh->LocalNz; jz++) {
            b5(jx, jy, jz) = b5(jx - 1, jy, jz);
          }

    invert->setInnerBoundaryFlags(INVERT_AC_GRAD);
    invert->setOuterBoundaryFlags(INVERT_AC_GRAD);
    invert->setCoefA(a5);
    invert->setCoefC(c5);
    invert->setCoefD(d5);

    try {
      sol5 = invert->solve(sliceXZ(b5, mesh->ystart));
      error5 = (f5 - sol5) / f5;
      absolute_error5 = f5 - sol5;
      //     max_error5 = max_error_at_ystart(abs(error5));
      max_error5 = max_error_at_ystart(abs(absolute_error5));
    } catch (BoutException& err) {
      output << "BoutException occured in invert->solve(b5): " << err.what() << endl;
      max_error5 = -1;
    }

    output << endl << "Test 5: different profiles, PETSc 2nd order" << endl;
    //   output<<"Time to set up is "<<Timer::getTime("petscsetup")<<". Time to solve is "<<Timer::getTime("petscsolve")<<endl;
    //   output<<"Magnitude of maximum relative error is "<<max_error5<<endl;
    output << "Magnitude of maximum absolute error is " << max_error5 << endl;
    //   Timer::resetTime("petscsetup");
    //   Timer::resetTime("petscsolve");

    dump["a5"] = a5;
    dump["b5"] = b5;
    dump["c5"] = c5;
    dump["d5"] = d5;
    dump["f5"] = f5;
    dump["sol5"] = sol5;
    dump["error5"] = error5;
    dump["absolute_error5"] = absolute_error5;
    dump["max_error5"] = max_error5;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Test 6: Cosine x-profiles, 4th order Krylov
    Field3D sol6;
    Field3D error6,
        absolute_error6; //Absolute value of relative error: abs( (f5-sol5)/f5 )
    BoutReal max_error6; //Output of test
    invert_4th->setInnerBoundaryFlags(INVERT_AC_GRAD);
    invert_4th->setOuterBoundaryFlags(INVERT_AC_GRAD);
    invert_4th->setGlobalFlags(INVERT_4TH_ORDER);
    invert_4th->setCoefA(a5);
    invert_4th->setCoefC(c5);
    invert_4th->setCoefD(d5);

    try {
      sol6 = invert_4th->solve(sliceXZ(b5, mesh->ystart));
      error6 = (f5 - sol6) / f5;
      absolute_error6 = f5 - sol6;
      //     max_error6 = max_error_at_ystart(abs(error6));
      max_error6 = max_error_at_ystart(abs(absolute_error6));
    } catch (BoutException& err) {
      output
          << "BoutException occured in invert->solve(b6): Laplacian inversion failed to "
             "converge (probably)"
          << endl;
      max_error6 = -1;
    }

    output << endl << "Test 6: different profiles, PETSc 4th order" << endl;
    //   output<<"Time to set up is "<<Timer::getTime("petscsetup")<<". Time to solve is "<<Timer::getTime("petscsolve")<<endl;
    //   output<<"Magnitude of maximum relative error is "<<max_error6<<endl;
    output << "Magnitude of maximum absolute error is " << max_error6 << endl;
    //   Timer::resetTime("petscsetup");
    //   Timer::resetTime("petscsolve");

    dump["a6"] = a5;
    dump["b6"] = b5;
    dump["c6"] = c5;
    dump["d6"] = d5;
    dump["f6"] = f5;
    dump["sol6"] = sol6;
    dump["error6"] = error6;
    dump["absolute_error6"] = absolute_error6;
    dump["max_error6"] = max_error6;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Test 7+8: Cosine x-profiles, z-independent coefficients and compare with SPT method
    Field2D a7, c7, d7;
    Field3D b7;
    Field3D sol7, sol8;
    Field3D error7, absolute_error7, error8, absolute_error8;
    BoutReal max_error7, max_error8;

    a7 = DC(a5);
    c7 = DC(c5);
    d7 = DC(d5);
    b7 = d7 * Delp2(f5) + Grad_perp(c7) * Grad_perp(f5) / c7 + a7 * f5;
    if (mesh->firstX())
      for (int jx = mesh->xstart - 1; jx >= 0; jx--)
        for (int jy = 0; jy < mesh->LocalNy; jy++)
          for (int jz = 0; jz < mesh->LocalNz; jz++) {
            b7(jx, jy, jz) = b7(jx + 1, jy, jz);
          }
    if (mesh->lastX())
      for (int jx = mesh->xend + 1; jx < mesh->LocalNx; jx++)
        for (int jy = 0; jy < mesh->LocalNy; jy++)
          for (int jz = 0; jz < mesh->LocalNz; jz++) {
            b7(jx, jy, jz) = b7(jx - 1, jy, jz);
          }

    invert->setInnerBoundaryFlags(INVERT_AC_GRAD);
    invert->setOuterBoundaryFlags(INVERT_AC_GRAD);
    invert->setCoefA(a7);
    invert->setCoefC(c7);
    invert->setCoefD(d7);

    try {
      sol7 = invert->solve(sliceXZ(b7, mesh->ystart));
      error7 = (f5 - sol7) / f5;
      absolute_error7 = f5 - sol7;
      //     max_error7 = max_error_at_ystart(abs(error7));
      max_error7 = max_error_at_ystart(abs(absolute_error7));
    } catch (BoutException& err) {
      output << "BoutException occured in invert->solve(b7): " << err.what() << endl;
      max_error7 = -1;
    }

    output
        << endl
        << "Test 7: different profiles, with coefficients constant in z, PETSc 2nd order"
        << endl;
    //   output<<"Time to set up is "<<Timer::getTime("petscsetup")<<". Time to solve is "<<Timer::getTime("petscsolve")<<endl;
    //   output<<"Magnitude of maximum relative error is "<<max_error7<<endl;
    output << "Magnitude of maximum absolute error is " << max_error7 << endl;
    //   Timer::resetTime("petscsetup");
    //   Timer::resetTime("petscsolve");

    dump["a7"] = a7;
    dump["b7"] = b7;
    dump["c7"] = c7;
    dump["d7"] = d7;
    dump["f7"] = f5;
    dump["sol7"] = sol7;
    dump["error7"] = error7;
    dump["absolute_error7"] = absolute_error7;
    dump["max_error7"] = max_error7;

    invert_SPT->setInnerBoundaryFlags(INVERT_AC_GRAD);
    invert_SPT->setOuterBoundaryFlags(INVERT_AC_GRAD | INVERT_DC_GRAD);
    invert_SPT->setCoefA(a7);
    invert_SPT->setCoefC(c7);
    invert_SPT->setCoefD(d7);

    sol8 = invert_SPT->solve(sliceXZ(b7, mesh->ystart));
    error8 = (f5 - sol8) / f5;
    absolute_error8 = f5 - sol8;
    //   max_error8 = max_error_at_ystart(abs(error8));
    max_error8 = max_error_at_ystart(abs(absolute_error8));

    output
        << endl
        << "Test 8: different profiles, with coefficients constant in z, default solver"
        << endl;
    //   output<<"Time to set up is "<<Timer::getTime("petscsetup")<<". Time to solve is "<<Timer::getTime("petscsolve")<<endl;
    //   output<<"Magnitude of maximum relative error is "<<max_error8<<endl;
    output << "Magnitude of maximum absolute error is " << max_error8 << endl;
    //   Timer::resetTime("petscsetup");
    //   Timer::resetTime("petscsolve");

    dump["a8"] = a7;
    dump["b8"] = b7;
    dump["c8"] = c7;
    dump["d8"] = d7;
    dump["f8"] = f5;
    dump["sol8"] = sol8;
    dump["error8"] = error8;
    dump["absolute_error8"] = absolute_error8;
    dump["max_error8"] = max_error8;

    // Write and close the output file
    bout::writeDefaultOutputFile(dump);

    MPI_Barrier(BoutComm::get()); // Wait for all processors to write data
  }

  bout::checkForUnusedOptions();

  BoutFinalise();
  return 0;
}

BoutReal max_error_at_ystart(const Field3D& error) {
  const auto* mesh = error.getMesh();
  BoutReal local_max_error = error(mesh->xstart, mesh->ystart, 0);

  for (int jx = mesh->xstart; jx <= mesh->xend; jx++)
    for (int jz = 0; jz < mesh->LocalNz; jz++)
      if (local_max_error < error(jx, mesh->ystart, jz))
        local_max_error = error(jx, mesh->ystart, jz);

  BoutReal max_error;

  MPI_Allreduce(&local_max_error, &max_error, 1, MPI_DOUBLE, MPI_MAX, BoutComm::get());

  return max_error;
}
