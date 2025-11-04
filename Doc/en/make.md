# Make Guide

## Source Code
[Github](https://github.com/chaosgoo/mcompass)

## 3d Model
[MakerWorld CN](https://makerworld.com.cn/zh/models/667420#profileId-611642)
![MakerWorld CN](/makerworldcn.jpg)

## PCB

[PCB and Panel](https://oshwhub.com/chaosgoo/wcompass)

![Front](/FrontPCB.png)


![Bottom](/BottomPCB.png)


## Materials
* PCB board thickness: 1.0mm, black solder mask, manufactured by JLCPCB;
* Panel manufactured by LCSC Mall, parameters: 1.0mm semi-transparent black acrylic with strong light shielding, no back adhesive, to be glued together later;
* Light diffuser material: PET LGT075J, no back adhesive, to be glued together later;
* LED: WS2812B 0807 (1.7x2.0x0.85mm)
* Battery: 213455 500mAh.  (21mm × 34mm × 55mm)
* Hardware: M2×3×3.2 knurled nut, M2×4 hex screw.
* GPS: ATGM336H 5N71 module + antenna (13.1mm × 15.7mm).
* QMC5883L has been discontinued, now supports QMC5883P, and the firmware will automatically recognize it

## Q&A
### December 02, 2024

**Q**: How long does GPS positioning take?

**A**: According to the Taobao product description and actual tests, the first positioning takes more than 30 seconds.

**Q**: According to the LED datasheet in the project schematic, the operating voltage should be 3.5V~5.5V, but the schematic actually uses an LDO 3.3V output as the power supply.

**A**: This is a design error. From the actual working situation, no serious problems have been shown yet. If there is a plan for a second version, this design will be fixed.

### June 11, 2025
**Q**: No response when plugged into the computer after production.

**A**: 
* Check if the switch is turned on; flipping it upward is the on state.
* Check if the ESP32-C3 is soldered successfully. You can remove the chip, face the back upward, and check if any pins are not tinned.
    * The one in the picture just had no solder on D-, causing failure to connect to the computer. In this case, manually add solder to the module.
![bottom_esp32c3](/bottom_esp32c3.png)

* Check if the LDO output is normal.
* Check if the LEDs are oriented incorrectly.

**Q**: After production, when plugged into the computer, the Device Manager keeps refreshing.

**A**: A new ESP32-C3 without firmware will keep reporting Flash errors and restart continuously, thus losing the USB connection. Just flash the firmware directly.

**Q**: No response during flashing.

**A**: Please confirm whether the firmware is checked.
![checked_firmware_bin](/checked_firmware_bin.png)

**Q**: LEDs stop lighting up from a certain point.

**A**: Check the soldering of the last working LED and the first non-working LED.

**Q**: 'X4' are displayed after powering on.

**A**: Check the soldering of the geomagnetic sensor and whether the geomagnetic sensor model is correct.

**Q**: QMC5883L is discontinued and unavailable.

**A**: It can be replaced with QMC5883P, and the firmware already supports it.