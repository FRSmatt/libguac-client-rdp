
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is libguac-client-rdp.
 *
 * The Initial Developer of the Original Code is
 * Michael Jumper.
 * Portions created by the Initial Developer are Copyright (C) 2011
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#include <stdio.h>
#include <stdlib.h>

#include <cairo/cairo.h>

#include <guacamole/log.h>
#include <guacamole/guacio.h>
#include <guacamole/client.h>
#include <guacamole/protocol.h>

#include <freerdp/freerdp.h>

#include "rdp_handlers.h"

void guac_rdp_ui_error(rdpInst* inst, const char* text) {

    guac_client* client = (guac_client*) inst->param1;
    GUACIO* io = client->io;

    guac_send_error(io, text);
    guac_flush(io);

}

void guac_rdp_ui_warning(rdpInst* inst, const char* text) {
    guac_log_info("guac_rdp_ui_warning: %s\n", text);
}

void guac_rdp_ui_unimpl(rdpInst* inst, const char* text) {
    guac_log_info("guac_rdp_ui_unimpl: %s\n", text);
}

void guac_rdp_ui_begin_update(rdpInst* inst) {
    /* UNUSED */
}

void guac_rdp_ui_end_update(rdpInst* inst) {
    guac_client* client = (guac_client*) inst->param1;
    GUACIO* io = client->io;
    guac_flush(io);
}

void guac_rdp_ui_desktop_save(rdpInst* inst, int offset, int x, int y, int cx, int cy) {
    guac_log_info("guac_rdp_ui_desktop_save: STUB\n");
}

void guac_rdp_ui_desktop_restore(rdpInst* inst, int offset, int x, int y, int cx, int cy) {
    guac_log_info("guac_rdp_ui_desktop_restore: STUB\n");
}


RD_HBITMAP guac_rdp_ui_create_bitmap(rdpInst* inst, int width, int height, uint8* data) {

    /* Allocate buffer */
    guac_client* client = (guac_client*) inst->param1;
    GUACIO* io = client->io; 
    guac_layer* buffer = guac_client_alloc_buffer(client);

    int x, y;
    int stride;
    int bpp = inst->settings->server_depth / 8;
    unsigned char* image_buffer;
    unsigned char* image_buffer_row;

    cairo_surface_t* surface;

    /* Init Cairo buffer */
    stride = cairo_format_stride_for_width(CAIRO_FORMAT_RGB24, width);
    image_buffer = malloc(height*stride);
    image_buffer_row = image_buffer;

    /* Copy image data from image data to buffer */
    for (y = 0; y<height; y++) {

        unsigned int*  image_buffer_current;
        
        /* Get current buffer row, advance to next */
        image_buffer_current  = (unsigned int*) image_buffer_row;
        image_buffer_row     += stride;

        for (x = 0; x<width; x++) {

            unsigned char red, green, blue;
            unsigned int v;

            switch (bpp) {
                case 3:
                    blue  = *((unsigned char*) data++);
                    green = *((unsigned char*) data++);
                    red   = *((unsigned char*) data++);
                    break;

                case 2:
                    v  = *((unsigned char*) data++);
                    v |= *((unsigned char*) data++) << 8;

                    red   = ((v >> 8) & 0xF8) | ((v >> 13) & 0x07);
                    green = ((v >> 3) & 0xFC) | ((v >>  9) & 0x03);
                    blue  = ((v << 3) & 0xF8) | ((v >>  2) & 0x07);
                    break;

                default: /* The Magenta of Failure */
                    red   = 0xFF;
                    green = 0x00;
                    blue  = 0xFF;
            }

            /* Output RGB */
            *(image_buffer_current++) = (red << 16) | (green << 8) | blue;

        }
    }

    surface = cairo_image_surface_create_for_data(image_buffer, CAIRO_FORMAT_RGB24, width, height, stride);
    guac_send_png(io, GUAC_COMP_OVER, buffer, 0, 0, surface);

    /* Free surface */
    cairo_surface_destroy(surface);
    free(image_buffer);

    return (RD_HBITMAP) buffer;

}

void guac_rdp_ui_paint_bitmap(rdpInst* inst, int x, int y, int cx, int cy, int width, int height, uint8* data) {
    guac_log_info("guac_rdp_ui_paint_bitmap: STUB\n");
}

void guac_rdp_ui_destroy_bitmap(rdpInst* inst, RD_HBITMAP bmp) {

    /* Free buffer */
    guac_client* client = (guac_client*) inst->param1;
    guac_client_free_buffer(client, (guac_layer*) bmp);

}

void guac_rdp_ui_line(rdpInst* inst, uint8 opcode, int startx, int starty, int endx, int endy, RD_PEN* pen) {
    guac_log_info("guac_rdp_ui_line: STUB\n");
}

