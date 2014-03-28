# /ADK/make/doc.mk
#
# This file is a part of ADK library.
# Copyright (c) 2012-2014, Artyom Lebedev <artyom.lebedev@gmail.com>
# All rights reserved.
# See COPYING file for copyright details.

ifdef ADK_DOC_NAMESPACE
ADK_SERV_DIR = $(1)/$(2)/$(ADK_DOC_NAMESPACE)
else
ADK_SERV_DIR = $(1)/$(2)
endif

ADK_DOC_DEPLOY_DIR += $(ADK_DOC_DIR)

ADK_DOC_RSYNC_FLAGS = -rvzlh --delete --inplace

.PHONY: update deploy

update:
	@if [ -d $(call ADK_SERV_DIR,$(ADK_DOC_DIR),pages) ]; then \
		$(RSYNC) $(ADK_DOC_RSYNC_FLAGS) $(call ADK_SERV_DIR,$(ADK_DOC_DIR),pages)/ pages; \
	else \
		echo "Warning: no pages directory"; \
	fi
	@if [ -d $(call ADK_SERV_DIR,$(ADK_DOC_DIR),media) ]; then \
		$(RSYNC) $(ADK_DOC_RSYNC_FLAGS) $(call ADK_SERV_DIR,$(ADK_DOC_DIR),media)/ media; \
	else \
		echo "Warning: no media directory"; \
	fi

deploy:
	$(foreach dir, $(ADK_DOC_DEPLOY_DIR), $(RSYNC) $(ADK_DOC_RSYNC_FLAGS) pages/ $(call ADK_SERV_DIR,$(dir),pages) &&) true
	$(foreach dir, $(ADK_DOC_DEPLOY_DIR), $(RSYNC) $(ADK_DOC_RSYNC_FLAGS) media/ $(call ADK_SERV_DIR,$(dir),media) &&) true
