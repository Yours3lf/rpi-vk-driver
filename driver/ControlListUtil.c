#include "ControlListUtil.h"

#include <stdint.h>

uint32_t divRoundUp(uint32_t n, uint32_t d)
{
	return (((n) + (d) - 1) / (d));
}

//move bits to offset, mask rest to 0
uint32_t moveBits(uint32_t d, uint32_t bits, uint32_t offset)
{
	return (d << offset) & (~(~0 << bits) << offset);
}

uint32_t clHasEnoughSpace(ControlList* cl, uint32_t size)
{
	assert(cl);
	assert(cl->buffer);
	assert(cl->nextFreeByte);
	uint32_t currSize = cl->nextFreeByte - cl->buffer;
	if(currSize + size < cl->numBlocks * cl->blockSize - 4)
	{
		return 1; //fits!
	}
	else
	{
		return 0; //need to reallocate
	}
}

void clInit(ControlList* cl, void* buffer, uint32_t blockSize)
{
	assert(cl);
	assert(buffer);
	cl->buffer = buffer;
	cl->numBlocks = 1;
	cl->blockSize = blockSize;
	cl->nextFreeByte = &cl->buffer[0];
	cl->currMarker = 0;
}

void clInsertNewCLMarker(ControlList* cl,
						 ControlList* handlesCL,
						 ControlList* shaderRecCL,
						 uint32_t shaderRecCount,
						 ControlList* uniformsCL)
{
	//to be inserted when you'd insert tile binning mode config
	assert(cl);
	assert(handlesCL);
	assert(shaderRecCL);
	assert(uniformsCL);

	CLMarker marker = {};
	marker.handlesBuf = handlesCL->buffer;
	marker.shaderRecBuf = shaderRecCL->buffer;
	marker.uniformsBuf = uniformsCL->buffer;

	//close current marker
	if(cl->currMarker && !cl->currMarker->size)
	{
		clCloseCurrentMarker(cl, handlesCL, shaderRecCL, shaderRecCount, uniformsCL);
	}

	//if this is not the first marker
	if(cl->currMarker)
	{
		marker.handlesBuf = cl->currMarker->handlesBuf + cl->currMarker->handlesSize;
		marker.shaderRecBuf = cl->currMarker->shaderRecBuf + cl->currMarker->shaderRecSize;
		marker.uniformsBuf = cl->currMarker->uniformsBuf + cl->currMarker->uniformsSize;
		marker.shaderRecCount = cl->currMarker->shaderRecCount; //initialize with previous marker's data
	}

	*(CLMarker*)cl->nextFreeByte = marker;
	if(cl->currMarker)
	{
		cl->currMarker->nextMarker = cl->nextFreeByte;
	}
	cl->currMarker = cl->nextFreeByte;
	cl->nextFreeByte += sizeof(CLMarker);
}

void clCloseCurrentMarker(ControlList* cl, ControlList* handlesCL, ControlList* shaderRecCL, uint32_t shaderRecCount, ControlList* uniformsCL)
{
	assert(cl);
	assert(handlesCL);
	assert(shaderRecCL);
	assert(uniformsCL);
	assert(cl->currMarker);
	cl->currMarker->size = cl->nextFreeByte - ((uint8_t*)cl->currMarker + sizeof(CLMarker));
	cl->currMarker->handlesSize = handlesCL->nextFreeByte - cl->currMarker->handlesBuf;
	cl->currMarker->shaderRecSize = shaderRecCL->nextFreeByte - cl->currMarker->shaderRecBuf;
	cl->currMarker->uniformsSize = uniformsCL->nextFreeByte - cl->currMarker->uniformsBuf;
	cl->currMarker->shaderRecCount = shaderRecCount - cl->currMarker->shaderRecCount; //update shader rec count to reflect added shader recs
}

void clInsertData(ControlList* cl, uint32_t size, uint8_t* data)
{
	assert(cl);
	memcpy(cl->nextFreeByte, data, size);
	cl->nextFreeByte += size;
}