void guac_rdp_ui_rect(rdpInst* inst, int x, int y, int cx, int cy, int color) {

    guac_client* client = (guac_client*) inst->param1;
    GUACIO* io = client->io;

    unsigned char red, green, blue;

    switch (inst->settings->server_depth) {
        case 24:
            red   = (color >> 16) & 0xFF;
            green = (color >>  8) & 0xFF;
            blue  = (color      ) & 0xFF;
            break;

        case 16:
            red   = ((color >> 8) & 0xF8) | ((color >> 13) & 0x07);
            green = ((color >> 3) & 0xFC) | ((color >>  9) & 0x03);
            blue  = ((color << 3) & 0xF8) | ((color >>  2) & 0x07);
            break;

        default: /* The Magenta of Failure */
            red   = 0xFF;
            green = 0x00;
            blue  = 0xFF;
    }

    /* Send rectangle */
    guac_send_rect(io, GUAC_COMP_OVER, GUAC_DEFAULT_LAYER,
            x, y, cx, cy,
            red, green, blue, 0xFF);

}

void guac_rdp_ui_polygon(rdpInst* inst, uint8 opcode, uint8 fillmode, RD_POINT* point, int npoints, RD_BRUSH* brush, int bgcolor, int fgcolor) {
    guac_log_info("guac_rdp_ui_polygon: STUB\n");
}

void guac_rdp_ui_polyline(rdpInst* inst, uint8 opcode, RD_POINT* points, int npoints, RD_PEN* pen) {
    guac_log_info("guac_rdp_ui_polyline: STUB\n");
}

void guac_rdp_ui_ellipse(rdpInst* inst, uint8 opcode, uint8 fillmode, int x, int y, int cx, int cy, RD_BRUSH*  brush, int bgcolor, int fgcolor) {
    guac_log_info("guac_rdp_ui_ellipse: STUB\n");
}

void guac_rdp_ui_start_draw_glyphs(rdpInst* inst, int bgcolor, int fgcolor) {
    /* UNUSED ... for now */
}

void guac_rdp_ui_draw_glyph(rdpInst* inst, int x, int y, int width, int height, RD_HGLYPH glyph) {

    guac_client* client = (guac_client*) inst->param1;
    GUACIO* io = client->io;

    guac_send_copy(io,
            (guac_layer*) glyph, 0, 0, width, height,
            GUAC_COMP_OVER, GUAC_DEFAULT_LAYER, x, y);

}

void guac_rdp_ui_end_draw_glyphs(rdpInst* inst, int x, int y, int cx, int cy) {
    /* UNUSED */
}

uint32 guac_rdp_ui_get_toggle_keys_state(rdpInst* inst) {
    guac_log_info("guac_rdp_ui_get_toggle_keys_state: STUB\n");
    return 0;
}

void guac_rdp_ui_bell(rdpInst* inst) {
    guac_log_info("guac_rdp_ui_bell: STUB\n");
}

void guac_rdp_ui_destblt(rdpInst* inst, uint8 opcode, int x, int y, int cx, int cy) {
    guac_log_info("guac_rdp_ui_destblt: STUB\n");
}

void guac_rdp_ui_patblt(rdpInst* inst, uint8 opcode, int x, int y, int cx, int cy, RD_BRUSH* brush, int bgcolor, int fgcolor) {
    guac_log_info("guac_rdp_ui_patblt: STUB\n");
}

void guac_rdp_ui_screenblt(rdpInst* inst, uint8 opcode, int x, int y, int cx, int cy, int srcx, int srcy) {
    guac_log_info("guac_rdp_ui_screenblt: STUB\n");
}

void guac_rdp_ui_memblt(rdpInst* inst, uint8 opcode, int x, int y, int width, int height, RD_HBITMAP src, int srcx, int srcy) {

    guac_client* client = (guac_client*) inst->param1;
    GUACIO* io = client->io;

    guac_log_info("guac_rdp_ui_memblt: opcode=%i, index=%i\n", opcode,
            ((guac_layer*) src)->index);

    guac_send_copy(io,
            (guac_layer*) src, srcx, srcy, width, height,
            GUAC_COMP_OVER, GUAC_DEFAULT_LAYER, x, y);

}

void guac_rdp_ui_triblt(rdpInst* inst, uint8 opcode, int x, int y, int cx, int cy, RD_HBITMAP src, int srcx, int srcy, RD_BRUSH* brush, int bgcolor,  int fgcolor) {
    guac_log_info("guac_rdp_ui_triblt: STUB\n");
}

