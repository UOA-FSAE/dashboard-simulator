#include <screens.h>
#include <lvgl.h>
#include <vehicle.h>

#ifndef USE_SIMULATOR
#include <ltdc.h>
#endif

extern volatile Vehicle_Data the_vehicle;

#define LINE_HEIGHT 14
#define LEFT_COLUMN_TAB 5

// Driver Type
#define SOFTWARE_INVERT_SCREEN 0    // TODO: implement this?
#define USE_DOUBLE_BUFFER 0

// Screen switching flags
enum SCREENS current_screen;

static volatile uint8_t screen_switch_flag = 0;
static volatile uint8_t screen_update_flag = 0;

// Display Driver
static lv_disp_draw_buf_t the_display_buf;
static lv_disp_drv_t the_display_drv;                 /*A variable to hold the drivers.*/

lv_disp_t * disp;

// Static or global buffer(s).
#if USE_DOUBLE_BUFFER
volatile lv_color_t *buf_1 = (lv_color_t *)0xC0000000;
volatile lv_color_t *buf_2 = (lv_color_t *)0xC0000000+272*480*4;
#else
volatile lv_color_t buf_1[100*100];
volatile lv_color_t buf_2[100*100];
#endif

///////////
// Fonts //
///////////
LV_FONT_DECLARE(bitstream_vera_sans_26)
LV_FONT_DECLARE(bitstream_vera_sans_30)
LV_FONT_DECLARE(bitstream_vera_sans_80)

////////////
// Styles //
////////////
static lv_style_t speedometer_style;
static lv_style_t colored_label_style;
static lv_style_t regular_text;
static lv_style_t title_text;       // Style for title text (unused)
static lv_style_t accumulator_style;
static lv_style_t text_box_style; // Text box style (for border)


static lv_style_t regen_style;    // Regen Style

/////////////
// Objects //
/////////////
lv_obj_t *driver_screen;        // Driver Screen Objects

lv_obj_t * rpm_arc;
lv_obj_t * throttle_arc;
lv_obj_t * rpm_label;
lv_obj_t * throttle_label;

lv_obj_t *debug_screen;         // Debug Screen and Objects

lv_obj_t *dbs_battery_voltage;
lv_obj_t *dbs_steering_position;
lv_obj_t *dbs_throttle_position;
lv_obj_t *dbs_brake_pressure_front;
lv_obj_t *dbs_brake_pressure_rear;
lv_obj_t *dbs_acc_soc;
lv_obj_t *dbs_acc_lowest_cell_voltage;
lv_obj_t *dbs_acc_highest_cell_voltage;
lv_obj_t *dbs_fl_motor_inverter_temp_err_code;
lv_obj_t *dbs_fr_motor_inverter_temp_err_code;
lv_obj_t *dbs_rl_motor_inverter_temp_err_code;
lv_obj_t *dbs_rr_motor_inverter_temp_err_code;
lv_obj_t *dbs_motor_loop_coolant_temp;
lv_obj_t *dbs_inverter_loop_coolant_temp;

lv_obj_t *energy_screen;    // energy screen objects

lv_obj_t * regen_power;
lv_obj_t * regen_label;
lv_obj_t * accumulator_power;
lv_obj_t * accumulator_text;
lv_obj_t * accumulator_label;
lv_obj_t * glv_power;
lv_obj_t * glv_text;
lv_obj_t * glv_label;
lv_obj_t * power_mode;
lv_obj_t * power_mode_label;

lv_obj_t * lap_screen;  //  Lap time screen objects

lv_obj_t * current_lap_time;
lv_obj_t * lap_delta;
lv_obj_t * best_lap_time;
lv_obj_t * previous_lap_time;
lv_obj_t * lap_number;
lv_obj_t * lap_label;

