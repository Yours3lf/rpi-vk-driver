/* Generated code, see packets.xml and gen_packet_header.py */


/* Packets, enums and structures for V3D 2.1.
 *
 * This file has been generated, do not hand edit.
 */

#ifndef V3D21_PACK_H
#define V3D21_PACK_H

#include "v3d_packet_helpers.h"


enum V3D21_Compare_Function {
        V3D_COMPARE_FUNC_NEVER               =      0,
        V3D_COMPARE_FUNC_LESS                =      1,
        V3D_COMPARE_FUNC_EQUAL               =      2,
        V3D_COMPARE_FUNC_LEQUAL              =      3,
        V3D_COMPARE_FUNC_GREATER             =      4,
        V3D_COMPARE_FUNC_NOTEQUAL            =      5,
        V3D_COMPARE_FUNC_GEQUAL              =      6,
        V3D_COMPARE_FUNC_ALWAYS              =      7,
};

enum V3D21_Primitive {
        V3D_PRIM_POINTS                      =      0,
        V3D_PRIM_LINES                       =      1,
        V3D_PRIM_LINE_LOOP                   =      2,
        V3D_PRIM_LINE_STRIP                  =      3,
        V3D_PRIM_TRIANGLES                   =      4,
        V3D_PRIM_TRIANGLE_STRIP              =      5,
        V3D_PRIM_TRIANGLE_FAN                =      6,
};

#define V3D21_HALT_opcode                      0
#define V3D21_HALT_header                       \
   .opcode                              =      0

struct V3D21_HALT {
   uint32_t                             opcode;
};

static inline void
V3D21_HALT_pack(__gen_user_data *data, uint8_t * restrict cl,
                const struct V3D21_HALT * restrict values)
{
   cl[ 0] = __gen_uint(values->opcode, 0, 7);

}

#define V3D21_HALT_length                      1
#ifdef __gen_unpack_address
static inline void
V3D21_HALT_unpack(const uint8_t * restrict cl,
                  struct V3D21_HALT * restrict values)
{
   values->opcode = __gen_unpack_uint(cl, 0, 7);
}
#endif


#define V3D21_NOP_opcode                       1
#define V3D21_NOP_header                        \
   .opcode                              =      1

struct V3D21_NOP {
   uint32_t                             opcode;
};

static inline void
V3D21_NOP_pack(__gen_user_data *data, uint8_t * restrict cl,
               const struct V3D21_NOP * restrict values)
{
   cl[ 0] = __gen_uint(values->opcode, 0, 7);

}

#define V3D21_NOP_length                       1
#ifdef __gen_unpack_address
static inline void
V3D21_NOP_unpack(const uint8_t * restrict cl,
                 struct V3D21_NOP * restrict values)
{
   values->opcode = __gen_unpack_uint(cl, 0, 7);
}
#endif


#define V3D21_FLUSH_opcode                     4
#define V3D21_FLUSH_header                      \
   .opcode                              =      4

struct V3D21_FLUSH {
   uint32_t                             opcode;
};

static inline void
V3D21_FLUSH_pack(__gen_user_data *data, uint8_t * restrict cl,
                 const struct V3D21_FLUSH * restrict values)
{
   cl[ 0] = __gen_uint(values->opcode, 0, 7);

}

#define V3D21_FLUSH_length                     1
#ifdef __gen_unpack_address
static inline void
V3D21_FLUSH_unpack(const uint8_t * restrict cl,
                   struct V3D21_FLUSH * restrict values)
{
   values->opcode = __gen_unpack_uint(cl, 0, 7);
}
#endif


#define V3D21_FLUSH_ALL_STATE_opcode           5
#define V3D21_FLUSH_ALL_STATE_header            \
   .opcode                              =      5

struct V3D21_FLUSH_ALL_STATE {
   uint32_t                             opcode;
};

static inline void
V3D21_FLUSH_ALL_STATE_pack(__gen_user_data *data, uint8_t * restrict cl,
                           const struct V3D21_FLUSH_ALL_STATE * restrict values)
{
   cl[ 0] = __gen_uint(values->opcode, 0, 7);

}

#define V3D21_FLUSH_ALL_STATE_length           1
#ifdef __gen_unpack_address
static inline void
V3D21_FLUSH_ALL_STATE_unpack(const uint8_t * restrict cl,
                             struct V3D21_FLUSH_ALL_STATE * restrict values)
{
   values->opcode = __gen_unpack_uint(cl, 0, 7);
}
#endif


#define V3D21_START_TILE_BINNING_opcode        6
#define V3D21_START_TILE_BINNING_header         \
   .opcode                              =      6

struct V3D21_START_TILE_BINNING {
   uint32_t                             opcode;
};

static inline void
V3D21_START_TILE_BINNING_pack(__gen_user_data *data, uint8_t * restrict cl,
                              const struct V3D21_START_TILE_BINNING * restrict values)
{
   cl[ 0] = __gen_uint(values->opcode, 0, 7);

}

#define V3D21_START_TILE_BINNING_length        1
#ifdef __gen_unpack_address
static inline void
V3D21_START_TILE_BINNING_unpack(const uint8_t * restrict cl,
                                struct V3D21_START_TILE_BINNING * restrict values)
{
   values->opcode = __gen_unpack_uint(cl, 0, 7);
}
#endif


#define V3D21_INCREMENT_SEMAPHORE_opcode       7
#define V3D21_INCREMENT_SEMAPHORE_header        \
   .opcode                              =      7

struct V3D21_INCREMENT_SEMAPHORE {
   uint32_t                             opcode;
};

static inline void
V3D21_INCREMENT_SEMAPHORE_pack(__gen_user_data *data, uint8_t * restrict cl,
                               const struct V3D21_INCREMENT_SEMAPHORE * restrict values)
{
   cl[ 0] = __gen_uint(values->opcode, 0, 7);

}

#define V3D21_INCREMENT_SEMAPHORE_length       1
#ifdef __gen_unpack_address
static inline void
V3D21_INCREMENT_SEMAPHORE_unpack(const uint8_t * restrict cl,
                                 struct V3D21_INCREMENT_SEMAPHORE * restrict values)
{
   values->opcode = __gen_unpack_uint(cl, 0, 7);
}
#endif


#define V3D21_WAIT_ON_SEMAPHORE_opcode         8
#define V3D21_WAIT_ON_SEMAPHORE_header          \
   .opcode                              =      8

struct V3D21_WAIT_ON_SEMAPHORE {
   uint32_t                             opcode;
};

static inline void
V3D21_WAIT_ON_SEMAPHORE_pack(__gen_user_data *data, uint8_t * restrict cl,
                             const struct V3D21_WAIT_ON_SEMAPHORE * restrict values)
{
   cl[ 0] = __gen_uint(values->opcode, 0, 7);

}

#define V3D21_WAIT_ON_SEMAPHORE_length         1
#ifdef __gen_unpack_address
static inline void
V3D21_WAIT_ON_SEMAPHORE_unpack(const uint8_t * restrict cl,
                               struct V3D21_WAIT_ON_SEMAPHORE * restrict values)
{
   values->opcode = __gen_unpack_uint(cl, 0, 7);
}
#endif


#define V3D21_BRANCH_opcode                   16
#define V3D21_BRANCH_header                     \
   .opcode                              =     16

struct V3D21_BRANCH {
   uint32_t                             opcode;
   __gen_address_type                   address;
};

static inline void
V3D21_BRANCH_pack(__gen_user_data *data, uint8_t * restrict cl,
                  const struct V3D21_BRANCH * restrict values)
{
   cl[ 0] = __gen_uint(values->opcode, 0, 7);

   __gen_emit_reloc(data, &values->address);
   cl[ 1] = __gen_address_offset(&values->address);

   cl[ 2] = __gen_address_offset(&values->address) >> 8;

   cl[ 3] = __gen_address_offset(&values->address) >> 16;

   cl[ 4] = __gen_address_offset(&values->address) >> 24;

}

#define V3D21_BRANCH_length                    5
#ifdef __gen_unpack_address
static inline void
V3D21_BRANCH_unpack(const uint8_t * restrict cl,
                    struct V3D21_BRANCH * restrict values)
{
   values->opcode = __gen_unpack_uint(cl, 0, 7);
   values->address = __gen_unpack_address(cl, 8, 39);
}
#endif


#define V3D21_BRANCH_TO_SUB_LIST_opcode       17
#define V3D21_BRANCH_TO_SUB_LIST_header         \
   .opcode                              =     17

struct V3D21_BRANCH_TO_SUB_LIST {
   uint32_t                             opcode;
   __gen_address_type                   address;
};

static inline void
V3D21_BRANCH_TO_SUB_LIST_pack(__gen_user_data *data, uint8_t * restrict cl,
                              const struct V3D21_BRANCH_TO_SUB_LIST * restrict values)
{
   cl[ 0] = __gen_uint(values->opcode, 0, 7);

   __gen_emit_reloc(data, &values->address);
   cl[ 1] = __gen_address_offset(&values->address);

   cl[ 2] = __gen_address_offset(&values->address) >> 8;

   cl[ 3] = __gen_address_offset(&values->address) >> 16;

   cl[ 4] = __gen_address_offset(&values->address) >> 24;

}

