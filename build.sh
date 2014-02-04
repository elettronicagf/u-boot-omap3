#choose the right one for your toolchain
CROSS_COMPILE=arm-angstrom-linux-gnueabi-

make CROSS_COMPILE=$CROSS_COMPILE distclean
make CROSS_COMPILE=$CROSS_COMPILE omap3_egf_config
make CROSS_COMPILE=$CROSS_COMPILE 