void clInsertUniformConstant(ControlList* cl, uint32_t data)
{
	assert(cl);
	*(uint32_t*)cl->nextFreeByte = data;
	cl->nextFreeByte += 4;
}

void clInsertUniformXYScale(ControlList* cl, float data)
{
	assert(cl);
	*(float*)cl->nextFreeByte = data;
	cl->nextFreeByte += 4;
}

void clInsertUniformZOffset(ControlList* cl, float data)
{
	assert(cl);
	*(float*)cl->nextFreeByte = data;
	cl->nextFreeByte += 4;
}

void clInsertHalt(ControlList* cl)
{
	assert(cl);
	assert(cl->buffer);
	assert(cl->nextFreeByte);
	*cl->nextFreeByte = V3D21_HALT_opcode;
	cl->nextFreeByte++;
}

void clInsertNop(ControlList* cl)
{
	assert(cl);
	assert(cl->buffer);
	assert(cl->nextFreeByte);
	*cl->nextFreeByte = V3D21_NOP_opcode;
	cl->nextFreeByte++;
}

void clInsertFlush(ControlList* cl)
{
	assert(cl);
	assert(cl->buffer);
	assert(cl->nextFreeByte);
	*cl->nextFreeByte = V3D21_FLUSH_opcode;
	cl->nextFreeByte++;
}

void clInsertFlushAllState(ControlList* cl)
{
	assert(cl);
	assert(cl->buffer);
	assert(cl->nextFreeByte);
	*cl->nextFreeByte = V3D21_FLUSH_ALL_STATE_opcode;
	cl->nextFreeByte++;
}

void clInsertStartTileBinning(ControlList* cl)
{
	assert(cl);
	assert(cl->buffer);
	assert(cl->nextFreeByte);
	*cl->nextFreeByte = V3D21_START_TILE_BINNING_opcode;
	cl->nextFreeByte++;
}

void clInsertIncrementSemaphore(ControlList* cl)
{
	assert(cl);
	assert(cl->buffer);
	assert(cl->nextFreeByte);
	*cl->nextFreeByte = V3D21_INCREMENT_SEMAPHORE_opcode;
	cl->nextFreeByte++;
}

void clInsertWaitOnSemaphore(ControlList* cl)
{
	assert(cl);
	assert(cl->buffer);
	assert(cl->nextFreeByte);
	*cl->nextFreeByte = V3D21_WAIT_ON_SEMAPHORE_opcode;
	cl->nextFreeByte++;
}

//input: 2 cls (cl, handles cl)
void clInsertBranch(ControlList* cls, ControlListAddress address)
{
	assert(cls);
	assert(cls->buffer);
	assert(cls->nextFreeByte);
	*cls->nextFreeByte = V3D21_BRANCH_opcode; cls->nextFreeByte++;
	//TODO is this correct?
	//clEmitShaderRelocation(cls, &address);
	*(uint32_t*)cls->nextFreeByte = address.offset; cls->nextFreeByte += 4;
}

//input: 2 cls (cl, handles cl)
void clInsertBranchToSubList(ControlList* cls, ControlListAddress address)
{
	assert(cls);
	assert(cls->buffer);
	assert(cls->nextFreeByte);
	*cls->nextFreeByte = V3D21_BRANCH_TO_SUB_LIST_opcode; cls->nextFreeByte++;
	//TODO is this correct?
	//clEmitShaderRelocation(cls, &address);
	*(uint32_t*)cls->nextFreeByte = address.offset; cls->nextFreeByte += 4;
}

void clInsertReturnFromSubList(ControlList* cl)
{
	assert(cl);
	assert(cl->buffer);
	assert(cl->nextFreeByte);
	*cl->nextFreeByte = V3D21_RETURN_FROM_SUB_LIST_opcode;
	cl->nextFreeByte++;
}

