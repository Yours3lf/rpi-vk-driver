#include "ControlListUtil.h"
#include "ConsecutivePoolAllocator.h"

#include <stdint.h>

uint32_t divRoundUp(uint32_t n, uint32_t d)
{
	return (((n) + (d) - 1) / (d));
}

//move bits to offset, mask rest to 0
uint32_t moveBits(uint32_t d, uint32_t bits, uint32_t offset)
{
	return (d << offset) & (~(~0u << bits) << offset);
}

uint32_t clHasEnoughSpace(ControlList* cl, uint32_t size)
{
	assert(cl);
	assert(cl->CPA);
	uint32_t currSize = cl->nextFreeByteOffset - cl->offset;

	if((currSize + size) <= (cl->numBlocks * cl->blockSize))
	{
		return 1; //fits!
	}
	else
	{
		return 0; //need to reallocate
	}
}

void clInit(ControlList* cl, void* CPA, uint32_t offset, uint32_t blockSize)
{
	assert(cl);
	assert(CPA);
	cl->offset = offset;
	cl->numBlocks = 1;
	cl->blockSize = blockSize;
	cl->nextFreeByteOffset = offset;
	cl->currMarkerOffset = -1;
	cl->CPA = CPA;
}

void clInsertNewCLMarker(ControlList* cl,
						 ControlList* handlesCL,
						 ControlList* shaderRecCL,
						 uint32_t shaderRecCount,
						 ControlList* uniformsCL)
{
	//to be inserted when you'd insert tile binning mode config
	assert(cl);
	assert(cl->CPA);
	assert(handlesCL);
	assert(shaderRecCL);
	assert(uniformsCL);

	CLMarker marker = {0};
	marker.memGuard = 0xDDDDDDDD;
	marker.nextMarkerOffset = -1;

	//close current marker
	if(cl->currMarkerOffset != ~0u && !((CLMarker*)getCPAptrFromOffset(cl->CPA, cl->currMarkerOffset))->size)
	{
		clCloseCurrentMarker(cl, handlesCL, shaderRecCL, shaderRecCount, uniformsCL);
	}

	//if this is not the first marker
	if(cl->currMarkerOffset != ~0u)
	{
		CLMarker* currMarker = getCPAptrFromOffset(cl->CPA, cl->currMarkerOffset);
		marker.handlesBufOffset = currMarker->handlesBufOffset + currMarker->handlesSize;
		marker.shaderRecBufOffset = currMarker->shaderRecBufOffset + currMarker->shaderRecSize;
		marker.uniformsBufOffset = currMarker->uniformsBufOffset + currMarker->uniformsSize;
		marker.shaderRecCount = currMarker->shaderRecCount; //initialize with previous marker's data
	}

	*(CLMarker*)getCPAptrFromOffset(cl->CPA, cl->nextFreeByteOffset) = marker;
	if(cl->currMarkerOffset != ~0u)
	{
		((CLMarker*)getCPAptrFromOffset(cl->CPA, cl->currMarkerOffset))->nextMarkerOffset = (cl->nextFreeByteOffset - cl->offset);
	}
	cl->currMarkerOffset = cl->nextFreeByteOffset;
	cl->nextFreeByteOffset += sizeof(CLMarker);
}

void clCloseCurrentMarker(ControlList* cl, ControlList* handlesCL, ControlList* shaderRecCL, uint32_t shaderRecCount, ControlList* uniformsCL)
{
	assert(cl);
	assert(cl->CPA);
	assert(handlesCL);
	assert(shaderRecCL);
	assert(uniformsCL);
	CLMarker* currMarker = getCPAptrFromOffset(cl->CPA, cl->currMarkerOffset);
	currMarker->size = cl->nextFreeByteOffset - (cl->currMarkerOffset + sizeof(CLMarker));
	currMarker->handlesSize = handlesCL->nextFreeByteOffset - (currMarker->handlesBufOffset + handlesCL->offset);
	currMarker->shaderRecSize = shaderRecCL->nextFreeByteOffset - (currMarker->shaderRecBufOffset + shaderRecCL->offset);
	currMarker->uniformsSize = uniformsCL->nextFreeByteOffset - (currMarker->uniformsBufOffset + uniformsCL->offset);
	currMarker->shaderRecCount = shaderRecCount - currMarker->shaderRecCount; //update shader rec count to reflect added shader recs
}

void clInsertData(ControlList* cl, uint32_t size, void* data)
{
	assert(cl);
	assert(cl->CPA);
	memcpy(getCPAptrFromOffset(cl->CPA, cl->nextFreeByteOffset), data, size);
	cl->nextFreeByteOffset += size;
}

