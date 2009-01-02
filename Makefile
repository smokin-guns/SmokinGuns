#######################################
# Makefile interface for cons
#######################################

all:
	@echo "==> Making SG [RELEASE] (engine and mod)"
	cd ./code && perl ./unix/cons -- vm
debug:
	@echo "==> Making SG [DEBUG] (engine and mod)"
	cd ./code && perl ./unix/cons -- vm debug
engine:
	@echo "==> Making SG [RELEASE] (engine only)"
	cd ./code && perl ./unix/cons
engine-debug:
	@echo "==> Making SG [DEBUG] (engine only)"
	cd ./code && perl ./unix/cons -- debug
engine-sse:
	@echo "==> Making SG [RELEASE] (engine only, with sse support)"
	cd ./code && perl ./unix/cons -- sse
engine-sse2:
	@echo "==> Making SG [RELEASE] (engine only, with sse2 support)"
	cd ./code && perl ./unix/cons -- sse2
engine-smp:
	@echo "==> Making SG [RELEASE] (engine only, with smp and sse2 support)"
	cd ./code && perl ./unix/cons -- smp sse2
engine-static:
	@echo "==> Making SG [RELEASE] (static dedicated engine only)"
	cd ./code && perl ./unix/cons -- static
engine-sse-static:
	@echo "==> Making SG [RELEASE] (static dedicated engine only, with sse support)"
	cd ./code && perl ./unix/cons -- sse static
engine-sse2-static:
	@echo "==> Making SG [RELEASE] (static dedicated engine only, with sse2 support)"
	cd ./code && perl ./unix/cons -- sse2 static
mod:
	@echo "==> Making SG [RELEASE] (mod only)"
	cd ./code && perl ./unix/cons -- vm nocore
mod-pk3: mod
	@echo "==> Making engine only PK3 for SG"
	cd ./code/install/smokinguns && zip -0 ../sg_engine.pk3 vm/*.qvm
mod-debug:
	@echo "==> Making SG [DEBUG] (mod only)"
	cd ./code && perl ./unix/cons -- vm nocore debug
so:
	@echo "==> Making SG [RELEASE] (binary mod only)"
	cd ./code && perl ./unix/cons -- nocore so
so-debug:
	@echo "==> Making SG [DEBUG] (binary mod only)"
	cd ./code && perl ./unix/cons -- nocore so debug
so-sse:
	@echo "==> Making SG [RELEASE] (binary mod, only with sse support)"
	cd ./code && perl ./unix/cons -- nocore so sse
so-sse2:
	@echo "==> Making SG [RELEASE] (binary mod, only with sse2 support)"
	cd ./code && perl ./unix/cons -- nocore so sse2
bspc:
	@echo "==> Making SG [RELEASE] (BSPC only)"
	cd ./code && perl ./unix/cons -- nocore bspc
clean:
	@echo "==> Cleaning SG [RELEASE]"
	cd ./code && perl ./unix/cons -- clean
clean-debug:
	@echo "==> Cleaning SG [DEBUG]"
	cd ./code && perl ./unix/cons -- clean debug
