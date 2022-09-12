#pragma once

#include <Eigen/Core>

#include "CplxFunc.h"
#include "mpi_utils.h"
#include "trees/MultiResolutionAnalysis.h"
#include <map>
#include <vector>

#ifdef MRCPP_HAS_MPI
#include <mpi.h>
#else
using MPI_Comm = int;
#endif

// define a class for things that can be sent with MPI

template <int D> class MultiResolutionAnalysis;

using namespace Eigen;

using IntVector = Eigen::VectorXi;
using DoubleVector = Eigen::VectorXd;
using ComplexVector = Eigen::VectorXcd;

using IntMatrix = Eigen::MatrixXi;
using DoubleMatrix = Eigen::MatrixXd;
using ComplexMatrix = Eigen::MatrixXcd;

namespace mrcpp {

namespace omp {
extern int n_threads;
} // namespace omp

class Bank;
extern Bank dataBank;

namespace mpi {

extern std::vector<int> bankmaster;

void initialize();
void finalize();
void barrier(MPI_Comm comm);

bool grand_master();
bool share_master();
bool my_orb(int j);
bool my_orb(CplxFunc orbj);

// bool my_unique_orb(const Orbital &orb);
void free_foreign(MPI_FuncVector &Phi);

void send_function(CplxFunc &func, int dst, int tag, MPI_Comm comm = mpi::comm_wrk);
void recv_function(CplxFunc &func, int src, int tag, MPI_Comm comm = mpi::comm_wrk);
void share_function(CplxFunc &func, int src, int tag, MPI_Comm comm);

void reduce_function(double prec, CplxFunc &func, MPI_Comm comm);
void broadcast_function(CplxFunc &func, MPI_Comm comm);

void reduce_Tree_noCoeff(mrcpp::FunctionTree<3> &tree, MPI_Comm comm);
void allreduce_Tree_noCoeff(mrcpp::FunctionTree<3> &tree, std::vector<CplxFunc> &Phi, MPI_Comm comm);
void broadcast_Tree_noCoeff(mrcpp::FunctionTree<3> &tree, MPI_Comm comm);

void allreduce_vector(IntVector &vec, MPI_Comm comm);
void allreduce_vector(DoubleVector &vec, MPI_Comm comm);
void allreduce_vector(ComplexVector &vec, MPI_Comm comm);
void allreduce_matrix(IntMatrix &vec, MPI_Comm comm);
void allreduce_matrix(DoubleMatrix &mat, MPI_Comm comm);
void allreduce_matrix(ComplexMatrix &mat, MPI_Comm comm);

} // namespace mpi

} // namespace mrcpp