void init_screens() {
    ////////////
    // Styles //
    ////////////

    lv_style_init(&regular_text);
    lv_style_set_text_font(&regular_text,&lv_font_montserrat_18);
    lv_style_set_text_align(&regular_text,LV_ALIGN_CENTER);

    lv_style_init(&title_text);
    lv_style_set_text_font(&title_text,&lv_font_montserrat_28);
    lv_style_set_text_align(&title_text,LV_ALIGN_CENTER);

    lv_style_init(&colored_label_style);
    lv_style_set_text_align(&colored_label_style,LV_ALIGN_TOP_LEFT);
    lv_style_set_radius(&colored_label_style,5);
    lv_style_set_pad_all(&colored_label_style,10);
    lv_style_set_bg_opa(&colored_label_style,LV_OPA_COVER);

    //////////////////
    // Driver Screen//
    //////////////////
    driver_screen = lv_obj_create(NULL);    // Driver Screen Init
    lv_obj_set_style_bg_color(driver_screen, lv_color_hex(0x01121f), LV_PART_MAIN);
    lv_obj_set_style_text_color(driver_screen, lv_color_hex(0xffffff), LV_PART_MAIN);

    lv_style_init(&speedometer_style);    // Speedometer foreground style
    lv_style_set_arc_rounded(&speedometer_style,false);
    lv_style_set_arc_width(&speedometer_style,40);

    rpm_label = lv_label_create(driver_screen);     // RPM Label
    lv_obj_align(rpm_label, LV_ALIGN_TOP_LEFT, 76, 136+50);
    lv_obj_set_style_bg_color(rpm_label,lv_palette_main(LV_PALETTE_GREEN),LV_PART_MAIN);
    lv_obj_add_style(rpm_label,&colored_label_style,LV_PART_MAIN);

    throttle_label = lv_label_create(driver_screen);    // THROTTLE Label
    lv_obj_align(throttle_label, LV_ALIGN_TOP_LEFT, 50+240, 136+50);
    lv_obj_set_style_bg_color(throttle_label,lv_palette_main(LV_PALETTE_GREEN),LV_PART_MAIN);
    lv_obj_add_style(throttle_label,&colored_label_style,LV_PART_MAIN);

    rpm_arc = lv_arc_create(driver_screen);   // RPM Arc
    lv_arc_set_bg_angles(rpm_arc, 150, 30);
    lv_arc_set_angles(rpm_arc,150,-20);
    lv_obj_set_size(rpm_arc, 180, 180);

    lv_obj_add_style(rpm_arc,&speedometer_style,LV_PART_INDICATOR);
    lv_obj_add_style(rpm_arc,&speedometer_style,LV_PART_MAIN);
    lv_obj_set_style_arc_color(rpm_arc,lv_palette_main(LV_PALETTE_DEEP_ORANGE),LV_PART_INDICATOR);
    lv_obj_remove_style(rpm_arc, NULL, LV_PART_KNOB);   /*Be sure the knob is not displayed*/
    lv_obj_clear_flag(rpm_arc, LV_OBJ_FLAG_CLICKABLE);  /*To not allow adjusting by click*/
    lv_obj_align(rpm_arc, LV_ALIGN_LEFT_MID, 40, 0);

    throttle_arc = lv_arc_create(driver_screen);    // THROTTLE Arc
    lv_arc_set_bg_angles(throttle_arc, 150, 30);
    lv_arc_set_angles(throttle_arc,150,-20);
    lv_obj_set_size(throttle_arc, 180, 180);

    lv_obj_add_style(throttle_arc,&speedometer_style,LV_PART_INDICATOR);
    lv_obj_add_style(throttle_arc,&speedometer_style,LV_PART_MAIN);
    lv_obj_set_style_arc_color(throttle_arc,lv_palette_main(LV_PALETTE_DEEP_PURPLE),LV_PART_INDICATOR);
    lv_obj_remove_style(throttle_arc, NULL, LV_PART_KNOB);   // Be sure the knob is not displayed
    lv_obj_clear_flag(throttle_arc, LV_OBJ_FLAG_CLICKABLE);  // To not allow adjusting by click
    lv_obj_align(throttle_arc, LV_ALIGN_RIGHT_MID, -40, 0);


    //////////////////
    // Debug Screen //
    //////////////////

    debug_screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(debug_screen, lv_color_hex(0x003a57), LV_PART_MAIN);
    lv_obj_set_style_text_color(debug_screen, lv_color_hex(0xffffff), LV_PART_MAIN);

    // Labels:
    // These are separated so that later I can only update the text if required
    // Battery Voltage
    dbs_battery_voltage = lv_label_create(debug_screen);
    lv_label_set_text_fmt(dbs_battery_voltage, "GLV Voltage: %dV", the_vehicle.glv.voltage);
    lv_obj_set_style_text_align(dbs_battery_voltage, LV_ALIGN_TOP_LEFT, 0);
    lv_obj_align(dbs_battery_voltage, LV_ALIGN_TOP_LEFT, LEFT_COLUMN_TAB, 1 * LINE_HEIGHT);

    // Steering Position
    dbs_steering_position = lv_label_create(debug_screen);
    lv_label_set_text_fmt(dbs_steering_position, "Steering Position: %d deg", the_vehicle.driver.steeringAngle);
    lv_obj_set_style_text_align(dbs_steering_position, LV_ALIGN_TOP_LEFT, 0);
    lv_obj_align(dbs_steering_position, LV_ALIGN_TOP_LEFT, LEFT_COLUMN_TAB, 2 * LINE_HEIGHT);

    // Throttle Position
    dbs_throttle_position = lv_label_create(debug_screen);
    lv_label_set_text_fmt(dbs_throttle_position, "Throttle Position: %d %% ", the_vehicle.driver.throttle);
    lv_obj_set_style_text_align(dbs_throttle_position, LV_ALIGN_TOP_LEFT, 0);
    lv_obj_align(dbs_throttle_position, LV_ALIGN_TOP_LEFT, LEFT_COLUMN_TAB, 3 * LINE_HEIGHT);

    // Brake Pressure
    dbs_brake_pressure_front = lv_label_create(debug_screen);
    lv_label_set_text_fmt(dbs_brake_pressure_front, "Front Brake Pressure: %d kPa",
                          the_vehicle.driver.frontBrakePressure);
    lv_obj_set_style_text_align(dbs_brake_pressure_front, LV_ALIGN_TOP_LEFT, 0);
    lv_obj_align(dbs_brake_pressure_front, LV_ALIGN_TOP_LEFT, LEFT_COLUMN_TAB, 4 * LINE_HEIGHT);

    // Brake Pressure
    dbs_brake_pressure_rear = lv_label_create(debug_screen);
    lv_label_set_text_fmt(dbs_brake_pressure_rear, "Rear Brake Pressure: %d kPa", the_vehicle.driver.rearBrakePressure);
    lv_obj_set_style_text_align(dbs_brake_pressure_rear, LV_ALIGN_TOP_LEFT, 0);
    lv_obj_align(dbs_brake_pressure_rear, LV_ALIGN_TOP_LEFT, LEFT_COLUMN_TAB, 5 * LINE_HEIGHT);

    // Accumulator SOC
    dbs_acc_soc = lv_label_create(debug_screen);
    lv_label_set_text_fmt(dbs_acc_soc, "ACC SOC: %d %%", the_vehicle.ts.soc);
    lv_obj_set_style_text_align(dbs_acc_soc, LV_ALIGN_TOP_LEFT, 0);
    lv_obj_align(dbs_acc_soc, LV_ALIGN_TOP_LEFT, LEFT_COLUMN_TAB, 6 * LINE_HEIGHT);

    // Accumulator Lowest Cell Voltage
    dbs_acc_lowest_cell_voltage = lv_label_create(debug_screen);
    lv_label_set_text_fmt(dbs_acc_lowest_cell_voltage, "ACC V-low : %.2f V", the_vehicle.ts.minVoltage);
    lv_obj_set_style_text_align(dbs_acc_lowest_cell_voltage, LV_ALIGN_TOP_LEFT, 0);
    lv_obj_align(dbs_acc_lowest_cell_voltage, LV_ALIGN_TOP_LEFT, LEFT_COLUMN_TAB, 7 * LINE_HEIGHT);

    // Accumulator Highest Cell Voltage
    dbs_acc_highest_cell_voltage = lv_label_create(debug_screen);
    lv_label_set_text_fmt(dbs_acc_highest_cell_voltage, "ACC V-high: %.2f V", the_vehicle.ts.maxVoltage);
    lv_obj_set_style_text_align(dbs_acc_highest_cell_voltage, LV_ALIGN_TOP_LEFT, 0);
    lv_obj_align(dbs_acc_highest_cell_voltage, LV_ALIGN_TOP_LEFT, LEFT_COLUMN_TAB, 8 * LINE_HEIGHT);

    // Second Column
    // front left motor temps
    dbs_fl_motor_inverter_temp_err_code = lv_label_create(debug_screen);
    lv_label_set_text_fmt(dbs_fl_motor_inverter_temp_err_code, "FLT: MT=%d\tIT=%d\terr=%d",
                          the_vehicle.drive[0].motorTemp, the_vehicle.drive[0].inverterTemp,
                          the_vehicle.drive[0].errorCode);
    lv_obj_set_style_text_align(dbs_fl_motor_inverter_temp_err_code, LV_ALIGN_TOP_LEFT, 0);
    lv_obj_align(dbs_fl_motor_inverter_temp_err_code, LV_ALIGN_TOP_LEFT, 240, 1 * LINE_HEIGHT);

    // front right motor temps
    dbs_fr_motor_inverter_temp_err_code = lv_label_create(debug_screen);
    lv_label_set_text_fmt(dbs_fr_motor_inverter_temp_err_code, "FLT: MT=%d\tIT=%d\terr=%d",
                          the_vehicle.drive[1].motorTemp, the_vehicle.drive[1].inverterTemp,
                          the_vehicle.drive[1].errorCode);
    lv_obj_set_style_text_align(dbs_fr_motor_inverter_temp_err_code, LV_ALIGN_TOP_LEFT, 0);
    lv_obj_align(dbs_fr_motor_inverter_temp_err_code, LV_ALIGN_TOP_LEFT, 240, 2 * LINE_HEIGHT);

    dbs_rl_motor_inverter_temp_err_code = lv_label_create(debug_screen);
    lv_label_set_text_fmt(dbs_rl_motor_inverter_temp_err_code, "FLT: MT=%d\tIT=%d\terr=%d",
                          the_vehicle.drive[2].motorTemp, the_vehicle.drive[2].inverterTemp,
                          the_vehicle.drive[2].errorCode);
    lv_obj_set_style_text_align(dbs_rl_motor_inverter_temp_err_code, LV_ALIGN_TOP_LEFT, 0);
    lv_obj_align(dbs_rl_motor_inverter_temp_err_code, LV_ALIGN_TOP_LEFT, 240, 3 * LINE_HEIGHT);

    dbs_rr_motor_inverter_temp_err_code = lv_label_create(debug_screen);
    lv_label_set_text_fmt(dbs_rr_motor_inverter_temp_err_code, "FLT: MT=%d\tIT=%d\terr=%d",
                          the_vehicle.drive[3].motorTemp, the_vehicle.drive[3].inverterTemp,
                          the_vehicle.drive[3].errorCode);
    lv_obj_set_style_text_align(dbs_rr_motor_inverter_temp_err_code, LV_ALIGN_TOP_LEFT, 0);
    lv_obj_align(dbs_rr_motor_inverter_temp_err_code, LV_ALIGN_TOP_LEFT, 240, 4 * LINE_HEIGHT);

    dbs_motor_loop_coolant_temp = lv_label_create(debug_screen);
    lv_label_set_text_fmt(dbs_motor_loop_coolant_temp, "motor coolant temp: %d deg", -1);
    lv_obj_set_style_text_align(dbs_motor_loop_coolant_temp, LV_ALIGN_TOP_LEFT, 0);
    lv_obj_align(dbs_motor_loop_coolant_temp, LV_ALIGN_TOP_LEFT, 240, 5 * LINE_HEIGHT);

    dbs_inverter_loop_coolant_temp = lv_label_create(debug_screen);
    lv_label_set_text_fmt(dbs_inverter_loop_coolant_temp, "inverter coolant temp: %d deg", -1);
    lv_obj_set_style_text_align(dbs_inverter_loop_coolant_temp, LV_ALIGN_TOP_LEFT, 0);
    lv_obj_align(dbs_inverter_loop_coolant_temp, LV_ALIGN_TOP_LEFT, 240, 6 * LINE_HEIGHT);

    ///////////////////
    // Energy Screen //
    ///////////////////
    energy_screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(energy_screen, lv_color_hex(0x01121f), LV_PART_MAIN);   // Energy Screen Init
    lv_obj_set_style_text_color(energy_screen, lv_color_hex(0xffffff), LV_PART_MAIN);

    lv_style_init(&regen_style);
    lv_style_set_bg_opa(&regen_style, LV_OPA_COVER);
    lv_style_set_bg_color(&regen_style, lv_palette_main(LV_PALETTE_GREEN));
    lv_style_set_bg_grad_color(&regen_style, lv_palette_main(LV_PALETTE_BLUE));
    lv_style_set_bg_grad_dir(&regen_style, LV_GRAD_DIR_VER);
    lv_style_set_radius(&regen_style,6);

    regen_power = lv_bar_create(energy_screen);
    lv_obj_set_style_radius(regen_power,6,LV_PART_MAIN);
    lv_obj_add_style(regen_power, &regen_style, LV_PART_INDICATOR);
    lv_obj_set_size(regen_power, 40, 160);
    lv_obj_align(regen_power, LV_ALIGN_LEFT_MID, 30, 10);
    lv_bar_set_range(regen_power, 0, 100);
    lv_bar_set_value(regen_power, 80, LV_ANIM_OFF);

    regen_label = lv_label_create(energy_screen);    // Regen Label
    lv_obj_add_style(regen_label,&regular_text,LV_PART_MAIN);
    lv_label_set_text(regen_label,"Regen");
    lv_obj_align(regen_label, LV_ALIGN_BOTTOM_LEFT, 20, -10);

    lv_style_init(&accumulator_style);
    lv_style_set_bg_opa(&accumulator_style, LV_OPA_COVER);
    lv_style_set_bg_color(&accumulator_style, lv_palette_main(LV_PALETTE_GREEN));
    lv_style_set_radius(&accumulator_style, 3);

    accumulator_power = lv_bar_create(energy_screen);  // Accumulator Bar
    lv_obj_set_style_radius(accumulator_power,6,LV_PART_MAIN);
    lv_obj_add_style(accumulator_power, &accumulator_style, LV_PART_INDICATOR);
    lv_obj_set_size(accumulator_power, 80, 160);
    lv_obj_align(accumulator_power, LV_ALIGN_CENTER, -70, 10);
    lv_bar_set_range(accumulator_power, 0, 100);
    lv_bar_set_value(accumulator_power, 80, LV_ANIM_OFF);

    accumulator_text = lv_label_create(energy_screen);     // Accumulator Live Text
    lv_obj_align(accumulator_text, LV_ALIGN_CENTER, -70, 0);
    lv_obj_set_style_text_font(accumulator_text,&lv_font_montserrat_26,LV_PART_MAIN);
    lv_label_set_text_fmt(accumulator_text, "%d %%", 80);

    accumulator_label = lv_label_create(energy_screen);    // Accumulator Label
    lv_obj_add_style(accumulator_label,&regular_text,LV_PART_MAIN);
    lv_label_set_text(accumulator_label,"Accumulator");
    lv_obj_align(accumulator_label, LV_ALIGN_BOTTOM_MID, -70, -10);

    glv_power = lv_bar_create(energy_screen);    // GLV Bar
    lv_obj_set_style_radius(glv_power,6,LV_PART_MAIN);
    lv_obj_add_style(glv_power, &accumulator_style, LV_PART_INDICATOR);
    lv_obj_set_size(glv_power, 80, 140);
    lv_obj_align(glv_power, LV_ALIGN_CENTER, 70, 20);
    lv_bar_set_range(glv_power, 0, 100);
    lv_bar_set_value(glv_power, 60, LV_ANIM_OFF);

    glv_text = lv_label_create(energy_screen);     // GLV Live Text
    lv_obj_align(glv_text, LV_ALIGN_CENTER, 70, 20);
    lv_obj_set_style_text_font(glv_text,&lv_font_montserrat_26,LV_PART_MAIN);
    lv_label_set_text_fmt(glv_text, "%d %%", 60);

    glv_label = lv_label_create(energy_screen);    // GLV Label
    lv_obj_add_style(glv_label,&regular_text,LV_PART_MAIN);
    lv_label_set_text(glv_label,"GLV");
    lv_obj_align(glv_label, LV_ALIGN_BOTTOM_MID, 70, -10);

    power_mode = lv_bar_create(energy_screen);     // Power Mode Bar
    lv_obj_set_style_radius(power_mode,6,LV_PART_MAIN);
    lv_obj_add_style(power_mode, &accumulator_style, LV_PART_INDICATOR);
    lv_obj_set_size(power_mode, 30, 200);
    lv_obj_align(power_mode, LV_ALIGN_RIGHT_MID, -40, -10);
    lv_bar_set_range(power_mode, 0, 6);
    lv_bar_set_value(power_mode, 1, LV_ANIM_OFF);

    power_mode_label = lv_label_create(energy_screen);    // Power Mode Label
    lv_obj_add_style(power_mode_label,&regular_text,LV_PART_MAIN);
    lv_label_set_text(power_mode_label,"P-Mode");
    lv_obj_align(power_mode_label, LV_ALIGN_BOTTOM_RIGHT, -20, -10);

    //////////////////////
    //  Lap Time Screen //
    //////////////////////
    lap_screen = lv_obj_create(NULL);    // Lap screen init
    lv_obj_set_style_bg_color(lap_screen, lv_color_hex(0x01121f), LV_PART_MAIN);
    lv_obj_set_style_text_color(lap_screen, lv_color_hex(0xffffff), LV_PART_MAIN);

    lv_style_init(&text_box_style);

    lv_style_set_radius(&text_box_style, 0);
    lv_style_set_outline_width(&text_box_style, 5);
    lv_style_set_outline_color(&text_box_style, lv_palette_main(LV_PALETTE_BLUE_GREY));
    lv_style_set_outline_pad(&text_box_style, 10);

    current_lap_time = lv_label_create(lap_screen);    // Current lap time
    lv_obj_add_style(current_lap_time,&text_box_style,LV_PART_MAIN);
    lv_obj_set_style_text_font(current_lap_time,&bitstream_vera_sans_30,LV_PART_MAIN);

    uint32_t minutes = the_vehicle.race.currentLapTime/60000;
    uint32_t seconds = the_vehicle.race.currentLapTime/1000-60*minutes;
    lv_label_set_text_fmt(current_lap_time,"%02d : %02d : %03d",minutes,seconds,the_vehicle.race.currentLapTime%1000);

    lv_obj_align(current_lap_time, LV_ALIGN_TOP_LEFT,40, 80);

    lap_delta = lv_label_create(lap_screen);    // Lap Delta
    lv_obj_add_style(lap_delta,&text_box_style,LV_PART_MAIN);
    lv_obj_set_style_text_font(lap_delta,&bitstream_vera_sans_30,LV_PART_MAIN);
    lv_label_set_text_fmt(lap_delta,"%.2f",the_vehicle.race.deltaLapTime/1000);
    lv_obj_align(lap_delta, LV_ALIGN_TOP_RIGHT,-40, 80);

    best_lap_time = lv_label_create(lap_screen);    // Best Lap Time
    lv_obj_add_style(best_lap_time,&text_box_style,LV_PART_MAIN);
    lv_obj_set_style_text_font(best_lap_time,&bitstream_vera_sans_26,LV_PART_MAIN);

    minutes = the_vehicle.race.bestLapTime/60000;
    seconds = the_vehicle.race.bestLapTime/1000-60*minutes;
    lv_label_set_text_fmt(best_lap_time,"%02d : %02d : %03d",minutes,seconds,the_vehicle.race.bestLapTime%1000);

    lv_obj_align(best_lap_time, LV_ALIGN_TOP_LEFT,40, 140);

    previous_lap_time = lv_label_create(lap_screen);    // Previous Lap Time
    lv_obj_add_style(previous_lap_time, &text_box_style, LV_PART_MAIN);
    lv_obj_set_style_text_font(previous_lap_time, &bitstream_vera_sans_26, LV_PART_MAIN);
    lv_label_set_text(previous_lap_time, "01 : 08 : 473");  // TODO: Put previous lap time here

    lv_obj_align(previous_lap_time, LV_ALIGN_TOP_LEFT, 40, 200);

    lap_number = lv_label_create(lap_screen);    // Lap Number
    lv_obj_add_style(lap_number,&text_box_style,LV_PART_MAIN);
    lv_obj_set_style_text_font(lap_number,&bitstream_vera_sans_80,LV_PART_MAIN);
    lv_label_set_text_fmt(lap_number,"%02d ",the_vehicle.race.lapNumber);
    lv_obj_align(lap_number, LV_ALIGN_TOP_RIGHT,-40, 158);

    lap_label = lv_label_create(lap_screen);    // Laps Text
    lv_obj_set_style_text_font(lap_label,&lv_font_montserrat_18,LV_PART_MAIN);
    lv_label_set_text(lap_label,"laps");
    lv_obj_align(lap_label, LV_ALIGN_TOP_RIGHT,-60, 198);

    lv_scr_load(driver_screen);
    current_screen = DRIVER_SCREEN;

}

