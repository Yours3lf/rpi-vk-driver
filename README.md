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
  - [ ] blending
- [x] Shader compiler chain
  - [x] QPU assembler / disassembler
- [x] Resources
  - [x] Descriptor support
  - [x] VkSampler support
  - [x] Push constant support
- [ ] Render to texture features
  - [ ] VkRenderPass support
  - [ ] Subpass support
  - [ ] Multiple attachments
  - [ ] MSAA support
- [ ] Synchronization
  - [ ] vkCmdPipelineBarrier support
- [ ] Performance
  - [ ] Performance counters
  - [ ] Queries
  - [ ] Shader performance info
- [ ] Emulated features
  - [ ] Clear command support
  - [ ] Copy command support
- [ ] Platform features
  - [ ] Layer support
  - [ ] Pipeline cache support
- [ ] WSI
  - [ ] Direct to display support
- [ ] Secondary command buffers
- [ ] Try to pass as much of the VK CTS as possible with existing feature set


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

### What additional features will this driver support?
- I already added support (to be polished) to load shader assembly. This will enable devs to optimise shaders to the last cycle.
- I'll probably add something to indicate towards the developer that things are emulated or not supported at all.
- Videocore IV provides some performance counters these will be exposed
- Videocore IV supports some texture formats that are not present in the spec
  - bw1: 1 bit black and white
  - a4: 4 bit alpha
  - a1: 1 bit alpha
- vector graphics support?
