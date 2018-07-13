/*
 * Open Surge Engine
 * collisionmask.h - collision mask
 * Copyright (C) 2012, 2018  Alexandre Martins <alemartf(at)gmail(dot)com>
 * http://opensurge2d.org
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdint.h>
#include "collisionmask.h"
#include "../../core/video.h"
#include "../../core/image.h"
#include "../../core/util.h"

/* private stuff ;) */
#define MEM_ALIGNMENT               sizeof(void*) /* 4 */ /* must be a power of two */
#define MASK_ALIGN(x)               (((x) + (MEM_ALIGNMENT - 1)) & ~(MEM_ALIGNMENT - 1)) /* make x a multiple of MEM_ALIGNMENT */
struct collisionmask_t {
    char* mask;
    int width;
    int height;
    int pitch;
    uint16* gmap[4];
};

/* is MEM_ALIGNMENT a power of two? */
#define IS_POWER_OF_TWO(n)          (((n) & ((n) - 1)) == 0)
typedef char _mask_assert[ !!(IS_POWER_OF_TWO(MEM_ALIGNMENT)) * 2 - 1 ];

/* utilities */
static const int MASK_MAXSIZE = UINT16_MAX; /* masks cannot be larger than this */
static uint16* create_groundmap(const collisionmask_t* mask, grounddir_t ground_direction);
static inline uint16* destroy_groundmap(uint16* gmap);
static inline uint16* get_groundmap(const collisionmask_t* mask, grounddir_t ground_direction);

/* public methods */
collisionmask_t *collisionmask_create(const struct image_t *image, int x, int y, int width, int height)
{
    collisionmask_t *mask = mallocx(sizeof *mask);
    uint32 maskcolor = video_get_maskcolor();

    /* basic params */
    mask->width = clip(width, 1, image_width(image));
    mask->height = clip(height, 1, image_height(image));
    mask->pitch = MASK_ALIGN(mask->width);

    /* validating the size */
    if(mask->width > MASK_MAXSIZE || mask->height > MASK_MAXSIZE) {
        fatal_error("Masks cannot be larger than %d pixels.", MASK_MAXSIZE);
        free(mask);
        return NULL;
    }

    /* create the collision mask */
    mask->mask = mallocx((mask->pitch * mask->height) * sizeof(*(mask->mask)));
    for(int j = 0; j < mask->height; j++) {
        for(int i = 0; i < mask->width; i++)
            mask->mask[j * mask->pitch + i] = (image_getpixel(image, x + i, y + j) != maskcolor);
    }

    /* create the ground maps */
    mask->gmap[0] = create_groundmap(mask, GD_DOWN);
    mask->gmap[1] = create_groundmap(mask, GD_LEFT);
    mask->gmap[2] = create_groundmap(mask, GD_UP);
    mask->gmap[3] = create_groundmap(mask, GD_RIGHT);

    /* done! */
    return mask;
}

collisionmask_t *collisionmask_destroy(collisionmask_t *mask)
{
    if(mask != NULL) {
        destroy_groundmap(mask->gmap[3]);
        destroy_groundmap(mask->gmap[2]);
        destroy_groundmap(mask->gmap[1]);
        destroy_groundmap(mask->gmap[0]);
        free(mask->mask);
        free(mask);
    }
    return NULL;
}

int collisionmask_width(const collisionmask_t* mask)
{
    return mask ? mask->width : 0;
}

int collisionmask_height(const collisionmask_t* mask)
{
    return mask ? mask->height : 0;
}

int collisionmask_pitch(const collisionmask_t* mask)
{
    return mask ? mask->pitch : 0;
}

int collisionmask_peek(const collisionmask_t* mask, int x, int y)
{
    if(mask) {
        if(x >= 0 && x < mask->width && y >= 0 && y < mask->height)
            return collisionmask_at(mask, x, y, MASK_ALIGN(mask->width));
    }
    return 0;
}

int collisionmask_locate_ground(const collisionmask_t* mask, int x, int y, grounddir_t ground_direction)
{
    if(mask != NULL) {
        const uint16* gmap = get_groundmap(mask, ground_direction);
        if(gmap != NULL) {
            x = clip(x, 0, mask->width - 1);
            y = clip(y, 0, mask->height - 1);
            if(ground_direction == GD_DOWN || ground_direction == GD_UP) {
                int p = MASK_ALIGN(mask->width);
                return gmap[p * y + x];
            }
            else {
                int p = MASK_ALIGN(mask->height);
                return gmap[p * x + y];
            }
        }
    }

    return 0;
}



/* --- private utilities --- */

