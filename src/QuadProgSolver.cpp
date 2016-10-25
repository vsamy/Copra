//This file is part of ModelPreviewController.

//ModelPreviewController is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.
//
//ModelPreviewController is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.
//
//You should have received a copy of the GNU Lesser General Public License
//along with ModelPreviewController.  If not, see <http://www.gnu.org/licenses/>.

#include "QuadProgSolver.h"
#include <iostream>

namespace mpc
{

/*
 * QuadProg dense
 */

QuadProgDenseSolver::QuadProgDenseSolver()
    : solver_(std::make_unique<Eigen::QuadProgDense>())
{
}

int QuadProgDenseSolver::SI_fail() const
{
    return solver_->fail();
}

int QuadProgDenseSolver::SI_iter() const
{
    return solver_->iter()(0);
}

void QuadProgDenseSolver::SI_inform() const
{
    switch (solver_->fail())
    {
    case 0:
        std::cout << "No problems" << std::endl;
        break;
    case 1:
        std::cout << "The minimization problem has no solution" << std::endl;
        break;
    case 2:
        std::cout << "Problems with the decomposition of Q (Is it symmetric?)"
                  << std::endl;
        break;
    }
}

const Eigen::VectorXd &QuadProgDenseSolver::SI_result() const
{
    return solver_->result();
}

void QuadProgDenseSolver::SI_problem(int nrVar, int nrEq, int nrInEq)
{
    // QuadProg does not have bound constrains.
    // They will be transformed into inequality constrains.
    // The bound limits constrain size is 2 times (upper and lower bound) the
    // number of variables
    solver_->problem(nrVar, nrEq, nrInEq + 2 * nrVar);
}

bool QuadProgDenseSolver::SI_solve(
    const Eigen::MatrixXd &Q, const Eigen::VectorXd &C,
    const Eigen::MatrixXd &Aeq, const Eigen::VectorXd &Beq,
    const Eigen::MatrixXd &Aineq, const Eigen::VectorXd &Bineq,
    const Eigen::VectorXd &XL, const Eigen::VectorXd &XU)
{
    auto nrLines = XL.rows();
    auto ALines = Aineq.rows();
    Eigen::MatrixXd I = Eigen::MatrixXd::Identity(nrLines, nrLines);
    Eigen::MatrixXd ineqMat(ALines + 2 * nrLines, Aineq.cols());
    Eigen::VectorXd ineqVec(ALines + 2 * nrLines);
    ineqMat.topRows(ALines) = Aineq;
    ineqMat.block(ALines, 0, nrLines, nrLines) = I;
    ineqMat.bottomRows(nrLines) = -I;
    ineqVec.head(ALines) = Bineq;
    ineqVec.segment(ALines, nrLines) = XU;
    ineqVec.tail(nrLines) = -XL;

    return solver_->solve(Q, C, Aeq, Beq, ineqMat, ineqVec);
}

} // namespace pc