/*void clInsertStoreMultiSampleResolvedTileColorBuffer(ControlList* cl)
{
	assert(cl);
	assert(cl->buffer);
	assert(cl->nextFreeByte);
	*cl->nextFreeByte = V3D21_STORE_MULTI_SAMPLE_RESOLVED_TILE_COLOR_BUFFER_opcode;
	cl->nextFreeByte++;
}*/

/*void clInsertStoreMultiSampleResolvedTileColorBufferAndEOF(ControlList* cl)
{
	assert(cl);
	assert(cl->buffer);
	assert(cl->nextFreeByte);
	*cl->nextFreeByte = V3D21_STORE_MULTI_SAMPLE_RESOLVED_TILE_COLOR_BUFFER_AND_EOF_opcode;
	cl->nextFreeByte++;
}*/

/*
//input: 2 cls (cl, handles cl)
void clInsertStoreFullResolutionTileBuffer(ControlList* cls,
										   ControlListAddress address,
										   uint32_t lastTile, //0/1
										   uint32_t disableClearOnWrite, //0/1
										   uint32_t disableZStencilBufferWrite, //0/1
										   uint32_t disableColorBufferWrite) //0/1
{
	assert(cls);
	assert(cls->buffer);
	assert(cls->nextFreeByte);
	*cls->nextFreeByte = V3D21_STORE_FULL_RESOLUTION_TILE_BUFFER_opcode; cls->nextFreeByte++;
	//TODO is this correct?
	clEmitShaderRelocation(cls, &address);
	*(uint32_t*)cls->nextFreeByte =
			moveBits(disableColorBufferWrite, 1, 0) |
			moveBits(disableZStencilBufferWrite, 1, 1) |
			moveBits(disableClearOnWrite, 1, 2) |
			moveBits(lastTile, 1, 3) |
			moveBits(address.offset, 28, 4);
	cls->nextFreeByte += 4;
}
*/

/*
//input: 2 cls (cl, handles cl)
void clInsertReLoadFullResolutionTileBuffer(ControlList* cls,
											ControlListAddress address,
											uint32_t disableZStencilBufferRead, //0/1
											uint32_t disableColorBufferRead) //0/1
{
	assert(cls);
	assert(cls->buffer);
	assert(cls->nextFreeByte);
	*cls->nextFreeByte = V3D21_RE_LOAD_FULL_RESOLUTION_TILE_BUFFER_opcode; cls->nextFreeByte++;
	//TODO is this correct?
	clEmitShaderRelocation(cls, &address);
	*(uint32_t*)cls->nextFreeByte =
			moveBits(disableColorBufferRead, 1, 0) |
			moveBits(disableZStencilBufferRead, 1, 1) |
			moveBits(address.offset, 28, 4);
	cls->nextFreeByte += 4;
}
*/

/*
//input: 2 cls (cl, handles cl)
void clInsertStoreTileBufferGeneral(ControlList* cls,
									ControlListAddress address,
									uint32_t lastTileOfFrame, //0/1
									uint32_t disableZStencilBufferDump, //0/1
									uint32_t disableColorBufferDump, //0/1
									uint32_t disableZStencilBufferClearOnStoreDump, //0/1
									uint32_t disableColorBufferClearOnStoreDump, //0/1
									uint32_t disableDoubleBufferSwap, //0/1
									uint32_t pixelColorFormat, //0/1/2 RGBA8/BGR565dither/BGR565nodither
									uint32_t mode, //0/1/2 sample0/decimate4x/decimate16x
									uint32_t format, //0/1/2 raster/t/lt
									uint32_t bufferToStore) //0/1/2/3/5 none/color/zstencil/z/full
{
	assert(cls);
	assert(cls->buffer);
	assert(cls->nextFreeByte);
	*cls->nextFreeByte = V3D21_STORE_TILE_BUFFER_GENERAL_opcode; cls->nextFreeByte++;
	//TODO is this correct?
	*cls->nextFreeByte =
			moveBits(bufferToStore, 3, 0) |
			moveBits(format, 2, 4) |
			moveBits(mode, 2, 6);
	cls->nextFreeByte++;
	*cls->nextFreeByte =
			moveBits(pixelColorFormat, 2, 0) |
			moveBits(disableDoubleBufferSwap, 1, 4) |
			moveBits(disableColorBufferClearOnStoreDump, 1, 5) |
			moveBits(disableZStencilBufferClearOnStoreDump, 1, 6) |
			moveBits(1, 1, 7); //disable vg mask
	cls->nextFreeByte++;
	clEmitShaderRelocation(cls, &address);
	*(uint32_t*)cls->nextFreeByte =
			moveBits(disableColorBufferDump, 1, 0) |
			moveBits(disableZStencilBufferDump, 1, 1) |
			moveBits(1, 1, 2) | //disable vg mask
			moveBits(lastTileOfFrame, 1, 3) |
			moveBits(address.offset, 28, 4);
	cls->nextFreeByte += 4;
}
*/

