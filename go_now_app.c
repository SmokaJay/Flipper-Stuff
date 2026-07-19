#include <furi.h>
#include <furi_hal_pwm.h>
#include <gui/gui.h>
#include <input/input.h>

#define PWM_FREQUENCY_HZ 14
#define DUTY_CYCLE_PERCENT 50
#define POWER_PWM_CHANNEL FuriHalPwmOutputIdTim1PA7

static void input_callback(InputEvent* input_event, void* ctx) {
    UNUSED(ctx);
    if(input_event->type == InputTypePress && input_event->key == InputKeyOk) {
        furi_hal_pwm_start(POWER_PWM_CHANNEL, PWM_FREQUENCY_HZ, DUTY_CYCLE_PERCENT);
    } else if(input_event->type == InputTypeRelease && input_event->key == InputKeyOk) {
        furi_hal_pwm_stop(POWER_PWM_CHANNEL);
    }
}

static void render_callback(Canvas* canvas, void* ctx) {
    UNUSED(ctx);
    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 64, 32, AlignCenter, AlignCenter, "Press Go to Emit 14Hz");
}

int32_t main(void) {
    FuriMessageQueue* input_queue = furi_message_queue_alloc(8, sizeof(InputEvent));
    ViewPort* viewport = view_port_alloc();
    view_port_draw_callback_set(viewport, render_callback, NULL);
    view_port_input_callback_set(viewport, input_callback, NULL);

    Gui* gui = furi_record_open("gui");
    gui_add_view_port(gui, viewport, GuiLayerFullscreen);

    InputEvent input_event;
    while(1) {
        furi_check(furi_message_queue_get(input_queue, &input_event, FuriWaitForever) == FuriStatusOk);
    }

    furi_hal_pwm_stop(POWER_PWM_CHANNEL);
    furi_message_queue_free(input_queue);
    view_port_free(viewport);
    furi_record_close("gui");

    return 0;
}
