// Matrix multiplication: C = A * B.
 
// Thread block size
#define BLOCK_SIZE 16
  
//////////////////////////////////////////////////////
//! Matrix multiplication on the device: C = A * B
//! wA is A's width and wB is B's width
//////////////////////////////////////////////////////
__kernel void
matrixMul(__global float* C, 
          __global float* A, 
          __global float* B, 
	  __global int* mask,
	  int wA, int wB, int BlockSize, int TileSize, int TileNumByBlockEdge)
{
    // Block index
    int bx = get_group_id(0);
    int by = get_group_id(1);
 
    int bxMax = wA / BlockSize + 1;
    int byMax = wB / BlockSize + 1;
    
    // Thread index
    int tx = get_local_id(0);
    int ty = get_local_id(1);

    // Tile index
    int tilex;
    int tiley;
    
    // Number of tiles per block edge
    int TileNumByBlock = TileNumByBlockEdge * TileNumByBlockEdge;
    float Csub = 0;
 
    // Index of the first sub-matrix of A processed 
    // by the block
    int aBegin = wA * BlockSize * by;
 
    // Index of the last sub-matrix of A processed 
    // by the block
    int aEnd   = aBegin + wA - 1;
 
    // Step size used to iterate through the 
    // sub-matrices of A
    int aStep  = BlockSize;
 
    // Index of the first sub-matrix of B processed 
    // by the block
    int bBegin = BlockSize * bx;
 
    // Step size used to iterate through the 
    // sub-matrices of B
    int bStep  = BlockSize * wB;
 
    // Loop over all the sub-matrices of A and B
    // required to compute the block sub-matrix
    for (int a = aBegin, b = bBegin;
             a <= aEnd;
             a += aStep, b += bStep) 
    {

        // Declaration of the local memory array As 
        // used to store the sub-matrix of A
        __local float As[BLOCK_SIZE][BLOCK_SIZE];
 
        // Declaration of the local memory array Bs 
        // used to store the sub-matrix of B
        __local float Bs[BLOCK_SIZE][BLOCK_SIZE];
 
        // Load the matrices from global memory
        // to local memory; each thread loads
        // one element of each matrix

//	for (tiley = 0; tiley < TileNumByBlockEdge; tiley += 1)
//	{
//	    for (tilex = 0; tilex < TileNumByBlockEdge; tilex +=1)
//	    {
//	        if (mask[by * bxMax *TileNumByBlock + bx * TileNumByBlock + tiley * TileNumByBlockEdge + tilex] == 1)
//		{
//		    if ((tx >= tilex * TileSize) && tx < ((tilex+1) * TileSize) && (ty >= tiley * TileSize) && (ty < (tiley+1) * TileSize))
//		    {
		        As[ty][tx] = A[a + wA * ty + tx];
			Bs[ty][tx] = B[b + wB * ty + tx];
//		    }
//		}
//	    }
//	}
	
	/*
	for (int i = 0; i < TileNumByBlock; i++) 
	{
	    if (mask[by * bxMax * TileNumByBlock + bx * TileNumByBlock + i] == 1)
	    {
	        tilex = i % TileNumByBlockEdge;
	        tiley = i / TileNumByBlockEdge;

	        if ((tx >= tilex * TileSize) && (tx < (tilex+1) * TileSize) && (ty >= tiley * TileSize ) && (ty < (tiley + 1) * TileSize))	
                As[ty][tx] = A[a + wA * ty + tx];
                Bs[ty][tx] = B[b + wB * ty + tx];
	    }
	}
 	*/
        // Synchronize to make sure the matrices 
        // are loaded
        barrier(CLK_LOCAL_MEM_FENCE);
 
        // Multiply the two matrices together;
        // each thread computes one element
        // of the block sub-matrix
        for (int k = 0; k < BLOCK_SIZE; ++k)
            Csub += As[ty][k] * Bs[k][tx];
 
        // Synchronize to make sure that the preceding
        // computation is done before loading two new
        // sub-matrices of A and B in the next iteration
        barrier(CLK_LOCAL_MEM_FENCE);
 
    }
 
    // Write the block sub-matrix to device memory;
    // each thread writes one element
    int c = wB * BLOCK_SIZE * by + BLOCK_SIZE * bx;
    C[c + wB * ty + tx] = Csub;

}
