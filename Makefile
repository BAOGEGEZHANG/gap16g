DRV_DIR = $(shell pwd)/drv
LIB_DIR = $(shell pwd)/lib
SRC_DIR = $(shell pwd)/src
######################################################################
all:
	$(MAKE) -C $(DRV_DIR)
	$(MAKE) -C $(LIB_DIR)
	$(MAKE) -C $(SRC_DIR)
	
clean:
	$(MAKE) -C $(DRV_DIR) clean
	$(MAKE) -C $(LIB_DIR) clean
	$(MAKE) -C $(SRC_DIR) clean
