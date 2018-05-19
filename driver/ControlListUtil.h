#pragma once

#if defined (__cplusplus)
extern "C" {
#endif

#include <stdint.h>

typedef struct ControlListAddress
{
	uint32_t address;
	uint32_t offset;
} ControlListAddress;

static inline void clEmitShaderRelocation(struct ControlList* cl, const ControlListAddress* address);

#define __gen_user_data struct ControlList
#define __gen_address_type ControlListAddress
#define __gen_address_offset(reloc) ((reloc)->offset)
#define __gen_emit_reloc clEmitShaderRelocation

#include <broadcom/v3d_packet_v21_pack.h>

typedef struct ControlList
{
	uint8_t buffer[4092]; //TODO size?
	uint8_t* nextFreeByte; //pointer to the next available free byte
} ControlList;

//move bits to offset, mask rest to 0
uint32_t moveBits(uint32_t d, uint32_t bits, uint32_t offset)
{
	return (d << offset) & (~(~0 << bits) << offset);
}

void clInit(ControlList* cl)
{
	assert(cl);
	cl->nextFreeByte = &buffer[0];
}

void clInsertHalt(ControlList* cl)
{
	assert(cl);
	assert(cl->nextFreeByte);
	*cl->nextFreeByte = V3D21_HALT_opcode;
	cl->nextFreeByte++;
}

void clInsertNop(ControlList* cl)
{
	assert(cl);
	assert(cl->nextFreeByte);
	*cl->nextFreeByte = V3D21_NOP_opcode;
	cl->nextFreeByte++;
}

void clInsertFlush(ControlList* cl)
{
	assert(cl);
	assert(cl->nextFreeByte);
	*cl->nextFreeByte = V3D21_FLUSH_opcode;
	cl->nextFreeByte++;
}

void clInsertFlushAllState(ControlList* cl)
{
	assert(cl);
	assert(cl->nextFreeByte);
	*cl->nextFreeByte = V3D21_FLUSH_ALL_STATE_opcode;
	cl->nextFreeByte++;
}

void clInsertStartTileBinning(ControlList* cl)
{
	assert(cl);
	assert(cl->nextFreeByte);
	*cl->nextFreeByte = V3D21_START_TILE_BINNING_opcode;
	cl->nextFreeByte++;
}

void clInsertIncrementSemaphore(ControlList* cl)
{
	assert(cl);
	assert(cl->nextFreeByte);
	*cl->nextFreeByte = V3D21_INCREMENT_SEMAPHORE_opcode;
	cl->nextFreeByte++;
}

void clInsertWaitOnSemaphore(ControlList* cl)
{
	assert(cl);
	assert(cl->nextFreeByte);
	*cl->nextFreeByte = V3D21_WAIT_ON_SEMAPHORE_opcode;
	cl->nextFreeByte++;
}

void clInsertBranch(ControlList* cl)
{
	assert(cl);
	assert(cl->nextFreeByte);
	*cl->nextFreeByte = V3D21_BRANCH_opcode;
	//TODO
	cl->nextFreeByte++;
}

void clInsertBranchToSubList(ControlList* cl)
{
	assert(cl);
	assert(cl->nextFreeByte);
	*cl->nextFreeByte = V3D21_BRANCH_TO_SUB_LIST_opcode;
	//TODO
	cl->nextFreeByte++;
}

void clInsertReturnFromSubList(ControlList* cl)
{
	assert(cl);
	assert(cl->nextFreeByte);
	*cl->nextFreeByte = V3D21_RETURN_FROM_SUB_LIST_opcode;
	cl->nextFreeByte++;
}

void clInsertStoreMultiSampleResolvedTileColorBuffer(ControlList* cl)
{
	assert(cl);
	assert(cl->nextFreeByte);
	*cl->nextFreeByte = V3D21_STORE_MULTI_SAMPLE_RESOLVED_TILE_COLOR_BUFFER_opcode;
	cl->nextFreeByte++;
}

void clInsertStoreMultiSampleResolvedTileColorBufferAndEOF(ControlList* cl)
{
	assert(cl);
	assert(cl->nextFreeByte);
	*cl->nextFreeByte = V3D21_STORE_MULTI_SAMPLE_RESOLVED_TILE_COLOR_BUFFER_AND_EOF_opcode;
	cl->nextFreeByte++;
}

void clInsertStoreFullResolutionTileBuffer(ControlList* cl)
{
	assert(cl);
	assert(cl->nextFreeByte);
	*cl->nextFreeByte = V3D21_STORE_FULL_RESOLUTION_TILE_BUFFER_opcode;
	//TODO
	cl->nextFreeByte++;
}

void clInsertReLoadFullResolutionTileBuffer(ControlList* cl)
{
	assert(cl);
	assert(cl->nextFreeByte);
	*cl->nextFreeByte = V3D21_RE_LOAD_FULL_RESOLUTION_TILE_BUFFER_opcode;
	//TODO
	cl->nextFreeByte++;
}

void clInsertStoreTileBufferGeneral(ControlList* cl)
{
	assert(cl);
	assert(cl->nextFreeByte);
	*cl->nextFreeByte = V3D21_STORE_TILE_BUFFER_GENERAL_opcode;
	//TODO
	cl->nextFreeByte++;
}

void clInsertLoadTileBufferGeneral(ControlList* cl)
{
	assert(cl);
	assert(cl->nextFreeByte);
	*cl->nextFreeByte = V3D21_LOAD_TILE_BUFFER_GENERAL_opcode;
	//TODO
	cl->nextFreeByte++;
}

void clInsertIndexedPrimitiveList(ControlList* cl,
								  uint32_t maxIndex,
								  uint32_t indicesAddress,
								  uint32_t length,
								  uint32_t indexType, //0/1: 8 or 16 bit
								  enum V3D21_Primitive primitiveMode)
{
	assert(cl);
	assert(cl->nextFreeByte);
	*cl->nextFreeByte = V3D21_INDEXED_PRIMITIVE_LIST_opcode; cl->nextFreeByte++;
	*cl->nextFreeByte = moveBits(indexType, 4, 4) | moveBits(primitiveMode, 4, 0); cl->nextFreeByte++;
	*(uint32_t*)cl->nextFreeByte = length; cl->nextFreeByte += 4;
	*(uint32_t*)cl->nextFreeByte = indicesAddress; cl->nextFreeByte += 4;
	*(uint32_t*)cl->nextFreeByte = maxIndex; cl->nextFreeByte += 4;
}

void clInsertVertexArrayPrimitives(ControlList* cl,
								  uint32_t firstVertexIndex,
								  uint32_t length,
								  enum V3D21_Primitive primitiveMode)
{
	assert(cl);
	assert(cl->nextFreeByte);
	*cl->nextFreeByte = V3D21_VERTEX_ARRAY_PRIMITIVES_opcode; cl->nextFreeByte++;
	*cl->nextFreeByte = moveBits(primitiveMode, 8, 0); cl->nextFreeByte++;
	*(uint32_t*)cl->nextFreeByte = length; cl->nextFreeByte += 4;
	*(uint32_t*)cl->nextFreeByte = firstVertexIndex; cl->nextFreeByte += 4;
}

void clInsertPrimitiveListFormat(ControlList* cl,
								  uint32_t dataType, //1/3: 16 or 32 bit
								  uint32_t primitiveType) //0/1/2/3: point/line/tri/rhy
{
	assert(cl);
	assert(cl->nextFreeByte);
	*cl->nextFreeByte = V3D21_PRIMITIVE_LIST_FORMAT_opcode; cl->nextFreeByte++;
	*cl->nextFreeByte = moveBits(dataType, 4, 4) | moveBits(primitiveType, 4, 0); cl->nextFreeByte++;
}

void clInsertShaderState(ControlList* cl,
						  uint32_t address,
						  uint32_t extendedShaderRecord, //0/1: true/false
						 uint32_t numberOfAttributeArrays)
{
	assert(cl);
	assert(cl->nextFreeByte);
	*cl->nextFreeByte = V3D21_GL_SHADER_STATE_opcode; cl->nextFreeByte++;
	//TODO is this correct?
	*(uint32_t*)cl->nextFreeByte =
			moveBits(address, 28, 4) |
			moveBits(extendedShaderRecord, 1, 3) |
			moveBits(numberOfAttributeArrays, 3, 0); cl->nextFreeByte += 4;
}

void clInsertClearColors(ControlList* cl,
						uint32_t clearStencil,
						uint32_t clearZ, //24 bit Z
						uint64_t clearColor) //2x RGBA8 or 1x RGBA16
{
	assert(cl);
	assert(cl->nextFreeByte);
	*cl->nextFreeByte = V3D21_CLEAR_COLORS_opcode; cl->nextFreeByte++;
	*(uint64_t*)cl->nextFreeByte = clearColor; cl->nextFreeByte += 8;
	*(uint32_t*)cl->nextFreeByte = clearZ; cl->nextFreeByte += 4; //24 bits for Z, 8 bit for vg mask (unused)
	*cl->nextFreeByte = clearStencil; cl->nextFreeByte++;
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
	assert(cl->nextFreeByte);
	*cl->nextFreeByte = V3D21_CONFIGURATION_BITS_opcode; cl->nextFreeByte++;
	*(uint32_t)cl->nextFreeByte =
			moveBits(enableForwardFacingPrimitive, 1, 0) |
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
			moveBits(earlyZUpdatesEnable, 1, 17); cl->nextFreeByte += 4;
}

void clInsertFlatShadeFlags(ControlList* cl,
						uint32_t flags)
{
	assert(cl);
	assert(cl->nextFreeByte);
	*cl->nextFreeByte = V3D21_FLAT_SHADE_FLAGS_opcode; cl->nextFreeByte++;
	*(uint32_t*)cl->nextFreeByte = flags; cl->nextFreeByte += 4;
}

void clInsertPointSize(ControlList* cl,
						float size)
{
	assert(cl);
	assert(cl->nextFreeByte);
	*cl->nextFreeByte = V3D21_POINT_SIZE_opcode; cl->nextFreeByte++;
	*(float*)cl->nextFreeByte = size; cl->nextFreeByte += 4;
}

void clInsertLineWidth(ControlList* cl,
						float width)
{
	assert(cl);
	assert(cl->nextFreeByte);
	*cl->nextFreeByte = V3D21_LINE_WIDTH_opcode; cl->nextFreeByte++;
	*(float*)cl->nextFreeByte = width; cl->nextFreeByte += 4;
}

void clInsertRHTXBoundary(ControlList* cl,
						uint32_t boundary) //sint16
{
	assert(cl);
	assert(cl->nextFreeByte);
	*cl->nextFreeByte = V3D21_RHT_X_BOUNDARY_opcode; cl->nextFreeByte++;
	*(uint32_t*)cl->nextFreeByte = moveBits(boundary, 16, 0); cl->nextFreeByte += 2;
}

void clInsertDepthOffset(ControlList* cl,
						uint32_t units, //float 187
						 uint32_t factor) //float 187
{
	assert(cl);
	assert(cl->nextFreeByte);
	*cl->nextFreeByte = V3D21_DEPTH_OFFSET_opcode; cl->nextFreeByte++;
	*(uint32_t*)cl->nextFreeByte = moveBits(factor, 16, 0) | moveBits(units, 16, 16); cl->nextFreeByte += 4;
}

void clInsertClipWindow(ControlList* cl,
						uint32_t width, //uint16
						uint32_t height, //uint16
						uint32_t bottomPixelCoord, //uint16
						uint32_t leftPixelCoord)  //uint16
{
	assert(cl);
	assert(cl->nextFreeByte);
	*cl->nextFreeByte = V3D21_CLIP_WINDOW_opcode; cl->nextFreeByte++;
	*(uint32_t*)cl->nextFreeByte = moveBits(leftPixelCoord, 16, 0) | moveBits(bottomPixelCoord, 16, 16); cl->nextFreeByte += 4;
	*(uint32_t*)cl->nextFreeByte = moveBits(width, 16, 0) | moveBits(height, 16, 16); cl->nextFreeByte += 4;
}

void clInsertViewPortOffset(ControlList* cl,
						uint32_t x, //sint16
						uint32_t y, //sint16
						)
{
	assert(cl);
	assert(cl->nextFreeByte);
	*cl->nextFreeByte = V3D21_VIEWPORT_OFFSET_opcode; cl->nextFreeByte++;
	*(uint32_t*)cl->nextFreeByte = moveBits(x, 16, 0) | moveBits(y, 16, 16); cl->nextFreeByte += 4;
}

void clInsertZMinMaxClippingPlanes(ControlList* cl,
						float minZw,
						float maxZw,
						)
{
	assert(cl);
	assert(cl->nextFreeByte);
	*cl->nextFreeByte = V3D21_Z_MIN_AND_MAX_CLIPPING_PLANES_opcode; cl->nextFreeByte++;
	*(float*)cl->nextFreeByte = minZw; cl->nextFreeByte += 4;
	*(float*)cl->nextFreeByte = maxZw; cl->nextFreeByte += 4;
}

void clInsertClipperXYScaling(ControlList* cl,
						float width, //half height in 1/16 of pixel
						float height, //half width in 1/16 of pixel
						)
{
	assert(cl);
	assert(cl->nextFreeByte);
	*cl->nextFreeByte = V3D21_CLIPPER_XY_SCALING_opcode; cl->nextFreeByte++;
	*(float*)cl->nextFreeByte = width; cl->nextFreeByte += 4;
	*(float*)cl->nextFreeByte = height; cl->nextFreeByte += 4;
}

void clInsertClipperZScaleOffset(ControlList* cl,
						float zOffset, //zc to zs
						float zScale, //zc to zs
						)
{
	assert(cl);
	assert(cl->nextFreeByte);
	*cl->nextFreeByte = V3D21_CLIPPER_Z_SCALE_AND_OFFSET_opcode; cl->nextFreeByte++;
	*(float*)cl->nextFreeByte = zScale; cl->nextFreeByte += 4;
	*(float*)cl->nextFreeByte = zOffset; cl->nextFreeByte += 4;
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
	assert(cl->nextFreeByte);
	*cl->nextFreeByte = V3D21_TILE_BINNING_MODE_CONFIGURATION_opcode; cl->nextFreeByte++;
	*(uint32_t*)cl->nextFreeByte = tileAllocationMemoryAddress; cl->nextFreeByte += 4;
	*(uint32_t*)cl->nextFreeByte = tileAllocationMemorySize; cl->nextFreeByte += 4;
	*(uint32_t*)cl->nextFreeByte = tileStateDataArrayAddress; cl->nextFreeByte += 4;
	*(uint32_t*)cl->nextFreeByte = widthInPixels; cl->nextFreeByte += 4;
	*cl->nextFreeByte =
			moveBits(multisampleMode4x, 1, 0) |
			moveBits(tileBuffer64BitColorDepth, 1, 1) |
			moveBits(autoInitializeTileStateDataArray, 1, 2) |
			moveBits(tileAllocationInitialBlockSize, 2, 3) |
			moveBits(tileAllocationBlockSize, 2, 5) |
			moveBits(doubleBufferInNonMsMode, 1, 7); cl->nextFreeByte++;
}

void clInsertTileRenderingModeConfiguration(ControlList* cl,
						uint32_t doubleBufferInNonMsMode, //0/1
						uint32_t earlyZEarlyCovDisable, //0/1
						uint32_t earlyZUpdateDirection, //0/1 lt,le/gt,ge
						uint32_t selectCoverageMode, //0/1
						uint32_t memoryFormat, //0/1/2 linear/t/lt
						uint32_t decimateMode, //0/1/2 0x/4x/16x
						uint32_t nonHDRFrameFormatColorFormat, //0/1/2 bgr565dithered/rgba8/bgr565nodither
						uint32_t multisampleMode4x, //0/1
						uint32_t widthPixels,
						uint32_t heightPixels,
						ControlListAddress memoryAddress)
{
	assert(cl);
	assert(cl->nextFreeByte);
	*cl->nextFreeByte = V3D21_TILE_RENDERING_MODE_CONFIGURATION_opcode; cl->nextFreeByte++;
	//TODO
}

void clInsertTileCoordinates(ControlList* cl,
						uint32_t tileColumnNumber, //int8
						uint32_t tileRowNumber) //int8
{
	assert(cl);
	assert(cl->nextFreeByte);
	*cl->nextFreeByte = V3D21_TILE_COORDINATES_opcode; cl->nextFreeByte++;
	*(uint32_t*)cl->nextFreeByte = moveBits(tileColumnNumber, 8, 0) | moveBits(tileRowNumber, 8, 8); cl->nextFreeByte += 2;
}

void clInsertGEMRelocations(ControlList* cl,
							uint32_t buffer0,
							uint32_t buffer1)
{
	assert(cl);
	assert(cl->nextFreeByte);
	*cl->nextFreeByte = V3D21_GEM_RELOCATIONS_opcode; cl->nextFreeByte++;
	*(uint32_t*)cl->nextFreeByte = buffer0; cl->nextFreeByte += 4;
	*(uint32_t*)cl->nextFreeByte = buffer1; cl->nextFreeByte += 4;
}

void clInsertShaderRecord(ControlList* cl,
						  uint32_t fragmentShaderIsSingleThreaded, //0/1
						  uint32_t pointSizeIncludedInShadedVertexData, //0/1
						  uint32_t enableClipping, //0/1
						  uint32_t fragmentNumberOfUnusedUniforms,
						  uint32_t fragmentNumberOfVaryings,
						  ControlListAddress fragmentCodeAddress,
						  uint32_t fragmentUniformsAddress,
						  uint32_t vertexNumberOfUnusedUniforms,
						  uint32_t vertexAttributeArraySelectBits,
						  uint32_t vertexTotalAttributesSize,
						  ControlListAddress vertexCodeAddress,
						  uint32_t vertexUniformsAddress)
{
	assert(cl);
	assert(cl->nextFreeByte);
	//TODO
}

void clInsertAttributeRecord(ControlList* cl,
						  ControlListAddress address,
						  uint32_t sizeBytes,
						  uint32_t stride,
						  uint32_t vertexVPMOffset)
{
	assert(cl);
	assert(cl->nextFreeByte);
	uint32_t sizeBytesMinusOne = sizeBytes - 1;
	//TODO
}

static inline void clEmitShaderRelocation(struct ControlList* cl, const ControlListAddress* address)
{
	//TODO
}

#if defined (__cplusplus)
}
#endif
