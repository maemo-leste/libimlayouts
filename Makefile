all: libimlayouts.so.0.0.0

libimlayouts.so.0.0.0: imlayouts.c imlayouts.h
	$(CC) -Wall -fPIC $(CFLAGS) $(shell pkg-config --cflags --libs glib-2.0) --std=gnu99 -shared -Wl,-soname=libimlayouts.so.0 $< -o $@

clean:
	$(RM) libimlayouts.so.0.0.0

install:
	install -d "$(DESTDIR)/usr/lib/"
	install -m 644 libimlayouts.so.0.0.0 "$(DESTDIR)/usr/lib/"
	ln -s libimlayouts.so.0.0.0 "$(DESTDIR)/usr/lib/libimlayouts.so.0"
	ln -s libimlayouts.so.0 "$(DESTDIR)/usr/lib/libimlayouts.so"
	install -d "$(DESTDIR)/usr/include/"
	install -m 644 imlayouts.h "$(DESTDIR)/usr/include/"
	install -d "$(DESTDIR)/usr/lib/pkgconfig"
	install -m 644 imlayouts.pc "$(DESTDIR)/usr/lib/pkgconfig/"
