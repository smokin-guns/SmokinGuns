#######################################
# Makefile interface for cons
#######################################

REVISION=$(shell LANG=C svnversion -n .)

all:
	@echo "==> Making SG [RELEASE] (engine and mod)"
	cd ./code && perl ./unix/cons -- vm rev=$(REVISION)
debug:
	@echo "==> Making SG [DEBUG] (engine and mod)"
	cd ./code && perl ./unix/cons -- vm debug rev=$(REVISION)
engine:
	@echo "==> Making SG [RELEASE] (engine only)"
	cd ./code && perl ./unix/cons --  rev=$(REVISION)
engine-debug:
	@echo "==> Making SG [DEBUG] (engine only)"
	cd ./code && perl ./unix/cons -- debug rev=$(REVISION)
engine-sse:
	@echo "==> Making SG [RELEASE] (engine only, with sse support)"
	cd ./code && perl ./unix/cons -- sse rev=$(REVISION)
engine-sse2:
	@echo "==> Making SG [RELEASE] (engine only, with sse2 support)"
	cd ./code && perl ./unix/cons -- sse2 rev=$(REVISION)
engine-smp:
	@echo "==> Making SG [RELEASE] (engine only, with smp and sse2 support)"
	cd ./code && perl ./unix/cons -- smp sse2 rev=$(REVISION)
engine-static:
	@echo "==> Making SG [RELEASE] (static dedicated engine only)"
	cd ./code && perl ./unix/cons -- static rev=$(REVISION)
engine-sse-static:
	@echo "==> Making SG [RELEASE] (static dedicated engine only, with sse support)"
	cd ./code && perl ./unix/cons -- sse static rev=$(REVISION)
engine-sse2-static:
	@echo "==> Making SG [RELEASE] (static dedicated engine only, with sse2 support)"
	cd ./code && perl ./unix/cons -- sse2 static rev=$(REVISION)
mod:
	@echo "==> Making SG [RELEASE] (mod only)"
	cd ./code && perl ./unix/cons -- vm nocore rev=$(REVISION)
mod-pk3: mod
	@echo "==> Making engine only PK3 for SG"
	cd ./install/smokinguns && zip -0 sg_engine.pk3 vm/*.qvm
mod-debug:
	@echo "==> Making SG [DEBUG] (mod only)"
	cd ./code && perl ./unix/cons -- vm nocore debug rev=$(REVISION)
so:
	@echo "==> Making SG [RELEASE] (binary mod only)"
	cd ./code && perl ./unix/cons -- nocore so rev=$(REVISION)
so-debug:
	@echo "==> Making SG [DEBUG] (binary mod only)"
	cd ./code && perl ./unix/cons -- nocore so debug rev=$(REVISION)
so-sse:
	@echo "==> Making SG [RELEASE] (binary mod, only with sse support)"
	cd ./code && perl ./unix/cons -- nocore so sse rev=$(REVISION)
so-sse2:
	@echo "==> Making SG [RELEASE] (binary mod, only with sse2 support)"
	cd ./code && perl ./unix/cons -- nocore so sse2 rev=$(REVISION)
bspc:
	@echo "==> Making SG [RELEASE] (BSPC only)"
	cd ./code && perl ./unix/cons -- nocore bspc rev=$(REVISION)
clean:
	@echo "==> Cleaning SG [RELEASE]"
	cd ./code && perl ./unix/cons -- clean
clean-debug:
	@echo "==> Cleaning SG [DEBUG]"
	cd ./code && perl ./unix/cons -- clean debug
