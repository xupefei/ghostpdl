# Portions Copyright (C) 2001 artofcode LLC. 
#  Portions Copyright (C) 1996, 2001 Artifex Software Inc.
#  Portions Copyright (C) 1988, 2000 Aladdin Enterprises.
#  This software is based in part on the work of the Independent JPEG Group.
#  All Rights Reserved.
#
#  This software is distributed under license and may not be copied, modified
#  or distributed except as expressly authorized under the terms of that
#  license.  Refer to licensing information at http://www.artifex.com/ or
#  contact Artifex Software, Inc., 101 Lucas Valley Road #110,
#  San Rafael, CA  94903, (415)492-9861, for further information.

# $RCSfile$ $Revision$
# Partial makefile common to all Unix and Desqview/X configurations.
# This is the next-to-last part of the makefile for these configurations.

# Define the rule for building standard configurations.
STDDIRS:
	@if test ! -d $(BINDIR); then mkdir $(BINDIR); fi
	@if test ! -d $(GLGENDIR); then mkdir $(GLGENDIR); fi
	@if test ! -d $(GLOBJDIR); then mkdir $(GLOBJDIR); fi
	@if test ! -d $(PSGENDIR); then mkdir $(PSGENDIR); fi
	@if test ! -d $(PSOBJDIR); then mkdir $(PSOBJDIR); fi

# Define a rule for building profiling configurations.
PGDIRS: STDDIRS
	@if test ! -d $(BINDIR)/$(PGRELDIR); then mkdir $(BINDIR)/$(PGRELDIR); fi
	@if test ! -d $(GLGENDIR)/$(PGRELDIR); then mkdir $(GLGENDIR)/$(PGRELDIR); fi
	@if test ! -d $(GLOBJDIR)/$(PGRELDIR); then mkdir $(GLOBJDIR)/$(PGRELDIR); fi
	@if test ! -d $(PSGENDIR)/$(PGRELDIR); then mkdir $(PSGENDIR)/$(PGRELDIR); fi
	@if test ! -d $(PSOBJDIR)/$(PGRELDIR); then mkdir $(PSOBJDIR)/$(PGRELDIR); fi

PGDEFS=GENOPT='-DPROFILE' CFLAGS='$(CFLAGS_PROFILE) $(GCFLAGS) $(XCFLAGS)'\
 LDFLAGS='$(XLDFLAGS) -pg' XLIBS='Xt SM ICE Xext X11' CC_LEAF='$(CC_LEAF_PG)'\
 BINDIR=$(BINDIR)/$(PGRELDIR)\
 GLGENDIR=$(GLGENDIR)/$(PGRELDIR) GLOBJDIR=$(GLOBJDIR)/$(PGRELDIR)\
 PSGENDIR=$(PSGENDIR)/$(PGRELDIR) PSOBJDIR=$(PSOBJDIR)/$(PGRELDIR)

pg: PGDIRS
	$(MAKE) $(PGDEFS) default

pgclean: PGDIRS
	$(MAKE) $(PGDEFS) clean

# Define a rule for building debugging configurations.
DEBUGDIRS: STDDIRS
	@if test ! -d $(BINDIR)/$(DEBUGRELDIR); then mkdir $(BINDIR)/$(DEBUGRELDIR); fi
	@if test ! -d $(GLGENDIR)/$(DEBUGRELDIR); then mkdir $(GLGENDIR)/$(DEBUGRELDIR); fi
	@if test ! -d $(GLOBJDIR)/$(DEBUGRELDIR); then mkdir $(GLOBJDIR)/$(DEBUGRELDIR); fi
	@if test ! -d $(PSGENDIR)/$(DEBUGRELDIR); then mkdir $(PSGENDIR)/$(DEBUGRELDIR); fi
	@if test ! -d $(PSOBJDIR)/$(DEBUGRELDIR); then mkdir $(PSOBJDIR)/$(DEBUGRELDIR); fi

DEBUGDEFS=GENOPT='-DDEBUG' CFLAGS='$(CFLAGS_DEBUG) $(GCFLAGS) $(XCFLAGS)'\
 BINDIR=$(BINDIR)/$(DEBUGRELDIR)\
 GLGENDIR=$(GLGENDIR)/$(DEBUGRELDIR) GLOBJDIR=$(GLOBJDIR)/$(DEBUGRELDIR)\
 PSGENDIR=$(PSGENDIR)/$(DEBUGRELDIR) PSOBJDIR=$(PSOBJDIR)/$(DEBUGRELDIR)

debug: DEBUGDIRS
	$(MAKE) $(DEBUGDEFS) default

debugclean: DEBUGDIRS
	$(MAKE) $(DEBUGDEFS) clean

# The rule for gconfigv.h is here because it is shared between Unix and
# DV/X environments.
$(gconfigv_h): $(GLSRC)unix-end.mak $(TOP_MAKEFILES) $(ECHOGS_XE)
	$(ECHOGS_XE) -w $(gconfigv_h) -x 23 define USE_ASM -x 2028 -q $(USE_ASM)-0 -x 29
	$(ECHOGS_XE) -a $(gconfigv_h) -x 23 define USE_FPU -x 2028 -q $(FPU_TYPE)-0 -x 29
	$(ECHOGS_XE) -a $(gconfigv_h) -x 23 define EXTEND_NAMES 0$(EXTEND_NAMES)
	$(ECHOGS_XE) -a $(gconfigv_h) -x 23 define SYSTEM_CONSTANTS_ARE_WRITABLE 0$(SYSTEM_CONSTANTS_ARE_WRITABLE)

# Emacs tags maintenance.

TAGS:
	etags -t $(GLSRC)*.[ch] $(PSSRC)*.[ch]