void clInsertUniformConstant(ControlList* cl, uint32_t data)
{
	assert(cl);
	assert(cl->CPA);
	memcpy(getCPAptrFromOffset(cl->CPA, cl->nextFreeByteOffset), &data, sizeof(uint32_t));
	cl->nextFreeByteOffset += 4;
}

void clInsertUniformXYScale(ControlList* cl, float data)
{
	assert(cl);
	assert(cl->CPA);
	memcpy(getCPAptrFromOffset(cl->CPA, cl->nextFreeByteOffset), &data, sizeof(float));
	cl->nextFreeByteOffset += 4;
}

void clInsertUniformZOffset(ControlList* cl, float data)
{
	assert(cl);
	assert(cl->CPA);
	memcpy(getCPAptrFromOffset(cl->CPA, cl->nextFreeByteOffset), &data, sizeof(float));
	cl->nextFreeByteOffset += 4;
}

void clInsertHalt(ControlList* cl)
{
	assert(cl);
	assert(cl->CPA);
	uint8_t opCode = V3D21_HALT_opcode;
	memcpy(getCPAptrFromOffset(cl->CPA, cl->nextFreeByteOffset), &opCode, sizeof(uint8_t));
	cl->nextFreeByteOffset++;
}

void clInsertNop(ControlList* cl)
{
	assert(cl);
	assert(cl->CPA);
	uint8_t opCode = V3D21_NOP_opcode;
	memcpy(getCPAptrFromOffset(cl->CPA, cl->nextFreeByteOffset), &opCode, sizeof(uint8_t));
	cl->nextFreeByteOffset++;
}

void clInsertFlush(ControlList* cl)
{
	assert(cl);
	assert(cl->CPA);
	uint8_t opCode = V3D21_FLUSH_opcode;
	memcpy(getCPAptrFromOffset(cl->CPA, cl->nextFreeByteOffset), &opCode, sizeof(uint8_t));
	cl->nextFreeByteOffset++;
}

void clInsertFlushAllState(ControlList* cl)
{
	assert(cl);
	assert(cl->CPA);
	uint8_t opCode = V3D21_FLUSH_ALL_STATE_opcode;
	memcpy(getCPAptrFromOffset(cl->CPA, cl->nextFreeByteOffset), &opCode, sizeof(uint8_t));
	cl->nextFreeByteOffset++;
}

void clInsertStartTileBinning(ControlList* cl)
{
	assert(cl);
	assert(cl->CPA);
	uint8_t opCode = V3D21_START_TILE_BINNING_opcode;
	memcpy(getCPAptrFromOffset(cl->CPA, cl->nextFreeByteOffset), &opCode, sizeof(uint8_t));
	cl->nextFreeByteOffset++;
}

void clInsertIncrementSemaphore(ControlList* cl)
{
	assert(cl);
	assert(cl->CPA);
	uint8_t opCode = V3D21_INCREMENT_SEMAPHORE_opcode;
	memcpy(getCPAptrFromOffset(cl->CPA, cl->nextFreeByteOffset), &opCode, sizeof(uint8_t));
	cl->nextFreeByteOffset++;
}

void clInsertWaitOnSemaphore(ControlList* cl)
{
	assert(cl);
	assert(cl->CPA);
	uint8_t opCode = V3D21_WAIT_ON_SEMAPHORE_opcode;
	memcpy(getCPAptrFromOffset(cl->CPA, cl->nextFreeByteOffset), &opCode, sizeof(uint8_t));
	cl->nextFreeByteOffset++;
}

//input: 2 cls (cl, handles cl)
void clInsertBranch(ControlList* cls, ControlListAddress address)
{
	assert(cls);
	assert(cls->CPA);
	uint8_t opCode = V3D21_BRANCH_opcode;
	memcpy(getCPAptrFromOffset(cls->CPA, cls->nextFreeByteOffset), &opCode, sizeof(uint8_t)); cls->nextFreeByteOffset++;
	//TODO is this correct?
	//clEmitShaderRelocation(cls, &address);
	memcpy(getCPAptrFromOffset(cls->CPA, cls->nextFreeByteOffset), &address.offset, sizeof(uint32_t)); cls->nextFreeByteOffset += 4;
}

//input: 2 cls (cl, handles cl)
void clInsertBranchToSubList(ControlList* cls, ControlListAddress address)
{
	assert(cls);
	assert(cls->CPA);
	uint8_t opCode = V3D21_BRANCH_TO_SUB_LIST_opcode;
	memcpy(getCPAptrFromOffset(cls->CPA, cls->nextFreeByteOffset), &opCode, sizeof(uint8_t)); cls->nextFreeByteOffset++;
	//TODO is this correct?
	//clEmitShaderRelocation(cls, &address);
	memcpy(getCPAptrFromOffset(cls->CPA, cls->nextFreeByteOffset), &address.offset, sizeof(uint32_t)); cls->nextFreeByteOffset += 4;
}

