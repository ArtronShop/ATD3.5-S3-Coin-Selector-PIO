#include <Arduino.h>
#include <lvgl.h>
#include <ATD3.5-S3.h>
#include "gui/ui.h"

#define EN_PIN  1
#define SIG_PIN 2

volatile bool pulse_trigger_flag = false;
int total_money = 0;
int item_price = 10;

void update_index_ui() {
  lv_label_set_text_fmt(ui_money, "%.0f", (float) total_money);
  if (total_money >= item_price) {
    lv_obj_clear_state(ui_ok_btn, LV_STATE_DISABLED);
  } else {
    lv_obj_add_state(ui_ok_btn, LV_STATE_DISABLED);
  }
}

void item_select_cb(lv_event_t * e) {
  item_price = (int) lv_event_get_user_data(e);
  update_index_ui();
}

void setup() {
  Serial.begin(115200);
  
  // Setup peripherals
  Display.begin(0); // rotation number 0
  Touch.begin();
  Sound.begin();
  // Card.begin(); // uncomment if you want to Read/Write/Play/Load file in MicroSD Card
  pinMode(EN_PIN, OUTPUT);
  digitalWrite(EN_PIN, HIGH);
  delay(100);
  pinMode(SIG_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(SIG_PIN), []() {
    pulse_trigger_flag = true;
  }, FALLING);
  
  // Map peripheral to LVGL
  Display.useLVGL(); // Map display to LVGL
  Touch.useLVGL(); // Map touch screen to LVGL
  Sound.useLVGL(); // Map speaker to LVGL
  // Card.useLVGL(); // Map MicroSD Card to LVGL File System

  Display.enableAutoSleep(120); // Auto off display if not touch in 2 min
  
  // Add load your UI function
  ui_init();

  // Add event handle
  lv_obj_add_event_cb(ui_item1, item_select_cb, LV_EVENT_CLICKED, (void*) 10);
  lv_obj_add_event_cb(ui_item2, item_select_cb, LV_EVENT_CLICKED, (void*) 10);
  lv_obj_add_event_cb(ui_item3, item_select_cb, LV_EVENT_CLICKED, (void*) 10);

  lv_obj_add_event_cb(ui_ok_btn, [](lv_event_t * e) {
    total_money -= item_price;
    update_index_ui();
    digitalWrite(EN_PIN, LOW);
    lv_disp_load_scr(ui_ok);
    
    lv_timer_create([](lv_timer_t * timer) {
      lv_disp_load_scr(ui_Index);
      digitalWrite(EN_PIN, HIGH);
      delay(100);
      pulse_trigger_flag = false;
      lv_timer_del(timer);
    }, 3000, NULL);
  }, LV_EVENT_CLICKED, NULL);

  update_index_ui();
}

void loop() {
  Display.loop(); // Keep GUI work

  if (pulse_trigger_flag) {
    total_money += 5;
    update_index_ui();
    delay(200);
    pulse_trigger_flag = false;
  }
}
