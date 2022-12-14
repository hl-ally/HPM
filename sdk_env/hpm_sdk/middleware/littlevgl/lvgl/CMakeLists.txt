# Copyright (c) 2021 HPMicro
# SPDX-License-Identifier: BSD-3-Clause

sdk_compile_definitions("-DLV_CONF_INCLUDE_SIMPLE=1")

sdk_inc(.)
sdk_inc(src)
sdk_inc(src/misc)
sdk_inc(src/gpu)
sdk_inc(src/core)
sdk_inc(src/hal)
sdk_inc(src/extra)
sdk_inc(src/extra/layouts)
sdk_inc(src/extra/layouts/flex)
sdk_inc(src/extra/layouts/grid)
sdk_inc(src/extra/themes)
sdk_inc(src/extra/themes/basic)
sdk_inc(src/extra/themes/default)
sdk_inc(src/extra/themes/mono)
sdk_inc(src/extra/widgets)
sdk_inc(src/extra/widgets/calendar)
sdk_inc(src/extra/widgets/led)
sdk_inc(src/extra/widgets/win)
sdk_inc(src/extra/widgets/chart)
sdk_inc(src/extra/widgets/spinbox)
sdk_inc(src/extra/widgets/imgbtn)
sdk_inc(src/extra/widgets/spinner)
sdk_inc(src/extra/widgets/tabview)
sdk_inc(src/extra/widgets/msgbox)
sdk_inc(src/extra/widgets/span)
sdk_inc(src/extra/widgets/colorwheel)
sdk_inc(src/extra/widgets/list)
sdk_inc(src/extra/widgets/animimg)
sdk_inc(src/extra/widgets/meter)
sdk_inc(src/extra/widgets/tileview)
sdk_inc(src/extra/widgets/keyboard)
sdk_inc(src/font)
sdk_inc(src/draw)
sdk_inc(src/draw/sw)
sdk_inc(src/draw/sdl)
sdk_inc(src/draw/hpm67xx_pdma)
sdk_inc(src/widgets)

sdk_src(src/misc/lv_utils.c)
sdk_src(src/misc/lv_txt.c)
sdk_src(src/misc/lv_style.c)
sdk_src(src/misc/lv_style_gen.c)
sdk_src(src/misc/lv_mem.c)
sdk_src(src/misc/lv_fs.c)
sdk_src(src/misc/lv_txt_ap.c)
sdk_src(src/misc/lv_color.c)
sdk_src(src/misc/lv_ll.c)
sdk_src(src/misc/lv_log.c)
sdk_src(src/misc/lv_area.c)
sdk_src(src/misc/lv_timer.c)
sdk_src(src/misc/lv_async.c)
sdk_src(src/misc/lv_templ.c)
sdk_src(src/misc/lv_printf.c)
sdk_src(src/misc/lv_tlsf.c)
sdk_src(src/misc/lv_math.c)
sdk_src(src/misc/lv_anim.c)
sdk_src(src/misc/lv_gc.c)
sdk_src(src/misc/lv_bidi.c)
sdk_src(src/core/lv_indev.c)
sdk_src(src/core/lv_theme.c)
sdk_src(src/core/lv_disp.c)
sdk_src(src/core/lv_obj_class.c)
sdk_src(src/core/lv_refr.c)
sdk_src(src/core/lv_event.c)
sdk_src(src/core/lv_obj_draw.c)
sdk_src(src/core/lv_obj.c)
sdk_src(src/core/lv_obj_style_gen.c)
sdk_src(src/core/lv_obj_style.c)
sdk_src(src/core/lv_group.c)
sdk_src(src/core/lv_indev_scroll.c)
sdk_src(src/core/lv_obj_pos.c)
sdk_src(src/core/lv_obj_tree.c)
sdk_src(src/core/lv_obj_scroll.c)
sdk_src(src/hal/lv_hal_tick.c)
sdk_src(src/hal/lv_hal_indev.c)
sdk_src(src/hal/lv_hal_disp.c)
sdk_src(src/extra/layouts/flex/lv_flex.c)
sdk_src(src/extra/layouts/grid/lv_grid.c)