void clInsertReturnFromSubList(ControlList* cl)
{
	assert(cl);
	assert(cl->CPA);
	uint8_t opCode = V3D21_RETURN_FROM_SUB_LIST_opcode;
	memcpy(getCPAptrFromOffset(cl->CPA, cl->nextFreeByteOffset), &opCode, sizeof(uint8_t));
	cl->nextFreeByteOffset++;
}

void clInsertIndexedPrimitiveList(ControlList* cl,
								  uint32_t maxIndex,
								  uint32_t indicesAddress,
								  uint32_t length,
								  uint32_t indexType, //0/1: 8 or 16 bit
								  enum V3D21_Primitive primitiveMode)
{
	assert(cl);
	assert(cl->CPA);
	uint8_t opCode = V3D21_INDEXED_PRIMITIVE_LIST_opcode;
	memcpy(getCPAptrFromOffset(cl->CPA, cl->nextFreeByteOffset), &opCode, sizeof(uint8_t)); cl->nextFreeByteOffset++;
	uint8_t tmp = moveBits(indexType, 4, 4) | moveBits(primitiveMode, 4, 0);
	memcpy(getCPAptrFromOffset(cl->CPA, cl->nextFreeByteOffset), &tmp, sizeof(uint8_t));  cl->nextFreeByteOffset++;
	memcpy(getCPAptrFromOffset(cl->CPA, cl->nextFreeByteOffset), &length, sizeof(uint32_t)); cl->nextFreeByteOffset += 4;
	memcpy(getCPAptrFromOffset(cl->CPA, cl->nextFreeByteOffset), &indicesAddress, sizeof(uint32_t)); cl->nextFreeByteOffset += 4;
	memcpy(getCPAptrFromOffset(cl->CPA, cl->nextFreeByteOffset), &maxIndex, sizeof(uint32_t)); cl->nextFreeByteOffset += 4;
}

void clInsertVertexArrayPrimitives(ControlList* cl,
								  uint32_t firstVertexIndex,
								  uint32_t length,
								  enum V3D21_Primitive primitiveMode)
{
	assert(cl);
	assert(cl->CPA);
	uint8_t opCode = V3D21_VERTEX_ARRAY_PRIMITIVES_opcode;
	memcpy(getCPAptrFromOffset(cl->CPA, cl->nextFreeByteOffset), &opCode, sizeof(uint8_t)); cl->nextFreeByteOffset++;
	uint8_t tmp = moveBits(primitiveMode, 8, 0);
	memcpy(getCPAptrFromOffset(cl->CPA, cl->nextFreeByteOffset), &tmp, sizeof(uint8_t));  cl->nextFreeByteOffset++;
	memcpy(getCPAptrFromOffset(cl->CPA, cl->nextFreeByteOffset), &length, sizeof(uint32_t)); cl->nextFreeByteOffset += 4;
	memcpy(getCPAptrFromOffset(cl->CPA, cl->nextFreeByteOffset), &firstVertexIndex, sizeof(uint32_t)); cl->nextFreeByteOffset += 4;
}

void clInsertShaderState(ControlList* cl,
						  uint32_t address,
						  uint32_t extendedShaderRecord, //0/1: true/false
						 uint32_t numberOfAttributeArrays)
{
	assert(cl);
	assert(cl->CPA);
	uint8_t opCode = V3D21_GL_SHADER_STATE_opcode;
	memcpy(getCPAptrFromOffset(cl->CPA, cl->nextFreeByteOffset), &opCode, sizeof(uint8_t)); cl->nextFreeByteOffset++;
	uint32_t tmp =	moveBits(address, 28, 4) |
			moveBits(extendedShaderRecord, 1, 3) |
			moveBits(numberOfAttributeArrays, 3, 0);
	memcpy(getCPAptrFromOffset(cl->CPA, cl->nextFreeByteOffset), &tmp, sizeof(uint32_t));
	cl->nextFreeByteOffset += 4;
}

