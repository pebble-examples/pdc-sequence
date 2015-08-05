#include <pebble.h>

// Milliseconds between frames
#define DELTA 13

// Number of resources
#define NUM_SEQUENCES 9

static Window *s_main_window;
static Layer *s_canvas_layer;
static GDrawCommandSequence *s_command_seq;
static AppTimer *s_timer;

static int s_index = 0, s_resource_index = 1;

static void next_frame_handler(void *context) {
  // Draw the next frame
  layer_mark_dirty(s_canvas_layer);

  // Continue the sequence
  s_timer = app_timer_register(DELTA, next_frame_handler, NULL);
}

static void change_sequence(int delta) {
  // Cancel current animation
  if(s_timer) {
    app_timer_cancel(s_timer);
  }
  if(s_command_seq) {
    gdraw_command_sequence_destroy(s_command_seq);
  }

  // Load the next resource
  s_resource_index += delta;
  if(s_resource_index > NUM_SEQUENCES) {
    // 0 is a reserved resource ID
    s_resource_index = 1;
  }
  if(s_resource_index < 1) {
    s_resource_index = NUM_SEQUENCES;
  }
  s_command_seq = gdraw_command_sequence_create_with_resource(s_resource_index);
  window_set_background_color(s_main_window, (GColor){ .a = 3, .r = rand() % 4, .g = rand() % 4, .b = rand() % 4 });

  // Start the next animation  
  s_index = 0;
  s_timer = app_timer_register(DELTA, next_frame_handler, NULL);
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  change_sequence(-1);
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  change_sequence(1);
}

static void config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void update_proc(Layer *layer, GContext *ctx) {
  // Get the next frame
  GDrawCommandFrame *frame = gdraw_command_sequence_get_frame_by_index(s_command_seq, s_index);

  // If another frame was found, draw it    
  if (frame) {
    gdraw_command_frame_draw(ctx, s_command_seq, frame, GPoint(0, 30));
  }

  // Advance to the next frame, wrapping if neccessary
  int num_frames = gdraw_command_sequence_get_num_frames(s_command_seq);
  s_index++;
  if (s_index == num_frames) {
    s_index = 0;
  }
}

static void main_window_load(Window *window) {
  // Get Window information
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // Create the canvas Layer
  s_canvas_layer = layer_create(GRect(30, 30, bounds.size.w, bounds.size.h));

  // Set the LayerUpdateProc
  layer_set_update_proc(s_canvas_layer, update_proc);

  // Add to parent Window
  layer_add_child(window_layer, s_canvas_layer);
}

static void main_window_unload(Window *window) {
  layer_destroy(s_canvas_layer);
  gdraw_command_sequence_destroy(s_command_seq);
}

static void init() {
  // Load the first sequence
  s_command_seq = gdraw_command_sequence_create_with_resource(1);

  s_main_window = window_create();
  window_set_click_config_provider(s_main_window, config_provider);
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });
  window_stack_push(s_main_window, true);

  // Start the animation
  s_timer = app_timer_register(DELTA, next_frame_handler, NULL);
}

static void deinit() {
  window_destroy(s_main_window);
}

int main() {
  init();
  app_event_loop();
  deinit();
}
