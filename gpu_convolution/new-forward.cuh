#ifndef MXNET_OPERATOR_NEW_FORWARD_CUH_
#define MXNET_OPERATOR_NEW_FORWARD_CUH_
#include <mxnet/base.h>
#include <stdio.h>
#include <math.h>
#define TILE_WIDTH 16
#define TILE_WIDTH2 24

namespace mxnet
{
namespace op
{

__global__ void forward_kernel_layer2(float *y, float *x, float *w, int B, int C, int H, int K, int W, int M) {
	#define k4d(i3, i2, i1, i0) w[(i3)*(C*K*K) + (i2)*(K*K) + (i1)*(K) + i0]
	#define x4d(i3, i2, i1, i0) x[(i3)*(C*H*W) + (i2)*(H*W) + (i1)*(W) + i0]
	#define y4d(i3, i2, i1, i0) y[(i3)*(M*H_out*W_out) + (i2)*(H_out*W_out) + (i1)*(W_out) + i0]

	__shared__ float W_tile[TILE_WIDTH][TILE_WIDTH];
	__shared__ float X_tile[TILE_WIDTH][TILE_WIDTH];

	int b = blockIdx.z;
	int tx = threadIdx.x;
	int ty = threadIdx.y;
	int row = blockIdx.y*TILE_WIDTH + ty;
	int col = blockIdx.x*TILE_WIDTH + tx;
	int Ksq = K*K;
	int unrollColumn = C*Ksq;
	const int H_out = H - K + 1;
	const int W_out = W - K + 1;
	
	float pValue = 0.0;
	int X_b = b;
	int W_m = row;
	int num_blocks = ceil(unrollColumn/(1.0*TILE_WIDTH));
	for (int i = 0; i < num_blocks; i++) {
		int col_t = i*TILE_WIDTH + tx;
		int row_t = i*TILE_WIDTH + ty;

		// load X into shared mem
		
		int X_c = row_t/(Ksq);
		int X_p = (row_t%(Ksq))/K;
		int X_q = (row_t%(Ksq))%K;
		int X_h = col/W_out;
		int X_w = col%W_out;

		if (row_t < unrollColumn && col < H_out*W_out) {
			X_tile[ty][tx] = x4d(X_b, X_c, X_h + X_p, X_w + X_q);
		} else {
			X_tile[ty][tx] = 0;
		}

		// load W into shared mem
		
		int W_c = col_t/(Ksq);
		int W_h = (col_t%(Ksq))/K;
		int W_w = (col_t%(Ksq))%K;
		if ((col_t < unrollColumn) && (row < M)){
			W_tile[ty][tx] = k4d(W_m, W_c, W_h, W_w);
		} else {
			W_tile[ty][tx] = 0;
		}
		__syncthreads();
		
		for (int j = 0; j < TILE_WIDTH; j++){
			pValue += W_tile[ty][j]*X_tile[j][tx];
		}
		__syncthreads();
	}

	// write result
	int Y_b = b;
	int Y_m = row;
	int Y_h = col / W_out;
	int Y_w = col % W_out;

	if (row < M && col < W_out*H_out) {
		y4d(Y_b, Y_m, Y_h, Y_w) = pValue;
	}
}


__global__ void forward_kernel_layer1(float *y, float *x, float *w, int B, int C, int H, int K, int W, int M) {
	#define k4d(i3, i2, i1, i0) w[(i3)*(C*K*K) + (i2)*(K*K) + (i1)*(K) + i0]
	#define x4d(i3, i2, i1, i0) x[(i3)*(C*H*W) + (i2)*(H*W) + (i1)*(W) + i0]
	#define y4d(i3, i2, i1, i0) y[(i3)*(M*H_out*W_out) + (i2)*(H_out*W_out) + (i1)*(W_out) + i0]

	__shared__ float W_tile[TILE_WIDTH2][TILE_WIDTH2];
	__shared__ float X_tile[TILE_WIDTH2][TILE_WIDTH2];

	int b = blockIdx.z;
	int tx = threadIdx.x;
	int ty = threadIdx.y;
	int row = blockIdx.y*TILE_WIDTH2 + ty;
	int col = blockIdx.x*TILE_WIDTH2 + tx;
	int Ksq = K*K;
	int unrollColumn = C*Ksq;
	const int H_out = H - K + 1;
	const int W_out = W - K + 1;
	
	float pValue = 0.0;
	int X_b = b;
	int W_m = row;
	int num_blocks = ceil(unrollColumn/(1.0*TILE_WIDTH2));
	for (int i = 0; i < num_blocks; i++) {
		int col_t = i*TILE_WIDTH2 + tx;
		int row_t = i*TILE_WIDTH2 + ty;

		// load X into shared mem
		int X_c = row_t/(Ksq);
		int X_p = (row_t%(Ksq))/K;
		int X_q = (row_t%(Ksq))%K;
		int X_h = col/W_out;
		int X_w = col%W_out;

		if (row_t < unrollColumn && col < H_out*W_out) {
			X_tile[ty][tx] = x4d(X_b, X_c, X_h + X_p, X_w + X_q);
		} else {
			X_tile[ty][tx] = 0;
		}

		// load W into shared mem
		int W_c = col_t/(Ksq);
		int W_h = (col_t%(Ksq))/K;
		int W_w = (col_t%(Ksq))%K;

		if ((col_t < unrollColumn) && (row < M)){
			W_tile[ty][tx] = k4d(W_m, W_c, W_h, W_w);
		} else {
			W_tile[ty][tx] = 0;
		}
		__syncthreads();
		
		for (int j = 0; j < TILE_WIDTH2; j++){
			pValue += W_tile[ty][j]*X_tile[j][tx];
		}
		__syncthreads();
	}

	// write result
	int Y_b = b;
	int Y_m = row;
	int Y_h = col / W_out;
	int Y_w = col % W_out;

	if (row < M && col < W_out*H_out) {
		y4d(Y_b, Y_m, Y_h, Y_w) = pValue;
	}
}

/*
This function is called by new-inl.h
*/
template <>
void forward<gpu, float>(mshadow::Tensor<gpu, 4, float> &y, const mshadow::Tensor<gpu, 4, float> &x, const mshadow::Tensor<gpu, 4, float> &w)
{
	const int B = x.shape_[0];
	const int M = y.shape_[1];
	const int C = x.shape_[1];
	const int H = x.shape_[2];
	const int W = x.shape_[3];
	const int K = w.shape_[3];

	const int H_out = H - K + 1;
	const int W_out = W - K + 1;

	if (M <= 16) {
		dim3 blockDim(TILE_WIDTH, TILE_WIDTH, 1);
		dim3 gridDim(ceil(H_out*W_out/(1.0*TILE_WIDTH)), ceil(M/(1.0*TILE_WIDTH)), B);
		forward_kernel_layer2<<<gridDim, blockDim>>>(y.dptr_, x.dptr_, w.dptr_, B, C, H, K, W, M);
		MSHADOW_CUDA_CALL(cudaDeviceSynchronize());
	} else {
		dim3 blockDim(TILE_WIDTH2, TILE_WIDTH2, 1);
		dim3 gridDim(ceil(H_out*W_out/(1.0*TILE_WIDTH2)), ceil(M/(1.0*TILE_WIDTH2)), B);
		forward_kernel_layer1<<<gridDim, blockDim>>>(y.dptr_, x.dptr_, w.dptr_, B, C, H, K, W, M);
		MSHADOW_CUDA_CALL(cudaDeviceSynchronize());
	}
}

} // end namespace mxnet
} // end namespace op