void clInsertConfigurationBits(ControlList* cl,
						uint32_t earlyZUpdatesEnable, //0/1
						uint32_t earlyZEnable, //0/1
						uint32_t zUpdatesEnable, //0/1
						enum V3D21_Compare_Function depthTestFunction,
						uint32_t coverageReadMode, //0/1 clear/leave as is
						uint32_t coveragePipeSelect, //0/1
						uint32_t coverageUpdateMode, //0/1/2/3 nonzero, odd, or, zero
						uint32_t coverageReadType, //0/1 4*8bit, 16 bit mask
						uint32_t rasterizerOversampleMode, //0/1/2 none, 4x, 16x
						uint32_t enableDepthOffset, //0/1
						uint32_t clockwisePrimitives, //0/1
						uint32_t enableReverseFacingPrimitive, //0/1
						uint32_t enableForwardFacingPrimitive) //0/1
{
	assert(cl);
	assert(cl->CPA);
	uint8_t opCode = V3D21_CONFIGURATION_BITS_opcode;
	memcpy(getCPAptrFromOffset(cl->CPA, cl->nextFreeByteOffset), &opCode, sizeof(uint8_t)); cl->nextFreeByteOffset++;
	uint32_t tmp = moveBits(enableForwardFacingPrimitive, 1, 0) |
			moveBits(enableReverseFacingPrimitive, 1, 1) |
			moveBits(clockwisePrimitives, 1, 2) |
			moveBits(enableDepthOffset, 1, 3) |
			moveBits(coverageReadType, 1, 5) |
			moveBits(rasterizerOversampleMode, 2, 6) |
			moveBits(coveragePipeSelect, 1, 8) |
			moveBits(coverageUpdateMode, 2, 9) |
			moveBits(coverageReadMode, 1, 11) |
			moveBits(depthTestFunction, 3, 12) |
			moveBits(zUpdatesEnable, 1, 15) |
			moveBits(earlyZEnable, 1, 16) |
			moveBits(earlyZUpdatesEnable, 1, 17);
	memcpy(getCPAptrFromOffset(cl->CPA, cl->nextFreeByteOffset), &tmp, sizeof(uint32_t));
	cl->nextFreeByteOffset += 3;
}

void clInsertFlatShadeFlags(ControlList* cl,
						uint32_t flags)
{
	assert(cl);
	assert(cl->CPA);
	uint8_t opCode = V3D21_FLAT_SHADE_FLAGS_opcode;
	memcpy(getCPAptrFromOffset(cl->CPA, cl->nextFreeByteOffset), &opCode, sizeof(uint8_t)); cl->nextFreeByteOffset++;
	memcpy(getCPAptrFromOffset(cl->CPA, cl->nextFreeByteOffset), &flags, sizeof(uint32_t)); cl->nextFreeByteOffset += 4;
}

void clInsertPointSize(ControlList* cl,
						float size)
{
	assert(cl);
	assert(cl->CPA);
	uint8_t opCode = V3D21_POINT_SIZE_opcode;
	memcpy(getCPAptrFromOffset(cl->CPA, cl->nextFreeByteOffset), &opCode, sizeof(uint8_t)); cl->nextFreeByteOffset++;
	memcpy(getCPAptrFromOffset(cl->CPA, cl->nextFreeByteOffset), &size, sizeof(float)); cl->nextFreeByteOffset += 4;
}

void clInsertLineWidth(ControlList* cl,
						float width)
{
	assert(cl);
	assert(cl->CPA);
	uint8_t opCode = V3D21_LINE_WIDTH_opcode;
	memcpy(getCPAptrFromOffset(cl->CPA, cl->nextFreeByteOffset), &opCode, sizeof(uint8_t));	cl->nextFreeByteOffset++;
	memcpy(getCPAptrFromOffset(cl->CPA, cl->nextFreeByteOffset), &width, sizeof(float)); cl->nextFreeByteOffset += 4;
}

void clInsertRHTXBoundary(ControlList* cl,
						uint32_t boundary) //sint16
{
	assert(cl);
	assert(cl->CPA);
	uint8_t opCode = V3D21_RHT_X_BOUNDARY_opcode;
	memcpy(getCPAptrFromOffset(cl->CPA, cl->nextFreeByteOffset), &opCode, sizeof(uint8_t)); cl->nextFreeByteOffset++;
	uint16_t tmp = moveBits(boundary, 16, 0);
	memcpy(getCPAptrFromOffset(cl->CPA, cl->nextFreeByteOffset), &tmp, sizeof(uint16_t)); cl->nextFreeByteOffset += 2;
}

uint32_t f32_to_f187(float f32)
{
   uint32_t bits = *(uint32_t*)&f32;
   return bits >> 16;
}