/*
//input: 2 cls (cl, handles cl)
void clInsertLoadTileBufferGeneral(ControlList* cls,
								   ControlListAddress address,
								   uint32_t disableZStencilBufferLoad, //0/1
								   uint32_t disableColorBufferLoad, //0/1
								   uint32_t pixelColorFormat, //0/1/2 RGBA8/BGR565dither/BGR565nodither
								   uint32_t mode, //0/1/2 sample0/decimate4x/decimate16x
								   uint32_t format, //0/1/2 raster/t/lt
								   uint32_t bufferToLoad) //0/1/2/3/5 none/color/zstencil/z/full
{
	assert(cls);
	assert(cls->buffer);
	assert(cls->nextFreeByte);
	*cls->nextFreeByte = V3D21_LOAD_TILE_BUFFER_GENERAL_opcode; cls->nextFreeByte++;
	//TODO is this correct?
	*cls->nextFreeByte =
			moveBits(bufferToLoad, 3, 0) |
			moveBits(format, 2, 4);
	cls->nextFreeByte++;
	*cls->nextFreeByte =
			moveBits(pixelColorFormat, 2, 0);
	cls->nextFreeByte++;
	clEmitShaderRelocation(cls, &address);
	*(uint32_t*)cls->nextFreeByte =
			moveBits(disableColorBufferLoad, 1, 0) |
			moveBits(disableZStencilBufferLoad, 1, 1) |
			moveBits(1, 1, 2) | //disable vg mask
			moveBits(address.offset, 28, 4);
	cls->nextFreeByte += 4;

}
*/

