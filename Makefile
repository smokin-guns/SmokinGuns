#######################################
# Makefile interface for cons
#######################################

all:
	@echo "==> Making SG [RELEASE] (engine and mod)"
	cd ./code && perl ./unix/cons -- vm noso
debug:
	@echo "==> Making SG [DEBUG] (engine and mod)"
	cd ./code && perl ./unix/cons -- vm noso debug
engine:
	@echo "==> Making SG [RELEASE] (engine only)"
	cd ./code && perl ./unix/cons -- noso
engine-debug:
	@echo "==> Making SG [DEBUG] (engine only)"
	cd ./code && perl ./unix/cons -- noso debug
engine-pk3:
	@echo "==> Making engine only PK3 for SG"
	cd ./code/install/missionpack && zip vm/*.qvm ../sg_engine.pk3
mod:
	@echo "==> Making SG [RELEASE] (mod only)"
	cd ./code && perl ./unix/cons -- vm noso nocore
mod-debug:
	@echo "==> Making SG [DEBUG] (mod only)"
	cd ./code && perl ./unix/cons -- vm noso nocore debug
bspc:
	@echo "==> Making SG [RELEASE] (BSPC only)"
	cd ./code && perl ./unix/cons -- nocore noso bspc
clean:
	@echo "==> Cleaning SG [RELEASE]"
	cd ./code && perl ./unix/cons -- clean
clean-debug:
	@echo "==> Cleaning SG [DEBUG]"
	cd ./code && perl ./unix/cons -- clean debug