void try_update_screen() {
    if (!screen_update_flag) return;
    screen_update_flag = 0;
    if (lv_scr_act() == driver_screen) {
        // Update Driver Screen
        lv_label_set_text_fmt(throttle_label, "Throttle: %d %%", the_vehicle.driver.throttle);
        lv_label_set_text_fmt(rpm_label, "RPM: %d", the_vehicle.driver.rpm);
        lv_arc_set_angles(rpm_arc, 150, 150+(240*(int)the_vehicle.driver.rpm/20000));
        lv_arc_set_angles(throttle_arc, 150, 150+(240*(int)the_vehicle.driver.throttle)/100);

    } else if (lv_scr_act() == debug_screen) {
        // Update Debug Screen
        lv_label_set_text_fmt(dbs_battery_voltage, "GLV Voltage: %.2fV", the_vehicle.glv.voltage);
        lv_label_set_text_fmt(dbs_steering_position, "Steering Position: %d deg", the_vehicle.driver.steeringAngle);
        lv_label_set_text_fmt(dbs_throttle_position, "Throttle Position: %d %% ", the_vehicle.driver.throttle);
        lv_label_set_text_fmt(dbs_brake_pressure_front, "Front Brake Pressure: %d kPa",
                              the_vehicle.driver.frontBrakePressure);
        lv_label_set_text_fmt(dbs_brake_pressure_rear, "Rear Brake Pressure: %d kPa", the_vehicle.driver.rearBrakePressure);
        lv_label_set_text_fmt(dbs_acc_soc, "ACC SOC: %d %%", the_vehicle.ts.soc);
        lv_label_set_text_fmt(dbs_acc_lowest_cell_voltage, "ACC V-low : %.2f V", the_vehicle.ts.minVoltage);
        lv_label_set_text_fmt(dbs_acc_highest_cell_voltage, "ACC V-high: %.2f V", the_vehicle.ts.maxVoltage);
        lv_label_set_text_fmt(dbs_fl_motor_inverter_temp_err_code, "FLT: MT=%d\tIT=%d\terr=%d",
                              the_vehicle.drive[0].motorTemp, the_vehicle.drive[0].inverterTemp,
                              the_vehicle.drive[0].errorCode);
        lv_label_set_text_fmt(dbs_fr_motor_inverter_temp_err_code, "FLT: MT=%d\tIT=%d\terr=%d",
                              the_vehicle.drive[1].motorTemp, the_vehicle.drive[1].inverterTemp,
                              the_vehicle.drive[1].errorCode);
        lv_label_set_text_fmt(dbs_rl_motor_inverter_temp_err_code, "FLT: MT=%d\tIT=%d\terr=%d",
                              the_vehicle.drive[2].motorTemp, the_vehicle.drive[2].inverterTemp,
                              the_vehicle.drive[2].errorCode);
        lv_label_set_text_fmt(dbs_rr_motor_inverter_temp_err_code, "FLT: MT=%d\tIT=%d\terr=%d",
                              the_vehicle.drive[3].motorTemp, the_vehicle.drive[3].inverterTemp,
                              the_vehicle.drive[3].errorCode);
    } else if (lv_scr_act() == energy_screen) {
        // TODO: Make bars red if below 20%
        lv_bar_set_value(regen_power, 0, LV_ANIM_OFF);  // TODO: CAN MESSAGE NEEDED
        lv_bar_set_value(accumulator_power, the_vehicle.ts.soc, LV_ANIM_OFF);
        lv_label_set_text_fmt(accumulator_text, "%d %%", the_vehicle.ts.soc);
        lv_bar_set_value(glv_power, the_vehicle.glv.soc, LV_ANIM_OFF);
        lv_label_set_text_fmt(glv_text, "%d %%", the_vehicle.glv.soc);
        lv_bar_set_value(power_mode, 1, LV_ANIM_OFF);   // TODO: CAN MESSAGE NEEDED
    } else if (lv_scr_act() == lap_screen) {
        uint32_t minutes = the_vehicle.race.currentLapTime/60000;
        uint32_t seconds = the_vehicle.race.currentLapTime/1000-60*minutes;
        lv_label_set_text_fmt(current_lap_time,"%02d : %02d : %03d",minutes,seconds,the_vehicle.race.currentLapTime%1000);

        lv_label_set_text_fmt(lap_delta,"%.2f",the_vehicle.race.deltaLapTime/1000);

        minutes = the_vehicle.race.bestLapTime/60000;
        seconds = the_vehicle.race.bestLapTime/1000-60*minutes;
        lv_label_set_text_fmt(best_lap_time,"%02d : %02d : %03d",minutes,seconds,the_vehicle.race.bestLapTime%1000);

        lv_label_set_text_fmt(lap_number,"%02d ",the_vehicle.race.lapNumber);
    }
}

