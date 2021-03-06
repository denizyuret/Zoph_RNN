//CUDA utilility for LSTM RNN

#ifndef BZ_CUDA_UTIL_H
#define BZ_CUDA_UTIL_H

#include <stdlib.h>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_real.hpp>
#include <Eigen/Dense>
#include <cmath>
#include <stdio.h>   
#include <stdlib.h>

#include "cublas_v2.h"
#include <cuda_runtime.h>
#include <thrust/device_vector.h>
#include <thrust/device_ptr.h>
#include <thrust/transform.h>
#include <thrust/iterator/constant_iterator.h>
#include "cuda_profiler_api.h"

//This is used since all cuBLAS storage is column major
#define IDX2C(i,j,ld) (((j)*(ld))+(i))

namespace BZ_CUDA {
boost::random::mt19937 gen;
double lower = -0.08;
double upper = 0.08;
}
#define UNCONST(t,c,uc) Eigen::MatrixBase<t> &uc = const_cast<Eigen::MatrixBase<t>&>(c);

void CUDA_ERROR_WRAPPER(cudaError_t cudaStat,std::string error_message) {
	if (cudaStat != cudaSuccess) {
		std::cout << error_message << std::endl;
		exit (EXIT_FAILURE);
	}
}

void CUBLAS_ERROR_WRAPPER(cublasStatus_t cudaStat,std::string error_message) {
	if (cudaStat != cudaSuccess) {
		std::cout << error_message << std::endl;
		exit (EXIT_FAILURE);
	}
}


 void CUDA_GET_LAST_ERROR() {
	cudaError_t code = cudaGetLastError();
	if ( cudaSuccess != code ) {
		std::cout << "Error in kernel\n";
		std::cout << "NO MESSAGE\n";
		fprintf(stderr,"GPUassert: %s\n", cudaGetErrorString(code));
		exit (EXIT_FAILURE);
	}
}

 void CUDA_GET_LAST_ERROR(std::string msg) {
	cudaError_t code = cudaGetLastError();
	if ( cudaSuccess != code ) {
		std::cout << "Error in kernel\n";
		fprintf(stderr,"GPUassert: %s\n", cudaGetErrorString(code));
		std::cout << msg << "\n";
		exit (EXIT_FAILURE);
	}
}

// void CUDA_GET_LAST_ERROR(std::string message) {
// 	if ( cudaSuccess != cudaGetLastError() ) {
// 		std::cout << "Error in kernel: " << message << "\n" ;
// 	}
// }

//Can be used for either double or float, use floats for performance, but doubles for gradient checking
template<typename dType>
void initialize_Matrix(dType *h_matrix,int rows,int cols) {
	boost::uniform_real<> distribution(BZ_CUDA::lower,BZ_CUDA::upper);
	for(int j=0; j<cols; j++) {
		for(int i=0; i<rows; i++) {
			h_matrix[IDX2C(i,j,rows)] =  (dType)distribution(BZ_CUDA::gen);
		}
	}
}

template<typename dType>
void initialize_Matrix_ones(dType *h_matrix,int rows,int cols) {
	for(int j=0; j<cols; j++) {
		for(int i=0; i<rows; i++) {
			h_matrix[IDX2C(i,j,rows)] =  1;
		}
	}
}

template<typename dType>
void initialize_Matrix_zeros(dType *h_matrix,int rows,int cols) {
	for(int j=0; j<cols; j++) {
		for(int i=0; i<rows; i++) {
			h_matrix[IDX2C(i,j,rows)] =  0;
		}
	}
}


template<typename dType>
void allocate_Matrix_CPU(dType **h_matrix,int rows,int cols) {
	*h_matrix = (dType *)malloc(rows*cols*sizeof(dType));
}

template<typename dType>
void allocate_Matrix_GPU(dType **d_matrix,int rows,int cols) {
	CUDA_ERROR_WRAPPER(cudaMalloc((void**)d_matrix, rows*cols*sizeof(dType)),"GPU memory allocation failed\n");
}

template<typename dType>
void set_matrix_cuBLAS(dType *h_matrix,dType *d_matrix,int rows,int cols) {
	CUBLAS_ERROR_WRAPPER(cublasSetMatrix(rows, cols, sizeof(dType), h_matrix, rows, d_matrix, rows),"cuBLAS set matrix failed\n");
}

template<typename dType>
void set_vector_cuBLAS(dType *h_vector,dType *d_vector,int rows) {
	CUBLAS_ERROR_WRAPPER(cublasSetVector(rows, sizeof(dType), h_vector, 1, d_vector, 1),"cuBLAS set vector failed\n");
}

