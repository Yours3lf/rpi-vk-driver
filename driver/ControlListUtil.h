#pragma once

#if defined (__cplusplus)
extern "C" {
#endif

#include <stdint.h>

typedef struct ControlListAddress
{
	uint32_t handle; //handle to buffer object
	uint32_t offset; //offset within buffer object
} ControlListAddress;

typedef struct CLMarker
{
	uint32_t memGuard;

	//current binning cl buf position is this struct in the CL plus sizeof(this struct)
	//struct CLMarker* nextMarker; //
	uint32_t nextMarkerOffset;
	uint32_t size; //in bytes
	void* writeImage; //_image* to render to
	void* readImage;
	void* writeDepthStencilImage; //_image* to render depth/stencil to
	void* readDepthStencilImage;
	void* writeMSAAimage;
	void* writeMSAAdepthStencilImage;
	uint32_t writeImageOffset;
	uint32_t readImageOffset;
	uint32_t writeDepthStencilImageOffset;
	uint32_t readDepthStencilImageOffset;
	uint32_t writeMSAAimageOffset;
	uint32_t writeMSAAdepthStencilImageOffset;
	uint32_t flags; //used to store clear flag etc.
	uint32_t performResolve;
	uint32_t readMSAAimage;
	uint32_t readMSAAdepthStencilImage;
	void* perfmonID;
	uint32_t clearColor[2];
	uint32_t clearDepth, clearStencil;
	uint32_t width, height; //render w/h
	uint32_t mipLevel;

	uint32_t numDrawCallsSubmitted;

	//pointers that point to where all the other CL data is
	//plus sizes
	//uint8_t* handlesBuf; //
	uint32_t handlesBufOffset; //relative offset, because underlying buffer could be moved
	uint32_t handlesSize;
	//uint8_t* shaderRecBuf; //
	uint32_t shaderRecBufOffset;
	uint32_t shaderRecSize;
	uint32_t shaderRecCount;
	//uint8_t* uniformsBuf; //
	uint32_t uniformsBufOffset;
	uint32_t uniformsSize;
} CLMarker;

typedef struct ControlList
{
	void* CPA;
	//uint8_t* buffer;
	uint32_t offset; //offset into CPA buf
	uint32_t numBlocks;
	uint32_t blockSize;
	//uint8_t* nextFreeByte; //pointer to the next available free byte
	uint32_t nextFreeByteOffset; //pointer to the next available free byte
	//CLMarker* currMarker;
	uint32_t currMarkerOffset;
} ControlList;

void clEmitShaderRelocation(ControlList* relocCl, ControlList* handlesCl, uint32_t handlesOffset, uint32_t handlesSize, const ControlListAddress* address);
void clDummyRelocation(ControlList* relocCl, const ControlListAddress* address);

#define __gen_user_data struct ControlList
#define __gen_address_type ControlListAddress
#define __gen_address_offset(reloc) ((reloc)->offset)
#define __gen_emit_reloc clDummyRelocation

#include <string.h>

#include "brcm/cle/v3d_packet_v21_pack.h"

uint32_t divRoundUp(uint32_t n, uint32_t d);
uint32_t moveBits(uint32_t d, uint32_t bits, uint32_t offset);
uint32_t clHasEnoughSpace(ControlList* cl, uint32_t size);
void clInit(ControlList* cl, void* CPA, uint32_t offset, uint32_t blockSize);
void clInsertNewCLMarker(ControlList* cl,
						 ControlList* handlesCL,
						 ControlList* shaderRecCL,
						 uint32_t shaderRecCount,
						 ControlList* uniformsCL);
void clCloseCurrentMarker(ControlList* cl, ControlList* handlesCL, ControlList* shaderRecCL, uint32_t shaderRecCount, ControlList* uniformsCL);
void clInsertData(ControlList* cl, uint32_t size, void* data);
void clInsertUniformConstant(ControlList* cl, uint32_t data);
void clInsertUniformXYScale(ControlList* cl, float data);
void clInsertUniformZOffset(ControlList* cl, float data);
void clInsertHalt(ControlList* cl);
void clInsertNop(ControlList* cl);
void clInsertFlush(ControlList* cl);
void clInsertFlushAllState(ControlList* cl);
void clInsertStartTileBinning(ControlList* cl);
void clInsertIncrementSemaphore(ControlList* cl);
void clInsertWaitOnSemaphore(ControlList* cl);
void clInsertBranch(ControlList* cls, ControlListAddress address);
void clInsertBranchToSubList(ControlList* cls, ControlListAddress address);
void clInsertReturnFromSubList(ControlList* cl);
void clInsertStoreMultiSampleResolvedTileColorBuffer(ControlList* cl);
void clInsertStoreMultiSampleResolvedTileColorBufferAndEOF(ControlList* cl);
void clInsertIndexedPrimitiveList(ControlList* cl,
								  uint32_t maxIndex,
								  uint32_t indicesAddress,
								  uint32_t length,
								  uint32_t indexType, //0/1: 8 or 16 bit
								  enum V3D21_Primitive primitiveMode);
void clInsertVertexArrayPrimitives(ControlList* cl,
								  uint32_t firstVertexIndex,
								  uint32_t length,
								  enum V3D21_Primitive primitiveMode);
void clInsertPrimitiveListFormat(ControlList* cl,
								  uint32_t dataType, //1/3: 16 or 32 bit
								  uint32_t primitiveType); //0/1/2/3: point/line/tri/rhy
void clInsertShaderState(ControlList* cl,
						  uint32_t address,
						  uint32_t extendedShaderRecord, //0/1: true/false
						 uint32_t numberOfAttributeArrays);
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
						uint32_t enableForwardFacingPrimitive); //0/1
