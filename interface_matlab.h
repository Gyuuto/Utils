#ifndef INTERFACE_MATLAB_H
#define INTERFACE_MATLAB_H

// These functions need linker option "-L/Applications/MATLAB_R2015b.app/bin/maci64 -L/Applications/MATLAB_R2015b.app/sys/os/maci64 -lmat -lmx"
#include "/Applications/MATLAB_R2015b.app/extern/include/mat.h"

#include <complex>
#include <string>
#include <vector>

// Generate Dense Matrix File for Matlab
// filename : name of mat-file (extension(.mat) is automatically concatenated by this function)
// element : elements of target matrix
void CreateMatFile ( std::string filename, const std::vector< std::vector<double> >& element )
{
	MATFile *mat;
	mxArray *data;
	int N = element.size(), M = element[0].size();
	mat = matOpen((filename+".mat").c_str(), "w");

	data = mxCreateDoubleMatrix(N, M, mxREAL);
	double* ptr_pr = mxGetPr(data);
	for( int i = 0; i < M; ++i ){
		for( int j = 0; j < N; ++j ){
			*(ptr_pr + i*N + j) = element[j][i];
		}
	}
	
	int status = matPutVariable(mat, filename.c_str(), data);
	mxDestroyArray(data);
	matClose(mat);
}

// Generate Dense Matrix File for Matlab
// filename : name of mat-file (extension(.mat) is automatically concatenated by this function)
// element : elements of target matrix
void CreateMatFile ( std::string filename, const std::vector< std::vector< std::complex<double> > >& element )
{
	MATFile *mat;
	mxArray *data;
	int N = element.size(), M = element[0].size();
	mat = matOpen((filename+".mat").c_str(), "w");

	data = mxCreateDoubleMatrix(N, M, mxCOMPLEX);
	double* ptr_pr = mxGetPr(data);
	double* ptr_pi = mxGetPi(data);
	for( int i = 0; i < M; ++i ){
		for( int j = 0; j < N; ++j ){
			*(ptr_pr + i*N + j) = element[j][i].real();
			*(ptr_pi + i*N + j) = element[j][i].imag();
		}
	}
	
	int status = matPutVariable(mat, filename.c_str(), data);
	mxDestroyArray(data);
	matClose(mat);
}


// Generate Sparse Matrix File for Matlab
// filename : name of mat-file (extension(.mat) is automatically concatenated by this function)
// element : elements of target matrix in sparse presentation
// indices : elements of indices (indices must be sorted by row-direction)
void CreateSMatFile ( std::string filename, const std::vector< std::vector<double> >& element, const std::vector< std::vector<int> >& indices )
{
	MATFile *mat;
	mxArray *data;
	int non_zero = 0;
	int N = element.size(), M = element.size();
	mat = matOpen((filename+".mat").c_str(), "w");

	for( int i = 0; i < N; ++i ){
		non_zero += element[i].size();
	}
	
	data = mxCreateSparse(N, N, non_zero, mxREAL);

	double *ptr_pr;
	size_t *ptr_ir, *ptr_jc;
	ptr_pr = mxGetPr(data);
	ptr_ir = mxGetIr(data);
	ptr_jc = mxGetJc(data);

	int row = 0, col = 0;
	for( int i = 0; i < non_zero; ++i ){
		if( row >= element[col].size() ){
			row = 0;
			col++;
		}
		
		ptr_pr[i] = element[col][row];
		ptr_ir[i] = indices[col][row];
		++row;
	}
	ptr_jc[0] = 0;
	for( int i = 1; i < N+1; ++i ){
		ptr_jc[i] = ptr_jc[i-1] + element[i-1].size();
	}

	int status = matPutVariable(mat, filename.c_str(), data);
	if( status != 0 ){
		std::cerr << "Create MAT File failed! filename : " << filename << std::endl;
	}
	
	mxDestroyArray(data);
	matClose(mat);
}

#endif
