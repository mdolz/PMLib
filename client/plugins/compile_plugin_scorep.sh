icc -c -fPIC scorep_pmlib_plugin.c -o libPmlibPlugin.o -I/home/dolz/pmlib `pkg-config --cflags --libs glib-2.0` `scorep-config --cppflags`
icc -shared -Wl,-soname,libPmlibPlugin.so -o libPmlibPlugin.so libPmlibPlugin.o /home/dolz/pmlib/pmlib.a `pkg-config --libs glib-2.0`