#define V3D21_BRANCH_TO_SUB_LIST_length        5
#ifdef __gen_unpack_address
static inline void
V3D21_BRANCH_TO_SUB_LIST_unpack(const uint8_t * restrict cl,
                                struct V3D21_BRANCH_TO_SUB_LIST * restrict values)
{
   values->opcode = __gen_unpack_uint(cl, 0, 7);
   values->address = __gen_unpack_address(cl, 8, 39);
}
#endif


#define V3D21_RETURN_FROM_SUB_LIST_opcode     18
#define V3D21_RETURN_FROM_SUB_LIST_header       \
   .opcode                              =     18

struct V3D21_RETURN_FROM_SUB_LIST {
   uint32_t                             opcode;
};

static inline void
V3D21_RETURN_FROM_SUB_LIST_pack(__gen_user_data *data, uint8_t * restrict cl,
                                const struct V3D21_RETURN_FROM_SUB_LIST * restrict values)
{
   cl[ 0] = __gen_uint(values->opcode, 0, 7);

}

#define V3D21_RETURN_FROM_SUB_LIST_length      1
#ifdef __gen_unpack_address
static inline void
V3D21_RETURN_FROM_SUB_LIST_unpack(const uint8_t * restrict cl,
                                  struct V3D21_RETURN_FROM_SUB_LIST * restrict values)
{
   values->opcode = __gen_unpack_uint(cl, 0, 7);
}
#endif


#define V3D21_STORE_MULTI_SAMPLE_RESOLVED_TILE_COLOR_BUFFER_opcode     24
#define V3D21_STORE_MULTI_SAMPLE_RESOLVED_TILE_COLOR_BUFFER_header\
   .opcode                              =     24

struct V3D21_STORE_MULTI_SAMPLE_RESOLVED_TILE_COLOR_BUFFER {
   uint32_t                             opcode;
};

static inline void
V3D21_STORE_MULTI_SAMPLE_RESOLVED_TILE_COLOR_BUFFER_pack(__gen_user_data *data, uint8_t * restrict cl,
                                                         const struct V3D21_STORE_MULTI_SAMPLE_RESOLVED_TILE_COLOR_BUFFER * restrict values)
{
   cl[ 0] = __gen_uint(values->opcode, 0, 7);

}

#define V3D21_STORE_MULTI_SAMPLE_RESOLVED_TILE_COLOR_BUFFER_length      1
#ifdef __gen_unpack_address
static inline void
V3D21_STORE_MULTI_SAMPLE_RESOLVED_TILE_COLOR_BUFFER_unpack(const uint8_t * restrict cl,
                                                           struct V3D21_STORE_MULTI_SAMPLE_RESOLVED_TILE_COLOR_BUFFER * restrict values)
{
   values->opcode = __gen_unpack_uint(cl, 0, 7);
}
#endif


#define V3D21_STORE_MULTI_SAMPLE_RESOLVED_TILE_COLOR_BUFFER_AND_EOF_opcode     25
#define V3D21_STORE_MULTI_SAMPLE_RESOLVED_TILE_COLOR_BUFFER_AND_EOF_header\
   .opcode                              =     25

struct V3D21_STORE_MULTI_SAMPLE_RESOLVED_TILE_COLOR_BUFFER_AND_EOF {
   uint32_t                             opcode;
};

static inline void
V3D21_STORE_MULTI_SAMPLE_RESOLVED_TILE_COLOR_BUFFER_AND_EOF_pack(__gen_user_data *data, uint8_t * restrict cl,
                                                                 const struct V3D21_STORE_MULTI_SAMPLE_RESOLVED_TILE_COLOR_BUFFER_AND_EOF * restrict values)
{
   cl[ 0] = __gen_uint(values->opcode, 0, 7);

}

#define V3D21_STORE_MULTI_SAMPLE_RESOLVED_TILE_COLOR_BUFFER_AND_EOF_length      1
#ifdef __gen_unpack_address
static inline void
V3D21_STORE_MULTI_SAMPLE_RESOLVED_TILE_COLOR_BUFFER_AND_EOF_unpack(const uint8_t * restrict cl,
                                                                   struct V3D21_STORE_MULTI_SAMPLE_RESOLVED_TILE_COLOR_BUFFER_AND_EOF * restrict values)
{
   values->opcode = __gen_unpack_uint(cl, 0, 7);
}
#endif


#define V3D21_STORE_FULL_RESOLUTION_TILE_BUFFER_opcode     26
#define V3D21_STORE_FULL_RESOLUTION_TILE_BUFFER_header\
   .opcode                              =     26

struct V3D21_STORE_FULL_RESOLUTION_TILE_BUFFER {
   uint32_t                             opcode;
   __gen_address_type                   address;
   bool                                 last_tile;
   bool                                 disable_clear_on_write;
   bool                                 disable_z_stencil_buffer_write;
   bool                                 disable_color_buffer_write;
};

static inline void
V3D21_STORE_FULL_RESOLUTION_TILE_BUFFER_pack(__gen_user_data *data, uint8_t * restrict cl,
                                             const struct V3D21_STORE_FULL_RESOLUTION_TILE_BUFFER * restrict values)
{
   cl[ 0] = __gen_uint(values->opcode, 0, 7);

   __gen_emit_reloc(data, &values->address);
   cl[ 1] = __gen_address_offset(&values->address) |
            __gen_uint(values->last_tile, 3, 3) |
            __gen_uint(values->disable_clear_on_write, 2, 2) |
            __gen_uint(values->disable_z_stencil_buffer_write, 1, 1) |
            __gen_uint(values->disable_color_buffer_write, 0, 0);

   cl[ 2] = __gen_address_offset(&values->address) >> 8;

   cl[ 3] = __gen_address_offset(&values->address) >> 16;

   cl[ 4] = __gen_address_offset(&values->address) >> 24;

}

#define V3D21_STORE_FULL_RESOLUTION_TILE_BUFFER_length      5
#ifdef __gen_unpack_address
static inline void
V3D21_STORE_FULL_RESOLUTION_TILE_BUFFER_unpack(const uint8_t * restrict cl,
                                               struct V3D21_STORE_FULL_RESOLUTION_TILE_BUFFER * restrict values)
{
   values->opcode = __gen_unpack_uint(cl, 0, 7);
   values->address = __gen_unpack_address(cl, 12, 39);
   values->last_tile = __gen_unpack_uint(cl, 11, 11);
   values->disable_clear_on_write = __gen_unpack_uint(cl, 10, 10);
   values->disable_z_stencil_buffer_write = __gen_unpack_uint(cl, 9, 9);
   values->disable_color_buffer_write = __gen_unpack_uint(cl, 8, 8);
}
#endif


#define V3D21_RE_LOAD_FULL_RESOLUTION_TILE_BUFFER_opcode     27
#define V3D21_RE_LOAD_FULL_RESOLUTION_TILE_BUFFER_header\
   .opcode                              =     27

struct V3D21_RE_LOAD_FULL_RESOLUTION_TILE_BUFFER {
   uint32_t                             opcode;
   __gen_address_type                   address;
   bool                                 disable_z_stencil_buffer_read;
   bool                                 disable_color_buffer_read;
};

static inline void
V3D21_RE_LOAD_FULL_RESOLUTION_TILE_BUFFER_pack(__gen_user_data *data, uint8_t * restrict cl,
                                               const struct V3D21_RE_LOAD_FULL_RESOLUTION_TILE_BUFFER * restrict values)
{
   cl[ 0] = __gen_uint(values->opcode, 0, 7);

   __gen_emit_reloc(data, &values->address);
   cl[ 1] = __gen_address_offset(&values->address) |
            __gen_uint(values->disable_z_stencil_buffer_read, 1, 1) |
            __gen_uint(values->disable_color_buffer_read, 0, 0);

   cl[ 2] = __gen_address_offset(&values->address) >> 8;

   cl[ 3] = __gen_address_offset(&values->address) >> 16;

   cl[ 4] = __gen_address_offset(&values->address) >> 24;

}

#define V3D21_RE_LOAD_FULL_RESOLUTION_TILE_BUFFER_length      5
#ifdef __gen_unpack_address
static inline void
V3D21_RE_LOAD_FULL_RESOLUTION_TILE_BUFFER_unpack(const uint8_t * restrict cl,
                                                 struct V3D21_RE_LOAD_FULL_RESOLUTION_TILE_BUFFER * restrict values)
{
   values->opcode = __gen_unpack_uint(cl, 0, 7);
   values->address = __gen_unpack_address(cl, 12, 39);
   values->disable_z_stencil_buffer_read = __gen_unpack_uint(cl, 9, 9);
   values->disable_color_buffer_read = __gen_unpack_uint(cl, 8, 8);
}
#endif


#define V3D21_STORE_TILE_BUFFER_GENERAL_opcode     28
#define V3D21_STORE_TILE_BUFFER_GENERAL_header  \
   .opcode                              =     28

