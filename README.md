# PMIC-debugfs
- Debug FS to access TPS register - TPS65910/AM335x
- Just call "pmic_debugfs_init" function in the TPS driver code to intialize debugfs for TPS
- A node would be created under /sys/debug/pmic, use redirect symbols to read and write register