void clInsertIndexedPrimitiveList(ControlList* cl,
								  uint32_t maxIndex,
								  uint32_t indicesAddress,
								  uint32_t length,
								  uint32_t indexType, //0/1: 8 or 16 bit
								  enum V3D21_Primitive primitiveMode)
{
	assert(cl);
	assert(cl->buffer);
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

/*void clInsertPrimitiveListFormat(ControlList* cl,
								  uint32_t dataType, //1/3: 16 or 32 bit
								  uint32_t primitiveType) //0/1/2/3: point/line/tri/rhy
{
	assert(cl);
	assert(cl->buffer);
	assert(cl->nextFreeByte);
	*cl->nextFreeByte = V3D21_PRIMITIVE_LIST_FORMAT_opcode; cl->nextFreeByte++;
	*cl->nextFreeByte = moveBits(dataType, 4, 4) | moveBits(primitiveType, 4, 0); cl->nextFreeByte++;
}*/

void clInsertShaderState(ControlList* cl,
						  uint32_t address,
						  uint32_t extendedShaderRecord, //0/1: true/false
						 uint32_t numberOfAttributeArrays)
{
	assert(cl);
	assert(cl->nextFreeByte);
	*cl->nextFreeByte = V3D21_GL_SHADER_STATE_opcode; cl->nextFreeByte++;
	*(uint32_t*)cl->nextFreeByte =
			moveBits(address, 28, 4) |
			moveBits(extendedShaderRecord, 1, 3) |
			moveBits(numberOfAttributeArrays, 3, 0); cl->nextFreeByte += 4;
}

/*
void clInsertClearColors(ControlList* cl,
						uint32_t clearStencil,
						uint32_t clearZ, //24 bit Z
						uint64_t clearColor) //2x RGBA8 or 1x RGBA16
{
	assert(cl);
	assert(cl->buffer);
	assert(cl->nextFreeByte);
	*cl->nextFreeByte = V3D21_CLEAR_COLORS_opcode; cl->nextFreeByte++;
	*(uint64_t*)cl->nextFreeByte = clearColor; cl->nextFreeByte += 8;
	*(uint32_t*)cl->nextFreeByte = clearZ; cl->nextFreeByte += 4; //24 bits for Z, 8 bit for vg mask (unused)
	*cl->nextFreeByte = clearStencil; cl->nextFreeByte++;
}
*/

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
	assert(cl->buffer);
	assert(cl->nextFreeByte);
	*cl->nextFreeByte = V3D21_CONFIGURATION_BITS_opcode; cl->nextFreeByte++;
	*(uint32_t*)cl->nextFreeByte =
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
			moveBits(earlyZUpdatesEnable, 1, 17); cl->nextFreeByte += 3;
}

void clInsertFlatShadeFlags(ControlList* cl,
						uint32_t flags)
{
	assert(cl);
	assert(cl->buffer);
	assert(cl->nextFreeByte);
	*cl->nextFreeByte = V3D21_FLAT_SHADE_FLAGS_opcode; cl->nextFreeByte++;
	*(uint32_t*)cl->nextFreeByte = flags; cl->nextFreeByte += 4;
}

void clInsertPointSize(ControlList* cl,
						float size)
{
	assert(cl);
	assert(cl->buffer);
	assert(cl->nextFreeByte);
	*cl->nextFreeByte = V3D21_POINT_SIZE_opcode; cl->nextFreeByte++;
	*(float*)cl->nextFreeByte = size; cl->nextFreeByte += 4;
}

void clInsertLineWidth(ControlList* cl,
						float width)
{
	assert(cl);
	assert(cl->buffer);
	assert(cl->nextFreeByte);
	*cl->nextFreeByte = V3D21_LINE_WIDTH_opcode; cl->nextFreeByte++;
	*(float*)cl->nextFreeByte = width; cl->nextFreeByte += 4;
}

void clInsertRHTXBoundary(ControlList* cl,
						uint32_t boundary) //sint16
{
	assert(cl);
	assert(cl->buffer);
	assert(cl->nextFreeByte);
	*cl->nextFreeByte = V3D21_RHT_X_BOUNDARY_opcode; cl->nextFreeByte++;
	*(uint16_t*)cl->nextFreeByte = moveBits(boundary, 16, 0); cl->nextFreeByte += 2;
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
	assert(cl->buffer);
	assert(cl->nextFreeByte);
	*cl->nextFreeByte = V3D21_CLIP_WINDOW_opcode; cl->nextFreeByte++;
	*(uint32_t*)cl->nextFreeByte = moveBits(leftPixelCoord, 16, 0) | moveBits(bottomPixelCoord, 16, 16); cl->nextFreeByte += 4;
	*(uint32_t*)cl->nextFreeByte = moveBits(width, 16, 0) | moveBits(height, 16, 16); cl->nextFreeByte += 4;
}

//viewport centre x/y coordinate
void clInsertViewPortOffset(ControlList* cl,
						int16_t x, //sint16
						int16_t y //sint16
						)
{
	assert(cl);
	assert(cl->buffer);
	assert(cl->nextFreeByte);
	*cl->nextFreeByte = V3D21_VIEWPORT_OFFSET_opcode; cl->nextFreeByte++;
	*(int16_t*)cl->nextFreeByte = x * 16; cl->nextFreeByte += 2;
	*(int16_t*)cl->nextFreeByte = y * 16; cl->nextFreeByte += 2;
}