struct V3D21_STORE_TILE_BUFFER_GENERAL {
   uint32_t                             opcode;
   __gen_address_type                   memory_base_address_of_frame_tile_dump_buffer;
   bool                                 last_tile_of_frame;
   bool                                 disable_vg_mask_buffer_dump;
   bool                                 disable_z_stencil_buffer_dump;
   bool                                 disable_color_buffer_dump;
   bool                                 disable_vg_mask_buffer_clear_on_store_dump;
   bool                                 disable_z_stencil_buffer_clear_on_store_dump;
   bool                                 disable_color_buffer_clear_on_store_dump;
   uint32_t                             pixel_color_format;
#define RGBA8888                                 0
#define BGR565_DITHERED                          1
#define BGR565_NO_DITHER                         2
   uint32_t                             mode;
#define SAMPLE_0                                 0
#define DECIMATE_X4                              1
#define DECIMATE_X16                             2
   uint32_t                             format;
#define RASTER                                   0
#define T                                        1
#define LT                                       2
   uint32_t                             buffer_to_store;
#define NONE                                     0
#define COLOR                                    1
#define Z_STENCIL                                2
#define Z                                        3
#define VG_MASK                                  4
};

static inline void
V3D21_STORE_TILE_BUFFER_GENERAL_pack(__gen_user_data *data, uint8_t * restrict cl,
                                     const struct V3D21_STORE_TILE_BUFFER_GENERAL * restrict values)
{
   cl[ 0] = __gen_uint(values->opcode, 0, 7);

   cl[ 1] = __gen_uint(values->mode, 6, 7) |
            __gen_uint(values->format, 4, 5) |
            __gen_uint(values->buffer_to_store, 0, 2);

   cl[ 2] = __gen_uint(values->disable_vg_mask_buffer_clear_on_store_dump, 7, 7) |
            __gen_uint(values->disable_z_stencil_buffer_clear_on_store_dump, 6, 6) |
            __gen_uint(values->disable_color_buffer_clear_on_store_dump, 5, 5) |
            __gen_uint(values->pixel_color_format, 0, 1);

   __gen_emit_reloc(data, &values->memory_base_address_of_frame_tile_dump_buffer);
   cl[ 3] = __gen_address_offset(&values->memory_base_address_of_frame_tile_dump_buffer) |
            __gen_uint(values->last_tile_of_frame, 3, 3) |
            __gen_uint(values->disable_vg_mask_buffer_dump, 2, 2) |
            __gen_uint(values->disable_z_stencil_buffer_dump, 1, 1) |
            __gen_uint(values->disable_color_buffer_dump, 0, 0);

   cl[ 4] = __gen_address_offset(&values->memory_base_address_of_frame_tile_dump_buffer) >> 8;

   cl[ 5] = __gen_address_offset(&values->memory_base_address_of_frame_tile_dump_buffer) >> 16;

   cl[ 6] = __gen_address_offset(&values->memory_base_address_of_frame_tile_dump_buffer) >> 24;

}

#define V3D21_STORE_TILE_BUFFER_GENERAL_length      7
#ifdef __gen_unpack_address
static inline void
V3D21_STORE_TILE_BUFFER_GENERAL_unpack(const uint8_t * restrict cl,
                                       struct V3D21_STORE_TILE_BUFFER_GENERAL * restrict values)
{
   values->opcode = __gen_unpack_uint(cl, 0, 7);
   values->memory_base_address_of_frame_tile_dump_buffer = __gen_unpack_address(cl, 28, 55);
   values->last_tile_of_frame = __gen_unpack_uint(cl, 27, 27);
   values->disable_vg_mask_buffer_dump = __gen_unpack_uint(cl, 26, 26);
   values->disable_z_stencil_buffer_dump = __gen_unpack_uint(cl, 25, 25);
   values->disable_color_buffer_dump = __gen_unpack_uint(cl, 24, 24);
   values->disable_vg_mask_buffer_clear_on_store_dump = __gen_unpack_uint(cl, 23, 23);
   values->disable_z_stencil_buffer_clear_on_store_dump = __gen_unpack_uint(cl, 22, 22);
   values->disable_color_buffer_clear_on_store_dump = __gen_unpack_uint(cl, 21, 21);
   values->pixel_color_format = __gen_unpack_uint(cl, 16, 17);
   values->mode = __gen_unpack_uint(cl, 14, 15);
   values->format = __gen_unpack_uint(cl, 12, 13);
   values->buffer_to_store = __gen_unpack_uint(cl, 8, 10);
}
#endif


#define V3D21_LOAD_TILE_BUFFER_GENERAL_opcode     29
#define V3D21_LOAD_TILE_BUFFER_GENERAL_header   \
   .opcode                              =     29

struct V3D21_LOAD_TILE_BUFFER_GENERAL {
   uint32_t                             opcode;
   __gen_address_type                   memory_base_address_of_frame_tile_dump_buffer;
   bool                                 disable_vg_mask_buffer_load;
   bool                                 disable_z_stencil_buffer_load;
   bool                                 disable_color_buffer_load;
   uint32_t                             pixel_color_format;
#define RGBA8888                                 0
#define BGR565_DITHERED                          1
#define BGR565_NO_DITHER                         2
   uint32_t                             mode;
#define SAMPLE_0                                 0
#define DECIMATE_X4                              1
#define DECIMATE_X16                             2
   uint32_t                             format;
#define RASTER                                   0
#define T                                        1
#define LT                                       2
   uint32_t                             buffer_to_store;
#define NONE                                     0
#define COLOR                                    1
#define Z_STENCIL                                2
#define Z                                        3
#define VG_MASK                                  4
};

static inline void
V3D21_LOAD_TILE_BUFFER_GENERAL_pack(__gen_user_data *data, uint8_t * restrict cl,
                                    const struct V3D21_LOAD_TILE_BUFFER_GENERAL * restrict values)
{
   cl[ 0] = __gen_uint(values->opcode, 0, 7);

   cl[ 1] = __gen_uint(values->mode, 6, 7) |
            __gen_uint(values->format, 4, 5) |
            __gen_uint(values->buffer_to_store, 0, 2);

   cl[ 2] = __gen_uint(values->pixel_color_format, 0, 1);

   __gen_emit_reloc(data, &values->memory_base_address_of_frame_tile_dump_buffer);
   cl[ 3] = __gen_address_offset(&values->memory_base_address_of_frame_tile_dump_buffer) |
            __gen_uint(values->disable_vg_mask_buffer_load, 2, 2) |
            __gen_uint(values->disable_z_stencil_buffer_load, 1, 1) |
            __gen_uint(values->disable_color_buffer_load, 0, 0);

   cl[ 4] = __gen_address_offset(&values->memory_base_address_of_frame_tile_dump_buffer) >> 8;

   cl[ 5] = __gen_address_offset(&values->memory_base_address_of_frame_tile_dump_buffer) >> 16;

   cl[ 6] = __gen_address_offset(&values->memory_base_address_of_frame_tile_dump_buffer) >> 24;

}

#define V3D21_LOAD_TILE_BUFFER_GENERAL_length      7
#ifdef __gen_unpack_address
static inline void
V3D21_LOAD_TILE_BUFFER_GENERAL_unpack(const uint8_t * restrict cl,
                                      struct V3D21_LOAD_TILE_BUFFER_GENERAL * restrict values)
{
   values->opcode = __gen_unpack_uint(cl, 0, 7);
   values->memory_base_address_of_frame_tile_dump_buffer = __gen_unpack_address(cl, 28, 55);
   values->disable_vg_mask_buffer_load = __gen_unpack_uint(cl, 26, 26);
   values->disable_z_stencil_buffer_load = __gen_unpack_uint(cl, 25, 25);
   values->disable_color_buffer_load = __gen_unpack_uint(cl, 24, 24);
   values->pixel_color_format = __gen_unpack_uint(cl, 16, 17);
   values->mode = __gen_unpack_uint(cl, 14, 15);
   values->format = __gen_unpack_uint(cl, 12, 13);
   values->buffer_to_store = __gen_unpack_uint(cl, 8, 10);
}
#endif


#define V3D21_INDEXED_PRIMITIVE_LIST_opcode     32
#define V3D21_INDEXED_PRIMITIVE_LIST_header     \
   .opcode                              =     32

struct V3D21_INDEXED_PRIMITIVE_LIST {
   uint32_t                             opcode;
   uint32_t                             maximum_index;
   uint32_t                             address_of_indices_list;
   uint32_t                             length;
   uint32_t                             index_type;
#define _8_BIT                                   0
#define _16_BIT                                  1
   enum V3D21_Primitive                 primitive_mode;
};

static inline void
V3D21_INDEXED_PRIMITIVE_LIST_pack(__gen_user_data *data, uint8_t * restrict cl,
                                  const struct V3D21_INDEXED_PRIMITIVE_LIST * restrict values)
{
   cl[ 0] = __gen_uint(values->opcode, 0, 7);

   cl[ 1] = __gen_uint(values->index_type, 4, 7) |
            __gen_uint(values->primitive_mode, 0, 3);


   memcpy(&cl[2], &values->length, sizeof(values->length));

   memcpy(&cl[6], &values->address_of_indices_list, sizeof(values->address_of_indices_list));

   memcpy(&cl[10], &values->maximum_index, sizeof(values->maximum_index));
}

