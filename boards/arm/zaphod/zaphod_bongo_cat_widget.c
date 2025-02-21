/*
 * Copyright (c) 2021 Pete Johanson
 *
 * SPDX-License-Identifier: MIT
 */

#include <zmk/event_manager.h>
#include <zmk/events/wpm_state_changed.h>

#include <logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include "zaphod_bongo_cat_widget.h"

static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);

enum anim_state {
	anim_state_none,
	anim_state_idle,
	anim_state_slow,
	anim_state_fast
} current_anim_state;

const void* *images;
uint8_t images_len;

LV_IMG_DECLARE(idle_img1);
LV_IMG_DECLARE(idle_img2);
LV_IMG_DECLARE(idle_img3);
LV_IMG_DECLARE(idle_img4);
LV_IMG_DECLARE(idle_img5);

LV_IMG_DECLARE(slow_img);

LV_IMG_DECLARE(fast_img1);
LV_IMG_DECLARE(fast_img2);

const void* idle_images[] = {
	&idle_img1,
	&idle_img2,
	&idle_img3,
	&idle_img4,
	&idle_img5,
};

const void* fast_images[] = {
	&fast_img1,
	&fast_img2,
};

void set_img_src(void *var, lv_anim_value_t val) {
    lv_obj_t *img = (lv_obj_t *)var;
    lv_img_set_src(img, images[val]);
}



void state_widget_wpm(struct zaphod_bongo_cat_widget *widget, int wpm) {
    LOG_DBG("anim state %d", current_anim_state);
    loop:while (wpm = 0) {
	lv_anim_init(&widget->anim);
        lv_anim_set_var(&widget->anim, widget->obj);
	lv_anim_set_time(&widget->anim, 1000);
	lv_anim_set_values(&widget->anim, 0, 4);
	lv_anim_set_exec_cb(&widget->anim, set_img_src);
	lv_anim_set_repeat_count(&widget->anim, 10);
	images = idle_images;
	current_anim_state = anim_state_idle;
	lv_anim_start(&widget->anim);
	goto loop;
	}
    if (wpm < CONFIG_ZAPHOD_BONGO_CAT_IDLE_LIMIT) {
	if (current_anim_state != anim_state_idle) {
	    LOG_DBG("Set source to idle images!");
            lv_anim_init(&widget->anim);
            lv_anim_set_var(&widget->anim, widget->obj);
	    lv_anim_set_time(&widget->anim, 1000);
	    lv_anim_set_values(&widget->anim, 0, 4);
	    lv_anim_set_exec_cb(&widget->anim, set_img_src);
	    lv_anim_set_repeat_count(&widget->anim, LV_ANIM_REPEAT_INFINITE);
	    lv_anim_set_repeat_delay(&widget->anim, 100);
	    images = idle_images;
	    current_anim_state = anim_state_idle;
	    lv_anim_start(&widget->anim);
	}
    } else if (wpm < CONFIG_ZAPHOD_BONGO_CAT_SLOW_LIMIT) {
	if (current_anim_state != anim_state_slow) {
	    LOG_DBG("Set source to slow image!");
	    lv_anim_del(widget->obj, set_img_src);
	    lv_img_set_src(widget->obj, &slow_img);
	    current_anim_state = anim_state_slow;
	}
    } else {
	if (current_anim_state != anim_state_fast) {
	    LOG_DBG("Set source to fast images!");
            lv_anim_init(&widget->anim);
	    lv_anim_set_time(&widget->anim, 500);
	    lv_anim_set_repeat_delay(&widget->anim, 500);
            lv_anim_set_var(&widget->anim, widget->obj);
	    lv_anim_set_values(&widget->anim, 0, 1);
	    lv_anim_set_exec_cb(&widget->anim, set_img_src);
	    lv_anim_set_repeat_count(&widget->anim, LV_ANIM_REPEAT_INFINITE);
	    images = fast_images;
	    current_anim_state = anim_state_fast;
	    lv_anim_start(&widget->anim);
	}
    }
}

int zaphod_bongo_cat_widget_init(struct zaphod_bongo_cat_widget *widget, lv_obj_t *parent) {
    widget->obj = lv_img_create(parent, NULL);
    

    lv_img_set_auto_size(widget->obj, true);
    state_widget_wpm(widget, 0);

    sys_slist_append(&widgets, &widget->node);

    return 0;
}

lv_obj_t *zaphod_bongo_cat_widget_obj(struct zaphod_bongo_cat_widget  *widget) {
    return widget->obj;
}

int wpm_status_listener(const zmk_event_t *eh) {
    struct zaphod_bongo_cat_widget *widget;
    struct zmk_wpm_state_changed *ev = as_zmk_wpm_state_changed(eh);
    LOG_DBG("LISTENER");
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) { LOG_DBG("Set the WPM %d", ev->state); state_widget_wpm(widget, ev->state); }
    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(zaphod_bongo_cat_widget, wpm_status_listener)
ZMK_SUBSCRIPTION(zaphod_bongo_cat_widget, zmk_wpm_state_changed);
