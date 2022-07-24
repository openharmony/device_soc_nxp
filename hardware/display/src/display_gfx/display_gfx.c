/*
 * Copyright (C) 2021–2022 Beijing OSWare Technology Co., Ltd
 * This file contains confidential and proprietary information of
 * OSWare Technology Co., Ltd
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "hdf_log.h"
#include "display_type.h"
#include "g2d.h"
#include "display_common.h"
#include "display_gfx.h"

#ifndef errno_t
typedef int errno_t;
#endif

void *handle = NULL;
#define ALIGN_UP(x, a) ((((x) + ((a)-1)) / (a)) * (a))
extern errno_t memset_s(void *dest, size_t destMax, int c, size_t count);

int32_t Imx8mmInitGfx(void)
{
    g2d_open(&handle);
    return DISPLAY_SUCCESS;
}

int32_t Imx8mmDeinitGfx(void)
{
    g2d_close(handle);
    return DISPLAY_SUCCESS;
}

enum g2d_format colorSpaceModeChange(PixelFormat color)
{
    enum g2d_format nxpFormat;
    switch (color) {
        case PIXEL_FMT_RGB_565:          /**< RGB565 format */
            nxpFormat = G2D_RGB565;
            break;
        case PIXEL_FMT_RGBX_8888:        /**< RGBX8888 format */
            nxpFormat = G2D_RGBX8888;
            break;
        case PIXEL_FMT_RGBA_8888:        /**< RGBA8888 format */
            nxpFormat = G2D_RGBA8888;
            break;
        case PIXEL_FMT_RGB_888:          /**< RGB888 format */
            nxpFormat = G2D_RGB888;
            break;
        case PIXEL_FMT_BGR_565:          /**< BGR565 format */
            nxpFormat = G2D_BGR565;
            break;
        case PIXEL_FMT_BGRX_8888:        /**< BGRX8888 format */
            nxpFormat = G2D_BGRX8888;
            break;
        case PIXEL_FMT_BGRA_8888:        /**< BGRA8888 format */
            nxpFormat = G2D_BGRA8888;
            break;
        default:
            nxpFormat = G2D_FORMAT_UNKNOWN;
            break;
    }

    return nxpFormat;
}

int32_t blendTypeChange(BlendType blendType)
{
    int32_t nxpBlendType;
    switch (blendType) {
        case BLEND_SRC:              /**< SRC blending */
            nxpBlendType = G2D_SRC_ALPHA;
            break;
        case BLEND_DST:              /**< DST blending */
            nxpBlendType = G2D_DST_ALPHA;
            break;
        case BLEND_SRCOVER:          /**< SRC_OVER blending */
            nxpBlendType = G2D_ONE_MINUS_SRC_ALPHA;
            break;
        case BLEND_DSTOVER:          /**< DST_OVER blending */
            nxpBlendType = G2D_ONE_MINUS_DST_ALPHA;
            break;
        default:
            nxpBlendType = -1;
            break;
    }

    return nxpBlendType;
}

int32_t TransformTypeChange(TransformType type)
{
    int32_t nxpRotateType;
    switch (type) {
        case ROTATE_90:            /**< Rotation by 90 degrees */
            nxpRotateType = G2D_ROTATION_90;
            break;
        case ROTATE_180:             /**< Rotation by 180 degrees */
            nxpRotateType = G2D_ROTATION_180;
            break;
        case ROTATE_270:             /**< Rotation by 270 degrees */
            nxpRotateType = G2D_ROTATION_270;
            break;
        default:
            nxpRotateType = G2D_ROTATION_0;        /**< No rotation */
            break;
    }

    return nxpRotateType;
}

int32_t Imx8mmFillRect(ISurface *surface, IRect *rect, uint32_t color, GfxOpt *opt)
{
    int err = 0;
    CHECK_NULLPOINTER_RETURN_VALUE(surface, DISPLAY_NULL_PTR);
    CHECK_NULLPOINTER_RETURN_VALUE(rect, DISPLAY_NULL_PTR);
    CHECK_NULLPOINTER_RETURN_VALUE(opt, DISPLAY_NULL_PTR);

    struct g2d_surface dst_surfase;

    err = memset_s(&dst_surfase, sizeof(dst_surfase), 0x00, sizeof(dst_surfase));
    if (err != 0) {
        HDF_LOGE("%s: memset_s failed", __func__);
    }

    /*---------------dst_surfase-------------------*/
    dst_surfase.format = colorSpaceModeChange(surface->enColorFmt);
    dst_surfase.planes[0] = surface->phyAddr;
    dst_surfase.left = rect->x;
    dst_surfase.top = rect->y;
    dst_surfase.right = (rect->x + rect->w);
    dst_surfase.bottom = (rect->y + rect->h);
    dst_surfase.stride = surface->stride;
    dst_surfase.width = surface->width;
    dst_surfase.height = surface->height;

    if (opt->blendType) {
        dst_surfase.blendfunc = blendTypeChange(opt->blendType);
    }

    if (opt->enGlobalAlpha) {
        dst_surfase.global_alpha = opt->globalAlpha;
    }
    dst_surfase.clrcolor = color;
    dst_surfase.rot = TransformTypeChange(opt->rotateType);

    g2d_clear(handle, &dst_surfase);

    return DISPLAY_SUCCESS;
}

