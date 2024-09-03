FAT_PART_SIZE_MB = 40
MIN_ROOTSERVER_SIZE_MB = 1

SEL4_KERNEL = kernel-x86_64-pc99
SEL4_ROOTSERVER = myproject-image-x86_64-pc99
GRUB_CFG = grub.cfg
QEMU_BIN = qemu-system-x86_64

SEL4_D = sel4/
GRUB_D = grub/

BUILD_D = build/
TEMP_D = $(BUILD_D)/temp
ESP_ROOT_D = $(TEMP_D)/esp_root
SEL4_PROJECT_SRC_D = $(SEL4_D)/projects/myproject/src/

SEL4_PROJECT_FILES = $(shell find $(SEL4_PROJECT_SRC_D) -name "*.*")

.PHONY: all clean run simulate

all: | $(BUILD_D)/disk.img


run: $(BUILD_D)/disk.img
	$(QEMU_BIN) -cpu Nehalem,-vme,-pdpe1gb,-xsave,-xsaveopt,-xsavec,-fsgsbase,-invpcid,+syscall,+lm,enforce -nographic -serial mon:stdio -m size=3G -drive if=pflash,format=raw,readonly=on,file=OVMF_CODE.fd -drive format=raw,file=build/disk.img

simulate: $(BUILD_D)/disk.img
	cd $(SEL4_D)/build/ && ./simulate

# Make final image
$(BUILD_D)/disk.img: $(TEMP_D)/esp.img
	dd if=$(TEMP_D)/esp.img of=$(TEMP_D)/disk.img oflag=seek_bytes bs=1M seek=2048b # Copy ESP to disk image, leaving first 2048 sectors empty

	parted -s $(TEMP_D)/disk.img mklabel gpt mkpart '"EFI SYSTEM"' fat16 2048s 100% set 1 esp on # Create partition entry for ESP

	cp $(TEMP_D)/disk.img $@

# Make and format image of EFI partition and copy files into it
$(TEMP_D)/esp.img: $(ESP_ROOT_D)/EFI/BOOT/BOOTX64.EFI $(ESP_ROOT_D)/boot/sel4kernel $(ESP_ROOT_D)/boot/rootserver $(ESP_ROOT_D)/EFI/BOOT/grub.cfg
	dd if=/dev/zero of=$@ bs=1M count=$(FAT_PART_SIZE_MB) status=none
	mkfs.vfat -F 32 -n "EFI SYSTEM" $@
	mcopy -i $@ -s $(ESP_ROOT_D)/* ::

# sel4 compilation

$(SEL4_D)/build/build.ninja:
	mkdir -p $(@D)
	cd $(SEL4_D)/build/ && ../init-build.sh -DPLATFORM=x86_64 -DSIMULATION=TRUE -DKernelHugePage=OFF

$(SEL4_D)/build/images/$(SEL4_KERNEL): $(SEL4_D)/build/build.ninja
	cd $(SEL4_D)/build/ && ninja

$(SEL4_D)/build/images/$(SEL4_ROOTSERVER): $(SEL4_D)/build/build.ninja $(SEL4_PROJECT_FILES)
	cd $(SEL4_D)/build/ && ninja


# Copy files to final locations

$(ESP_ROOT_D)/EFI/BOOT/grub.cfg: $(GRUB_CFG)
	cp $< $@

$(ESP_ROOT_D)/boot/sel4kernel: $(SEL4_D)/build/images/$(SEL4_KERNEL)
	mkdir -p $(@D)
	cp $< $@

$(ESP_ROOT_D)/EFI/BOOT/BOOTX64.EFI: BOOTX64.EFI
	mkdir -p $(@D)
	cp $< $@

# Copy file by first creating empty file to specified size, then copy rootserver contents into it
$(ESP_ROOT_D)/boot/rootserver: $(SEL4_D)/build/images/$(SEL4_ROOTSERVER)
	mkdir -p $(@D)
	dd if=/dev/zero of=$@ bs=1M count=$(MIN_ROOTSERVER_SIZE_MB)
	dd if=$< of=$@ conv=notrunc


clean:
	rm -rf $(BUILD_D)/
	cd $(SEL4_D)/build/ && ninja clean