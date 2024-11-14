#include <Arduino.h>
#include <lvgl.h>
#include "display.h"
#include "esp_bsp.h"
#include "lv_port.h"

/**
 * Set the rotation degree:
 *      - 0: 0 degree
 *      - 90: 90 degree
 *      - 180: 180 degree
 *      - 270: 270 degree
 *
 */
#define LVGL_PORT_ROTATION_DEGREE               (90)

/**
/* To use the built-in examples of LVGL uncomment the include below.
 * You also need to copy `lvgl/examples` to `lvgl/src/examples`. 
 */
#include <examples/lv_examples.h>



// Episode data structure
struct Episode {
    uint8_t number;
    const char* title;
};

// Episode database
const Episode EPISODES[] = {
    {1, "Ship In A Bottle"},
    {2, "The Owls of Athens"},
    {3, "The Frog Princess"},
    {4, "The Ballet Shoe"},
    {5, "The Hamish"},
    {6, "The Wise Man"},
    {7, "The Elephant"},
    {8, "The Mouse Mill"},
    {9, "The Giant"},
    {10, "The Old Man's Beard"},
    {11, "The Fiddle"},
    {12, "Flying"},
    {13, "Uncle Feedle"}
};

const int NUM_EPISODES = sizeof(EPISODES) / sizeof(EPISODES[0]);

// Function to get episode title by number
const char* getEpisodeTitle(uint8_t episodeNumber) {
    if (episodeNumber < 1 || episodeNumber > NUM_EPISODES) {
        return "Invalid Episode";
    }
    return EPISODES[episodeNumber - 1].title;
}



static void scroll_event_cb(lv_event_t * e)
{
    lv_obj_t * cont = lv_event_get_target(e);

    lv_area_t cont_a;
    lv_obj_get_coords(cont, &cont_a);
    lv_coord_t cont_y_center = cont_a.y1 + lv_area_get_height(&cont_a) / 2;

    lv_coord_t r = lv_obj_get_height(cont) * 5 / 10;
    uint32_t i;
    uint32_t child_cnt = lv_obj_get_child_cnt(cont);
    for(i = 0; i < child_cnt; i++) {
        lv_obj_t * child = lv_obj_get_child(cont, i);
        lv_area_t child_a;
        lv_obj_get_coords(child, &child_a);

        lv_coord_t child_y_center = child_a.y1 + lv_area_get_height(&child_a) / 2;

        lv_coord_t diff_y = child_y_center - cont_y_center;
        diff_y = LV_ABS(diff_y);

        /*Get the x of diff_y on a circle.*/
        lv_coord_t x;
        /*If diff_y is out of the circle use the last point of the circle (the radius)*/
        if(diff_y >= r) {
            x = r;
        }
        else {
            /*Use Pythagoras theorem to get x from radius and y*/
            uint32_t x_sqr = r * r - diff_y * diff_y;
            lv_sqrt_res_t res;
            lv_sqrt(x_sqr, &res, 0x8000);   /*Use lvgl's built in sqrt root function*/
            x = r - res.i;
        }

        /*Translate the item by the calculated X coordinate*/
        lv_obj_set_style_translate_x(child, x, 0);

        /*Use some opacity with larger translations*/
        lv_opa_t opa = lv_map(x, 0, r, LV_OPA_TRANSP, LV_OPA_COVER);
        lv_obj_set_style_opa(child, LV_OPA_COVER - opa, 0);

        // If this is the centered item, update the title display
        if (diff_y < 10) { // Small threshold for "centered" state
            // Get the episode number from the button's label
            lv_obj_t* label = lv_obj_get_child(child, 0);
            const char* btn_text = lv_label_get_text(label);
            int episode_num;
            sscanf(btn_text, "Episode %d", &episode_num);
            
            // Get and display the episode title
            const char* title = getEpisodeTitle(episode_num);
            
            // Update play button label
            lv_obj_t* play_label = (lv_obj_t*)lv_obj_get_user_data(cont);
            if (play_label) {
                lv_label_set_text_fmt(play_label, "Play - %s", title);
            }
        }
    }
}


