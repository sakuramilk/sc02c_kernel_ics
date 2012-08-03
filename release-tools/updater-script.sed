assert(getprop("ro.product.device") == "galaxys2" || getprop("ro.build.product") == "galaxys2" || 
       getprop("ro.product.device") == "GT-I9100" || getprop("ro.build.product") == "GT-I9100" || 
       getprop("ro.product.device") == "GT-I9100M" || getprop("ro.build.product") == "GT-I9100M" || 
       getprop("ro.product.device") == "GT-I9100P" || getprop("ro.build.product") == "GT-I9100P" || 
       getprop("ro.product.device") == "SC-02C" || getprop("ro.build.product") == "SC-02C" || 
       getprop("ro.product.device") == "GT-I9100T" || getprop("ro.build.product") == "GT-I9100T");
ui_print("");
ui_print("");
ui_print("------------------------------------------------");
ui_print("@VERSION");
ui_print("  KBC Developers:");
ui_print("    Homura Akemi");
ui_print("    Ma34s3");
ui_print("    Sakuramilk");
ui_print("------------------------------------------------");
ui_print("");
show_progress(0.500000, 0);

ui_print("flashing kernel image...");
assert(package_extract_file("zImage", "/tmp/zImage"),
       write_raw_image("/tmp/zImage", "/dev/block/mmcblk0p5"),
       delete("/tmp/zImage"));
show_progress(0.100000, 0);

ui_print("flash complete. Enjoy!");
set_progress(1.000000);

