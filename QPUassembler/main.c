#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "qpu_assembler.h"
#include "shaders.h"

int main()
{
#define shader singleTextureClippingPlane_AlphaDisabled_DstSrcAlpha_SrcZero_DepthStencilEnabled_FS

	for(uint32_t c = 0; c < sizeof(shader)/sizeof(uint64_t); ++c)
	{
		disassemble_qpu_asm(shader[c]);
	}

	return 0;
}
