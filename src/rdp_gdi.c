
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

#include <freerdp/freerdp.h>

#include <guacamole/client.h>

#include "client.h"

void guac_rdp_gdi_dstblt(rdpContext* context, DSTBLT_ORDER* dstblt) {
    guac_client* client = ((rdp_freerdp_context*) context)->client;
    guac_client_log_info(client, "guac_rdp_gdi_dstblt()");
}

void guac_rdp_gdi_patblt(rdpContext* context, PATBLT_ORDER* patblt) {
    guac_client* client = ((rdp_freerdp_context*) context)->client;
    guac_client_log_info(client, "guac_rdp_gdi_patblt()");
}

void guac_rdp_gdi_scrblt(rdpContext* context, SCRBLT_ORDER* scrblt) {
    guac_client* client = ((rdp_freerdp_context*) context)->client;
    guac_client_log_info(client, "guac_rdp_gdi_scrblt()");
}

void guac_rdp_gdi_memblt(rdpContext* context, MEMBLT_ORDER* memblt) {
    guac_client* client = ((rdp_freerdp_context*) context)->client;
    guac_client_log_info(client, "guac_rdp_gdi_memblt()");
}

void guac_rdp_gdi_opaquerect(rdpContext* context, OPAQUE_RECT_ORDER* opaque_rect) {

    guac_client* client = ((rdp_freerdp_context*) context)->client;
    uint32 color = freerdp_color_convert(opaque_rect->color,
            context->instance->settings->color_depth, 32,
            ((rdp_freerdp_context*) context)->clrconv);

    const guac_layer* current_layer = ((rdp_guac_client_data*) client->data)->current_surface;

    guac_protocol_send_rect(client->socket,
            GUAC_COMP_OVER, current_layer,
            opaque_rect->nLeftRect, opaque_rect->nTopRect,
            opaque_rect->nWidth, opaque_rect->nHeight,
            (color >> 16) & 0xFF,
            (color >> 8 ) & 0xFF,
            (color      ) & 0xFF,
            255);

}

void guac_rdp_gdi_palette_update(rdpContext* context, PALETTE_UPDATE* palette) {

    CLRCONV* clrconv = ((rdp_freerdp_context*) context)->clrconv;
    clrconv->palette->count = palette->number;
    clrconv->palette->entries = palette->entries;

}