void clInsertDepthOffset(ControlList* cl,
						float units,
						 float factor)
{
	assert(cl);
	assert(cl->CPA);
	uint8_t opCode = V3D21_DEPTH_OFFSET_opcode;
	memcpy(getCPAptrFromOffset(cl->CPA, cl->nextFreeByteOffset), &opCode, sizeof(uint8_t)); cl->nextFreeByteOffset++;
	uint32_t tmp = moveBits(f32_to_f187(factor), 16, 0) | moveBits(f32_to_f187(units), 16, 16);
	memcpy(getCPAptrFromOffset(cl->CPA, cl->nextFreeByteOffset), &tmp, sizeof(uint32_t)); cl->nextFreeByteOffset += 4;
}

void clInsertClipWindow(ControlList* cl,
						uint32_t width, //uint16
						uint32_t height, //uint16
						uint32_t bottomPixelCoord, //uint16
						uint32_t leftPixelCoord)  //uint16
{
	assert(cl);
	assert(cl->CPA);
	uint8_t opCode = V3D21_CLIP_WINDOW_opcode;
	memcpy(getCPAptrFromOffset(cl->CPA, cl->nextFreeByteOffset), &opCode, sizeof(uint8_t)); cl->nextFreeByteOffset++;
	uint32_t tmp = moveBits(leftPixelCoord, 16, 0) | moveBits(bottomPixelCoord, 16, 16);
	memcpy(getCPAptrFromOffset(cl->CPA, cl->nextFreeByteOffset), &tmp, sizeof(uint32_t)); cl->nextFreeByteOffset += 4;
	tmp = moveBits(width, 16, 0) | moveBits(height, 16, 16);
	memcpy(getCPAptrFromOffset(cl->CPA, cl->nextFreeByteOffset), &tmp, sizeof(uint32_t)); cl->nextFreeByteOffset += 4;
}

uint16_t get16bitSignedFixedNumber(float x)
{
	int32_t integerPart = roundf(x * 16.0f);
	return integerPart & 0xffff;
}

//viewport centre x/y coordinate
void clInsertViewPortOffset(ControlList* cl,
						float x,
						float y
						)
{
	assert(cl);
	assert(cl->CPA);
	uint8_t opCode = V3D21_VIEWPORT_OFFSET_opcode;
	memcpy(getCPAptrFromOffset(cl->CPA, cl->nextFreeByteOffset), &opCode, sizeof(uint8_t)); cl->nextFreeByteOffset++;
	//expects 16 bit signed fixed point number with 4 fractional bits
	uint16_t tmp = get16bitSignedFixedNumber(x);
	memcpy(getCPAptrFromOffset(cl->CPA, cl->nextFreeByteOffset), &tmp, sizeof(uint16_t)); cl->nextFreeByteOffset += 2;
	tmp = get16bitSignedFixedNumber(y);
	memcpy(getCPAptrFromOffset(cl->CPA, cl->nextFreeByteOffset), &tmp, sizeof(uint16_t)); cl->nextFreeByteOffset += 2;
}

void clInsertZMinMaxClippingPlanes(ControlList* cl,
						float minZw,
						float maxZw
						)
{
	assert(cl);
	assert(cl->CPA);
	uint8_t opCode = V3D21_Z_MIN_AND_MAX_CLIPPING_PLANES_opcode;
	memcpy(getCPAptrFromOffset(cl->CPA, cl->nextFreeByteOffset), &opCode, sizeof(uint8_t)); cl->nextFreeByteOffset++;
	memcpy(getCPAptrFromOffset(cl->CPA, cl->nextFreeByteOffset), &minZw, sizeof(float)); cl->nextFreeByteOffset += 4;
	memcpy(getCPAptrFromOffset(cl->CPA, cl->nextFreeByteOffset), &maxZw, sizeof(float)); cl->nextFreeByteOffset += 4;
}

void clInsertClipperXYScaling(ControlList* cl,
						float width, //half width in 1/16 of pixel
						float height //half height in 1/16 of pixel
						)
{
	assert(cl);
	assert(cl->CPA);
	uint8_t opCode = V3D21_CLIPPER_XY_SCALING_opcode;
	memcpy(getCPAptrFromOffset(cl->CPA, cl->nextFreeByteOffset), &opCode, sizeof(uint8_t)); cl->nextFreeByteOffset++;
	memcpy(getCPAptrFromOffset(cl->CPA, cl->nextFreeByteOffset), &width, sizeof(float)); cl->nextFreeByteOffset += 4;
	memcpy(getCPAptrFromOffset(cl->CPA, cl->nextFreeByteOffset), &height, sizeof(float)); cl->nextFreeByteOffset += 4;
}

