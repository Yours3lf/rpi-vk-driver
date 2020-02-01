# rpi-vk-driver
(not conformant yet, can't use official name or logo)

## Milestones
- [x] clear screen example working
- [x] triangle example working
  - [x] shader from assembly, vertices from vertex buffer object, no uniforms, color hardcoded
  - [x] uniforms for matrix multiplication and animation
  - [x] texture coordinates and texture sampling
  - [x] varyings
  - [x] Multiple vertex attributes
  - [x] Depth buffers
  - [x] Stencil buffers
  - [x] Indexed draw calls
  - [x] blending
  - [ ] mipmapping
  - [ ] cube mapping
  - [x] shadow mapping / depth texture sampling
  - [ ] HDR rendering
  - [ ] Multi threaded cmdbuf generation test
- [x] Shader compiler chain
  - [x] QPU assembler / disassembler
- [x] Resources
  - [x] Descriptor support
  - [x] VkSampler support
  - [x] Push constant support
- [x] Platform features
  - [x] Layer support
- [ ] Emulated features
  - [ ] Clear command support
  - [ ] Copy command support
- [ ] Render to texture features
  - [ ] VkRenderPass support
  - [ ] Subpass support
  - [ ] MSAA support
- [ ] Synchronization
  - [ ] vkCmdPipelineBarrier support
- [ ] Performance
  - [ ] Performance counters
  - [ ] Queries
  - [ ] Shader performance info
- [ ] WSI
  - [ ] Direct to display support
- [ ] Secondary command buffers
- [ ] Fixes
  - [ ] Hardware bug workarounds
  - [ ] Handle offsets wherever required
  - [ ] Handle subresource ranges properly
  - [ ] Handle allocation scopes properly
  - [ ] Shader module creation might not be thread safe
- [ ] Try to pass as much of the VK CTS as possible with existing feature set
- [ ] Github pages
- [ ] Wiki
  - [ ] Performance recommendations
  - [ ] How to do blending, depth/stencil testing, attributes

## VK CTS progress
- Passed:        7894/67979 (11.6%) 
- Failed:        878/67979 (1.3%)
- Not supported: 59206/67979 (87.1%)
- Warnings:      1/67979 (0.0%)

Conformance run is considered passing if all tests finish with allowed result
codes. 
Following status
codes are allowed:

- Pass
- NotSupported
- QualityWarning
- CompatibilityWarning 

There are about 470.000 conformance tests.

## FAQ
### Will this ever be a fully functional VK driver?
As far as I know the PI is NOT fully VK capable on the hardware level. I can already see that some things will need to be emulated and others won't ever be emulated.

### What performance should you expect?
Performance wise, the Pi is quite capable. The specs and architecture is close to the GPU in the iPhone 4s. The only problem I see is bandwidth as you only have about 7GB/s compared to 12-25GB/s on typical mobile phones. So post processing is a huge no and you'd need to be very careful about the techniques that you use. Eg. you'd need to stay on chip at all times. 
CPU performance (eg. number of draw calls) should be enough on the quad-core PIs as you can easily utilise all cores using VK.

### What features will not be supported?
- 3D textures
- sparse textures
- compute shaders (though could be supported to some extent if the kernel side would support it)
- some texture formats
- some render target texture formats
- occlusion queries (https://github.com/anholt/mesa/wiki/VC4-OpenGL-support)
- indirect draws
- spirv shaders
- events
- proper semaphore support
- tessellation shaders
- geometry shaders
- 32 bit indices
- instancing
- some vertex buffer formats
- pipeline caches (doesn't make sense with assembly shaders)

### What additional features will this driver support?
- I already added support (to be polished) to load shader assembly. This will enable devs to optimise shaders to the last cycle.
- I'll probably add something to indicate towards the developer that things are emulated or not supported at all.
- Videocore IV provides some performance counters these will be exposed
- Videocore IV supports some texture formats that are not present in the spec
  - bw1: 1 bit black and white
  - a4: 4 bit alpha
  - a1: 1 bit alpha
- vector graphics support?

### Shader patching
The Broadcom Videocore IV needs a couple of operations to happen in shader code that might have fixed function hardware on other platforms.  
These are: 
- writing stencil state setup register
- writing depth value to depth buffer
- performing blending in software
- writing vertex parameter memory read and write setup registers

Since the project will not include a compiler, but rather works with an assembly based shader setup, I decided not to patch shaders based on the state provided to the driver, but rather let the developer have full control.
This means that regardless of what  
- depth write state
- blending state
- stencil state
- vertex attribute state  

is passed to the driver, this will not be reflected in the final behaviour unless the developer adds it to the assembly shaders.
Helper functionality will be provided to aid with encoding register values. Additionally, general documentation will be provided on how to perform these operations.

This will enable developers to take full control and optimise shaders to the last cycle.