template<typename dType>
void get_matrix_cuBLAS(dType *h_matrix,dType *d_matrix,int rows,int cols) {
	CUBLAS_ERROR_WRAPPER(cublasGetMatrix(rows, cols, sizeof(dType), d_matrix, rows, h_matrix, rows),"cuBLAS get matrix failed\n");
}

template<typename dType>
void get_vector_cuBLAS(dType *h_vector,dType *d_vector,int rows) {
	CUBLAS_ERROR_WRAPPER(cublasGetVector(rows, sizeof(dType), d_vector, 1, h_vector, 1),"cuBLAS get vector failed\n");
}

template<typename dType>
void full_matrix_setup(dType **h_matrix,dType **d_matrix,int rows,int cols) {
	allocate_Matrix_CPU(h_matrix,rows,cols);
	initialize_Matrix(*h_matrix,rows,cols);
	allocate_Matrix_GPU(d_matrix,rows,cols);
	set_matrix_cuBLAS(*h_matrix,*d_matrix,rows,cols);

	#ifndef CPU_DEBUG
	free(*h_matrix);
	#endif
}


template<typename dType>
void full_matrix_setup_0(dType **h_matrix,dType **d_matrix,int rows,int cols) {
	allocate_Matrix_CPU(h_matrix,rows,cols);
	initialize_Matrix_zeros(*h_matrix,rows,cols);
	allocate_Matrix_GPU(d_matrix,rows,cols);
	set_matrix_cuBLAS(*h_matrix,*d_matrix,rows,cols);

	#ifndef CPU_DEBUG
	free(*h_matrix);
	#endif
}


template<typename dType>
void full_vector_setup(dType **h_vector,dType **d_vector,int rows) {
	allocate_Matrix_CPU(h_vector,rows,1);
	initialize_Matrix(*h_vector,rows,1);
	allocate_Matrix_GPU(d_vector,rows,1);
	set_vector_cuBLAS(*h_vector,*d_vector,rows);

	#ifndef CPU_DEBUG
	free(*h_vector);
	#endif
}

template<typename dType>
void full_vector_setup_ones(dType **h_vector,dType **d_vector,int rows) {
	allocate_Matrix_CPU(h_vector,rows,1);
	initialize_Matrix_ones(*h_vector,rows,1);
	allocate_Matrix_GPU(d_vector,rows,1);
	set_vector_cuBLAS(*h_vector,*d_vector,rows);

	#ifndef CPU_DEBUG
	free(*h_vector);
	#endif
}

void initialize_vector_vocab(int *h_vector,int rows,int vocab_size) {
	boost::uniform_real<> distribution(0,1);
	for(int i=0; i<rows; i++) {
		h_vector[i] = (int)(vocab_size*distribution(BZ_CUDA::gen));
	}
}

void initialize_vector_vocab_01(int *h_vector,int rows) {
	 srand (time(NULL));
	for(int i=0; i<rows; i++) {
		h_vector[i] = (int)(rand()%2);
	}
}

void full_vector_setup_vocab(int **h_vector,int **d_vector,int rows,int vocab_size) {
	allocate_Matrix_CPU(h_vector,rows,1);
	initialize_vector_vocab(*h_vector,rows,vocab_size);
	allocate_Matrix_GPU(d_vector,rows,1);
	set_vector_cuBLAS(*h_vector,*d_vector,rows);

	#ifndef CPU_DEBUG
	free(*h_vector);
	#endif
}

void full_vector_setup_vocab_01(int **h_vector,int **d_vector,int rows) {
	allocate_Matrix_CPU(h_vector,rows,1);
	initialize_vector_vocab_01(*h_vector,rows);
	allocate_Matrix_GPU(d_vector,rows,1);
	set_vector_cuBLAS(*h_vector,*d_vector,rows);

	#ifndef CPU_DEBUG
	free(*h_vector);
	#endif
}

template<typename dType>
void print_matrix(dType *h_matrix,int rows,int cols) {
	for(int i=0; i<rows; i++) {
		for(int j=0; j<cols; j++) {
			std::cout << h_matrix[IDX2C(i,j,rows)] << " ";
		}
		std::cout << "\n";
	}
	std::cout << "\n";
}


template<typename Derived>
void print_eigen_matrix(const Eigen::MatrixBase<Derived> &h_mat) {
	for(int i=0; i<h_mat.rows(); i++) {
		for(int j=0; j<h_mat.cols(); j++) {
			std::cout << h_mat(i,j) << " ";
		}
		std::cout << "\n";
	}
	std::cout << "\n";
}