sdk_src(src/extra/libs/bmp/lv_bmp.c)
sdk_src(src/extra/libs/ffmpeg/lv_ffmpeg.c)
sdk_src(src/extra/libs/freetype/lv_freetype.c)
sdk_src(src/extra/libs/fsdrv/lv_fs_fatfs.c)
sdk_src(src/extra/libs/fsdrv/lv_fs_posix.c)
sdk_src(src/extra/libs/fsdrv/lv_fs_stdio.c)
sdk_src(src/extra/libs/fsdrv/lv_fs_win32.c)
sdk_src(src/extra/libs/gif/gifdec.c)
sdk_src(src/extra/libs/gif/lv_gif.c)
sdk_src(src/extra/libs/png/lodepng.c)
sdk_src(src/extra/libs/png/lv_png.c)
sdk_src(src/extra/libs/qrcode/lv_qrcode.c)
sdk_src(src/extra/libs/qrcode/qrcodegen.c)
sdk_src(src/extra/libs/rlottie/lv_rlottie.c)
sdk_src(src/extra/libs/sjpg/lv_sjpg.c)
sdk_src(src/extra/libs/sjpg/tjpgd.c)

sdk_src(src/extra/others/fragment/lv_fragment.c)
sdk_src(src/extra/others/fragment/lv_fragment_manager.c)
sdk_src(src/extra/others/gridnav/lv_gridnav.c)
sdk_src(src/extra/others/ime/lv_ime_pinyin.c)
sdk_src(src/extra/others/imgfont/lv_imgfont.c)
sdk_src(src/extra/others/monkey/lv_monkey.c)
sdk_src(src/extra/others/msg/lv_msg.c)
sdk_src(src/extra/others/snapshot/lv_snapshot.c)

sdk_src(src/extra/lv_extra.c)
sdk_src(src/extra/themes/basic/lv_theme_basic.c)
sdk_src(src/extra/themes/default/lv_theme_default.c)
sdk_src(src/extra/themes/mono/lv_theme_mono.c)
sdk_src(src/extra/widgets/calendar/lv_calendar_header_dropdown.c)
sdk_src(src/extra/widgets/calendar/lv_calendar_header_arrow.c)
sdk_src(src/extra/widgets/calendar/lv_calendar.c)
sdk_src(src/extra/widgets/led/lv_led.c)
sdk_src(src/extra/widgets/win/lv_win.c)
sdk_src(src/extra/widgets/chart/lv_chart.c)
sdk_src(src/extra/widgets/spinbox/lv_spinbox.c)
sdk_src(src/extra/widgets/imgbtn/lv_imgbtn.c)
sdk_src(src/extra/widgets/spinner/lv_spinner.c)
sdk_src(src/extra/widgets/tabview/lv_tabview.c)
sdk_src(src/extra/widgets/msgbox/lv_msgbox.c)
sdk_src(src/extra/widgets/span/lv_span.c)
sdk_src(src/extra/widgets/colorwheel/lv_colorwheel.c)
sdk_src(src/extra/widgets/list/lv_list.c)
sdk_src(src/extra/widgets/animimg/lv_animimg.c)
sdk_src(src/extra/widgets/meter/lv_meter.c)
sdk_src(src/extra/widgets/tileview/lv_tileview.c)
sdk_src(src/extra/widgets/keyboard/lv_keyboard.c)
sdk_src(src/extra/widgets/menu/lv_menu.c)
sdk_src(src/font/lv_font_montserrat_28_compressed.c)
sdk_src(src/font/lv_font_montserrat_10.c)
sdk_src(src/font/lv_font_montserrat_14.c)
sdk_src(src/font/lv_font_unscii_8.c)
sdk_src(src/font/lv_font_montserrat_40.c)
sdk_src(src/font/lv_font_montserrat_44.c)
sdk_src(src/font/lv_font_loader.c)
sdk_src(src/font/lv_font_dejavu_16_persian_hebrew.c)
sdk_src(src/font/lv_font_montserrat_36.c)
sdk_src(src/font/lv_font_montserrat_38.c)
sdk_src(src/font/lv_font_montserrat_32.c)
sdk_src(src/font/lv_font_montserrat_28.c)
sdk_src(src/font/lv_font_montserrat_22.c)
sdk_src(src/font/lv_font_montserrat_26.c)
sdk_src(src/font/lv_font_montserrat_18.c)
sdk_src(src/font/lv_font_montserrat_12.c)
sdk_src(src/font/lv_font_fmt_txt.c)
sdk_src(src/font/lv_font_montserrat_16.c)
sdk_src(src/font/lv_font_montserrat_48.c)
sdk_src(src/font/lv_font_montserrat_42.c)
sdk_src(src/font/lv_font_montserrat_46.c)
sdk_src(src/font/lv_font.c)
sdk_src(src/font/lv_font_unscii_16.c)
sdk_src(src/font/lv_font_montserrat_34.c)
sdk_src(src/font/lv_font_montserrat_8.c)
sdk_src(src/font/lv_font_montserrat_30.c)
sdk_src(src/font/lv_font_montserrat_20.c)
sdk_src(src/font/lv_font_simsun_16_cjk.c)
sdk_src(src/font/lv_font_montserrat_24.c)
sdk_src(src/font/lv_font_montserrat_12_subpx.c)
sdk_src(src/draw/lv_draw_line.c)
sdk_src(src/draw/lv_draw_triangle.c)
sdk_src(src/draw/lv_img_decoder.c)
sdk_src(src/draw/lv_draw_arc.c)
sdk_src(src/draw/lv_draw_rect.c)
sdk_src(src/draw/lv_draw_img.c)
sdk_src(src/draw/lv_img_cache.c)
sdk_src(src/draw/lv_draw_mask.c)
sdk_src(src/draw/lv_draw_label.c)
sdk_src(src/draw/lv_img_buf.c)
sdk_src(src/draw/lv_draw_layer.c)
sdk_src(src/draw/lv_draw_transform.c)
sdk_src(src/draw/lv_draw.c)