void clInsertZMinMaxClippingPlanes(ControlList* cl,
						float minZw,
						float maxZw
						)
{
	assert(cl);
	assert(cl->nextFreeByte);
	*cl->nextFreeByte = V3D21_Z_MIN_AND_MAX_CLIPPING_PLANES_opcode; cl->nextFreeByte++;
	*(float*)cl->nextFreeByte = minZw; cl->nextFreeByte += 4;
	*(float*)cl->nextFreeByte = maxZw; cl->nextFreeByte += 4;
}

void clInsertClipperXYScaling(ControlList* cl,
						float width, //half width in 1/16 of pixel
						float height //half height in 1/16 of pixel
						)
{
	assert(cl);
	assert(cl->buffer);
	assert(cl->nextFreeByte);
	*cl->nextFreeByte = V3D21_CLIPPER_XY_SCALING_opcode; cl->nextFreeByte++;
	*(float*)cl->nextFreeByte = width; cl->nextFreeByte += 4;
	*(float*)cl->nextFreeByte = height; cl->nextFreeByte += 4;
}

void clInsertClipperZScaleOffset(ControlList* cl,
						float zOffset, //zc to zs
						float zScale //zc to zs
						)
{
	assert(cl);
	assert(cl->buffer);
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
	assert(cl->buffer);
	assert(cl->nextFreeByte);
	*cl->nextFreeByte = V3D21_TILE_BINNING_MODE_CONFIGURATION_opcode; cl->nextFreeByte++;
	*(uint32_t*)cl->nextFreeByte = tileAllocationMemoryAddress; cl->nextFreeByte += 4;
	*(uint32_t*)cl->nextFreeByte = tileAllocationMemorySize; cl->nextFreeByte += 4;
	*(uint32_t*)cl->nextFreeByte = tileStateDataArrayAddress; cl->nextFreeByte += 4;
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

	uint32_t widthInTiles = divRoundUp(widthInPixels, tileSizeW);
	uint32_t heightInTiles = divRoundUp(heightInPixels, tileSizeH);
	*(uint8_t*)cl->nextFreeByte = widthInTiles; cl->nextFreeByte++;
	*(uint8_t*)cl->nextFreeByte = heightInTiles; cl->nextFreeByte++;
	*cl->nextFreeByte =
			moveBits(multisampleMode4x, 1, 0) |
			moveBits(tileBuffer64BitColorDepth, 1, 1) |
			moveBits(autoInitializeTileStateDataArray, 1, 2) |
			moveBits(tileAllocationInitialBlockSize, 2, 3) |
			moveBits(tileAllocationBlockSize, 2, 5) |
			moveBits(doubleBufferInNonMsMode, 1, 7); cl->nextFreeByte++;
}

