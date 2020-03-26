#pragma once

#include <time.h>
#include <Windows.h>


struct gdi_radar_config {
	LPCWSTR className;
	LPCWSTR windowName;
	double minimumUpdateTime;
	UINT64 maximumRedrawFails;
	size_t reservedEntities;
	bool drawAngles;
};

struct gdi_radar_context;


static inline HINSTANCE gdi_radar_get_fake_hinstance()
{
	LONG_PTR hi = GetWindowLongW(GetActiveWindow(), -6);
	return (HINSTANCE)hi;
}
HWND gdi_radar_get_hwnd(struct gdi_radar_context * const ctx);
struct gdi_radar_context * const
	gdi_radar_configure(struct gdi_radar_config const * const cfg,
		HINSTANCE hInst);
bool gdi_radar_init(struct gdi_radar_context * const ctx);


enum entity_color {
	EC_BLUE, EC_BLACK, EC_RED
};

struct entity {
	int pos[2];
	float angle;
	int angle_line_length;
	enum entity_color color;
	const char *name;
};

static inline float degree2radian(int a) {
	return (a * 0.017453292519f);
}
void gdi_radar_add_entity(struct gdi_radar_context * const ctx,
	struct entity * const ent);
void gdi_radar_set_entity(struct gdi_radar_context * const ctx, size_t i,
	struct entity * const ent);
void gdi_radar_clear_entities(struct gdi_radar_context * const ctx);
bool gdi_radar_check_if_redraw_necessary(struct gdi_radar_context * const ctx);
bool gdi_radar_redraw_if_necessary(struct gdi_radar_context * const ctx);
void gdi_radar_set_game_dimensions(struct gdi_radar_context * const ctx,
	UINT64 GameMapWidth, UINT64 GameMapHeight, bool StickToBottom = true);
static inline void gdi_radar_set_game_dimensions(
	struct gdi_radar_context * const ctx,
	float GameMapWidth, float GameMapHeight, bool StickToBottom = true)
{
	gdi_radar_set_game_dimensions(ctx,
		(UINT64)GameMapWidth, (UINT64)GameMapHeight, StickToBottom);
}
LRESULT gdi_radar_process_window_events_blocking(struct gdi_radar_context * const ctx);
LRESULT gdi_radar_process_window_events_nonblocking(struct gdi_radar_context * const ctx);
void gdi_radar_close_and_cleanup(struct gdi_radar_context ** const ctx);