# 
# Usage: To re-create this platform project launch xsct with below options.
# xsct X:\vitis_2022\surf_test\platform.tcl
# 
# OR launch xsct and run below command.
# source X:\vitis_2022\surf_test\platform.tcl
# 
# To create the platform in a different location, modify the -out option of "platform create" command.
# -out option specifies the output directory of the platform project.

platform create -name {surf_test}\
-hw {V:\surf_test\hardware\surf_test.xsa}\
-proc {psu_cortexa53_0} -os {standalone} -arch {64-bit} -fsbl-target {psu_cortexa53_0} -out {X:/vitis_2022}

platform write
platform generate -domains 
platform active {surf_test}
platform generate
bsp reload
bsp config stdin "psu_coresight_0"
bsp config stdout "psu_coresight_0"
bsp write
bsp reload
catch {bsp regenerate}
platform generate -domains standalone_domain 
platform active {surf_test}
bsp reload
bsp config stdin "psu_coresight_0"
bsp write
platform generate -domains 
