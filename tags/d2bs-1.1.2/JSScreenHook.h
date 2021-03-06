#ifndef __JSSCREENHOOK_H__
#define __JSSCREENHOOK_H__

#include "js32.h"

JSAPI_FUNC(hook_remove);
void hook_finalize(JSContext *cx, JSObject *obj);

/*********************************************************
					Frame Header
**********************************************************/
JSAPI_FUNC(frame_ctor);
JSAPI_PROP(frame_getProperty);
JSAPI_PROP(frame_setProperty);

static JSClass frame_class = {
    "Frame",	JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_PropertyStub, frame_getProperty, frame_setProperty,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, hook_finalize,
	NULL, NULL, NULL, frame_ctor, NULL, NULL, NULL, NULL
};

enum frame_tinyid {
	FRAME_X,
	FRAME_Y,
	FRAME_XSIZE,
	FRAME_YSIZE,
	FRAME_VISIBLE,
	FRAME_ALIGN,
	FRAME_ONCLICK,
	FRAME_ONHOVER,
	FRAME_ZORDER
};

static JSPropertySpec frame_props[] = {
	{"x",		FRAME_X,		JSPROP_STATIC_VAR},
	{"y",		FRAME_Y,		JSPROP_STATIC_VAR},
	{"xsize",	FRAME_XSIZE,	JSPROP_STATIC_VAR},
	{"ysize",	FRAME_YSIZE,	JSPROP_STATIC_VAR},
	{"visible",	FRAME_VISIBLE,	JSPROP_STATIC_VAR},
	{"align",	FRAME_ALIGN,	JSPROP_STATIC_VAR},
	{"zorder",	FRAME_ZORDER,	JSPROP_STATIC_VAR},
	{"click",	FRAME_ONCLICK,	JSPROP_STATIC_VAR},
	{"hover",	FRAME_ONHOVER,	JSPROP_STATIC_VAR},
	{0},
};

static JSFunctionSpec frame_methods[] = {
	{"remove",			hook_remove,			0},
	{0},
};

/*********************************************************
					box Header
**********************************************************/
JSAPI_FUNC(box_ctor);
JSAPI_PROP(box_getProperty);
JSAPI_PROP(box_setProperty);

static JSClass box_class = {
    "Box",	JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_PropertyStub, box_getProperty, box_setProperty,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, hook_finalize,
	NULL, NULL, NULL, box_ctor, NULL, NULL, NULL, NULL
};

enum box_tinyid {
	BOX_X,
	BOX_Y,
	BOX_XSIZE,
	BOX_YSIZE,
	BOX_COLOR,
	BOX_OPACITY,
	BOX_VISIBLE,
	BOX_ALIGN,
	BOX_ONCLICK,
	BOX_ONHOVER,
	BOX_ZORDER
};

static JSPropertySpec box_props[] = {
	{"x",		BOX_X,			JSPROP_STATIC_VAR},
	{"y",		BOX_Y,			JSPROP_STATIC_VAR},
	{"xsize",	BOX_XSIZE,		JSPROP_STATIC_VAR},
	{"ysize",	BOX_YSIZE,		JSPROP_STATIC_VAR},
	{"visible",	BOX_VISIBLE,	JSPROP_STATIC_VAR},
	{"color",	BOX_COLOR,		JSPROP_STATIC_VAR},
	{"opacity",	BOX_OPACITY,	JSPROP_STATIC_VAR},
	{"align",	BOX_ALIGN,		JSPROP_STATIC_VAR},
	{"zorder",	BOX_ZORDER,		JSPROP_STATIC_VAR},
	{"click",	BOX_ONCLICK,	JSPROP_STATIC_VAR},
	{"hover",	BOX_ONHOVER,	JSPROP_STATIC_VAR},
	{0},
};

static JSFunctionSpec box_methods[] = {
	{"remove",			hook_remove,			0},
	{0},
};


/*********************************************************
					Line Header
**********************************************************/
JSAPI_FUNC(line_ctor);
JSAPI_PROP(line_getProperty);
JSAPI_PROP(line_setProperty);

static JSClass line_class = {
    "Line",	JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_PropertyStub, line_getProperty, line_setProperty,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, hook_finalize,
	NULL, NULL, NULL, line_ctor, NULL, NULL, NULL, NULL
};