void select_episode(void)
{
    // Set background color
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x003a57), LV_PART_MAIN);
    

    // Create and style the play button
    lv_obj_t * play_btn = lv_btn_create(lv_scr_act());
    lv_obj_set_size(play_btn, 400, 40);  // Set button size
    lv_obj_align(play_btn, LV_ALIGN_BOTTOM_MID, 0, -10);  // Position at bottom
    
    // Style the play button
    lv_obj_set_style_bg_color(play_btn, lv_color_hex(0x00aa00), LV_PART_MAIN);  // Green background
    lv_obj_set_style_bg_color(play_btn, lv_color_hex(0x008800), LV_STATE_PRESSED);  // Darker when pressed
    lv_obj_set_style_shadow_width(play_btn, 10, 0);
    lv_obj_set_style_shadow_color(play_btn, lv_color_hex(0x000000), 0);
    lv_obj_set_style_shadow_opa(play_btn, LV_OPA_30, 0);
    
    // Create label for the play button
    lv_obj_t * play_label = lv_label_create(play_btn);
    lv_obj_set_style_text_font(play_label, &lv_font_montserrat_24, 0);
    lv_label_set_text(play_label, "Play - Ship In A Bottle");  // Initial text
    lv_obj_center(play_label);






    // Create the container for episodes (moved up to accommodate play button)
    lv_obj_t * cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(cont, 240, 240);  
    lv_obj_align(cont, LV_ALIGN_CENTER, 0, -25);  // Moved up slightly
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_add_event_cb(cont, scroll_event_cb, LV_EVENT_SCROLL, NULL);
    lv_obj_set_style_radius(cont, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_clip_corner(cont, true, 0);
    lv_obj_set_scroll_dir(cont, LV_DIR_VER);
    lv_obj_set_scroll_snap_y(cont, LV_SCROLL_SNAP_CENTER);
    lv_obj_set_scrollbar_mode(cont, LV_SCROLLBAR_MODE_OFF);
    
    // Store play button label pointer in container's user data
    lv_obj_set_user_data(cont, play_label);

    // Create episode buttons
    for(uint32_t i = 0; i < NUM_EPISODES; i++) {
        lv_obj_t * btn = lv_btn_create(cont);
        lv_obj_set_width(btn, lv_pct(100));
        
        // Style the button
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x005577), LV_PART_MAIN);
        lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, LV_PART_MAIN);
        
        lv_obj_t * label = lv_label_create(btn);
        lv_label_set_text_fmt(label, "Episode %"LV_PRIu32, i+1);
        lv_obj_center(label);
    }

    // Update the buttons position manually for first
    lv_event_send(cont, LV_EVENT_SCROLL, NULL);

    // Be sure the first button is in the middle
    lv_obj_scroll_to_view(lv_obj_get_child(cont, 0), LV_ANIM_OFF);
}

void setup()
{
    String title = "LVGL porting example";

    Serial.begin(115200);
    Serial.println(title + " start");

    Serial.println("Initialize panel device");
    bsp_display_cfg_t cfg = {
        .lvgl_port_cfg = ESP_LVGL_PORT_INIT_CONFIG(),
        .buffer_size = EXAMPLE_LCD_QSPI_H_RES * EXAMPLE_LCD_QSPI_V_RES,
#if LVGL_PORT_ROTATION_DEGREE == 90
        .rotate = LV_DISP_ROT_90,
#elif LVGL_PORT_ROTATION_DEGREE == 270
        .rotate = LV_DISP_ROT_270,
#elif LVGL_PORT_ROTATION_DEGREE == 180
        .rotate = LV_DISP_ROT_180,
#elif LVGL_PORT_ROTATION_DEGREE == 0
        .rotate = LV_DISP_ROT_NONE,
#endif
    };

    bsp_display_start_with_config(&cfg);
    bsp_display_backlight_on();

    Serial.println("Create UI");
    /* Lock the mutex due to the LVGL APIs are not thread-safe */
    bsp_display_lock(0);

    select_episode();

    /* Release the mutex */
    bsp_display_unlock();

    Serial.println(title + " end");
}

void loop()
{
    Serial.println("IDLE loop");
    delay(1000);
}