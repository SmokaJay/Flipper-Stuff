#include <furi.h>
#include <gui/gui.h>
#include <input/input.h>
#include <gpio.h>
#include <irda.h>

#define PWM_FREQUENCY_HZ 14
#define DUTY_CYCLE_PERCENT 50
#define POWER_PIN &gpio_ext, GpioPin5

static bool emit_signal = false;

static void ir_emit_pwm(bool emit) {
    emit_signal = emit;
    if (emit) {
        uint32_t period_us = 1000000 / PWM_FREQUENCY_HZ;
        uint32_t on_time_us = period_us * DUTY_CYCLE_PERCENT / 100;
        uint32_t off_time_us = period_us - on_time_us;

        while (emit_signal) {
            gpio_set_pin_high(POWER_PIN);
            gpio_set_pin_high(&gpio_ir, GPIO_PIN);
            furi_delay_us(on_time_us);
            gpio_set_pin_low(&gpio_ir, GPIO_PIN);
            furi_delay_us(off_time_us);
        }
        gpio_set_pin_low(POWER_PIN);
    }
}

static void input_callback(InputEvent* input_event, void* ctx) {
    if (input_event->type == InputTypePress && input_event->key == InputKeyOk) {
        ir_emit_pwm(true);
    } else if (input_event->type == InputTypeRelease && input_event->key == InputKeyOk) {
        ir_emit_pwm(false);
    }
}

static void render_callback(Canvas* canvas, void* ctx) {
    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 64, 32, AlignCenter, AlignCenter, "Press Go to Emit 14Hz");
}

int32_t main(void) {
    FuriMessageQueue* input_queue = furi_message_queue_alloc(8, sizeof(InputEvent));
    ViewPort* viewport = view_port_alloc();
    view_port_draw_callback_set(viewport, render_callback, NULL);
    view_port_input_callback_set(viewport, input_callback, input_queue);

    Gui* gui = furi_record_open("gui");
    gui_add_view_port(gui, viewport, GuiLayerFullscreen);

    gpio_init(POWER_PIN, GpioModeOutputPP, GpioPullNo, GpioSpeedHigh);

    InputEvent input_event;
    while (1) {
        furi_check(furi_message_queue_get(input_queue, &input_event, FuriWaitForever) == FuriStatusOk);
        input_callback(&input_event, NULL);
    }

    furi_message_queue_free(input_queue);
    view_port_free(viewport);
    furi_record_close("gui");

    return 0;
}