enum line_tinyid {
	LINE_X,
	LINE_Y,
	LINE_XSIZE,
	LINE_YSIZE,
	LINE_COLOR,
	LINE_VISIBLE,
	LINE_ONCLICK,
	LINE_ONHOVER,
	LINE_ZORDER
};

static JSPropertySpec line_props[] = {
	{"x",		LINE_X,			JSPROP_STATIC_VAR},
	{"y",		LINE_Y,			JSPROP_STATIC_VAR},
	{"x2",		LINE_XSIZE,		JSPROP_STATIC_VAR},
	{"y2",		LINE_YSIZE,		JSPROP_STATIC_VAR},
	{"visible",	LINE_VISIBLE,	JSPROP_STATIC_VAR},
	{"color",	LINE_COLOR,		JSPROP_STATIC_VAR},
	{"zorder",	LINE_ZORDER,	JSPROP_STATIC_VAR},
	{"click",	LINE_ONCLICK,	JSPROP_STATIC_VAR},
	{"hover",	LINE_ONHOVER,	JSPROP_STATIC_VAR},
	{0},
};

static JSFunctionSpec line_methods[] = {
	{"remove",			hook_remove,			0},
	{0},
};

/*********************************************************
					Text Header
**********************************************************/
JSAPI_FUNC(text_ctor);
JSAPI_PROP(text_getProperty);
JSAPI_PROP(text_setProperty);

static JSClass text_class = {
    "Text",	JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_PropertyStub, text_getProperty, text_setProperty,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, hook_finalize,
	NULL, NULL, NULL, text_ctor, NULL, NULL, NULL, NULL
};

enum text_tinyid {
	TEXT_X,
	TEXT_Y,
	TEXT_COLOR,
	TEXT_FONT,
	TEXT_TEXT,
	TEXT_ALIGN,
	TEXT_VISIBLE,
	TEXT_ONCLICK,
	TEXT_ONHOVER,
	TEXT_ZORDER
};

static JSPropertySpec text_props[] = {
	{"x",		TEXT_X,			JSPROP_STATIC_VAR},
	{"y",		TEXT_Y,			JSPROP_STATIC_VAR},
	{"color",	TEXT_COLOR,		JSPROP_STATIC_VAR},
	{"font",	TEXT_FONT,		JSPROP_STATIC_VAR},
	{"visible",	TEXT_VISIBLE,	JSPROP_STATIC_VAR},
	{"text",	TEXT_TEXT,		JSPROP_STATIC_VAR},
	{"align",	TEXT_ALIGN,		JSPROP_STATIC_VAR},
	{"zorder",	TEXT_ZORDER,	JSPROP_STATIC_VAR},
	{"click",	TEXT_ONCLICK,	JSPROP_STATIC_VAR},
	{"hover",	TEXT_ONHOVER,	JSPROP_STATIC_VAR},
	{0},
};

static JSFunctionSpec text_methods[] = {
	{"remove",			hook_remove,			0},
	{0},
};

/*********************************************************
					Image Header
**********************************************************/
JSAPI_FUNC(image_ctor);
JSAPI_PROP(image_getProperty);
JSAPI_PROP(image_setProperty);

static JSClass image_class = {
    "Image",	JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_PropertyStub, image_getProperty, image_setProperty,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, hook_finalize,
	NULL, NULL, NULL, image_ctor, NULL, NULL, NULL, NULL
};

enum image_tinyid {
	IMAGE_X,
	IMAGE_Y,
	IMAGE_LOCATION,
	IMAGE_ALIGN,
	IMAGE_VISIBLE,
	IMAGE_ONCLICK,
	IMAGE_ONHOVER,
	IMAGE_ZORDER
};

static JSPropertySpec image_props[] = {
	{"x",		IMAGE_X,			JSPROP_STATIC_VAR},
	{"y",		IMAGE_Y,			JSPROP_STATIC_VAR},
	{"visible",	IMAGE_VISIBLE,		JSPROP_STATIC_VAR},
	{"location",IMAGE_LOCATION,		JSPROP_STATIC_VAR},
	{"align",	IMAGE_ALIGN,		JSPROP_STATIC_VAR},
	{"zorder",	IMAGE_ZORDER,		JSPROP_STATIC_VAR},
	{"click",	IMAGE_ONCLICK,		JSPROP_STATIC_VAR},
	{"hover",	IMAGE_ONHOVER,		JSPROP_STATIC_VAR},
	{0},
};

static JSFunctionSpec image_methods[] = {
	{"remove",			hook_remove,			0},
	{0},
};

#endif