#define V3D21_INDEXED_PRIMITIVE_LIST_length     14
#ifdef __gen_unpack_address
static inline void
V3D21_INDEXED_PRIMITIVE_LIST_unpack(const uint8_t * restrict cl,
                                    struct V3D21_INDEXED_PRIMITIVE_LIST * restrict values)
{
   values->opcode = __gen_unpack_uint(cl, 0, 7);
   values->maximum_index = __gen_unpack_uint(cl, 80, 111);
   values->address_of_indices_list = __gen_unpack_uint(cl, 48, 79);
   values->length = __gen_unpack_uint(cl, 16, 47);
   values->index_type = __gen_unpack_uint(cl, 12, 15);
   values->primitive_mode = __gen_unpack_uint(cl, 8, 11);
}
#endif


#define V3D21_VERTEX_ARRAY_PRIMITIVES_opcode     33
#define V3D21_VERTEX_ARRAY_PRIMITIVES_header    \
   .opcode                              =     33

struct V3D21_VERTEX_ARRAY_PRIMITIVES {
   uint32_t                             opcode;
   uint32_t                             index_of_first_vertex;
   uint32_t                             length;
   enum V3D21_Primitive                 primitive_mode;
};

static inline void
V3D21_VERTEX_ARRAY_PRIMITIVES_pack(__gen_user_data *data, uint8_t * restrict cl,
                                   const struct V3D21_VERTEX_ARRAY_PRIMITIVES * restrict values)
{
   cl[ 0] = __gen_uint(values->opcode, 0, 7);

   cl[ 1] = __gen_uint(values->primitive_mode, 0, 3);


   memcpy(&cl[2], &values->length, sizeof(values->length));

   memcpy(&cl[6], &values->index_of_first_vertex, sizeof(values->index_of_first_vertex));
}

#define V3D21_VERTEX_ARRAY_PRIMITIVES_length     10
#ifdef __gen_unpack_address
static inline void
V3D21_VERTEX_ARRAY_PRIMITIVES_unpack(const uint8_t * restrict cl,
                                     struct V3D21_VERTEX_ARRAY_PRIMITIVES * restrict values)
{
   values->opcode = __gen_unpack_uint(cl, 0, 7);
   values->index_of_first_vertex = __gen_unpack_uint(cl, 48, 79);
   values->length = __gen_unpack_uint(cl, 16, 47);
   values->primitive_mode = __gen_unpack_uint(cl, 8, 11);
}
#endif


#define V3D21_PRIMITIVE_LIST_FORMAT_opcode     56
#define V3D21_PRIMITIVE_LIST_FORMAT_header      \
   .opcode                              =     56

struct V3D21_PRIMITIVE_LIST_FORMAT {
   uint32_t                             opcode;
   uint32_t                             data_type;
#define _16_BIT_INDEX                            1
#define _32_BIT_X_Y                              3
   uint32_t                             primitive_type;
#define POINTS_LIST                              0
#define LINES_LIST                               1
#define TRIANGLES_LIST                           2
#define RHY_LIST                                 3
};

static inline void
V3D21_PRIMITIVE_LIST_FORMAT_pack(__gen_user_data *data, uint8_t * restrict cl,
                                 const struct V3D21_PRIMITIVE_LIST_FORMAT * restrict values)
{
   cl[ 0] = __gen_uint(values->opcode, 0, 7);

   cl[ 1] = __gen_uint(values->data_type, 4, 7) |
            __gen_uint(values->primitive_type, 0, 3);

}

#define V3D21_PRIMITIVE_LIST_FORMAT_length      2
#ifdef __gen_unpack_address
static inline void
V3D21_PRIMITIVE_LIST_FORMAT_unpack(const uint8_t * restrict cl,
                                   struct V3D21_PRIMITIVE_LIST_FORMAT * restrict values)
{
   values->opcode = __gen_unpack_uint(cl, 0, 7);
   values->data_type = __gen_unpack_uint(cl, 12, 15);
   values->primitive_type = __gen_unpack_uint(cl, 8, 11);
}
#endif


#define V3D21_GL_SHADER_STATE_opcode          64
#define V3D21_GL_SHADER_STATE_header            \
   .opcode                              =     64

struct V3D21_GL_SHADER_STATE {
   uint32_t                             opcode;
   uint32_t                             address;
   bool                                 extended_shader_record;
   uint32_t                             number_of_attribute_arrays;
};

static inline void
V3D21_GL_SHADER_STATE_pack(__gen_user_data *data, uint8_t * restrict cl,
                           const struct V3D21_GL_SHADER_STATE * restrict values)
{
   cl[ 0] = __gen_uint(values->opcode, 0, 7);

   cl[ 1] = __gen_uint(values->address, 0, 27) |
            __gen_uint(values->extended_shader_record, 3, 3) |
            __gen_uint(values->number_of_attribute_arrays, 0, 2);

   cl[ 2] = __gen_uint(values->address, 0, 27) >> 8;

   cl[ 3] = __gen_uint(values->address, 0, 27) >> 16;

   cl[ 4] = __gen_uint(values->address, 0, 27) >> 24;

}

#define V3D21_GL_SHADER_STATE_length           5
#ifdef __gen_unpack_address
static inline void
V3D21_GL_SHADER_STATE_unpack(const uint8_t * restrict cl,
                             struct V3D21_GL_SHADER_STATE * restrict values)
{
   values->opcode = __gen_unpack_uint(cl, 0, 7);
   values->address = __gen_unpack_uint(cl, 8, 35);
   values->extended_shader_record = __gen_unpack_uint(cl, 11, 11);
   values->number_of_attribute_arrays = __gen_unpack_uint(cl, 8, 10);
}
#endif


#define V3D21_CLEAR_COLORS_opcode            114
#define V3D21_CLEAR_COLORS_header               \
   .opcode                              =    114

struct V3D21_CLEAR_COLORS {
   uint32_t                             opcode;
   uint32_t                             clear_stencil;
   uint32_t                             clear_vg_mask;
   uint32_t                             clear_zs;
   uint64_t                             clear_color;
};

static inline void
V3D21_CLEAR_COLORS_pack(__gen_user_data *data, uint8_t * restrict cl,
                        const struct V3D21_CLEAR_COLORS * restrict values)
{
   cl[ 0] = __gen_uint(values->opcode, 0, 7);

   cl[ 1] = __gen_uint(values->clear_color, 0, 63);

   cl[ 2] = __gen_uint(values->clear_color, 0, 63) >> 8;

   cl[ 3] = __gen_uint(values->clear_color, 0, 63) >> 16;

   cl[ 4] = __gen_uint(values->clear_color, 0, 63) >> 24;

   cl[ 5] = __gen_uint(values->clear_color, 0, 63) >> 32;

   cl[ 6] = __gen_uint(values->clear_color, 0, 63) >> 40;

   cl[ 7] = __gen_uint(values->clear_color, 0, 63) >> 48;

   cl[ 8] = __gen_uint(values->clear_color, 0, 63) >> 56;

   cl[ 9] = __gen_uint(values->clear_zs, 0, 23);

   cl[10] = __gen_uint(values->clear_zs, 0, 23) >> 8;

   cl[11] = __gen_uint(values->clear_zs, 0, 23) >> 16;

   cl[12] = __gen_uint(values->clear_vg_mask, 0, 7);

   cl[13] = __gen_uint(values->clear_stencil, 0, 7);

}

#define V3D21_CLEAR_COLORS_length             14
#ifdef __gen_unpack_address
static inline void
V3D21_CLEAR_COLORS_unpack(const uint8_t * restrict cl,
                          struct V3D21_CLEAR_COLORS * restrict values)
{
   values->opcode = __gen_unpack_uint(cl, 0, 7);
   values->clear_stencil = __gen_unpack_uint(cl, 104, 111);
   values->clear_vg_mask = __gen_unpack_uint(cl, 96, 103);
   values->clear_zs = __gen_unpack_uint(cl, 72, 95);
   values->clear_color = __gen_unpack_uint(cl, 8, 71);
}
#endif


#define V3D21_CONFIGURATION_BITS_opcode       96
#define V3D21_CONFIGURATION_BITS_header         \
   .opcode                              =     96

struct V3D21_CONFIGURATION_BITS {
   uint32_t                             opcode;
   bool                                 early_z_updates_enable;
   bool                                 early_z_enable;
   bool                                 z_updates_enable;
   enum V3D21_Compare_Function          depth_test_function;
   uint32_t                             coverage_read_mode;
   bool                                 coverage_pipe_select;
   uint32_t                             rasteriser_oversample_mode;
   uint32_t                             coverage_read_type;
   bool                                 antialiased_points_and_lines;
   bool                                 enable_depth_offset;
   bool                                 clockwise_primitives;
   bool                                 enable_reverse_facing_primitive;
   bool                                 enable_forward_facing_primitive;
};

