#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"
#include "mofftime.h"
#include "mstrftime.h"
#include "mtimedefs.h"

#define MY_UUID { 0x2B, 0xF5, 0xFE, 0x91, 0xD2, 0x71, 0x4A, 0x6E, 0xA8, 0x21, 0x78, 0x61, 0x39, 0x00, 0x8E, 0xE9 }
PBL_APP_INFO(MY_UUID,
             "Martian Pebbles", "Midgard Systems",
             1, 2, /* App version */
             DEFAULT_MENU_ICON,
             APP_INFO_WATCH_FACE);

Window window;
TextLayer gregorian_time;
TextLayer gregorian_date;
TextLayer aresian_time;
TextLayer aresian_date;

#define BUFFER_SIZE 32
char gd_buffer[BUFFER_SIZE];
char gt_buffer[BUFFER_SIZE];
char ad_buffer[BUFFER_SIZE];
char at_buffer[BUFFER_SIZE];

void display_gregorian(PblTm *tick_time) {
  static char *t_fmt, *d_fmt;
  if(clock_is_24h_style()) {
    t_fmt = "%H:%M:%S";
  } else {
    t_fmt = "%I:%M:%S %p";
  }
  d_fmt = "%a %e %b %Y";
  string_format_time(gt_buffer, BUFFER_SIZE, t_fmt, tick_time);
  text_layer_set_text(&gregorian_time, gt_buffer);
  string_format_time(gd_buffer, BUFFER_SIZE, d_fmt, tick_time);
  text_layer_set_text(&gregorian_date, gd_buffer);
}

#define LOCAL_GMT_OFFSET_AT_MIDNIGHT_ONE_JAN_2013 -8
#define MIDNIGHT_ONE_JAN_2013_UTC 1356998400

