CC = gcc
LD = gcc
CFLAGS += -Wall -Wextra -Wpedantic -fPIC
ASAP = asap-5.2.0

INSTALL := install -m664
MKDIR := mkdir
RMDIR := rmdir
LN := ln
RM := rm

PLUGINDIR := $(PREFIX)/lib/mpv-mpris
PREFIX := /usr/local
SYS_SCRIPTS_DIR := /etc/mpv/scripts
USR_SCRIPTS_DIR := $(HOME)/.config/mpv/scripts

.PHONY: clean install-system install-user \
	uninstall-system uninstall-user

mpv-sap.so:	asap.o mpv-sap.o
	$(LD) -shared -o $@ $^

asap.o:
	$(CC) $(CFLAGS) -c -o $@ $(ASAP)/asap.c

mpv-sap.o:	mpv-sap.c $(ASAP)/asap.h
	$(CC) $(CFLAGS) -DASAP_H='"$(ASAP)/asap.h"' -c -o $@ $<


ifneq ($(shell id -u),0)
install: install-user
uninstall: uninstall-user
else
install: install-system
uninstall: uninstall-system
endif

install-user: mpv-sap.so
	$(MKDIR) -p $(USR_SCRIPTS_DIR)
	$(INSTALL) mpv-sap.so $(USR_SCRIPTS_DIR)/sap.so

uninstall-user:
	$(RM) -f $(USR_SCRIPTS_DIR)/sap.so
	$(RMDIR) -p $(USR_SCRIPTS_DIR)

install-system: mpv-sap.so
	$(MKDIR) -p $(DESTDIR)$(PLUGINDIR)
	$(INSTALL) mpv-sap.so $(DESTDIR)$(PLUGINDIR)/sap.so
	$(MKDIR) -p $(DESTDIR)$(SYS_SCRIPTS_DIR)
	$(LN) -s $(PLUGINDIR)/sap.so $(DESTDIR)$(SYS_SCRIPTS_DIR)

uninstall-system:
	$(RM) -f $(DESTDIR)$(SYS_SCRIPTS_DIR)/sap.so
	$(RMDIR) -p $(DESTDIR)$(SYS_SCRIPTS_DIR)
	$(RM) -f $(DESTDIR)$(PLUGINDIR)/sap.so
	$(RMDIR) -p $(DESTDIR)$(PLUGINDIR)

clean:
	rm -rf asap.o mpv-sap.o mpv-sap.so
