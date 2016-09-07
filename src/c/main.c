#include <pebble.h>

#define KEY_PRICE 0
#define KEY_HEIGHT 1

static Window *s_main_window; // static window

static TextLayer *s_price_layer; // bitcoin price
static TextLayer *s_time_layer; // static text layer
static TextLayer *s_block_layer; // block # layer under logo


static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  // Write the current hours and minutes into a buffer
  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ?
  "%H:%M" : "%I:%M", tick_time);

  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, s_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
   // Get bitcoin data updates every 5 minutes
  if(tick_time->tm_min % 5 == 0) {
    // Begin dictionary
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);

    // Add a key-value pair
    dict_write_uint8(iter, 0, 0);

    // Send the message!
    app_message_outbox_send();
  }
}


static void main_window_load(Window *window) {
 // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  // Create the TextLayer for price
  s_price_layer = text_layer_create(GRect(0, 10, 144, 50));
  text_layer_set_background_color(s_price_layer, GColorClear);
  text_layer_set_text_color(s_price_layer, GColorBlack);
  text_layer_set_text_alignment(s_price_layer, GTextAlignmentCenter);
  text_layer_set_font(s_price_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text(s_price_layer, "Loading..."); //"BTC/USD $606"); // fill in for now
  
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_price_layer));
  // Create the TextLayer with specific bounds
  s_time_layer = text_layer_create(
      GRect(0, PBL_IF_ROUND_ELSE(58, 52), bounds.size.w, 50));

  // Improve the layout to be more like a watchface
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_LECO_42_NUMBERS));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  
  // Create the TextLayer for block
  s_block_layer = text_layer_create(GRect(0,125,144,168));
  text_layer_set_background_color(s_block_layer, GColorClear);
  text_layer_set_text_color(s_block_layer, GColorBlack);
  text_layer_set_text_alignment(s_block_layer, GTextAlignmentCenter);
  text_layer_set_font(s_block_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text(s_block_layer,"Loading..."); // "Block 420444"); // fill in for now
  
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_block_layer));
}

static void main_window_unload(Window *window) {
  // Destroy TextLayer
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_block_layer);
  text_layer_destroy(s_price_layer);
}


static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
   // Store incoming information
  static char price_buffer[16];
  static char block_buffer[16];

  // Read tuples for data
  Tuple *price_tuple = dict_find(iterator, KEY_PRICE);
  Tuple *block_tuple = dict_find(iterator, KEY_HEIGHT);

  // If all data is available, use it
  if(price_tuple && block_tuple) {
    snprintf(price_buffer, sizeof(price_buffer), "$%d", (int)price_tuple->value->int32);
    snprintf(block_buffer, sizeof(block_buffer), "%d", (int)block_tuple->value->int32);
    
    //set text
    text_layer_set_text(s_price_layer, price_buffer);
    text_layer_set_text(s_block_layer, block_buffer );
  }
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}
static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();
  
  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  
   // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  // Make sure the time is displayed from the start
  update_time();
  
  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  
    // Open AppMessage
  const int inbox_size = 128;
  const int outbox_size = 128;
  app_message_open(inbox_size, outbox_size);
}

static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
}

// main function
int main(void) {
  init();
  app_event_loop();
  deinit();
}