static int month_len[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

time_t pebble_to_epoch(PblTm *time) {
  int epoch = MIDNIGHT_ONE_JAN_2013_UTC;
  epoch += time->tm_sec;
  epoch += (60 * time->tm_min);
  epoch += (3600 * time->tm_hour);
  epoch += (86400 * (time->tm_mday - 1));
  for(int m = 0; m < time->tm_mon; m++) {
    epoch += (86400 * month_len[m]);
    if(m == 2)
      epoch -= 3600; // DST
    if(m == 10)
      epoch += 3600; // DST
  }
  // tm_isdst is always false
  // if(!time->tm_isdst)
  //   epoch -= 3600;
  for(int y = 2013; y < time->tm_year; y++) {
    epoch += (86400 * 365);
    if(__isleap(y))
      epoch += 86400;
  }
  if(__isleap(time->tm_year) && time->tm_mon > 1)
    epoch += 86400;
  epoch -= (3600 * LOCAL_GMT_OFFSET_AT_MIDNIGHT_ONE_JAN_2013);
  return epoch;
}

void display_aresian(PblTm *tick_time) {
  time_t now = pebble_to_epoch(tick_time);
  struct tm mars_tick_time;
  m_offtime((const time_t *)&now, 1, &mars_tick_time);
  mstrftime(at_buffer, BUFFER_SIZE, "%H:%M:%S", &mars_tick_time);
  text_layer_set_text(&aresian_time, at_buffer);
  mstrftime(ad_buffer, BUFFER_SIZE, "%a %e %b %Y", &mars_tick_time);
  text_layer_set_text(&aresian_date, ad_buffer);
  
  if(mars_tick_time.tm_hour == 23 &&
     mars_tick_time.tm_min  == 59) {
    if(mars_tick_time.tm_sec == 0) {
       vibes_short_pulse();
    } else if(mars_tick_time.tm_sec == 57 ||
            mars_tick_time.tm_sec == 58 ||
            mars_tick_time.tm_sec == 59) {
      vibes_double_pulse();
    }
  } else if(mars_tick_time.tm_hour == 24 &&
          mars_tick_time.tm_min  == 0 &&
          mars_tick_time.tm_sec  == 0) {
    vibes_long_pulse();
  } else if(mars_tick_time.tm_hour == 24 &&
          mars_tick_time.tm_min  == 38 &&
          mars_tick_time.tm_sec  == 35) {
    vibes_short_pulse();
  } else if(mars_tick_time.tm_hour == 24 &&
          mars_tick_time.tm_min  == 39 &&
          (mars_tick_time.tm_sec == 32 ||
           mars_tick_time.tm_sec == 33 ||
           mars_tick_time.tm_sec == 34)) {
      vibes_double_pulse();
  } else if(mars_tick_time.tm_hour == 0 &&
          mars_tick_time.tm_min  == 0 &&
          mars_tick_time.tm_sec  == 0) {
    vibes_long_pulse();
  }
}

void handle_tick(AppContextRef ctx, PebbleTickEvent *event) {
  display_gregorian(event->tick_time);
  display_aresian(event->tick_time);
}

#define TIME_FONT FONT_KEY_DROID_SERIF_28_BOLD
#define time_height 29
#define DATE_FONT FONT_KEY_GOTHIC_24_BOLD
#define date_height 28
#define block_space 0
#define screen_height 168
#define screen_width 144

void handle_init(AppContextRef ctx) {
  GRect frame;
  (void)ctx;
  
  window_init(&window, "main");
  window_stack_push(&window, true /* Animated */);
  window_set_background_color(&window, GColorBlack);
  
  // int window_height = window.layer.frame.size.h;
  // int frame_height = window_height / 3;
  // int frame_padding = ( window_height - ( 2 * frame_height ) ) / 3;
  // frame = window.layer.frame;
  // frame.size.h = frame_height;
  // frame.origin.y = frame_padding;
  // text_layer_init(&gregorian, frame);
  // frame.origin.y = frame_padding + frame_height + frame_padding;
  // text_layer_init(&aresian,   frame);
  
  int block_height = (time_height + block_space + date_height);
  int margin = (screen_height - (2 * block_height)) / 3;
  frame.size.w = screen_width;
  frame.origin.x = 0;
  frame.size.h = time_height;
  frame.origin.y = margin;
  text_layer_init(&gregorian_time, frame);
  frame.size.h = date_height;
  frame.origin.y += time_height + block_space;
  text_layer_init(&gregorian_date, frame);
  frame.size.h = time_height;
  frame.origin.y += date_height + margin;
  text_layer_init(&aresian_time,   frame);
  frame.size.h = date_height;
  frame.origin.y += time_height + block_space;
  text_layer_init(&aresian_date,   frame);
  
  text_layer_set_background_color(&gregorian_time, GColorBlack);
  text_layer_set_background_color(&gregorian_date, GColorBlack);
  text_layer_set_background_color(&aresian_time,   GColorBlack);
  text_layer_set_background_color(&aresian_date,   GColorBlack);
  text_layer_set_text_color(&gregorian_time, GColorWhite);
  text_layer_set_text_color(&gregorian_date, GColorWhite);
  text_layer_set_text_color(&aresian_time,   GColorWhite);
  text_layer_set_text_color(&aresian_date,   GColorWhite);
  
  text_layer_set_font(&gregorian_time, fonts_get_system_font(TIME_FONT));
  text_layer_set_font(&aresian_time,   fonts_get_system_font(TIME_FONT));
  text_layer_set_font(&gregorian_date, fonts_get_system_font(DATE_FONT));
  text_layer_set_font(&aresian_date,   fonts_get_system_font(DATE_FONT));
  
  text_layer_set_text_alignment(&gregorian_time, GTextAlignmentCenter);
  text_layer_set_text_alignment(&gregorian_date, GTextAlignmentCenter);
  text_layer_set_text_alignment(&aresian_time,   GTextAlignmentCenter);
  text_layer_set_text_alignment(&aresian_date,   GTextAlignmentCenter);
  
  layer_add_child(&window.layer, &gregorian_time.layer);
  layer_add_child(&window.layer, &gregorian_date.layer);
  layer_add_child(&window.layer, &aresian_time.layer);
  layer_add_child(&window.layer, &aresian_date.layer);
  
  PblTm tick_time;
  get_time(&tick_time);
  display_gregorian(&tick_time);
  display_aresian(&tick_time);
}


void pbl_main(void *params) {
  PebbleAppHandlers handlers = {
    .init_handler   = &handle_init,
    // .deinit_handler = &handle_deinit,

    .tick_info = {
      .tick_handler = &handle_tick,
      .tick_units   = SECOND_UNIT
    }
  };
  app_event_loop(params, &handlers);
}