void update_screen() {
    screen_update_flag = 1;
}

void change_screens(enum SCREENS screen) {
    current_screen = screen;
    switch (screen) {
        case DRIVER_SCREEN:
            lv_scr_load(driver_screen);
            break;
        case DEBUG_SCREEN:
            lv_scr_load(debug_screen);
            break;
        case ENERGY_SCREEN:
            lv_scr_load(energy_screen);
            break;
        case LAP_SCREEN:
            lv_scr_load(lap_screen);
            break;
    }
}

void cycle_screens() {
    screen_switch_flag = 1;
}

void try_cycle_screens() {
    if (screen_switch_flag){
        screen_switch_flag = 0;
        current_screen = (current_screen + 1) % 4;
        change_screens(current_screen);
    }
}

#ifndef USE_SIMULATOR

// Private local flush buffer function
#if USE_DOUBLE_BUFFER
void flush_callback(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p)
{
//	(hltdc.LayerCfg[0]).FBStartAdress = (uint32_t)color_p;
    HAL_LTDC_SetAddress(&hltdc, (uint32_t)color_p, LTDC_LAYER_1);
    /* IMPORTANT!!!
    * Inform the graphics library that you are ready with the flushing*/
    lv_disp_flush_ready(disp_drv);
}
#else
void flush_callback(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p)
{
    volatile uint32_t *ram_address = (uint32_t *)0xC0000000;
    int width = area->x2 - area->x1+1;
    for (int i = 0;i<=area->y2-area->y1;i++){
        memcpy(ram_address+(area->y1+i)*480+area->x1,color_p+width*i,4*width);
    }
    /* IMPORTANT!!!
    * Inform the graphics library that you are ready with the flushing*/
    lv_disp_flush_ready(disp_drv);
}
#endif