template<typename dType>
void print_thrust_matrix(thrust::host_vector<dType> &h_mat,int rows,int cols) {
	for(int i=0; i<rows; i++) {
		for(int j=0; j<cols; j++) {
			std::cout << h_mat[IDX2C(i,j,rows)] << " ";
		}
		std::cout << "\n";
	}
	std::cout << "\n";
}



//returns true if eigen matrix is the same, false otherwise
template<typename Derived,typename dType>
bool eigen_check(const Eigen::MatrixBase<Derived> &h_eigen_mat,dType *h_cuda_matrix) {
	for(int i=0; i<h_eigen_mat.rows(); i++) {
		for(int j=0; j<h_eigen_mat.cols(); j++) {
			if(h_eigen_mat(i,j) != h_cuda_matrix[IDX2C(i,j,h_eigen_mat.rows())]) {
				return false;
			}
		}
	}
	return true;
}

//returns true if eigen matrix is the same, false otherwise
template<typename Derived,typename dType>
bool eigen_check_thres(const Eigen::MatrixBase<Derived> &h_eigen_mat,dType *h_cuda_matrix,dType threshold) {
	int num_bad = 0;
	dType max_fail = 0;
	dType average_fail = 0;
	for(int i=0; i<h_eigen_mat.rows(); i++) {
		for(int j=0; j<h_eigen_mat.cols(); j++) {
			if(  std::abs(h_eigen_mat(i,j) -h_cuda_matrix[IDX2C(i,j,h_eigen_mat.rows())]) > threshold  ) {
				//std::cout << "Eigen check failing at: " << i << " " << j << "\n";
				//std::cout << "Difference: " << std::abs(h_eigen_mat(i,j) - h_cuda_matrix[IDX2C(i,j,h_eigen_mat.rows())]) << "\n";
				dType diff = std::abs(h_eigen_mat(i,j)-h_cuda_matrix[IDX2C(i,j,h_eigen_mat.rows())] );
				average_fail+=diff;
				if(diff > max_fail) {
					max_fail = diff;
				}
				num_bad++;
			}
		}
	}
	
	if(num_bad > 0) {
		std::cout << "Total that could fail: " << h_eigen_mat.rows()*h_eigen_mat.cols() << "\n";
		std::cout << "Number in eigen check that failed: " << num_bad << "\n";
		std::cout << "Max fail: " << max_fail << "\n";
		std::cout << "average fail: " << average_fail/num_bad << "\n";
		return false;
	} 
	return true;
}

#include <set>
template<typename Derived,typename dType>
void eigen_check_thrust_ptr(const Eigen::MatrixBase<Derived> &h_eigen_mat,dType *d_ptr,std::string msg,dType threshold) {
	//thrust::device_ptr<dType> debug_ptr = thrust::device_pointer_cast(d_ptr);

	int tot_size = h_eigen_mat.rows()*h_eigen_mat.cols()*sizeof(dType);
	dType * h_temp = (dType *)malloc(tot_size);
	cudaMemcpy(h_temp, d_ptr, tot_size, cudaMemcpyDeviceToHost);
	int num_bad =0;
	dType max_fail = 0;
	dType average_fail = 0;
	std::set<int> myset;
	std::set<int> myset2;
	for(int i=0; i<h_eigen_mat.rows(); i++) {
		for(int j=0; j<h_eigen_mat.cols(); j++) {
			if(  std::abs(h_eigen_mat(i,j) -h_temp[IDX2C(i,j,h_eigen_mat.rows())]) > threshold  ) {
				dType diff = std::abs(h_eigen_mat(i,j)-h_temp[IDX2C(i,j,h_eigen_mat.rows())] );
				average_fail+=diff;
				if(diff > max_fail) {
					max_fail = diff;
				}
				myset.insert(j);
				num_bad++;
				myset2.insert(i);
			}
		}
	}
	
	if(num_bad > 0) {
		std::cout << "Operation: " << msg << " failed\n";
		std::cout << "Total that could fail: " << h_eigen_mat.rows()*h_eigen_mat.cols() << "\n";
		std::cout << "Number in eigen check that failed: " << num_bad << "\n";
		std::cout << "Max fail: " << max_fail << "\n";
		std::cout << "average fail: " << average_fail/num_bad << "\n";
		for (auto it=myset.begin(); it!=myset.end(); ++it)
    		std::cout << ' ' << *it;
    	std::cout << "\n\n";

    	for (auto it=myset2.begin(); it!=myset2.end(); ++it)
    		std::cout << ' ' << *it;
    	std::cout << "\n\n";
    	//std::cout << h_eigen_mat << "\n\n\n\n";
  //   	for(int i=0; i<h_eigen_mat.rows(); i++) {
		// 	for(int j=0; j<h_eigen_mat.cols(); j++) {
		// 		std::cout << h_temp[IDX2C(i,j,h_eigen_mat.rows())] << " ";
		// 	}
		// 	std::cout << "\n";
		// }
		std::cout << "\n";
		exit (EXIT_FAILURE);
	} 
	free(h_temp);
}

