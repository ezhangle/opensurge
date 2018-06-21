/*
 * Open Surge Engine
 * set_zindex.c - Sets a new zindex value for this object (affects the order it will be rendered)
 * Copyright (C) 2010  Alexandre Martins <alemartf(at)gmail(dot)com>
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

#include <math.h>
#include "set_zindex.h"
#include "../../core/util.h"

/* objectdecorator_setzindex_t class */
typedef struct objectdecorator_setzindex_t objectdecorator_setzindex_t;
struct objectdecorator_setzindex_t {
    objectdecorator_t base; /* inherits from objectdecorator_t */
    expression_t *zindex;
};

/* private methods */
static void setzindex_init(objectmachine_t *obj);
static void setzindex_release(objectmachine_t *obj);
static void setzindex_update(objectmachine_t *obj, player_t **team, int team_size, brick_list_t *brick_list, item_list_t *item_list, object_list_t *object_list);
static void setzindex_render(objectmachine_t *obj, v2d_t camera_position);



/* public methods */

/* class constructor */
objectmachine_t* objectdecorator_setzindex_new(objectmachine_t *decorated_machine, expression_t *zindex)
{
    objectdecorator_setzindex_t *me = mallocx(sizeof *me);
    objectdecorator_t *dec = (objectdecorator_t*)me;
    objectmachine_t *obj = (objectmachine_t*)dec;

    obj->init = setzindex_init;
    obj->release = setzindex_release;
    obj->update = setzindex_update;
    obj->render = setzindex_render;
    obj->get_object_instance = objectdecorator_get_object_instance; /* inherits from superclass */
    dec->decorated_machine = decorated_machine;
    me->zindex = zindex;

    return obj;
}




/* private methods */
void setzindex_init(objectmachine_t *obj)
{
    objectdecorator_t *dec = (objectdecorator_t*)obj;
    objectmachine_t *decorated_machine = dec->decorated_machine;

    ; /* empty */

    decorated_machine->init(decorated_machine);
}

void setzindex_release(objectmachine_t *obj)
{
    objectdecorator_t *dec = (objectdecorator_t*)obj;
    objectmachine_t *decorated_machine = dec->decorated_machine;
    objectdecorator_setzindex_t *me = (objectdecorator_setzindex_t*)obj;

    expression_destroy(me->zindex);

    decorated_machine->release(decorated_machine);
    free(obj);
}

void setzindex_update(objectmachine_t *obj, player_t **team, int team_size, brick_list_t *brick_list, item_list_t *item_list, object_list_t *object_list)
{
    objectdecorator_t *dec = (objectdecorator_t*)obj;
    objectmachine_t *decorated_machine = dec->decorated_machine;
    objectdecorator_setzindex_t *me = (objectdecorator_setzindex_t*)obj;
    object_t *object = obj->get_object_instance(obj);
    float zindex = expression_evaluate(me->zindex); /* no clip() */

    /* update */
    object->zindex = zindex;

    /* decorator pattern */
    decorated_machine->update(decorated_machine, team, team_size, brick_list, item_list, object_list);
}

void setzindex_render(objectmachine_t *obj, v2d_t camera_position)
{
    objectdecorator_t *dec = (objectdecorator_t*)obj;
    objectmachine_t *decorated_machine = dec->decorated_machine;

    ; /* empty */

    decorated_machine->render(decorated_machine, camera_position);
}

