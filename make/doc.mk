# /ADK/make/doc.mk
#
# This file is a part of ADK library.
# Copyright (c) 2012, Artyom Lebedev <artyom.lebedev@gmail.com>
# All rights reserved.
# See COPYING file for copyright details.

ifdef ADK_DOC_NAMESPACE
ADK_SERV_DIR = $(1)/$(2)/$(ADK_DOC_NAMESPACE)
else
ADK_SERV_DIR = $(1)/$(2)
endif

ADK_DOC_DEPLOY_DIR += $(ADK_DOC_DIR)

.PHONY: update

update:
	if [ -d $(call ADK_SERV_DIR,$(ADK_DOC_DIR),pages) ]; then \
		$(RSYNC) -arvz --delete $(call ADK_SERV_DIR,$(ADK_DOC_DIR),pages)/ pages; \
	else \
		echo "Warning: no pages directory"; \
	fi
	if [ -d $(call ADK_SERV_DIR,$(ADK_DOC_DIR),media) ]; then \
		$(RSYNC) -arvz --delete $(call ADK_SERV_DIR,$(ADK_DOC_DIR),media)/ media; \
	else \
		echo "Warning: no media directory"; \
	fi

deploy:
	$(foreach dir, $(ADK_DOC_DEPLOY_DIR), $(RSYNC) -arvz --delete pages/ $(call ADK_SERV_DIR,$(dir),pages) &&) true
	$(foreach dir, $(ADK_DOC_DEPLOY_DIR), $(RSYNC) -arvz --delete media/ $(call ADK_SERV_DIR,$(dir),media) &&) true