template<typename dType>
void check_GPU_GPU(dType *mat1,dType *mat2,dType threshold,int rows,int cols,std::string msg) {
	thrust::device_ptr<dType> debug_ptr = thrust::device_pointer_cast(mat1);
	thrust::device_ptr<dType> debug_ptr2 = thrust::device_pointer_cast(mat2);
	int num_bad =0;
	dType max_fail = 0;
	dType average_fail = 0;

	for(int i=0; i<rows; i++) {
		for(int j=0; j<cols; j++) {
			int idx = IDX2C(i,j,rows);
			if(  std::abs(debug_ptr2[idx] - debug_ptr[idx]) > threshold  ) {
				dType diff = std::abs( debug_ptr2[idx] - debug_ptr[idx] );
				average_fail+=diff;
				if(diff > max_fail) {
					max_fail = diff;
				}
				num_bad++;
			}
		}
	}

	if(num_bad > 0) {
		std::cout << "Operation: " << msg << " failed\n";
		std::cout << "Total that could fail: " << rows*cols << "\n";
		std::cout << "Number in eigen check that failed: " << num_bad << "\n";
		std::cout << "Max fail: " << max_fail << "\n";
		std::cout << "average fail: " << average_fail/num_bad << "\n";
		exit (EXIT_FAILURE);
	} 
}


//returns true if eigen matrix is the same, false otherwise
template<typename Derived,typename dType>
bool eigen_check_thres_thrust(const Eigen::MatrixBase<Derived> &h_eigen_mat,thrust::host_vector<dType> &h_mat,dType threshold) {
	for(int i=0; i<h_eigen_mat.rows(); i++) {
		for(int j=0; j<h_eigen_mat.cols(); j++) {
			if(  std::abs(h_eigen_mat(i,j) -h_mat[IDX2C(i,j,h_eigen_mat.rows())]) > threshold  ) {
				return false;
			}
		}
	}
	return true;
}

//Copy a matrix in column major format to eigen
template<typename Derived,typename dType>
void copy_to_eigen(const Eigen::MatrixBase<Derived> &h_eigen_mat_const,dType *h_cuda_matrix) {
	UNCONST(Derived,h_eigen_mat_const,h_eigen_mat);
	for(int i=0; i<h_eigen_mat.rows(); i++) {
		for(int j=0; j<h_eigen_mat.cols(); j++) {
			h_eigen_mat(i,j) = h_cuda_matrix[IDX2C(i,j,h_eigen_mat.rows())];
		}
	}
}

//Copy a matrix in column major format to eigen
template<typename Derived,typename dType>
void copy_to_eigen_thrust(const Eigen::MatrixBase<Derived> &h_eigen_mat_const,
	thrust::host_vector<dType> &h_mat_thrust) 
{
	UNCONST(Derived,h_eigen_mat_const,h_eigen_mat);
	for(int i=0; i<h_eigen_mat.rows(); i++) {
		for(int j=0; j<h_eigen_mat.cols(); j++) {
			h_eigen_mat(i,j) = h_mat_thrust[IDX2C(i,j,h_eigen_mat.rows())];
		}
	}
}


//note there are no thrust matrices only vectors
template<typename dType>
void initialize_thrust_vector(thrust::host_vector<dType> &h_vec,int size) {
	boost::uniform_real<> distribution(BZ_CUDA::lower,BZ_CUDA::upper);
	for(int i=0; i<size; i++) {
		h_vec[i] = (dType)distribution(BZ_CUDA::gen);
	}
}

template<typename dType>
void print_GPU_Matrix(dType *d_ptr,int rows,int cols) {
	thrust::device_ptr<dType> debug_ptr = thrust::device_pointer_cast(d_ptr);
	for(int i=0; i<rows; i++) {
		for(int j=0; j<cols; j++) {
			std::cout << debug_ptr[IDX2C(i,j,rows)] << " ";
		}
		std::cout << "\n";
	}
	std::cout << "\n";
}



//------------------------------------CUBLAS ERROR WRAPPERS--------------------------------------


