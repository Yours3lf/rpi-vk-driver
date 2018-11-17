# rpi-vk-driver
(not conformant yet, can't use official name or logo)

## Milestones
- [x] clear screen example working
- [x] triangle example working
  - [x] shader from assembly, vertices from vertex buffer object, no uniforms, color hardcoded
  - [ ] uniforms for matrix multiplication and animation
  - [ ] texture coordinates and texture sampling
  - [ ] shader compiled from spirv

## VK CTS progress
- Passed:        261/3888 (6.7%)  
- Failed:        130/3888 (3.3%)  
- Not supported: 3497/3888 (89.9%)  
- Warnings:      0/3888 (0.0%)  
(dEQP-VK.api.object_management.multithreaded_per_thread_device.buffer_view_uniform_r8g8b8a8_unorm runs out of memory, I need to get a RPi 3)
