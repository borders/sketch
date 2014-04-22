
bin_PROGRAMS=sketch

sketch_LDADD   = -lm -lgsl -lgslcblas $(GTK2_LIBS)
sketch_CFLAGS  = $(GTK2_CFLAGS)

sketch_SOURCES = main.c \
	constraint.c \
	sketch_types.c \
	solver.c gui.c \
	draw_gtk_cairo.c
