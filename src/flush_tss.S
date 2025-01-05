.global flush_tss
flush_tss:
	mov $0x18, %ax
	ltr %ax
	ret