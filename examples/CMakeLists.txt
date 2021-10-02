
set(RTSV_EX ${CMAKE_CURRENT_SOURCE_DIR})
set(RTSV_CMD -queue 100 -vcd_out 1 -msc_out 1 -msc_page_max_levels 30 -msc_inst_dist 40 -msc_box_height 8 -msc_level_height 10 -msc_mark_grain 2 -msc_mark_disp 1)

# full feature single processor example
add_custom_command(OUTPUT general.pdf
                   COMMAND rtsv ARGS ${RTSV_CMD} -vcd general.vcd -msc general.tex -- ${RTSV_EX}/general.rts
						 DEPENDS general.rts)
add_custom_target(general DEPENDS general.pdf)
add_custom_target(general-view COMMAND okular general.pdf DEPENDS general)


# host/copro example
add_custom_command(OUTPUT vsora.pdf
  COMMAND rtsv ARGS ${RTSV_CMD} -msc vsora.tex -vcd vsora.vcd -- ${RTSV_EX}/host.rts ${RTSV_EX}/copro.rts
						 DEPENDS host.rts copro.rts)
add_custom_target(vsora DEPENDS vsora.pdf)
add_custom_target(vsora-view COMMAND okular vsora.pdf DEPENDS vsora)
