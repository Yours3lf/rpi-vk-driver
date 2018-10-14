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

#define CONTROL_LIST_SIZE 4096

typedef struct ControlList
{
	uint8_t* buffer; //TODO size?
	uint32_t numBlocks;
	uint8_t* nextFreeByte; //pointer to the next available free byte
} ControlList;

void clEmitShaderRelocation(ControlList* relocCl, ControlList* handlesCl, const ControlListAddress* address);
void clDummyRelocation(ControlList* relocCl, const ControlListAddress* address);

#define __gen_user_data struct ControlList
#define __gen_address_type ControlListAddress
#define __gen_address_offset(reloc) ((reloc)->offset)
#define __gen_emit_reloc clDummyRelocation

#include "brcm/cle/v3d_packet_v21_pack.h"

uint32_t divRoundUp(uint32_t n, uint32_t d);
uint32_t moveBits(uint32_t d, uint32_t bits, uint32_t offset);
uint32_t clSize(ControlList* cl);
uint32_t clHasEnoughSpace(ControlList* cl, uint32_t size);
void clInit(ControlList* cl, void* buffer);
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
						uint32_t units, //float 187
						 uint32_t factor); //float 187
void clInsertClipWindow(ControlList* cl,
						uint32_t width, //uint16
						uint32_t height, //uint16
						uint32_t bottomPixelCoord, //uint16
						uint32_t leftPixelCoord);  //uint16
void clInsertViewPortOffset(ControlList* cl,
						int16_t x, //sint16
						int16_t y //sint16
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
						  ControlListAddress address,
						  uint32_t sizeBytes,
						  uint32_t stride,
						  uint32_t vertexVPMOffset,
						  uint32_t coordinateVPMOffset);
uint32_t clGetHandleIndex(ControlList* handlesCl, uint32_t handle);

#if defined (__cplusplus)
}
#endif