static inline void
V3D21_CONFIGURATION_BITS_pack(__gen_user_data *data, uint8_t * restrict cl,
                              const struct V3D21_CONFIGURATION_BITS * restrict values)
{
   cl[ 0] = __gen_uint(values->opcode, 0, 7);

   cl[ 1] = __gen_uint(values->rasteriser_oversample_mode, 6, 7) |
            __gen_uint(values->coverage_read_type, 5, 5) |
            __gen_uint(values->antialiased_points_and_lines, 4, 4) |
            __gen_uint(values->enable_depth_offset, 3, 3) |
            __gen_uint(values->clockwise_primitives, 2, 2) |
            __gen_uint(values->enable_reverse_facing_primitive, 1, 1) |
            __gen_uint(values->enable_forward_facing_primitive, 0, 0);

   cl[ 2] = __gen_uint(values->z_updates_enable, 7, 7) |
            __gen_uint(values->depth_test_function, 4, 6) |
            __gen_uint(values->coverage_read_mode, 3, 3) |
            __gen_uint(values->coverage_pipe_select, 0, 0);

   cl[ 3] = __gen_uint(values->early_z_updates_enable, 1, 1) |
            __gen_uint(values->early_z_enable, 0, 0);

}

#define V3D21_CONFIGURATION_BITS_length        4
#ifdef __gen_unpack_address
static inline void
V3D21_CONFIGURATION_BITS_unpack(const uint8_t * restrict cl,
                                struct V3D21_CONFIGURATION_BITS * restrict values)
{
   values->opcode = __gen_unpack_uint(cl, 0, 7);
   values->early_z_updates_enable = __gen_unpack_uint(cl, 25, 25);
   values->early_z_enable = __gen_unpack_uint(cl, 24, 24);
   values->z_updates_enable = __gen_unpack_uint(cl, 23, 23);
   values->depth_test_function = __gen_unpack_uint(cl, 20, 22);
   values->coverage_read_mode = __gen_unpack_uint(cl, 19, 19);
   values->coverage_pipe_select = __gen_unpack_uint(cl, 16, 16);
   values->rasteriser_oversample_mode = __gen_unpack_uint(cl, 14, 15);
   values->coverage_read_type = __gen_unpack_uint(cl, 13, 13);
   values->antialiased_points_and_lines = __gen_unpack_uint(cl, 12, 12);
   values->enable_depth_offset = __gen_unpack_uint(cl, 11, 11);
   values->clockwise_primitives = __gen_unpack_uint(cl, 10, 10);
   values->enable_reverse_facing_primitive = __gen_unpack_uint(cl, 9, 9);
   values->enable_forward_facing_primitive = __gen_unpack_uint(cl, 8, 8);
}
#endif


#define V3D21_FLAT_SHADE_FLAGS_opcode         97
#define V3D21_FLAT_SHADE_FLAGS_header           \
   .opcode                              =     97

struct V3D21_FLAT_SHADE_FLAGS {
   uint32_t                             opcode;
   uint32_t                             flat_shading_flags;
};

static inline void
V3D21_FLAT_SHADE_FLAGS_pack(__gen_user_data *data, uint8_t * restrict cl,
                            const struct V3D21_FLAT_SHADE_FLAGS * restrict values)
{
   cl[ 0] = __gen_uint(values->opcode, 0, 7);


   memcpy(&cl[1], &values->flat_shading_flags, sizeof(values->flat_shading_flags));
}

#define V3D21_FLAT_SHADE_FLAGS_length          5
#ifdef __gen_unpack_address
static inline void
V3D21_FLAT_SHADE_FLAGS_unpack(const uint8_t * restrict cl,
                              struct V3D21_FLAT_SHADE_FLAGS * restrict values)
{
   values->opcode = __gen_unpack_uint(cl, 0, 7);
   values->flat_shading_flags = __gen_unpack_uint(cl, 8, 39);
}
#endif


#define V3D21_POINT_SIZE_opcode               98
#define V3D21_POINT_SIZE_header                 \
   .opcode                              =     98

struct V3D21_POINT_SIZE {
   uint32_t                             opcode;
   float                                point_size;
};

static inline void
V3D21_POINT_SIZE_pack(__gen_user_data *data, uint8_t * restrict cl,
                      const struct V3D21_POINT_SIZE * restrict values)
{
   cl[ 0] = __gen_uint(values->opcode, 0, 7);


   memcpy(&cl[1], &values->point_size, sizeof(values->point_size));
}

#define V3D21_POINT_SIZE_length                5
#ifdef __gen_unpack_address
static inline void
V3D21_POINT_SIZE_unpack(const uint8_t * restrict cl,
                        struct V3D21_POINT_SIZE * restrict values)
{
   values->opcode = __gen_unpack_uint(cl, 0, 7);
   values->point_size = __gen_unpack_float(cl, 8, 39);
}
#endif


#define V3D21_LINE_WIDTH_opcode               99
#define V3D21_LINE_WIDTH_header                 \
   .opcode                              =     99

struct V3D21_LINE_WIDTH {
   uint32_t                             opcode;
   float                                line_width;
};

static inline void
V3D21_LINE_WIDTH_pack(__gen_user_data *data, uint8_t * restrict cl,
                      const struct V3D21_LINE_WIDTH * restrict values)
{
   cl[ 0] = __gen_uint(values->opcode, 0, 7);


   memcpy(&cl[1], &values->line_width, sizeof(values->line_width));
}

#define V3D21_LINE_WIDTH_length                5
#ifdef __gen_unpack_address
static inline void
V3D21_LINE_WIDTH_unpack(const uint8_t * restrict cl,
                        struct V3D21_LINE_WIDTH * restrict values)
{
   values->opcode = __gen_unpack_uint(cl, 0, 7);
   values->line_width = __gen_unpack_float(cl, 8, 39);
}
#endif


#define V3D21_RHT_X_BOUNDARY_opcode          100
#define V3D21_RHT_X_BOUNDARY_header             \
   .opcode                              =    100

struct V3D21_RHT_X_BOUNDARY {
   uint32_t                             opcode;
   int32_t                              rht_primitive_x_boundary;
};

static inline void
V3D21_RHT_X_BOUNDARY_pack(__gen_user_data *data, uint8_t * restrict cl,
                          const struct V3D21_RHT_X_BOUNDARY * restrict values)
{
   cl[ 0] = __gen_uint(values->opcode, 0, 7);

   cl[ 1] = __gen_sint(values->rht_primitive_x_boundary, 0, 15);

   cl[ 2] = __gen_sint(values->rht_primitive_x_boundary, 0, 15) >> 8;

}

#define V3D21_RHT_X_BOUNDARY_length            3
#ifdef __gen_unpack_address
static inline void
V3D21_RHT_X_BOUNDARY_unpack(const uint8_t * restrict cl,
                            struct V3D21_RHT_X_BOUNDARY * restrict values)
{
   values->opcode = __gen_unpack_uint(cl, 0, 7);
   values->rht_primitive_x_boundary = __gen_unpack_sint(cl, 8, 23);
}
#endif


#define V3D21_DEPTH_OFFSET_opcode            101
#define V3D21_DEPTH_OFFSET_header               \
   .opcode                              =    101

struct V3D21_DEPTH_OFFSET {
   uint32_t                             opcode;
   uint32_t                             depth_offset_units;
   uint32_t                             depth_offset_factor;
};

static inline void
V3D21_DEPTH_OFFSET_pack(__gen_user_data *data, uint8_t * restrict cl,
                        const struct V3D21_DEPTH_OFFSET * restrict values)
{
   cl[ 0] = __gen_uint(values->opcode, 0, 7);

   cl[ 1] = __gen_uint(values->depth_offset_factor, 0, 15);

   cl[ 2] = __gen_uint(values->depth_offset_factor, 0, 15) >> 8;

   cl[ 3] = __gen_uint(values->depth_offset_units, 0, 15);

   cl[ 4] = __gen_uint(values->depth_offset_units, 0, 15) >> 8;

}

#define V3D21_DEPTH_OFFSET_length              5
#ifdef __gen_unpack_address
static inline void
V3D21_DEPTH_OFFSET_unpack(const uint8_t * restrict cl,
                          struct V3D21_DEPTH_OFFSET * restrict values)
{
   values->opcode = __gen_unpack_uint(cl, 0, 7);
   values->depth_offset_units = __gen_unpack_uint(cl, 24, 39);
   values->depth_offset_factor = __gen_unpack_uint(cl, 8, 23);
}
#endif


#define V3D21_CLIP_WINDOW_opcode             102
#define V3D21_CLIP_WINDOW_header                \
   .opcode                              =    102

struct V3D21_CLIP_WINDOW {
   uint32_t                             opcode;
   uint32_t                             clip_window_height_in_pixels;
   uint32_t                             clip_window_width_in_pixels;
   uint32_t                             clip_window_bottom_pixel_coordinate;
   uint32_t                             clip_window_left_pixel_coordinate;
};

static inline void
V3D21_CLIP_WINDOW_pack(__gen_user_data *data, uint8_t * restrict cl,
                       const struct V3D21_CLIP_WINDOW * restrict values)
{
   cl[ 0] = __gen_uint(values->opcode, 0, 7);

   cl[ 1] = __gen_uint(values->clip_window_left_pixel_coordinate, 0, 15);

   cl[ 2] = __gen_uint(values->clip_window_left_pixel_coordinate, 0, 15) >> 8;

   cl[ 3] = __gen_uint(values->clip_window_bottom_pixel_coordinate, 0, 15);

   cl[ 4] = __gen_uint(values->clip_window_bottom_pixel_coordinate, 0, 15) >> 8;

   cl[ 5] = __gen_uint(values->clip_window_width_in_pixels, 0, 15);

   cl[ 6] = __gen_uint(values->clip_window_width_in_pixels, 0, 15) >> 8;

   cl[ 7] = __gen_uint(values->clip_window_height_in_pixels, 0, 15);

   cl[ 8] = __gen_uint(values->clip_window_height_in_pixels, 0, 15) >> 8;

}