void init_displays() {
    lv_init();

    /*Initialize `disp_buf` with the buffer(s) */

#if USE_DOUBLE_BUFFER
    lv_disp_draw_buf_init(&the_display_buf, buf_1, buf_2, 480*272);

    lv_disp_drv_init(&the_display_drv);            /*Basic initialization*/
    the_display_drv.draw_buf = &the_display_buf;            /*Set an initialized buffer*/
    the_display_drv.direct_mode = 1;
    the_display_drv.full_refresh = 1;
    the_display_drv.sw_rotate = 0;
    the_display_drv.hor_res = 480;
    the_display_drv.ver_res = 272;
    the_display_drv.rotated = LV_DISP_ROT_180;
    the_display_drv.flush_cb = flush_callback;
    disp = lv_disp_drv_register(&the_display_drv);
#else
    lv_disp_draw_buf_init(&the_display_buf, buf_1, buf_2, 100*100);

    lv_disp_drv_init(&the_display_drv);            /*Basic initialization*/
    the_display_drv.draw_buf = &the_display_buf;            /*Set an initialized buffer*/
    the_display_drv.direct_mode = 0;
    the_display_drv.full_refresh = 0;
    the_display_drv.sw_rotate = 1;
    the_display_drv.hor_res = 480;
    the_display_drv.ver_res = 272;
    the_display_drv.rotated = LV_DISP_ROT_180;
    the_display_drv.flush_cb = flush_callback;
    disp = lv_disp_drv_register(&the_display_drv);
#endif
}

#endif

