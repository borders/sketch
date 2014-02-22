
bin_PROGRAMS=main

main_LDADD   = -lm -lgsl -lgslcblas $(GTK2_LIBS)
main_CFLAGS  = $(GTK2_CFLAGS)

main_SOURCES = main.c \
	constraint.c \
	sketch_types.c \
	solver.c gui.c \
	draw_gtk_cairo.c