void clInsertClipperZScaleOffset(ControlList* cl,
						float zOffset, //zc to zs
						float zScale //zc to zs
						)
{
	assert(cl);
	assert(cl->CPA);
	uint8_t opCode = V3D21_CLIPPER_Z_SCALE_AND_OFFSET_opcode;
	memcpy(getCPAptrFromOffset(cl->CPA, cl->nextFreeByteOffset), &opCode, sizeof(uint8_t)); cl->nextFreeByteOffset++;
	memcpy(getCPAptrFromOffset(cl->CPA, cl->nextFreeByteOffset), &zScale, sizeof(float)); cl->nextFreeByteOffset += 4;
	memcpy(getCPAptrFromOffset(cl->CPA, cl->nextFreeByteOffset), &zOffset, sizeof(float)); cl->nextFreeByteOffset += 4;
}

void clInsertTileBinningModeConfiguration(ControlList* cl,
						uint32_t doubleBufferInNonMsMode, //0/1
						uint32_t tileAllocationBlockSize, //0/1/2/3 32/64/128/256 bytes
						uint32_t tileAllocationInitialBlockSize, //0/1/2/3 32/64/128/256 bytes
						uint32_t autoInitializeTileStateDataArray, //0/1
						uint32_t tileBuffer64BitColorDepth, //0/1
						uint32_t multisampleMode4x, //0/1
						uint32_t widthInPixels,
						uint32_t heightInPixels,
						uint32_t tileStateDataArrayAddress, //16 byte aligned, size of 48 bytes * num tiles
						uint32_t tileAllocationMemorySize,
						uint32_t tileAllocationMemoryAddress
						)
{
	assert(cl);
	assert(cl->CPA);
	uint8_t opCode = V3D21_TILE_BINNING_MODE_CONFIGURATION_opcode;
	memcpy(getCPAptrFromOffset(cl->CPA, cl->nextFreeByteOffset), &opCode, sizeof(uint8_t)); cl->nextFreeByteOffset++;
	memcpy(getCPAptrFromOffset(cl->CPA, cl->nextFreeByteOffset), &tileAllocationMemoryAddress, sizeof(uint32_t)); cl->nextFreeByteOffset += 4;
	memcpy(getCPAptrFromOffset(cl->CPA, cl->nextFreeByteOffset), &tileAllocationMemorySize, sizeof(uint32_t)); cl->nextFreeByteOffset += 4;
	memcpy(getCPAptrFromOffset(cl->CPA, cl->nextFreeByteOffset), &tileStateDataArrayAddress, sizeof(uint32_t)); cl->nextFreeByteOffset += 4;
	uint32_t tileSizeW = 64;
	uint32_t tileSizeH = 64;

	if(multisampleMode4x)
	{
		tileSizeW >>= 1;
		tileSizeH >>= 1;
	}

	if(tileBuffer64BitColorDepth)
	{
		tileSizeH >>= 1;
	}

	uint8_t widthInTiles = divRoundUp(widthInPixels, tileSizeW);
	uint8_t heightInTiles = divRoundUp(heightInPixels, tileSizeH);
	memcpy(getCPAptrFromOffset(cl->CPA, cl->nextFreeByteOffset), &widthInTiles, sizeof(uint8_t)); cl->nextFreeByteOffset++;
	memcpy(getCPAptrFromOffset(cl->CPA, cl->nextFreeByteOffset), &heightInTiles, sizeof(uint8_t)); cl->nextFreeByteOffset++;
	uint8_t tmp = moveBits(multisampleMode4x, 1, 0) |
			moveBits(tileBuffer64BitColorDepth, 1, 1) |
			moveBits(autoInitializeTileStateDataArray, 1, 2) |
			moveBits(tileAllocationInitialBlockSize, 2, 3) |
			moveBits(tileAllocationBlockSize, 2, 5) |
			moveBits(doubleBufferInNonMsMode, 1, 7);
	memcpy(getCPAptrFromOffset(cl->CPA, cl->nextFreeByteOffset), &tmp, sizeof(uint8_t));
	cl->nextFreeByteOffset++;
}

void clInsertGEMRelocations(ControlList* cl,
							uint32_t buffer0,
							uint32_t buffer1)
{
	assert(cl);
	assert(cl->CPA);
	uint8_t opCode = V3D21_GEM_RELOCATIONS_opcode;
	memcpy(getCPAptrFromOffset(cl->CPA, cl->nextFreeByteOffset), &opCode, sizeof(uint8_t)); cl->nextFreeByteOffset++;
	memcpy(getCPAptrFromOffset(cl->CPA, cl->nextFreeByteOffset), &buffer0, sizeof(uint32_t)); cl->nextFreeByteOffset += 4;
	memcpy(getCPAptrFromOffset(cl->CPA, cl->nextFreeByteOffset), &buffer1, sizeof(uint32_t)); cl->nextFreeByteOffset += 4;
}