RD_HGLYPH guac_rdp_ui_create_glyph(rdpInst* inst, int width, int height, uint8* data) {

    /* Allocate buffer */
    guac_client* client = (guac_client*) inst->param1;
    GUACIO* io = client->io;
    guac_layer* glyph = guac_client_alloc_buffer(client);

    int x, y, i;
    int stride;
    unsigned char* image_buffer;
    unsigned char* image_buffer_row;

    cairo_surface_t* surface;

    /* Init Cairo buffer */
    stride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, width);
    image_buffer = malloc(height*stride);
    image_buffer_row = image_buffer;

    /* Copy image data from image data to buffer */
    for (y = 0; y<height; y++) {

        unsigned int*  image_buffer_current;
        
        /* Get current buffer row, advance to next */
        image_buffer_current  = (unsigned int*) image_buffer_row;
        image_buffer_row     += stride;

        for (x = 0; x<width;) {

            /* Get byte from image data */
            unsigned int v = *(data++);

            /* Read bits, write pixels */
            for (i = 0; i<8 && x<width; i++, x++) {

                /* Output RGB */
                if (v & 0x80)
                    *(image_buffer_current++) = 0xFF000000;
                else
                    *(image_buffer_current++) = 0x00000000;

                /* Next bit */
                v <<= 1;

            }

        }
    }

    surface = cairo_image_surface_create_for_data(image_buffer, CAIRO_FORMAT_ARGB32, width, height, stride);
    guac_send_png(io, GUAC_COMP_OVER, glyph, 0, 0, surface);

    /* Free surface */
    cairo_surface_destroy(surface);
    free(image_buffer);


    return (RD_HGLYPH) glyph;

}

void guac_rdp_ui_destroy_glyph(rdpInst* inst, RD_HGLYPH glyph) {

    /* Free buffer */
    guac_client* client = (guac_client*) inst->param1;
    guac_client_free_buffer(client, (guac_layer*) glyph);

}

int guac_rdp_ui_select(rdpInst* inst, int rdp_socket) {
    return 1;
}

void guac_rdp_ui_set_clip(rdpInst* inst, int x, int y, int cx, int cy) {
    guac_log_info("guac_rdp_ui_set_clip: STUB\n");
}

void guac_rdp_ui_reset_clip(rdpInst* inst) {
    guac_log_info("guac_rdp_ui_reset_clip: STUB\n");
}

void guac_rdp_ui_resize_window(rdpInst* inst) {
    guac_log_info("guac_rdp_ui_resize_window: %ix%i\n", inst->settings->width, inst->settings->height);
}

void guac_rdp_ui_set_cursor(rdpInst* inst, RD_HCURSOR cursor) {
    guac_log_info("guac_rdp_ui_set_cursor: STUB\n");
}

void guac_rdp_ui_destroy_cursor(rdpInst* inst, RD_HCURSOR cursor) {
    guac_log_info("guac_rdp_ui_destroy_cursor: STUB\n");
}

RD_HCURSOR guac_rdp_ui_create_cursor(rdpInst* inst, unsigned int x, unsigned int y, int width, int height, uint8* andmask, uint8* xormask, int bpp) {
    guac_log_info("guac_rdp_ui_create_cursor: STUB\n");
    return NULL;
}

void guac_rdp_ui_set_null_cursor(rdpInst* inst) {
    guac_log_info("guac_rdp_ui_set_null_cursor: STUB\n");
}

void guac_rdp_ui_set_default_cursor(rdpInst* inst) {
    guac_log_info("guac_rdp_ui_set_default_cursor: STUB\n");
}

RD_HPALETTE guac_rdp_ui_create_colormap(rdpInst* inst, RD_PALETTE* colors) {
    guac_log_info("guac_rdp_ui_create_colormap: STUB\n");
    return NULL;
}

void guac_rdp_ui_move_pointer(rdpInst* inst, int x, int y) {
    guac_log_info("guac_rdp_ui_move_pointer: STUB\n");
}

void guac_rdp_ui_set_colormap(rdpInst* inst, RD_HPALETTE map) {
    guac_log_info("guac_rdp_ui_set_colormap: STUB\n");
}

RD_HBITMAP guac_rdp_ui_create_surface(rdpInst* inst, int width, int height, RD_HBITMAP old) {

    /* Allocate and return buffer */
    guac_client* client = (guac_client*) inst->param1;
    return (RD_HBITMAP) guac_client_alloc_buffer(client);

}

void guac_rdp_ui_set_surface(rdpInst* inst, RD_HBITMAP surface) {

    guac_client* client = (guac_client*) inst->param1;
    GUACIO* io = client->io;

    /* Init desktop */
    if (surface == NULL) {

        guac_send_name(io, inst->settings->server);
        guac_send_size(io, inst->settings->width, inst->settings->height);
        guac_flush(io);

    }
    else
        guac_log_info("guac_rdp_ui_set_surface: STUB (surface=%p) ... %ix%i\n", surface, inst->settings->width, inst->settings->height);

}

void guac_rdp_ui_destroy_surface(rdpInst* inst, RD_HBITMAP surface) {

    /* Free buffer */
    guac_client* client = (guac_client*) inst->param1;
    guac_client_free_buffer(client, (guac_layer*) surface);

}

void guac_rdp_ui_channel_data(rdpInst* inst, int chan_id, char* data, int data_size, int flags, int total_size) {
    guac_log_info("guac_rdp_ui_channel_data: STUB\n");
}


