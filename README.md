# rpi-vk-driver
(not conformant yet, can't use official name or logo)

## Milestones
- [x] clear screen example working
- [x] triangle example working
  - [x] shader from assembly, vertices from vertex buffer object, no uniforms, color hardcoded
  - [x] uniforms for matrix multiplication and animation
  - [x] texture coordinates and texture sampling
  - [ ] varyings
- [x] Shader compiler chain
  - [x] QPU assembler / disassembler
- [x] Resources
  - [x] Descriptor support
  - [x] VkSampler support
  - [x] Push constant support
- [ ] Multipass rendering
  - [ ] VkRenderPass support
  - [ ] Subpass support
- [ ] Synchronization
  - [ ] vkCmdPipelineBarrier support
  - [ ] VkEvent support
- [ ] Indexed draw call support
- [ ] Clear command support
- [ ] Layer support
- [ ] Pipeline cache support
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

## FAQ
### Will this ever be a fully functional VK driver?
As far as I know the PI is NOT fully VK capable on the hardware level. I can already see that some things will need to be emulated and others won't ever be emulated. The compiler is also a huge unknown at this point as I plan to write one from scratch.

### What performance should you expect?
Performance wise, the Pi is quite capable. The specs and architecture is close to the GPU in the iPhone 4s. The only problem I see is bandwidth as you only have about 7GB/s compared to 12-25GB/s on typical mobile phones. So post processing is a huge no and you'd need to be very careful about the techniques that you use. Eg. you'd need to stay on chip at all times. 
CPU performance (eg. number of draw calls) should be enough on the quad-core PIs as you can easily utilise all cores using VK.

### What will be emulated?
- I already plan to emulate a couple of the basics such as copy commands using compute
- Tessellation and geometry shaders won't be considered, it just doesn't make sense
- Sparse resources might be implemented, but I don't think performance would be great
- Compute shaders could also be implemented, though I think I would need to modify the kernel side for that, no LDS or any of the fancy stuff though...
- As far as I know the PI doesn't support occlusion queries (https://github.com/anholt/mesa/wiki/VC4-OpenGL-support)
- Indirect draws are probably out of scope
- I already added support (to be polished) to load shader assembly. This will enable devs to optimise shaders to the last cycle.
- I'll probably add something to indicate towards the developer that things are emulated or not supported at all.