/*
void clInsertTileRenderingModeConfiguration(ControlList* cls,
						ControlListAddress address,
						uint32_t doubleBufferInNonMsMode, //0/1
						uint32_t earlyZEarlyCovDisable, //0/1
						uint32_t earlyZUpdateDirection, //0/1 lt,le/gt,ge
						uint32_t selectCoverageMode, //0/1
						uint32_t memoryFormat, //0/1/2 linear/t/lt
						uint32_t decimateMode, //0/1/2 0x/4x/16x
						uint32_t nonHDRFrameFormatColorFormat, //0/1/2 bgr565dithered/rgba8/bgr565nodither
						uint32_t tileBufferHDRMode, //0/1
						uint32_t multisampleMode4x, //0/1
						uint32_t widthPixels,
						uint32_t heightPixels)
{
	assert(cls);
	assert(cls->buffer);
	assert(cls->nextFreeByte);
	*cls->nextFreeByte = V3D21_TILE_RENDERING_MODE_CONFIGURATION_opcode; cls->nextFreeByte++;
	//TODO is this correct?
	clEmitShaderRelocation(cls, &address);
	*(uint32_t*)cls->nextFreeByte = address.offset; cls->nextFreeByte += 4;
	*(uint32_t*)cls->nextFreeByte = moveBits(widthPixels, 16, 0) | moveBits(heightPixels, 16, 16); cls->nextFreeByte += 4;
	*(uint16_t*)cls->nextFreeByte =
			moveBits(multisampleMode4x, 1, 0) |
			moveBits(tileBufferHDRMode, 1, 1) |
			moveBits(nonHDRFrameFormatColorFormat, 2, 2) |
			moveBits(decimateMode, 2, 4) |
			moveBits(memoryFormat, 2, 6) |
			moveBits(0, 1, 8) | //vg buffer enable
			moveBits(selectCoverageMode, 1, 9) |
			moveBits(earlyZUpdateDirection, 1, 10) |
			moveBits(earlyZEarlyCovDisable, 1, 11) |
			moveBits(doubleBufferInNonMsMode, 1, 12); cls->nextFreeByte += 2;
}
*/

/*
void clInsertTileCoordinates(ControlList* cl,
						uint32_t tileColumnNumber, //int8
						uint32_t tileRowNumber) //int8
{
	assert(cl);
	assert(cl->buffer);
	assert(cl->nextFreeByte);
	*cl->nextFreeByte = V3D21_TILE_COORDINATES_opcode; cl->nextFreeByte++;
	*(uint16_t*)cl->nextFreeByte = moveBits(tileColumnNumber, 8, 0) | moveBits(tileRowNumber, 8, 8); cl->nextFreeByte += 2;
}
*/

void clInsertGEMRelocations(ControlList* cl,
							uint32_t buffer0,
							uint32_t buffer1)
{
	assert(cl);
	assert(cl->buffer);
	assert(cl->nextFreeByte);
	*cl->nextFreeByte = V3D21_GEM_RELOCATIONS_opcode; cl->nextFreeByte++;
	*(uint32_t*)cl->nextFreeByte = buffer0; cl->nextFreeByte += 4;
	*(uint32_t*)cl->nextFreeByte = buffer1; cl->nextFreeByte += 4;
}

//input: 2 cls (cl, handles cl)
void clInsertShaderRecord(ControlList* cls,
						  ControlList* relocCl,
						  ControlList* handlesCl,
						  uint8_t* handlesBuf, uint32_t handlesSize,
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
	assert(cls->buffer);
	assert(cls->nextFreeByte);
	*cls->nextFreeByte =
			moveBits(fragmentShaderIsSingleThreaded, 1, 0) |
			moveBits(pointSizeIncludedInShadedVertexData, 1, 1) |
			moveBits(enableClipping, 1, 2); cls->nextFreeByte++;
	*cls->nextFreeByte = 0; cls->nextFreeByte++;
	*(uint16_t*)cls->nextFreeByte = moveBits(fragmentNumberOfUsedUniforms, 16, 0); cls->nextFreeByte++;
	*cls->nextFreeByte |= fragmentNumberOfVaryings; cls->nextFreeByte++;
	clEmitShaderRelocation(relocCl, handlesCl, handlesBuf, handlesSize, &fragmentCodeAddress);
	*(uint32_t*)cls->nextFreeByte = fragmentCodeAddress.offset; cls->nextFreeByte += 4;
	*(uint32_t*)cls->nextFreeByte = fragmentUniformsAddress; cls->nextFreeByte += 4;

	*(uint16_t*)cls->nextFreeByte = moveBits(vertexNumberOfUsedUniforms, 16, 0); cls->nextFreeByte += 2;
	*cls->nextFreeByte = vertexAttributeArraySelectBits; cls->nextFreeByte++;
	*cls->nextFreeByte = vertexTotalAttributesSize; cls->nextFreeByte++;
	clEmitShaderRelocation(relocCl, handlesCl, handlesBuf, handlesSize, &vertexCodeAddress);
	//TODO wtf??? --> check kernel side...
	uint32_t offset = moveBits(vertexCodeAddress.offset, 32, 0) | moveBits(vertexUniformsAddress, 32, 0);
	*(uint32_t*)cls->nextFreeByte = offset; cls->nextFreeByte += 4;
	cls->nextFreeByte += 4;

	*(uint16_t*)cls->nextFreeByte = moveBits(coordinateNumberOfUsedUniforms, 16, 0); cls->nextFreeByte += 2;
	*cls->nextFreeByte = coordinateAttributeArraySelectBits; cls->nextFreeByte++;
	*cls->nextFreeByte = coordinateTotalAttributesSize; cls->nextFreeByte++;
	clEmitShaderRelocation(relocCl, handlesCl, handlesBuf, handlesSize, &coordinateCodeAddress);
	*(uint32_t*)cls->nextFreeByte = coordinateCodeAddress.offset; cls->nextFreeByte += 4;
	*(uint32_t*)cls->nextFreeByte = coordinateUniformsAddress; cls->nextFreeByte += 4;
}

