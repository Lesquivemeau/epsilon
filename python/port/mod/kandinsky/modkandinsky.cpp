extern "C" {
#include "modkandinsky.h"
#include <py/objtuple.h>
#include <py/runtime.h>
}
#include <kandinsky.h>
#include <ion.h>
#include "port.h"

static KDColor ColorForTuple(mp_obj_t tuple) {
    size_t len;
    mp_obj_t * elem;

    mp_obj_get_array(tuple, &len, &elem);
    if (len != 3) {
      mp_raise_TypeError("color needs 3 components");
    }

    return KDColor::RGB888(
      mp_obj_get_int(elem[0]),
      mp_obj_get_int(elem[1]),
      mp_obj_get_int(elem[2])
    );
}

static mp_obj_t TupleForRGB(uint8_t r, uint8_t g, uint8_t b) {
  mp_obj_tuple_t * t = static_cast<mp_obj_tuple_t *>(MP_OBJ_TO_PTR(mp_obj_new_tuple(3, NULL)));
  t->items[0] = MP_OBJ_NEW_SMALL_INT(r);
  t->items[1] = MP_OBJ_NEW_SMALL_INT(g);
  t->items[2] = MP_OBJ_NEW_SMALL_INT(b);
  return MP_OBJ_FROM_PTR(t);
}

/* KDIonContext::sharedContext needs to be set to the wanted Rect before
 * calling kandinsky_get_pixel, kandinsky_set_pixel and kandinsky_draw_string.
 * We do this here with displaySandbox(), which pushes the SandboxController on
 * the stackViewController and forces the window to redraw itself.
 * KDIonContext::sharedContext is set to the frame of the last object drawn. */

mp_obj_t modkandinsky_color(mp_obj_t red, mp_obj_t green, mp_obj_t blue) {
  return TupleForRGB(
    mp_obj_get_int(red),
    mp_obj_get_int(green),
    mp_obj_get_int(blue)
  );
}

/* Calling ExecutionEnvironment::displaySandbox() hides the console and switches
 * to another mode. So it's a good idea to retrieve and handle input parameters
 * before calling displaySandbox, otherwise error messages (such as TypeError)
 * won't be visible until the user comes back to the console screen. */

mp_obj_t modkandinsky_get_pixel(mp_obj_t x, mp_obj_t y) {
  KDPoint point(mp_obj_get_int(x), mp_obj_get_int(y));
  KDColor c = KDIonContext::sharedContext()->getPixel(point);
  return TupleForRGB(c.red(), c.green(), c.blue());
}

mp_obj_t modkandinsky_set_pixel(mp_obj_t x, mp_obj_t y, mp_obj_t color) {
  KDPoint point(mp_obj_get_int(x), mp_obj_get_int(y));
  KDColor kdColor = ColorForTuple(color);
  MicroPython::ExecutionEnvironment::currentExecutionEnvironment()->displaySandbox();
  KDIonContext::sharedContext()->setPixel(point, kdColor);
  return mp_const_none;
}

mp_obj_t modkandinsky_draw_string(size_t n_args, const mp_obj_t * args) {
  const char * text = mp_obj_str_get_str(args[0]);
  KDPoint point(mp_obj_get_int(args[1]), mp_obj_get_int(args[2]));
  KDColor textColor = (n_args >= 4) ? ColorForTuple(args[3]) : KDColorBlack;
  KDColor backgroundColor = (n_args >= 5) ? ColorForTuple(args[4]) : KDColorWhite;
  MicroPython::ExecutionEnvironment::currentExecutionEnvironment()->displaySandbox();
  KDIonContext::sharedContext()->drawString(text, point, KDFont::LargeFont, textColor, backgroundColor);
  return mp_const_none;
}

mp_obj_t modkandinsky_fill_rect(size_t n_args, const mp_obj_t * args) {
  KDRect rect(
    mp_obj_get_int(args[0]),
    mp_obj_get_int(args[1]),
    mp_obj_get_int(args[2]),
    mp_obj_get_int(args[3])
  );
  KDColor color = ColorForTuple(args[4]);
  MicroPython::ExecutionEnvironment::currentExecutionEnvironment()->displaySandbox();
  KDIonContext::sharedContext()->fillRect(rect, color);
  return mp_const_none;
}

mp_obj_t modkandinsky_wait_vblank() {
  micropython_port_interrupt_if_needed();
  Ion::Display::waitForVBlank();
  return mp_const_none;
}

struct key2mp
{
    Ion::Keyboard::Key key;
    mp_obj_t string;
};