#define V3D21_CLIP_WINDOW_length               9
#ifdef __gen_unpack_address
static inline void
V3D21_CLIP_WINDOW_unpack(const uint8_t * restrict cl,
                         struct V3D21_CLIP_WINDOW * restrict values)
{
   values->opcode = __gen_unpack_uint(cl, 0, 7);
   values->clip_window_height_in_pixels = __gen_unpack_uint(cl, 56, 71);
   values->clip_window_width_in_pixels = __gen_unpack_uint(cl, 40, 55);
   values->clip_window_bottom_pixel_coordinate = __gen_unpack_uint(cl, 24, 39);
   values->clip_window_left_pixel_coordinate = __gen_unpack_uint(cl, 8, 23);
}
#endif


#define V3D21_VIEWPORT_OFFSET_opcode         103
#define V3D21_VIEWPORT_OFFSET_header            \
   .opcode                              =    103

struct V3D21_VIEWPORT_OFFSET {
   uint32_t                             opcode;
   float                                viewport_centre_y_coordinate;
   float                                viewport_centre_x_coordinate;
};

static inline void
V3D21_VIEWPORT_OFFSET_pack(__gen_user_data *data, uint8_t * restrict cl,
                           const struct V3D21_VIEWPORT_OFFSET * restrict values)
{
   cl[ 0] = __gen_uint(values->opcode, 0, 7);

   cl[ 1] = __gen_sfixed(values->viewport_centre_x_coordinate, 0, 15, 4);

   cl[ 2] = __gen_sfixed(values->viewport_centre_x_coordinate, 0, 15, 4) >> 8;

   cl[ 3] = __gen_sfixed(values->viewport_centre_y_coordinate, 0, 15, 4);

   cl[ 4] = __gen_sfixed(values->viewport_centre_y_coordinate, 0, 15, 4) >> 8;

}

#define V3D21_VIEWPORT_OFFSET_length           5
#ifdef __gen_unpack_address
static inline void
V3D21_VIEWPORT_OFFSET_unpack(const uint8_t * restrict cl,
                             struct V3D21_VIEWPORT_OFFSET * restrict values)
{
   values->opcode = __gen_unpack_uint(cl, 0, 7);
   values->viewport_centre_y_coordinate = __gen_unpack_sfixed(cl, 24, 39, 4);
   values->viewport_centre_x_coordinate = __gen_unpack_sfixed(cl, 8, 23, 4);
}
#endif


#define V3D21_Z_MIN_AND_MAX_CLIPPING_PLANES_opcode    104
#define V3D21_Z_MIN_AND_MAX_CLIPPING_PLANES_header\
   .opcode                              =    104

struct V3D21_Z_MIN_AND_MAX_CLIPPING_PLANES {
   uint32_t                             opcode;
   float                                maximum_zw;
   float                                minimum_zw;
};

static inline void
V3D21_Z_MIN_AND_MAX_CLIPPING_PLANES_pack(__gen_user_data *data, uint8_t * restrict cl,
                                         const struct V3D21_Z_MIN_AND_MAX_CLIPPING_PLANES * restrict values)
{
   cl[ 0] = __gen_uint(values->opcode, 0, 7);


   memcpy(&cl[1], &values->minimum_zw, sizeof(values->minimum_zw));

   memcpy(&cl[5], &values->maximum_zw, sizeof(values->maximum_zw));
}

#define V3D21_Z_MIN_AND_MAX_CLIPPING_PLANES_length      9
#ifdef __gen_unpack_address
static inline void
V3D21_Z_MIN_AND_MAX_CLIPPING_PLANES_unpack(const uint8_t * restrict cl,
                                           struct V3D21_Z_MIN_AND_MAX_CLIPPING_PLANES * restrict values)
{
   values->opcode = __gen_unpack_uint(cl, 0, 7);
   values->maximum_zw = __gen_unpack_float(cl, 40, 71);
   values->minimum_zw = __gen_unpack_float(cl, 8, 39);
}
#endif


#define V3D21_CLIPPER_XY_SCALING_opcode      105
#define V3D21_CLIPPER_XY_SCALING_header         \
   .opcode                              =    105

struct V3D21_CLIPPER_XY_SCALING {
   uint32_t                             opcode;
   float                                viewport_half_height_in_1_16th_of_pixel;
   float                                viewport_half_width_in_1_16th_of_pixel;
};

static inline void
V3D21_CLIPPER_XY_SCALING_pack(__gen_user_data *data, uint8_t * restrict cl,
                              const struct V3D21_CLIPPER_XY_SCALING * restrict values)
{
   cl[ 0] = __gen_uint(values->opcode, 0, 7);


   memcpy(&cl[1], &values->viewport_half_width_in_1_16th_of_pixel, sizeof(values->viewport_half_width_in_1_16th_of_pixel));

   memcpy(&cl[5], &values->viewport_half_height_in_1_16th_of_pixel, sizeof(values->viewport_half_height_in_1_16th_of_pixel));
}

#define V3D21_CLIPPER_XY_SCALING_length        9
#ifdef __gen_unpack_address
static inline void
V3D21_CLIPPER_XY_SCALING_unpack(const uint8_t * restrict cl,
                                struct V3D21_CLIPPER_XY_SCALING * restrict values)
{
   values->opcode = __gen_unpack_uint(cl, 0, 7);
   values->viewport_half_height_in_1_16th_of_pixel = __gen_unpack_float(cl, 40, 71);
   values->viewport_half_width_in_1_16th_of_pixel = __gen_unpack_float(cl, 8, 39);
}
#endif


#define V3D21_CLIPPER_Z_SCALE_AND_OFFSET_opcode    106
#define V3D21_CLIPPER_Z_SCALE_AND_OFFSET_header \
   .opcode                              =    106

struct V3D21_CLIPPER_Z_SCALE_AND_OFFSET {
   uint32_t                             opcode;
   float                                viewport_z_offset_zc_to_zs;
   float                                viewport_z_scale_zc_to_zs;
};

static inline void
V3D21_CLIPPER_Z_SCALE_AND_OFFSET_pack(__gen_user_data *data, uint8_t * restrict cl,
                                      const struct V3D21_CLIPPER_Z_SCALE_AND_OFFSET * restrict values)
{
   cl[ 0] = __gen_uint(values->opcode, 0, 7);


   memcpy(&cl[1], &values->viewport_z_scale_zc_to_zs, sizeof(values->viewport_z_scale_zc_to_zs));

   memcpy(&cl[5], &values->viewport_z_offset_zc_to_zs, sizeof(values->viewport_z_offset_zc_to_zs));
}

#define V3D21_CLIPPER_Z_SCALE_AND_OFFSET_length      9
#ifdef __gen_unpack_address
static inline void
V3D21_CLIPPER_Z_SCALE_AND_OFFSET_unpack(const uint8_t * restrict cl,
                                        struct V3D21_CLIPPER_Z_SCALE_AND_OFFSET * restrict values)
{
   values->opcode = __gen_unpack_uint(cl, 0, 7);
   values->viewport_z_offset_zc_to_zs = __gen_unpack_float(cl, 40, 71);
   values->viewport_z_scale_zc_to_zs = __gen_unpack_float(cl, 8, 39);
}
#endif


#define V3D21_TILE_BINNING_MODE_CONFIGURATION_opcode    112
#define V3D21_TILE_BINNING_MODE_CONFIGURATION_header\
   .opcode                              =    112

struct V3D21_TILE_BINNING_MODE_CONFIGURATION {
   uint32_t                             opcode;
   bool                                 double_buffer_in_non_ms_mode;
   uint32_t                             tile_allocation_block_size;
#define BLOCK_SIZE_32                            0
#define BLOCK_SIZE_64                            1
#define BLOCK_SIZE_128                           2
#define BLOCK_SIZE_256                           3
   uint32_t                             tile_allocation_initial_block_size;
#define BLOCK_SIZE_32                            0
#define BLOCK_SIZE_64                            1
#define BLOCK_SIZE_128                           2
#define BLOCK_SIZE_256                           3
   bool                                 auto_initialise_tile_state_data_array;
   bool                                 tile_buffer_64_bit_color_depth;
   bool                                 multisample_mode_4x;
   uint32_t                             height_in_tiles;
   uint32_t                             width_in_tiles;
   uint32_t                             tile_state_data_array_address;
   uint32_t                             tile_allocation_memory_size;
   uint32_t                             tile_allocation_memory_address;
};

