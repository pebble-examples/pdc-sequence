#include <pebble.h>

// Milliseconds between frames
#define DELTA 13

static int s_index = 0;

static Window *s_main_window;
static Layer *s_canvas_layer;
static GDrawCommandSequence *s_command_seq;

static void next_frame_handler(void *context) {
  // Draw the next frame
  layer_mark_dirty(s_canvas_layer);

  // Continue the sequence
  app_timer_register(DELTA, next_frame_handler, NULL);
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
  light_enable(true);

  // Load the sequence
  s_command_seq = gdraw_command_sequence_create_with_resource(RESOURCE_ID_CLOCK_SEQUENCE);

  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });
  window_stack_push(s_main_window, true);

  // Start the animation
  app_timer_register(DELTA, next_frame_handler, NULL);
}

static void deinit() {
  window_destroy(s_main_window);
}

int main() {
  init();
  app_event_loop();
  deinit();
}