/* Creates a new ground map */
uint16* create_groundmap(const collisionmask_t* mask, grounddir_t ground_direction)
{
    int w = mask->width, h = mask->height;
    int pitch = mask->pitch;
    uint16* gmap = NULL;
    int x, y, p;

    /* compute the position of the ground */
    /* here we use a dynamic programming approach: */
    switch(ground_direction)
    {
        /*
         * the ground is "down":
         *
         *                  y                        if mask(x,y) = 1 and mask(x,y-1) = 0
         * gmap(x,y)   =    gmap(x,y-1)              if mask(x,y) = 1 and mask(x,y-1) = 1
         *                  gmap(x,y+1)              if mask(x,y) = 0 and y < h-1
         *                  y                        if mask(x,y) = 0 and y = h-1
         */
        case GD_DOWN:
            p = MASK_ALIGN(mask->width);
            gmap = mallocx((p * h) * sizeof(*gmap));
            for(x = 0; x < w; x++) {
                if(collisionmask_at(mask, x, 0, pitch))
                    gmap[x] = 0;
                for(y = 1; y < h; y++) {
                    if(collisionmask_at(mask, x, y, pitch)) {
                        if(collisionmask_at(mask, x, y-1, pitch))
                            gmap[p * y + x] = gmap[p * (y-1) + x];
                        else
                            gmap[p * y + x] = y;
                    }
                }
                if(!collisionmask_at(mask, x, h-1, pitch))
                    gmap[p * (h-1) + x] = h-1;
                for(y = h-2; y >= 0; y--) {
                    if(!collisionmask_at(mask, x, y, pitch))
                        gmap[p * y + x] = gmap[p * (y+1) + x];
                }
            }
            break;

        /* the ground is "to the left" */
        case GD_LEFT:
            p = MASK_ALIGN(mask->height);
            gmap = mallocx((p *  w) * sizeof(*gmap));
            for(y = 0; y < h; y++) {
                if(collisionmask_at(mask, w-1, y, pitch))
                    gmap[p * (w-1) + y] = w-1;
                for(x = w-2; x >= 0; x--) {
                    if(collisionmask_at(mask, x, y, pitch)) {
                        if(collisionmask_at(mask, x+1, y, pitch))
                            gmap[p * x + y] = gmap[p * (x+1) + y];
                        else
                            gmap[p * x + y] = x;
                    }
                }
                if(!collisionmask_at(mask, 0, y, pitch))
                    gmap[y] = 0;
                for(x = 1; x < w; x++) {
                    if(!collisionmask_at(mask, x, y, pitch))
                        gmap[p * x + y] = gmap[p * (x-1) + y];
                }
            }
            break;

        /* the ground is upwards */
        case GD_UP:
            p = MASK_ALIGN(mask->width);
            gmap = mallocx((p * h) * sizeof(*gmap));
            for(x = 0; x < w; x++) {
                if(collisionmask_at(mask, x, h-1, pitch))
                    gmap[p * (h-1) + x] = h-1;
                for(y = h-2; y >= 0; y--) {
                    if(collisionmask_at(mask, x, y, pitch)) {
                        if(collisionmask_at(mask, x, y+1, pitch))
                            gmap[p * y + x] = gmap[p * (y+1) + x];
                        else
                            gmap[p * y + x] = y;
                    }
                }
                if(!collisionmask_at(mask, x, 0, pitch))
                    gmap[x] = 0;
                for(y = 1; y < h; y++) {
                    if(!collisionmask_at(mask, x, y, pitch))
                        gmap[p * y + x] = gmap[p * (y-1) + x];
                }
            }
            break;

        /* the ground is "to the right" */
        case GD_RIGHT:
            p = MASK_ALIGN(mask->height);
            gmap = mallocx((p * w) * sizeof(*gmap));
            for(y = 0; y < h; y++) {
                if(collisionmask_at(mask, 0, y, pitch))
                    gmap[y] = 0;
                for(x = 1; x < w; x++) {
                    if(collisionmask_at(mask, x, y, pitch)) {
                        if(collisionmask_at(mask, x-1, y, pitch))
                            gmap[p * x + y] = gmap[p * (x-1) + y];
                        else
                            gmap[p * x + y] = x;
                    }
                }
                if(!collisionmask_at(mask, w-1, y, pitch))
                    gmap[p * (w-1) + y] = w-1;
                for(x = w-2; x >=0; x--) {
                    if(!collisionmask_at(mask, x, y, pitch))
                        gmap[p * x + y] = gmap[p * (x+1) + y];
                }
            }
            break;
    }

    /* done */
    return gmap;
}

/* Destroys an existing ground map */
uint16* destroy_groundmap(uint16* gmap)
{
    if(gmap != NULL)
        free(gmap);
    return NULL;
}

/* Gets a ground map from a mask */
uint16* get_groundmap(const collisionmask_t* mask, grounddir_t ground_direction)
{
    switch(ground_direction) {
        case GD_DOWN:
            return mask->gmap[0];
        case GD_LEFT:
            return mask->gmap[1];
        case GD_UP:
            return mask->gmap[2];
        case GD_RIGHT:
            return mask->gmap[3];
        default:
            return NULL;
    }
}