static inline void
V3D21_TILE_BINNING_MODE_CONFIGURATION_pack(__gen_user_data *data, uint8_t * restrict cl,
                                           const struct V3D21_TILE_BINNING_MODE_CONFIGURATION * restrict values)
{
   cl[ 0] = __gen_uint(values->opcode, 0, 7);


   memcpy(&cl[1], &values->tile_allocation_memory_address, sizeof(values->tile_allocation_memory_address));

   memcpy(&cl[5], &values->tile_allocation_memory_size, sizeof(values->tile_allocation_memory_size));

   memcpy(&cl[9], &values->tile_state_data_array_address, sizeof(values->tile_state_data_array_address));
   cl[13] = __gen_uint(values->width_in_tiles, 0, 7);

   cl[14] = __gen_uint(values->height_in_tiles, 0, 7);

   cl[15] = __gen_uint(values->double_buffer_in_non_ms_mode, 7, 7) |
            __gen_uint(values->tile_allocation_block_size, 5, 6) |
            __gen_uint(values->tile_allocation_initial_block_size, 3, 4) |
            __gen_uint(values->auto_initialise_tile_state_data_array, 2, 2) |
            __gen_uint(values->tile_buffer_64_bit_color_depth, 1, 1) |
            __gen_uint(values->multisample_mode_4x, 0, 0);

}

#define V3D21_TILE_BINNING_MODE_CONFIGURATION_length     16
#ifdef __gen_unpack_address
static inline void
V3D21_TILE_BINNING_MODE_CONFIGURATION_unpack(const uint8_t * restrict cl,
                                             struct V3D21_TILE_BINNING_MODE_CONFIGURATION * restrict values)
{
   values->opcode = __gen_unpack_uint(cl, 0, 7);
   values->double_buffer_in_non_ms_mode = __gen_unpack_uint(cl, 127, 127);
   values->tile_allocation_block_size = __gen_unpack_uint(cl, 125, 126);
   values->tile_allocation_initial_block_size = __gen_unpack_uint(cl, 123, 124);
   values->auto_initialise_tile_state_data_array = __gen_unpack_uint(cl, 122, 122);
   values->tile_buffer_64_bit_color_depth = __gen_unpack_uint(cl, 121, 121);
   values->multisample_mode_4x = __gen_unpack_uint(cl, 120, 120);
   values->height_in_tiles = __gen_unpack_uint(cl, 112, 119);
   values->width_in_tiles = __gen_unpack_uint(cl, 104, 111);
   values->tile_state_data_array_address = __gen_unpack_uint(cl, 72, 103);
   values->tile_allocation_memory_size = __gen_unpack_uint(cl, 40, 71);
   values->tile_allocation_memory_address = __gen_unpack_uint(cl, 8, 39);
}
#endif


#define V3D21_TILE_RENDERING_MODE_CONFIGURATION_opcode    113
#define V3D21_TILE_RENDERING_MODE_CONFIGURATION_header\
   .opcode                              =    113

struct V3D21_TILE_RENDERING_MODE_CONFIGURATION {
   uint32_t                             opcode;
   bool                                 double_buffer_in_non_ms_mode;
   bool                                 early_z_early_cov_disable;
   bool                                 early_z_update_direction_gt_ge;
   bool                                 select_coverage_mode;
   bool                                 enable_vg_mask_buffer;
   uint32_t                             memory_format;
#define RASTER                                   0
#define T                                        1
#define LT                                       2
   uint32_t                             decimate_mode;
   uint32_t                             non_hdr_frame_buffer_color_format;
#define RENDERING_CONFIG_BGR565_DITHERED         0
#define RENDERING_CONFIG_RGBA8888                1
#define RENDERING_CONFIG_BGR565_NO_DITHER        2
   bool                                 tile_buffer_64_bit_color_depth;
   bool                                 multisample_mode_4x;
   uint32_t                             height_pixels;
   uint32_t                             width_pixels;
   __gen_address_type                   memory_address;
};

static inline void
V3D21_TILE_RENDERING_MODE_CONFIGURATION_pack(__gen_user_data *data, uint8_t * restrict cl,
                                             const struct V3D21_TILE_RENDERING_MODE_CONFIGURATION * restrict values)
{
   cl[ 0] = __gen_uint(values->opcode, 0, 7);

   __gen_emit_reloc(data, &values->memory_address);
   cl[ 1] = __gen_address_offset(&values->memory_address);

   cl[ 2] = __gen_address_offset(&values->memory_address) >> 8;

   cl[ 3] = __gen_address_offset(&values->memory_address) >> 16;

   cl[ 4] = __gen_address_offset(&values->memory_address) >> 24;

   cl[ 5] = __gen_uint(values->width_pixels, 0, 15);

   cl[ 6] = __gen_uint(values->width_pixels, 0, 15) >> 8;

   cl[ 7] = __gen_uint(values->height_pixels, 0, 15);

   cl[ 8] = __gen_uint(values->height_pixels, 0, 15) >> 8;

   cl[ 9] = __gen_uint(values->memory_format, 6, 7) |
            __gen_uint(values->decimate_mode, 4, 5) |
            __gen_uint(values->non_hdr_frame_buffer_color_format, 2, 3) |
            __gen_uint(values->tile_buffer_64_bit_color_depth, 1, 1) |
            __gen_uint(values->multisample_mode_4x, 0, 0);

   cl[10] = __gen_uint(values->double_buffer_in_non_ms_mode, 4, 4) |
            __gen_uint(values->early_z_early_cov_disable, 3, 3) |
            __gen_uint(values->early_z_update_direction_gt_ge, 2, 2) |
            __gen_uint(values->select_coverage_mode, 1, 1) |
            __gen_uint(values->enable_vg_mask_buffer, 0, 0);

}

#define V3D21_TILE_RENDERING_MODE_CONFIGURATION_length     11
#ifdef __gen_unpack_address
static inline void
V3D21_TILE_RENDERING_MODE_CONFIGURATION_unpack(const uint8_t * restrict cl,
                                               struct V3D21_TILE_RENDERING_MODE_CONFIGURATION * restrict values)
{
   values->opcode = __gen_unpack_uint(cl, 0, 7);
   values->double_buffer_in_non_ms_mode = __gen_unpack_uint(cl, 84, 84);
   values->early_z_early_cov_disable = __gen_unpack_uint(cl, 83, 83);
   values->early_z_update_direction_gt_ge = __gen_unpack_uint(cl, 82, 82);
   values->select_coverage_mode = __gen_unpack_uint(cl, 81, 81);
   values->enable_vg_mask_buffer = __gen_unpack_uint(cl, 80, 80);
   values->memory_format = __gen_unpack_uint(cl, 78, 79);
   values->decimate_mode = __gen_unpack_uint(cl, 76, 77);
   values->non_hdr_frame_buffer_color_format = __gen_unpack_uint(cl, 74, 75);
   values->tile_buffer_64_bit_color_depth = __gen_unpack_uint(cl, 73, 73);
   values->multisample_mode_4x = __gen_unpack_uint(cl, 72, 72);
   values->height_pixels = __gen_unpack_uint(cl, 56, 71);
   values->width_pixels = __gen_unpack_uint(cl, 40, 55);
   values->memory_address = __gen_unpack_address(cl, 8, 39);
}
#endif


#define V3D21_TILE_COORDINATES_opcode        115
#define V3D21_TILE_COORDINATES_header           \
   .opcode                              =    115

struct V3D21_TILE_COORDINATES {
   uint32_t                             opcode;
   uint32_t                             tile_row_number;
   uint32_t                             tile_column_number;
};

static inline void
V3D21_TILE_COORDINATES_pack(__gen_user_data *data, uint8_t * restrict cl,
                            const struct V3D21_TILE_COORDINATES * restrict values)
{
   cl[ 0] = __gen_uint(values->opcode, 0, 7);

   cl[ 1] = __gen_uint(values->tile_column_number, 0, 7);

   cl[ 2] = __gen_uint(values->tile_row_number, 0, 7);

}

#define V3D21_TILE_COORDINATES_length          3
#ifdef __gen_unpack_address
static inline void
V3D21_TILE_COORDINATES_unpack(const uint8_t * restrict cl,
                              struct V3D21_TILE_COORDINATES * restrict values)
{
   values->opcode = __gen_unpack_uint(cl, 0, 7);
   values->tile_row_number = __gen_unpack_uint(cl, 16, 23);
   values->tile_column_number = __gen_unpack_uint(cl, 8, 15);
}
#endif


#define V3D21_GEM_RELOCATIONS_opcode         254
#define V3D21_GEM_RELOCATIONS_header            \
   .opcode                              =    254

struct V3D21_GEM_RELOCATIONS {
   uint32_t                             opcode;
   uint32_t                             buffer_1;
   uint32_t                             buffer_0;
};

static inline void
V3D21_GEM_RELOCATIONS_pack(__gen_user_data *data, uint8_t * restrict cl,
                           const struct V3D21_GEM_RELOCATIONS * restrict values)
{
   cl[ 0] = __gen_uint(values->opcode, 0, 7);


   memcpy(&cl[1], &values->buffer_0, sizeof(values->buffer_0));

   memcpy(&cl[5], &values->buffer_1, sizeof(values->buffer_1));
}

