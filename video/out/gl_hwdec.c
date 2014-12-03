/*
 * This file is part of MPlayer.
 *
 * MPlayer is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * MPlayer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with MPlayer; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * You can alternatively redistribute this file and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 */

#include <stddef.h>
#include <string.h>

#include "config.h"

#include "common/common.h"
#include "common/msg.h"
#include "gl_hwdec.h"

extern const struct gl_hwdec_driver gl_hwdec_vaglx;
extern const struct gl_hwdec_driver gl_hwdec_vda;
extern const struct gl_hwdec_driver gl_hwdec_vdpau;

static const struct gl_hwdec_driver *const mpgl_hwdec_drivers[] = {
#if HAVE_VAAPI_GLX
    &gl_hwdec_vaglx,
#endif
#if HAVE_VDA_GL
    &gl_hwdec_vda,
#endif
#if HAVE_VDPAU_GL_X11
    &gl_hwdec_vdpau,
#endif
    NULL
};

static struct gl_hwdec *load_hwdec_driver(struct mp_log *log, GL *gl,
                                          const struct gl_hwdec_driver *drv,
                                          struct mp_hwdec_info *info)
{
    struct gl_hwdec *hwdec = talloc(NULL, struct gl_hwdec);
    *hwdec = (struct gl_hwdec) {
        .driver = drv,
        .log = mp_log_new(hwdec, log, drv->api_name),
        .gl = gl,
        .info = info,
        .gl_texture_target = GL_TEXTURE_2D,
    };
    if (hwdec->driver->create(hwdec) < 0) {
        talloc_free(hwdec);
        mp_err(log, "Couldn't load hwdec driver '%s'\n", drv->api_name);
        return NULL;
    }
    return hwdec;
}

struct gl_hwdec *gl_hwdec_load_api(struct mp_log *log, GL *gl,
                                   const char *api_name,
                                   struct mp_hwdec_info *info)
{
    for (int n = 0; mpgl_hwdec_drivers[n]; n++) {
        const struct gl_hwdec_driver *drv = mpgl_hwdec_drivers[n];
        if (api_name && strcmp(drv->api_name, api_name) == 0) {
            struct gl_hwdec *r = load_hwdec_driver(log, gl, drv, info);
            if (r)
                return r;
        }
    }
    return NULL;
}

void gl_hwdec_uninit(struct gl_hwdec *hwdec)
{
    if (hwdec)
        hwdec->driver->destroy(hwdec);
    talloc_free(hwdec);
}
