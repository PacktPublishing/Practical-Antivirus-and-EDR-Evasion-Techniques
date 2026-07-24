rule shellcode_detected
{

	strings:
	$shellcode_entry_bytes = { FC 48 83 E4 F0 E8 }
	$shellcode_ending_bytes = { 59 41 89 DA FF D5 }
	$VirtualAlloc_str = "VirtualAlloc"
	$RtlMoveMemory_str = "RtlMoveMemory"
	$VirtualProtect_str = "VirtualProtect"
	$CreateThread_str = "CreateThread"

	condition:
		any of them

}
