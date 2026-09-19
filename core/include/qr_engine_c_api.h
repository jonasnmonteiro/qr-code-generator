#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

typedef struct {
    const char *text;
    int error_correction_level;
    double canvas_size;
    double margin;
    const char *background_color;
    const char *module_color;
    int module_style;
    double module_scale;

    int logo_shape;
    double logo_scale;
    double logo_padding_ratio;
    const char *logo_data_uri;

    int gradient_type;
    double gradient_rotation;
    const char *gradient_color_start;
    const char *gradient_color_end;

    double eye_radius_outer;
    double eye_radius_inner;
    const char *eye_color_outer;
    const char *eye_color_inner;
} C_QrConfig;

typedef struct {
    uint8_t *data;
    size_t size;
    int error_code;
    const char *error_message;
} C_BinaryResult;

typedef struct {
    char *svg_string;
    int error_code;
    const char *error_message;
} C_StringResult;

C_StringResult qr_generate_svg_c(const C_QrConfig *config);
C_BinaryResult qr_generate_png_c(const C_QrConfig *config);

void qr_free_string_c(char *str);
void qr_free_binary_c(uint8_t *buffer);

#ifdef __cplusplus
}
#endif
