all: libimlayouts.so.0.0.0 gen_vkb

libimlayouts.so.0.0.0: imlayouts.c imlayouts.h
	$(CC) -Wall -fPIC $(CFLAGS) $(shell pkg-config --cflags --libs glib-2.0) --std=gnu99 -shared -Wl,-soname=libimlayouts.so.0 $< -o $@

gen_vkb: gen_vkb.c
	$(CC) -Wall $(CFLAGS) $(shell pkg-config --cflags --libs glib-2.0 expat) libimlayouts.so.0.0.0 --std=gnu99 $< -o $@

clean:
	$(RM) libimlayouts.so.0.0.0 gen_vkb

install:
	install -d "$(DESTDIR)/usr/lib/"
	install -m 644 libimlayouts.so.0.0.0 "$(DESTDIR)/usr/lib/"
	ln -s libimlayouts.so.0.0.0 "$(DESTDIR)/usr/lib/libimlayouts.so.0"
	ln -s libimlayouts.so.0 "$(DESTDIR)/usr/lib/libimlayouts.so"
	install -d "$(DESTDIR)/usr/include/"
	install -m 644 imlayouts.h "$(DESTDIR)/usr/include/"
	install -d "$(DESTDIR)/usr/lib/pkgconfig"
	install -m 644 imlayouts.pc "$(DESTDIR)/usr/lib/pkgconfig/"
	install -d "$(DESTDIR)/usr/bin/"
	install -m 755 gen_vkb "$(DESTDIR)/usr/bin/"
	install -m 755 decode_vkb.pl "$(DESTDIR)/usr/bin/"

