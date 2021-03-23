#include <cstring>

const int parallels = 16;
const int loop_size = 4096/parallels;
using uint = unsigned int;

void small_vadd(uint* input1, uint* input2, uint* output, int id) {
	for (int i = 0; i < loop_size; ++i) {
		int index = id * loop_size + i; // for block
		// int index = loop_size * i + id; // for cyclic
		output[index] = input1[index] + input2[index];
	}
}

void vadd(uint* input1, uint* input2, uint* output) {

	#pragma HLS dataflow

    for (int i = 0; i < parallels; ++i) {
        #pragma HLS unroll
    	small_vadd(input1, input2, output, i);
    }
}

extern "C" {

void krnl_vadd(const uint *in1, // Read-Only Vector 1
          const uint *in2, // Read-Only Vector 2
          uint *out_r,     // Output Result
          int size                 // Size in integer
    ) {
    #pragma HLS INTERFACE m_axi port = in1 offset = slave bundle = gmem
    #pragma HLS INTERFACE m_axi port = in2 offset = slave bundle = gmem
    #pragma HLS INTERFACE m_axi port = out_r offset = slave bundle = gmem
    #pragma HLS INTERFACE s_axilite port = in1 bundle = control
    #pragma HLS INTERFACE s_axilite port = in2 bundle = control
    #pragma HLS INTERFACE s_axilite port = out_r bundle = control
    #pragma HLS INTERFACE s_axilite port = size bundle = control
    #pragma HLS INTERFACE s_axilite port = return bundle = control


	uint l_in1[4096];
	uint l_in2[4096];
	uint l_out[4096];

	// block
	#pragma HLS array_partition variable=l_in1 block factor=16
	#pragma HLS array_partition variable=l_in2 block factor=16
	#pragma HLS array_partition variable=l_out block factor=16
	
	// cyclic
	//#pragma HLS array_partition variable=l_in1 cyclic factor=16
	//#pragma HLS array_partition variable=l_in2 cyclic factor=16
	//#pragma HLS array_partition variable=l_out cyclic factor=16

	std::memcpy(l_in1, in1, sizeof(uint) * 4096);
	std::memcpy(l_in2, in2, sizeof(uint) * 4096);

	vadd(l_in1, l_in2, l_out);

    std::memcpy(out_r, l_out, sizeof(uint) * 4096);
}

}