key2mp keyMapping[] =
{
    { Ion::Keyboard::Key::A1, MP_ROM_QSTR(MP_QSTR_left) },
    { Ion::Keyboard::Key::A2, MP_ROM_QSTR(MP_QSTR_up) },
    { Ion::Keyboard::Key::A3, MP_ROM_QSTR(MP_QSTR_down) },
    { Ion::Keyboard::Key::A4, MP_ROM_QSTR(MP_QSTR_right) },
    { Ion::Keyboard::Key::A5, MP_ROM_QSTR(MP_QSTR_OK) },
    { Ion::Keyboard::Key::A6, MP_ROM_QSTR(MP_QSTR_back) },

    { Ion::Keyboard::Key::C1, MP_ROM_QSTR(MP_QSTR_shift) },
    { Ion::Keyboard::Key::C2, MP_ROM_QSTR(MP_QSTR_alpha) },
    { Ion::Keyboard::Key::C3, MP_ROM_QSTR(MP_QSTR_xnt) },
    { Ion::Keyboard::Key::C4, MP_ROM_QSTR(MP_QSTR_var) },
    { Ion::Keyboard::Key::C5, MP_ROM_QSTR(MP_QSTR_toolbox) },
    { Ion::Keyboard::Key::C6, MP_ROM_QSTR(MP_QSTR_backspace) },

    { Ion::Keyboard::Key::D1, MP_ROM_QSTR(MP_QSTR_exp) },
    { Ion::Keyboard::Key::D2, MP_ROM_QSTR(MP_QSTR_ln) },
    { Ion::Keyboard::Key::D3, MP_ROM_QSTR(MP_QSTR_log) },
    { Ion::Keyboard::Key::D4, MP_ROM_QSTR(MP_QSTR_imaginary) },
    { Ion::Keyboard::Key::D5, MP_ROM_QSTR(MP_QSTR_toolbox) },
    { Ion::Keyboard::Key::D6, MP_ROM_QSTR(MP_QSTR_backspace) },

    { Ion::Keyboard::Key::E1, MP_ROM_QSTR(MP_QSTR_sin) },
    { Ion::Keyboard::Key::E2, MP_ROM_QSTR(MP_QSTR_cos) },
    { Ion::Keyboard::Key::E3, MP_ROM_QSTR(MP_QSTR_tan) },
    { Ion::Keyboard::Key::E4, MP_ROM_QSTR(MP_QSTR_pi) },
    { Ion::Keyboard::Key::E5, MP_ROM_QSTR(MP_QSTR_sqrt) },
    { Ion::Keyboard::Key::E6, MP_ROM_QSTR(MP_QSTR_square) },

    { Ion::Keyboard::Key::F1, MP_ROM_QSTR(MP_QSTR_7) },
    { Ion::Keyboard::Key::F2, MP_ROM_QSTR(MP_QSTR_8) },
    { Ion::Keyboard::Key::F3, MP_ROM_QSTR(MP_QSTR_9) },
    { Ion::Keyboard::Key::F4, MP_ROM_QSTR(MP_QSTR__paren_open_) },
    { Ion::Keyboard::Key::F5, MP_ROM_QSTR(MP_QSTR__paren_close_) },

    { Ion::Keyboard::Key::G1, MP_ROM_QSTR(MP_QSTR_4) },
    { Ion::Keyboard::Key::G2, MP_ROM_QSTR(MP_QSTR_5) },
    { Ion::Keyboard::Key::G3, MP_ROM_QSTR(MP_QSTR_6) },
    { Ion::Keyboard::Key::G4, MP_ROM_QSTR(MP_QSTR__star_) },
    { Ion::Keyboard::Key::G5, MP_ROM_QSTR(MP_QSTR__slash_) },

    { Ion::Keyboard::Key::H1, MP_ROM_QSTR(MP_QSTR_1) },
    { Ion::Keyboard::Key::H2, MP_ROM_QSTR(MP_QSTR_2) },
    { Ion::Keyboard::Key::H3, MP_ROM_QSTR(MP_QSTR_3) },
    { Ion::Keyboard::Key::H4, MP_ROM_QSTR(MP_QSTR__plus_) },
    { Ion::Keyboard::Key::H5, MP_ROM_QSTR(MP_QSTR__hyphen_) },

    { Ion::Keyboard::Key::I1, MP_ROM_QSTR(MP_QSTR_0) },
    { Ion::Keyboard::Key::I2, MP_ROM_QSTR(MP_QSTR__dot_) },
    { Ion::Keyboard::Key::I3, MP_ROM_QSTR(MP_QSTR_EE) },
    { Ion::Keyboard::Key::I4, MP_ROM_QSTR(MP_QSTR_Ans) },
    { Ion::Keyboard::Key::I5, MP_ROM_QSTR(MP_QSTR_EXE) },
};

mp_obj_t modkandinsky_get_keys() {
  micropython_port_interrupt_if_needed();

  Ion::Keyboard::State keys = Ion::Keyboard::scan();
  mp_obj_t result = mp_obj_new_set(0, nullptr);

  for (unsigned i = 0; i < sizeof(keyMapping)/sizeof(key2mp); i++) {
      if (keys.keyDown(keyMapping[i].key)) {
          mp_obj_set_store(result, keyMapping[i].string);
      }
  }

  return result;
}
