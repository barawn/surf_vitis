# Usage with Vitis IDE:
# In Vitis IDE create a Single Application Debug launch configuration,
# change the debug type to 'Attach to running target' and provide this 
# tcl script in 'Execute Script' option.
# Path of this script: X:\vitis_2022\surf_startup_system\_ide\scripts\systemdebugger_surf_startup_system_standalone.tcl
# 
# 
# Usage with xsct:
# In an external shell use the below command and launch symbol server.
# symbol_server.bat -S -s tcp::1534
# To debug using xsct, launch xsct and run below command
# source X:\vitis_2022\surf_startup_system\_ide\scripts\systemdebugger_surf_startup_system_standalone.tcl
# 
connect -path [list tcp::1534 tcp:localhost:3121]
source C:/Xilinx/Vitis/2022.1/scripts/vitis/util/zynqmp_utils.tcl
targets -set -nocase -filter {name =~"APU*" && jtag_cable_name =~ "Platform Cable USB II 13724327082b01" && jtag_device_ctx=="jsn-DLC10-13724327082b01-5ba00477-0"}
rst -system
after 3000
targets -set -filter {jtag_cable_name =~ "Platform Cable USB II 13724327082b01" && level==0 && jtag_device_ctx=="jsn-DLC10-13724327082b01-147e5093-0"}
fpga -file X:/vitis_2022/surf_startup/_ide/bitstream/surf_test.bit
targets -set -nocase -filter {name =~"APU*" && jtag_cable_name =~ "Platform Cable USB II 13724327082b01" && jtag_device_ctx=="jsn-DLC10-13724327082b01-5ba00477-0"}
loadhw -hw X:/vitis_2022/surf_test/export/surf_test/hw/surf_test.xsa -mem-ranges [list {0x80000000 0xbfffffff} {0x400000000 0x5ffffffff} {0x1000000000 0x7fffffffff}] -regs
configparams force-mem-access 1
targets -set -nocase -filter {name =~"APU*" && jtag_cable_name =~ "Platform Cable USB II 13724327082b01" && jtag_device_ctx=="jsn-DLC10-13724327082b01-5ba00477-0"}
set mode [expr [mrd -value 0xFF5E0200] & 0xf]
targets -set -nocase -filter {name =~ "*A53*#0" && jtag_cable_name =~ "Platform Cable USB II 13724327082b01" && jtag_device_ctx=="jsn-DLC10-13724327082b01-5ba00477-0"}
rst -processor
dow X:/vitis_2022/surf_test/export/surf_test/sw/surf_test/boot/fsbl.elf
set bp_43_4_fsbl_bp [bpadd -addr &XFsbl_Exit]
con -block -timeout 60
bpremove $bp_43_4_fsbl_bp
targets -set -nocase -filter {name =~ "*A53*#0" && jtag_cable_name =~ "Platform Cable USB II 13724327082b01" && jtag_device_ctx=="jsn-DLC10-13724327082b01-5ba00477-0"}
rst -processor
dow X:/vitis_2022/surf_startup/Debug/surf_startup.elf
configparams force-mem-access 0
bpadd -addr &main