///////////////////////////////////////////DOUBLE DEFINE BEGIN///////////////////////////////////////
inline cublasStatus_t cublas_gemm_wrapper(cublasHandle_t handle, cublasOperation_t transa, cublasOperation_t transb,
	 int m, int n, int k, const float *alpha, const float *A, int lda, 
	 const float *B, int ldb, const float *beta, float *C, int ldc) 
{
	return cublasSgemm(handle, transa, transb, m, n, k, alpha, A, lda, B, ldb, beta, C, ldc);
}

inline cublasStatus_t cublas_gemm_wrapper(cublasHandle_t handle, cublasOperation_t transa, cublasOperation_t transb,
	 int m, int n, int k, const double *alpha, const double *A, int lda, 
	 const double *B, int ldb, const double *beta, double *C, int ldc) 
{
	return cublasDgemm(handle, transa, transb, m, n, k, alpha, A, lda, B, ldb, beta, C, ldc);
}
///////////////////////////////////////////DOUBLE DEFINE END///////////////////////////////////////


///////////////////////////////////////////DOUBLE DEFINE BEGIN///////////////////////////////////////
inline cublasStatus_t cublas_geam_wrapper(cublasHandle_t handle, cublasOperation_t transa, cublasOperation_t transb, 
	int m, int n, const float *alpha, const float *A, int lda, const float *beta, 
	const float *B, int ldb, float *C, int ldc) 
{
	return cublasSgeam(handle, transa, transb, m, n, alpha, A, lda, beta, B, ldb, C, ldc);

}

inline cublasStatus_t cublas_geam_wrapper(cublasHandle_t handle, cublasOperation_t transa, cublasOperation_t transb, 
	int m, int n, const double *alpha, const double *A, int lda, const double *beta, 
	const double *B, int ldb, double *C, int ldc) 
{
	return cublasDgeam(handle, transa, transb, m, n, alpha, A, lda, beta, B, ldb, C, ldc);

}
///////////////////////////////////////////DOUBLE DEFINE END///////////////////////////////////////


///////////////////////////////////////////DOUBLE DEFINE BEGIN///////////////////////////////////////
inline cublasStatus_t cublas_gemv_wrapper(cublasHandle_t handle, cublasOperation_t trans, int m, 
	int n, const float *alpha, const float *A, int lda, const float *x, int incx, 
	const float *beta, float *y, int incy) 
{
	return cublasSgemv(handle, trans, m, n, alpha, A, lda, x, incx, beta, y, incy);
}

inline cublasStatus_t cublas_gemv_wrapper(cublasHandle_t handle, cublasOperation_t trans, int m, 
	int n, const double *alpha, const double *A, int lda, const double *x, int incx, 
	const double *beta, double *y, int incy) 
{
	return cublasDgemv(handle, trans, m, n, alpha, A, lda, x, incx, beta, y, incy);
}
///////////////////////////////////////////DOUBLE DEFINE END///////////////////////////////////////



///////////////////////////////////////////DOUBLE DEFINE BEGIN///////////////////////////////////////
inline cublasStatus_t cublas_dgmm_wrapper(cublasHandle_t handle, cublasSideMode_t mode, int m, int n, 
	const float *A, int lda, const float *x, int incx, float *C, int ldc)
{
	return cublasSdgmm(handle, mode, m, n, A, lda, x, incx, C, ldc);
}

inline cublasStatus_t cublas_dgmm_wrapper(cublasHandle_t handle, cublasSideMode_t mode, int m, int n, 
	const double *A, int lda, const double *x, int incx, double *C, int ldc)
{
	return cublasDdgmm(handle, mode, m, n, A, lda, x, incx, C, ldc);
}

///////////////////////////////////////////DOUBLE DEFINE END///////////////////////////////////////


//atomic add for doubles,since undefined in cuda
__device__ double atomicAddDouble(double* address, double val)
{
    unsigned long long int* address_as_ull =
                                          (unsigned long long int*)address;
    unsigned long long int old = *address_as_ull, assumed;
    do {
        assumed = old;
        old = atomicCAS(address_as_ull, assumed, 
                        __double_as_longlong(val + 
                        __longlong_as_double(assumed)));
    } while (assumed != old);
    return __longlong_as_double(old);
}


__device__
inline float cuda_exp_wrapper(float x) {
	return expf(x);
}

__device__
inline double cuda_exp_wrapper(double x) {
	return exp(x);
}

__device__
inline float cuda_log_wrapper(float x) {
	return logf(x);
}

__device__
inline double cuda_log_wrapper(double x) {
	return log(x);
}



#endif