sdk_src(src/draw/sw/lv_draw_sw_transform.c)
sdk_src(src/draw/sw/lv_draw_sw_rect.c)
sdk_src(src/draw/sw/lv_draw_sw_polygon.c)
sdk_src(src/draw/sw/lv_draw_sw_line.c)
sdk_src(src/draw/sw/lv_draw_sw_letter.c)
sdk_src(src/draw/sw/lv_draw_sw_layer.c)
sdk_src(src/draw/sw/lv_draw_sw_img.c)
sdk_src(src/draw/sw/lv_draw_sw_gradient.c)
sdk_src(src/draw/sw/lv_draw_sw_dither.c)
sdk_src(src/draw/sw/lv_draw_sw_blend.c)
sdk_src(src/draw/sw/lv_draw_sw_arc.c)
sdk_src(src/draw/sw/lv_draw_sw.c)

sdk_src(src/draw/sdl/lv_draw_sdl_utils.c)
sdk_src(src/draw/sdl/lv_draw_sdl_texture_cache.c)
sdk_src(src/draw/sdl/lv_draw_sdl_stack_blur.c)
sdk_src(src/draw/sdl/lv_draw_sdl_rect.c)
sdk_src(src/draw/sdl/lv_draw_sdl_polygon.c)
sdk_src(src/draw/sdl/lv_draw_sdl_mask.c)
sdk_src(src/draw/sdl/lv_draw_sdl_line.c)
sdk_src(src/draw/sdl/lv_draw_sdl_layer.c)
sdk_src(src/draw/sdl/lv_draw_sdl_label.c)
sdk_src(src/draw/sdl/lv_draw_sdl_img.c)
sdk_src(src/draw/sdl/lv_draw_sdl_composite.c)
sdk_src(src/draw/sdl/lv_draw_sdl_bg.c)
sdk_src(src/draw/sdl/lv_draw_sdl_arc.c)
sdk_src(src/draw/sdl/lv_draw_sdl.c)

sdk_src(src/draw/hpm67xx_pdma/lv_gpu_hpm_pdma.c)

sdk_src(src/widgets/lv_label.c)
sdk_src(src/widgets/lv_roller.c)
sdk_src(src/widgets/lv_btn.c)
sdk_src(src/widgets/lv_textarea.c)
sdk_src(src/widgets/lv_btnmatrix.c)
sdk_src(src/widgets/lv_img.c)
sdk_src(src/widgets/lv_slider.c)
sdk_src(src/widgets/lv_dropdown.c)
sdk_src(src/widgets/lv_bar.c)
sdk_src(src/widgets/lv_table.c)
sdk_src(src/widgets/lv_arc.c)
sdk_src(src/widgets/lv_line.c)
sdk_src(src/widgets/lv_checkbox.c)
sdk_src(src/widgets/lv_switch.c)
sdk_src(src/widgets/lv_canvas.c)
sdk_src(src/widgets/lv_objx_templ.c)