int32_t Imx8mmBlit(ISurface *srcSurface, IRect *srcRect, ISurface *dstSurface, IRect *dstRect, GfxOpt *opt)
{
    int err = 0;
    struct g2d_surface src_surfase;
    struct g2d_surface dst_surfase;

    err = memset_s(&src_surfase, sizeof(src_surfase), 0x00, sizeof(src_surfase));
    if (err != 0) {
        HDF_LOGE("%s: memset_s failed", __func__);
    }

    err = memset_s(&dst_surfase, sizeof(src_surfase), 0x00, sizeof(dst_surfase));
    if (err != 0) {
        HDF_LOGE("%s: memset_s failed", __func__);
    }

    CHECK_NULLPOINTER_RETURN_VALUE(srcSurface, DISPLAY_NULL_PTR);
    CHECK_NULLPOINTER_RETURN_VALUE(srcRect, DISPLAY_NULL_PTR);
    CHECK_NULLPOINTER_RETURN_VALUE(dstSurface, DISPLAY_NULL_PTR);
    CHECK_NULLPOINTER_RETURN_VALUE(dstRect, DISPLAY_NULL_PTR);
    CHECK_NULLPOINTER_RETURN_VALUE(opt, DISPLAY_NULL_PTR);

    /*---------------src_surfase-------------------*/
    src_surfase.format = colorSpaceModeChange(srcSurface->enColorFmt);
    src_surfase.planes[0] = srcSurface->phyAddr;
    src_surfase.left = srcRect->x;
    src_surfase.top = srcRect->y;
    src_surfase.right = (srcRect->x + srcRect->w);
    src_surfase.bottom = (srcRect->y + srcRect->h);
    src_surfase.stride = ALIGN_UP(srcSurface->width, 16);
    src_surfase.width = srcSurface->width;
    src_surfase.height = srcSurface->height;
    src_surfase.blendfunc = G2D_ONE;

    if (opt->enGlobalAlpha) {
        src_surfase.global_alpha = opt->globalAlpha;
    }
    src_surfase.rot = TransformTypeChange(opt->rotateType);

    /*---------------dst_surfase-------------------*/
    dst_surfase.format = colorSpaceModeChange(dstSurface->enColorFmt);
    dst_surfase.planes[0] = dstSurface->phyAddr;
    dst_surfase.left = dstRect->x;
    dst_surfase.top = dstRect->y;
    dst_surfase.right = (dstRect->x + dstRect->w);
    dst_surfase.bottom = (dstRect->y + dstRect->h);
    dst_surfase.stride = ALIGN_UP(dstSurface->width, 16);
    dst_surfase.width = dstSurface->width;
    dst_surfase.height = dstSurface->height;
    if (opt->blendType) {
        dst_surfase.blendfunc = blendTypeChange(opt->blendType);
    }

    g2d_enable(handle, 0);
    g2d_blit(handle, &src_surfase, &dst_surfase);
    g2d_finish(handle);
    g2d_disable(handle, 0);

    return DISPLAY_SUCCESS;
}

int32_t Imx8mmSync(int32_t timeOut)
{
    (void)timeOut;

    return DISPLAY_SUCCESS;
}

int32_t GfxInitialize(GfxFuncs **funcs)
{
    if (funcs == NULL) {
        HDF_LOGE("%s: funcs is null", __func__);
        return DISPLAY_NULL_PTR;
    }

    GfxFuncs *gfxFuncs = (GfxFuncs *)malloc(sizeof(GfxFuncs));
    if (gfxFuncs == NULL) {
        HDF_LOGE("%s: funcs is null", __func__);
        return DISPLAY_NULL_PTR;
    }

    gfxFuncs->InitGfx = Imx8mmInitGfx;
    gfxFuncs->DeinitGfx = Imx8mmDeinitGfx;
    gfxFuncs->FillRect = Imx8mmFillRect;
    gfxFuncs->Blit = Imx8mmBlit;
    gfxFuncs->Sync = Imx8mmSync;
    *funcs = gfxFuncs;
    HDF_LOGI("%s: gfx initialize success", __func__);
    return DISPLAY_SUCCESS;
}

int32_t GfxUninitialize(GfxFuncs *funcs)
{
    if (funcs == NULL) {
        HDF_LOGE("%s: funcs is null", __func__);
        return DISPLAY_NULL_PTR;
    }
    free(funcs);
    HDF_LOGI("%s: gfx uninitialize success", __func__);
    return DISPLAY_SUCCESS;
}