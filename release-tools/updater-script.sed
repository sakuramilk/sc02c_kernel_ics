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