void clInsertFlatShadeFlags(ControlList* cl,
						uint32_t flags);
void clInsertPointSize(ControlList* cl,
						float size);
void clInsertLineWidth(ControlList* cl,
						float width);
void clInsertRHTXBoundary(ControlList* cl,
						uint32_t boundary); //sint16
void clInsertDepthOffset(ControlList* cl,
						float units,
						 float factor);
void clInsertClipWindow(ControlList* cl,
						uint32_t width, //uint16
						uint32_t height, //uint16
						uint32_t bottomPixelCoord, //uint16
						uint32_t leftPixelCoord);  //uint16
void clInsertViewPortOffset(ControlList* cl,
						float x,
						float y
						);
void clInsertZMinMaxClippingPlanes(ControlList* cl,
						float minZw,
						float maxZw
						);
void clInsertClipperXYScaling(ControlList* cl,
						float width, //half height in 1/16 of pixel
						float height //half width in 1/16 of pixel
						);
void clInsertClipperZScaleOffset(ControlList* cl,
						float zOffset, //zc to zs
						float zScale //zc to zs
						);
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
						);
void clInsertGEMRelocations(ControlList* cl,
							uint32_t buffer0,
							uint32_t buffer1);
void clInsertShaderRecord(ControlList* cls,
						  ControlList* relocCl,
						  ControlList* handlesCl,
						  uint32_t handlesOffset,
						  uint32_t handlesSize,
						  uint32_t fragmentShaderIsSingleThreaded, //0/1
						  uint32_t pointSizeIncludedInShadedVertexData, //0/1
						  uint32_t enableClipping, //0/1
						  uint32_t fragmentNumberOfUnusedUniforms,
						  uint32_t fragmentNumberOfVaryings,
						  uint32_t fragmentUniformsAddress,
						  ControlListAddress fragmentCodeAddress,
						  uint32_t vertexNumberOfUnusedUniforms,
						  uint32_t vertexAttributeArraySelectBits,
						  uint32_t vertexTotalAttributesSize,
						  uint32_t vertexUniformsAddress,
						  ControlListAddress vertexCodeAddress,
						  uint32_t coordinateNumberOfUnusedUniforms,
						  uint32_t coordinateAttributeArraySelectBits,
						  uint32_t coordinateTotalAttributesSize,
						  uint32_t coordinateUniformsAddress,
						  ControlListAddress coordinateCodeAddress);
void clInsertAttributeRecord(ControlList* cls,
							 ControlList* relocCl,
							 ControlList* handlesCl,
							 uint32_t handlesOffset, uint32_t handlesSize,
						  ControlListAddress address,
						  uint32_t sizeBytes,
						  uint32_t stride,
						  uint32_t vertexVPMOffset,
						  uint32_t coordinateVPMOffset);
uint32_t clGetHandleIndex(ControlList* handlesCl, uint32_t handlesOffset, uint32_t handlesSize, uint32_t handle);

#if defined (__cplusplus)
}
#endif