#define V3D21_GEM_RELOCATIONS_length           9
#ifdef __gen_unpack_address
static inline void
V3D21_GEM_RELOCATIONS_unpack(const uint8_t * restrict cl,
                             struct V3D21_GEM_RELOCATIONS * restrict values)
{
   values->opcode = __gen_unpack_uint(cl, 0, 7);
   values->buffer_1 = __gen_unpack_uint(cl, 40, 71);
   values->buffer_0 = __gen_unpack_uint(cl, 8, 39);
}
#endif


#define V3D21_SHADER_RECORD_header              \


struct V3D21_SHADER_RECORD {
   bool                                 fragment_shader_is_single_threaded;
   bool                                 point_size_included_in_shaded_vertex_data;
   bool                                 enable_clipping;
   uint32_t                             fragment_shader_number_of_uniforms_not_used_currently;
   uint32_t                             fragment_shader_number_of_varyings;
   __gen_address_type                   fragment_shader_code_address;
   uint32_t                             fragment_shader_uniforms_address;
   uint32_t                             vertex_shader_number_of_uniforms_not_used_currently;
   uint32_t                             vertex_shader_attribute_array_select_bits;
   uint32_t                             vertex_shader_total_attributes_size;
   __gen_address_type                   vertex_shader_code_address;
   uint32_t                             vertex_shader_uniforms_address;
   uint32_t                             coordinate_shader_number_of_uniforms_not_used_currently;
   uint32_t                             coordinate_shader_attribute_array_select_bits;
   uint32_t                             coordinate_shader_total_attributes_size;
   __gen_address_type                   coordinate_shader_code_address;
   uint32_t                             coordinate_shader_uniforms_address;
};

static inline void
V3D21_SHADER_RECORD_pack(__gen_user_data *data, uint8_t * restrict cl,
                         const struct V3D21_SHADER_RECORD * restrict values)
{
   cl[ 0] = __gen_uint(values->fragment_shader_is_single_threaded, 0, 0) |
            __gen_uint(values->point_size_included_in_shaded_vertex_data, 1, 1) |
            __gen_uint(values->enable_clipping, 2, 2);

   cl[ 1] = 0;
   cl[ 2] = __gen_uint(values->fragment_shader_number_of_uniforms_not_used_currently, 0, 15);

   cl[ 3] = __gen_uint(values->fragment_shader_number_of_uniforms_not_used_currently, 0, 15) >> 8 |
            __gen_uint(values->fragment_shader_number_of_varyings, 0, 7);

   __gen_emit_reloc(data, &values->fragment_shader_code_address);
   cl[ 4] = __gen_address_offset(&values->fragment_shader_code_address);

   cl[ 5] = __gen_address_offset(&values->fragment_shader_code_address) >> 8;

   cl[ 6] = __gen_address_offset(&values->fragment_shader_code_address) >> 16;

   cl[ 7] = __gen_address_offset(&values->fragment_shader_code_address) >> 24;


   memcpy(&cl[8], &values->fragment_shader_uniforms_address, sizeof(values->fragment_shader_uniforms_address));
   cl[12] = __gen_uint(values->vertex_shader_number_of_uniforms_not_used_currently, 0, 15);

   cl[13] = __gen_uint(values->vertex_shader_number_of_uniforms_not_used_currently, 0, 15) >> 8;

   cl[14] = __gen_uint(values->vertex_shader_attribute_array_select_bits, 0, 7);

   cl[15] = __gen_uint(values->vertex_shader_total_attributes_size, 0, 7);

   __gen_emit_reloc(data, &values->vertex_shader_code_address);
   cl[16] = __gen_address_offset(&values->vertex_shader_code_address) |
            __gen_uint(values->vertex_shader_uniforms_address, 0, 31);

   cl[17] = __gen_address_offset(&values->vertex_shader_code_address) >> 8 |
            __gen_uint(values->vertex_shader_uniforms_address, 0, 31) >> 8;

   cl[18] = __gen_address_offset(&values->vertex_shader_code_address) >> 16 |
            __gen_uint(values->vertex_shader_uniforms_address, 0, 31) >> 16;

   cl[19] = __gen_address_offset(&values->vertex_shader_code_address) >> 24 |
            __gen_uint(values->vertex_shader_uniforms_address, 0, 31) >> 24;

   cl[20] = 0;
   cl[21] = 0;
   cl[22] = 0;
   cl[23] = 0;
   cl[24] = __gen_uint(values->coordinate_shader_number_of_uniforms_not_used_currently, 0, 15);

   cl[25] = __gen_uint(values->coordinate_shader_number_of_uniforms_not_used_currently, 0, 15) >> 8;

   cl[26] = __gen_uint(values->coordinate_shader_attribute_array_select_bits, 0, 7);

   cl[27] = __gen_uint(values->coordinate_shader_total_attributes_size, 0, 7);

   __gen_emit_reloc(data, &values->coordinate_shader_code_address);
   cl[28] = __gen_address_offset(&values->coordinate_shader_code_address);

   cl[29] = __gen_address_offset(&values->coordinate_shader_code_address) >> 8;

   cl[30] = __gen_address_offset(&values->coordinate_shader_code_address) >> 16;

   cl[31] = __gen_address_offset(&values->coordinate_shader_code_address) >> 24;


   memcpy(&cl[32], &values->coordinate_shader_uniforms_address, sizeof(values->coordinate_shader_uniforms_address));
}

#define V3D21_SHADER_RECORD_length            36
#ifdef __gen_unpack_address
static inline void
V3D21_SHADER_RECORD_unpack(const uint8_t * restrict cl,
                           struct V3D21_SHADER_RECORD * restrict values)
{
   values->fragment_shader_is_single_threaded = __gen_unpack_uint(cl, 0, 0);
   values->point_size_included_in_shaded_vertex_data = __gen_unpack_uint(cl, 1, 1);
   values->enable_clipping = __gen_unpack_uint(cl, 2, 2);
   values->fragment_shader_number_of_uniforms_not_used_currently = __gen_unpack_uint(cl, 16, 31);
   values->fragment_shader_number_of_varyings = __gen_unpack_uint(cl, 24, 31);
   values->fragment_shader_code_address = __gen_unpack_address(cl, 32, 63);
   values->fragment_shader_uniforms_address = __gen_unpack_uint(cl, 64, 95);
   values->vertex_shader_number_of_uniforms_not_used_currently = __gen_unpack_uint(cl, 96, 111);
   values->vertex_shader_attribute_array_select_bits = __gen_unpack_uint(cl, 112, 119);
   values->vertex_shader_total_attributes_size = __gen_unpack_uint(cl, 120, 127);
   values->vertex_shader_code_address = __gen_unpack_address(cl, 128, 159);
   values->vertex_shader_uniforms_address = __gen_unpack_uint(cl, 128, 159);
   values->coordinate_shader_number_of_uniforms_not_used_currently = __gen_unpack_uint(cl, 192, 207);
   values->coordinate_shader_attribute_array_select_bits = __gen_unpack_uint(cl, 208, 215);
   values->coordinate_shader_total_attributes_size = __gen_unpack_uint(cl, 216, 223);
   values->coordinate_shader_code_address = __gen_unpack_address(cl, 224, 255);
   values->coordinate_shader_uniforms_address = __gen_unpack_uint(cl, 256, 287);
}
#endif


#define V3D21_ATTRIBUTE_RECORD_header           \


struct V3D21_ATTRIBUTE_RECORD {
   __gen_address_type                   address;
   uint32_t                             number_of_bytes_minus_1;
   uint32_t                             stride;
   uint32_t                             vertex_shader_vpm_offset;
   uint32_t                             coordinate_shader_vpm_offset;
};

static inline void
V3D21_ATTRIBUTE_RECORD_pack(__gen_user_data *data, uint8_t * restrict cl,
                            const struct V3D21_ATTRIBUTE_RECORD * restrict values)
{
   __gen_emit_reloc(data, &values->address);
   cl[ 0] = __gen_address_offset(&values->address);

   cl[ 1] = __gen_address_offset(&values->address) >> 8;

   cl[ 2] = __gen_address_offset(&values->address) >> 16;

   cl[ 3] = __gen_address_offset(&values->address) >> 24;

   cl[ 4] = __gen_uint(values->number_of_bytes_minus_1, 0, 7);

   cl[ 5] = __gen_uint(values->stride, 0, 7);

   cl[ 6] = __gen_uint(values->vertex_shader_vpm_offset, 0, 7);

   cl[ 7] = __gen_uint(values->coordinate_shader_vpm_offset, 0, 7);

}

#define V3D21_ATTRIBUTE_RECORD_length          8
#ifdef __gen_unpack_address
static inline void
V3D21_ATTRIBUTE_RECORD_unpack(const uint8_t * restrict cl,
                              struct V3D21_ATTRIBUTE_RECORD * restrict values)
{
   values->address = __gen_unpack_address(cl, 0, 31);
   values->number_of_bytes_minus_1 = __gen_unpack_uint(cl, 32, 39);
   values->stride = __gen_unpack_uint(cl, 40, 47);
   values->vertex_shader_vpm_offset = __gen_unpack_uint(cl, 48, 55);
   values->coordinate_shader_vpm_offset = __gen_unpack_uint(cl, 56, 63);
}
#endif


#endif /* V3D21_PACK_H */