//input: 2 cls (cl, handles cl)
void clInsertShaderRecord(ControlList* cls,
						  ControlList* relocCl,
						  ControlList* handlesCl,
						  uint32_t handlesOffset,
						  uint32_t handlesSize,
						  uint32_t fragmentShaderIsSingleThreaded, //0/1
						  uint32_t pointSizeIncludedInShadedVertexData, //0/1
						  uint32_t enableClipping, //0/1
						  uint32_t fragmentNumberOfUsedUniforms,
						  uint32_t fragmentNumberOfVaryings,
						  uint32_t fragmentUniformsAddress,
						  ControlListAddress fragmentCodeAddress,
						  uint32_t vertexNumberOfUsedUniforms,
						  uint32_t vertexAttributeArraySelectBits,
						  uint32_t vertexTotalAttributesSize,
						  uint32_t vertexUniformsAddress,
						  ControlListAddress vertexCodeAddress,
						  uint32_t coordinateNumberOfUsedUniforms,
						  uint32_t coordinateAttributeArraySelectBits,
						  uint32_t coordinateTotalAttributesSize,
						  uint32_t coordinateUniformsAddress,
						  ControlListAddress coordinateCodeAddress)
{
	assert(cls);
	assert(cls->CPA);
	uint8_t tmp = moveBits(fragmentShaderIsSingleThreaded, 1, 0) |
			moveBits(pointSizeIncludedInShadedVertexData, 1, 1) |
			moveBits(enableClipping, 1, 2);
	memcpy(getCPAptrFromOffset(cls->CPA, cls->nextFreeByteOffset), &tmp, sizeof(uint8_t));
	cls->nextFreeByteOffset++;
	tmp = 0;
	memcpy(getCPAptrFromOffset(cls->CPA, cls->nextFreeByteOffset), &tmp, sizeof(uint8_t)); cls->nextFreeByteOffset++;
	uint16_t tmp2 = moveBits(fragmentNumberOfUsedUniforms, 16, 0);
	memcpy(getCPAptrFromOffset(cls->CPA, cls->nextFreeByteOffset), &tmp2, sizeof(uint16_t)); cls->nextFreeByteOffset++;
	memcpy(&tmp, getCPAptrFromOffset(cls->CPA, cls->nextFreeByteOffset), sizeof(uint8_t));
	tmp |= fragmentNumberOfVaryings;
	memcpy(getCPAptrFromOffset(cls->CPA, cls->nextFreeByteOffset), &tmp, sizeof(uint8_t)); cls->nextFreeByteOffset++;
	clEmitShaderRelocation(relocCl, handlesCl, handlesOffset, handlesSize, &fragmentCodeAddress);
	memcpy(getCPAptrFromOffset(cls->CPA, cls->nextFreeByteOffset), &fragmentCodeAddress.offset, sizeof(uint32_t)); cls->nextFreeByteOffset += 4;
	memcpy(getCPAptrFromOffset(cls->CPA, cls->nextFreeByteOffset), &fragmentUniformsAddress, sizeof(uint32_t)); cls->nextFreeByteOffset += 4;

	tmp2 = moveBits(vertexNumberOfUsedUniforms, 16, 0);
	memcpy(getCPAptrFromOffset(cls->CPA, cls->nextFreeByteOffset), &tmp2, sizeof(uint16_t)); cls->nextFreeByteOffset += 2;
	memcpy(getCPAptrFromOffset(cls->CPA, cls->nextFreeByteOffset), &vertexAttributeArraySelectBits, sizeof(uint8_t)); cls->nextFreeByteOffset++;
	memcpy(getCPAptrFromOffset(cls->CPA, cls->nextFreeByteOffset), &vertexTotalAttributesSize, sizeof(uint8_t)); cls->nextFreeByteOffset++;
	clEmitShaderRelocation(relocCl, handlesCl, handlesOffset, handlesSize, &vertexCodeAddress);
	//wtf??? --> shader code will always have an offset of 0 so this is fine
	uint32_t offset = moveBits(vertexCodeAddress.offset, 32, 0) | moveBits(vertexUniformsAddress, 32, 0);
	memcpy(getCPAptrFromOffset(cls->CPA, cls->nextFreeByteOffset), &offset, sizeof(uint32_t)); cls->nextFreeByteOffset += 4;
	cls->nextFreeByteOffset += 4;

	tmp2 = moveBits(coordinateNumberOfUsedUniforms, 16, 0);
	memcpy(getCPAptrFromOffset(cls->CPA, cls->nextFreeByteOffset), &tmp2, sizeof(uint16_t)); cls->nextFreeByteOffset += 2;
	memcpy(getCPAptrFromOffset(cls->CPA, cls->nextFreeByteOffset), &coordinateAttributeArraySelectBits, sizeof(uint8_t)); cls->nextFreeByteOffset++;
	memcpy(getCPAptrFromOffset(cls->CPA, cls->nextFreeByteOffset), &coordinateTotalAttributesSize, sizeof(uint8_t)); cls->nextFreeByteOffset++;
	clEmitShaderRelocation(relocCl, handlesCl, handlesOffset, handlesSize, &coordinateCodeAddress);
	memcpy(getCPAptrFromOffset(cls->CPA, cls->nextFreeByteOffset), &coordinateCodeAddress.offset, sizeof(uint32_t)); cls->nextFreeByteOffset += 4;
	memcpy(getCPAptrFromOffset(cls->CPA, cls->nextFreeByteOffset), &coordinateUniformsAddress, sizeof(uint32_t)); cls->nextFreeByteOffset += 4;
}

