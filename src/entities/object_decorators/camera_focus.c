/*
 * Open Surge Engine
 * camera_focus.c - request/drop camera focus
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

#include "camera_focus.h"
#include "../../core/util.h"
#include "../../scenes/level.h"

/* objectdecorator_camerafocus_t class */
typedef struct objectdecorator_camerafocus_t objectdecorator_camerafocus_t;
struct objectdecorator_camerafocus_t {
    objectdecorator_t base; /* inherits from objectdecorator_t */
    void (*strategy)(objectmachine_t*); /* strategy pattern */
};

/* private methods */
static void camerafocus_init(objectmachine_t *obj);
static void camerafocus_release(objectmachine_t *obj);
static void camerafocus_update(objectmachine_t *obj, player_t **team, int team_size, brick_list_t *brick_list, item_list_t *item_list, object_list_t *object_list);
static void camerafocus_render(objectmachine_t *obj, v2d_t camera_position);

static objectmachine_t* make_decorator(objectmachine_t *decorated_machine, void (*strategy)(objectmachine_t*));

/* private strategies */
static void request_camera_focus(objectmachine_t *obj);
static void drop_camera_focus(objectmachine_t *obj);



/* public methods */

/* class constructor */
objectmachine_t* objectdecorator_requestcamerafocus_new(objectmachine_t *decorated_machine)
{
    return make_decorator(decorated_machine, request_camera_focus);
}

objectmachine_t* objectdecorator_dropcamerafocus_new(objectmachine_t *decorated_machine)
{
    return make_decorator(decorated_machine, drop_camera_focus);
}

objectmachine_t* make_decorator(objectmachine_t *decorated_machine, void (*strategy)(objectmachine_t*))
{
    objectdecorator_camerafocus_t *me = mallocx(sizeof *me);
    objectdecorator_t *dec = (objectdecorator_t*)me;
    objectmachine_t *obj = (objectmachine_t*)dec;

    obj->init = camerafocus_init;
    obj->release = camerafocus_release;
    obj->update = camerafocus_update;
    obj->render = camerafocus_render;
    obj->get_object_instance = objectdecorator_get_object_instance; /* inherits from superclass */
    dec->decorated_machine = decorated_machine;
    me->strategy = strategy;

    return obj;
}




/* private methods */
void camerafocus_init(objectmachine_t *obj)
{
    objectdecorator_t *dec = (objectdecorator_t*)obj;
    objectmachine_t *decorated_machine = dec->decorated_machine;

    ; /* empty */

    decorated_machine->init(decorated_machine);
}

void camerafocus_release(objectmachine_t *obj)
{
    objectdecorator_t *dec = (objectdecorator_t*)obj;
    objectmachine_t *decorated_machine = dec->decorated_machine;

    ; /* empty */

    decorated_machine->release(decorated_machine);
    free(obj);
}

void camerafocus_update(objectmachine_t *obj, player_t **team, int team_size, brick_list_t *brick_list, item_list_t *item_list, object_list_t *object_list)
{
    objectdecorator_t *dec = (objectdecorator_t*)obj;
    objectmachine_t *decorated_machine = dec->decorated_machine;
    objectdecorator_camerafocus_t *me = (objectdecorator_camerafocus_t*)obj;

    me->strategy(obj);

    decorated_machine->update(decorated_machine, team, team_size, brick_list, item_list, object_list);
}

void camerafocus_render(objectmachine_t *obj, v2d_t camera_position)
{
    objectdecorator_t *dec = (objectdecorator_t*)obj;
    objectmachine_t *decorated_machine = dec->decorated_machine;

    ; /* empty */

    decorated_machine->render(decorated_machine, camera_position);
}


/* private strategies */
void request_camera_focus(objectmachine_t *obj)
{
    level_set_camera_focus( obj->get_object_instance(obj)->actor );
}

void drop_camera_focus(objectmachine_t *obj)
{
    if(level_get_camera_focus() == obj->get_object_instance(obj)->actor)
        level_set_camera_focus( level_player()->actor );
}
