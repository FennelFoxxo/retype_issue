FAT_PART_SIZE_MB = 40
MIN_ROOTSERVER_SIZE_MB = 1

SEL4_KERNEL = kernel-x86_64-pc99
SEL4_ROOTSERVER = hello-world-image-x86_64-pc99
GRUB_CFG = grub.cfg

SEL4_D = sel4/
GRUB_D = grub/
BUILD_D = build/
TEMP_D = $(BUILD_D)/temp
ESP_ROOT_D = $(TEMP_D)/esp_root

.PHONY: all clean

all: | $(BUILD_D)/disk.img $(BUILD_D)/vbox_disk.vdi


# Make final image
$(BUILD_D)/disk.img: $(TEMP_D)/esp.img
	dd if=$(TEMP_D)/esp_part.img of=$(TEMP_D)/disk.img oflag=seek_bytes bs=1M seek=2048b status=none # Copy ESP to end of current disk image

	parted -s $(TEMP_D)/disk.img mklabel gpt mkpart '"EFI SYSTEM"' fat16 2048s 100% set 1 esp on # Create partition entry for ESP

	cp $(TEMP_D)/disk.img $@

# Make and format image of EFI partition and copy files into it
$(TEMP_D)/esp.img: $(ESP_ROOT_D)/EFI/BOOT/BOOTX64.EFI $(ESP_ROOT_D)/boot/sel4kernel $(ESP_ROOT_D)/boot/rootserver $(ESP_ROOT_D)/EFI/BOOT/grub.cfg
	dd if=/dev/zero of=$@ bs=1M count=$(FAT_PART_SIZE_MB) status=none
	mkfs.vfat -F 32 -n "EFI SYSTEM" $@
	mcopy -i $@ -s $(ESP_ROOT_D)/* ::

$(ESP_ROOT_D)/EFI/BOOT/BOOTX64.EFI: $(GRUB_D)/grub-mkimage
	mkdir -p $(@D)
	$(GRUB_D)/grub-mkimage -d $(GRUB_D)/grub-core -o $@ -O x86_64-efi -p "" configfile fat part_gpt gzio multiboot reboot cpuid echo sleep video video_bochs video_cirrus efi_gop efi_uga normal chain boot multiboot2


# Copy files to final locations

$(ESP_ROOT_D)/EFI/BOOT/grub.cfg: $(GRUB_CFG)
	cp $< $@

$(ESP_ROOT_D)/boot/sel4kernel: $(SEL4_D)/hello-world_build/images/$(SEL4_KERNEL)
	cp $< $@

$(ESP_ROOT_D)/boot/rootserver: $(SEL4_D)/hello-world_build/images/$(SEL4_ROOTSERVER)
	dd if=/dev/zero of=$@ bs=1M count=$(MIN_ROOTSERVER_SIZE_MB)
	dd if=$< of=$@ conv=notrunc

$(SEL4_D)/hello-world_build/images/$(SEL4_KERNEL): $(SEL4_D)/build/build.ninja
	cd $(SEL4_D)/build/ && ninja

$(SEL4_D)/hello-world_build/images/$(SEL4_ROOTSERVER): $(SEL4_D)/build/build.ninja
	cd $(SEL4_D)/build/ && ninja

$(SEL4_D)/build/build.ninja:
	mkdir -p $(@D)
	cd $(SEL4_D)/build/ && ../init-build.sh -DPLATFORM=x86_64 -DSIMULATION=TRUE -DKernelHugePage=OFF


$(GRUB_D)/grub-mkimage:
	cd $(GRUB_D) && ./bootstrap && ./configure --target=x86_64 --with-platform=efi && make


clean:
	rm -rf $(TEMP_D)
	rm -rf $(SEL4_D)/build/