//input: 2 cls (cl, handles cl)
void clInsertAttributeRecord(ControlList* cls,
							 ControlList* relocCl,
							 ControlList* handlesCl,
							 uint32_t handlesOffset, uint32_t handlesSize,
						  ControlListAddress address,
						  uint32_t sizeBytes,
						  uint32_t stride,
						  uint32_t vertexVPMOffset,
						  uint32_t coordinateVPMOffset)
{
	assert(cls);
	assert(cls->CPA);
	uint32_t sizeBytesMinusOne = sizeBytes - 1;
	clEmitShaderRelocation(relocCl, handlesCl, handlesOffset, handlesSize, &address);
	memcpy(getCPAptrFromOffset(cls->CPA, cls->nextFreeByteOffset), &address.offset, sizeof(uint32_t)); cls->nextFreeByteOffset += 4;
	memcpy(getCPAptrFromOffset(cls->CPA, cls->nextFreeByteOffset), &sizeBytesMinusOne, sizeof(uint8_t)); cls->nextFreeByteOffset++;
	memcpy(getCPAptrFromOffset(cls->CPA, cls->nextFreeByteOffset), &stride, sizeof(uint8_t)); cls->nextFreeByteOffset++;
	memcpy(getCPAptrFromOffset(cls->CPA, cls->nextFreeByteOffset), &vertexVPMOffset, sizeof(uint8_t)); cls->nextFreeByteOffset++;
	memcpy(getCPAptrFromOffset(cls->CPA, cls->nextFreeByteOffset), &coordinateVPMOffset, sizeof(uint8_t)); cls->nextFreeByteOffset++;
}

uint32_t clGetHandleIndex(ControlList* handlesCl, uint32_t handlesOffset, uint32_t handlesSize, uint32_t handle)
{
	uint32_t c = 0;

	//if curr marker is closed already we need to work with the stored size
	uint32_t numHandles = (handlesSize ? handlesSize : (handlesCl->nextFreeByteOffset - handlesOffset)) / 4;

	for(; c < numHandles; ++c)
	{
		if(((uint32_t*)getCPAptrFromOffset(handlesCl->CPA, handlesOffset))[c] == handle)
		{
			//found
			return c;
		}
	}

	//write handle to handles cl
	memcpy(getCPAptrFromOffset(handlesCl->CPA, handlesCl->nextFreeByteOffset), &handle, sizeof(uint32_t));
	handlesCl->nextFreeByteOffset += 4;

	assert(handlesCl->nextFreeByteOffset < handlesCl->offset + handlesCl->blockSize * handlesCl->numBlocks);

	return c;
}

//input: 2 cls (cl + handles cl)
inline void clEmitShaderRelocation(ControlList* relocCl, ControlList* handlesCl, uint32_t handlesOffset, uint32_t handlesSize, const ControlListAddress* address)
{
	assert(relocCl);
	assert(relocCl->CPA);
	assert(handlesCl);
	assert(handlesCl->CPA);
	assert(address);
	assert(address->handle);

	//store offset within handles in cl
	uint32_t idx = clGetHandleIndex(handlesCl, handlesOffset, handlesSize, address->handle);
	memcpy(getCPAptrFromOffset(relocCl->CPA, relocCl->nextFreeByteOffset), &idx, sizeof(uint32_t));
	relocCl->nextFreeByteOffset += 4;
}

inline void clDummyRelocation(ControlList* relocCl, const ControlListAddress* address)
{}