//input: 2 cls (cl, handles cl)
void clInsertAttributeRecord(ControlList* cls,
							 ControlList* relocCl,
							 ControlList* handlesCl,
							 uint8_t* handlesBuf, uint32_t handlesSize,
						  ControlListAddress address,
						  uint32_t sizeBytes,
						  uint32_t stride,
						  uint32_t vertexVPMOffset,
						  uint32_t coordinateVPMOffset)
{
	assert(cls);
	assert(cls->buffer);
	assert(cls->nextFreeByte);
	uint32_t sizeBytesMinusOne = sizeBytes - 1;
	clEmitShaderRelocation(relocCl, handlesCl, handlesBuf, handlesSize, &address);
	*(uint32_t*)cls->nextFreeByte = address.offset; cls->nextFreeByte += 4;
	*cls->nextFreeByte = sizeBytesMinusOne; cls->nextFreeByte++;
	*cls->nextFreeByte = stride; cls->nextFreeByte++;
	*cls->nextFreeByte = vertexVPMOffset; cls->nextFreeByte++;
	*cls->nextFreeByte = coordinateVPMOffset; cls->nextFreeByte++;
}

uint32_t clGetHandleIndex(ControlList* handlesCl, uint8_t* handlesBuf, uint32_t handlesSize, uint32_t handle)
{
	uint32_t c = 0;

	//if curr marker is closed already we need to work with the stored size
	uint32_t numHandles = (handlesSize ? handlesSize : (handlesCl->nextFreeByte - handlesBuf)) / 4;

	for(; c < numHandles; ++c)
	{
		if(((uint32_t*)handlesBuf)[c] == handle)
		{
			//found
			return c;
		}
	}

	//write handle to handles cl
	*(uint32_t*)handlesCl->nextFreeByte = handle;
	handlesCl->nextFreeByte += 4;

	return c;
}

//input: 2 cls (cl + handles cl)
inline void clEmitShaderRelocation(ControlList* relocCl, ControlList* handlesCl, uint8_t* handlesBuf, uint32_t handlesSize, const ControlListAddress* address)
{
	assert(relocCl);
	assert(relocCl->buffer);
	assert(relocCl->nextFreeByte);
	assert(handlesCl);
	assert(handlesCl->buffer);
	assert(handlesCl->nextFreeByte);
	assert(address);
	assert(address->handle);

	//store offset within handles in cl
	*(uint32_t*)relocCl->nextFreeByte = clGetHandleIndex(handlesCl, handlesBuf, handlesSize, address->handle);
	relocCl->nextFreeByte += 4;
}

inline void clDummyRelocation(ControlList* relocCl, const ControlListAddress* address)
{}
